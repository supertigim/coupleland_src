// Connection.h: interface for the CConnection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONNECTION_H__68994442_874A_4F6D_AC8A_944DC26A37F7__INCLUDED_)
#define AFX_CONNECTION_H__68994442_874A_4F6D_AC8A_944DC26A37F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PacketPool.h"

class CConnection  
{
//------------------------ �� �� �� �� -------------------------//
public:
	CConnection();
	virtual ~CConnection();

	// �� ��ü ���� 
	BOOL				Create(	DWORD			dwIndex,
								HANDLE			hIOCP,
								SOCKET			listener,
								CPacketPool*	pPacketPool,
								char			cConnType,
								int				nMaxBuf,
								int				nCheckPacketSize,
								int				nMaxUser);
protected:
	// Socket�� ������ ���� �� ���Ĺ��� ���� ����  
	BOOL				Open();

	// ���α׷� ����� ���� ���� 
	BOOL				Shutdown(LPOVERLAPPEDPLUS lpOverlapPlus, BOOL bForce=FALSE );

	// IOCP�� ����� Ŭ���̾�Ʈ ���ϰ� ���ε� 
	BOOL				BindIOCP( LPOVERLAPPEDPLUS lpOverlapPlus );

	/////////////////////////
	// �� ��Ŷ �Ҵ� ���� ���� 
	LPOVERLAPPEDPLUS	PrepareAcptPacket();
	LPOVERLAPPEDPLUS	PrepareRecvPacket(UINT buflen);
	LPOVERLAPPEDPLUS	PrepareProcPacket(char *pbuf, UINT buflen);

	BOOL				ReleaseAcptPacket(LPOVERLAPPEDPLUS lpOverlapPlus);
	BOOL				ReleaseRecvPacket(LPOVERLAPPEDPLUS lpOverlapPlus);
	BOOL				ReleaseProcPacket(LPOVERLAPPEDPLUS lpOverlapPlus);

	// ���� Receive ó��
	BOOL				RecvPost(UINT buflen=0);

	// ���� ó�� ���� �Լ� 
	int					DispatchPacket( LPOVERLAPPEDPLUS lpOverlapPlus );

	// ���� Process Data ó�� 
	virtual void		ProcessReceivedData(LPOVERLAPPEDPLUS lpOverlapPlus);
	virtual void		ProcessReceivedData(char* psrcbuf, int srclen);

public:
	// ���� ���� ���� 
	void				Disconnect();							

	// ���� ����, ���Ĺ��� ���� ��ȭ 
	virtual BOOL		Close_Open(LPOVERLAPPEDPLUS lpOverlapPlus, BOOL bForce=FALSE );

	BOOL				DoIo(LPOVERLAPPEDPLUS lpOverlapPlus );	// IOCP ó�� �ڵ鷯 

	 
	virtual BOOL		SendPost( char *pbuf, UINT buflen );	// ���� Send ó��

	// Send �����忡�� ����Ÿ�� �޾Ƽ� ������ ó��, ���� ����
	BOOL				DispatchPacket(char * psrcbuf,int srclen);

	BOOL				GetClientIP(char *szIP);				// �������� IP��� 
	BOOL				GetClientIP(sockaddr_in& addr);			// �������� IP���

	DWORD				GetIndex();								// ���ᰳü�� INDEX ��� 
	long				GetRecvTickCnt();						// ���ŵɶ� üũ�Ǵ� Tick ī��Ʈ ��ȯ 

	BOOL				IsConnect();							// ����� ��ü���� �˻� 
	int					GetTotalConectionCnt();					// �� ����� 
	int					GetMaxUser();							// �ִ� ����� ��� 

//------------------------ �� �� �� �� -------------------------//
protected:
	int					m_nMaxBuf;								// �ִ� ���� ������ 
	int					m_nMaxPacketSize;						// �ִ� ��Ŷ ������
	int					m_nCheckPakcetSize;						// äũ ��Ŷ ������ 
	int					m_nMaxUser;								// ���� ���� �ִ�� 

	SOCKET				m_Socket;								// Ŭ���̾�Ʈ ���� ���� 
	SOCKET				m_sckListener;							// Listener ���� 

	sockaddr_in			m_Local;								// ���� ������ ������ �ִ� ��ü 
	sockaddr_in			m_Peer;									// Ŭ���̾�Ʈ ������ ������ �ִ� ��ü 
	DWORD				m_dwIndex;								// �� Ŭ���̾�Ʈ ���� ��ü�� ��ȣ 

	int					m_nBytesSent;							// ���� ����Ʈ �˻� ���� 

	HANDLE				m_hIOCP;								// IOCP �ڵ� 	

	CPacketPool*		m_pPacketPool;							// ��ŶǮ ������ 

	char				m_ConnType;								// ���� type	
	BOOL				m_bIsConnect;							// �� ��ü�� Ŭ���̾�Ʈ�� ���� �˻�
	long				m_uRecvTickCnt;							// ���� Tick Count 
	CRITICAL_SECTION	m_CS;	

	// checksum ó�� 
	int					m_nRecvFlag;							// ���� ���� ǥ�� 
	int					m_nRecvTotalSize;						// ����Ÿ �� ���� 
	int					m_nRecvRead;							// ������� ���� �� 
	char				m_szRcvData[DEF_MAXBUFSIZE];				

	static int			m_nConnCnt;								// ��ü ���� �� 
};

#endif // !defined(AFX_CONNECTION_H__68994442_874A_4F6D_AC8A_944DC26A37F7__INCLUDED_)
