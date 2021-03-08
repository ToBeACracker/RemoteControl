// MyDll.cpp : Defines the exported functions for the DLL.
//


#include "pch.h"
#include "framework.h"
#include "MyDll.h"
#include <stdio.h>



//共享段 保证所有进程加载dll后 此数据保持一致
#pragma data_seg("shared")
HWND g_hMainWnd = NULL; //Win32 main window
#pragma data_seg()

LRESULT CALLBACK HookKeyboard(int code, WPARAM wParam, LPARAM lParam)
{
	COPYDATASTRUCT cds = { 0 };
	CHAR sz[30] = { 0 };

	wsprintfA(sz, "%c key pressed", wParam);
	cds.cbData = strlen(sz) + 1;
	cds.lpData = sz;

	SendMessage(g_hMainWnd, WM_COPYDATA, NULL, (LPARAM)&cds); //将按键记录发送到Win32 main window(server)

	return CallNextHookEx(NULL, code, wParam, lParam);
}


static HHOOK hhk = NULL;
HMODULE HandleFromAddress(LPVOID pAddr);

BOOL SetKeyHook(HWND hMainWnd)
{
	g_hMainWnd = hMainWnd;

	hhk = SetWindowsHookEx(
		WH_KEYBOARD, 
		HookKeyboard, 
		HandleFromAddress(HookKeyboard), //钩子回调函数所在模块
			0 //全局钩子
		);

	if (hhk == NULL)
		return FALSE;
	
	return TRUE;
}

void UnKeyHook()
{
	if (hhk != NULL)
	{
		UnhookWindowsHookEx(hhk);
		hhk = NULL;
	}
}

HMODULE HandleFromAddress(LPVOID pAddr)
{
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery(pAddr, &mbi, sizeof(mbi));
	return (HMODULE)mbi.AllocationBase;
}



