#pragma once
#include "afxcmn.h"

// CAddModuleDlg dialog
#include <Windows.h>
#include <vector>

typedef struct _Selected_Module {
	DWORD pid;
	HMODULE hmod;
} Selected_Module, *PSelected_Module;

class CAddModuleDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAddModuleDlg)

	enum { BUF_MAX_COUNT = 512 };

	std::vector<DWORD> _pids;
	std::vector<HMODULE> _hmods;
	//DWORD *_pid_list;
	//DWORD _pid_count;

	//HMODULE *_hmod_list;
	//DWORD _hmod_count;
	int _selected_pid;
	int _selected_hmod;
	PSelected_Module _out;

	BOOL _is_x64;

	bool GetProcName(TCHAR *buf, UINT size, DWORD pid);
	void RefreshProcList();
	void RefreshModList(UINT pid);
	void SelectProc(int inx);
	void SelectMod(int inx);
public:
	CAddModuleDlg(PSelected_Module out, CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddModuleDlg();

// Dialog Data
	enum { IDD = IDD_ADD_PROC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CListCtrl CListProc;
	CListCtrl CListMod;
	afx_msg void OnBnClickedButton8();
	afx_msg void OnNMClickList7(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBegindragList7(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBegindragList6(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClickList6(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBegintrackList7(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOk();
};
