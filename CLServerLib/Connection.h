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
//------------------------ 멤 버 함 수 -------------------------//
public:
	CConnection();
	virtual ~CConnection();

	// 이 객체 생성 
	BOOL				Create(	DWORD			dwIndex,
								HANDLE			hIOCP,
								SOCKET			listener,
								CPacketPool*	pPacketPool,
								char			cConnType,
								int				nMaxBuf,
								int				nCheckPacketSize,
								int				nMaxUser);
protected:
	// Socket과 리스너 연결 및 스탠바이 상태 만듬  
	BOOL				Open();

	// 프로그램 종료시 완전 삭제 
	BOOL				Shutdown(LPOVERLAPPEDPLUS lpOverlapPlus, BOOL bForce=FALSE );

	// IOCP와 연결된 클라이언트 소켓과 바인딩 
	BOOL				BindIOCP( LPOVERLAPPEDPLUS lpOverlapPlus );

	/////////////////////////
	// 각 패킷 할당 해제 관련 
	LPOVERLAPPEDPLUS	PrepareAcptPacket();
	LPOVERLAPPEDPLUS	PrepareRecvPacket(UINT buflen);
	LPOVERLAPPEDPLUS	PrepareProcPacket(char *pbuf, UINT buflen);

	BOOL				ReleaseAcptPacket(LPOVERLAPPEDPLUS lpOverlapPlus);
	BOOL				ReleaseRecvPacket(LPOVERLAPPEDPLUS lpOverlapPlus);
	BOOL				ReleaseProcPacket(LPOVERLAPPEDPLUS lpOverlapPlus);

	// 실제 Receive 처리
	BOOL				RecvPost(UINT buflen=0);

	// 내부 처리 전송 함수 
	int					DispatchPacket( LPOVERLAPPEDPLUS lpOverlapPlus );

	// 실제 Process Data 처리 
	virtual void		ProcessReceivedData(LPOVERLAPPEDPLUS lpOverlapPlus);
	virtual void		ProcessReceivedData(char* psrcbuf, int srclen);

public:
	// 연결 강제 종료 
	void				Disconnect();							

	// 연결 끊고, 스탠바이 상태 전화 
	virtual BOOL		Close_Open(LPOVERLAPPEDPLUS lpOverlapPlus, BOOL bForce=FALSE );

	BOOL				DoIo(LPOVERLAPPEDPLUS lpOverlapPlus );	// IOCP 처리 핸들러 

	 
	virtual BOOL		SendPost( char *pbuf, UINT buflen );	// 실제 Send 처리

	// Send 스레드에서 데이타를 받아서 보내는 처리, 에코 서비스
	BOOL				DispatchPacket(char * psrcbuf,int srclen);

	BOOL				GetClientIP(char *szIP);				// 연결자의 IP얻기 
	BOOL				GetClientIP(sockaddr_in& addr);			// 연결자의 IP얻기

	DWORD				GetIndex();								// 연결개체의 INDEX 얻기 
	long				GetRecvTickCnt();						// 수신될때 체크되는 Tick 카운트 반환 

	BOOL				IsConnect();							// 연결된 객체인지 검사 
	int					GetTotalConectionCnt();					// 총 연결수 
	int					GetMaxUser();							// 최대 연결수 얻기 

//------------------------ 멤 버 변 수 -------------------------//
protected:
	int					m_nMaxBuf;								// 최대 버퍼 사이즈 
	int					m_nMaxPacketSize;						// 최대 패킷 사이즈
	int					m_nCheckPakcetSize;						// 채크 패킷 사이즈 
	int					m_nMaxUser;								// 서버 연결 최대수 

	SOCKET				m_Socket;								// 클라이언트 연결 소켓 
	SOCKET				m_sckListener;							// Listener 소켓 

	sockaddr_in			m_Local;								// 서버 정보를 가지고 있는 객체 
	sockaddr_in			m_Peer;									// 클라이언트 정보를 가지고 있는 객체 
	DWORD				m_dwIndex;								// 이 클라이언트 연결 객체의 번호 

	int					m_nBytesSent;							// 보낸 바이트 검사 변수 

	HANDLE				m_hIOCP;								// IOCP 핸들 	

	CPacketPool*		m_pPacketPool;							// 패킷풀 포인터 

	char				m_ConnType;								// 서버 type	
	BOOL				m_bIsConnect;							// 이 객체가 클라이언트와 연결 검사
	long				m_uRecvTickCnt;							// 수신 Tick Count 
	CRITICAL_SECTION	m_CS;	

	// checksum 처리 
	int					m_nRecvFlag;							// 수신 상태 표시 
	int					m_nRecvTotalSize;						// 데이타 총 길이 
	int					m_nRecvRead;							// 현재까지 읽은 값 
	char				m_szRcvData[DEF_MAXBUFSIZE];				

	static int			m_nConnCnt;								// 전체 연결 수 
};

#endif // !defined(AFX_CONNECTION_H__68994442_874A_4F6D_AC8A_944DC26A37F7__INCLUDED_)
