#include "error.h"

void PrintError()
{
	LPTSTR pMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&pMsgBuf,
		0, NULL
	);

	OutputDebugString(pMsgBuf);

	LocalFree(pMsgBuf);
}