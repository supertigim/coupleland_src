// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers


// Windows Header Files:
//#include <windows.h>
#include<afxwin.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include<assert.h>
#include<process.h>
#include<winsock2.h>											// WINDOWS SOCKET ���� ��� ���� 
#pragma comment(lib,"ws2_32.lib")								// SOCKET ���� ���̺귯�� �ε� 
#include "8String.h"
#include "7String.h"
// [1] ����ũ��
#define		MAXBUFSIZE				1024						// Mem Pool ���� ������ 
#define		MAXPACKETSIZE			MAXBUFSIZE					// ��Ŷ ���� ������ 
#define		MAX_BUFF_SIZE			MAXPACKETSIZE
/////////////////////
// ������ �޽��� ���� 
#define WM_STATUS_MSG	WM_APP + 0x1001
#define WM_REC_MSG		WM_APP + 0x1002
#define WM_SEND_ON		WM_APP + 0x1003
#define WM_SEND_OFF		WM_APP + 0x1004
#define WM_CONNECT		WM_APP + 0x1005
#define WN_DISCONNECT	WM_APP + 0x1006


// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
