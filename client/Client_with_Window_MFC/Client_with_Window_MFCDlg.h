
// Client_with_Window_MFCDlg.h : header file
//

#pragma once


// CClientwithWindowMFCDlg dialog
class CClientwithWindowMFCDlg : public CDialogEx
{
// Construction
public:
	CClientwithWindowMFCDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIENT_WITH_WINDOW_MFC_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	SOCKET m_s;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CEdit m_editCmd;
	CEdit m_outCmd;
	CStatic m_outKey;
	virtual void PostNcDestroy();
};
