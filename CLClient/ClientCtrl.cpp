// ClientCtrl.cpp: implementation of the CClientCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ClientCtrl.h"

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 생성자							 			//
// [2]PARAMETER : void											//
// [3]RETURN :	void											//
// [4]DATE : 2000년 9월 10일									//
//////////////////////////////////////////////////////////////////
CClientCtrl::CClientCtrl()
{
	//////////////
	// 변수 초기화
	m_bConnect = FALSE;	
	m_hSocket = NULL;
	m_hEvent = NULL;
	m_hThread = NULL;

	//////////////////////////
	// 타이머 과련 
	m_uTimerID_Check = -1;
	m_uTimerPeriod = -1;

	m_uSendTickCnt = 0;

	InitializeCriticalSection(&m_criticalSection);				

	WinSockInit();
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 소멸자 							 			//
// [2]PARAMETER : void											//
// [3]RETURN :	void											//
// [4]DATE : 2000년 9월 11일									//
//////////////////////////////////////////////////////////////////
CClientCtrl::~CClientCtrl()
{
    Stop();														// 종료 함수 호출 

	if(m_hEvent) WSACloseEvent(m_hEvent);
	m_hEvent = NULL;
	if(m_hThread) CloseHandle(m_hThread);
	m_hThread = NULL;

	DeleteCriticalSection(&m_criticalSection);
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 클래스 초기화 					 			//
// [2]PARAMETER : strIPAddr - 연결IP주소, nPort - 포트번호,		//
//				  hWnd - 부모 윈도우 핸들						//
// [3]RETURN :	정상 - TRUE, 실패 - FALSE						//
// [4]DATE : 2000년 9월 11일									//
//////////////////////////////////////////////////////////////////
BOOL CClientCtrl::Init( int nPort, HWND hWnd, char* szIP )
{
	m_nPort = nPort;											// 포트 번호 
	UINT dwThreadId = 0;										// 스레드 생성을 위한 변수 
	m_hWnd = hWnd;												// 부모 핸들 
	m_Queue.Create(MAXQUEUESIZE);

	// 파일로 IP ADDRESS를 얻을수 없다면,
	if(!szIP)
	{
		if(!GetIPByFile())	return FALSE;						// 에러 반환 
	}
	else
	{
		m_strIPAddr = szIP;
	}

	m_hThread= (HANDLE)_beginthreadex(	NULL,					// Security
										0,						// Stack size - use default
										SocketThreadProc,		// Thread fn entry point
										(void*) this,	    
										0,						// Init flag
										&dwThreadId);			// Thread address
	
	return !(m_hThread == NULL);								// 스레드 생성 검사 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 시스템 정지 					 			//
// [2]PARAMETER : void											//
// [3]RETURN :	void											//
// [4]DATE : 2000년 9월 10일									//
//////////////////////////////////////////////////////////////////
void CClientCtrl::Stop()
{
	if(m_hSocket) closesocket(m_hSocket);
	m_hSocket = NULL;

	SetEvent(m_hEvent);											// 스레드 알림

	/////////////////////////////////
	// 타이머 죽이기
	if(m_uTimerID_Check != -1)	timeKillEvent(m_uTimerID_Check); 
	if(m_uTimerPeriod != -1)	timeEndPeriod(m_uTimerPeriod); 

	m_uTimerID_Check = -1;
	m_uTimerPeriod = -1;

	m_uSendTickCnt = 0;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Main Thread, 네트워크 이벤트 처리			//
// [2]PARAMETER : void											//
// [3]RETURN :	void											//
// [4]DATE : 2000년 9월 10일									//
//////////////////////////////////////////////////////////////////
unsigned CClientCtrl::SocketThreadProc(LPVOID lParam)
{
	// 클래스를 변수로 받음
	CClientCtrl* pThis = reinterpret_cast<CClientCtrl*>(lParam);

	WSANETWORKEVENTS events;										// 네트워크 이벤트 변수 

	BOOL bThreadRun = TRUE;											// 무한 루프 변수 
	int nRet;

	// 연결이 되었는지 검사 
	if(!pThis->Connect())
	{
		Sleep(100);													// Sleep...
		pThis->OnClose();	
		return 0;
	}

	// 스레드 무한 루프 
	while(bThreadRun)
	{
		DWORD dwRet;
		dwRet = WSAWaitForMultipleEvents(1,
										 &pThis->m_hEvent,
										 FALSE,
										 INFINITE,
										 FALSE);                                                              

		// Figure out what happened
		nRet = WSAEnumNetworkEvents(pThis->m_hSocket,
									pThis->m_hEvent,
									&events);
		
		// 소켓 에러라면,
		if(nRet == SOCKET_ERROR)
		{
			pThis->OnClose();
			break;
		}

		///////////////////
		// Handle events //
		bThreadRun = pThis->NetworkEventHanlder(events.lNetworkEvents);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 이벤트 처리 (On Write)						//
// [2]PARAMETER : void											//
// [3]RETURN :	false 반환										//
// [4]DATE : 2000년 9월 11일									//
//////////////////////////////////////////////////////////////////
BOOL CClientCtrl::OnWrite()
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 이벤트 처리 (On Close)						//
// [2]PARAMETER : void											//
// [3]RETURN :	false 반환										//
// [4]DATE : 2000년 9월 11일									//
//////////////////////////////////////////////////////////////////
BOOL CClientCtrl::OnClose()
{
	m_bConnect = FALSE;											// Connect 변수 변경 

	C8String str = "Client Disconnected";
	::PostMessage(m_hWnd, WM_STATUS_MSG, 0, (LPARAM) AllocBuffer(str.CopyString()));
	::PostMessage(m_hWnd, WN_DISCONNECT, 0, (LPARAM) AllocBuffer(str.CopyString()));

//	::PostMessage(m_hWnd, WM_SOCK_CLOSE,0,0);
	return FALSE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 이벤트 처리 (On Connect)					//
// [2]PARAMETER : void											//
// [3]RETURN :	false 반환										//
// [4]DATE : 2000년 9월 11일									//
//////////////////////////////////////////////////////////////////
BOOL CClientCtrl::OnConnect()
{	
	m_bConnect = TRUE;											// 연결 변수 설정 ON

	//////////////////////////////////////////////////////////////
	// Initialize Server Side Object and be Ready to prepare Data Process
	//////////////////////////////////////////////////////////////
	if(!send(m_hSocket, (LPCTSTR)"iC8",3,0))
	{
		Stop();
		return FALSE;
	}

	// Posting Connection status
	C8String str = "Client Connected";
	::PostMessage(m_hWnd, WM_STATUS_MSG, 0, (LPARAM) AllocBuffer(str.CopyString()));
	::PostMessage(m_hWnd, WM_CONNECT, 0, (LPARAM) AllocBuffer(str.CopyString()));

	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 서버 연결을 위한 함수 						//
// [2]PARAMETER : void											//
// [3]RETURN :	정상 - TRUE, 실패 - FALSE						//
// [4]DATE : 2000년 9월 11일									//
//////////////////////////////////////////////////////////////////
BOOL CClientCtrl::Connect()
{	
	int nRet = 0;

	// 연결중이라면 
	if(m_bConnect) return TRUE;

	// 처리 헤더 초기화 
	m_bReadHeader = TRUE;

	// 네트워크 이벤트가 살아 있다면 
	if (m_hEvent)
	{
		WSACloseEvent(m_hEvent);
		m_hEvent = NULL;
	}

	// 소켓이 남아 있다면 
	if (m_hSocket)
	{
		closesocket(m_hSocket);
		m_hSocket = NULL;
	}
	
	// 소켓 생성 
	m_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);


	// 소켓 생성 검사 
	if (m_hSocket == INVALID_SOCKET) return FALSE;

	// 네트워크 이벤트 핸들 생성 
	m_hEvent = WSACreateEvent();
	if (m_hEvent == WSA_INVALID_EVENT)
	{
		Stop();
		return FALSE;
	}

	SOCKADDR_IN		saServer;		

	memset(&saServer,0,sizeof(saServer));

	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = inet_addr(m_strIPAddr.CopyString());
	saServer.sin_port = htons(m_nPort);

	// 서버와 Connect
	nRet = connect(m_hSocket,(sockaddr*)&saServer, sizeof(saServer));

	// 소켓 에러이거나 블럭킹이 되었다면 
	if(nRet == SOCKET_ERROR &&	WSAGetLastError() != WSAEWOULDBLOCK)
	{
		Stop();
		return FALSE;
	}

	// Request async notification
	nRet = WSAEventSelect(m_hSocket,
						  m_hEvent,
						  FD_CONNECT|FD_CLOSE|FD_READ);			// 신호를 선별하여 받게 한다 
	
	// 에러라면
	if(nRet == SOCKET_ERROR)
	{
		Stop();
		return FALSE;
	}

	// [1] Timer 동작 
	m_uTimerPeriod = 1;
	timeBeginPeriod(m_uTimerPeriod);
	m_uTimerID_Check = timeSetEvent(1000* 5 , 5, (LPTIMECALLBACK)TimerProc,(DWORD)this,TIME_PERIODIC);  // 여기서 5는 오차범위 1000는 딜레이

	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 데이타 전송									//
// [2]PARAMETER : strData - 전송할 데이타 스트링				//
// [3]RETURN :	void											//
// [4]DATE : 2000년 9월 29일									//
//////////////////////////////////////////////////////////////////
void CClientCtrl::SendPacketData(char *strData,int nSize)
{
	EnterCriticalSection(&m_criticalSection);					// 크리티칼 섹션 진입 

	m_sendBuff.Write(strData, nSize);							// 데이타를 넣는다 
	DoAsyncSendBuff();											// 데이타를 보낸다 

	LeaveCriticalSection(&m_criticalSection);					// 데이타 전송 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 데이타를 비동기로 전송 						//
// [2]PARAMETER : void											//
// [3]RETURN :	void											//
// [4]DATE : 2000년 9월 11일									//
//////////////////////////////////////////////////////////////////
void CClientCtrl::DoAsyncSendBuff()
{	
	int nSendDataLen = m_sendBuff.GetBufferLen();				// 버퍼 길이 

	if(nSendDataLen > MAXBUFSIZE) assert(1);

	char* pData = m_sendBuff.GetBuffer();						// 데이타를 얻는다 
	
	m_nBytesSent = 0;											// 보내진 데이타 길이 변수 초기화 
	
	// 보낸 데이타의 길이가 보내야할 데이타의 길이보다 짧다면 
	while (m_nBytesSent < nSendDataLen)  
	{
		int nBytes;

		// 데이타를 보냈는데 에러가 있다면 
		if ((nBytes = send(m_hSocket, (LPCTSTR)pData, nSendDataLen - m_nBytesSent,0))
			== SOCKET_ERROR)
		{
			// 블럭킹 에러라면 
			if (GetLastError() == WSAEWOULDBLOCK) 
			{
				Sleep(0);
				break;
			}
			else// 블럭킹 에러가 아니라면 
			{				
				closesocket(m_hSocket);					
				m_nBytesSent = 0;				
				return;
			}
		}
		else // 에러가 없다면 
		{
			m_nBytesSent += nBytes;
			m_sendBuff.Delete(nBytes);			
		}
	}
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 파일에서 IP얻기		 						//
// [2]PARAMETER : void											//
// [3]RETURN :	TRUE - 성공 									//
// [4]DATE : 2000년 9월 11일									//
//////////////////////////////////////////////////////////////////
BOOL CClientCtrl::GetIPByFile()
{
	FILE *fileOpen = NULL;
	fileOpen = fopen("Server.txt","rb");

	if(!fileOpen) return FALSE;

	char szIP[16];
	ZeroMemory(szIP, sizeof(szIP));

	fread(szIP,sizeof(szIP),1,fileOpen);

	m_strIPAddr = szIP;

	fcloseall();

	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 메인 윈도에 데이타 전송을 위한 버퍼 생성	//
// [2]PARAMETER : strMsg - 버퍼로 생성하기 위한 데이타 			//
// [3]RETURN :	void											//
// [4]DATE : 2000년 9월 14일									//
//////////////////////////////////////////////////////////////////
char* CClientCtrl::AllocBuffer(char* pData)
{
	int nLen = strlen(pData);									// 문자 길이 
	char *pBuffer = new char[nLen+1];							// 할당 
	strcpy(pBuffer,pData);										// 데이타 카피 
	assert(pBuffer != NULL);									// 확인 
	return pBuffer;												// 포인터 반환 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 메인 윈도에 데이타 전송을 위한 버퍼 생성	//
// [2]PARAMETER : strMsg - 버퍼로 생성하기 위한 데이타 			//
// [3]RETURN :	void											//
// [4]DATE : 2000년 9월 29일									//
//////////////////////////////////////////////////////////////////
char* CClientCtrl::AllocBuffer(char* pData,int nLen)
{
	char *pBuffer = new char[nLen];								// 할당
	CopyMemory(pBuffer,pData,nLen);								// 데이타 카피 
	return pBuffer;												// 포인터 반환
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 네트워크 메세지를 핸들링 하여 분기			//
// [2]PARAMETER : lEvent - 이벤트					 			//
// [3]RETURN :	void											//
// [4]DATE : 2000년 9월 14일									//
//////////////////////////////////////////////////////////////////
BOOL CClientCtrl::NetworkEventHanlder(long lEvent)
{
	BOOL bFlag = TRUE;

	//////////////
	// 메세지 분기 
	switch(lEvent)
	{
	case FD_READ:
		bFlag = OnReadData();	
		//bFlag = OnReadPacket();
		break;
	case FD_WRITE:
		bFlag = OnWrite();
		break;
	case FD_CLOSE:
		bFlag = OnClose();
		break;
	case FD_CONNECT:
		bFlag = OnConnect();
		break;
	default:
		break;
	}

	return bFlag;
}

BOOL CClientCtrl::WinSockInit()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(2, 2);
 	err = WSAStartup( wVersionRequested, &wsaData );
	if(err != 0) return FALSE;
 
	if(LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 ) 
	{
		WSACleanup( );
		return FALSE; 
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 데이타 전송									//
// [2]PARAMETER : strData - 전송할 데이타 스트링				//
// [3]RETURN :	void											//
// [4]DATE : 2000년 9월 29일									//
//////////////////////////////////////////////////////////////////
void CClientCtrl::SendData(char *strData, int nSize)
{
	if(!m_hSocket) return;
	EnterCriticalSection(&m_criticalSection);
//	Encrypto(strData,nSize,"김정윤임다");
	m_sendBuff.Write((char*)&nSize, sizeof(nSize));				// 데이타 사이즈 넣고 
	m_sendBuff.Write(strData, nSize);							// 데이타를 넣는다 
	InterlockedExchange((LONG*)&m_uSendTickCnt,GetTickCount());
	DoAsyncSendBuff();											// 데이타를 보낸다 
	LeaveCriticalSection(&m_criticalSection);
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 이벤트 처리 (On Read)						//
// [2]PARAMETER : void											//
// [3]RETURN :	정상 - TRUE, 실패 - FALSE						//
// [4]DATE : 2000년 9월 11일									//
//////////////////////////////////////////////////////////////////
BOOL CClientCtrl::OnReadPacket()
{
	C7String pData;
	if (m_bReadHeader)											// 이번 패킷에서 Read가 처음이라면 
	{
		m_nTotalRead = 0;
		DWORD dwRead = 0;
		
		ioctlsocket(m_hSocket, FIONREAD, &dwRead);				// Read buffer의 데이타 크기를 읽는다

		if (dwRead < 4)											// 4보다 작다면, 처리 없음 
		{
			return FALSE;							
		}

		recv(m_hSocket, (char*)&m_nBlockSize, (sizeof(int)),0);	// 처리할 데이타 크기를 입력 받는다 
		m_bReadHeader = FALSE;									// Read Header Flag -> FALSE
		pData.Empty(m_strBlock);								// 블럭 처리를 위한 데이타 변수 초기화 
	}
	else
	{
		DWORD dwRead = 0;
		char Packet[MAXPACKETSIZE];
		
		ioctlsocket(m_hSocket, FIONREAD, &dwRead);				// 처리할 데이타 크기를 얻는다 

		if (dwRead <= 0) return FALSE;							// 처리 데이타 크기 0 이하면,
		
		if (dwRead > (unsigned)(m_nBlockSize - m_nTotalRead)) dwRead = m_nBlockSize - m_nTotalRead;		// 길이 조정 

		recv(m_hSocket, Packet, dwRead, 0);						// 데이타 Receive
		pData.AppendBufferLen(m_strBlock,Packet,m_nTotalRead,dwRead);				// m_strBlock += Packet
																					// 멤버 변수에 받은 데이타 추가 
		m_nTotalRead+=dwRead;									// 처리 길이 설정
		if(m_nTotalRead == m_nBlockSize)						// 받은 데이타와 처리길이가 같아 지면 
		{
			CopyMemory(Packet,(char *)&m_nBlockSize,sizeof(m_nBlockSize));				// 완전한 Packet Data를 만들기 위한 Size 삽입
			pData.AppendBufferLen(Packet,m_strBlock,sizeof(m_nBlockSize),m_nBlockSize);	// 데이터 삽입.

			m_Queue.Add(Packet,m_nBlockSize+sizeof(m_nBlockSize));
			::PostMessage(m_hWnd, WM_REC_MSG, 0, NULL);			// 완전한 Packet Data return
			m_bReadHeader = TRUE;								// 헤더 변수 초기화 
		}
	}
	return TRUE;
}

BOOL CClientCtrl::OnReadData()
{
	C7String pData;
	if (m_bReadHeader)											// 이번 패킷에서 Read가 처음이라면 
	{
		m_nTotalRead = 0;
		DWORD dwRead = 0;

		m_strBlock[0] = NULL;
		m_nBlockSize = 0 ;
		
		ioctlsocket(m_hSocket, FIONREAD, &dwRead);				// Read buffer의 데이타 크기를 읽는다

		if (dwRead < 4)	return FALSE;							// 4보다 작다면, 처리 없음 

		recv(m_hSocket, (char*)&m_nBlockSize, (sizeof(int)),0);	// 처리할 데이타 크기를 입력 받는다 
		m_bReadHeader = FALSE;									// Read Header Flag -> FALSE
		pData.Empty(m_strBlock);								// 블럭 처리를 위한 데이타 변수 초기화 
	}
	else
	{
		DWORD dwRead = 0;
		char Packet[MAXPACKETSIZE];
		
		ioctlsocket(m_hSocket, FIONREAD, &dwRead);				// 처리할 데이타 크기를 얻는다 

		if (dwRead <= 0) return FALSE;							// 처리 데이타 크기 0 이하면,
		
		if (dwRead > (unsigned)(m_nBlockSize - m_nTotalRead)) dwRead = m_nBlockSize - m_nTotalRead;		// 길이 조정 

		recv(m_hSocket, Packet, dwRead, 0);						// 데이타 Receive
		pData.AppendBufferLen(m_strBlock,Packet,m_nTotalRead,dwRead);				// m_strBlock += Packet
																					// 멤버 변수에 받은 데이타 추가 
		m_nTotalRead+=dwRead;									// 처리 길이 설정
		if(m_nTotalRead == m_nBlockSize)						// 받은 데이타와 처리길이가 같아 지면 
		{
			m_Queue.Add(m_strBlock,m_nBlockSize);
			::PostMessage(m_hWnd, WM_REC_MSG, 0, NULL);			// 완전한 Packet Data return

			m_bReadHeader = TRUE;								// 헤더 변수 초기화 
		}
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 타이머 프로시져		 						//
// [2]PARAMETER : void											//
// [3]RETURN :	void											//
// [4]DATE : 2001년 12월 10일									//
//////////////////////////////////////////////////////////////////
void CClientCtrl::TimerProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
/*
	CClientCtrl* pThis = (CClientCtrl*)dwUser;
	DWORD nRet = GetTickCount();
	nRet -= pThis->m_uSendTickCnt;
	char szCHK[4];
	wsprintf(szCHK,"chk");
	szCHK[3] = NULL;

	if(!pThis->m_bConnect) return ;

	// 데이타를 날려준다 
	if(uID == (UINT)(pThis->m_uTimerID_Check))
		if(nRet > 1000 * 4)	pThis->SendData(szCHK,sizeof(szCHK));
*/
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 모니터링에 패킷 알림						//
// [2]PARAMETER : void											//
// [3]RETURN :	void											//
// [4]DATE : 2001년 12월 10일									//
//////////////////////////////////////////////////////////////////
void CClientCtrl::SendMessageToMonitor(char *pszData, int nSize, int nFlag)
{
	if(pszData == NULL || nSize <= 0) return;

	HWND hwnd =  NULL;
	if((hwnd = ::FindWindow(NULL,(LPCTSTR)"MessageMonitor")) == NULL) return;

	PCOPYDATASTRUCT pcds = new COPYDATASTRUCT;

	HGLOBAL hGlobal = NULL;
	hGlobal =::GlobalAlloc(GMEM_MOVEABLE,nSize+1);
	if(hGlobal == NULL) return;
	
	LPVOID pVoid = (LPVOID)::GlobalLock(hGlobal);
	if(pVoid != NULL) pcds->lpData = pVoid;
	::GlobalUnlock(hGlobal);

	memcpy(pcds->lpData, (const void*)pszData, nSize);
	pcds->cbData = nSize;
	pcds->dwData = nFlag;

	::SendMessage(hwnd,WM_COPYDATA,0,(LPARAM)(PCOPYDATASTRUCT)pcds);
	::GlobalFree(hGlobal);

	delete pcds;
}

void CClientCtrl::SetParent(HWND hWnd)
{
	m_hWnd = hWnd;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// 암호화 모듈 
/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CClientCtrl::Encrypto(char *szData, int nLen, char *szKey)
{
	if(nLen <= 0 || szData == NULL) return FALSE;
	if(szKey == NULL)
	{
		// NULL일때, Key 값을 만들어준다. 
	}

	int nKeyLen = strlen(szKey);									// 키의 길이
	if(nKeyLen <= 0) return FALSE;

	int i;
	MyDATA* bitEnCode = NULL;

	for(i = 0 ; i < nLen ; i++)
	{
		szData[i] = szData[i]^szKey[(i%nKeyLen)];
		bitEnCode = (MyDATA*)&szData[i];
		switch( nLen%(i+1) )
		{
		case 0:	bitEnCode->bit.bit5 = bitEnCode->bit.bit5^0x00;
				bitEnCode->bit.bit1 = bitEnCode->bit.bit1^bitEnCode->bit.bit5;
				bitEnCode->bit.bit4 = bitEnCode->bit.bit4^0x01;
				bitEnCode->bit.bit2 = bitEnCode->bit.bit2^bitEnCode->bit.bit4;
				break;
		case 1: bitEnCode->bit.bit3 = bitEnCode->bit.bit3^0x01;
				bitEnCode->bit.bit7 = bitEnCode->bit.bit7^bitEnCode->bit.bit3;
				break;
		default:bitEnCode->bit.bit6 = bitEnCode->bit.bit6^bitEnCode->bit.bit8;
				break;
		}
	}
	return TRUE;
}

BOOL CClientCtrl::Decrypto(char *szData, int nLen, char *szKey)
{
	if(nLen <= 0 || szData == NULL) return FALSE;
	if(szKey == NULL)
	{
		// NULL일때, Key 값을 만들어준다. 
	}

	int nKeyLen = strlen(szKey);									// 키의 길이
	if(nKeyLen <= 0) return FALSE;

	int i;
	MyDATA* bitEnCode = NULL;

	for(i = 0 ; i < nLen ; i++)
	{
		bitEnCode = (MyDATA*)&szData[i];
		switch( nLen%(i+1) )
		{
		case 0:	bitEnCode->bit.bit1 = bitEnCode->bit.bit1^bitEnCode->bit.bit5;
				bitEnCode->bit.bit5 = bitEnCode->bit.bit5^0x00;
				bitEnCode->bit.bit2 = bitEnCode->bit.bit2^bitEnCode->bit.bit4;
				bitEnCode->bit.bit4 = bitEnCode->bit.bit4^0x01;
				break;
		case 1: bitEnCode->bit.bit7 = bitEnCode->bit.bit7^bitEnCode->bit.bit3;
				bitEnCode->bit.bit3 = bitEnCode->bit.bit3^0x01;
				break;
		default:bitEnCode->bit.bit6 = bitEnCode->bit.bit6^bitEnCode->bit.bit8;
				break;
		}
		szData[i] = szData[i]^szKey[(i%nKeyLen)];
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// 정윤 테스트 함수 
/////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Send Thread를 돌리기 위한 임시 함수 		//
// [2]PARAMETER : void											//
// [3]RETURN :	void											//
// [4]DATE : 2000년 9월 11일									//
//////////////////////////////////////////////////////////////////
void CClientCtrl::TestLoop()
{
	HANDLE	Handle;
	UINT	dwThreadId;	

	Handle = (HANDLE)_beginthreadex(NULL,			// Security
				 0,									// Stack size - use default
				 ThreadSendProc,     				// Thread fn entry point
				 (void*) this,	      
				 0,									// Init flag
				 &dwThreadId);						// Thread address

	CloseHandle(Handle);
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Test를 위한 Send Thread						//
// [2]PARAMETER : lpVoid - 이 클래스 							//
// [3]RETURN :	void											//
// [4]DATE : 2000년 9월 11일									//
//////////////////////////////////////////////////////////////////
unsigned CClientCtrl::ThreadSendProc(LPVOID lpVoid)
{
	CClientCtrl* pThis = reinterpret_cast<CClientCtrl*>(lpVoid);

	PostMessage(pThis->m_hWnd,WM_SEND_ON,NULL,NULL);			// 메인 윈도우에 

	DWORD i = 1;

	while(i <= 100*1000 && pThis->m_bConnect == TRUE)
	{
		Sleep(0);
		char szTemp[200];
		memset(szTemp,NULL,200);
		wsprintf(szTemp,"Msg:%d",(int)i);
		pThis->SendData(szTemp,strlen(szTemp));
		if(!pThis->m_bConnect) return 0;
		i++;
	}

	PostMessage(pThis->m_hWnd,WM_SEND_OFF,NULL,NULL);			// 메인 윈도우에 알림 

	return 0; // Normal Thread Exit Code...
}
