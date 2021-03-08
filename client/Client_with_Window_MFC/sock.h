#pragma once
#include "common.h"

//initialize server or client
//1.sucess:return new socket
//2.fail:return INVALID_SOCKET
SOCKET InitSocket(); 

//client connect Server
//1.fail:return SOCKET_ERROR
//2.otherwise, success
int ConnectServer(SOCKET s, const char ip[]);

//Server accept connection form client
//sucess:return new socket
//fail:return INVALID_SOCKET
SOCKET WaitConnection(SOCKET sListen);

BOOL SendBytes(SOCKET s, char buffer[], int BytesToSend);
BOOL RecvBytes(SOCKET s, char buffer[], int BytesToRecv);

