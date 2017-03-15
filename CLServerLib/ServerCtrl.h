// ServerCtrl.h: interface for the CServerCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVERCTRL_H__90D3D47F_6114_4C92_9D88_2B2E3633E709__INCLUDED_)
#define AFX_SERVERCTRL_H__90D3D47F_6114_4C92_9D88_2B2E3633E709__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Connection Class 
#include "Connection.h"

class CServerCtrl  
{
//----------------------- Member Functions ---------------------//
public:
	CServerCtrl();
	virtual ~CServerCtrl();
private:
	HANDLE						CreateIOCP();					// IOCP 핸들 생성 
	SOCKET						CreateListenSocket(int nServerPort);// Listen Socket 생성 
	BOOL						InitSocket();					// 소켓 라이브러리 활성
	int							GetNumberOfProcess();			// CPU수를 구한다 => 스레드 구동시 이용 
public:
	BOOL						Run();							// 서버 구동 
	BOOL						Stop();							// 서버 정지 

	static unsigned __stdcall	CALLBACK	Thread_Main(LPVOID lpVoid);// IOCP 구동 메인 Thread
	static void __stdcall		CALLBACK	TimerProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

//----------------------- Member Variables ---------------------//
protected:
	HANDLE						m_hServerStopEvent;				// IOCP 종료를 위한 핸들
};

#endif // !defined(AFX_SERVERCTRL_H__90D3D47F_6114_4C92_9D88_2B2E3633E709__INCLUDED_)
