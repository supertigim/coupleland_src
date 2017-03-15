#ifndef _C7STRING_
#define _C7STRING_

// 8String.h: interface for the C8String class.
//
//////////////////////////////////////////////////////////////////////
class C7String  
{
private:
	char	temp_buff[255];
	int		m_len;

public:
	C7String();
	virtual ~C7String();

	bool	IsEmpty(char *buff);
	void	Empty(char *buff);
	int		Compare(char *buff,char *lpsz) const;
	char*	Mid(char *buff,int nFirst);
	char*	Mid(char *buff,int nFirst, int nCount);
	char*	Left(char *buff,int nCount);
	char*	Right(char *buff,int nCount);
	char*	Spanlncluding(char *buff,char *lpszCharSet);
	char*	SpanExcluding(char *buff,char *lpszCharSet);
	void	MakeUpper(char *buff);
	void	MakeLower(char *buff);
	void	MakeReverse(char *buff);
	int		Replace(char *buff,char chOld,char chNew);
	int		Remove(char *buff,char ch);
	int		Insert(char *buff,int nlindex,char ch);
	int		Insert(char *buff,int nlindex,char *pstr);
	int		Delete(char *buff,int nlindex,int nCount);
	int		Delete(char *buff,int nlindex,int nCount,int t_len);	// NULL
	void	Format(char *buff,char *lpszFormat, ...);
	void	TrimLeft(char *buff,char *chTargets);
	void	TrimLeft(char *buff);
	void	TrimLeft(char *buff,char chTarget);
	void	TrimRight(char *buff,char *chTargets);
	void	TrimRight(char *buff);
	void	TrimRight(char *buff,char chTarget);
	int		Find(char *buff,char ch);
	int		Find(char *buff,char *lpszSub);
	int		Find(char *buff,char ch,int nStart);
	int		Find(char *buff,char *sptr,int nStart);
	int		ReverseFind(char *buff,char ch);
	int		FindOneOf(char *buff,char *lpszCharSet);
	char*	GetBuffer(int nMinBufLength);
	//==================================================================
	// 추가 함수 
	//==================================================================
	void	CopyBuffer(char *buff,LPCTSTR lpch);
	void	CopyBufferLen(char *buff,LPCTSTR lpch,int len);
	void	AppendBufferLen(char *buff,LPCTSTR lpch,int nFirst,int nCount);
	//--------------------------------------
	// 추가 확장 함수
	//--------------------------------------
	char*	BeginLeftChar(char *buff,char ch);
	char*	AfterLeftChar(char *buff,char ch);
	char*	BeginRightChar(char *buff,char ch);
	char*	AfterRightChar(char *buff,char ch);

	int		Strlen(char *buff);
	void	Strcpy(char *buff, const char *string);
	void	Strncpy(char *buff, const char *string,int length);
	void	Strcat(char *buff, const char *string);
	void	Memcpy(char *buff, const char *string,int length);
};

#endif
