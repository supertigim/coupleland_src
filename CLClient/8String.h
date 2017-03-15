// 8String.h: interface for the C8String class.
//
//////////////////////////////////////////////////////////////////////

class C8String  
{
private:
	char	*temp_buff;
	int		m_len;
	bool	m_BufferLock;
	char	*m_buff;

public:
	C8String();
	C8String(const char *const String);
	virtual ~C8String();

	int		GetLength(void);
	bool	IsEmpty(void);
	void	Empty(void);
	char	GetAt(int nlindex);
	void	SetAt(int nlindex, char ch);
	int		Compare(char *lpsz) const;
	char*	Mid(int nFirst);
	char*	Mid(int nFirst, int nCount);
	char*	Left(int nCount);
	char*	Right(int nCount);
	char*   Spanlncluding(char *lpszCharSet);
	char*	SpanExcluding(char *lpszCharSet);
	void	MakeUpper(void);
	void	MakeLower(void);
	void	MakeReverse(void);
	int		Replace(char chOld,char chNew);
	int		Remove(char ch);
	int		Insert(int nlindex,char ch);
	int		Insert(int nlindex,char *pstr);
	int		Delete(int nlindex,int nCount);
	void 	Format(char *lpszFormat, ...);
	void	TrimLeft(char *chTargets);
	void	TrimLeft();
	void	TrimLeft(char chTarget);
	void	TrimRight(char *chTargets);
	void	TrimRight();
	void	TrimRight(char chTarget);
	int		Find(char ch);
	int		Find(char *lpszSub);
	int		Find(char ch,int nStart);
	int		Find(char *sptr,int nStart);
	int		ReverseFind(char ch);
	int		FindOneOf(char *lpszCharSet);
	char*	GetBuffer(int nMinBufLength);
	void	ReleaseBuffer();
	void	LockBuffer(void);
	void	UnlockBuffer(void);
//----------------------------------------------------
// �߰� �Լ�
//----------------------------------------------------
	void	CopyBuffer(LPCTSTR lpch);
	char*	CopyString(void);

//----------------------------------------------------
// CopyMemory�� �̿��� �Լ� (NULL���� ���� �����ϰ� �۾�)
//----------------------------------------------------
	void	CopyBufferLen(LPCTSTR lpch,int len);
	void	ReceiveBufferLen(LPCTSTR lpch,int len);
	char*	CopyStringLen(int len);

//----------------------------------------------------
// �߰� Ȯ�� �Լ�
//----------------------------------------------------
	char*	BeginLeftChar(char ch);
	char*	AfterLeftChar(char ch);
	char*	BeginRightChar(char ch);
	char*	AfterRightChar(char ch);

//--------------------------------------
	char&		operator[] (unsigned short offset);
	C8String	operator+ (const C8String &rhs);
	void		operator+= (const C8String &rhs);
	C8String&	operator= (const C8String &rhs);

private:

	void	Create_TempBuffer(int len);
	void	Create_Buffer(int len);

	int		Strlen(const char *string);
	void	Strcpy(char *buff, const char *string);
	void	Strncpy(char *buff, const char *string,int length);
	void	Strcat(char *buff, const char *string);
	void	ErrorMessage();
};
