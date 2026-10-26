// ProjectPreferencesDialog.cpp : implementation file
//

#include "stdafx.h"
#include <lux_app/LpvsApp.h>
#include <lux_app/LpvsProjectPreferencesDialog.h>
#include "..\..\include\lux_app\lpvsprojectpreferencesdialog.h"
#include "lux_app/lpvsmainframe.h"

using namespace LPVS;

// ProjectPreferencesDialog dialog

IMPLEMENT_DYNAMIC(LpvsProjectPreferencesDialog, CDialog)
LpvsProjectPreferencesDialog::LpvsProjectPreferencesDialog(CWnd* pParent /*=NULL*/)
	: CDialog(LpvsProjectPreferencesDialog::IDD, pParent)
	, m_sceneFileName(_T(""))
	, m_lightingModelFileName(_T(""))
	, m_sceneOutputFileName(_T(""))
{
	Reset();
}

LpvsProjectPreferencesDialog::~LpvsProjectPreferencesDialog()
{
	Reset();
}

void LpvsProjectPreferencesDialog::Reset()
{
	m_settingsChanged = false;
}

void LpvsProjectPreferencesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SCENE_FILE_EDIT, m_sceneFileName);
	DDX_Text(pDX, IDC_LIGHTING_MODEL_EDIT, m_lightingModelFileName);
	DDX_Text(pDX, IDC_SCENE_OUTPUT_FILE_EDIT, m_sceneOutputFileName);
}


BEGIN_MESSAGE_MAP(LpvsProjectPreferencesDialog, CDialog)
	ON_BN_CLICKED(IDC_SET_LIGHTING_MODEL_FILE, OnBnClickedSetLightingModelFile)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_SET_SCENE_FILE, OnBnClickedSetSceneFile)
	ON_BN_CLICKED(IDC_SET_SCENE_FILE_OUTPUT, OnBnClickedSetSceneFileOutput)
END_MESSAGE_MAP()


// ProjectPreferencesDialog message handlers

void LpvsProjectPreferencesDialog::OnBnClickedSetLightingModelFile()
{
	char strFilter[] = { "XML Files (*.xml)|*.xml|All Files (*.*)|*.*||" };

	CFileDialog openFile(
		true,
		"XML",
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		strFilter,
		this->GetWindow(GW_HWNDFIRST));

	if( openFile.DoModal() == IDOK )
	{
		CEdit* textBox;
		textBox = reinterpret_cast<CEdit *>(GetDlgItem(IDC_LIGHTING_MODEL_EDIT));
		// Format as text
		
		std::string combined = openFile.GetPathName().GetBuffer();

		// Load text into edit control
		textBox->SetWindowText(combined.c_str());
	}
}

void LpvsProjectPreferencesDialog::OnBnClickedOk()
{
	//Wow, this implicitly calls do data exchange. Thanks MFC.
	UpdateData();

	LUX::Lux* pLux = LPVSCORE.GetLightingSystemPtr();

	std::string oldLightingModel = pLux->GetLightingModelFileName();
	std::string newLightingModel = m_lightingModelFileName.GetBuffer();

	if (oldLightingModel != newLightingModel)
	{
		pLux->SetLightingModelFileName(m_lightingModelFileName.GetBuffer());
		if (newLightingModel.length() > 0)
		{
			//TODO: Temporary - implement task queuing system in LPVSCORE. We should not be loading here.
			pLux->ReadLightingModelProperties();
		}
	}

	pLux->SetOutputSceneFileName(m_sceneOutputFileName.GetBuffer());

	std::string oldScene = pLux->GetInputSceneFileName();
	std::string newScene = m_sceneFileName.GetBuffer();

	if (oldScene != newScene)
	{
		pLux->SetInputSceneFileName(m_sceneFileName.GetBuffer());
		if (newScene.length() > 0)
		{
			//TODO: Temporary - implement task queuing system in LPVSCORE. We should not be loading here.
			LPVSCORE.ReadScene();

			LPVSCORE.GetSceneRenderer()->SetSettings(gpMainFrm->m_appSettings.m_RendererSettings);
		}
	}

	OnOK();
}

void LpvsProjectPreferencesDialog::OnBnClickedSetSceneFile()
{
	char strFilter[] = { "Abstract 3D Files (*.a3d)|*.a3d|SmakEd 1/2 Files (*.smk)|*.smk|SoftImage dotXSI Files (*.xsi)|*.xsi|All Files (*.*)|*.*||" };

	CFileDialog openFile(
		true,
		"A3D",
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		strFilter,
		this->GetWindow(GW_HWNDFIRST));

	if( openFile.DoModal() == IDOK )
	{
		CEdit* textBox;
		textBox = reinterpret_cast<CEdit *>(GetDlgItem(IDC_SCENE_FILE_EDIT));
		// Format as text
		
		std::string combined = openFile.GetPathName().GetBuffer();

		// Load text into edit control
		textBox->SetWindowText(combined.c_str());
	}
}

BOOL LpvsProjectPreferencesDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	LUX::Lux* pLux = LPVSCORE.GetLightingSystemPtr();

	CEdit* textBox;

	{
		// Load text into edit control
		textBox = reinterpret_cast<CEdit *>(GetDlgItem(IDC_SCENE_FILE_EDIT));
		std::string sceneFile = pLux->GetInputSceneFileName();
		textBox->SetWindowText(sceneFile.c_str());
	}
	{
		// Load text into edit control
		textBox = reinterpret_cast<CEdit *>(GetDlgItem(IDC_SCENE_OUTPUT_FILE_EDIT));
		std::string targetSceneFile = pLux->GetOutputSceneFileName();
		textBox->SetWindowText(targetSceneFile.c_str());
	}
	{
		// Load text into edit control
		textBox = reinterpret_cast<CEdit *>(GetDlgItem(IDC_LIGHTING_MODEL_EDIT));
		std::string lightingModelFile = pLux->GetLightingModelFileName();
		textBox->SetWindowText(lightingModelFile.c_str());
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void LpvsProjectPreferencesDialog::OnBnClickedSetSceneFileOutput()
{
	char strFilter[] = { "Abstract 3D Files (*.a3d)|*.a3d|SmakEd 1/2 Files (*.smk)|*.smk|SoftImage dotXSI Files (*.xsi)|*.xsi|All Files (*.*)|*.*||" };

	CFileDialog saveFile(
		false,
		"A3D",
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		strFilter,
		this->GetWindow(GW_HWNDFIRST));

	if( saveFile.DoModal() == IDOK )
	{
		CEdit* textBox;
		textBox = reinterpret_cast<CEdit *>(GetDlgItem(IDC_SCENE_OUTPUT_FILE_EDIT));
		// Format as text
		
		std::string combined = saveFile.GetPathName().GetBuffer();

		// Load text into edit control
		textBox->SetWindowText(combined.c_str());
	}
}
