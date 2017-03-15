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
	HANDLE						CreateIOCP();					// IOCP �ڵ� ���� 
	SOCKET						CreateListenSocket(int nServerPort);// Listen Socket ���� 
	BOOL						InitSocket();					// ���� ���̺귯�� Ȱ��
	int							GetNumberOfProcess();			// CPU���� ���Ѵ� => ������ ������ �̿� 
public:
	BOOL						Run();							// ���� ���� 
	BOOL						Stop();							// ���� ���� 

	static unsigned __stdcall	CALLBACK	Thread_Main(LPVOID lpVoid);// IOCP ���� ���� Thread
	static void __stdcall		CALLBACK	TimerProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

//----------------------- Member Variables ---------------------//
protected:
	HANDLE						m_hServerStopEvent;				// IOCP ���Ḧ ���� �ڵ�
};

#endif // !defined(AFX_SERVERCTRL_H__90D3D47F_6114_4C92_9D88_2B2E3633E709__INCLUDED_)
