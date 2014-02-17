#include "stdafx.h"
#include "PETools.h"
#include "PEToolsDlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include "AddModuleDlg.h"
#include "ChangeAlignDlg.h"
#include <Psapi.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_USER_STOP_CMP	WM_USER + 542

CPECompareToolsDlg::CPECompareToolsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPECompareToolsDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPECompareToolsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, CListSectors);
	DDX_Control(pDX, IDC_LIST2, CListApps);
	DDX_Control(pDX, IDC_LIST3, CListDiff);
	DDX_Control(pDX, IDC_COMBO3, CComboOpenBase);
	DDX_Control(pDX, IDC_COMBO2, CComboRepeat);
	DDX_Control(pDX, IDC_COMBO4, CComboOutStuct);
	DDX_Control(pDX, IDOK, CButtonCmp);
	DDX_Control(pDX, IDOK2, CButtonSave);
	DDX_Control(pDX, IDC_BUTTON4, CButtonAligm);
	DDX_Control(pDX, IDC_BUTTON5, CButtonDelApp);
	DDX_Control(pDX, IDC_EDIT1, CEditPath);
	DDX_Control(pDX, IDC_BUTTON8, CButtonAddMod);
	DDX_Control(pDX, IDC_BUTTON7, CButtoAddFile);
	DDX_Control(pDX, IDC_COMBO6, CComboOffset);
}

BEGIN_MESSAGE_MAP(CPECompareToolsDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(HDN_BEGINTRACK, 0, &CPECompareToolsDlg::OnHdnBegintrackList1)
	//ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST3, &CPECompareToolsDlg::OnLvnItemchangedList3)
	ON_BN_CLICKED(IDC_BUTTON4, &CPECompareToolsDlg::OnBnClickedButton4)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK1, &CPECompareToolsDlg::OnNMClickSyslink1)
	ON_BN_CLICKED(IDC_BUTTON6, &CPECompareToolsDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON8, &CPECompareToolsDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON7, &CPECompareToolsDlg::OnBnClickedButton7)
	ON_NOTIFY(HDN_ITEMDBLCLICK, 0, &CPECompareToolsDlg::OnItemdblclickList1)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CPECompareToolsDlg::OnRclickList1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CPECompareToolsDlg::OnDblclkList1)
	ON_BN_CLICKED(IDOK, &CPECompareToolsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDOK2, &CPECompareToolsDlg::OnBnClickedOk2)
	ON_BN_CLICKED(IDCANCEL, &CPECompareToolsDlg::OnBnClickedCancel)
	ON_MESSAGE(WM_USER_STOP_CMP, &CPECompareToolsDlg::OnUserStopCmp)
	ON_BN_CLICKED(IDC_BUTTON5, &CPECompareToolsDlg::OnBnClickedButton5)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CPECompareToolsDlg message handlers

BOOL CPECompareToolsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);	
	SetIcon(m_hIcon, FALSE);

	CImageList *img_list = new CImageList();
	CBitmap bm;
	img_list->Create(15, 15, TRUE, 0, 1);
	bm.LoadBitmap(IDB_BITMAP1);
	img_list->Add(&bm, RGB(192, 192, 192));

	CListSectors.SetImageList(img_list, LVSIL_SMALL);
	CListSectors.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	CListSectors.InsertColumn(0, TEXT("Sector"), LVCFMT_LEFT, 79);
	CListSectors.InsertColumn(1, TEXT("Flags"), LVCFMT_CENTER, 55);
	CListSectors.InsertColumn(2, TEXT("Align"), LVCFMT_CENTER, 45);

	CListApps.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	CListApps.InsertColumn(0, TEXT("#"), LVCFMT_CENTER, 30);
	CListApps.InsertColumn(1, TEXT("Type"), LVCFMT_CENTER, 50);
	CListApps.InsertColumn(3, TEXT("Path"), LVCFMT_LEFT, 259);

	CListDiff.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	CListDiff.InsertColumn(0, TEXT("#"), LVCFMT_CENTER, 38);
	CListDiff.InsertColumn(1, TEXT("App"), LVCFMT_CENTER, 33);
	CListDiff.InsertColumn(2, TEXT("Offset"), LVCFMT_LEFT, 65);
	CListDiff.InsertColumn(3, TEXT("Size"), LVCFMT_LEFT, 55);

	CComboOpenBase.AddString(TEXT("File"));
	CComboOpenBase.AddString(TEXT("Module"));
	//CComboOpenBase.SelectString(0, TEXT("File"));
	CComboOpenBase.SetCurSel(0);

	CComboRepeat.AddString(TEXT("Scan once"));
	CComboRepeat.AddString(TEXT("Custom"));
	//CComboRepeat.SelectString(0, TEXT("Scan once"));
	CComboRepeat.SetCurSel(0);

	CComboOutStuct.AddString(TEXT("Normal HEX"));
	CComboOutStuct.AddString(TEXT("C-style struct"));
	//CComboOutStuct.SelectString(0, TEXT("Normal HEX"));
	CComboOutStuct.SetCurSel(0);

	CComboOffset.AddString(TEXT("Relative Virtual Offset"));
	CComboOffset.AddString(TEXT("Relative File Offset"));
	CComboOffset.AddString(TEXT("Virtual Address"));
	//CComboOffset.SelectString(0, TEXT("Relative Virtual Offset"));
	CComboOffset.SetCurSel(0);

	CheckButtonsState();
	CheckStartButtonsState();
	RefreshInform();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPECompareToolsDlg::OnPaint()
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

void CPECompareToolsDlg::CheckButtonsState()
{
	bool state = _disp.IsBaseAppOpened();
	CButtonAligm.EnableWindow(state);
	CButtonDelApp.EnableWindow(state);
	CButtonAddMod.EnableWindow(state);
	CButtoAddFile.EnableWindow(state);
}

void CPECompareToolsDlg::CheckStartButtonsState()
{
	bool save_state = (_disp.GetCompareState() == 0 && _disp.GetDiffCount() > 0 ? true : false),
		cmp_state = _disp.IsBaseAppOpened();
	CButtonSave.EnableWindow(save_state);
	CButtonCmp.EnableWindow(cmp_state);
}

bool CPECompareToolsDlg::GetModuleStrName(CString &str, DWORD pid, HMODULE hmod)
{
	TCHAR path[MAX_PATH];
	HMODULE hproc_mod;
	DWORD needed;

	if (!str.IsEmpty()) {
		str.Empty();
	}

	HANDLE hproc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (!hproc) {
		return false;
	}

	if (!EnumProcessModules(hproc, &hproc_mod, sizeof(hproc_mod), &needed)) {
		return false;
	}
	GetModuleBaseName(hproc, hproc_mod, path, MAX_PATH);
	str.Format(TEXT("%s pid:%d ("), path, pid);

	if (!GetModuleBaseName(hproc, hmod, path, MAX_PATH)) {
		CloseHandle(hproc);
		return false;
	}

	str += path;

#ifdef _WIN64
	str.AppendFormat(TEXT(" %012X)"), hmod);
#else
	str.AppendFormat(TEXT(" %08X)"), hmod);
#endif

	CloseHandle(hproc);
	return true;
}

void CPECompareToolsDlg::RefreshSections()
{
	PIMAGE_SECTION_HEADER psect;
	UINT sec_count;
	IPEManager *mngr;
	char name[IMAGE_SIZEOF_SHORT_NAME + 1] = {};
	CString str;
	int def_img;

	CListSectors.DeleteAllItems();

	if (_disp.IsBaseAppOpened()) {
		mngr = _disp.GetBaseInstance();

		psect = mngr->GetSectorPtr(&sec_count);
		if (!psect) {
			return;
		}

		//header
		CListSectors.InsertItem(0, TEXT("[header]"), 0);
		CListSectors.SetItemText(0, 1, TEXT("r--i---"));
		CListSectors.SetItemText(0, 2, TEXT("4"));
		_disp.SetCompareSector(0, true);
		_disp.SetCompareAlign(0, 4);

		//sections
		for (unsigned int i = 0, inx = 1; i < sec_count; i++, inx++) {
			memcpy(name, psect[i].Name, IMAGE_SIZEOF_SHORT_NAME);
			str = name;

			def_img = 1;
			if (!(psect[i].Characteristics & IMAGE_SCN_MEM_WRITE)) {
				def_img = 0;
				_disp.SetCompareSector(inx, true);
			}

			_disp.SetCompareAlign(inx, 4);

			CListSectors.InsertItem(inx, str, def_img);
			CListSectors.SetItemText(inx, 2, TEXT("4"));

			str.Empty();
			str.AppendChar(psect[i].Characteristics & IMAGE_SCN_MEM_READ ? 'r' : '-');
			str.AppendChar(psect[i].Characteristics & IMAGE_SCN_MEM_WRITE ? 'w' : '-');
			str.AppendChar(psect[i].Characteristics & IMAGE_SCN_MEM_EXECUTE ? 'e' : '-');
			str.AppendChar(psect[i].Characteristics & IMAGE_SCN_CNT_CODE ? 'c' : '-');
			str.AppendChar(psect[i].Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA ? 'i' : '-');
			str.AppendChar(psect[i].Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA ? 'u' : '-');
			str.AppendChar(psect[i].Characteristics & IMAGE_SCN_MEM_DISCARDABLE ? 'd' : '-');
			CListSectors.SetItemText(inx, 1, str);
		}
	}
}

void CPECompareToolsDlg::RefreshInform()
{
	if (_disp.IsBaseAppOpened()) {
		IPEManager *mngr = _disp.GetBaseInstance();
		TCHAR str[MAX_PATH], *ptype;

		switch (mngr->GetArch()) {
		case PE_32:
			ptype = TEXT("PE(32)");
			_stprintf_s(str, TEXT("%08X"), mngr->GetHOpt32()->ImageBase);
			break;
		case PE_64:
			ptype = TEXT("PE+(64)");
			_stprintf_s(str, TEXT("%012X"), mngr->GetHOpt64()->ImageBase);
			break;
		default:
			ptype = TEXT("unknown");
			break;
		}

		SetDlgItemText(IDC_STATIC9, ptype);
		SetDlgItemText(IDC_STATIC11, str);

		_stprintf_s(str, TEXT("%X"), mngr->GetPeakVirtualSize());
		SetDlgItemText(IDC_STATIC13, str);
	} else {
		SetDlgItemText(IDC_STATIC9, TEXT("unknown"));
		SetDlgItemText(IDC_STATIC11, TEXT("00000000"));
		SetDlgItemText(IDC_STATIC13, TEXT("0"));
	}
}

void CPECompareToolsDlg::RefreshCmpInform()
{
	if (_disp.IsBaseAppOpened()) {
		CString str;
		unsigned int val;
		
		val = _disp.GetDiffCount();
		str.Format(TEXT("%d"), val);
		SetDlgItemText(IDC_STATIC15, str);

		val = _disp.GetDiffCycles();
		str.Format(TEXT("%d"), val);
		SetDlgItemText(IDC_STATIC18, str);

		ConvSecToTimer(str, (unsigned int)_disp.GetScanTime());
		SetDlgItemText(IDC_STATIC20, str);
	} else {
		SetDlgItemText(IDC_STATIC15, TEXT("0"));
		SetDlgItemText(IDC_STATIC18, TEXT("0"));
		SetDlgItemText(IDC_STATIC20, TEXT("00:00:00"));
	}
}

void CPECompareToolsDlg::ConvSecToTimer(CString &str, unsigned int sec)
{
	unsigned int hours, mins, secs;

	str.Empty();

	hours = sec / (60 * 60);
	mins = (sec - (hours * 3600)) / 60;
	secs = sec - (hours * 3600) - (mins * 60);

	str.Format(TEXT("%02d:%02d:%02d"), hours, mins, secs);
}

unsigned int CPECompareToolsDlg::GetAppNumById(unsigned int id)
{
	std::list<unsigned int>::iterator it = _app_list.begin();
	unsigned int i = 0;
	while (it != _app_list.end()) {
		if (*it == id) {
			return i;
		}
		it++, i++;
	}
	return -1;
}

void CPECompareToolsDlg::stop_compare_callback(void *param)
{
	CPECompareToolsDlg *pthis = (CPECompareToolsDlg *)param;
	pthis->PostMessage(WM_USER_STOP_CMP, 0, 0);
}

bool CPECompareToolsDlg::enum_diffs_callback(DWORD offset, UINT size, unsigned int id, void *params)
{
	CPECompareToolsDlg *pthis = (CPECompareToolsDlg *)params;
	CString *pstr = (CString *)pthis->_out_str;
	IPEManager *pmngr;

	pmngr = pthis->_disp.GetDiffInstance(id);
	if (pthis->_last_id != id) {
		unsigned int inx = pthis->GetAppNumById(id);
		if (inx == -1) {
			return false;
		}

		CString str = pthis->CListApps.GetItemText(inx, 2);
		pstr->AppendFormat(TEXT("//Diff module: %s\r\n"), (LPCWSTR)str);
		pthis->_last_id = id;

		if (pthis->_out_type == 2) {
			pmngr->GetRelocsBase(&pthis->_last_addr);
		}
	}

	if (pthis->_out_type == 1) {
		pmngr->ConvVirtualToRaw(offset, &offset);
	}

	if (pthis->_out_type != 2) {
		switch (pthis->CComboOutStuct.GetCurSel()) {
		case 1:
			pstr->AppendFormat(TEXT("{0x%08X, %d},\r\n"), offset, size);
			break;
		default:
			pstr->AppendFormat(TEXT("%08X %d\r\n"), offset, size);
			break;
		}
	} else {
		switch (pthis->CComboOutStuct.GetCurSel()) {
		case 1:
			if (pmngr->GetArch() == PE_64) {
				pstr->AppendFormat(TEXT("{0x%012X, %d},\r\n"), pthis->_last_addr.val64 + offset, size);
			} else {
				pstr->AppendFormat(TEXT("{0x%08X, %d},\r\n"), pthis->_last_addr.val32l + offset, size);
			}
			break;
		default:
			if (pmngr->GetArch() == PE_64) {
				pstr->AppendFormat(TEXT("%012X %d\r\n"), pthis->_last_addr.val64 + offset, size);
			} else {
				pstr->AppendFormat(TEXT("%08X %d\r\n"), pthis->_last_addr.val32l + offset, size);
			}
			break;
		}
	}
	return true;
}

bool CPECompareToolsDlg::add_to_list_callback(DWORD offset, UINT size, unsigned int id, void *params)
{
	CPECompareToolsDlg *pthis = (CPECompareToolsDlg *)params;
	unsigned int inx = pthis->GetAppNumById(id);
	int item, count;
	CString str;
	
	count = pthis->CListDiff.GetItemCount();
	str.Format(TEXT("%d"), count + 1);
	item = pthis->CListDiff.InsertItem(count, str);

	str.Format(TEXT("%d"), inx + 1);
	pthis->CListDiff.SetItemText(item, 1, str);

	str.Format(TEXT("%08X"), offset);
	pthis->CListDiff.SetItemText(item, 2, str);

	str.Format(TEXT("%d"), size);
	pthis->CListDiff.SetItemText(item, 3, str);

	return true;
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPECompareToolsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//Disable column resize
void CPECompareToolsDlg::OnHdnBegintrackList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	*pResult = 1;
}

void CPECompareToolsDlg::OnBnClickedButton4()
{
	int inx = (int)CListSectors.GetFirstSelectedItemPosition() - 1;
	//int a = CListSectors.GetSelectedCount();
	INT_PTR res;
	TCHAR str[MAX_PATH];
	int align_size;
	if (inx == -1 || _disp.GetCompareState() == 1) {
		return;
	}

	CListSectors.GetItemText(inx, 2, str, MAX_PATH);
	align_size = _tstoi(str);

	CChangeAlignDlg dlg(align_size);
	res = dlg.DoModal();
	if (res != IDOK) {
		return;
	}

	_stprintf_s(str, TEXT("%d"), dlg.GetAlignSize());
	CListSectors.SetItemText(inx, 2, str);
	_disp.SetCompareAlign(inx, dlg.GetAlignSize());
}


void CPECompareToolsDlg::OnNMClickSyslink1(NMHDR *pNMHDR, LRESULT *pResult)
{
	ShellExecute(NULL, TEXT("open"), TEXT("http://armored.pro"), NULL, NULL, SW_SHOWNORMAL);
	*pResult = 0;
}


void CPECompareToolsDlg::OnBnClickedButton6()
{
	CString str, path;
	INT_PTR result;
	Selected_Module sel = {};

	if (_disp.GetCompareState() == 1) {
		return;
	}

	CComboOpenBase.GetWindowTextW(str);
	if (str == TEXT("File")) {
		CFileDialog CFile(true, TEXT(".exe"), TEXT(""), 
			0, TEXT("PE Applications (exe, dll)|*.exe;*.dll|All Files (*.*)|*.*||"));
		result = CFile.DoModal();
		path = CFile.GetPathName();
	} else {
		CAddModuleDlg dlg(&sel, NULL);
		result = dlg.DoModal();
	}

	if (result != IDOK) {
		return;
	}

	CListApps.DeleteAllItems();

	if (_disp.IsBaseAppOpened()) {
		_disp.CloseBaseApp();
		_app_list.clear();
	}

	if (str == TEXT("File")) {
		if (_disp.OpenBaseApp((LPCWSTR)path)) {
			CEditPath.SetWindowTextW(path);
		} else {
			CEditPath.SetWindowTextW(TEXT("Loading failed!"));
		}
	} else {
		if (_disp.OpenBaseApp(sel.pid, sel.hmod)) {
			if (!GetModuleStrName(str, sel.pid, sel.hmod)) {
				str = TEXT("unknown");
			}
			CEditPath.SetWindowTextW(str);
		} else {
			CEditPath.SetWindowTextW(TEXT("Loading failed!"));
		}
	}

	CheckButtonsState();
	CheckStartButtonsState();
	RefreshSections();
	RefreshInform();
}


void CPECompareToolsDlg::OnBnClickedButton8()
{
	INT_PTR result;
	Selected_Module sel;
	CAddModuleDlg dlg(&sel, NULL);
	CString str;
	int inx;
	unsigned int id;

	if (_disp.GetCompareState() == 1) {
		return;
	}

	result = dlg.DoModal();
	if (result != IDOK) {
		return;
	}
	
	id = _disp.AddDiffApp(sel.pid, sel.hmod);
	if (id == CMP_DISP_INVALID_ID) {
		MessageBox(TEXT("Can't load this module"), TEXT("Load error"), MB_ICONERROR);
		return;
	}

	_app_list.push_back(id);

	str.Format(TEXT("%d"), CListApps.GetItemCount() + 1);
	inx = CListApps.InsertItem(CListApps.GetItemCount(), str);
	
	CListApps.SetItemText(inx, 1, TEXT("module"));

	if (!GetModuleStrName(str, sel.pid, sel.hmod)) {
		str = TEXT("unknown");
	}
	CListApps.SetItemText(inx, 2, str);
}


void CPECompareToolsDlg::OnBnClickedButton7()
{
	INT_PTR result;
	CFileDialog CFile(true, TEXT(".exe"), TEXT(""), 
		0, TEXT("PE Applications (exe, dll)|*.exe;*.dll|All Files (*.*)|*.*||"));
	int inx;
	CString str;
	unsigned int id;

	if (_disp.GetCompareState() == 1) {
		return;
	}

	result = CFile.DoModal();
	if (result != IDOK) {
		return;
	}

	id = _disp.AddDiffApp(CFile.GetPathName());
	if (id == CMP_DISP_INVALID_ID) {
		MessageBox(TEXT("Can't load this file"), TEXT("Load error"), MB_ICONERROR);
		return;
	}

	_app_list.push_back(id);

	str.Format(TEXT("%d"), CListApps.GetItemCount() + 1);
	inx = CListApps.InsertItem(CListApps.GetItemCount(), str);

	CListApps.SetItemText(inx, 1, TEXT("file"));
	CListApps.SetItemText(inx, 2, CFile.GetPathName());

}

void CPECompareToolsDlg::OnItemdblclickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}


void CPECompareToolsDlg::OnRclickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}


void CPECompareToolsDlg::OnDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int inx = pNMItemActivate->iItem;
	
	LVITEM lvi;
	lvi.mask = LVIF_IMAGE;
	lvi.iItem = inx;
	lvi.iSubItem = 0;

	if (_disp.GetCompareSector(inx)) {
		lvi.iImage = 1;
		_disp.SetCompareSector(inx, false);
	} else {
		lvi.iImage = 0;
		_disp.SetCompareSector(inx, true);
	}

	CListSectors.SetItem(&lvi);
	*pResult = 0;
}


void CPECompareToolsDlg::OnBnClickedOk()
{//compare
	if (_disp.GetCompareState() == 0) {
		CString str;

		if (!_disp.IsBaseAppOpened()) {
			MessageBox(TEXT("Please choose base module"), TEXT("Startup error"), MB_ICONERROR);
			return;
		} else if (!_disp.IsDiffAppsOpened()) {
			MessageBox(TEXT("Please choose different modules"), TEXT("Startup error"), MB_ICONERROR);
			return;
		}

		CComboRepeat.GetWindowText(str);
		if (!_disp.StartCompare((str == TEXT("Custom") ? DRM_INFINITE : DRM_ONCE), 
		4, 0x10000, stop_compare_callback, this)) {
			MessageBox(TEXT("Can't start compare"), TEXT("Startup error"), MB_ICONERROR);
			return;
		}

		CButtonCmp.SetWindowText(TEXT("Stop"));
		CListDiff.DeleteAllItems();

		_timer_id = SetTimer(REFRESH_TIMER_ID, 200, NULL);
		RefreshCmpInform();
		CheckStartButtonsState();
	} else {
		CButtonCmp.EnableWindow(FALSE);
		_disp.StopCompareAsync(stop_compare_callback, this);
	}
}


void CPECompareToolsDlg::OnBnClickedOk2()
{//save
	CFileDialog dlg(false, TEXT(".txt"), TEXT("output.txt"), OFN_OVERWRITEPROMPT, TEXT("Text file (txt)|*.txt|All Files (*.*)|*.*||"));
	CFile file; 
	INT_PTR res;
	CString str_out, str_path;
	SYSTEMTIME stime;

	res = dlg.DoModal();
	if (res != IDOK) {
		return;
	}

	if (!file.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite)) {
		MessageBox(TEXT("Can't open output file"), TEXT("Save error"), MB_ICONWARNING);
		return;
	}

	GetSystemTime(&stime);
	
	str_out.Format(TEXT("//Report at %02d.%02d.%04d %02d:%02d:%02d\r\n"),
		stime.wDay, stime.wMonth, stime.wYear, stime.wHour, stime.wMinute, stime.wSecond);

	_last_id = -1;
	_out_str = &str_out;
	_out_type = CComboOffset.GetCurSel();

	CEditPath.GetWindowText(str_path);
	str_out.AppendFormat(TEXT("//Base module: %s\r\n"), (LPCWSTR)str_path);

	if (!_disp.EnumDiffElem(enum_diffs_callback, this)) {
		MessageBox(TEXT("Can't contain output data"), TEXT("Save error"), MB_ICONWARNING);
		return;
	}

	try {
		file.Write(str_out.GetBuffer(), str_out.GetLength() * sizeof(TCHAR));
		file.Flush();
	} catch (...) {
		MessageBox(TEXT("Can't save output file"), TEXT("Save error"), MB_ICONWARNING);
	}

	return;
}


void CPECompareToolsDlg::OnBnClickedCancel()
{//exit
	CDialogEx::OnCancel();
}

afx_msg LRESULT CPECompareToolsDlg::OnUserStopCmp(WPARAM wParam, LPARAM lParam)
{
	CButtonCmp.EnableWindow(TRUE);
	CButtonCmp.SetWindowText(TEXT("Compare"));
	CheckStartButtonsState();
	RefreshCmpInform();

	_disp.EnumDiffElem(add_to_list_callback, this);

	return 0;
}

void CPECompareToolsDlg::OnBnClickedButton5()
{
	int inx = (int)CListApps.GetFirstSelectedItemPosition() - 1;
	std::list<unsigned int>::iterator it;
	unsigned int id = CMP_DISP_INVALID_ID, i = 0;
	CString str;

	if (inx == -1 || _disp.GetCompareState() == 1) {
		return;
	}

	it = _app_list.begin();
	while (it != _app_list.end()) {
		if (i == inx) {
			id = *it;
			break;
		}
		it++, i++;
	}
	if (id == CMP_DISP_INVALID_ID) {
		return;
	}

	if (!_disp.RemoveDiffApp(id)) {
		MessageBox(TEXT("Removing error!"), TEXT("Operation error"), MB_ICONERROR);
	}
	CListApps.DeleteItem(inx);
	_app_list.erase(it);

	i = CListApps.GetItemCount();
	for (unsigned int a = 0; a < i; a++) {
		str.Format(TEXT("%d"), a + 1);
		CListApps.SetItemText(a, 0, str);
	}
}


void CPECompareToolsDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == _timer_id) {
		RefreshCmpInform();
		if (_disp.GetCompareState() != 1) {
			KillTimer(nIDEvent);
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}
