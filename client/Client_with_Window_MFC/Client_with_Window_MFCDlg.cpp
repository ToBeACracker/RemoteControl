
// Client_with_Window_MFCDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "Client_with_Window_MFC.h"
#include "Client_with_Window_MFCDlg.h"
#include "afxdialogex.h"
#include "sock.h"
#include "DataPacket.h"
#include "error.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CClientwithWindowMFCDlg dialog



CClientwithWindowMFCDlg::CClientwithWindowMFCDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLIENT_WITH_WINDOW_MFC_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientwithWindowMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_editCmd);
	DDX_Control(pDX, IDC_EDIT2, m_outCmd);
	DDX_Control(pDX, IDC_STATIC2, m_outKey);
}

BEGIN_MESSAGE_MAP(CClientwithWindowMFCDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CClientwithWindowMFCDlg message handlers
DWORD WINAPI ShowResult(LPVOID lpParam);
BOOL CClientwithWindowMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//1.Init
	m_s = InitSocket();
	if (m_s == INVALID_SOCKET)
	{
		PrintError();
		PostQuitMessage(0);
		return FALSE;
	}

	//2.connect
	int nRet = ConnectServer(m_s, "127.0.0.1");
	if (nRet == SOCKET_ERROR)
	{
		PrintError();
		PostQuitMessage(0);
		return FALSE;
	}

	//3.Create ShowThread
	HANDLE hChildThread = CreateThread(NULL, 0, ShowResult, (LPVOID)m_s, 0, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}



// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CClientwithWindowMFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CClientwithWindowMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


BOOL CClientwithWindowMFCDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	//watch VK_RETURN and send command
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		//The handle to the window with the keyboard focus
		if (GetFocus() == (CWnd*)&m_editCmd)
		{
			char SendBuffer[256];
			DataPacket* pPkt = (DataPacket*)SendBuffer;
			char* pData = pPkt->data;

			GetWindowTextA(m_editCmd.m_hWnd, pData, sizeof(SendBuffer) - sizeof(DataPacket));

			DWORD nLen = strlen(pData);
			pData[nLen++] = '\n';

			pPkt->len = nLen;
			pPkt->type = PACKET_REQ_CMD;

			SendBytes(m_s, (char*)pPkt, pPkt->len + sizeof(pPkt));

			return TRUE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


DWORD WINAPI ShowResult(LPVOID lpParam)
{
	//注意缓存可能会爆
	static char cmdBuffer[10000] = { 0 };
	static DWORD len = 0;

	SOCKET s = (SOCKET)lpParam;

	CClientwithWindowMFCDlg* pDlg = (CClientwithWindowMFCDlg*)AfxGetApp()->m_pMainWnd;

	while (TRUE)
	{
		char RecvBuffer[1000] = { 0 };
		DataPacket* pPkt = (DataPacket*)RecvBuffer;

		//recv header
		if (RecvBytes(s, (char*)pPkt, sizeof(DataPacket)) == FALSE)
		{
			PrintError();
			closesocket(s);
			return 0;
		}

		//recv data
		RecvBytes(s, pPkt->data, pPkt->len);

		//handle data according to data type
		if (pPkt->type == PACKET_RLY_KEYBOARD)
		{
			CStatic* pOutKey = &pDlg->m_outKey;

			SetWindowTextA(pOutKey->m_hWnd, pPkt->data);
		}
		else if (pPkt->type == PACKET_RLY_CMD)
		{
			CEdit* pOutCmd = &pDlg->m_outCmd;

			memcpy(cmdBuffer + len, pPkt->data, pPkt->len);
			len += pPkt->len;
			SetWindowTextA(pOutCmd->m_hWnd, cmdBuffer);
		}
	}
}

void CClientwithWindowMFCDlg::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class
	if (m_s != INVALID_SOCKET)
	{
		closesocket(m_s);
	}
	WSACleanup();
	CDialogEx::PostNcDestroy();
}
