// CLServerLib.cpp : Defines the entry point for the console application.
//
#include "ServerCtrl.h"

BOOL __stdcall ConsoleHandler( DWORD ConsoleEvent );			// 콘솔 핸들 선언 

CServerCtrl	server;

// Function of React In Console Mode
BOOL __stdcall ConsoleHandler( DWORD ConsoleEvent )
{
	switch ( ConsoleEvent )
	{
	case CTRL_LOGOFF_EVENT:		// FALL THROUGH
	case CTRL_C_EVENT:			// FALL THROUGH
	case CTRL_BREAK_EVENT:		// FALL THROUGH
	case CTRL_CLOSE_EVENT:		// FALL THROUGH
	case CTRL_SHUTDOWN_EVENT:
		{
			server.Stop();
			return TRUE;
		}
		// FALL THROUGH because we cannot abort right now
	default:
		return FALSE;
	}
}

int main(int argc, char* argv[])
{
	// set control-c break handler for console application
	SetConsoleCtrlHandler( ConsoleHandler, TRUE );

	server.Run();

	// reset control-c break handler for console application
	SetConsoleCtrlHandler( ConsoleHandler, FALSE );
	return 0;
}
