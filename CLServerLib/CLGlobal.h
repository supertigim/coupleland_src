#ifndef _CL_GLOBAL_
#define _CL_GLOBAL_

//--------------------------------------------------------------//
//							Constants			                //
//--------------------------------------------------------------//

// [1] Buffer size
#define		DEF_MAXBUFSIZE			1024						// Mem Pool and Socket Buffer size 
//#define		MAXBUFSIZE			4096						// Mem Pool and Socket Buffer size in weding server
#define		DEF_MAXPACKETSIZE		DEF_MAXBUFSIZE				// same as MAXBUFSIZE 

#define		DEF_SQL_ORDER_LENGTH	500							// buffer size for SQL query

//#define		MAXUSER					10000	 				// Maximum connections
#define		DEF_MAXUSER				50	 						// Maximum connections for Test
#define		DEF_MAXCLIENT			MAXUSER						// same as MAXUSER 

// [2] Server Declarations
#define		IOCP_SHUTDOWN			((DWORD) -1L)				// Closing Server  
#define		DEF_SERVER_PORT			999							// PORT Number

#define		DEF_SERVER_NAME			"IOCP Base Library Server"	// 내서버 타입
#define		DEF_SERVER_VERSION		"B2002_04_03"

#define		CONNECT_CHECK_SIZE		3							// 초기 접속 오는 패킷 사이즈 ** WARINING **

#define		IP_LENGTH				15							// 실제 IP길이 

// 연결 형태 //
#define		UNKNOWN_CONNECTION		0							// 초기화 상태 
#define		CLIENT_CONNECTION		1							// 클라이언트와 연결 
#define		MRSERVER_CONNECTION		2							// 메세지 라우티 서버와 연결 


// [3] Timer 
#define		TIMELIMIT_PERIOD	30*1000 // 30초
#define		TIMELIMIT_SEND		10*1000 // 10초

// [4] Default log file 
#define		DEFAULT_LOG_FILE		"LogErrorFile.txt"

/*
// [5] Relating with Message Routing Server
#define		MRSERVER_IP_FILE		"MrsServer.txt"				// 파일 이름 
#define		MRS_PORT				55024						// Message Routing Server PORT
#define		MRS_CONN_NUM			40							// Message Routing Server 열결 수 
*/
//--------------------------------------------------------------//
//                Header Files & Load Library                   //
//--------------------------------------------------------------//
#define WIN32_LEAN_AND_MEAN										// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x500

#include<windows.h>												// win32 
#include<time.h>												// timer 
#include<assert.h>												// assert
#include<process.h>												// Thread 
#include<stdio.h>												// standard I/O

#include<winsock2.h>											// win32 socket 
#pragma comment(lib,"ws2_32.lib")								

#include<Mswsock.h>												// extension socket library 
#pragma comment(lib,"mswsock.lib")

#include "mmsystem.h"											// multimedia library 
#pragma comment(lib,"winmm.lib")

//--------------------------------------------------------------//
//                       	Structures                          //
//--------------------------------------------------------------//

// [1] 열거형 정의 : 클라이언트 작동 상태를 정의 한다 
typedef enum _CONN_STATUS_ 
{
	ClientIoUnknown,											// Raw status
	ClientIoAccept,												// accept status 
    ClientIoRead,												// reading status  
	ClientIoProcess												// Data Proccesing status 
} CONN_STATUS, *pCONN_STATUS;

// [2] 확장 오버랩 구조체 : IOCP처리시 사용 
typedef struct _OVERLAPPEDPLUS {
    OVERLAPPED		overlapped;
    SOCKET			sckListen,									// listen socket handle
					sckClient;									// send/recv socket handle
    CONN_STATUS		nConnState;									// operation flag
    WSABUF			wbuf;										// WSA buffer						
    DWORD			dwBytes,									// Processing Data Size
					dwFlags;									//
	PVOID			pClientConn;								// Processing Client 
	char			ConnType;
}OVERLAPPEDPLUS, *LPOVERLAPPEDPLUS;

//--------------------------------------------------------------//
//                     	External functions                      //
//--------------------------------------------------------------//

// [1] Writing Error Inforamtion 
static BOOL LogError(char* pszContext, char* pszFileName = DEFAULT_LOG_FILE)
{
	if(pszContext == NULL || pszFileName == NULL) return FALSE;
		
	FILE*		hLogFile = NULL;
	va_list		argptr;
	char		szOutStr[1024];

	hLogFile = fopen(pszFileName, "ab" );						// Open File Pointer 

	// 에러 검사 
	if(hLogFile == NULL) return FALSE;
	
	va_start(argptr, pszContext);
	vsprintf(szOutStr, pszContext, argptr);
	va_end(argptr);

	// 시간, 날짜 
	char	szDate[32],
			szTime[32];
	_tzset();
	_strdate( szDate );
	_strtime( szTime );

	int nBytesWritten = fprintf(hLogFile, "%s  [날짜:%s 시간:%s]\r\n", szOutStr, szDate, szTime );// LOG 내용 

	fflush( hLogFile );
	fclose( hLogFile );
	return TRUE;
}
extern BOOL LogError(char* pszContext, char* pszFileName);
#endif

