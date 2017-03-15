// Buffer.h: interface for the CBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUFFER_H__F8BD10E8_A999_4CD8_8C1D_A69A8A8DF82A__INCLUDED_)
#define AFX_BUFFER_H__F8BD10E8_A999_4CD8_8C1D_A69A8A8DF82A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

///////////////////////////////
// 네트워크 전송 데이타 Packing
#pragma pack( push, struct_packing_before )
#pragma pack(1) //메모리를 바이트 단위로 정렬한다.


//////////////////////////////////////////////////////////////////
// [1]CLASS NAME : CBuffer 클래스								//
// [2]DESCRIPTION : 네트워크 데이타 전송 버퍼					//
// [3]DATE : 2000년 9월 10일									//
//////////////////////////////////////////////////////////////////
class CBuffer  
{
//------------------------ 멤 버 함 수 -------------------------//
public:
	CBuffer();													// Contruction
	virtual ~CBuffer();											// Destruction

	void ClearBuffer();											// 버퍼 삭제후, 재설정 	

	UINT Delete(UINT nSize);									// 데이타 삭제 
	UINT Read(char* pData, UINT nSize);							// 데이타 읽기 
	BOOL Write(char *pData, UINT nSize);						// 데이타 쓰기 by CString 
	BOOL Write(C8String& strData);								// 데이타 쓰기 
	UINT GetBufferLen();										// 전체 버퍼 크기 얻기 
	int Scan(char* pScan,UINT nPos);							// 찾을 문자열이 현재 버퍼에 있는 검사 
	BOOL Insert(char* pData, UINT nSize);						// 데이타 삽입 -> 맨 앞에 입력 
	BOOL Insert(C8String& strData);								// 데이타 삽입 

	void Copy(CBuffer& buffer);									// 버퍼  복사 

	char* GetBuffer(UINT nPos=0);								// 버퍼 데이타 포인터 넘김 

	void FileWrite(C8String& strFileName);						// 파일로 작성 

protected:
	UINT ReAllocateBuffer(UINT nRequestedSize);					// 데이타 포인터 재설정 
	UINT DeAllocateBuffer(UINT nRequestedSize);					// 데이타 포인터 재설정 (위와 동일)
	UINT GetMemSize();											// 현재 데이타 크기 반환 			

//------------------------ 멤 버 변 수 -------------------------//
protected:
	char*	m_pBase;											// 전체 데이타 포인터  
	char*	m_pPtr;												// 데이타 처리를 위한 포인터 
																// 데이타의 맨 끝부분을 가리킨다 
	UINT	m_nSize;											// 데이타 크기  
};

#pragma pack( pop, struct_packing_before )

#endif // !defined(AFX_BUFFER_H__F8BD10E8_A999_4CD8_8C1D_A69A8A8DF82A__INCLUDED_)
