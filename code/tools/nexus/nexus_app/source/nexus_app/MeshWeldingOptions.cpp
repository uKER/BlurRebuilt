#include <nexus_app/stdafx.h>
#include "../../resource.h"
#include "..\..\include\nexus_app\meshweldingoptions.h"


using namespace Nexus;

IMPLEMENT_DYNAMIC(MeshWeldingOptions, CDialog)
MeshWeldingOptions::MeshWeldingOptions(CWnd* pParent /*=NULL*/)
	: CDialog(MeshWeldingOptions::IDD, pParent)
{
	m_weldDistance = 0.0001f;
	m_cancelled = true;
}

MeshWeldingOptions::~MeshWeldingOptions()
{
}

void MeshWeldingOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//Aquire information from text boxes.
	DDX_Text(pDX, IDC_MW_WELDDISTANCE_EDIT, m_weldDistance);

	DDX_Control(pDX, IDC_MW_WELDDISTANCE_EDIT, m_weldDistanceEdit);
	DDX_Control(pDX, IDC_MW_WELDDISTANCESPIN, m_weldDistanceSpin);
}


BEGIN_MESSAGE_MAP(MeshWeldingOptions, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// MeshWeldingOptions message handlers

BOOL MeshWeldingOptions::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_weldDistanceSpin.SetBuddy(&m_weldDistanceEdit);
	m_weldDistanceSpin.SetRange(0.f, 10000.f);
	m_weldDistanceSpin.SetPos(m_weldDistance);
	m_weldDistanceSpin.SetDelta(0.0001f);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void MeshWeldingOptions::OnBnClickedOk()
{
	m_cancelled = false;
	OnOK();
}

void MeshWeldingOptions::OnBnClickedCancel()
{
	m_cancelled = true;
	OnCancel();
}
