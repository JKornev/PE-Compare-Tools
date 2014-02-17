
// PE Compare Tools.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "PETools.h"
#include "PEToolsDlg.h"
#include <Windows.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPECompareToolsApp

BEGIN_MESSAGE_MAP(CPECompareToolsApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CPECompareToolsApp construction

CPECompareToolsApp::CPECompareToolsApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CPECompareToolsApp object

CPECompareToolsApp theApp;


// CPECompareToolsApp initialization

BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	LUID luid;
	BOOL bRet=FALSE;

	if (LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
	{
		TOKEN_PRIVILEGES tp;

		tp.PrivilegeCount=1;
		tp.Privileges[0].Luid=luid;
		tp.Privileges[0].Attributes=(bEnablePrivilege) ? SE_PRIVILEGE_ENABLED: 0;
		//
		//  Enable the privilege or disable all privileges.
		//
		if (AdjustTokenPrivileges(hToken, FALSE, &tp, NULL, (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
		{
			//
			//  Check to see if you have proper access.
			//  You may get "ERROR_NOT_ALL_ASSIGNED".
			//
			bRet=(GetLastError() == ERROR_SUCCESS);
		}
	}
	return bRet;
}

BOOL CPECompareToolsApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	//Set debug privilege
	HANDLE hProcess=GetCurrentProcess();
	HANDLE hToken;
	if (OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken)) {
		SetPrivilege(hToken, SE_DEBUG_NAME, TRUE);
		CloseHandle(hToken);
	}

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CPECompareToolsDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

