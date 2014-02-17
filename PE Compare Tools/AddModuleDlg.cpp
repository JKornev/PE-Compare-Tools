// AddModuleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PETools.h"
#include "AddModuleDlg.h"
#include "afxdialogex.h"
#include <psapi.h>

#pragma comment(lib, "psapi.lib")

// CAddModuleDlg dialog

IMPLEMENT_DYNAMIC(CAddModuleDlg, CDialogEx)
	
CAddModuleDlg::CAddModuleDlg(PSelected_Module out, CWnd* pParent /*=NULL*/)
	: CDialogEx(CAddModuleDlg::IDD, pParent),
	_is_x64(FALSE), _selected_pid(-1), _selected_hmod(-1)
{
	_pids.reserve(BUF_MAX_COUNT);
	_hmods.reserve(BUF_MAX_COUNT);
	_out = out;
}

CAddModuleDlg::~CAddModuleDlg()
{
}

void CAddModuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST7, CListProc);
	DDX_Control(pDX, IDC_LIST6, CListMod);
}

BEGIN_MESSAGE_MAP(CAddModuleDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON8, &CAddModuleDlg::OnBnClickedButton8)
	ON_NOTIFY(NM_CLICK, IDC_LIST7, &CAddModuleDlg::OnNMClickList7)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST7, &CAddModuleDlg::OnBegindragList7)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST6, &CAddModuleDlg::OnBegindragList6)
	ON_NOTIFY(NM_CLICK, IDC_LIST6, &CAddModuleDlg::OnClickList6)
	ON_NOTIFY(HDN_BEGINTRACK, 0, &CAddModuleDlg::OnBegintrackList7)
	ON_BN_CLICKED(IDOK, &CAddModuleDlg::OnBnClickedOk)
END_MESSAGE_MAP()

bool CAddModuleDlg::GetProcName(TCHAR *buf, UINT size, DWORD pid)
{
	HANDLE hproc;
	HMODULE hmod;
	DWORD needed;
	BOOL is_wow64 = false;

	hproc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (!hproc) {
		return false;
	}

	if (_is_x64) {
		IsWow64Process(hproc, &is_wow64);
	}

	if (!EnumProcessModules(hproc, &hmod, sizeof(hmod), &needed)) {
		return false;
	}

	GetModuleBaseName(hproc, hmod, buf, size);

#ifdef _WIN64
	if (is_wow64) {
		_tcscat(buf, TEXT(" *32"));
	}
#endif

	return true;
}

void CAddModuleDlg::RefreshProcList()
{
	TCHAR temp[MAX_PATH];
	DWORD pid_list[BUF_MAX_COUNT], pid_count;

	CListProc.DeleteAllItems();
	CListMod.DeleteAllItems();

	if (!EnumProcesses(pid_list, sizeof(DWORD) * BUF_MAX_COUNT, &pid_count) ) {
		return;
	}

	pid_count /= sizeof(DWORD);
	if (pid_count >= BUF_MAX_COUNT) {
		pid_count = BUF_MAX_COUNT - 1;
	}

	_pids.clear();

	for (unsigned int i = 0, inx; i < pid_count; i++) {
		if (!GetProcName(temp, MAX_PATH, pid_list[i])) {
			continue;
		}

		inx = _pids.size();
		CListProc.InsertItem(inx, temp);
		_stprintf_s(temp, TEXT("%d"), pid_list[i]);
		CListProc.SetItemText(inx, 1, temp);
		_pids.push_back(pid_list[i]);
	}

	SetDlgItemText(IDC_STATIC2, TEXT("PID: ???"));
	SetDlgItemText(IDC_STATIC3, TEXT("Module: ???"));

	_selected_pid = -1;
	_selected_hmod = -1;
}

void CAddModuleDlg::RefreshModList(UINT pid)
{
	TCHAR mod_name[MAX_PATH], 
		temp[MAX_PATH];
	HANDLE hproc;
	DWORD needed;

	CListMod.DeleteAllItems();
	if (CListProc.GetSelectedCount() == 0) {
		return;
	}

	hproc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (!hproc) {
		return;
	}

	HMODULE hmods[BUF_MAX_COUNT];
	if (!EnumProcessModules(hproc, (HMODULE *)&hmods, sizeof(hmods), &needed)) {
		CloseHandle(hproc);
		return;
	}

	needed /= sizeof(HMODULE);
	if (needed >= BUF_MAX_COUNT) {
		needed = BUF_MAX_COUNT - 1;
	}

	_hmods.clear();

	for (unsigned int i = 0, inx; i < needed; i++) {
		inx = _hmods.size();

		if (!GetModuleBaseName(hproc, hmods[i], temp, MAX_PATH)) {
			_stprintf_s(temp, TEXT("???"));
		}

#ifdef _WIN64
		_stprintf_s(mod_name, TEXT("%012X %s "), hmods[i], temp);
#else 
		_stprintf_s(mod_name, TEXT("%08X %s "), hmods[i], temp);
#endif

		CListMod.InsertItem(inx, mod_name);
		_hmods.push_back(hmods[i]);
	}

	SetDlgItemText(IDC_STATIC3, TEXT("Module: ???"));
	CloseHandle(hproc);
	_selected_hmod = -1;
}

void CAddModuleDlg::SelectProc(int inx)
{
	TCHAR label[MAX_PATH];

	RefreshModList(_pids[inx]);

	_stprintf_s(label, TEXT("PID: %d"), _pids[inx]);
	SetDlgItemText(IDC_STATIC2, label);
	_selected_pid = inx;
}

void CAddModuleDlg::SelectMod(int inx)
{
	TCHAR label[MAX_PATH];

#ifdef _WIN64
	_stprintf_s(label, TEXT("Module: %012X"), _hmods[inx]);
#else 
	_stprintf_s(label, TEXT("Module: %08X"), _hmods[inx]);
#endif

	SetDlgItemText(IDC_STATIC3, label);
	_selected_hmod = inx;
}

BOOL CAddModuleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	CListProc.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	CListProc.InsertColumn(0, TEXT(""), 0, 133);
	CListProc.InsertColumn(1, TEXT(""), 0, 40);

	CListMod.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	CListMod.InsertColumn(0, TEXT(""), 0, 175);

	SYSTEM_INFO sinfo;
	GetNativeSystemInfo(&sinfo);
	_is_x64 = (sinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? TRUE : FALSE);

	RefreshProcList();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CAddModuleDlg::OnBnClickedButton8()
{
	RefreshProcList();
}


void CAddModuleDlg::OnNMClickList7(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (pNMItemActivate->iItem != -1) {
		SelectProc(pNMItemActivate->iItem);
	}
	*pResult = 0;
}

void CAddModuleDlg::OnBegindragList7(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pNMLV->iItem != -1) {
		SelectProc(pNMLV->iItem);
	}
	*pResult = 0;
}


void CAddModuleDlg::OnBegindragList6(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pNMLV->iItem != -1) {
		SelectMod(pNMLV->iItem);
	}
	*pResult = 0;
}


void CAddModuleDlg::OnClickList6(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (pNMItemActivate->iItem != -1) {
		SelectMod(pNMItemActivate->iItem);
	}
	*pResult = 0;
}


void CAddModuleDlg::OnBegintrackList7(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}



void CAddModuleDlg::OnBnClickedOk()
{
	if (_selected_hmod == -1 || _selected_pid == -1) {
		MessageBox(TEXT("Please, select module"), TEXT("Error"), MB_ICONWARNING);
		return;
	}
	_out->hmod = _hmods[_selected_hmod];
	_out->pid = _pids[_selected_pid];
	CDialogEx::OnOK();
}
