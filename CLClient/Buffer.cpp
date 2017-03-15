// Buffer.cpp: implementation of the CBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "ProtoGlobal.h"
#include "Buffer.h"
#include <math.h>

/////////////////////////////////////////////////////////////////////////////////////////
// ������
/////////////////////////////////////////////////////////////////////////////////////////
CBuffer::CBuffer()
{
	m_nSize = MAX_BUFF_SIZE;

	// �ִ� ������� ����Ÿ ���� �Ҵ�
	m_pPtr = m_pBase = new char[m_nSize];
}

/////////////////////////////////////////////////////////////////////////////////////////
// �Ҹ��� 
/////////////////////////////////////////////////////////////////////////////////////////
CBuffer::~CBuffer()
{
	// �Ҵ� ����Ÿ ���� ��
	if (m_pBase)
		delete [] m_pBase;
}
	
/////////////////////////////////////////////////////////////////////////////////////////
// ����Ÿ�� ���ۿ� ����	
/////////////////////////////////////////////////////////////////////////////////////////
BOOL CBuffer::Write(char *pData, UINT nSize)
{
	ReAllocateBuffer(nSize + GetBufferLen());					// �޸� ���Ҵ� 

	CopyMemory(m_pPtr,pData,nSize);								// ó�� ������ �ڷ� ����Ÿ ���� 

	m_pPtr+=nSize;												// ó�� ������ �ڷ� �̵� 

	return nSize;												// ó�� ��� ��ȯ 
}

/////////////////////////////////////////////////////////////////////////////////////////
// ����Ÿ ��Ʈ���� ���ۿ� ����		 			
/////////////////////////////////////////////////////////////////////////////////////////
BOOL CBuffer::Write(C8String &strData)
{
	int nSize = strData.GetLength();							// ũ�� ��� 

	return Write(strData.CopyString(), nSize);					// ����Ÿ ����
}

/////////////////////////////////////////////////////////////////////////////////////////
// ���ۿ� ����Ÿ ����				 			
/////////////////////////////////////////////////////////////////////////////////////////
BOOL CBuffer::Insert(char* pData, UINT nSize)
{
	ReAllocateBuffer(nSize + GetBufferLen());					// �޸� Ȯ�� 

	MoveMemory(m_pBase+nSize,m_pBase,GetMemSize() - nSize);		// ���̽� ������ ��ġ�� ����Ÿ�� �ڷ� �̵� 

	CopyMemory(m_pBase,pData,nSize);							// ���̽� ������ ��ġ�� pData ���� 

	m_pPtr+=nSize;												// ó�� ������ nSize�� �̵� 

	return nSize;												// ó�� ��ȯ 
}

/////////////////////////////////////////////////////////////////////////////////////////
// ���ۿ� ����Ÿ ����				 			
/////////////////////////////////////////////////////////////////////////////////////////
BOOL CBuffer::Insert(C8String& strData)
{
	int nSize = strData.GetLength();							// ũ�� ���
	return Insert(strData.CopyString(), nSize);					// ����Ÿ ���� 
}	

/////////////////////////////////////////////////////////////////////////////////////////
// ����Ÿ�� ������ ���� ��ŭ ����� 			
/////////////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::Read(char* pData, UINT nSize)
{
	// ��ü �޸� ũ�� ���� ū ���̶�� 
	if (nSize > GetMemSize())
		return 0;

	// ���� �������� ����Ÿ���� ū ���̶��  
	if (nSize > GetBufferLen())
		nSize = GetBufferLen();									// ���� ���� ����Ÿ ũ��� ó�� 

		
	// ũ�Ⱑ �ִٸ�,
	if (nSize)
	{
		CopyMemory(pData,m_pBase,nSize);						// �޸� ī�� 

		MoveMemory(m_pBase,m_pBase+nSize,GetMemSize() - nSize);	// ���̽� ����Ÿ �����ͷ�  
																// nSize��ŭ ���� ����Ÿ�� 
																// ������ �̵���Ų�� 

		m_pPtr -= nSize;										// ó�� ����Ÿ ������ �̵� 
	}
		
	DeAllocateBuffer(GetBufferLen());							// �޸� �缳�� 

	return nSize;												// ���� ũ�� ��ȯ 
}

/////////////////////////////////////////////////////////////////////////////////////////
// ��ü �Ҵ� ����(�޸�) ������ ��ȯ 
/////////////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::GetMemSize() 
{
	return m_nSize;												// ��ü �Ҵ� �޸� ������ ��ȯ 
}

/////////////////////////////////////////////////////////////////////////////////////////
// ���� ����Ÿ ũ��				 			
/////////////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::GetBufferLen() 
{
	// ���̽� ������ Ȯ�� 
	if (m_pBase == NULL)
		return 0;												// ���� 

	int nSize = m_pPtr - m_pBase;								// ũ�� ���� 

	return nSize;												// ���� ����Ÿ ũ�� ��ȯ 
}

/////////////////////////////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : �䱸�ϴ� ũ��� ���� ũ�� �缳��			
/////////////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::ReAllocateBuffer(UINT nRequestedSize)
{
	//	���� ���� ũ�� ���� ������,
	if (nRequestedSize < GetMemSize())
		return 0;

	// ���ο� ���� �Ҵ� ũ�� ���� 
	UINT nNewSize = (UINT) ceil(nRequestedSize / (double)MAX_BUFF_SIZE ) * MAX_BUFF_SIZE;

	// ���ο� ���� �Ҵ� 
	char* pNewBuffer = new char[nNewSize];

	UINT nBufferLen = GetBufferLen();							// ���� ����Ÿ ũ�� 

	CopyMemory(pNewBuffer,m_pBase,nBufferLen);					// ���� ����Ÿ�� �� ���ۿ� ���� 

	delete [] m_pBase;											// �� ����Ÿ ���� ���� 

	m_pBase = pNewBuffer;										// �����͸� ��� ������ �ѱ� 

	m_pPtr = m_pBase + nBufferLen;								// ó�� ������ �̵� 

	m_nSize = nNewSize;											// ������ �ѱ� 

	return m_nSize;												// ũ�� ��ȯ 
}

/////////////////////////////////////////////////////////////////////////////////////////
// ���� ������ �� ����							
/////////////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::DeAllocateBuffer(UINT nRequestedSize)
{
	//	���� ���� ũ�� ���� ������,
	if (nRequestedSize < GetBufferLen())
		return 0;

	// ���ο� ���� �Ҵ� ũ�� ���� 
	UINT nNewSize = (UINT) ceil(nRequestedSize / (double)MAX_BUFF_SIZE ) * MAX_BUFF_SIZE;

	if (nNewSize < GetMemSize())
		return 0;

	// ���ο� ���� �Ҵ�
	char* pNewBuffer = new char[nNewSize];

	UINT nBufferLen = GetBufferLen();							// ���� ����Ÿ ũ�� 
	CopyMemory(pNewBuffer,m_pBase,nBufferLen);					// ���� ����Ÿ�� �� ���ۿ� ����

	delete [] m_pBase;											// �� ����Ÿ ���� ����

	m_pBase = pNewBuffer;										// �����͸� ��� ������ �ѱ�

	m_pPtr = m_pBase + nBufferLen;								// ó�� ������ �̵� 

	m_nSize = nNewSize;											// ������ �ѱ�

	return m_nSize;												// ũ�� ��ȯ
}

/////////////////////////////////////////////////////////////////////////////////////////
// ���ۿ� ������ ����Ÿ�� �ִ��� ��ĳ��		
/////////////////////////////////////////////////////////////////////////////////////////
int CBuffer::Scan(char* pScan,UINT nPos)
{
	// ������ ��ġ�� ���� ������ ����Ÿ ũ�⺸�� ũ�ٸ�, 
	if (nPos > GetBufferLen() )			
		return -1;

	// ã�� ��ġ ������ �ޱ� 
	char* pStr = strstr((char*)(m_pBase+nPos),(char*)pScan);
	
	int nOffset = 0;											// ���� ���� 

	// ã�� ��ġ �����Ͱ� �ִٸ� 
	if (pStr)
		nOffset = (pStr - m_pBase) + strlen((char*)pScan);		// ��ġ 

	return nOffset;												// ��ȯ 
}

/////////////////////////////////////////////////////////////////////////////////////////
// ���� Ŭ���� �� Reset 						
/////////////////////////////////////////////////////////////////////////////////////////
void CBuffer::ClearBuffer()
{
	m_pPtr = m_pBase;											// ����Ÿ Ŭ��� ���� �۾�
																// �׻� ����Ÿ ó���� m_pPtr�� 

	DeAllocateBuffer(MAX_BUFF_SIZE);							// ���� ����Ÿ ���� �� ���� ����Ÿ ���� ����  
}

/////////////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION : ����Ÿ ī��									
/////////////////////////////////////////////////////////////////////////////////////////
void CBuffer::Copy(CBuffer& buffer)
{
	int nReSize = buffer.GetMemSize();							// �Է� ������ ��ü ���� ũ�� 
	int nSize = buffer.GetBufferLen();							// �Է� ���۳��� ����Ÿ ũ�� 

	ClearBuffer();												// ���� ���� Ŭ���� 

	ReAllocateBuffer(nReSize);									// �Է� ���ۿ� ũ�⸦ ���߱� 

	m_pPtr = m_pBase + nSize;									// ó�� ������ �̵� 

	// ����Ÿ ���� 
	CopyMemory(m_pBase,buffer.GetBuffer(),buffer.GetBufferLen());
}

/////////////////////////////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ���۳��� nPos��ŭ ��ġ�� ����Ÿ ��ȯ		
/////////////////////////////////////////////////////////////////////////////////////////
char* CBuffer::GetBuffer(UINT nPos)
{
	return m_pBase+nPos;										// ���� ������ ��ȯ 
}

/////////////////////////////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : Log ���� ����								
/////////////////////////////////////////////////////////////////////////////////////////
void CBuffer::FileWrite(C8String& strFileName)
{
	// ���� ���� 
	FILE* file = NULL;											// ���� ���� 
	file = fopen(strFileName.CopyString(),"w");					// ���� ���� 

	// ����� ������ �Ǿ��ٸ� 
	if(file != NULL)
	{	
		fwrite(m_pBase,(int)GetBufferLen(),1,file);				// ���Ͽ� ����Ÿ ���� 
		_fcloseall();											// ���� �ݱ� 
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// [1]DESCRIPTION : ���� ����Ÿ�� ����ų� ���� ����Ÿ�� ���ﶧ 
/////////////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::Delete(UINT nSize)
{
	// ��ü �޸� ũ�� ���� ū ���̶�� 
	if (nSize > GetMemSize())
		return 0;

	// ���� �������� ����Ÿ���� ū ���̶��  
	if (nSize > GetBufferLen())
		nSize = GetBufferLen();									// ���� ���� ����Ÿ ũ��� ó�� 

		
	// ũ�Ⱑ �ִٸ�,
	if (nSize)
	{
		MoveMemory(m_pBase,m_pBase+nSize,GetMemSize() - nSize);	// ���̽� ����Ÿ �����ͷ�  
																// nSize��ŭ ���� ����Ÿ�� 
																// ������ �̵���Ų�� 

		m_pPtr -= nSize;										// ó�� ����Ÿ ������ �̵� 
	}
		
	DeAllocateBuffer(GetBufferLen());							// �޸� �缳�� 

	return nSize;												// ���� ũ�� ��ȯ 
}