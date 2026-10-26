#include <nexus_app/stdafx.h>
#include "../../resource.h"
#include "..\..\include\nexus_app\calculatetangentsandbinormalsdialog.h"

using namespace Nexus;


IMPLEMENT_DYNAMIC(CalculateTangentsAndBinormalsDialog, CDialog)
CalculateTangentsAndBinormalsDialog::CalculateTangentsAndBinormalsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CalculateTangentsAndBinormalsDialog::IDD, pParent)
	, m_bRespectSplitCheck(TRUE)
	, m_bNormaliseCheck(TRUE)
	, m_bWeldCheck(TRUE)
{
	m_mag = 0.2f;
	m_angle = 80.f;
	m_threshold = 0.0001f;
	m_cancelled = true;
}

CalculateTangentsAndBinormalsDialog::~CalculateTangentsAndBinormalsDialog()
{
}

void CalculateTangentsAndBinormalsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//Aquire information from text boxes.
	DDX_Text(pDX, IDC_CTB_MAGEDIT,			m_mag);
	DDX_Text(pDX, IDC_CTB_ANGLEEDIT,		m_angle);
	DDX_Text(pDX, IDC_CTB_WELDDISTANCE_EDIT,m_threshold);

	DDX_Check(pDX, IDC_CTB_NORMALISECHECK, m_bNormaliseCheck);
	DDX_Check(pDX, IDC_CTB_WELDMESHCHECK, m_bWeldCheck);

	DDX_Control(pDX, IDC_CTB_MAGEDIT, m_magEdit);
	DDX_Control(pDX, IDC_CTB_MAGSPIN, m_magSpin);
	DDX_Control(pDX, IDC_CTB_ANGLEEDIT, m_angleEdit);
	DDX_Control(pDX, IDC_CTB_ANGLESPIN, m_angleSpin);

	DDX_Control(pDX, IDC_CTB_WELDDISTANCE_EDIT, m_weldDistanceEdit);
	DDX_Control(pDX, IDC_WELDDISTANCESPIN, m_weldDistanceSpin);
}


BEGIN_MESSAGE_MAP(CalculateTangentsAndBinormalsDialog, CDialog)
	ON_NOTIFY(UDN_DELTAPOS, IDC_CTB_MAGSPIN, OnDeltaposCtbTcreasespin)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CalculateTangentsAndBinormalsDialog message handlers

void CalculateTangentsAndBinormalsDialog::OnBnClickedCtbRespectsplit()
{
	// TODO: Add your control notification handler code here
}

void CalculateTangentsAndBinormalsDialog::OnDeltaposCtbTcreasespin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

BOOL CalculateTangentsAndBinormalsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_magSpin.SetBuddy(&m_magEdit);
	m_magSpin.SetRange(0.f, 10000.f);
	m_magSpin.SetPos(m_mag);
	m_magSpin.SetDelta(1.f);

	m_angleSpin.SetBuddy(&m_angleEdit);
	m_angleSpin.SetRange(0.f, 180.0f);
	m_angleSpin.SetPos(m_angle);
	m_angleSpin.SetDelta(1.f);

	m_weldDistanceSpin.SetBuddy(&m_weldDistanceEdit);
	m_weldDistanceSpin.SetRange(0.f, 10000.f);
	m_weldDistanceSpin.SetPos(m_threshold);
	m_weldDistanceSpin.SetDelta(0.0001f);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CalculateTangentsAndBinormalsDialog::OnBnClickedOk()
{
	m_cancelled = false;
	OnOK();
}

void CalculateTangentsAndBinormalsDialog::OnBnClickedCancel()
{
	m_cancelled = true;
	OnCancel();
}
