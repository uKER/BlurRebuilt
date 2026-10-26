// AddTextDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UIMaker2.h"
#include "AddTextDlg.h"


// CAddTextDlg dialog

IMPLEMENT_DYNAMIC(CAddTextDlg, CDialog)
CAddTextDlg::CAddTextDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAddTextDlg::IDD, pParent)
	, IDC_Value(_T(""))
	, TextString(_T(""))
{
}

CAddTextDlg::~CAddTextDlg()
{
}

void CAddTextDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, IDC_Value);
	DDX_Text(pDX, IDC_EDIT1, TextString);
}


BEGIN_MESSAGE_MAP(CAddTextDlg, CDialog)
END_MESSAGE_MAP()


// CAddTextDlg message handlers
