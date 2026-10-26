#pragma once


// CAddTextDlg dialog

class CAddTextDlg : public CDialog
{
	DECLARE_DYNAMIC(CAddTextDlg)

public:
	CAddTextDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddTextDlg();

// Dialog Data
	enum { IDD = ID_ADD_TEXT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString IDC_Value;
	CString TextString;
};
