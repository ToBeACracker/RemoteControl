
// Client_with_Window_MFC.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CClient:
// See Client_with_Window_MFC.cpp for the implementation of this class
//

class CClient : public CWinApp
{
public:
	CClient();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CClient theApp;
