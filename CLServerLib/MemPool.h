#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

// ��� ���� 
#include "CLGlobal.h"												// ��ü ó�� �Լ�

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Template Class 
//////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(push, struct_packing_before)
#pragma pack(1)													//�޸𸮸� ����Ʈ ������ �����Ѵ�.

template <class TYPE>
class CMemPool
{
protected:
	// INDEX Structure  
	struct INDEX
    {
		INDEX* pNext;											// ���� Index �����͸� ������ 
		int nIndex;												// ���� �ε��� ��ȣ 

		// Structure constructure
		INDEX(){
			pNext = NULL;									
			nIndex = 0;
		}
    }; 
//---------------------- Member Functions ----------------------//
public:
	// Construction & Destruction
    CMemPool(int nTotalCnt, int nAllocBufferSize=0);			// �Ҵ� ���� ������ ���� ������ �Է� 
	~CMemPool();												// �Ҵ� ���� ó�� 

	// Attributes
    int GetCount();												// ���� �Ҵ� ���� �� 
    int GetEmpty();												// ��� �ִ� ��Ŷ �� 
    int GetIndex( TYPE *ptr );									// ���ۿ� ���� �ε��� ��� 
	
	// Operations
    TYPE* Alloc();												// TYPE���� �޸� �Ҵ� 
    BOOL Free( TYPE *ptr );										// �Ҵ� ���� �޸𸮸� ���� 

//---------------------- Member Variables ----------------------//
protected:
    int		m_nCount;											// ���� �Ҵ� ���� �� 											
    int		m_nTotalCnt;										// ������� �Ҵ� ��Ŷ �� 
    int		m_nAllocBufferSize;									// �� ��Ŷ�� �Ҵ� ���� ���� ������ 
    int		m_nPerPacketSize;									// �� ��Ŷ�� ��ü ������ 

    PVOID	m_pPakets;											// ��ü �Ҵ� ������ 
    INDEX*	m_pFreeRoom;										// ���� ����ִ� ó�� �ε��� ������ 

    CRITICAL_SECTION m_cs;										// ũ��ƼĮ ���� ���� 
};
#pragma pack( pop, struct_packing_before )
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ������ - �޸� �Ҵ� ��ƾ 					//
// [2]PARAMETER : nTotalCnt - �Ҵ��� ��Ŷ�� ��					//
//				  nAllockBufferSize - �Ҵ��� ���� ������		//
// [3]RETURN :													//
// [4]DATE : 2000�� 9�� 4��										//
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
		//				< �Ҵ� ����Ÿ ���� >					// 
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

	// �Ҵ� ����Ÿ ���� �� ������ ������ ������ ��� 
    pIndex = (INDEX*) ((DWORD)pIndex + (m_nTotalCnt - 1) * m_nPerPacketSize);

	// �Ҵ� ����Ÿ ������ ������ ���� Linked List�� �����Ͽ� �ö���� 
    for( int i = m_nTotalCnt-1; i >= 0; i-- )
    {
        pIndex->pNext = m_pFreeRoom;								// �� �������� pNext = NULL�̴� 
        pIndex->nIndex = i;											// INDEX �۾� 

        m_pFreeRoom = pIndex;										// ���� ������ ���� ���� 

#ifdef _DEBUG
		// �� �� ���� ExBlockSize�� �κп� ����Ÿ ���� �۾� 
        if( m_nAllocBufferSize )
            memset((PVOID)( (DWORD)pIndex+sizeof(INDEX)+sizeof(TYPE)),
							(i%10+'0'), 
							m_nAllocBufferSize );
#endif
		// ���� ����Ʈ�� C8List ����Ʈ �̵� 
        pIndex = (INDEX*)((DWORD)pIndex - m_nPerPacketSize);
    }

	InitializeCriticalSectionAndSpinCount( &m_cs, 4000 );   // why 4000?
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : �Ҹ���  - �޸� �Ҵ� ���� 					//
// [2]PARAMETER :												//
// [3]RETURN :													//
// [4]DATE : 2000�� 9�� 4��										//
//////////////////////////////////////////////////////////////////
template <class TYPE>
CMemPool<TYPE>::~CMemPool<TYPE>()
{
    if(NULL != m_pPakets) VirtualFree( m_pPakets, 0, MEM_RELEASE );
    DeleteCriticalSection( &m_cs );								// ũ��ƼĮ ���� ���� ���� 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : �Ҵ�� ��Ŷ�� �� ��Ŷ ��ȯ					//
// [2]PARAMETER :												//
// [3]RETURN : ���ø� Ÿ�� ��ȯ 								//
// [4]DATE : 2000�� 9�� 4��										//
//////////////////////////////////////////////////////////////////
template <class TYPE>
TYPE* CMemPool<TYPE>::Alloc()
{
	// ���� �ʱ�ȭ 
    INDEX* pIndex = NULL;							
    TYPE* pType = NULL;

    EnterCriticalSection( &m_cs );								// ũ��ƼĮ ���� ���� 

    pIndex = m_pFreeRoom;
    if( pIndex != NULL )
    {
		// ����� �̵� �ε��� �����͸� ���� �ε����� �̵� 
        m_pFreeRoom = m_pFreeRoom->pNext;						
        m_nCount++;												// ī��Ʈ ���� 
		// ������ �ε��� �����Ϳ��� ���ø� Ÿ�� ��Ƽ ������ ��ȯ 
        pType = (TYPE*)(pIndex+1);								
		
		///////////////////////////////////////////
		// ġ������ ���� �߻� 
		// assert( m_nCount > 0 );									// make sure we don't overflow
		if(m_nCount <= 0)
		{
			LogError("CMemPool::Alloc()���� m_nCount���� �̻� �߻�");
			LeaveCriticalSection(&m_cs);
			return NULL;
		}

    }
    LeaveCriticalSection( &m_cs );								// ũ��ƼĮ ���� �� 
    return pType;												// ���ø� Ÿ�� ��ȯ 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : �Ҵ�� ��Ŷ�� ������� ��Ŷ ����			//
// [2]PARAMETER : ptr - ���ø� Ÿ�� ������ 						//
// [3]RETURN : TRUE - ����, FALSE - ���� 						//
// [4]DATE : 2000�� 9�� 4��										//
//////////////////////////////////////////////////////////////////
template <class TYPE>
BOOL CMemPool<TYPE>::Free( TYPE *ptr )
{
    BOOL bRet = FALSE;

    EnterCriticalSection( &m_cs );								// ũ��ƼĮ ���� ���� 

    INDEX* pIndex = ((INDEX*)(ptr))-1;							// ��� ��Ŷ�� �ε��� ������ ��� 

	// ������� ��Ŷ�� �ִٸ� 
    if( m_nCount > 0 )
    {
		// ����� �ε��� �����͸� ������ ���� ��ƾ 
        pIndex->pNext = m_pFreeRoom;							
        m_pFreeRoom = pIndex;
        m_nCount--;												// ��� ��Ŷ�� 1 ���� 
        bRet = TRUE;											// ���� ó�� �˸� 
    }

    LeaveCriticalSection( &m_cs );								// �׸�ƼĮ ���� Ż�� 
    return bRet;												// ���� ó�� ���� ��ȯ 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ������� ��Ŷ �� ���						//
// [2]PARAMETER : void											//
// [3]RETURN : int - ������� ��Ŷ �� ��ȯ 						//
// [4]DATE : 2000�� 9�� 4��										//
//////////////////////////////////////////////////////////////////
template <class TYPE>
int CMemPool<TYPE>::GetCount()
{
    int nRet = 0;												// ���� �ʱ�ȭ 
    EnterCriticalSection( &m_cs );								// ũ��ƼĮ ���� ����
    nRet = m_nCount;											// ���� ���� 
    LeaveCriticalSection( &m_cs );								// ũ��ƼĮ ���� ���� 
    return nRet;												// ��ȯ 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : �����ϰ� �մ� ��Ŷ �� ���				//
// [2]PARAMETER : void					 						//
// [3]RETURN : int - ��� ���ϴ� ��Ŷ �� ��ȯ					//
// [4]DATE : 2000�� 9�� 4��										//
//////////////////////////////////////////////////////////////////
template <class TYPE>
int CMemPool<TYPE>::GetEmpty()
{
    int nRet = 0;												// ���� �ʱ�ȭ 
    EnterCriticalSection( &m_cs );								// ũ��ƼĮ ���� ���� 
	nRet = m_nTotalCnt - m_nCount;								// ���� 
    LeaveCriticalSection( &m_cs );								// ũ��ƼĮ ���� ���� 
    return nRet;												// ��ȯ 
}

//////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ������� ��Ŷ�� INDEX���					//
// [2]PARAMETER : ptr - ���ø� Ÿ�� ������ 						//
// [3]RETURN : intE - ������� ��Ŷ�� �ε��� ��ȯ				//
// [4]DATE : 2000�� 9�� 4��										//
//////////////////////////////////////////////////////////////////
template <class TYPE>
int CMemPool<TYPE>::GetIndex( TYPE *ptr )
{
    int nRet = 0;												// ���� �ʱ�ȭ 
    EnterCriticalSection( &m_cs );								// ũ��ƼĮ ���� ����
    INDEX* pIndex = ((INDEX*)(ptr))-1;							// �ε��� ������ ��� 
    nRet = pIndex->nIndex;										// ���� ���� 
    LeaveCriticalSection( &m_cs );								// ũ��ƼĮ ���� ���� 
    return nRet;												// ��ȯ 
}
#endif