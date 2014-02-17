#pragma once

#include "afxcmn.h"
#include "afxwin.h"
//#include <vector>
#include <list>

#include "CmpDispatcher.h"


// CPECompareToolsDlg dialog
class CPECompareToolsDlg : public CDialogEx
{
// Construction
public:
	CPECompareToolsDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PECOMPARETOOLS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

private:

	CCmpDispatcher _disp;

	std::list<unsigned int> _app_list;

	enum { REFRESH_TIMER_ID = 1234 };
	UINT_PTR _timer_id;

	void CheckButtonsState();
	void CheckStartButtonsState();
	bool GetModuleStrName(CString &str, DWORD pid, HMODULE hmod);
	void RefreshSections();
	void RefreshInform();
	void RefreshCmpInform();
	void AppendSaveBuf();

	static void stop_compare_callback(void *param);
	static bool enum_diffs_callback(DWORD offset, unsigned int size, unsigned int id, void *params);
	static bool add_to_list_callback(DWORD offset, unsigned int size, unsigned int id, void *params);
	void ConvSecToTimer(CString &str, unsigned int sec);
	unsigned int GetAppNumById(unsigned int id);

	unsigned int _last_id;
	UAddress _last_addr;
	int _out_type;
	CString *_out_str;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl CListSectors;
	afx_msg void OnHdnBegintrackList1(NMHDR *pNMHDR, LRESULT *pResult);
	CListCtrl CListApps;
	CListCtrl CListDiff;
	CComboBox CComboOpenBase;
	CComboBox CComboRepeat;
	CComboBox CComboOutStuct;
	CButton CButtonCmp;
	CButton CButtonSave;
	//afx_msg void OnLvnItemchangedList3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButton4();
	CButton CButtonAligm;
	CButton CButtonDelApp;
	afx_msg void OnNMClickSyslink1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButton6();
	CEdit CEditPath;
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton7();
	CButton CButtonAddMod;
	CButton CButtoAddFile;
	afx_msg void OnBnClickedButton3();
	afx_msg void OnItemdblclickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRclickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedOk2();
	afx_msg void OnBnClickedCancel();
protected:
//	afx_msg LRESULT OnUserStopCmp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserStopCmp(WPARAM wParam, LPARAM lParam);
//	afx_msg LRESULT OnTetest(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedButton5();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CComboBox CComboOffset;
};
