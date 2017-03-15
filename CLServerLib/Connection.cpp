// Connection.cpp: implementation of the CConnection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Connection.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Static 변수 초기화 
int CConnection::m_nConnCnt = 0;
//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 생성자										//
// [2]PARAMETER : void											//
// [3]RETURN : void												//
// [4]DATE : 2000년 9월 5일										//
//////////////////////////////////////////////////////////////////
CConnection::CConnection()
{
	m_nMaxBuf = m_nMaxPacketSize =	m_nCheckPakcetSize = 0;
	m_nMaxUser		= 0;
	m_sckListener	= NULL;
	m_Socket		= NULL;

	m_bIsConnect	= FALSE;
	m_ConnType		= UNKNOWN_CONNECTION;

	m_hIOCP			= NULL;
	m_pPacketPool	= NULL;
	m_uRecvTickCnt	= 0;

	
	m_nRecvFlag = 0;
	m_nRecvTotalSize;
	m_nRecvRead;					// 현재까지 읽은 값 
	m_szRcvData[0] = NULL;


	InitializeCriticalSectionAndSpinCount( &m_CS, 4000 );   // why 4000?
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 소멸자 										//
// [2]PARAMETER : void											//
// [3]RETURN : void												//
// [4]DATE : 2000년 9월 5일										//
//////////////////////////////////////////////////////////////////
CConnection::~CConnection()
{
	m_nMaxBuf = m_nMaxPacketSize =	m_nCheckPakcetSize = 0;
	m_nMaxUser		= 0;
	m_sckListener	= NULL;

	Disconnect();

	m_bIsConnect	= FALSE;
	m_ConnType		= UNKNOWN_CONNECTION;

	m_hIOCP			= NULL;
	m_pPacketPool	= NULL;
	m_uRecvTickCnt	= 0;

	DeleteCriticalSection( &m_CS);								// 크리티칼 섹션 변수 해제 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 멤버 변수 세팅 및 객체 초기화				//
// [2]PARAMETER : dwIndex - 객체 고유 ID						//
//				  hIOCP - IOCP Handle							//
//				  listener - Listen Socket						//
//				  pPacketPool - 패킷 Pool 포인터				//
//				  pMsg - BroadCasting을 휘한 메세지 처리 포인터 //
//				  MsgWait - 메세지 Send를 위한 이벤트 핸들		// 
// [3]RETURN : TRUE - 정상 처리, FALSE - 실패 					//
// [4]DATE : 2000년 9월 4일										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::Create(DWORD			dwIndex,
						 HANDLE			hIOCP,
						 SOCKET			listener,
						 CPacketPool*	pPacketPool,
						 char			cConnType,
						 int			nMaxBuf,
						 int			nCheckPacketSize,
						 int			nMaxUser)
{	
	// 객체의 멤버 변수 설정 
	m_dwIndex			= dwIndex;
	m_hIOCP				= hIOCP;
	m_sckListener		= listener;
	m_pPacketPool		= pPacketPool;

	m_nMaxBuf			= m_nMaxPacketSize		=	nMaxBuf;
	m_nCheckPakcetSize	= nCheckPacketSize;
	m_nMaxUser			= nMaxUser;

	// 객체 OPEN
	if(!this->Open())
	{
		// 실패하면 어떻게 해야 하는가? 이 컨넥션 클래스는 죽은 클래스가 된다.
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 객체 초기화, 리슨 소켓 결합					//
// [2]PARAMETER : void											//
// [3]RETURN : TRUE - 정상처리 , FALSE - 실패 					//
// [4]DATE : 2000년 9월 4일										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::Open()
{
	// 절대 값 있어야 함.
	assert(m_nMaxUser);
	assert(m_nMaxBuf);
	assert(m_nMaxPacketSize);

	// 패킷 Pool이 제대로 설정 되었는지 검사 
	if( !m_pPacketPool )
		return FALSE;

	// Listener Socket 상태 검사 
	if( m_sckListener == NULL )
		return FALSE;

 	// create socket for send/recv
	m_Socket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_IP,NULL,0,WSA_FLAG_OVERLAPPED );

	// 현재 소켓이 제대로 생성 되었는지 검사 
	if(m_Socket == NULL) return FALSE;

	// Accpet할 오버랩구조체와 패킷버퍼를 준비한다.
	LPOVERLAPPEDPLUS newolp = PrepareAcptPacket();

	if(NULL == newolp) return FALSE;

	/////////////////////////////////////////////////////////////////////
	// Socket과 Listener와 연결
	// Overlapped에 들어가는 변수가 나중에 IOCP 이벤트 발생 처리에 쓰인다 
	BOOL bRet = AcceptEx(newolp->sckListen,						// Listen Socket
						 newolp->sckClient,						// Socket
						 &(newolp->wbuf.buf[0]),				// 버퍼 포인터 
						 m_nMaxBuf,								// 버퍼 사이즈 
						 sizeof(sockaddr_in) + 16,				// 소켓 정보 - IP, Address, Name.. etc
						 sizeof(sockaddr_in) + 16,				// 소켓 정보 - IP, Address, Name.. etc
						 &newolp->dwBytes,						// 처리 바이트 크기 
						 &newolp->overlapped);					// *중요*

	newolp->ConnType	= m_ConnType;

	// 에러 처리 
	if(!bRet && WSAGetLastError() != WSA_IO_PENDING)
	{
		printf("%08X, AcceptEx(): SOCKET_ERROR, %d\n",	m_dwIndex, WSAGetLastError());
		closesocket(newolp->sckClient);
		newolp->sckClient = NULL;
		m_Socket = NULL;
		ReleaseAcptPacket(newolp);

		return FALSE;
	}

	/////////////////////////////////
	// 소켓의 성능 최적화를 위한 세팅 
	int zero = 0;
	int err = 0;

	// Send Buffer에 대한 세팅
	if( (err = setsockopt( m_Socket, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero))) == SOCKET_ERROR)
	{
	    closesocket(m_Socket);
		m_Socket = NULL;
		return FALSE;
    }

	// Receive Buffer에 대한 세팅 
	if((err = setsockopt( m_Socket, SOL_SOCKET, SO_RCVBUF, (char *)&zero, sizeof(zero) )) == SOCKET_ERROR)
	{
		closesocket(m_Socket);
		m_Socket = NULL;
		return FALSE;
    }
	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 객체의 클라이언트 연결 해제 및 초기화 		//
// [2]PARAMETER : lpOverlapPlus - 할당 패킷 					//
//				  bForce - 소켓의 버퍼 상태에 따른 처리를 위한..//
// [3]RETURN : TRUE - 정상 처리 								//
// [4]DATE : 2000년 9월 5일										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::Close_Open( LPOVERLAPPEDPLUS lpOverlapPlus, BOOL bForce )
{
	// 소켓과 리스너 상태 확인 
	if(m_Socket)
	{
		struct linger li = {0, 0};	// Default: SO_DONTLINGER

		// 소켓에 처리 되지 않은 데이타가 소켓 버퍼에 남아있다면,
		if(bForce) li.l_onoff = 1; // SO_LINGER, timeout = 0

		shutdown(m_Socket, SD_BOTH );						// 오잉? 이게 뭐지? ^^;; 담에 찾아보자 
															// 2001년 9월 6일 

		// 잔여 데이타가 있으면 제거, 없으면.. 통과? ^^;; 뭐 그런것..
		setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&li, sizeof(li));
		closesocket(m_Socket);								// 소켓 닫기 
		m_Socket = NULL;
	}

	// 할당 패킷 살태 검사후 패킷 할당 해제 
	if(NULL != lpOverlapPlus)
	{
		if(NULL != lpOverlapPlus->wbuf.buf && NULL != m_pPacketPool)
		{
			// 마지막으로 완료 되었던 오버랩구조체와 버퍼를 릴리즈한다.
			switch( lpOverlapPlus->nConnState)
			{
			case	ClientIoAccept:
					ReleaseAcptPacket( lpOverlapPlus );
					break;

			case	ClientIoRead:
					ReleaseRecvPacket( lpOverlapPlus );
					break;

			case	ClientIoProcess:
					ReleaseProcPacket( lpOverlapPlus );
					break;
			default:
					break;
			}
		}
	}

	InterlockedExchange((LONG*)&m_bIsConnect,0);

	// 총 연결 자 수 
	InterlockedDecrement((LONG*)&m_nConnCnt);
	if(m_nConnCnt < 0 )InterlockedExchange((LONG*)&m_nConnCnt, 0);


	printf("접속 해제 [INDEX:%d] <<Recv:%d Proc:%d Acpt:%d>>\n",(int)m_dwIndex,
																m_pPacketPool->m_nRecvCnt,
																m_pPacketPool->m_nProcCnt,
																m_pPacketPool->m_nAcptCnt+1);

	m_nRecvFlag = 0;
	m_nRecvTotalSize = 0;
	m_nRecvRead = 0;					// 현재까지 읽은 값 
	m_szRcvData[0] = NULL;

	// 이 패킷 다시 초기화 
	if(!this->Open())
	{
		// 실패하면 어떻게 해야 하는가? 이 컨넥션 클래스는 죽은 클래스가 된다.
		// 뭔가 죽은 상태를 표시해주고 체크하자. 그리고 나중에 타이머에서 다시 초기화 해본다.
		printf("Dead Connection.\n");
		Disconnect();
		return FALSE;
	}
	return TRUE;												// 정상 처리 
}


//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Server 종료시 이용 근데.. 여기선 안쓴다		//
// [2]PARAMETER : lpOverlapPlus - 할당 패킷 					//
//				  bForce - 소켓의 버퍼 상태에 따른 처리를 위한..//
// [3]RETURN : TRUE - 정상 처리 								//
// [4]DATE : 2000년 9월 5일										//
// [5]알림 : 위 함수와 동일										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::Shutdown( LPOVERLAPPEDPLUS lpOverlapPlus, BOOL bForce)
{
	if(m_Socket != NULL && m_sckListener != NULL)
	{
		struct linger li = {0, 0};								// Default: SO_DONTLINGER

		if( bForce ) li.l_onoff = 1;							// SO_LINGER, timeout = 0

		shutdown(m_Socket, SD_BOTH);
		setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&li, sizeof(li));
		closesocket(m_Socket);
		m_Socket = NULL;
	}

	if(NULL != lpOverlapPlus)
	{
		if(NULL != lpOverlapPlus->wbuf.buf && NULL != m_pPacketPool)
		{
			// 마지막으로 완료 되었던 오버랩구조체와 버퍼를 릴리즈한다.
			switch( lpOverlapPlus->nConnState)
			{
			case	ClientIoAccept:
					ReleaseAcptPacket( lpOverlapPlus );
					break;

			case	ClientIoRead:
					ReleaseRecvPacket( lpOverlapPlus );
					break;

			case	ClientIoProcess:
					ReleaseProcPacket( lpOverlapPlus );
					break;
			default:
					break;
			}
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : IOCP 처리 핸들링 							//
// [2]PARAMETER : lpOverlapPlus - 할당 패킷 					//
// [3]RETURN : TRUE - 정상 처리 								//
// [4]DATE : 2000년 9월 5일										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::DoIo( LPOVERLAPPEDPLUS lpOverlapPlus )
{
	// 할당 패킷의 상태 확인 
	switch(lpOverlapPlus->nConnState)
	{
	// ACCEPT관련 처리
	case ClientIoAccept:

			BindIOCP(lpOverlapPlus);							// 현재 소켓과 IOCP 바인딩 처리 

			InterlockedIncrement((LONG*)&m_bIsConnect);			// 접속 상태 변수 ON !!!
			InterlockedIncrement((LONG*)&m_nConnCnt);			// 총 접속자 수 1 증가

			// 검사된 연결자 아난 경우 연결 해제 
			if(lpOverlapPlus->dwBytes && lpOverlapPlus->wbuf.buf[0] == 'i' && lpOverlapPlus->wbuf.buf[1] == 'C' && lpOverlapPlus->wbuf.buf[2] == '8') printf("접속 성공 [INDEX:%d]\n",(int)m_dwIndex);
			else
			{
				Close_Open(lpOverlapPlus);
				break;
			}

			// Accept 할당 패킷 해제 
			if(!RecvPost())
			{
				Close_Open(lpOverlapPlus);
				break;
			}

			ReleaseAcptPacket( lpOverlapPlus );

			// TICK 카운트 설정 
			InterlockedExchange((LONG*)&m_uRecvTickCnt,GetTickCount());
			break;

	// RECEIVE 관련 처리 
	case ClientIoRead:
			// TICK 카운트 설정 
			InterlockedExchange((LONG*)&m_uRecvTickCnt,GetTickCount());

			// 처리 데이타가 없다면 
			if(lpOverlapPlus->dwBytes == 0)
			{
				this->Close_Open(lpOverlapPlus, FALSE);			// 에러, 객체 다시 초기화 
			}
			else// 에러라면 
			if((int)lpOverlapPlus->dwBytes == SOCKET_ERROR)
			{
				this->Close_Open(lpOverlapPlus, TRUE);			// 에러, 객체 다시 초기화 
			}
			else// 정상이라면 
			{
				// 메세지 저장 
				if(!DispatchPacket(lpOverlapPlus))
				{
					printf( "READ: failed to dispatch packet.\n" );
					this->Close_Open( lpOverlapPlus, TRUE );
				}
				else
				{
					// Receive 할당 패킷 해제 
					ReleaseRecvPacket( lpOverlapPlus );
				}
			}
			break;

	case ClientIoProcess:
			if(lpOverlapPlus->wbuf.len == 0) this->Disconnect();
			else				 			 ProcessReceivedData(lpOverlapPlus);
			ReleaseProcPacket(lpOverlapPlus);
			break;

	default:assert(0);
			break;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Accept패킷 할당 							//
// [2]PARAMETER : void											//
// [3]RETURN : LPOVERLAPPEDPLUS - overlappedplus구조체 반환		//
// [4]DATE : 2000년 9월 5일										//
//////////////////////////////////////////////////////////////////
LPOVERLAPPEDPLUS CConnection::PrepareAcptPacket()
{
	LPOVERLAPPEDPLUS newolp = NULL;

	// get accept overlapped structure and packet buffer.
	if(m_pPacketPool->AllocAcptPacket(newolp) == FALSE)
	{
		printf( "%08X, acpt packet alloc failed\n", m_dwIndex );
		return NULL;
	}

	// 치명적인 에러 
	if(!newolp)
	{
		LogError("CConnection::PrepareAcptPacket()에서 에러발생");
		return NULL;
	}

	// clear buffer
	memset(&newolp->overlapped	, NULL, sizeof(OVERLAPPED));
	memset(&newolp->wbuf.buf[0], NULL, sizeof(m_nMaxPacketSize));

	// init olp structure
	newolp->sckListen	= m_sckListener;
	newolp->sckClient	= m_Socket;
	newolp->nConnState	= ClientIoAccept;
	newolp->pClientConn = (PVOID)this;
	newolp->wbuf.len	= CONNECT_CHECK_SIZE;					//	newolp->wbuf.len	= MAXPACKETSIZE;
																// ** WARNING ** 
																// When you change your packet certfying correct connection,
																// you must change the size of definition 'CONNECT_CHECK_SIZE'.
	newolp->ConnType	= m_ConnType;

	return newolp;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Receive 패킷 할당 							//
// [2]PARAMETER : psrcbuf - 데이타 포인터 srclen - 크기			//
// [3]RETURN : LPOVERLAPPEDPLUS - overlappedplus구조체 반환		//
// [4]DATE : 2000년 9월 5일										//
//////////////////////////////////////////////////////////////////
LPOVERLAPPEDPLUS CConnection::PrepareRecvPacket(UINT srclen)
{
	// 치명적인 에러
	if(srclen > (UINT)m_nMaxPacketSize)
	{
		LogError("CConnection::PrepareRecvPacket에서 길이가 MAXPACKETSIZE보다 긴 데이타 들어옴");
		return NULL;
	}

	LPOVERLAPPEDPLUS newolp = NULL;

	// get recv overlapped structure and packet buffer.
	if( FALSE == m_pPacketPool->AllocRecvPacket(newolp) )
	{
		printf("%08X, recv packet alloc failed\n", m_dwIndex);
		return NULL;
	}

	// 치명적인 에러
	if(!newolp)
	{
		LogError("CConnection::PrepareRecvPacket에서 newolp가 NULL값으로 입력");
		return NULL;
	}

	// clear buffer
	memset(&newolp->overlapped	, NULL, sizeof(OVERLAPPED));
	memset(&newolp->wbuf.buf[0], NULL, sizeof(m_nMaxPacketSize));

	// init olp structure
	newolp->sckListen	= m_sckListener;
	newolp->sckClient	= m_Socket;
	newolp->nConnState	= ClientIoRead;
	newolp->pClientConn = (PVOID) this;
	newolp->ConnType	= m_ConnType;

	if(srclen == 0)	newolp->wbuf.len	= m_nMaxPacketSize; 
	else			newolp->wbuf.len	= srclen; 
	
	return newolp;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Proc 패킷 할당 								//
// [2]PARAMETER : psrcbuf - 데이타 포인터 srclen - 크기			//
// [3]RETURN : LPOVERLAPPEDPLUS - overlappedplus구조체 반환		//
// [4]DATE : 2000년 9월 5일										//
//////////////////////////////////////////////////////////////////
LPOVERLAPPEDPLUS CConnection::PrepareProcPacket(char *psrcbuf, UINT srclen)
{
	// 치명적인 에러 발생 
	if(srclen < 0 || srclen > (UINT)m_nMaxPacketSize)
	{
		LogError("CConnection::PrepareProcPacket()에서 데이타 길이 오류");
		return NULL;
	}

	LPOVERLAPPEDPLUS newolp = NULL;

	// get send overlapped structure and packet buffer.
	if(!m_pPacketPool->AllocProcPacket(newolp))
	{
		printf("%08X, send packet alloc failed\n", m_dwIndex);
		return NULL;
	}

	// 치명적인 에러 발생 
	if(!newolp)
	{
		LogError("CConnection::PrepareProcPacket()에서 newolp가 NULL포인터로 생성");
		return NULL;
	}
	
	// clear buffer
	memset(&newolp->overlapped	, NULL, sizeof(OVERLAPPED));
	memset(&newolp->wbuf.buf[0], NULL, sizeof(m_nMaxPacketSize));

	// init olp structure
	newolp->sckListen	= m_sckListener;
	newolp->sckClient	= m_Socket;
	newolp->nConnState	= ClientIoProcess;
	newolp->pClientConn	= (PVOID) this;
	newolp->wbuf.len	= srclen; // srclen
	newolp->ConnType	= m_ConnType;

	memcpy(newolp->wbuf.buf,psrcbuf,srclen);

	return newolp;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Accept 패킷 해제  							//
// [2]PARAMETER : LPOVERLAPPEDPLUS - 해제할 overlappedplus구조체//
// [3]RETURN : TRUE - 정상 처리									//
// [4]DATE : 2000년 9월 5일										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::ReleaseAcptPacket(LPOVERLAPPEDPLUS olp)
{
	if(NULL == olp)				return FALSE;
	if(NULL == olp->wbuf.buf)	return FALSE;

	if(!m_pPacketPool->FreeAcptPacket(olp))
	{
		printf( "%08X, free acpt packet failed\n", m_dwIndex );
		return FALSE;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Proc 패킷 해제  							//
// [2]PARAMETER : LPOVERLAPPEDPLUS - 해제할 overlappedplus구조체//
// [3]RETURN : TRUE - 정상 처리									//
// [4]DATE : 2000년 9월 5일										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::ReleaseProcPacket(LPOVERLAPPEDPLUS olp)
{
	if(NULL == olp)				return FALSE;
	if(NULL == olp->wbuf.buf)	return FALSE;

	if(!m_pPacketPool->FreeProcPacket(olp))
	{
		printf( "%08X, free Proc packet failed\n", m_dwIndex );
		return FALSE;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Receive 패킷 해제  							//
// [2]PARAMETER : LPOVERLAPPEDPLUS - 해제할 overlappedplus구조체//
// [3]RETURN : TRUE - 정상 처리									//
// [4]DATE : 2000년 9월 5일										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::ReleaseRecvPacket(LPOVERLAPPEDPLUS olp)
{
	if(olp == NULL)				return FALSE;
	if(olp->wbuf.buf == NULL)	return FALSE;

	if(!m_pPacketPool->FreeRecvPacket(olp))
	{
		printf("%08X, free recv packet failed\n", m_dwIndex);
		return FALSE;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Socket과 IOCP 바인딩 작업 					//
// [2]PARAMETER : LPOVERLAPPEDPLUS - overlappedplus구조체		//
// [3]RETURN : TRUE - 정상 처리									//
// [4]DATE : 2000년 9월 5일										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::BindIOCP(LPOVERLAPPEDPLUS lpOverlapPlus)
{
	// 치명적인 에러
	if(!lpOverlapPlus)
	{
		LogError("CConnection::BindIOCP()에서 lpOverlapPlus가 NULL 포인터로 생성");
		return FALSE;
	}

	int	locallen, remotelen;

	sockaddr_in *	plocal	= 0,
				*	premote	= 0;

	GetAcceptExSockaddrs(
		&(lpOverlapPlus->wbuf.buf[0]),
		m_nMaxBuf,
		sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16,
		(sockaddr **)&plocal,									// 서버단 
		&locallen,
		(sockaddr **)&premote,									// 로컬단 
		&remotelen
	);

	memcpy(&m_Local, plocal, sizeof(sockaddr_in));
	memcpy(&m_Peer, premote, sizeof(sockaddr_in));

	if(CreateIoCompletionPort((HANDLE)lpOverlapPlus->sckClient,m_hIOCP,0,0) == 0) return FALSE;
	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 메세지 저장 및 Receive 신호 발생, Socket정리//
// [2]PARAMETER : LPOVERLAPPEDPLUS - 해제할 overlappedplus구조체//
// [3]RETURN : TRUE - 정상 처리									//
// [4]DATE : 2000년 9월 5일										//
//////////////////////////////////////////////////////////////////

BOOL CConnection::DispatchPacket(LPOVERLAPPEDPLUS lpOverlapPlus)
{
	char * psrcbuf     = &( lpOverlapPlus->wbuf.buf[0] );
	int     srclen     =    lpOverlapPlus->dwBytes;

	switch(m_nRecvFlag)
	{
	// Normal State
	case 0:	if(srclen < sizeof(int))
			{
				assert(0);
				return FALSE;
			}
			CopyMemory(&m_nRecvTotalSize,psrcbuf,sizeof(int));

			if((int)(m_nRecvTotalSize + sizeof(int)) == srclen)
			{
				if(psrcbuf[4] != 'c' || psrcbuf[5] != 'h' || psrcbuf[6] != 'k') ProcessReceivedData(psrcbuf,srclen);
			}
			else if((int)(m_nRecvTotalSize + sizeof(int)) > srclen)
			{
				// 실제 들어온 길이가 계산값보다 작을 때,
				m_nRecvRead =  srclen - sizeof(int);
				assert(m_nRecvRead);

				CopyMemory(m_szRcvData,psrcbuf,srclen);
				m_nRecvFlag = 1;
				return RecvPost(m_nRecvTotalSize - m_nRecvRead);		// m_nRecvTotalSize - m_nRecvRead 값만큼 버퍼 할당
			}
			else
			{
				// 실제 들어온 길이가 계산값보다 클 때,
				while(srclen)
				{
					CopyMemory(m_szRcvData,psrcbuf,m_nRecvTotalSize + sizeof(int));
					m_szRcvData[m_nRecvTotalSize + sizeof(int)] = NULL;
					if(psrcbuf[4] != 'c' || psrcbuf[5] != 'h' || psrcbuf[6] != 'k') ProcessReceivedData(m_szRcvData,m_nRecvTotalSize + sizeof(int));

					srclen -= (m_nRecvTotalSize + sizeof(int));
					if(srclen == 0)
					{
						m_nRecvFlag = 0;
						break;
					}
					MoveMemory(&psrcbuf[0],&psrcbuf[m_nRecvTotalSize + sizeof(int)],srclen);

					if(srclen >= sizeof(int))
					{
						CopyMemory(&m_nRecvTotalSize,psrcbuf,sizeof(int));
						if((m_nRecvTotalSize + (int)sizeof(int)) > srclen)
						{
							m_nRecvRead = srclen - sizeof(int);
							m_nRecvFlag = 1;
							return RecvPost(m_nRecvTotalSize - m_nRecvRead);	// m_nRecvTotalSize - m_nRecvRead 값만큼 버퍼 할당
						}
					}
					else
					{
						m_nRecvRead = srclen;
						m_nRecvFlag = 2;
						return RecvPost(sizeof(int) - m_nRecvRead);				// sizeof(int) - m_nRecvRead 값만큼 버퍼 할당
					}
				}
			}
			break;

	// 원래길이 만큼 못 받는 데이타 처리 
	case 1: CopyMemory(&m_szRcvData[m_nRecvRead+sizeof(int)],psrcbuf,srclen);
			m_nRecvRead += srclen;

			if(m_nRecvTotalSize == m_nRecvRead)
			{
				if(psrcbuf[4] != 'c' || psrcbuf[5] != 'h' || psrcbuf[6] != 'k') ProcessReceivedData(m_szRcvData, m_nRecvRead + sizeof(int));
				m_nRecvFlag = 0;
			}
			else
			{
				return RecvPost(m_nRecvTotalSize - m_nRecvRead);	// m_nRecvTotalSize - m_nRecvRead 값만큼 버퍼 할당
			} 		
			break;

	// sizeof(int)의 크기를 만드는 처리
	case 2: CopyMemory(&m_szRcvData[m_nRecvRead],psrcbuf,srclen);
			if((m_nRecvRead + srclen) == (int)sizeof(int))
			{
				CopyMemory(&m_nRecvTotalSize,m_szRcvData,sizeof(int));
				m_nRecvRead = 0;
				m_nRecvFlag = 1;
				return RecvPost(m_nRecvTotalSize);	// m_nRecvTotalSize - m_nRecvRead 값만큼 버퍼 할당
			}
			else return FALSE;
			break;

	default:break;
	}

	// 그리고 새로운 recieve buffer를 준비하여 Post한다.
	return RecvPost();
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 외부에서 SendPost함수를 사용할수 있게..		//
// [2]PARAMETER : psrcbuf - 데이타 포인터 srclen - 크기			//
// [3]RETURN : TRUE - 정상 처리									//
// [4]DATE : 2000년 9월 5일										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::DispatchPacket(char * psrcbuf,int srclen)
{
	return SendPost(psrcbuf,srclen);							// 메세지 POSTING
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 데이타 전송 								//
// [2]PARAMETER : psrcbuf - 데이타 포인터 srclen - 크기			//
// [3]RETURN : TRUE - 정상 처리									//
// [4]DATE : 2000년 9월 5일										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::SendPost( char *pbuf, UINT buflen )
{
	if(!m_Socket) return FALSE;
	if(!m_bIsConnect) return FALSE;

	WSABUF	wsabuf;
	DWORD	dwRet;

	EnterCriticalSection(&m_CS);

	// 수신 TICK에 의한 접속 확인 
	dwRet = GetTickCount();
	dwRet -= m_uRecvTickCnt;

	// 클라이언트의 Setting에 주의 해서 만들어야 함
	// 잘못된 값 설정시 무고한(?) 클라이언트 무자비하게 절단 !!!!!!
	if(dwRet > TIMELIMIT_SEND && m_uRecvTickCnt != 0) 
	{
		Disconnect();
		LeaveCriticalSection(&m_CS);
		return FALSE;
	}

	wsabuf.buf	= pbuf;
	wsabuf.len	= buflen;
	dwRet		= buflen;

	int ret = WSASend(m_Socket,&wsabuf,1,&dwRet,0,NULL,NULL);

	if(ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		printf("%d, WSASend(): SOCKET_ERROR, %d\n",(int)m_dwIndex, WSAGetLastError());
		Disconnect();
		LeaveCriticalSection(&m_CS);
		return FALSE;
	}

	LeaveCriticalSection(&m_CS);
	return TRUE;	
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : IOCP로 부터 데이타 입력						//
// [2]PARAMETER : psrcbuf - 데이타 포인터 srclen - 크기			//
// [3]RETURN : TRUE - 정상 처리									//
// [4]DATE : 2000년 9월 5일										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::RecvPost(UINT buflen)
{
	if(m_Socket == NULL || m_bIsConnect == FALSE) return FALSE;

	// prepare recieve buffer
	LPOVERLAPPEDPLUS newolp = PrepareRecvPacket(buflen);

	// 제대로 할당 받았는지 조사
	if(newolp == NULL)	return FALSE;

	int ret = WSARecv(	newolp->sckClient,
						&newolp->wbuf,
						1,
						&newolp->dwBytes,						// 만약 호출했을때 바로 받았다면 여기로 받은 크기가 넘어오지만 iocp에서는 의미가 없다.
						&newolp->dwFlags,
						&newolp->overlapped,					// Overlapped 구조체 
						NULL );
	
	// 에러 처리 
	if(ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		printf("%08X, WSARecv(): SOCKET_ERROR, %d\n",	m_dwIndex, WSAGetLastError());
		ReleaseRecvPacket(newolp);
		return FALSE;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 받은 데이타를 처리하는 함수					//
// [2]PARAMETER : lpOverlapPlus - 수신 데이타 구조체			//
// [3]RETURN : TRUE - 정상 처리									//
// [4]DATE : 2002년 4월 2일										//
//////////////////////////////////////////////////////////////////
void CConnection::ProcessReceivedData(LPOVERLAPPEDPLUS lpOverlapPlus)
{
	if(!m_bIsConnect) return;

	char * psrcbuf     = &(lpOverlapPlus->wbuf.buf[0]);
	int     srclen     = lpOverlapPlus->dwBytes;

	LogError(&psrcbuf[4]);

	CConnection*	pConnectionArray	= (CConnection*)this - (int)m_dwIndex;

	for(int i = 0 ; i < m_nMaxUser; i++)
	{
		if(pConnectionArray[i].m_bIsConnect) 
			pConnectionArray[i].SendPost(psrcbuf, srclen);
		Sleep(0);
	}
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 받은 데이타를 처리하는 함수					//
// [2]PARAMETER : psrcbuf - 문자열, srclen - 문자열 길이		//
// [3]RETURN : TRUE - 정상 처리									//
// [4]DATE : 2002년 4월 2일										//
//////////////////////////////////////////////////////////////////
void CConnection::ProcessReceivedData(char *psrcbuf, int srclen)
{
	if(!m_bIsConnect) return;

	CConnection*	pConnectionArray	= (CConnection*)this - (int)m_dwIndex;

	psrcbuf[srclen] = NULL;

//	LogError(&psrcbuf[4]);

	for(int i = 0 ; i < m_nMaxUser; i++)
	{
		if(pConnectionArray[i].m_bIsConnect) 
			pConnectionArray[i].SendPost(psrcbuf, srclen);
		Sleep(0);
	}
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 연결 클라이언트 IP 얻기 					//
// [2]PARAMETER : szIP - 반환할 포인터							//
// [3]RETURN : TRUE - 정상 처리									//
// [4]DATE : 2002년 4월 2일										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::GetClientIP(char *szIP)
{
	if(szIP == NULL) return FALSE;

	if(!m_bIsConnect)
	{
		szIP[0] = NULL;
		return FALSE;
	}

	CopyMemory(szIP,(char*)&m_Peer.sin_addr.S_un.S_addr,IP_LENGTH+1);

	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 연결 클라이언트 IP 얻기 					//
// [2]PARAMETER : addr - 반환할 addr 구조체 					//
// [3]RETURN : TRUE - 정상 처리									//
// [4]DATE : 2002년 4월 2일										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::GetClientIP(sockaddr_in& addr)
{
	if(!m_bIsConnect)
	{
		memcpy(&addr,0,sizeof(sockaddr_in));
		return FALSE;
	}

	CopyMemory((char*)&addr,(char*)&m_Peer,sizeof(sockaddr_in));

	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 서버에서 연결 강제 종료  					//
// [2]PARAMETER : addr - 반환할 addr 구조체 					//
// [3]RETURN : TRUE - 정상 처리									//
// [4]DATE : 2002년 4월 2일										//
//////////////////////////////////////////////////////////////////
void CConnection::Disconnect()
{
	if(!m_bIsConnect) return;

	// 소켓과 리스너 상태 확인 
	if(m_Socket)
	{
		struct linger li = {1, 0};								// Default: SO_DONTLINGER
		shutdown(m_Socket, SD_BOTH );						

		// 잔여 데이타가 있으면 제거
		setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&li, sizeof(li));
		closesocket(m_Socket);									// 소켓 닫기 
		m_Socket = NULL;										// 초기화 
	}
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 연결 관리 객체의 Index값   					//
// [2]PARAMETER :												//
// [3]RETURN : TRUE - 정상 처리									//
// [4]DATE : 2002년 4월 2일										//
//////////////////////////////////////////////////////////////////
DWORD CConnection::GetIndex()
{
	return m_dwIndex;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 연결이 되어 있는지 검사   					//
// [2]PARAMETER :												//
// [3]RETURN : TRUE - 정상 처리									//
// [4]DATE : 2002년 4월 2일										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::IsConnect()
{
	return m_bIsConnect;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 수신했을 때의 TICK값	  					//
// [2]PARAMETER :												//
// [3]RETURN : TRUE - 정상 처리									//
// [4]DATE : 2002년 4월 2일										//
//////////////////////////////////////////////////////////////////
long CConnection::GetRecvTickCnt()
{
	return m_uRecvTickCnt;
}	

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 전체 연결된 클라이언트 수  					//
// [2]PARAMETER :												//
// [3]RETURN : TRUE - 정상 처리									//
// [4]DATE : 2002년 4월 2일										//
//////////////////////////////////////////////////////////////////
int CConnection::GetTotalConectionCnt()
{
	return  m_nConnCnt;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 최대 연결 가능 한 수	  					//
// [2]PARAMETER :												//
// [3]RETURN : TRUE - 정상 처리									//
// [4]DATE : 2002년 5월 9일										//
//////////////////////////////////////////////////////////////////
int CConnection::GetMaxUser()
{
	return m_nMaxUser;
}
