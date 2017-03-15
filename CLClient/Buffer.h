// Buffer.h: interface for the CBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUFFER_H__F8BD10E8_A999_4CD8_8C1D_A69A8A8DF82A__INCLUDED_)
#define AFX_BUFFER_H__F8BD10E8_A999_4CD8_8C1D_A69A8A8DF82A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

///////////////////////////////
// ��Ʈ��ũ ���� ����Ÿ Packing
#pragma pack( push, struct_packing_before )
#pragma pack(1) //�޸𸮸� ����Ʈ ������ �����Ѵ�.


//////////////////////////////////////////////////////////////////
// [1]CLASS NAME : CBuffer Ŭ����								//
// [2]DESCRIPTION : ��Ʈ��ũ ����Ÿ ���� ����					//
// [3]DATE : 2000�� 9�� 10��									//
//////////////////////////////////////////////////////////////////
class CBuffer  
{
//------------------------ �� �� �� �� -------------------------//
public:
	CBuffer();													// Contruction
	virtual ~CBuffer();											// Destruction

	void ClearBuffer();											// ���� ������, �缳�� 	

	UINT Delete(UINT nSize);									// ����Ÿ ���� 
	UINT Read(char* pData, UINT nSize);							// ����Ÿ �б� 
	BOOL Write(char *pData, UINT nSize);						// ����Ÿ ���� by CString 
	BOOL Write(C8String& strData);								// ����Ÿ ���� 
	UINT GetBufferLen();										// ��ü ���� ũ�� ��� 
	int Scan(char* pScan,UINT nPos);							// ã�� ���ڿ��� ���� ���ۿ� �ִ� �˻� 
	BOOL Insert(char* pData, UINT nSize);						// ����Ÿ ���� -> �� �տ� �Է� 
	BOOL Insert(C8String& strData);								// ����Ÿ ���� 

	void Copy(CBuffer& buffer);									// ����  ���� 

	char* GetBuffer(UINT nPos=0);								// ���� ����Ÿ ������ �ѱ� 

	void FileWrite(C8String& strFileName);						// ���Ϸ� �ۼ� 

protected:
	UINT ReAllocateBuffer(UINT nRequestedSize);					// ����Ÿ ������ �缳�� 
	UINT DeAllocateBuffer(UINT nRequestedSize);					// ����Ÿ ������ �缳�� (���� ����)
	UINT GetMemSize();											// ���� ����Ÿ ũ�� ��ȯ 			

//------------------------ �� �� �� �� -------------------------//
protected:
	char*	m_pBase;											// ��ü ����Ÿ ������  
	char*	m_pPtr;												// ����Ÿ ó���� ���� ������ 
																// ����Ÿ�� �� ���κ��� ����Ų�� 
	UINT	m_nSize;											// ����Ÿ ũ��  
};

#pragma pack( pop, struct_packing_before )

#endif // !defined(AFX_BUFFER_H__F8BD10E8_A999_4CD8_8C1D_A69A8A8DF82A__INCLUDED_)
