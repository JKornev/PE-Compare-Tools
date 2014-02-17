#pragma once
#include "afxwin.h"


// CChangeAlignDlg dialog

class CChangeAlignDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CChangeAlignDlg)

	UINT _align_size;
	//UINT _align_addr;

public:
	CChangeAlignDlg(UINT align_size, CWnd* pParent = NULL);   // standard constructor
	virtual ~CChangeAlignDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

	UINT GetAlignSize() const;
	//UINT GetAlignAddr() const;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CComboBox CComboSize;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
