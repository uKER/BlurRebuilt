// LayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "lux_app/LpvsApp.h"
#include "lux_app/LayerDlg.h"
#include "lux_App/CustomControls/DarkDialog.h"
#include "..\..\include\lux_app\layerdlg.h"

// CLayerDlg dialog

IMPLEMENT_DYNAMIC(CLayerDlg, CDarkDialog)
CLayerDlg::CLayerDlg(CWnd* pParent /*=NULL*/)
	: CDarkDialog(CLayerDlg::IDD, pParent)
{
}

CLayerDlg::~CLayerDlg()
{
	DeleteControls();
}

void CLayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLayerDlg, CDarkDialog)
END_MESSAGE_MAP()


// CLayerDlg message handlers

#define LIST_LEFT 23
#define LIST_TOP (35 + 17)
#define LIST_RIGHT (LIST_LEFT+94)
#define LIST_BOTTOM (LIST_TOP+116)
#define LIST_SPACING 18

CRect GetDynamicLayerRect(int y)
{
	return CRect(LIST_LEFT,LIST_TOP+y*LIST_SPACING,LIST_RIGHT,LIST_TOP+(y+1)*LIST_SPACING);
}

// create all the checkbox controls
void CLayerDlg::CreateControls()
{
	for (int i=0; i<m_BoxList.Size(); i++)
	{
		CWnd* pWnd = new CWnd;
		pWnd->Create(_T("BUTTON"), m_BoxList[i].pszName, WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, GetDynamicLayerRect(i), this, 3);
		pWnd->SetFont(this->GetFont());
		m_BoxList[i].pWnd = pWnd;
	}
}

// delete all the checkbox controls
void CLayerDlg::DeleteControls()
{
	for (int i=0; i<m_BoxList.Size(); i++)
	{
		m_BoxList[i].pWnd->DestroyWindow();
		delete m_BoxList[i].pWnd;
		m_BoxList[i].pWnd = NULL;
	}
	m_BoxList.Clear();
}

// add a new layer
void CLayerDlg::AddLayer(const char *pszName)
{
	SLayerBox newBox;

	newBox.pszName = pszName;
	newBox.pWnd = NULL;

    m_BoxList.PushBack( newBox );
}

void Log(char*,...);

