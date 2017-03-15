==================================================================================
							CoupleLand Server Library 
==================================================================================
	[1] Description : This Code is using for "CoupleLand Online Service".
					  Implementation of the IOCP(Input/Output Completion Port) 
					  Windows Advanced Technology,
					  Network Communication based on TCP Protocol, 
					  Fast Encryption/Decryption preventing Hacking,
					  Detecting a ghost client connection for maintaining 
					  optimized server surrounding,
					  Memory Pool System for preventing access violention error,
					  and Multi threading technolgy are included in this server.
	[2] Date		: April 03, 2002
	[3] Developler	: Jay Kim (Kim Jong Yoon)
	
==================================================================================

- CLGloabl.h - 
 All declaration using server code, defined structures, external functions, header files 
and addtional libraries are included. 

- MemPool.h - 
 Actual memery pooling class for each processing routine(Accept, Read, Data Processing).
Memory Allocation function is 'VirtualAlloc(...)'. 

- PacketPool.h, PacketPool.cpp - 
 This class controls all memory pools(CMemPool class). If you want to record errors happening 
in the class, you may enter the name of error file in second factor of PacketPool class Member 
function 'CPacket::Create(1st factor,2rd factor)' when start server.

- Connection.h, Connection.cpp - 
 This class controls all client requests such as connection, receiving data, sending data and so on.

- ServerCtrl.h, ServerCtrl.cpp - 
 This class is actually running server module. Memories needed in this server and all objects are
intialized in the class. Especially, if an error is occurred when is initialized, server is shutdown.

///////////////////////////////////////////////////////////////////////////////////
