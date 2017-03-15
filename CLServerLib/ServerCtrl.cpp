// ServerCtrl.cpp: implementation of the CServerCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerCtrl.h"

//////////////////////////////////////////////////////////////////
// Construction/Destruction			
//////////////////////////////////////////////////////////////////
CServerCtrl::CServerCtrl()
{
	m_hServerStopEvent	=	NULL;								// Thread-Stop Event Handle Init..
}

CServerCtrl::~CServerCtrl()
{
	if(m_hServerStopEvent) CloseHandle(m_hServerStopEvent);		// Close IOCP Event Handle
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : IOCP 핸들 생성								//
// [2]PARAMETER : void											//
// [3]RETURN : HANDLE = 생성된 IOCP								//
// [4]DATE : 2000년 9월 4일										//
//////////////////////////////////////////////////////////////////
HANDLE CServerCtrl::CreateIOCP()
{
	return CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0);
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Listen 소켓 생성 							//
// [2]PARAMETER : void											//
// [3]RETURN : SOCKET = 생성된 LISTEN SOCKET					//
// [4]DATE : 2000년 9월 4일										//
//////////////////////////////////////////////////////////////////
SOCKET CServerCtrl::CreateListenSocket(int nServerPort)
{
	SOCKET Socket = NULL;								// a Socket Variable for using Listener

	// 소켓 상태 설정 구조체 선언 
	SOCKADDR_IN		addr;
					addr.sin_family			= AF_INET;
					addr.sin_addr.s_addr	= INADDR_ANY;
					addr.sin_port			= htons( (short)nServerPort );

	// [1] Create Listen Socket
	Socket = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_IP,NULL,0,WSA_FLAG_OVERLAPPED );

	if(Socket == NULL)
		return Socket;

	// [2] bind listen socket
	if(bind(Socket,(SOCKADDR *)&addr,sizeof(addr)) != 0)
		return Socket;

	// [3] listening for an concurrent incoming connections limited in 5
	listen(Socket,5);

	return Socket;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : SOCKET과련 라이브러리 활성화 				//
// [2]PARAMETER : void											//
// [3]RETURN : void							 					//
// [4]DATE : 2000년 9월 4일										//
//////////////////////////////////////////////////////////////////
BOOL CServerCtrl::InitSocket()
{
	WSADATA wsaData;											// Initialzing Variables 
	return (SOCKET_ERROR != WSAStartup(0x202,&wsaData));		// Start Up
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 시스템의 CPU 수를 구함 						//
// [2]PARAMETER : void											//
// [3]RETURN : int = 현재 시스템의 CPU 수 반환 					//
// [4]DATE : 2000년 9월 4일										//
//////////////////////////////////////////////////////////////////
int CServerCtrl::GetNumberOfProcess()
{
	SYSTEM_INFO si;												// a System Info Structure Object
    GetSystemInfo( &si );										// Get the System Information
	return (int)si.dwNumberOfProcessors;						// return the number of processors
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 서버 구동			 						//
// [2]PARAMETER : void											//
// [3]RETURN : BOOL - 의미 없음				 					//
// [4]DATE : 2000년 9월 4일										//
//////////////////////////////////////////////////////////////////
BOOL CServerCtrl::Run()
{
	// [01] 변수 및 객체 선언 
	SOCKET			sckListener;								// Listen Socket

	int				nCnt = 0;									// 루프 변수  
	unsigned int	nDummy;										// 쓰레기 값 처리 
	int				nMaxThreadNum;								// 최대 스레드 수 결정 

	CConnection*	pConnections	= NULL;						// Client List 변수 
	CPacketPool*	pPacketPool		= NULL;						// 메모리 처리 변수 
	
	HANDLE*			pThread		= NULL;							// 메인 스레드 핸들 
	HANDLE			hIOCP		= NULL;							// IOCP 핸들 

	long			uTimerID		= 0;
	long			uTimerPeriod	= 1;

	// [03] initialize socket library
	if(!InitSocket())
		goto Error_Procss;

	// [04] Create Listener Socket
	if(NULL == (sckListener = CreateListenSocket(DEF_SERVER_PORT)))
		goto Error_Procss;

	// [05] Create IOCP 
	if(NULL == (hIOCP = CreateIOCP()))
		goto Error_Procss;

	// [06] Create memory pool for ovl structure and packet buffer
	if( 0 == (pPacketPool = new CPacketPool) )
		goto Error_Procss;

	// [07] Initializing Memory Pool
	if( 0 == pPacketPool->Create(DEF_MAXUSER,NULL,DEF_MAXPACKETSIZE))
		goto Error_Procss;

	// [08] Display Server Information 
	char	szDate[32],
			szTime[32];

	_tzset();
	_strdate( szDate );
	_strtime( szTime );
	printf("------------------------------------------------------------------------------\n");
	printf( "%s %s initialized at %s, %s\n", DEF_SERVER_NAME, DEF_SERVER_VERSION, szDate, szTime );
	printf("------------------------------------------------------------------------------\n");
	
	// [09] about Main Threads
	nMaxThreadNum	=	GetNumberOfProcess() * 2 + 1;				// CPU수 * 2개로 Thread 수 결정 

	if((pThread = new HANDLE[nMaxThreadNum]) == 0)					// Create thread Control Handle
		goto Error_Procss;

	for(nCnt = 0; nCnt < nMaxThreadNum; nCnt++)						// Run Thread 
	{
		if(0 == (pThread[nCnt] = (HANDLE)_beginthreadex(0,0,Thread_Main,hIOCP,0,&nDummy)))
			goto Error_Procss;
	}

	// [10] Connection Array(Client Controler..)
	if(0 == (pConnections = new CConnection[DEF_MAXUSER]))				// Create MAXUSER(1000) pConnections
		goto Error_Procss;

	for(nCnt = 0; nCnt < DEF_MAXUSER; nCnt++)							// Initialize pClientArray
		if(0 == pConnections[nCnt].Create(	nCnt,
											hIOCP,
											sckListener,
											pPacketPool,
											NULL,
											DEF_MAXBUFSIZE,
											DEF_MAXPACKETSIZE,
											DEF_MAXUSER)) goto Error_Procss;
		
	// [11] Connect listener socket to IOCP
	if(CreateIoCompletionPort((HANDLE)sckListener,hIOCP,0,0) == 0)
		goto Error_Procss;

	// [12] Timer Setting 
	timeBeginPeriod(uTimerPeriod);
	uTimerID = timeSetEvent(8000,20, (LPTIMECALLBACK)TimerProc,(DWORD)&pConnections[0],TIME_PERIODIC);  // 여기서 5는 오차범위 1000는 딜레이

	// [13] Create event for stop thread pool
	if(NULL == (m_hServerStopEvent = CreateEvent(0,TRUE,FALSE,0)))
		goto Error_Procss;

//--------------> Server Initializing has been done <---------------//

//////////////////////////////////////////////////////////////////////
//						 Waiting Infinitely							//
//////////////////////////////////////////////////////////////////////

	// wait for stop event signal, 주쓰레드는 여기서 대기상태로..
	WaitForSingleObject( m_hServerStopEvent, INFINITE );


//////////////////////////////////////////////////////////////////////
//						Server Closing Process						//
//////////////////////////////////////////////////////////////////////

	// [1] Queue Suicide Packets into each IOCP Main Thread
	for( nCnt = 0; nCnt < nMaxThreadNum; nCnt++ )
	{
		if(0 == PostQueuedCompletionStatus(hIOCP,0,IOCP_SHUTDOWN,0)) goto Error_Procss;
	}

	// [2] Wait for thread terminations
	nCnt = WaitForMultipleObjects( nMaxThreadNum, &pThread[0], TRUE, 5*1000);

	switch ( nCnt )
	{
		case WAIT_TIMEOUT:
			printf( "Not all threads died in time.\n" );
			break;
		case WAIT_FAILED:
			printf( "WAIT_FAILED, WaitForMultipleObjects()\n" );
			break;
		default:
			break;
	}
	
//////////////////////////////////////////////////////////////////////
//							Error Case Process						//
//////////////////////////////////////////////////////////////////////
Error_Procss:
	
	// [2] 타이머 죽이기
	timeKillEvent(uTimerID); 
	timeEndPeriod(uTimerPeriod);

	// [1] Close Thread Handles
	if( pThread )
	{
		for( nCnt = 0; nCnt < nMaxThreadNum; nCnt++ )
			CloseHandle( pThread[nCnt] );
		delete [] pThread;
		pThread = NULL;
	}	 

	// [3] Close Stop Event Handle
	if( m_hServerStopEvent )
	{
		CloseHandle( m_hServerStopEvent );
		m_hServerStopEvent = NULL;
	}

	// [4] Close All User Sockets
	if( pConnections )
	{
		for(nCnt = 0 ; nCnt < DEF_MAXUSER ; nCnt++)
		{
			pConnections[nCnt].Disconnect();
		}
		printf( "shutdown user context..\n" );
		delete [] pConnections;
		pConnections = NULL;
	}

	// [5] Shutdown the Packet Pool
	if( pPacketPool )
	{
		printf( "shutdown packet pool..\n" );
		delete pPacketPool;
		pPacketPool = NULL;
	}

	// [6] Close Listener Socket
	if(sckListener != NULL )
	{
		struct linger li = {1, 0};	// Default: SO_DONTLINGER
		shutdown(sckListener, SD_BOTH);
		setsockopt(sckListener, SOL_SOCKET, SO_LINGER, (char *)&li, sizeof(li));
		closesocket(sckListener );
		sckListener = NULL;
	}

	// [7] Close IOCP Handle
	if(hIOCP) CloseHandle(hIOCP);

	// [8] 소켓 라이브러리 종료 
	WSACleanup();

	// [9] Show the Result of Close Processing
	printf("shutdown sequence finished..\n\npress any key.\n");

	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 서버 정지 			 						//
// [2]PARAMETER : void											//
// [3]RETURN : BOOL - 의미 없음				 					//
// [4]DATE : 2000년 9월 4일										//
//////////////////////////////////////////////////////////////////
BOOL CServerCtrl::Stop()
{
	SetEvent(m_hServerStopEvent);								// 서버 정지 신호 발생 
	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : IOCP처리를 담당하는 메인 스레드 			//
// [2]PARAMETER : lpVoid - IOCP Handle							//
// [3]RETURN : BOOL - 의미 없음				 					//
// [4]DATE : 2000년 9월 4일										//
//////////////////////////////////////////////////////////////////
unsigned CServerCtrl::Thread_Main(LPVOID lpVoid)
{
	DWORD			dwIoSize;									// 완료처리 사이즈 얻음 
	ULONG			lCompletionKey;								// 무한 루프 탈출 용이 됨 
	BOOL			bSuccess;									// 블럭킹 처리 에러 확인  
	HANDLE			hIOCP = (HANDLE)lpVoid;						// IOCP 핸들 얻음 
	
	LPOVERLAPPED	lpOverlapped;								// 중첩 확장 포인터 

	////////////
	// 무한 루프 
	while( TRUE )
	{
		// IOCP 처리를 기다리는 BLOCKING MODE
		bSuccess = GetQueuedCompletionStatus(hIOCP,							// IOCP Handle
											 &dwIoSize,						// 처리 사이즈 
											 &lCompletionKey,				// 완료 키 
											 (LPOVERLAPPED*) &lpOverlapped,	// 중첩 확장 
											 INFINITE);						// Waiting Time 

		LPOVERLAPPEDPLUS lpOverlapPlus = (LPOVERLAPPEDPLUS) lpOverlapped;

		if(bSuccess)
		{
			// 종료 신호가 들어왔다면, 루프 탈출 
			if( lCompletionKey == IOCP_SHUTDOWN )
				break;

			if( NULL != lpOverlapPlus )
			{
				///////////////////////////////////////////////
				// 처리가 정상적으로 이루어진다면 이쪽으로...
				lpOverlapPlus->dwBytes = dwIoSize;				// 처리 데이타 Size
				// 처리 변수 Cast 변환 
				CConnection * lpClientConn = (CConnection *) lpOverlapPlus->pClientConn;
				lpClientConn->DoIo(lpOverlapPlus);				// IOCP 처리 핸들링 
			}
		}
		else
		{
			if( NULL == lpOverlapPlus )
			{
				printf( "Critical Error on GQCS()\n" );
			}
			else
			{
				// 처리 변수 Cast 변환
				CConnection* lpClientConn = (CConnection *) lpOverlapPlus->pClientConn;
				printf("IOCP 에러의한 Close_Open()호출\n");
				lpClientConn->Close_Open(lpOverlapPlus, TRUE);	// 연결 해제 
			}
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 타이머 함수						 			//
// [2]PARAMETER : dwUser  - 연결 객체를 넘기는 변수 			//
// [3]RETURN : void							 					//
// [4]DATE : 2000년 11월 21일									//
//////////////////////////////////////////////////////////////////
void CServerCtrl::TimerProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	CConnection* pConnection = NULL;
	CConnection* pConnectionArray = NULL;

	char	szTime[32];
	static int nConnectionCnt = 0;
	int nCnt = 0;
	int nMaxUser = 0;

	if((pConnectionArray = reinterpret_cast<CConnection*>(dwUser)) == NULL) assert(0);

	nMaxUser = pConnectionArray[0].GetMaxUser();
	
	_tzset();
	_strtime( szTime );

	while(nCnt < 500)
	{
		if(nConnectionCnt >= nMaxUser)
		{
			nConnectionCnt = 0;						
			break;
		}
		pConnection = &pConnectionArray[nConnectionCnt];
		if(pConnection->IsConnect())
		{	
			DWORD dwRet = GetTickCount();
			dwRet -= pConnection->GetRecvTickCnt();
				
			if(dwRet > TIMELIMIT_PERIOD)
			{
				printf("%d번 ID Connection Dead, Now Time : %s\n",pConnection->GetIndex(),szTime);
				pConnection->Disconnect();
			}
			else printf("%d번 ID Connection Alive, Now Time : %s\n",pConnection->GetIndex(),szTime);		
		}
		Sleep(8);
		nConnectionCnt++;
		nCnt++;
	}
}
