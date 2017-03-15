// PacketPool.cpp: implementation of the CPacketPool class.
//
//////////////////////////////////////////////////////////////////////
#include "PacketPool.h"

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ������					 					//
// [2]PARAMETER :												//
// [3]RETURN :													//
// [4]DATE : 2000�� 9�� 4��										//
//////////////////////////////////////////////////////////////////
CPacketPool::CPacketPool()
{
	m_hLogFile = NULL;											// ���� �ڵ� �ʱ�ȭ 
	m_pProcPool = m_pAcptPool = m_pRecvPool = NULL;				// ���� �ʱ�ȭ 

	m_nMaxPacketSize = 0;

	// For Test By KIM
	m_nAcptCnt = 0;
	m_nProcCnt = 0;
	m_nRecvCnt = 0;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : �Ҹ���  - �޸� �Ҵ� ���� 					//
// [2]PARAMETER :												//
// [3]RETURN :													//
// [4]DATE : 2000�� 9�� 4��										//
//////////////////////////////////////////////////////////////////
CPacketPool::~CPacketPool()
{
	LogClose();													// �α� ���� �ڵ� �ݱ� 

	m_nMaxPacketSize = 0;

	// �����ϴ� Accep ��Ŷ���� �޸� ���� 
	if(m_pAcptPool)
		delete m_pAcptPool;

	// �����ϴ� Receive ��Ŷ���� �޸� ���� 
	if(m_pRecvPool)
		delete m_pRecvPool;

	if(m_pProcPool)
		delete m_pProcPool;

	m_pProcPool =m_pAcptPool = m_pRecvPool  = NULL;				// ���� �ʱ�ȭ 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : �ʱ�ȭ �Լ�									//
// [2]PARAMETER : lpszFileName - ���� �̸�, ������ NULL			//
// [3]RETURN :	TRUE - ����ó�� FALSE - ���� 					//
// [4]DATE : 2000�� 9�� 4��										//
//////////////////////////////////////////////////////////////////
BOOL CPacketPool::Create(int nMaxUser,char *lpszFileName,int nMaxPacketSize)
{
	// ������(�̸�)�� ���� �ϸ�..
	if(lpszFileName) LogOpen( lpszFileName );					// ���� ���� 

	m_nMaxPacketSize = nMaxPacketSize;

	// �� ��Ŷ�� �޸� �Ҵ� 
	m_pAcptPool = new CMemPool <OVERLAPPEDPLUS> (nMaxUser*1, m_nMaxPacketSize+64);//64�� ���� ����. �̰� å�� ���� ����.
	m_pRecvPool = new CMemPool <OVERLAPPEDPLUS> (nMaxUser*2, m_nMaxPacketSize+64);
	m_pProcPool = new CMemPool <OVERLAPPEDPLUS> (nMaxUser*2, m_nMaxPacketSize+64);

	// ���� ���� 
	LogWrite("acpt=%08X, recv=%08X, proc = %08X",
			m_pAcptPool, m_pRecvPool, m_pProcPool);

	// �ϳ��� �޸� �Ҵ��� �ȵǾ��ٸ� 
	if( !m_pAcptPool || !m_pRecvPool || !m_pProcPool)
		return FALSE;											// FALSE  

	return TRUE;												// TRUE  
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ���� �ڵ� ����								//
// [2]PARAMETER : lpszFileName - ���� �̸�						//
// [3]RETURN :	TRUE - ����ó�� FALSE - ���� 					//
// [4]DATE : 2000�� 9�� 4��										//
//////////////////////////////////////////////////////////////////
BOOL CPacketPool::LogOpen( char *lpszFileName )
{
	// �̸��� ������ �ʾҴٸ� 
	if( m_hLogFile || lpszFileName == NULL ) return FALSE;		// FALSE ���� 
	
	m_hLogFile = fopen( lpszFileName, "ab" );					// ó�� ���� ���� 

	// ���� �˻� 
	if( !m_hLogFile ) return FALSE;

	return TRUE;												// TRUE ���� - ���� ó�� 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ���Ͽ� �����Ͽ� ����						//
// [2]PARAMETER : lpszFileName - ���� �̸� ���ϱ� 				//
// [3]RETURN :	TRUE - ����ó�� FALSE - ���� 					//
// [4]DATE : 2000�� 9�� 4��										//
//////////////////////////////////////////////////////////////////
BOOL __cdecl CPacketPool::LogWrite( char *lpszFmt, ... )
{
	va_list		argptr;
	char		szOutStr[1024];
	
	if(NULL == m_hLogFile) return FALSE;
	
	va_start(argptr, lpszFmt);
	vsprintf(szOutStr, lpszFmt, argptr);
	va_end(argptr);
	
	int nBytesWritten = fprintf( m_hLogFile, "%s\r\n", szOutStr );// LOG ���� 
	fflush( m_hLogFile );

	return TRUE;
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ���µ� ���� �ݱ� 							//
// [2]PARAMETER : void											//
// [3]RETURN :	void											//
// [4]DATE : 2000�� 9�� 4��										//
//////////////////////////////////////////////////////////////////
void CPacketPool::LogClose()
{
	// ���� �ڵ��� ���� �Ѵٸ�,
	if(m_hLogFile) fclose( m_hLogFile );						// ���� �ڵ� �ݱ� 
	m_hLogFile = NULL;											// ���� �ڵ� �ʱ�ȭ 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Accept Packet �Ҵ� 							//
// [2]PARAMETER : newolp - ���� ���� 							//
// [3]RETURN :	TRUE - ����ó�� 								//
// [4]DATE : 2000�� 9�� 4��										//
//////////////////////////////////////////////////////////////////
BOOL CPacketPool::AllocAcptPacket( LPOVERLAPPEDPLUS &newolp )
{
	LPOVERLAPPEDPLUS olp = NULL;								// ����ü ��ü ���� 
	BOOL bRet = FALSE;

	olp = m_pAcptPool->Alloc();									// Accept Pool���� �Ҵ� ���� 

	LogWrite( "AlocAcptPacket(%08X)", (DWORD)olp );				// logging

	// �Ҵ��� ����� �Ǿ��ٸ� 
	if(olp)
	{
		// �Ҵ� �޸� ó�� �κ� 
		newolp = olp;											// ���� ���� 
		newolp->wbuf.buf = (char*)(olp+1);						// ���� ���� 
		bRet = TRUE;											// ��ȯ�� - TRUE�� �ٲ� 
	}

	// For Test By KIM
	InterlockedIncrement((LONG*)&m_nAcptCnt);

	return bRet;												// ��ȯ 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Receive Packet �Ҵ� 						//
// [2]PARAMETER : newolp - ���� ���� 							//
// [3]RETURN :	TRUE - ����ó�� 								//
// [4]DATE : 2000�� 9�� 4��										//
//////////////////////////////////////////////////////////////////
BOOL CPacketPool::AllocRecvPacket( LPOVERLAPPEDPLUS &newolp )
{
	LPOVERLAPPEDPLUS olp = NULL;								// ����ü ��ü ���� 
	BOOL bRet = FALSE;

	olp = m_pRecvPool->Alloc();									// Receive Pool���� �Ҵ� ����

	LogWrite( "AlocRecvPacket(%08X)", (DWORD)olp );				// logging

	// �Ҵ��� ����� �Ǿ��ٸ� 
	if(olp)
	{
		// �Ҵ� �޸� ó�� �κ�
		newolp = olp;											// ���� ���� 
		newolp->wbuf.buf = (char*)(olp+1);						// ���� ���� 
		bRet   = TRUE;											// ��ȯ�� - TRUE�� �ٲ�
	}

	// For Test By KIM
	InterlockedIncrement((LONG*)&m_nRecvCnt);

	return bRet;												// ��ȯ
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Process Packet �Ҵ� 						//
// [2]PARAMETER : newolp - ���� ���� 							//
// [3]RETURN :	TRUE - ����ó�� 								//
// [4]DATE : 2000�� 10�� 7��									//
//////////////////////////////////////////////////////////////////
BOOL CPacketPool::AllocProcPacket(LPOVERLAPPEDPLUS &newolp)
{
	LPOVERLAPPEDPLUS olp = NULL;								// ����ü ��ü ���� 
	BOOL bRet = FALSE;

	olp = m_pProcPool->Alloc();									// Accept Pool���� �Ҵ� ���� 

	LogWrite( "AlocAcptPacket(%08X)", (DWORD)olp );				// logging

	// �Ҵ��� ����� �Ǿ��ٸ� 
	if(olp)
	{
		// �Ҵ� �޸� ó�� �κ� 
		newolp = olp;											// ���� ���� 
		newolp->wbuf.buf = (char*)(olp+1);						// ���� ���� 
		bRet   = TRUE;											// ��ȯ�� - TRUE�� �ٲ� 
	}

	// For Test By KIM
	InterlockedIncrement((LONG*)&m_nProcCnt);

	return bRet;												// ��ȯ 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Accept �Ҵ� ��Ŷ ����  						//
// [2]PARAMETER : olp - ������ ��Ŷ 							//
// [3]RETURN :	TRUE - ���� ó�� 								//
// [4]DATE : 2000�� 9�� 4��										//
//////////////////////////////////////////////////////////////////
BOOL CPacketPool::FreeAcptPacket( LPOVERLAPPEDPLUS olp )
{
	LogWrite( "FreeAcptPacket(%08X)", (DWORD)olp );				// Logging

	/// For Test By KIM
	InterlockedDecrement((LONG*)&m_nAcptCnt);
	if(m_nAcptCnt < 0 ) m_nAcptCnt = 0;

	return(m_pAcptPool->Free(olp));								// ��Ŷ ���� �� ��� ��ȯ 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Receive �Ҵ� ��Ŷ ����  					//
// [2]PARAMETER : olp - ������ ��Ŷ 							//
// [3]RETURN :	TRUE - ���� ó�� 								//
// [4]DATE : 2000�� 9�� 4��										//
//////////////////////////////////////////////////////////////////
BOOL CPacketPool::FreeRecvPacket( LPOVERLAPPEDPLUS olp )
{
	LogWrite( "FreeRecvPacket(%08X)", (DWORD)olp );				// Logging

	// For Test By KIM
	InterlockedDecrement((LONG*)&m_nRecvCnt);
	if(m_nRecvCnt < 0 ) m_nRecvCnt = 0;

	return( m_pRecvPool->Free(olp) );							// ��Ŷ ���� �� ��� ��ȯ						
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Process �Ҵ� ��Ŷ ����  					//
// [2]PARAMETER : olp - ������ ��Ŷ 							//
// [3]RETURN :	TRUE - ���� ó�� 								//
// [4]DATE : 2000�� 9�� 4��										//
//////////////////////////////////////////////////////////////////
BOOL CPacketPool::FreeProcPacket(LPOVERLAPPEDPLUS olp)
{
	LogWrite( "FreeProcPacket(%08X)", (DWORD)olp );				// Logging

	// For Test By KIM
	InterlockedDecrement((LONG*)&m_nProcCnt);
	if(m_nProcCnt < 0 ) m_nProcCnt = 0;

	return(m_pProcPool->Free(olp));								// ��Ŷ ���� �� ��� ��ȯ
}