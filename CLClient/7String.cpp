// 8String.cpp: implementation of the C7String class.
//
//////////////////////////////////////////////////////////////////////

#include "stdAfx.h"
//#include "ProtoGlobal.h"
#include "7String.h"
#include "assert.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

C7String::C7String()
{

}

C7String::~C7String()
{

}

bool C7String::IsEmpty(char *buff)
{
	if(buff) return false;
	else return true;
}

void C7String::Empty(char *buff)
{
	buff[0] = NULL;
}

int C7String::Compare(char *buff,char *lpsz) const
{
	if(buff == NULL || lpsz == NULL) return -1;			// ¿øÃµºÀ¼â

	return strcmp(buff,lpsz);
}

char* C7String::Mid(char *buff,int nFirst)
{
	m_len = Strlen(buff);
	
	int nCount = m_len - nFirst; 

	return Mid(buff,nFirst, nCount);
}

//	NPF	//
char* C7String::Mid(char *buff,int nFirst, int nCount)
{
	int len = nCount;

	for(int i=0;i<len;i++)
	{
		temp_buff[i] = buff[i+nFirst];
	}
	temp_buff[i] = NULL;

	return temp_buff;
}

//	NPF	//
char* C7String::Left(char *buff,int nCount)
{
	Memcpy(temp_buff,buff,nCount);
	temp_buff[nCount] = NULL;

	return temp_buff;
}

//	NPF	//
char* C7String::Right(char *buff,int nCount)
{
	for(int i=0;i<nCount;i++)
	{
		temp_buff[i] = buff[(m_len-nCount)+i];
	}
	temp_buff[i] = NULL;

	return temp_buff;
}

char* C7String::Spanlncluding(char *buff,char *lpszCharSet)
{
	bool ck;
	int len;

	len = Strlen(lpszCharSet);
	m_len = Strlen(buff);

	if(m_len+1 > 255) return NULL;
	Strcpy(temp_buff,buff);

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

char* C7String::SpanExcluding(char *buff,char *lpszCharSet)
{
	bool ck = false;
	int len;

	len = Strlen(lpszCharSet);
	m_len = Strlen(buff);

	if(m_len+1 > 255) return NULL;
	Strcpy(temp_buff,buff);

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

void C7String::MakeUpper(char *buff)
{
	if(buff == NULL) return;								// ¿øÃµºÀ¼â
	strupr(buff);
}

void C7String::MakeLower(char *buff)
{
	if(buff == NULL) return;								// ¿øÃµºÀ¼â
	strlwr(buff);
}

void C7String::MakeReverse(char *buff)
{
	if(buff == NULL) return;								// ¿øÃµºÀ¼â
	strrev(buff);
}

int C7String::Replace(char *buff,char chOld,char chNew)
{
	int count = 0;

	m_len = Strlen(buff);

	for(int i=0;i<m_len;i++)
	{
		if(buff[i] == chOld) 
		{
			buff[i] = chNew;
			count++;
		}
	}

	return count;
}

int C7String::Remove(char *buff,char ch)
{
	m_len = Strlen(buff);

	int count = 0;

	if(m_len+1 > 255) return -1;

	for(int i=0,j=0;i<m_len;i++)
	{
		if(buff[i] != ch) temp_buff[j++] = buff[i];
	}
	temp_buff[j] = NULL;

	count = m_len - j;

	Strcpy(buff,temp_buff);
	return count;
}

int C7String::Insert(char *buff,int nlindex,char ch)
{
	m_len = Strlen(buff);

	if(nlindex < 0) return m_len;

	if(m_len+2 > 255) return -1;

	if(nlindex < m_len)
	{
		for(int j=0,i=0;i<m_len;i++)
		{
			if(i == nlindex) temp_buff[j++] = ch;
			temp_buff[j++] = buff[i];
		}
	}
	else
	{
		Strcpy(temp_buff,buff);
		temp_buff[m_len] = ch;
	}
	temp_buff[m_len+1] = NULL;

	if(m_len+2 > 255) return -1;
	Strcpy(buff,temp_buff);
	m_len++;

	return m_len;
}

int C7String::Insert(char *buff,int nlindex,char *pstr)
{
	m_len = Strlen(buff);

	if(nlindex < 0) return m_len;

	int len = Strlen(pstr);

	if(m_len+len+1 > 255) return -1;

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
			temp_buff[j++] = buff[i];
		}
		temp_buff[j] = NULL;
	}
	else
	{
		Strcpy(temp_buff,buff);
		Strcat(temp_buff,pstr);
	}

	if(m_len+len+1 > 255) return -1;
	Strcpy(buff,temp_buff);
	
	m_len += len;

	return m_len;
}

int C7String::Delete(char *buff,int nlindex,int nCount = 1)
{
	m_len = Strlen(buff);

	int len = nCount;

	if(nlindex < 0 || nlindex >= m_len || nCount < 0) return m_len;
	if((nlindex+nCount) > m_len) len = m_len - nlindex;

	if((m_len-len)+1 > 255) return -1;

	for(int j=0,i=0;i<m_len-len;i++)
	{
		if(i == nlindex) j += nCount;
		temp_buff[i] = buff[j++];
	}
	temp_buff[i] = NULL;

	m_len -= len;

	if(m_len+1 > 255) return -1;
	Strcpy(buff,temp_buff);

	return m_len;
}

//	NPF	//
int C7String::Delete(char *buff,int nlindex,int nCount,int t_len)
{
	int len = nCount;

	if(nlindex < 0 || nlindex >= t_len || nCount < 0) return t_len;
	if((nlindex+nCount) > t_len) len = t_len - nlindex;

	if((t_len-len)+1 > 255) return -1;

	for(int j=0,i=0;i<t_len-len;i++)
	{
		if(i == nlindex) j += nCount;
		temp_buff[i] = buff[j++];
	}
	temp_buff[i] = NULL;

	t_len -= len;

	if(t_len+1 > 255) return -1;
	Memcpy(buff,temp_buff,t_len+1);

	return t_len;
}

void C7String::Format(char *buff,char *lpszFormat, ...)
{
	va_list		argptr;

	va_start(argptr, lpszFormat);
	vsprintf(buff, lpszFormat, argptr);
	va_end(argptr);
}

void C7String::TrimLeft(char *buff,char *chTargets)
{
	bool ck = false;
	int j,len;

	len = Strlen(chTargets);
	m_len = Strlen(buff);

	if(m_len+1 > 255 ) return;

	Strcpy(temp_buff,buff);

	for(int i=0;i<m_len;i++)
	{
		if(ck == false)
		{
			for(j=0;j<len;j++) 
			{
				if(buff[i] == chTargets[j]) break;
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
			temp_buff[j++] = buff[i];
		}
	}
	temp_buff[j] = NULL;

	m_len = Strlen(temp_buff);
	if(m_len+1 > 255) return;

	Strcpy(buff,temp_buff);
}

void C7String::TrimLeft(char *buff)
{
	TrimLeft(buff," ");
}

void C7String::TrimLeft(char *buff,char chTarget)
{
	char _buff[2];

	_buff[0] = chTarget;
	_buff[1] = NULL;

	TrimLeft(buff,_buff);
}

void C7String::TrimRight(char *buff,char *chTargets)
{
	int j,len;

	len = Strlen(chTargets);
	m_len = Strlen(buff);

	if(m_len+1 > 255) return;
	Strcpy(temp_buff,buff);

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
	if(m_len+1 > 255) return;

	Strcpy(buff,temp_buff);
}

void C7String::TrimRight(char *buff)
{
	TrimRight(buff," ");
}

void C7String::TrimRight(char *buff,char chTarget)
{
	char _buff[2];

	_buff[0] = chTarget;
	_buff[1] = NULL;

	TrimRight(buff,_buff);
}

int C7String::Find(char *buff,char ch)
{
	m_len = Strlen(buff);

	for(int i=0;i<m_len;i++)
	{
		if(buff[i] == ch) break;
	}

	return i;
}

int C7String::Find(char *buff,char *lpszSub)
{
	char *ptr;

	m_len = Strlen(buff);
	
	ptr = strstr(buff,lpszSub);

	return m_len - Strlen(ptr);
}

int C7String::Find(char *buff,char ch,int nStart)
{
	m_len = Strlen(buff);

	if(nStart >= m_len) return -1;

	for(int i=nStart;i<m_len;i++)
	{
		if(buff[i] == ch) break;
	}

	return i;
}

int C7String::Find(char *buff,char *sptr,int nStart)
{
	m_len = Strlen(buff);

	if(nStart >= m_len) return -1;

	char *ptr;

	if((m_len-nStart)+1 > 255) return -1;

	Strcpy(temp_buff,Mid(buff,nStart,(m_len-nStart)));
	
	ptr = strstr(temp_buff,sptr);

	return m_len-Strlen(ptr);
}

int C7String::ReverseFind(char *buff,char ch)
{
	char *ptr;

	m_len = Strlen(buff);

	ptr = strrchr(buff,ch);

	return m_len - Strlen(ptr);
}

int C7String::FindOneOf(char *buff,char *lpszCharSet)
{
	if(buff == NULL || lpszCharSet == NULL) return -1;		// ¿øÃµºÀ¼â

	return strcspn(buff,lpszCharSet);
}

char* C7String::GetBuffer(int nMinBufLength)
{
	if(nMinBufLength+1 > 255) return NULL;

	return temp_buff;
}

//==================================================================
// Ãß°¡ ÇÔ¼ö 
//==================================================================
void C7String::CopyBuffer(char *buff,LPCTSTR lpch)
{
	if(buff == NULL || lpch == NULL) return;				// ¿øÃµºÀ¼â

	Strcpy(buff,(char *)lpch);
}

//NPF
void C7String::CopyBufferLen(char *buff,LPCTSTR lpch,int len)
{
	Memcpy(buff,(char *)lpch,len);
}

void C7String::AppendBufferLen(char *buff,LPCTSTR lpch,int nFirst,int nCount)
{
	for(int i=0;i<nCount;i++)
	{
		buff[i+nFirst] = lpch[i];
	}
}

int C7String::Strlen(char *buff)
{
	if(buff == NULL) return 0;								// ¿øÃµºÀ¼â
	return strlen(buff);
}

void C7String::Strcpy(char *buff, const char *string)
{
	if(string == NULL) return;								// ¿øÃµºÀ¼â
	strcpy(buff,(char *)string);
}

void C7String::Strncpy(char *buff, const char *string,int length)
{
	if(string == NULL) return;								// ¿øÃµºÀ¼â
	strncpy(buff,string,length);
}

void C7String::Strcat(char *buff, const char *string)
{
	if(string == NULL) return;								// ¿øÃµºÀ¼â
	strcat(buff,string);
}

void C7String::Memcpy(char *buff, const char *string,int length)
{
	if(string == NULL) return;								// ¿øÃµºÀ¼â
	CopyMemory(buff,string,length);
}

//--------------------------------------
// Ãß°¡ È®Àå ÇÔ¼ö
//--------------------------------------
char* C7String::BeginLeftChar(char *buff,char ch)
{
	int i;

	i = Find(buff,ch);

	if(i+1 > 255) return NULL;

	Strncpy(temp_buff,buff,i);
	temp_buff[i] = NULL;

	return temp_buff;
}

char* C7String::AfterLeftChar(char *buff,char ch)
{
	int i;

	i = Find(buff,ch);

	if(i+1 > 255) return NULL;

	Strcpy(temp_buff,Mid(buff,i+1));

	return temp_buff;
}

char* C7String::BeginRightChar(char *buff,char ch)
{
	int len,i;

	m_len = Strlen(buff);

	i = ReverseFind(buff,ch);

	len = m_len-i;

	if(len > 255) return NULL;

	Strcpy(temp_buff,Mid(buff,i+1));

	return temp_buff;
}

char* C7String::AfterRightChar(char *buff,char ch)
{
	int i;

	i = ReverseFind(buff,ch);

	if(i+1 > 255) return NULL;

	Strcpy(temp_buff,Left(buff,i));

	return temp_buff;
}


