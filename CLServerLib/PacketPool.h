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
	// [����]
	BOOL Create(int nMaxUse			= DEF_MAXUSER, 
				char *lpszFileName	= NULL,
				int nMaxPacketSize	= DEF_MAXPACKETSIZE);		// Create Packet Pool

	// Operations
	BOOL AllocAcptPacket(LPOVERLAPPEDPLUS &newolp);				// Accept ��Ŷ ���� 
	BOOL AllocRecvPacket(LPOVERLAPPEDPLUS &newolp);				// Receive ��Ŷ ���� 
	BOOL AllocProcPacket(LPOVERLAPPEDPLUS &newolp);

	BOOL FreeAcptPacket(LPOVERLAPPEDPLUS olp);					// Accept ��Ŷ ���� 
	BOOL FreeRecvPacket(LPOVERLAPPEDPLUS olp);					// Receive ��Ŷ ���� 
	BOOL FreeProcPacket(LPOVERLAPPEDPLUS olp);

	// Implementation
protected:
	BOOL				LogOpen(char *lpszFileName);			// �α� ���� ���� 
	BOOL	__cdecl		LogWrite(char *lpszFmt, ...);			// �α� ���� ����� 
	void				LogClose();								// �α� ���� �ݱ� 

//---------------------- Member Variables ----------------------//
protected:
	FILE*				m_hLogFile;								// �α� ������ ����� ���� ���� �ڵ� ��ü 
	
	CMemPool			<OVERLAPPEDPLUS>* m_pAcptPool;			// Accept Packet
	CMemPool			<OVERLAPPEDPLUS>* m_pRecvPool;			// Receive Packet
	CMemPool			<OVERLAPPEDPLUS>* m_pProcPool;			// Process Packet

	int					m_nMaxPacketSize;						// ��Ŷ ������ ���Ѱ� 

// For Test By KIM
public:
	int m_nAcptCnt;												// Accept Packet Num In Use
	int m_nProcCnt;												// Process Packet Num In Use
	int m_nRecvCnt;												// Receive Packet Num In Use
};

#endif // !defined(AFX_PACKETPOOL_H__07F2012D_E779_42C7_B4FE_6A136CFFE9EA__INCLUDED_)
