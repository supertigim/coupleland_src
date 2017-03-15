// ClientCtrl.cpp: implementation of the CClientCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ClientCtrl.h"

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ������							 			//
// [2]PARAMETER : void											//
// [3]RETURN :	void											//
// [4]DATE : 2000�� 9�� 10��									//
//////////////////////////////////////////////////////////////////
CClientCtrl::CClientCtrl()
{
	//////////////
	// ���� �ʱ�ȭ
	m_bConnect = FALSE;	
	m_hSocket = NULL;
	m_hEvent = NULL;
	m_hThread = NULL;

	//////////////////////////
	// Ÿ�̸� ���� 
	m_uTimerID_Check = -1;
	m_uTimerPeriod = -1;

	m_uSendTickCnt = 0;

	InitializeCriticalSection(&m_criticalSection);				

	WinSockInit();
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : �Ҹ��� 							 			//
// [2]PARAMETER : void											//
// [3]RETURN :	void											//
// [4]DATE : 2000�� 9�� 11��									//
//////////////////////////////////////////////////////////////////
CClientCtrl::~CClientCtrl()
{
    Stop();														// ���� �Լ� ȣ�� 

	if(m_hEvent) WSACloseEvent(m_hEvent);
	m_hEvent = NULL;
	if(m_hThread) CloseHandle(m_hThread);
	m_hThread = NULL;

	DeleteCriticalSection(&m_criticalSection);
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Ŭ���� �ʱ�ȭ 					 			//
// [2]PARAMETER : strIPAddr - ����IP�ּ�, nPort - ��Ʈ��ȣ,		//
//				  hWnd - �θ� ������ �ڵ�						//
// [3]RETURN :	���� - TRUE, ���� - FALSE						//
// [4]DATE : 2000�� 9�� 11��									//
//////////////////////////////////////////////////////////////////
BOOL CClientCtrl::Init( int nPort, HWND hWnd, char* szIP )
{
	m_nPort = nPort;											// ��Ʈ ��ȣ 
	UINT dwThreadId = 0;										// ������ ������ ���� ���� 
	m_hWnd = hWnd;												// �θ� �ڵ� 
	m_Queue.Create(MAXQUEUESIZE);

	// ���Ϸ� IP ADDRESS�� ������ ���ٸ�,
	if(!szIP)
	{
		if(!GetIPByFile())	return FALSE;						// ���� ��ȯ 
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
	
	return !(m_hThread == NULL);								// ������ ���� �˻� 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : �ý��� ���� 					 			//
// [2]PARAMETER : void											//
// [3]RETURN :	void											//
// [4]DATE : 2000�� 9�� 10��									//
//////////////////////////////////////////////////////////////////
void CClientCtrl::Stop()
{
	if(m_hSocket) closesocket(m_hSocket);
	m_hSocket = NULL;

	SetEvent(m_hEvent);											// ������ �˸�

	/////////////////////////////////
	// Ÿ�̸� ���̱�
	if(m_uTimerID_Check != -1)	timeKillEvent(m_uTimerID_Check); 
	if(m_uTimerPeriod != -1)	timeEndPeriod(m_uTimerPeriod); 

	m_uTimerID_Check = -1;
	m_uTimerPeriod = -1;

	m_uSendTickCnt = 0;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Main Thread, ��Ʈ��ũ �̺�Ʈ ó��			//
// [2]PARAMETER : void											//
// [3]RETURN :	void											//
// [4]DATE : 2000�� 9�� 10��									//
//////////////////////////////////////////////////////////////////
unsigned CClientCtrl::SocketThreadProc(LPVOID lParam)
{
	// Ŭ������ ������ ����
	CClientCtrl* pThis = reinterpret_cast<CClientCtrl*>(lParam);

	WSANETWORKEVENTS events;										// ��Ʈ��ũ �̺�Ʈ ���� 

	BOOL bThreadRun = TRUE;											// ���� ���� ���� 
	int nRet;

	// ������ �Ǿ����� �˻� 
	if(!pThis->Connect())
	{
		Sleep(100);													// Sleep...
		pThis->OnClose();	
		return 0;
	}

	// ������ ���� ���� 
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
		
		// ���� �������,
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
// [1]DESCRIPTION : �̺�Ʈ ó�� (On Write)						//
// [2]PARAMETER : void											//
// [3]RETURN :	false ��ȯ										//
// [4]DATE : 2000�� 9�� 11��									//
//////////////////////////////////////////////////////////////////
BOOL CClientCtrl::OnWrite()
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : �̺�Ʈ ó�� (On Close)						//
// [2]PARAMETER : void											//
// [3]RETURN :	false ��ȯ										//
// [4]DATE : 2000�� 9�� 11��									//
//////////////////////////////////////////////////////////////////
BOOL CClientCtrl::OnClose()
{
	m_bConnect = FALSE;											// Connect ���� ���� 

	C8String str = "Client Disconnected";
	::PostMessage(m_hWnd, WM_STATUS_MSG, 0, (LPARAM) AllocBuffer(str.CopyString()));
	::PostMessage(m_hWnd, WN_DISCONNECT, 0, (LPARAM) AllocBuffer(str.CopyString()));

//	::PostMessage(m_hWnd, WM_SOCK_CLOSE,0,0);
	return FALSE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : �̺�Ʈ ó�� (On Connect)					//
// [2]PARAMETER : void											//
// [3]RETURN :	false ��ȯ										//
// [4]DATE : 2000�� 9�� 11��									//
//////////////////////////////////////////////////////////////////
BOOL CClientCtrl::OnConnect()
{	
	m_bConnect = TRUE;											// ���� ���� ���� ON

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
// [1]DESCRIPTION : ���� ������ ���� �Լ� 						//
// [2]PARAMETER : void											//
// [3]RETURN :	���� - TRUE, ���� - FALSE						//
// [4]DATE : 2000�� 9�� 11��									//
//////////////////////////////////////////////////////////////////
BOOL CClientCtrl::Connect()
{	
	int nRet = 0;

	// �������̶�� 
	if(m_bConnect) return TRUE;

	// ó�� ��� �ʱ�ȭ 
	m_bReadHeader = TRUE;

	// ��Ʈ��ũ �̺�Ʈ�� ��� �ִٸ� 
	if (m_hEvent)
	{
		WSACloseEvent(m_hEvent);
		m_hEvent = NULL;
	}

	// ������ ���� �ִٸ� 
	if (m_hSocket)
	{
		closesocket(m_hSocket);
		m_hSocket = NULL;
	}
	
	// ���� ���� 
	m_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);


	// ���� ���� �˻� 
	if (m_hSocket == INVALID_SOCKET) return FALSE;

	// ��Ʈ��ũ �̺�Ʈ �ڵ� ���� 
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

	// ������ Connect
	nRet = connect(m_hSocket,(sockaddr*)&saServer, sizeof(saServer));

	// ���� �����̰ų� ��ŷ�� �Ǿ��ٸ� 
	if(nRet == SOCKET_ERROR &&	WSAGetLastError() != WSAEWOULDBLOCK)
	{
		Stop();
		return FALSE;
	}

	// Request async notification
	nRet = WSAEventSelect(m_hSocket,
						  m_hEvent,
						  FD_CONNECT|FD_CLOSE|FD_READ);			// ��ȣ�� �����Ͽ� �ް� �Ѵ� 
	
	// �������
	if(nRet == SOCKET_ERROR)
	{
		Stop();
		return FALSE;
	}

	// [1] Timer ���� 
	m_uTimerPeriod = 1;
	timeBeginPeriod(m_uTimerPeriod);
	m_uTimerID_Check = timeSetEvent(1000* 5 , 5, (LPTIMECALLBACK)TimerProc,(DWORD)this,TIME_PERIODIC);  // ���⼭ 5�� �������� 1000�� ������

	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ����Ÿ ����									//
// [2]PARAMETER : strData - ������ ����Ÿ ��Ʈ��				//
// [3]RETURN :	void											//
// [4]DATE : 2000�� 9�� 29��									//
//////////////////////////////////////////////////////////////////
void CClientCtrl::SendPacketData(char *strData,int nSize)
{
	EnterCriticalSection(&m_criticalSection);					// ũ��ƼĮ ���� ���� 

	m_sendBuff.Write(strData, nSize);							// ����Ÿ�� �ִ´� 
	DoAsyncSendBuff();											// ����Ÿ�� ������ 

	LeaveCriticalSection(&m_criticalSection);					// ����Ÿ ���� 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ����Ÿ�� �񵿱�� ���� 						//
// [2]PARAMETER : void											//
// [3]RETURN :	void											//
// [4]DATE : 2000�� 9�� 11��									//
//////////////////////////////////////////////////////////////////
void CClientCtrl::DoAsyncSendBuff()
{	
	int nSendDataLen = m_sendBuff.GetBufferLen();				// ���� ���� 

	if(nSendDataLen > MAXBUFSIZE) assert(1);

	char* pData = m_sendBuff.GetBuffer();						// ����Ÿ�� ��´� 
	
	m_nBytesSent = 0;											// ������ ����Ÿ ���� ���� �ʱ�ȭ 
	
	// ���� ����Ÿ�� ���̰� �������� ����Ÿ�� ���̺��� ª�ٸ� 
	while (m_nBytesSent < nSendDataLen)  
	{
		int nBytes;

		// ����Ÿ�� ���´µ� ������ �ִٸ� 
		if ((nBytes = send(m_hSocket, (LPCTSTR)pData, nSendDataLen - m_nBytesSent,0))
			== SOCKET_ERROR)
		{
			// ��ŷ ������� 
			if (GetLastError() == WSAEWOULDBLOCK) 
			{
				Sleep(0);
				break;
			}
			else// ��ŷ ������ �ƴ϶�� 
			{				
				closesocket(m_hSocket);					
				m_nBytesSent = 0;				
				return;
			}
		}
		else // ������ ���ٸ� 
		{
			m_nBytesSent += nBytes;
			m_sendBuff.Delete(nBytes);			
		}
	}
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ���Ͽ��� IP���		 						//
// [2]PARAMETER : void											//
// [3]RETURN :	TRUE - ���� 									//
// [4]DATE : 2000�� 9�� 11��									//
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
// [1]DESCRIPTION : ���� ������ ����Ÿ ������ ���� ���� ����	//
// [2]PARAMETER : strMsg - ���۷� �����ϱ� ���� ����Ÿ 			//
// [3]RETURN :	void											//
// [4]DATE : 2000�� 9�� 14��									//
//////////////////////////////////////////////////////////////////
char* CClientCtrl::AllocBuffer(char* pData)
{
	int nLen = strlen(pData);									// ���� ���� 
	char *pBuffer = new char[nLen+1];							// �Ҵ� 
	strcpy(pBuffer,pData);										// ����Ÿ ī�� 
	assert(pBuffer != NULL);									// Ȯ�� 
	return pBuffer;												// ������ ��ȯ 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ���� ������ ����Ÿ ������ ���� ���� ����	//
// [2]PARAMETER : strMsg - ���۷� �����ϱ� ���� ����Ÿ 			//
// [3]RETURN :	void											//
// [4]DATE : 2000�� 9�� 29��									//
//////////////////////////////////////////////////////////////////
char* CClientCtrl::AllocBuffer(char* pData,int nLen)
{
	char *pBuffer = new char[nLen];								// �Ҵ�
	CopyMemory(pBuffer,pData,nLen);								// ����Ÿ ī�� 
	return pBuffer;												// ������ ��ȯ
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ��Ʈ��ũ �޼����� �ڵ鸵 �Ͽ� �б�			//
// [2]PARAMETER : lEvent - �̺�Ʈ					 			//
// [3]RETURN :	void											//
// [4]DATE : 2000�� 9�� 14��									//
//////////////////////////////////////////////////////////////////
BOOL CClientCtrl::NetworkEventHanlder(long lEvent)
{
	BOOL bFlag = TRUE;

	//////////////
	// �޼��� �б� 
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
// [1]DESCRIPTION : ����Ÿ ����									//
// [2]PARAMETER : strData - ������ ����Ÿ ��Ʈ��				//
// [3]RETURN :	void											//
// [4]DATE : 2000�� 9�� 29��									//
//////////////////////////////////////////////////////////////////
void CClientCtrl::SendData(char *strData, int nSize)
{
	if(!m_hSocket) return;
	EnterCriticalSection(&m_criticalSection);
//	Encrypto(strData,nSize,"�������Ӵ�");
	m_sendBuff.Write((char*)&nSize, sizeof(nSize));				// ����Ÿ ������ �ְ� 
	m_sendBuff.Write(strData, nSize);							// ����Ÿ�� �ִ´� 
	InterlockedExchange((LONG*)&m_uSendTickCnt,GetTickCount());
	DoAsyncSendBuff();											// ����Ÿ�� ������ 
	LeaveCriticalSection(&m_criticalSection);
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : �̺�Ʈ ó�� (On Read)						//
// [2]PARAMETER : void											//
// [3]RETURN :	���� - TRUE, ���� - FALSE						//
// [4]DATE : 2000�� 9�� 11��									//
//////////////////////////////////////////////////////////////////
BOOL CClientCtrl::OnReadPacket()
{
	C7String pData;
	if (m_bReadHeader)											// �̹� ��Ŷ���� Read�� ó���̶�� 
	{
		m_nTotalRead = 0;
		DWORD dwRead = 0;
		
		ioctlsocket(m_hSocket, FIONREAD, &dwRead);				// Read buffer�� ����Ÿ ũ�⸦ �д´�

		if (dwRead < 4)											// 4���� �۴ٸ�, ó�� ���� 
		{
			return FALSE;							
		}

		recv(m_hSocket, (char*)&m_nBlockSize, (sizeof(int)),0);	// ó���� ����Ÿ ũ�⸦ �Է� �޴´� 
		m_bReadHeader = FALSE;									// Read Header Flag -> FALSE
		pData.Empty(m_strBlock);								// �� ó���� ���� ����Ÿ ���� �ʱ�ȭ 
	}
	else
	{
		DWORD dwRead = 0;
		char Packet[MAXPACKETSIZE];
		
		ioctlsocket(m_hSocket, FIONREAD, &dwRead);				// ó���� ����Ÿ ũ�⸦ ��´� 

		if (dwRead <= 0) return FALSE;							// ó�� ����Ÿ ũ�� 0 ���ϸ�,
		
		if (dwRead > (unsigned)(m_nBlockSize - m_nTotalRead)) dwRead = m_nBlockSize - m_nTotalRead;		// ���� ���� 

		recv(m_hSocket, Packet, dwRead, 0);						// ����Ÿ Receive
		pData.AppendBufferLen(m_strBlock,Packet,m_nTotalRead,dwRead);				// m_strBlock += Packet
																					// ��� ������ ���� ����Ÿ �߰� 
		m_nTotalRead+=dwRead;									// ó�� ���� ����
		if(m_nTotalRead == m_nBlockSize)						// ���� ����Ÿ�� ó�����̰� ���� ���� 
		{
			CopyMemory(Packet,(char *)&m_nBlockSize,sizeof(m_nBlockSize));				// ������ Packet Data�� ����� ���� Size ����
			pData.AppendBufferLen(Packet,m_strBlock,sizeof(m_nBlockSize),m_nBlockSize);	// ������ ����.

			m_Queue.Add(Packet,m_nBlockSize+sizeof(m_nBlockSize));
			::PostMessage(m_hWnd, WM_REC_MSG, 0, NULL);			// ������ Packet Data return
			m_bReadHeader = TRUE;								// ��� ���� �ʱ�ȭ 
		}
	}
	return TRUE;
}

BOOL CClientCtrl::OnReadData()
{
	C7String pData;
	if (m_bReadHeader)											// �̹� ��Ŷ���� Read�� ó���̶�� 
	{
		m_nTotalRead = 0;
		DWORD dwRead = 0;

		m_strBlock[0] = NULL;
		m_nBlockSize = 0 ;
		
		ioctlsocket(m_hSocket, FIONREAD, &dwRead);				// Read buffer�� ����Ÿ ũ�⸦ �д´�

		if (dwRead < 4)	return FALSE;							// 4���� �۴ٸ�, ó�� ���� 

		recv(m_hSocket, (char*)&m_nBlockSize, (sizeof(int)),0);	// ó���� ����Ÿ ũ�⸦ �Է� �޴´� 
		m_bReadHeader = FALSE;									// Read Header Flag -> FALSE
		pData.Empty(m_strBlock);								// �� ó���� ���� ����Ÿ ���� �ʱ�ȭ 
	}
	else
	{
		DWORD dwRead = 0;
		char Packet[MAXPACKETSIZE];
		
		ioctlsocket(m_hSocket, FIONREAD, &dwRead);				// ó���� ����Ÿ ũ�⸦ ��´� 

		if (dwRead <= 0) return FALSE;							// ó�� ����Ÿ ũ�� 0 ���ϸ�,
		
		if (dwRead > (unsigned)(m_nBlockSize - m_nTotalRead)) dwRead = m_nBlockSize - m_nTotalRead;		// ���� ���� 

		recv(m_hSocket, Packet, dwRead, 0);						// ����Ÿ Receive
		pData.AppendBufferLen(m_strBlock,Packet,m_nTotalRead,dwRead);				// m_strBlock += Packet
																					// ��� ������ ���� ����Ÿ �߰� 
		m_nTotalRead+=dwRead;									// ó�� ���� ����
		if(m_nTotalRead == m_nBlockSize)						// ���� ����Ÿ�� ó�����̰� ���� ���� 
		{
			m_Queue.Add(m_strBlock,m_nBlockSize);
			::PostMessage(m_hWnd, WM_REC_MSG, 0, NULL);			// ������ Packet Data return

			m_bReadHeader = TRUE;								// ��� ���� �ʱ�ȭ 
		}
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Ÿ�̸� ���ν���		 						//
// [2]PARAMETER : void											//
// [3]RETURN :	void											//
// [4]DATE : 2001�� 12�� 10��									//
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

	// ����Ÿ�� �����ش� 
	if(uID == (UINT)(pThis->m_uTimerID_Check))
		if(nRet > 1000 * 4)	pThis->SendData(szCHK,sizeof(szCHK));
*/
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ����͸��� ��Ŷ �˸�						//
// [2]PARAMETER : void											//
// [3]RETURN :	void											//
// [4]DATE : 2001�� 12�� 10��									//
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
// ��ȣȭ ��� 
/////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CClientCtrl::Encrypto(char *szData, int nLen, char *szKey)
{
	if(nLen <= 0 || szData == NULL) return FALSE;
	if(szKey == NULL)
	{
		// NULL�϶�, Key ���� ������ش�. 
	}

	int nKeyLen = strlen(szKey);									// Ű�� ����
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
		// NULL�϶�, Key ���� ������ش�. 
	}

	int nKeyLen = strlen(szKey);									// Ű�� ����
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
// ���� �׽�Ʈ �Լ� 
/////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Send Thread�� ������ ���� �ӽ� �Լ� 		//
// [2]PARAMETER : void											//
// [3]RETURN :	void											//
// [4]DATE : 2000�� 9�� 11��									//
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
// [1]DESCRIPTION : Test�� ���� Send Thread						//
// [2]PARAMETER : lpVoid - �� Ŭ���� 							//
// [3]RETURN :	void											//
// [4]DATE : 2000�� 9�� 11��									//
//////////////////////////////////////////////////////////////////
unsigned CClientCtrl::ThreadSendProc(LPVOID lpVoid)
{
	CClientCtrl* pThis = reinterpret_cast<CClientCtrl*>(lpVoid);

	PostMessage(pThis->m_hWnd,WM_SEND_ON,NULL,NULL);			// ���� �����쿡 

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

	PostMessage(pThis->m_hWnd,WM_SEND_OFF,NULL,NULL);			// ���� �����쿡 �˸� 

	return 0; // Normal Thread Exit Code...
}
