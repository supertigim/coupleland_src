// ClientCtrl.h: interface for the CClientCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIENTCTRL_H__7F4F86BB_0725_4B4C_A973_0D6F16D597C3__INCLUDED_)
#define AFX_CLIENTCTRL_H__7F4F86BB_0725_4B4C_A973_0D6F16D597C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////
// 헤더 파일 모음 
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
//--------------------- 암호처리 내부 구조체 -------------------//
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
//------------------------ 멤 버 함 수 -------------------------//
public:
	CClientCtrl();												// 생성자 
	virtual ~CClientCtrl();										// 소멸자 

	//////////////////
	// 시작, 종료 함수 
	BOOL	Init(int nPort, HWND hWnd, char* szIP = NULL);// 초기화 
	void	Stop();												// 클라이언트 정지 
	void	SendPacketData(char *strData,int nSize);			// 데이타 전송 
	void	SendData(char *strData, int nSize);
	void	TestLoop(void);										// 테스트 스레드 구동

	void	SetParent(HWND hWnd);
	void	SendMessageToMonitor(char *pszData, int nSize, int nFlag);

protected:
	// Encryption-Decryption
	BOOL	Decrypto(char* szData, int nLen, char* szKey = NULL);
	BOOL	Encrypto(char* szData, int nLen, char* szKey = NULL);

	BOOL WinSockInit();											// Winsock Library Initialized

	//////////////////
	// MessageHandling
	BOOL OnWrite();												// On Write	  신호시
	BOOL OnConnect();											// On Connect 신호시	
	BOOL OnClose();												// On Close   신호시
	
	BOOL NetworkEventHanlder(long lEvent);						// 메세지 분기 함수 
	BOOL OnReadPacket();										// PACKET 형태로 데이타 읽기 => | 데이타 길이 | D A T A |
	BOOL OnReadData();											// 오직 데이타만 읽기 

	/////////////////
	// 내부 처리 함수 
	BOOL Connect();												// C-S 연결 
	BOOL GetIPByFile(void);										// IP를 파일로 얻게 한다 
	void DoAsyncSendBuff();										// 데이타 전송 

	////////////////////
	// static functions
	static char*			  AllocBuffer(char* pData);			// 데이타 할당
	static char*			  AllocBuffer(char* pData,int nLen);// NULL 문자에 상관없이 길이만큼 데이타 할당
	static unsigned CALLBACK  SocketThreadProc(LPVOID lParam);	// Main Thread 
	static void		CALLBACK  TimerProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);// Timer 
	static unsigned __stdcall ThreadSendProc(LPVOID lpVoid);

//------------------------ 멤 버 변 수 -------------------------//
protected:
	// 타이머 처리 
	long				m_uTimerID_Check;
	long				m_uTimerPeriod;

	BOOL				m_bConnect;								// 접속 상태 플래그 
	BOOL				m_bReadHeader;							// 현재 Receive가 처음인가 
	WSAEVENT			m_hEvent;								// 네트워크 이벤트 핸들러 
	HANDLE				m_hThread;								// 스레드 핸들 

	CRITICAL_SECTION	m_criticalSection;						// 크리티칼 섹션 변수 
	SOCKET				m_hSocket;								// 클라이언트 소켓 
	UINT				m_nPort;								// 포트 
	C8String			m_strIPAddr;							// Server IP저장 
	
	int					m_nBlockSize;							// 패킷 사이즈 
	int					m_nTotalRead;							// 천체 읽은양
	char				m_strBlock[MAXPACKETSIZE];				// 패킷 재구성 

	HWND				m_hWnd;									// 부모 윈도우 핸들 

	CBuffer				m_sendBuff;								// Send용 버퍼 

	INT					m_nSendDataLen;							// 보내는 데이타 길리 
	INT					m_nBytesSent;							// 보낸 데이타 길이 

	long				m_uSendTickCnt;
public:
	SQUEUE				m_Queue;

};

#endif // !defined(AFX_CLIENTCTRL_H__7F4F86BB_0725_4B4C_A973_0D6F16D597C3__INCLUDED_)
