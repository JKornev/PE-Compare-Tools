// ChangeAlignDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PETools.h"
#include "ChangeAlignDlg.h"
#include "afxdialogex.h"


// CChangeAlignDlg dialog

IMPLEMENT_DYNAMIC(CChangeAlignDlg, CDialogEx)

CChangeAlignDlg::CChangeAlignDlg(UINT align_size, CWnd* pParent /*=NULL*/)
	: CDialogEx(CChangeAlignDlg::IDD, pParent), _align_size(align_size)
{
}

CChangeAlignDlg::~CChangeAlignDlg()
{
}

void CChangeAlignDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO5, CComboSize);
}


BEGIN_MESSAGE_MAP(CChangeAlignDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CChangeAlignDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CChangeAlignDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CChangeAlignDlg message handlers


BOOL CChangeAlignDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

/*	CComboAddr.AddString(TEXT("1"));
	CComboAddr.AddString(TEXT("2"));
	CComboAddr.AddString(TEXT("4"));
	CComboAddr.AddString(TEXT("8"));
	CComboAddr.AddString(TEXT("16"));*/

	CComboSize.AddString(TEXT("1"));
	CComboSize.AddString(TEXT("2"));
	CComboSize.AddString(TEXT("4"));
	CComboSize.AddString(TEXT("8"));
	//CComboSize.AddString(TEXT("16"));

	CString str;
/*	str.AppendFormat(TEXT("%d"), _align_addr);
	CComboAddr.SelectString(-1, str);*/

	//str.Empty();
	str.AppendFormat(TEXT("%d"), _align_size);
	CComboSize.SelectString(-1, str);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

UINT CChangeAlignDlg::GetAlignSize() const
{
	return _align_size;
}

/*
UINT CChangeAlignDlg::GetAlignAddr() const
{
	return _align_addr;
}*/

void CChangeAlignDlg::OnBnClickedOk()
{
	CString str;

/*	CComboAddr.GetWindowText(str);
	_align_addr = _tstoi((LPCTSTR)str);*/
	CComboSize.GetWindowText(str);
	_align_size = _tstoi((LPCTSTR)str);

	CDialogEx::OnOK();
}


void CChangeAlignDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}
