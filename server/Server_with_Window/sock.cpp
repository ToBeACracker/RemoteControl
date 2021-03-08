#include "sock.h"

SOCKET InitSocket()
{
	//0.windows需要WS2_32.dll
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 0);
	WSAStartup(sockVersion, &wsaData);

	//1.socket创建一个套接字
	SOCKET s = socket(
		AF_INET, //IPv4,网络层用IPv4协议
		SOCK_STREAM, //TCP, 传输层用TCP协议
		0);

	return s;
}

int ConnectServer(SOCKET s, const char ip[])
{
	if (s == INVALID_SOCKET)
		return SOCKET_ERROR;

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = inet_addr(ip);
	addr.sin_port = htons(8888);

	return connect(s, (PSOCKADDR)&addr, sizeof(SOCKADDR_IN));
}

SOCKET WaitConnection(SOCKET sListen)
{
	if (sListen == INVALID_SOCKET)
		return INVALID_SOCKET;

	//1.bind/listen
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = inet_addr("0.0.0.0");
	addr.sin_port = htons(8888);

	int nRet = bind(sListen, (PSOCKADDR)&addr, sizeof(SOCKADDR_IN));

	if (nRet == SOCKET_ERROR)
		return INVALID_SOCKET;

	nRet = listen(sListen, 1);

	if (nRet == SOCKET_ERROR)
		return INVALID_SOCKET;

	//2.accept
	SOCKADDR ClientAddr;
	int len = sizeof(SOCKADDR);

	return accept(sListen, &ClientAddr, &len);
}


BOOL SendBytes(SOCKET s, char buffer[], int BytesToSend)
{
	if (s == INVALID_SOCKET)
		return FALSE;

	int BytesSent = 0;
	int nRet = 0;

	for (; BytesSent < BytesToSend; BytesSent += nRet)
	{
		nRet = send(s, buffer + BytesSent, BytesToSend - BytesSent, 0);

		if (nRet == SOCKET_ERROR)
			return FALSE;
	}

	return TRUE;
}

BOOL RecvBytes(SOCKET s, char buffer[], int BytesToRecv)
{
	if (s == INVALID_SOCKET)
		return FALSE;

	int BytesRecv = 0;
	int nRet = 0;

	for (; BytesRecv < BytesToRecv; BytesRecv += nRet)
	{
		nRet = recv(s, buffer + BytesRecv, BytesToRecv - BytesRecv, 0);

		if (nRet == SOCKET_ERROR || nRet == 0)
			return FALSE;
	}

	return TRUE;
}







