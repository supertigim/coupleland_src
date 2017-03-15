// Buffer.cpp: implementation of the CBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "ProtoGlobal.h"
#include "Buffer.h"
#include <math.h>

/////////////////////////////////////////////////////////////////////////////////////////
// 생성자
/////////////////////////////////////////////////////////////////////////////////////////
CBuffer::CBuffer()
{
	m_nSize = MAX_BUFF_SIZE;

	// 최대 사이즈로 데이타 버퍼 할당
	m_pPtr = m_pBase = new char[m_nSize];
}

/////////////////////////////////////////////////////////////////////////////////////////
// 소멸자 
/////////////////////////////////////////////////////////////////////////////////////////
CBuffer::~CBuffer()
{
	// 할당 데이타 삭제 ㅔ
	if (m_pBase)
		delete [] m_pBase;
}
	
/////////////////////////////////////////////////////////////////////////////////////////
// 데이타를 버퍼에 저장	
/////////////////////////////////////////////////////////////////////////////////////////
BOOL CBuffer::Write(char *pData, UINT nSize)
{
	ReAllocateBuffer(nSize + GetBufferLen());					// 메모리 재할당 

	CopyMemory(m_pPtr,pData,nSize);								// 처리 포인터 뒤로 데이타 복사 

	m_pPtr+=nSize;												// 처리 포인터 뒤로 이동 

	return nSize;												// 처리 결과 반환 
}

/////////////////////////////////////////////////////////////////////////////////////////
// 데이타 스트링을 버퍼에 저장		 			
/////////////////////////////////////////////////////////////////////////////////////////
BOOL CBuffer::Write(C8String &strData)
{
	int nSize = strData.GetLength();							// 크기 얻기 

	return Write(strData.CopyString(), nSize);					// 데이타 쓰기
}

/////////////////////////////////////////////////////////////////////////////////////////
// 버퍼에 데이타 삽입				 			
/////////////////////////////////////////////////////////////////////////////////////////
BOOL CBuffer::Insert(char* pData, UINT nSize)
{
	ReAllocateBuffer(nSize + GetBufferLen());					// 메모리 확장 

	MoveMemory(m_pBase+nSize,m_pBase,GetMemSize() - nSize);		// 베이스 포인터 위치의 데이타를 뒤로 이동 

	CopyMemory(m_pBase,pData,nSize);							// 베이스 포인터 위치에 pData 삽입 

	m_pPtr+=nSize;												// 처리 포인터 nSize로 이동 

	return nSize;												// 처리 반환 
}

/////////////////////////////////////////////////////////////////////////////////////////
// 버퍼에 데이타 삽입				 			
/////////////////////////////////////////////////////////////////////////////////////////
BOOL CBuffer::Insert(C8String& strData)
{
	int nSize = strData.GetLength();							// 크기 얻기
	return Insert(strData.CopyString(), nSize);					// 데이타 삽입 
}	

/////////////////////////////////////////////////////////////////////////////////////////
// 데이타를 읽은후 읽은 만큼 지운다 			
/////////////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::Read(char* pData, UINT nSize)
{
	// 전체 메모리 크기 보다 큰 값이라면 
	if (nSize > GetMemSize())
		return 0;

	// 현재 저장중인 데이타보단 큰 값이라면  
	if (nSize > GetBufferLen())
		nSize = GetBufferLen();									// 현재 저장 데이타 크기로 처리 

		
	// 크기가 있다면,
	if (nSize)
	{
		CopyMemory(pData,m_pBase,nSize);						// 메모리 카피 

		MoveMemory(m_pBase,m_pBase+nSize,GetMemSize() - nSize);	// 베이스 데이타 포인터로  
																// nSize만큼 뒤의 데이타를 
																// 앞으로 이동시킨다 

		m_pPtr -= nSize;										// 처리 데이타 포인터 이동 
	}
		
	DeAllocateBuffer(GetBufferLen());							// 메모리 재설정 

	return nSize;												// 지운 크기 반환 
}

/////////////////////////////////////////////////////////////////////////////////////////
// 전체 할당 버퍼(메모리) 사이즈 반환 
/////////////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::GetMemSize() 
{
	return m_nSize;												// 전체 할당 메모리 사이즈 반환 
}

/////////////////////////////////////////////////////////////////////////////////////////
// 현재 데이타 크기				 			
/////////////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::GetBufferLen() 
{
	// 베이스 포인터 확인 
	if (m_pBase == NULL)
		return 0;												// 에러 

	int nSize = m_pPtr - m_pBase;								// 크기 설정 

	return nSize;												// 현재 데이타 크기 반환 
}

/////////////////////////////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 요구하는 크기로 버퍼 크기 재설정			
/////////////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::ReAllocateBuffer(UINT nRequestedSize)
{
	//	현재 버퍼 크기 보다 작으면,
	if (nRequestedSize < GetMemSize())
		return 0;

	// 새로운 버퍼 할당 크기 설정 
	UINT nNewSize = (UINT) ceil(nRequestedSize / (double)MAX_BUFF_SIZE ) * MAX_BUFF_SIZE;

	// 새로운 버퍼 할당 
	char* pNewBuffer = new char[nNewSize];

	UINT nBufferLen = GetBufferLen();							// 현재 데이타 크기 

	CopyMemory(pNewBuffer,m_pBase,nBufferLen);					// 현재 데이타를 새 버퍼에 복사 

	delete [] m_pBase;											// 구 데이타 버퍼 해제 

	m_pBase = pNewBuffer;										// 포인터를 멤버 변수에 넘김 

	m_pPtr = m_pBase + nBufferLen;								// 처리 포인터 이동 

	m_nSize = nNewSize;											// 사이즈 넘김 

	return m_nSize;												// 크기 반환 
}

/////////////////////////////////////////////////////////////////////////////////////////
// 버퍼 삭제후 재 설정							
/////////////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::DeAllocateBuffer(UINT nRequestedSize)
{
	//	현재 버퍼 크기 보다 작으면,
	if (nRequestedSize < GetBufferLen())
		return 0;

	// 새로운 버퍼 할당 크기 설정 
	UINT nNewSize = (UINT) ceil(nRequestedSize / (double)MAX_BUFF_SIZE ) * MAX_BUFF_SIZE;

	if (nNewSize < GetMemSize())
		return 0;

	// 새로운 버퍼 할당
	char* pNewBuffer = new char[nNewSize];

	UINT nBufferLen = GetBufferLen();							// 현재 데이타 크기 
	CopyMemory(pNewBuffer,m_pBase,nBufferLen);					// 현재 데이타를 새 버퍼에 복사

	delete [] m_pBase;											// 구 데이타 버퍼 해제

	m_pBase = pNewBuffer;										// 포인터를 멤버 변수에 넘김

	m_pPtr = m_pBase + nBufferLen;								// 처리 포인터 이동 

	m_nSize = nNewSize;											// 사이즈 넘김

	return m_nSize;												// 크기 반환
}

/////////////////////////////////////////////////////////////////////////////////////////
// 버퍼에 지정한 데이타가 있는지 스캐닝		
/////////////////////////////////////////////////////////////////////////////////////////
int CBuffer::Scan(char* pScan,UINT nPos)
{
	// 지정한 위치가 현재 버퍼의 데이타 크기보다 크다면, 
	if (nPos > GetBufferLen() )			
		return -1;

	// 찾은 위치 포인터 받기 
	char* pStr = strstr((char*)(m_pBase+nPos),(char*)pScan);
	
	int nOffset = 0;											// 변수 설정 

	// 찾은 위치 포인터가 있다면 
	if (pStr)
		nOffset = (pStr - m_pBase) + strlen((char*)pScan);		// 위치 

	return nOffset;												// 반환 
}

/////////////////////////////////////////////////////////////////////////////////////////
// 버퍼 클리어 및 Reset 						
/////////////////////////////////////////////////////////////////////////////////////////
void CBuffer::ClearBuffer()
{
	m_pPtr = m_pBase;											// 데이타 클리어를 위한 작업
																// 항상 데이타 처리는 m_pPtr로 

	DeAllocateBuffer(MAX_BUFF_SIZE);							// 현재 데이타 삭제 및 새로 데이타 버퍼 생성  
}

/////////////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION : 데이타 카피									
/////////////////////////////////////////////////////////////////////////////////////////
void CBuffer::Copy(CBuffer& buffer)
{
	int nReSize = buffer.GetMemSize();							// 입력 버퍼의 전체 버퍼 크기 
	int nSize = buffer.GetBufferLen();							// 입력 버퍼내의 데이타 크기 

	ClearBuffer();												// 현재 버퍼 클리어 

	ReAllocateBuffer(nReSize);									// 입력 버퍼와 크기를 맞추기 

	m_pPtr = m_pBase + nSize;									// 처리 포인터 이동 

	// 데이타 복사 
	CopyMemory(m_pBase,buffer.GetBuffer(),buffer.GetBufferLen());
}

/////////////////////////////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 버퍼내의 nPos만큼 위치의 데이타 반환		
/////////////////////////////////////////////////////////////////////////////////////////
char* CBuffer::GetBuffer(UINT nPos)
{
	return m_pBase+nPos;										// 파일 포인터 반환 
}

/////////////////////////////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Log 파일 생성								
/////////////////////////////////////////////////////////////////////////////////////////
void CBuffer::FileWrite(C8String& strFileName)
{
	// 변수 설정 
	FILE* file = NULL;											// 파일 변수 
	file = fopen(strFileName.CopyString(),"w");					// 파일 오픈 

	// 제대로 오픈이 되었다면 
	if(file != NULL)
	{	
		fwrite(m_pBase,(int)GetBufferLen(),1,file);				// 파일에 데이타 쓰기 
		_fcloseall();											// 파일 닫기 
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : 읽은 데이타를 지우거나 버퍼 데이타를 지울때 
/////////////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::Delete(UINT nSize)
{
	// 전체 메모리 크기 보다 큰 값이라면 
	if (nSize > GetMemSize())
		return 0;

	// 현재 저장중인 데이타보단 큰 값이라면  
	if (nSize > GetBufferLen())
		nSize = GetBufferLen();									// 현재 저장 데이타 크기로 처리 

		
	// 크기가 있다면,
	if (nSize)
	{
		MoveMemory(m_pBase,m_pBase+nSize,GetMemSize() - nSize);	// 베이스 데이타 포인터로  
																// nSize만큼 뒤의 데이타를 
																// 앞으로 이동시킨다 

		m_pPtr -= nSize;										// 처리 데이타 포인터 이동 
	}
		
	DeAllocateBuffer(GetBufferLen());							// 메모리 재설정 

	return nSize;												// 지운 크기 반환 
}