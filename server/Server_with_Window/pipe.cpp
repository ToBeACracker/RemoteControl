#include "pipe.h"

BOOL RedirectCmd(HANDLE &WriteCmd, HANDLE &ReadCmd)
{
	//1.CreatePipe
	HANDLE CmdReadPipe;
	HANDLE CmdWritePipe;

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	int nRet = CreatePipe(&ReadCmd, &CmdWritePipe, &sa, 0);
	if (!nRet)
		return FALSE;

	nRet = CreatePipe(&CmdReadPipe, &WriteCmd, &sa, 0);
	if (!nRet)
		return FALSE;

	//2.Create child process(cmd.exe)
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi;

	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = CmdReadPipe;
	si.hStdOutput = CmdWritePipe;
	si.hStdError = CmdWritePipe;

	nRet = CreateProcess(
		__TEXT("C:\\Windows\\System32\\cmd.exe"),
		NULL,
		NULL,
		NULL,
		TRUE,
		CREATE_NO_WINDOW,
		NULL,
		NULL,
		&si, //子进程当前目录默认与父进程相同
		&pi
	);

	if (!nRet)
		return FALSE;

	return TRUE;
}
