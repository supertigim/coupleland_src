// PacketPool.h: interface for the CPacketPool class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PACKETPOOL_H__07F2012D_E779_42C7_B4FE_6A136CFFE9EA__INCLUDED_)
#define AFX_PACKETPOOL_H__07F2012D_E779_42C7_B4FE_6A136CFFE9EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MemPool.h"

class CPacketPool  
{
//---------------------- Member Functions ----------------------//
public:
	CPacketPool();												// Contructor 
	virtual ~CPacketPool();										// Destructor 

	///////////////////////////
	// [정윤]
	BOOL Create(int nMaxUse			= DEF_MAXUSER, 
				char *lpszFileName	= NULL,
				int nMaxPacketSize	= DEF_MAXPACKETSIZE);		// Create Packet Pool

	// Operations
	BOOL AllocAcptPacket(LPOVERLAPPEDPLUS &newolp);				// Accept 패킷 생성 
	BOOL AllocRecvPacket(LPOVERLAPPEDPLUS &newolp);				// Receive 패킷 생성 
	BOOL AllocProcPacket(LPOVERLAPPEDPLUS &newolp);

	BOOL FreeAcptPacket(LPOVERLAPPEDPLUS olp);					// Accept 패킷 해제 
	BOOL FreeRecvPacket(LPOVERLAPPEDPLUS olp);					// Receive 패킷 해제 
	BOOL FreeProcPacket(LPOVERLAPPEDPLUS olp);

	// Implementation
protected:
	BOOL				LogOpen(char *lpszFileName);			// 로그 파일 열기 
	BOOL	__cdecl		LogWrite(char *lpszFmt, ...);			// 로그 파일 만들기 
	void				LogClose();								// 로그 파일 닫기 

//---------------------- Member Variables ----------------------//
protected:
	FILE*				m_hLogFile;								// 로그 파일을 만들기 위한 파일 핸들 객체 
	
	CMemPool			<OVERLAPPEDPLUS>* m_pAcptPool;			// Accept Packet
	CMemPool			<OVERLAPPEDPLUS>* m_pRecvPool;			// Receive Packet
	CMemPool			<OVERLAPPEDPLUS>* m_pProcPool;			// Process Packet

	int					m_nMaxPacketSize;						// 패킷 사이즈 정한것 

// For Test By KIM
public:
	int m_nAcptCnt;												// Accept Packet Num In Use
	int m_nProcCnt;												// Process Packet Num In Use
	int m_nRecvCnt;												// Receive Packet Num In Use
};

#endif // !defined(AFX_PACKETPOOL_H__07F2012D_E779_42C7_B4FE_6A136CFFE9EA__INCLUDED_)
