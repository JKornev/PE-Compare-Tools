
// PE Compare Tools.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CPECompareToolsApp:
// See PE Compare Tools.cpp for the implementation of this class
//

class CPECompareToolsApp : public CWinApp
{
public:
	CPECompareToolsApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CPECompareToolsApp theApp;