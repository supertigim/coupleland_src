// ClientCtrl.h: interface for the CClientCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIENTCTRL_H__7F4F86BB_0725_4B4C_A973_0D6F16D597C3__INCLUDED_)
#define AFX_CLIENTCTRL_H__7F4F86BB_0725_4B4C_A973_0D6F16D597C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////
// ��� ���� ���� 
#include "Buffer.h"	

#include "mmsystem.h"
#pragma comment(lib,"winmm.lib")

#define MAXQUEUESIZE 2048

//////////////////////////////////////////////////////////////////
// Simple Queue Structure
//////////////////////////////////////////////////////////////////
typedef struct _SIMPLE_QUEUE_
{
	struct INDEX
	{
		INDEX*	pNext;
		int		nLen;
		char	szBuf[MAXBUFSIZE];
		INDEX()
		{
			pNext = NULL;
			nLen = 0;
			ZeroMemory(szBuf,sizeof(szBuf));
		}
	};
protected:
	INDEX* pList;
	INDEX* pHead;
	INDEX* pTail;
	int		nQueueCnt;
	int		nMaxQueueCnt;
	CRITICAL_SECTION cs;
public:
	_SIMPLE_QUEUE_()
	{
		pList = NULL;
		pHead = NULL;
		pTail = NULL;
		nQueueCnt = 0;
		nMaxQueueCnt = 0;
	}

	~_SIMPLE_QUEUE_()
	{
		try
		{
			DeleteCriticalSection(&cs);
		}
		catch(...) {}
	}

	BOOL Create(int nMAXCnt = 10)
	{
		if(nMAXCnt <= 0) return FALSE;
		nMaxQueueCnt = nMAXCnt;

		if((pList = new INDEX[nMaxQueueCnt]) == NULL)
		{
			nMaxQueueCnt = 0;
			return FALSE;
		}

		for(int i = nMaxQueueCnt - 1; i >= 0 ; i--)
		{
			if( (i+1) == nMaxQueueCnt)
			{
				pList[i].pNext = &pList[0];
				continue;
			}
			pList[i].pNext = &pList[i+1];		
		}
		pHead = pTail = &pList[0];
		InitializeCriticalSection(&cs);
		return TRUE;
	}

	void Reset()
	{
		EnterCriticalSection(&cs);
		pHead = pTail = &pList[0];
		nQueueCnt = 0;
		LeaveCriticalSection(&cs);
	}

	BOOL Add(char* szData = NULL, int nSize = 0)
	{
		if(!pList) return FALSE;
		if(nQueueCnt == nMaxQueueCnt)
		{
			nQueueCnt = 0;
			pHead = pTail = &pList[0];
			return FALSE;
		}
		if(!szData) return FALSE;
		if(szData[0] == NULL) return FALSE;
		if(nSize <= 0) return FALSE;
		if(nSize >= MAXBUFSIZE) return FALSE;

		EnterCriticalSection(&cs);

		pTail->nLen = nSize;

		CopyMemory(pTail->szBuf,szData,pTail->nLen);
		pTail->szBuf[pTail->nLen] = NULL;
		pTail = pTail->pNext;

		InterlockedIncrement((LONG*)&nQueueCnt);

		LeaveCriticalSection(&cs);

		return TRUE;
	}

	int Get(char* szData = NULL)
	{
		if(!pList) return -1;
		int nLen = -1;
		if(nQueueCnt == 0) return nLen;
		if(szData == NULL) return nLen;

		EnterCriticalSection(&cs);

		CopyMemory(szData,pHead->szBuf,pHead->nLen+1);
		pHead->szBuf[0] = NULL;
		szData[pHead->nLen] = NULL;

		nLen = pHead->nLen;
		pHead->nLen = 0;

		pHead = pHead->pNext;

		InterlockedDecrement((LONG*)&nQueueCnt);

		LeaveCriticalSection(&cs);
		return nLen;
	}

} SQUEUE;

//////////////////////////////////////////////////////////////////
// Client Socket Control Class
//////////////////////////////////////////////////////////////////
class CClientCtrl  
{
//--------------------- ��ȣó�� ���� ����ü -------------------//
protected:
	struct BIT
	{
		char bit1 : 1;
		char bit2 : 1;
		char bit3 : 1;
		char bit4 : 1;
		char bit5 : 1;
		char bit6 : 1;
		char bit7 : 1;
		char bit8 : 1;
	};

	union MyDATA
	{
		BIT		bit;
		char	Data;
	};
//------------------------ �� �� �� �� -------------------------//
public:
	CClientCtrl();												// ������ 
	virtual ~CClientCtrl();										// �Ҹ��� 

	//////////////////
	// ����, ���� �Լ� 
	BOOL	Init(int nPort, HWND hWnd, char* szIP = NULL);// �ʱ�ȭ 
	void	Stop();												// Ŭ���̾�Ʈ ���� 
	void	SendPacketData(char *strData,int nSize);			// ����Ÿ ���� 
	void	SendData(char *strData, int nSize);
	void	TestLoop(void);										// �׽�Ʈ ������ ����

	void	SetParent(HWND hWnd);
	void	SendMessageToMonitor(char *pszData, int nSize, int nFlag);

protected:
	// Encryption-Decryption
	BOOL	Decrypto(char* szData, int nLen, char* szKey = NULL);
	BOOL	Encrypto(char* szData, int nLen, char* szKey = NULL);

	BOOL WinSockInit();											// Winsock Library Initialized

	//////////////////
	// MessageHandling
	BOOL OnWrite();												// On Write	  ��ȣ��
	BOOL OnConnect();											// On Connect ��ȣ��	
	BOOL OnClose();												// On Close   ��ȣ��
	
	BOOL NetworkEventHanlder(long lEvent);						// �޼��� �б� �Լ� 
	BOOL OnReadPacket();										// PACKET ���·� ����Ÿ �б� => | ����Ÿ ���� | D A T A |
	BOOL OnReadData();											// ���� ����Ÿ�� �б� 

	/////////////////
	// ���� ó�� �Լ� 
	BOOL Connect();												// C-S ���� 
	BOOL GetIPByFile(void);										// IP�� ���Ϸ� ��� �Ѵ� 
	void DoAsyncSendBuff();										// ����Ÿ ���� 

	////////////////////
	// static functions
	static char*			  AllocBuffer(char* pData);			// ����Ÿ �Ҵ�
	static char*			  AllocBuffer(char* pData,int nLen);// NULL ���ڿ� ������� ���̸�ŭ ����Ÿ �Ҵ�
	static unsigned CALLBACK  SocketThreadProc(LPVOID lParam);	// Main Thread 
	static void		CALLBACK  TimerProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);// Timer 
	static unsigned __stdcall ThreadSendProc(LPVOID lpVoid);

//------------------------ �� �� �� �� -------------------------//
protected:
	// Ÿ�̸� ó�� 
	long				m_uTimerID_Check;
	long				m_uTimerPeriod;

	BOOL				m_bConnect;								// ���� ���� �÷��� 
	BOOL				m_bReadHeader;							// ���� Receive�� ó���ΰ� 
	WSAEVENT			m_hEvent;								// ��Ʈ��ũ �̺�Ʈ �ڵ鷯 
	HANDLE				m_hThread;								// ������ �ڵ� 

	CRITICAL_SECTION	m_criticalSection;						// ũ��ƼĮ ���� ���� 
	SOCKET				m_hSocket;								// Ŭ���̾�Ʈ ���� 
	UINT				m_nPort;								// ��Ʈ 
	C8String			m_strIPAddr;							// Server IP���� 
	
	int					m_nBlockSize;							// ��Ŷ ������ 
	int					m_nTotalRead;							// õü ������
	char				m_strBlock[MAXPACKETSIZE];				// ��Ŷ �籸�� 

	HWND				m_hWnd;									// �θ� ������ �ڵ� 

	CBuffer				m_sendBuff;								// Send�� ���� 

	INT					m_nSendDataLen;							// ������ ����Ÿ �渮 
	INT					m_nBytesSent;							// ���� ����Ÿ ���� 

	long				m_uSendTickCnt;
public:
	SQUEUE				m_Queue;

};

#endif // !defined(AFX_CLIENTCTRL_H__7F4F86BB_0725_4B4C_A973_0D6F16D597C3__INCLUDED_)
