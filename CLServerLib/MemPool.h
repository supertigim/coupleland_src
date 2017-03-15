#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

// 헤더 파일 
#include "CLGlobal.h"												// 전체 처리 함수

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Template Class 
//////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(push, struct_packing_before)
#pragma pack(1)													//메모리를 바이트 단위로 정렬한다.

template <class TYPE>
class CMemPool
{
protected:
	// INDEX Structure  
	struct INDEX
    {
		INDEX* pNext;											// 다음 Index 포인터를 가진다 
		int nIndex;												// 현재 인덱스 번호 

		// Structure constructure
		INDEX(){
			pNext = NULL;									
			nIndex = 0;
		}
    }; 
//---------------------- Member Functions ----------------------//
public:
	// Construction & Destruction
    CMemPool(int nTotalCnt, int nAllocBufferSize=0);			// 할당 받을 갯수와 버퍼 사이즈 입력 
	~CMemPool();												// 할당 해제 처리 

	// Attributes
    int GetCount();												// 현재 할당 받은 수 
    int GetEmpty();												// 비어 있는 패킷 수 
    int GetIndex( TYPE *ptr );									// 버퍼에 따른 인덱스 얻기 
	
	// Operations
    TYPE* Alloc();												// TYPE으로 메모리 할당 
    BOOL Free( TYPE *ptr );										// 할당 받은 메모리를 해제 

//---------------------- Member Variables ----------------------//
protected:
    int		m_nCount;											// 현재 할당 받은 수 											
    int		m_nTotalCnt;										// 만들어진 할당 패킷 수 
    int		m_nAllocBufferSize;									// 한 패킷당 할당 받은 버퍼 사이즈 
    int		m_nPerPacketSize;									// 한 패킷당 전체 사이즈 

    PVOID	m_pPakets;											// 전체 할당 포인터 
    INDEX*	m_pFreeRoom;										// 현재 비어있는 처리 인덱스 포인터 

    CRITICAL_SECTION m_cs;										// 크리티칼 섹션 변수 
};
#pragma pack( pop, struct_packing_before )
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 생성자 - 메모리 할당 루틴 					//
// [2]PARAMETER : nTotalCnt - 할당할 패킷의 수					//
//				  nAllockBufferSize - 할당할 버퍼 사이즈		//
// [3]RETURN :													//
// [4]DATE : 2000년 9월 4일										//
//////////////////////////////////////////////////////////////////
template <class TYPE>
CMemPool<TYPE>::CMemPool<TYPE>( int nTotalCnt, int nAllocBufferSize )
{
	assert( nTotalCnt > 0 );

    m_nCount	= 0;
    m_pFreeRoom = NULL;
    m_pPakets	= NULL;

	m_nTotalCnt			= nTotalCnt;							// MAXUSER * 2 
    m_nAllocBufferSize	= nAllocBufferSize;						// 1024 + 64
    m_nPerPacketSize	= sizeof(INDEX) + sizeof(TYPE) + m_nAllocBufferSize;

	m_pPakets = VirtualAlloc( NULL, 
							  m_nTotalCnt * m_nPerPacketSize,
							  MEM_RESERVE | MEM_COMMIT,         // reserve and commit
							  PAGE_READWRITE );

		////////////////////////////////////////////////////////// 
		//				< 할당 데이타 블럭도 >					// 
		//													    // 
		//	-----------------------------------------------     // 
		//  |    INDEX   |   TYPE  |    m_nExBlockSize     |    // 
		//  -----------------------------------------------     // 
		////////////////////////////////////////////////////////// 	

	assert( m_pPakets );
    INDEX* pIndex = (INDEX*) m_pPakets;
	assert( pIndex );

	//////////////////////////////////////////
    // init linear linked list for buffer pool

	// 할당 데이타 구조 맨 마지막 단위의 포인터 얻기 
    pIndex = (INDEX*) ((DWORD)pIndex + (m_nTotalCnt - 1) * m_nPerPacketSize);

	// 할당 데이타 구조의 마지막 부터 Linked List를 구성하여 올라오기 
    for( int i = m_nTotalCnt-1; i >= 0; i-- )
    {
        pIndex->pNext = m_pFreeRoom;								// 맨 마지막의 pNext = NULL이다 
        pIndex->nIndex = i;											// INDEX 작업 

        m_pFreeRoom = pIndex;										// 다음 연결을 위한 세팅 

#ifdef _DEBUG
		// 각 블럭 마다 ExBlockSize의 부분에 데이타 세팅 작업 
        if( m_nAllocBufferSize )
            memset((PVOID)( (DWORD)pIndex+sizeof(INDEX)+sizeof(TYPE)),
							(i%10+'0'), 
							m_nAllocBufferSize );
#endif
		// 다음 리스트로 C8List 포인트 이동 
        pIndex = (INDEX*)((DWORD)pIndex - m_nPerPacketSize);
    }

	InitializeCriticalSectionAndSpinCount( &m_cs, 4000 );   // why 4000?
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 소멸자  - 메모리 할당 해제 					//
// [2]PARAMETER :												//
// [3]RETURN :													//
// [4]DATE : 2000년 9월 4일										//
//////////////////////////////////////////////////////////////////
template <class TYPE>
CMemPool<TYPE>::~CMemPool<TYPE>()
{
    if(NULL != m_pPakets) VirtualFree( m_pPakets, 0, MEM_RELEASE );
    DeleteCriticalSection( &m_cs );								// 크리티칼 섹션 변수 해제 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 할당된 패킷중 빈 패킷 반환					//
// [2]PARAMETER :												//
// [3]RETURN : 템플릿 타입 반환 								//
// [4]DATE : 2000년 9월 4일										//
//////////////////////////////////////////////////////////////////
template <class TYPE>
TYPE* CMemPool<TYPE>::Alloc()
{
	// 변수 초기화 
    INDEX* pIndex = NULL;							
    TYPE* pType = NULL;

    EnterCriticalSection( &m_cs );								// 크리티칼 섹션 시작 

    pIndex = m_pFreeRoom;
    if( pIndex != NULL )
    {
		// 빈공간 이동 인덱스 포인터를 다음 인덱스로 이동 
        m_pFreeRoom = m_pFreeRoom->pNext;						
        m_nCount++;												// 카운트 증가 
		// 지정한 인덱스 포인터에서 템플릿 타입 위티 포인터 반환 
        pType = (TYPE*)(pIndex+1);								
		
		///////////////////////////////////////////
		// 치명적인 에러 발생 
		// assert( m_nCount > 0 );									// make sure we don't overflow
		if(m_nCount <= 0)
		{
			LogError("CMemPool::Alloc()에서 m_nCount에서 이상 발생");
			LeaveCriticalSection(&m_cs);
			return NULL;
		}

    }
    LeaveCriticalSection( &m_cs );								// 크리티칼 섹션 끝 
    return pType;												// 템플릿 타입 반환 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 할당된 패킷중 사용중인 패킷 삭제			//
// [2]PARAMETER : ptr - 템플릿 타입 포인터 						//
// [3]RETURN : TRUE - 정상, FALSE - 실패 						//
// [4]DATE : 2000년 9월 4일										//
//////////////////////////////////////////////////////////////////
template <class TYPE>
BOOL CMemPool<TYPE>::Free( TYPE *ptr )
{
    BOOL bRet = FALSE;

    EnterCriticalSection( &m_cs );								// 크리티칼 섹션 진입 

    INDEX* pIndex = ((INDEX*)(ptr))-1;							// 사용 패킷의 인덱스 포인터 얻기 

	// 사용중인 패킷이 있다면 
    if( m_nCount > 0 )
    {
		// 빈공간 인덱스 포인터를 앞으로 당기는 루틴 
        pIndex->pNext = m_pFreeRoom;							
        m_pFreeRoom = pIndex;
        m_nCount--;												// 사용 패킷수 1 감소 
        bRet = TRUE;											// 정상 처리 알림 
    }

    LeaveCriticalSection( &m_cs );								// 그리티칼 섹션 탈출 
    return bRet;												// 정상 처리 유무 반환 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 사용중인 패킷 수 얻기						//
// [2]PARAMETER : void											//
// [3]RETURN : int - 사용중인 패킷 수 반환 						//
// [4]DATE : 2000년 9월 4일										//
//////////////////////////////////////////////////////////////////
template <class TYPE>
int CMemPool<TYPE>::GetCount()
{
    int nRet = 0;												// 변수 초기화 
    EnterCriticalSection( &m_cs );								// 크리티칼 섹션 진입
    nRet = m_nCount;											// 변수 설정 
    LeaveCriticalSection( &m_cs );								// 크리티칼 섹션 해제 
    return nRet;												// 반환 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 사용안하고 잇는 패킷 수 얻기				//
// [2]PARAMETER : void					 						//
// [3]RETURN : int - 사용 안하는 패킷 수 반환					//
// [4]DATE : 2000년 9월 4일										//
//////////////////////////////////////////////////////////////////
template <class TYPE>
int CMemPool<TYPE>::GetEmpty()
{
    int nRet = 0;												// 변수 초기화 
    EnterCriticalSection( &m_cs );								// 크리티칼 섹션 진입 
	nRet = m_nTotalCnt - m_nCount;								// 설정 
    LeaveCriticalSection( &m_cs );								// 크리티칼 섹션 해제 
    return nRet;												// 반환 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 사용중인 패킷의 INDEX얻기					//
// [2]PARAMETER : ptr - 템플릿 타입 포인터 						//
// [3]RETURN : intE - 사용중인 패킷의 인덱스 반환				//
// [4]DATE : 2000년 9월 4일										//
//////////////////////////////////////////////////////////////////
template <class TYPE>
int CMemPool<TYPE>::GetIndex( TYPE *ptr )
{
    int nRet = 0;												// 변수 초기화 
    EnterCriticalSection( &m_cs );								// 크리티칼 섹션 진입
    INDEX* pIndex = ((INDEX*)(ptr))-1;							// 인데스 포인터 얻기 
    nRet = pIndex->nIndex;										// 변수 설정 
    LeaveCriticalSection( &m_cs );								// 크리티칼 섹션 해제 
    return nRet;												// 반환 
}
#endif