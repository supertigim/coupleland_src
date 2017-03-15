// IOCP_Client_Win32.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
//#include "SocketClass.h"
#include "process.h"
#include "ClientCtrl.h"

#define MAX_LOADSTRING 100
#define WS_VERSION_REQD  0x0101
#define WS_VERSION_MAJOR HIBYTE(WS_VERSION_REQD)
#define WS_VERSION_MINOR LOBYTE(WS_VERSION_REQD)

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];								// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];								// The title bar text

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
/////////////////////////////////////////////////////////////////////////
void	CreateControl(HWND hWnd);		// Control �����츦 ����� 
void	Conn(HWND hWnd);				// Connect Button�� ������ �� �����ϴ� �Լ� 
void	ChangeReConn();					// ���� ���� 
void	ClickButtonSend(HWND hWnd);		// Send ��ư�� ������ �� 
void	SocketInit(HWND hWnd);
void	ClickButtonLoop(HWND hWnd);		// Click Loop Button
void	Disconnect();					// �������� 
void	Timer(HWND hWnd);

HWND	hBtnReConn;				// Connect ��ư ������ �ڵ� 
HWND	hBtnSend;				// ����Ÿ Send ��ư �ڵ� 
HWND	hBtnLoop;				// ����Ÿ Loop ��ư �ڵ� 
HWND	hBtnDisConn;			// ���� ���� 
HWND	hEditText;				// ����Ÿ Edit �ڽ� �ڵ� 
HWND	hEditStatus;			// ���� Ȯ�� â ���� �ڵ� 
HWND	hEditReceive;			// ����Ÿ Receive�� ���̴� ����Ʈ �ڽ� �ڵ� 

CClientCtrl		g_socket;


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_IOCP_CLIENT_WIN32, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_IOCP_CLIENT_WIN32);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_IOCP_CLIENT_WIN32);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_IOCP_CLIENT_WIN32;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 400, 225, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	wmId    = LOWORD(wParam); 
	wmEvent = HIWORD(wParam); 
	char* pString;

	switch (message) 
	{
		case WM_CONNECT:
			{
			::EnableWindow(hBtnReConn,FALSE);
			::EnableWindow(hBtnDisConn,TRUE);
			::EnableWindow(hBtnLoop,TRUE);
			::EnableWindow(hBtnSend,TRUE);

//			char szTest[256];
//			ZeroMemory(szTest,256);
//			wsprintf(szTest,"��������");
//			g_socket.SendData(szTest,strlen(szTest));

			break;
			}
		case WN_DISCONNECT:
			::EnableWindow(hBtnReConn,TRUE);
			::EnableWindow(hBtnDisConn,FALSE);
			::EnableWindow(hBtnLoop,FALSE);
			::EnableWindow(hBtnSend,FALSE);
			break;
		// ���� �޼����� ������
		case WM_SEND_ON:
			::EnableWindow(hBtnLoop,FALSE);
			break;
		case WM_SEND_OFF:
			::EnableWindow(hBtnLoop,TRUE);
			break;
		case WM_STATUS_MSG:
			pString = reinterpret_cast<char*>(lParam);
			::SetWindowText(hEditStatus,pString);
			delete [] pString;
			break;

		// RECEIVE�� �޼����� â�� ���� 
		case WM_REC_MSG:
			{
			char szTest[MAXBUFSIZE];
			g_socket.m_Queue.Get(szTest);
			::SetWindowText(hEditReceive,szTest);
			break;
			}
		// ������ �߻��ϴ� �Լ� 
		case WM_CREATE:
			CreateControl(hWnd);			// ��Ʈ�� ���� 
			SocketInit(hWnd);
			break;

		case WM_SHOWWINDOW:
			Conn(hWnd);
			break;

		case WM_COMMAND:
			// ��ư�� �̺�Ʈ�� �߻��ߴٸ� 
			if(lParam == (LONG)hBtnReConn)
			{
				switch(wmEvent)
				{
					case BN_CLICKED:
						Conn(hWnd);		// Click Connect ��ư 
						break;
				}
			}
			else if(lParam == (LONG)hBtnSend)
			{
				switch(wmEvent)
				{
					case BN_CLICKED:
						ClickButtonSend(hWnd);
						break;
				}
			}
			else if(lParam ==(LONG)hBtnLoop)
			{
				switch(wmEvent)
				{
					case BN_CLICKED:
						ClickButtonLoop(hWnd);
						break;
				}
			}
			else if(lParam ==(LONG)hBtnDisConn)
			{
				switch(wmEvent)
				{
					case BN_CLICKED:
						//ChangeReConn();
						Disconnect();
						break;
				}
			}

			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:
				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				   break;
				case IDM_EXIT:
				   DestroyWindow(hWnd);
				   break;
				case IDM_CHANGET_CON:
					ChangeReConn();
					break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}

			break;

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			RECT rt;
			GetClientRect(hWnd, &rt);

			TextOut(hdc,200,10,"[STATUS]",sizeof("[STATUS]"));
			TextOut(hdc,200,85,"[RECEIVE]",sizeof("[RECEIVE]"));
			EndPaint(hWnd, &ps);
			break;

		case WM_TIMER:
			Timer(hWnd);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}

// ��Ʈ�� �����츦 ����� �Լ� 
void	CreateControl(HWND hWnd)
{
	// Connect Button ������ ����
	hBtnReConn = CreateWindow("button",
							"CONNECT",
							WS_CHILD|WS_VISIBLE,
							110 + 10,
							10,
							70,
							20,
							hWnd,
							NULL,
							hInst,
							NULL);

	// Edit ������ ���� =>  IP
	hBtnDisConn	=	CreateWindow("button",
								"Disconnect",
								WS_CHILD|WS_VISIBLE|WS_BORDER,
								10,
								10,
								100,
								20,
								hWnd,
								NULL,
								hInst,
								NULL);

	// Send Button ������ ����
	hBtnSend = CreateWindow("button",
							"Send",
							WS_CHILD|WS_VISIBLE,
							10,
							150,
							180,
							20,
							hWnd,
							NULL,
							hInst,
							NULL);

	// Loop Button ������ ����
	hBtnLoop = CreateWindow("button",
							"Loop",
							WS_CHILD|WS_VISIBLE,
							270,
							10,
							100,
							20,
							hWnd,
							NULL,
							hInst,
							NULL);

	// Edit ������ ���� => ���� ���� ���� 
	hEditText	=	CreateWindow("edit",
									"",
								WS_CHILD|WS_VISIBLE|WS_BORDER|ES_MULTILINE,
								10,
								40,
								180,
								100,
								hWnd,
								NULL,
								hInst,
								NULL);

	// Edit ������ ���� => ���� Ȯ�� 
	hEditStatus	=	CreateWindow("edit",
								"",
								WS_CHILD|WS_VISIBLE|WS_BORDER|ES_MULTILINE|ES_AUTOVSCROLL,
								200,
								40,
								180,
								40,
								hWnd,
								NULL,
								hInst,
								NULL);

	// Edit ������ ���� => ���� Ȯ�� 
	hEditReceive	=	CreateWindow("edit",
									"",
									WS_CHILD|WS_VISIBLE|WS_BORDER|ES_MULTILINE|WS_VSCROLL,
									200,
									110,
									180,
									60,
									hWnd,
									NULL,
									hInst,
									NULL);
}

void	Conn(HWND hWnd)
{
	g_socket.SetParent(hWnd);
	if(!g_socket.Init(999, hWnd))
//	if(!m_socket.Init(54024, hWnd))
	{
	
		MessageBox(hWnd,"����","�˸�",MB_OK);
	}
	
}

void ClickButtonSend(HWND hWnd)
{
//*
	char szText[100];
	GetWindowText(hEditText,szText,sizeof(szText));
//	CString str  = szText;
//	str.TrimRight();

	if(szText[0] != NULL) g_socket.SendData(szText,strlen(szText));
//*/
//	g_socket.NewConnect(999,"10.0.0.22");

}
//���� ������ ������ ��ư ������ �� 
void ClickButtonLoop(HWND hWnd)
{
	g_socket.TestLoop();
//	::SetTimer(hWnd,0,500,NULL);
}

void SocketInit(HWND hWnd)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );
 
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		WSACleanup();
		// Tell the user that we could not find a usable 
		// WinSock DLL.                                  
		return;
	}
 
	// Confirm that the WinSock DLL supports 2.2. //
	// Note that if the DLL supports versions greater    //
	// than 2.2 in addition to 2.2, it will still return //
	// 2.2 in wVersion since that is the version we      //
	// requested.                                        //

	if ( LOBYTE( wsaData.wVersion ) != 2 ||
			HIBYTE( wsaData.wVersion ) != 2 ) {
		// Tell the user that we could not find a usable //
		// WinSock DLL.   //
		WSACleanup( );
		return; 
	}
}

void Disconnect()
{
	g_socket.Stop();
}

void ChangeReConn()
{

}

void Timer(HWND hWnd)
{
	static long nTime = 0;
	if(nTime > 1000 * 60 * 14)
	{
		::KillTimer(hWnd,0);
		nTime = 0;
		return;
	}
	char szTemp[200];
	memset(szTemp,NULL,200);
	wsprintf(szTemp,"Msg:%d",(int)nTime);
	g_socket.SendData(szTemp,strlen(szTemp));

	nTime ++;
}
