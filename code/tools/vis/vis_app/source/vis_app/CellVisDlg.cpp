// CellVisDlg.cpp : implementation file
//

#include <vis_app/stdafx.h>
#include "../../resource.h"
#include "vis_app/CellVisDlg.h"


// CCellVisDlg dialog

IMPLEMENT_DYNAMIC(CCellVisDlg, CDialog)
CCellVisDlg::CCellVisDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCellVisDlg::IDD, pParent)
	, m_iCell(0)
{
}

CCellVisDlg::~CCellVisDlg()
{
}

void CCellVisDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_iCell);
	DDV_MinMaxInt(pDX, m_iCell, 0, 999);
}


BEGIN_MESSAGE_MAP(CCellVisDlg, CDialog)
END_MESSAGE_MAP()


// CCellVisDlg message handlers
