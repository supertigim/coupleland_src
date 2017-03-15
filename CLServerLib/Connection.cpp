// Connection.cpp: implementation of the CConnection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Connection.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Static ���� �ʱ�ȭ 
int CConnection::m_nConnCnt = 0;
//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ������										//
// [2]PARAMETER : void											//
// [3]RETURN : void												//
// [4]DATE : 2000�� 9�� 5��										//
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
	m_nRecvRead;					// ������� ���� �� 
	m_szRcvData[0] = NULL;


	InitializeCriticalSectionAndSpinCount( &m_CS, 4000 );   // why 4000?
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : �Ҹ��� 										//
// [2]PARAMETER : void											//
// [3]RETURN : void												//
// [4]DATE : 2000�� 9�� 5��										//
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

	DeleteCriticalSection( &m_CS);								// ũ��ƼĮ ���� ���� ���� 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ��� ���� ���� �� ��ü �ʱ�ȭ				//
// [2]PARAMETER : dwIndex - ��ü ���� ID						//
//				  hIOCP - IOCP Handle							//
//				  listener - Listen Socket						//
//				  pPacketPool - ��Ŷ Pool ������				//
//				  pMsg - BroadCasting�� ���� �޼��� ó�� ������ //
//				  MsgWait - �޼��� Send�� ���� �̺�Ʈ �ڵ�		// 
// [3]RETURN : TRUE - ���� ó��, FALSE - ���� 					//
// [4]DATE : 2000�� 9�� 4��										//
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
	// ��ü�� ��� ���� ���� 
	m_dwIndex			= dwIndex;
	m_hIOCP				= hIOCP;
	m_sckListener		= listener;
	m_pPacketPool		= pPacketPool;

	m_nMaxBuf			= m_nMaxPacketSize		=	nMaxBuf;
	m_nCheckPakcetSize	= nCheckPacketSize;
	m_nMaxUser			= nMaxUser;

	// ��ü OPEN
	if(!this->Open())
	{
		// �����ϸ� ��� �ؾ� �ϴ°�? �� ���ؼ� Ŭ������ ���� Ŭ������ �ȴ�.
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ��ü �ʱ�ȭ, ���� ���� ����					//
// [2]PARAMETER : void											//
// [3]RETURN : TRUE - ����ó�� , FALSE - ���� 					//
// [4]DATE : 2000�� 9�� 4��										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::Open()
{
	// ���� �� �־�� ��.
	assert(m_nMaxUser);
	assert(m_nMaxBuf);
	assert(m_nMaxPacketSize);

	// ��Ŷ Pool�� ����� ���� �Ǿ����� �˻� 
	if( !m_pPacketPool )
		return FALSE;

	// Listener Socket ���� �˻� 
	if( m_sckListener == NULL )
		return FALSE;

 	// create socket for send/recv
	m_Socket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_IP,NULL,0,WSA_FLAG_OVERLAPPED );

	// ���� ������ ����� ���� �Ǿ����� �˻� 
	if(m_Socket == NULL) return FALSE;

	// Accpet�� ����������ü�� ��Ŷ���۸� �غ��Ѵ�.
	LPOVERLAPPEDPLUS newolp = PrepareAcptPacket();

	if(NULL == newolp) return FALSE;

	/////////////////////////////////////////////////////////////////////
	// Socket�� Listener�� ����
	// Overlapped�� ���� ������ ���߿� IOCP �̺�Ʈ �߻� ó���� ���δ� 
	BOOL bRet = AcceptEx(newolp->sckListen,						// Listen Socket
						 newolp->sckClient,						// Socket
						 &(newolp->wbuf.buf[0]),				// ���� ������ 
						 m_nMaxBuf,								// ���� ������ 
						 sizeof(sockaddr_in) + 16,				// ���� ���� - IP, Address, Name.. etc
						 sizeof(sockaddr_in) + 16,				// ���� ���� - IP, Address, Name.. etc
						 &newolp->dwBytes,						// ó�� ����Ʈ ũ�� 
						 &newolp->overlapped);					// *�߿�*

	newolp->ConnType	= m_ConnType;

	// ���� ó�� 
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
	// ������ ���� ����ȭ�� ���� ���� 
	int zero = 0;
	int err = 0;

	// Send Buffer�� ���� ����
	if( (err = setsockopt( m_Socket, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero))) == SOCKET_ERROR)
	{
	    closesocket(m_Socket);
		m_Socket = NULL;
		return FALSE;
    }

	// Receive Buffer�� ���� ���� 
	if((err = setsockopt( m_Socket, SOL_SOCKET, SO_RCVBUF, (char *)&zero, sizeof(zero) )) == SOCKET_ERROR)
	{
		closesocket(m_Socket);
		m_Socket = NULL;
		return FALSE;
    }
	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ��ü�� Ŭ���̾�Ʈ ���� ���� �� �ʱ�ȭ 		//
// [2]PARAMETER : lpOverlapPlus - �Ҵ� ��Ŷ 					//
//				  bForce - ������ ���� ���¿� ���� ó���� ����..//
// [3]RETURN : TRUE - ���� ó�� 								//
// [4]DATE : 2000�� 9�� 5��										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::Close_Open( LPOVERLAPPEDPLUS lpOverlapPlus, BOOL bForce )
{
	// ���ϰ� ������ ���� Ȯ�� 
	if(m_Socket)
	{
		struct linger li = {0, 0};	// Default: SO_DONTLINGER

		// ���Ͽ� ó�� ���� ���� ����Ÿ�� ���� ���ۿ� �����ִٸ�,
		if(bForce) li.l_onoff = 1; // SO_LINGER, timeout = 0

		shutdown(m_Socket, SD_BOTH );						// ����? �̰� ����? ^^;; �㿡 ã�ƺ��� 
															// 2001�� 9�� 6�� 

		// �ܿ� ����Ÿ�� ������ ����, ������.. ���? ^^;; �� �׷���..
		setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&li, sizeof(li));
		closesocket(m_Socket);								// ���� �ݱ� 
		m_Socket = NULL;
	}

	// �Ҵ� ��Ŷ ���� �˻��� ��Ŷ �Ҵ� ���� 
	if(NULL != lpOverlapPlus)
	{
		if(NULL != lpOverlapPlus->wbuf.buf && NULL != m_pPacketPool)
		{
			// ���������� �Ϸ� �Ǿ��� ����������ü�� ���۸� �������Ѵ�.
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

	// �� ���� �� �� 
	InterlockedDecrement((LONG*)&m_nConnCnt);
	if(m_nConnCnt < 0 )InterlockedExchange((LONG*)&m_nConnCnt, 0);


	printf("���� ���� [INDEX:%d] <<Recv:%d Proc:%d Acpt:%d>>\n",(int)m_dwIndex,
																m_pPacketPool->m_nRecvCnt,
																m_pPacketPool->m_nProcCnt,
																m_pPacketPool->m_nAcptCnt+1);

	m_nRecvFlag = 0;
	m_nRecvTotalSize = 0;
	m_nRecvRead = 0;					// ������� ���� �� 
	m_szRcvData[0] = NULL;

	// �� ��Ŷ �ٽ� �ʱ�ȭ 
	if(!this->Open())
	{
		// �����ϸ� ��� �ؾ� �ϴ°�? �� ���ؼ� Ŭ������ ���� Ŭ������ �ȴ�.
		// ���� ���� ���¸� ǥ�����ְ� üũ����. �׸��� ���߿� Ÿ�̸ӿ��� �ٽ� �ʱ�ȭ �غ���.
		printf("Dead Connection.\n");
		Disconnect();
		return FALSE;
	}
	return TRUE;												// ���� ó�� 
}


//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Server ����� �̿� �ٵ�.. ���⼱ �Ⱦ���		//
// [2]PARAMETER : lpOverlapPlus - �Ҵ� ��Ŷ 					//
//				  bForce - ������ ���� ���¿� ���� ó���� ����..//
// [3]RETURN : TRUE - ���� ó�� 								//
// [4]DATE : 2000�� 9�� 5��										//
// [5]�˸� : �� �Լ��� ����										//
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
			// ���������� �Ϸ� �Ǿ��� ����������ü�� ���۸� �������Ѵ�.
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
// [1]DESCRIPTION : IOCP ó�� �ڵ鸵 							//
// [2]PARAMETER : lpOverlapPlus - �Ҵ� ��Ŷ 					//
// [3]RETURN : TRUE - ���� ó�� 								//
// [4]DATE : 2000�� 9�� 5��										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::DoIo( LPOVERLAPPEDPLUS lpOverlapPlus )
{
	// �Ҵ� ��Ŷ�� ���� Ȯ�� 
	switch(lpOverlapPlus->nConnState)
	{
	// ACCEPT���� ó��
	case ClientIoAccept:

			BindIOCP(lpOverlapPlus);							// ���� ���ϰ� IOCP ���ε� ó�� 

			InterlockedIncrement((LONG*)&m_bIsConnect);			// ���� ���� ���� ON !!!
			InterlockedIncrement((LONG*)&m_nConnCnt);			// �� ������ �� 1 ����

			// �˻�� ������ �Ƴ� ��� ���� ���� 
			if(lpOverlapPlus->dwBytes && lpOverlapPlus->wbuf.buf[0] == 'i' && lpOverlapPlus->wbuf.buf[1] == 'C' && lpOverlapPlus->wbuf.buf[2] == '8') printf("���� ���� [INDEX:%d]\n",(int)m_dwIndex);
			else
			{
				Close_Open(lpOverlapPlus);
				break;
			}

			// Accept �Ҵ� ��Ŷ ���� 
			if(!RecvPost())
			{
				Close_Open(lpOverlapPlus);
				break;
			}

			ReleaseAcptPacket( lpOverlapPlus );

			// TICK ī��Ʈ ���� 
			InterlockedExchange((LONG*)&m_uRecvTickCnt,GetTickCount());
			break;

	// RECEIVE ���� ó�� 
	case ClientIoRead:
			// TICK ī��Ʈ ���� 
			InterlockedExchange((LONG*)&m_uRecvTickCnt,GetTickCount());

			// ó�� ����Ÿ�� ���ٸ� 
			if(lpOverlapPlus->dwBytes == 0)
			{
				this->Close_Open(lpOverlapPlus, FALSE);			// ����, ��ü �ٽ� �ʱ�ȭ 
			}
			else// ������� 
			if((int)lpOverlapPlus->dwBytes == SOCKET_ERROR)
			{
				this->Close_Open(lpOverlapPlus, TRUE);			// ����, ��ü �ٽ� �ʱ�ȭ 
			}
			else// �����̶�� 
			{
				// �޼��� ���� 
				if(!DispatchPacket(lpOverlapPlus))
				{
					printf( "READ: failed to dispatch packet.\n" );
					this->Close_Open( lpOverlapPlus, TRUE );
				}
				else
				{
					// Receive �Ҵ� ��Ŷ ���� 
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
// [1]DESCRIPTION : Accept��Ŷ �Ҵ� 							//
// [2]PARAMETER : void											//
// [3]RETURN : LPOVERLAPPEDPLUS - overlappedplus����ü ��ȯ		//
// [4]DATE : 2000�� 9�� 5��										//
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

	// ġ������ ���� 
	if(!newolp)
	{
		LogError("CConnection::PrepareAcptPacket()���� �����߻�");
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
// [1]DESCRIPTION : Receive ��Ŷ �Ҵ� 							//
// [2]PARAMETER : psrcbuf - ����Ÿ ������ srclen - ũ��			//
// [3]RETURN : LPOVERLAPPEDPLUS - overlappedplus����ü ��ȯ		//
// [4]DATE : 2000�� 9�� 5��										//
//////////////////////////////////////////////////////////////////
LPOVERLAPPEDPLUS CConnection::PrepareRecvPacket(UINT srclen)
{
	// ġ������ ����
	if(srclen > (UINT)m_nMaxPacketSize)
	{
		LogError("CConnection::PrepareRecvPacket���� ���̰� MAXPACKETSIZE���� �� ����Ÿ ����");
		return NULL;
	}

	LPOVERLAPPEDPLUS newolp = NULL;

	// get recv overlapped structure and packet buffer.
	if( FALSE == m_pPacketPool->AllocRecvPacket(newolp) )
	{
		printf("%08X, recv packet alloc failed\n", m_dwIndex);
		return NULL;
	}

	// ġ������ ����
	if(!newolp)
	{
		LogError("CConnection::PrepareRecvPacket���� newolp�� NULL������ �Է�");
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
// [1]DESCRIPTION : Proc ��Ŷ �Ҵ� 								//
// [2]PARAMETER : psrcbuf - ����Ÿ ������ srclen - ũ��			//
// [3]RETURN : LPOVERLAPPEDPLUS - overlappedplus����ü ��ȯ		//
// [4]DATE : 2000�� 9�� 5��										//
//////////////////////////////////////////////////////////////////
LPOVERLAPPEDPLUS CConnection::PrepareProcPacket(char *psrcbuf, UINT srclen)
{
	// ġ������ ���� �߻� 
	if(srclen < 0 || srclen > (UINT)m_nMaxPacketSize)
	{
		LogError("CConnection::PrepareProcPacket()���� ����Ÿ ���� ����");
		return NULL;
	}

	LPOVERLAPPEDPLUS newolp = NULL;

	// get send overlapped structure and packet buffer.
	if(!m_pPacketPool->AllocProcPacket(newolp))
	{
		printf("%08X, send packet alloc failed\n", m_dwIndex);
		return NULL;
	}

	// ġ������ ���� �߻� 
	if(!newolp)
	{
		LogError("CConnection::PrepareProcPacket()���� newolp�� NULL�����ͷ� ����");
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
// [1]DESCRIPTION : Accept ��Ŷ ����  							//
// [2]PARAMETER : LPOVERLAPPEDPLUS - ������ overlappedplus����ü//
// [3]RETURN : TRUE - ���� ó��									//
// [4]DATE : 2000�� 9�� 5��										//
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
// [1]DESCRIPTION : Proc ��Ŷ ����  							//
// [2]PARAMETER : LPOVERLAPPEDPLUS - ������ overlappedplus����ü//
// [3]RETURN : TRUE - ���� ó��									//
// [4]DATE : 2000�� 9�� 5��										//
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
// [1]DESCRIPTION : Receive ��Ŷ ����  							//
// [2]PARAMETER : LPOVERLAPPEDPLUS - ������ overlappedplus����ü//
// [3]RETURN : TRUE - ���� ó��									//
// [4]DATE : 2000�� 9�� 5��										//
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
// [1]DESCRIPTION : Socket�� IOCP ���ε� �۾� 					//
// [2]PARAMETER : LPOVERLAPPEDPLUS - overlappedplus����ü		//
// [3]RETURN : TRUE - ���� ó��									//
// [4]DATE : 2000�� 9�� 5��										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::BindIOCP(LPOVERLAPPEDPLUS lpOverlapPlus)
{
	// ġ������ ����
	if(!lpOverlapPlus)
	{
		LogError("CConnection::BindIOCP()���� lpOverlapPlus�� NULL �����ͷ� ����");
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
		(sockaddr **)&plocal,									// ������ 
		&locallen,
		(sockaddr **)&premote,									// ���ô� 
		&remotelen
	);

	memcpy(&m_Local, plocal, sizeof(sockaddr_in));
	memcpy(&m_Peer, premote, sizeof(sockaddr_in));

	if(CreateIoCompletionPort((HANDLE)lpOverlapPlus->sckClient,m_hIOCP,0,0) == 0) return FALSE;
	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : �޼��� ���� �� Receive ��ȣ �߻�, Socket����//
// [2]PARAMETER : LPOVERLAPPEDPLUS - ������ overlappedplus����ü//
// [3]RETURN : TRUE - ���� ó��									//
// [4]DATE : 2000�� 9�� 5��										//
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
				// ���� ���� ���̰� ��갪���� ���� ��,
				m_nRecvRead =  srclen - sizeof(int);
				assert(m_nRecvRead);

				CopyMemory(m_szRcvData,psrcbuf,srclen);
				m_nRecvFlag = 1;
				return RecvPost(m_nRecvTotalSize - m_nRecvRead);		// m_nRecvTotalSize - m_nRecvRead ����ŭ ���� �Ҵ�
			}
			else
			{
				// ���� ���� ���̰� ��갪���� Ŭ ��,
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
							return RecvPost(m_nRecvTotalSize - m_nRecvRead);	// m_nRecvTotalSize - m_nRecvRead ����ŭ ���� �Ҵ�
						}
					}
					else
					{
						m_nRecvRead = srclen;
						m_nRecvFlag = 2;
						return RecvPost(sizeof(int) - m_nRecvRead);				// sizeof(int) - m_nRecvRead ����ŭ ���� �Ҵ�
					}
				}
			}
			break;

	// �������� ��ŭ �� �޴� ����Ÿ ó�� 
	case 1: CopyMemory(&m_szRcvData[m_nRecvRead+sizeof(int)],psrcbuf,srclen);
			m_nRecvRead += srclen;

			if(m_nRecvTotalSize == m_nRecvRead)
			{
				if(psrcbuf[4] != 'c' || psrcbuf[5] != 'h' || psrcbuf[6] != 'k') ProcessReceivedData(m_szRcvData, m_nRecvRead + sizeof(int));
				m_nRecvFlag = 0;
			}
			else
			{
				return RecvPost(m_nRecvTotalSize - m_nRecvRead);	// m_nRecvTotalSize - m_nRecvRead ����ŭ ���� �Ҵ�
			} 		
			break;

	// sizeof(int)�� ũ�⸦ ����� ó��
	case 2: CopyMemory(&m_szRcvData[m_nRecvRead],psrcbuf,srclen);
			if((m_nRecvRead + srclen) == (int)sizeof(int))
			{
				CopyMemory(&m_nRecvTotalSize,m_szRcvData,sizeof(int));
				m_nRecvRead = 0;
				m_nRecvFlag = 1;
				return RecvPost(m_nRecvTotalSize);	// m_nRecvTotalSize - m_nRecvRead ����ŭ ���� �Ҵ�
			}
			else return FALSE;
			break;

	default:break;
	}

	// �׸��� ���ο� recieve buffer�� �غ��Ͽ� Post�Ѵ�.
	return RecvPost();
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : �ܺο��� SendPost�Լ��� ����Ҽ� �ְ�..		//
// [2]PARAMETER : psrcbuf - ����Ÿ ������ srclen - ũ��			//
// [3]RETURN : TRUE - ���� ó��									//
// [4]DATE : 2000�� 9�� 5��										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::DispatchPacket(char * psrcbuf,int srclen)
{
	return SendPost(psrcbuf,srclen);							// �޼��� POSTING
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ����Ÿ ���� 								//
// [2]PARAMETER : psrcbuf - ����Ÿ ������ srclen - ũ��			//
// [3]RETURN : TRUE - ���� ó��									//
// [4]DATE : 2000�� 9�� 5��										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::SendPost( char *pbuf, UINT buflen )
{
	if(!m_Socket) return FALSE;
	if(!m_bIsConnect) return FALSE;

	WSABUF	wsabuf;
	DWORD	dwRet;

	EnterCriticalSection(&m_CS);

	// ���� TICK�� ���� ���� Ȯ�� 
	dwRet = GetTickCount();
	dwRet -= m_uRecvTickCnt;

	// Ŭ���̾�Ʈ�� Setting�� ���� �ؼ� ������ ��
	// �߸��� �� ������ ������(?) Ŭ���̾�Ʈ ���ں��ϰ� ���� !!!!!!
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
// [1]DESCRIPTION : IOCP�� ���� ����Ÿ �Է�						//
// [2]PARAMETER : psrcbuf - ����Ÿ ������ srclen - ũ��			//
// [3]RETURN : TRUE - ���� ó��									//
// [4]DATE : 2000�� 9�� 5��										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::RecvPost(UINT buflen)
{
	if(m_Socket == NULL || m_bIsConnect == FALSE) return FALSE;

	// prepare recieve buffer
	LPOVERLAPPEDPLUS newolp = PrepareRecvPacket(buflen);

	// ����� �Ҵ� �޾Ҵ��� ����
	if(newolp == NULL)	return FALSE;

	int ret = WSARecv(	newolp->sckClient,
						&newolp->wbuf,
						1,
						&newolp->dwBytes,						// ���� ȣ�������� �ٷ� �޾Ҵٸ� ����� ���� ũ�Ⱑ �Ѿ������ iocp������ �ǹ̰� ����.
						&newolp->dwFlags,
						&newolp->overlapped,					// Overlapped ����ü 
						NULL );
	
	// ���� ó�� 
	if(ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		printf("%08X, WSARecv(): SOCKET_ERROR, %d\n",	m_dwIndex, WSAGetLastError());
		ReleaseRecvPacket(newolp);
		return FALSE;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ���� ����Ÿ�� ó���ϴ� �Լ�					//
// [2]PARAMETER : lpOverlapPlus - ���� ����Ÿ ����ü			//
// [3]RETURN : TRUE - ���� ó��									//
// [4]DATE : 2002�� 4�� 2��										//
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
// [1]DESCRIPTION : ���� ����Ÿ�� ó���ϴ� �Լ�					//
// [2]PARAMETER : psrcbuf - ���ڿ�, srclen - ���ڿ� ����		//
// [3]RETURN : TRUE - ���� ó��									//
// [4]DATE : 2002�� 4�� 2��										//
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
// [1]DESCRIPTION : ���� Ŭ���̾�Ʈ IP ��� 					//
// [2]PARAMETER : szIP - ��ȯ�� ������							//
// [3]RETURN : TRUE - ���� ó��									//
// [4]DATE : 2002�� 4�� 2��										//
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
// [1]DESCRIPTION : ���� Ŭ���̾�Ʈ IP ��� 					//
// [2]PARAMETER : addr - ��ȯ�� addr ����ü 					//
// [3]RETURN : TRUE - ���� ó��									//
// [4]DATE : 2002�� 4�� 2��										//
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
// [1]DESCRIPTION : �������� ���� ���� ����  					//
// [2]PARAMETER : addr - ��ȯ�� addr ����ü 					//
// [3]RETURN : TRUE - ���� ó��									//
// [4]DATE : 2002�� 4�� 2��										//
//////////////////////////////////////////////////////////////////
void CConnection::Disconnect()
{
	if(!m_bIsConnect) return;

	// ���ϰ� ������ ���� Ȯ�� 
	if(m_Socket)
	{
		struct linger li = {1, 0};								// Default: SO_DONTLINGER
		shutdown(m_Socket, SD_BOTH );						

		// �ܿ� ����Ÿ�� ������ ����
		setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&li, sizeof(li));
		closesocket(m_Socket);									// ���� �ݱ� 
		m_Socket = NULL;										// �ʱ�ȭ 
	}
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ���� ���� ��ü�� Index��   					//
// [2]PARAMETER :												//
// [3]RETURN : TRUE - ���� ó��									//
// [4]DATE : 2002�� 4�� 2��										//
//////////////////////////////////////////////////////////////////
DWORD CConnection::GetIndex()
{
	return m_dwIndex;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ������ �Ǿ� �ִ��� �˻�   					//
// [2]PARAMETER :												//
// [3]RETURN : TRUE - ���� ó��									//
// [4]DATE : 2002�� 4�� 2��										//
//////////////////////////////////////////////////////////////////
BOOL CConnection::IsConnect()
{
	return m_bIsConnect;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : �������� ���� TICK��	  					//
// [2]PARAMETER :												//
// [3]RETURN : TRUE - ���� ó��									//
// [4]DATE : 2002�� 4�� 2��										//
//////////////////////////////////////////////////////////////////
long CConnection::GetRecvTickCnt()
{
	return m_uRecvTickCnt;
}	

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ��ü ����� Ŭ���̾�Ʈ ��  					//
// [2]PARAMETER :												//
// [3]RETURN : TRUE - ���� ó��									//
// [4]DATE : 2002�� 4�� 2��										//
//////////////////////////////////////////////////////////////////
int CConnection::GetTotalConectionCnt()
{
	return  m_nConnCnt;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : �ִ� ���� ���� �� ��	  					//
// [2]PARAMETER :												//
// [3]RETURN : TRUE - ���� ó��									//
// [4]DATE : 2002�� 5�� 9��										//
//////////////////////////////////////////////////////////////////
int CConnection::GetMaxUser()
{
	return m_nMaxUser;
}
