// 8String.cpp: implementation of the C8String class.
//
//////////////////////////////////////////////////////////////////////

//#include "ProtoGlobal.h"
#include "stdAfx.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


C8String::C8String()
{
	m_buff = NULL;
	temp_buff = NULL;
	m_BufferLock = false;
}

C8String::C8String(const char * const String)
{
	temp_buff = NULL;
	m_buff = NULL;

	m_len = Strlen(String);

	Create_Buffer(m_len+1);
	Strcpy(m_buff,String);
	m_BufferLock = false;
}

C8String::~C8String()
{
	delete [] m_buff;
	delete [] temp_buff;

	m_len = 0;
}

void C8String::Create_TempBuffer(int len)
{
	delete [] temp_buff;
	temp_buff = NULL;

	temp_buff = new char[len];
	if(temp_buff == NULL) ErrorMessage();
}

void C8String::Create_Buffer(int len)
{
	delete [] m_buff;
	m_buff = NULL;

	m_buff = new char[len];
	if(m_buff == NULL) ErrorMessage();
}

int C8String::GetLength(void)
{
	return m_len;
}

bool C8String::IsEmpty(void)
{
	if(m_buff) return false;
	else return true;
}

void C8String::Empty(void)
{
	if(m_BufferLock == true) return;

	delete [] m_buff;
	m_buff = NULL;

	m_len = 0;
}

char C8String::GetAt(int nlindex)
{
	if(m_len >= nlindex) return m_buff[nlindex];
	else return NULL;
}

void C8String::SetAt(int nlindex, char ch)
{
	if(m_BufferLock == true) return;

	if(m_len >= nlindex) m_buff[nlindex] = ch;
}

int C8String::Compare(char *lpsz) const
{
	if(m_buff == NULL || lpsz == NULL) return -1;			// ¿øÃµºÀ¼â

	return strcmp(m_buff,lpsz);
}

char* C8String::Mid(int nFirst)
{
	int nCount = m_len - nFirst; 
	return Mid(nFirst, nCount);
}

char* C8String::Mid(int nFirst, int nCount)
{
	int len = nCount;

	if(nFirst < 0 || nFirst >= m_len || nCount < 0) return NULL;
	if((nFirst+nCount) > m_len) len = m_len - nFirst;

	Create_TempBuffer(len+1);

	for(int i=0;i<len;i++)
	{
		temp_buff[i] = m_buff[i+nFirst];
	}
	temp_buff[i] = NULL;

	return temp_buff;
}

char* C8String::Left(int nCount)
{
	if(nCount < 0) return NULL;
	if(nCount > m_len) nCount = m_len;

	Create_TempBuffer(nCount+1);

	Strncpy(temp_buff,m_buff,nCount);
	temp_buff[nCount] = NULL;

	return temp_buff;
}

char* C8String::Right(int nCount)
{
	if(nCount < 0) return NULL;
	if(nCount > m_len) nCount = m_len;

	Create_TempBuffer(nCount+1);

	for(int i=0;i<nCount;i++)
	{
		temp_buff[i] = m_buff[(m_len-nCount)+i];
	}
	temp_buff[i] = NULL;

	return temp_buff;
}

char* C8String::Spanlncluding(char *lpszCharSet)
{
	bool ck;
	int len = Strlen(lpszCharSet);

	Create_TempBuffer(m_len+1);
	Strcpy(temp_buff,m_buff);

	for(int i=0;i<m_len;i++)
	{
		ck = false;
		for(int j=0;j<len;j++)
		{
			if(temp_buff[i] == lpszCharSet[j]) 
			{
				ck = true;
				break;
			}
		}
		if(ck == false) break;
	}
	temp_buff[i] = NULL;

	return temp_buff;
}

char* C8String::SpanExcluding(char *lpszCharSet)
{
	bool ck = false;
	int len = Strlen(lpszCharSet);

	Create_TempBuffer(m_len+1);
	Strcpy(temp_buff,m_buff);

	for(int i=0;i<m_len;i++)
	{
		for(int j=0;j<len;j++)
		{
			if(temp_buff[i] == lpszCharSet[j]) 
			{
				ck = true;
				break;
			}
		}
		if(ck == true) break;
	}
	temp_buff[i] = NULL;

	return temp_buff;
}

void C8String::MakeUpper(void)
{
	if(m_BufferLock == true || m_buff == NULL) return;

	strupr(m_buff);
}

void C8String::MakeLower(void)
{
	if(m_BufferLock == true || m_buff == NULL) return;

	strlwr(m_buff);
}

void C8String::MakeReverse(void)
{
	if(m_BufferLock == true || m_buff == NULL) return;

	strrev(m_buff);
}

int C8String::Replace(char chOld,char chNew)
{
	if(m_BufferLock == true) return -1;

	int count = 0;

	for(int i=0;i<m_len;i++)
	{
		if(m_buff[i] == chOld) 
		{
			m_buff[i] = chNew;
			count++;
		}
	}

	return count;
}

int C8String::Remove(char ch)
{
	if(m_BufferLock == true) return -1;

	int count = 0;

	Create_TempBuffer(m_len+1);

	for(int i=0,j=0;i<m_len;i++)
	{
		if(m_buff[i] != ch) temp_buff[j++] = m_buff[i];
	}
	temp_buff[j] = NULL;

	count = m_len - j;
	m_len = j;

	Strcpy(m_buff,temp_buff);
	return count;
}

int C8String::Insert(int nlindex,char ch)
{
	if(m_BufferLock == true) return -1;

	if(nlindex < 0) return m_len;

	Create_TempBuffer(m_len+2);

	if(nlindex < m_len)
	{
		for(int j=0,i=0;i<m_len;i++)
		{
			if(i == nlindex) temp_buff[j++] = ch;
			temp_buff[j++] = m_buff[i];
		}
	}
	else
	{
		Strcpy(temp_buff,m_buff);
		temp_buff[m_len] = ch;
	}
	temp_buff[m_len+1] = NULL;

	Create_Buffer(m_len+2);
	Strcpy(m_buff,temp_buff);
	m_len++;

	return m_len;
}

int C8String::Insert(int nlindex,char *pstr)
{
	if(m_BufferLock == true) return -1;

	if(nlindex < 0) return m_len;

	int len = Strlen(pstr);

	Create_TempBuffer(m_len+len+1);

	if(nlindex < m_len)
	{
		for(int j=0,i=0;i<m_len;i++)
		{
			if(i == nlindex) 
			{
				temp_buff[j] = NULL;
				Strcat(temp_buff,pstr);
				j += len;
			}
			temp_buff[j++] = m_buff[i];
		}
		temp_buff[j] = NULL;
	}
	else
	{
		Strcpy(temp_buff,m_buff);
		Strcat(temp_buff,pstr);
	}

	Create_Buffer(m_len+len+1);
	Strcpy(m_buff,temp_buff);
	
	m_len += len;

	return m_len;
}

int C8String::Delete(int nlindex,int nCount = 1)
{
	if(m_BufferLock == true) return -1;

	int len = nCount;

	if(nlindex < 0 || nlindex >= m_len || nCount < 0) return m_len;
	if((nlindex+nCount) > m_len) len = m_len - nlindex;

	Create_TempBuffer((m_len-len)+1);

	for(int j=0,i=0;i<m_len-len;i++)
	{
		if(i == nlindex) j += nCount;
		temp_buff[i] = m_buff[j++];
	}
	temp_buff[i] = NULL;

	m_len -= len;

	Create_Buffer(m_len+1);
	Strcpy(m_buff,temp_buff);

	return m_len;
}

void C8String::Format(char *lpszFormat, ...)
{
	if(m_BufferLock == true) return;

	va_list		argptr;

	Create_TempBuffer(65536);
	
	va_start(argptr, lpszFormat);
	vsprintf(temp_buff, lpszFormat, argptr);
	va_end(argptr);

	m_len = Strlen(temp_buff);
	Create_Buffer(m_len+1);
	Strcpy(m_buff,temp_buff);
	delete [] temp_buff;
	temp_buff = NULL;
}

void C8String::TrimLeft(char *chTargets)
{
	if(m_BufferLock == true) return;

	bool ck = false;
	int j,len = Strlen(chTargets);

	Create_TempBuffer(m_len+1);
	Strcpy(temp_buff,m_buff);

	for(int i=0;i<m_len;i++)
	{
		if(ck == false)
		{
			for(j=0;j<len;j++) 
			{
				if(m_buff[i] == chTargets[j]) break;
			}
			if(j == len) 
			{
				ck = true;
				j = 0;
				i--;
			}
		}
		else
		{
			temp_buff[j++] = m_buff[i];
		}
	}
	temp_buff[j] = NULL;

	m_len = Strlen(temp_buff);
	Create_Buffer(m_len+1);

	Strcpy(m_buff,temp_buff);
}

void C8String::TrimLeft()
{
	TrimLeft(" ");
}

void C8String::TrimLeft(char chTarget)
{
	char buff[2];

	buff[0] = chTarget;
	buff[1] = NULL;

	TrimLeft(buff);
}

void C8String::TrimRight(char *chTargets)
{
	if(m_BufferLock == true) return;

	int j,len = Strlen(chTargets);

	Create_TempBuffer(m_len+1);
	Strcpy(temp_buff,m_buff);

	for(int i=0;i<m_len;i++)
	{
		for(j=0;j<len;j++) 
		{
			if(temp_buff[m_len-(i+1)] == chTargets[j]) break;
		}
		if(j == len) 
		{
			temp_buff[m_len-i] = NULL;
			break;
		}
	}

	m_len = Strlen(temp_buff);
	Create_Buffer(m_len+1);

	Strcpy(m_buff,temp_buff);
}

void C8String::TrimRight()
{
	TrimRight(" ");
}

void C8String::TrimRight(char chTarget)
{
	char buff[2];

	buff[0] = chTarget;
	buff[1] = NULL;

	TrimRight(buff);
}

int C8String::Find(char ch)
{
	for(int i=0;i<m_len;i++)
	{
		if(m_buff[i] == ch) break;
	}

	return i;
}

int C8String::Find(char *lpszSub)
{
	char *ptr;
	
	ptr = strstr(m_buff,lpszSub);

	return m_len - Strlen(ptr);
}

int C8String::Find(char ch,int nStart)
{
	if(nStart >= m_len) return -1;

	for(int i=nStart;i<m_len;i++)
	{
		if(m_buff[i] == ch) break;
	}

	return i;
}

int C8String::Find(char *sptr,int nStart)
{
	if(nStart >= m_len) return -1;

	char *ptr;

	Create_TempBuffer((m_len-nStart)+1);

	Strcpy(temp_buff,Mid(nStart,(m_len-nStart)));
	
	ptr = strstr(temp_buff,sptr);

	return m_len-Strlen(ptr);
}

int C8String::ReverseFind(char ch)
{
	char *ptr;

	ptr = strrchr(m_buff,ch);

	return m_len - Strlen(ptr);
}

int C8String::FindOneOf(char *lpszCharSet)
{
	if(m_buff == NULL || lpszCharSet == NULL) return -1;		// ¿øÃµºÀ¼â

	return strcspn(m_buff,lpszCharSet);
}

char* C8String::GetBuffer(int nMinBufLength)
{
	Create_TempBuffer(nMinBufLength+1);

	return temp_buff;
}

void C8String::ReleaseBuffer()
{
	if(temp_buff) 
	{
		delete [] temp_buff;
		temp_buff = NULL;
	}
}

void C8String::LockBuffer(void)
{
	m_BufferLock = true;
}

void C8String::UnlockBuffer(void)
{
	m_BufferLock = false;
}

C8String C8String::operator+ (const C8String &rhs)
{
	if(rhs.m_len == 0) C8String();

	Create_TempBuffer((m_len + rhs.m_len)+1);

	if(m_buff) 
	{
		Strcpy(temp_buff,m_buff);
		Strcat(temp_buff,rhs.m_buff);
	}
	else Strcpy(temp_buff,rhs.m_buff); 

	return C8String(temp_buff);
}

void C8String::operator+= (const C8String &rhs)
{
	if(m_BufferLock == true || rhs.m_len == 0) return;

	m_len += rhs.m_len;
	Create_TempBuffer(m_len+1);

	if(m_buff) 
	{
		Strcpy(temp_buff,m_buff);
		Strcat(temp_buff,rhs.m_buff);
	}
	else Strcpy(temp_buff,rhs.m_buff);

	*this = temp_buff;
}

C8String& C8String::operator= (const C8String& rhs)
{
	if(m_BufferLock == true || this == &rhs || rhs.m_len == 0) return *this;

	m_len = rhs.m_len;
	Create_Buffer(m_len+1);

	Strcpy(m_buff,rhs.m_buff);

	return *this;
}
	
char& C8String::operator[] (unsigned short offset)
{
	if(offset > m_len) return m_buff[m_len-1];
	else return m_buff[offset];
}

//==================================================================
// Ãß°¡ ÇÔ¼ö 
//==================================================================
void C8String::CopyBuffer(LPCTSTR lpch)
{
	Strcpy((char *)lpch,m_buff);
}

void C8String::CopyBufferLen(LPCTSTR lpch,int len)
{
	CopyMemory((char *)lpch,m_buff,len);
}

char* C8String::CopyString(void)
{
	Create_TempBuffer(m_len+1);
	Strcpy(temp_buff,m_buff);

	return temp_buff;
}

char* C8String::CopyStringLen(int len)
{
	Create_TempBuffer(len+1);
	CopyMemory(temp_buff,m_buff,len);

	return temp_buff;
}

void C8String::ReceiveBufferLen(LPCTSTR lpch,int len)
{
	m_len = len;
	Create_Buffer(m_len+1);
	CopyMemory(m_buff,(char *)lpch,len);
}

int C8String::Strlen(const char *string)
{
	if(string == NULL) return 0;								// ¿øÃµºÀ¼â
	return strlen((char *)string);
}

void C8String::Strcpy(char *buff, const char *string)
{
	if(string == NULL) ErrorMessage();
	strcpy(buff,(char *)string);
}

void C8String::Strncpy(char *buff, const char *string,int length)
{
	if(string == NULL) ErrorMessage();
	strncpy(buff,string,length);
}

void C8String::Strcat(char *buff, const char *string)
{
	if(string == NULL) ErrorMessage();
	strcat(buff,string);
}

void C8String::ErrorMessage()
{
	::MessageBox(NULL,"NULL °ªÀ» ÂüÁ¶ ÇÕ´Ï´Ù.","C8String ¿¡·¯",MB_OK);
	assert(0);
}

//--------------------------------------
// Ãß°¡ È®Àå ÇÔ¼ö
//--------------------------------------
char* C8String::BeginLeftChar(char ch)
{
	int i;

	i = Find(ch);

	Create_TempBuffer(i+1);

	Strncpy(temp_buff,m_buff,i);
	temp_buff[i] = NULL;

	return temp_buff;
}

char* C8String::AfterLeftChar(char ch)
{
	int i;

	i = Find(ch);

	Create_TempBuffer(i+1);

	Strcpy(temp_buff,Mid(i+1));

	return temp_buff;
}

char* C8String::BeginRightChar(char ch)
{
	int len,i;

	i = ReverseFind(ch);

	len = m_len-i;

	Create_TempBuffer(len);

	Strcpy(temp_buff,Mid(i+1));

	return temp_buff;
}

char* C8String::AfterRightChar(char ch)
{
	int i;

	i = ReverseFind(ch);

	Create_TempBuffer(i+1);

	Strcpy(temp_buff,Left(i));

	return temp_buff;
}