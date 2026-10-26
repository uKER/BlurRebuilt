// F:\Subversion\tools\trunk\lpvs latest\lux_app\Source\lux_app\LpvsFileProgress.cpp : implementation file
//

#include "stdafx.h"
#include <lux_app/LpvsApp.h>
#include "lux_app/LpvsFileProgressDialog.h"
#include "..\..\include\lux_app\lpvsfileprogressdialog.h"


// LpvsFileProgress dialog

IMPLEMENT_DYNAMIC(LpvsFileProgressDialog, CDialog)
LpvsFileProgressDialog::LpvsFileProgressDialog(CWnd* pParent /*=NULL*/)
	: CDialog(LpvsFileProgressDialog::IDD, pParent)
{
}

LpvsFileProgressDialog::~LpvsFileProgressDialog()
{
}
/*
void LpvsFileProgressDialog::OnSubProgress(float percent)
{
	CProgressCtrl* pProgress;
	pProgress = reinterpret_cast<CProgressCtrl *>(GetDlgItem(IDC_PROGRESS_FILE_SUB));

	pProgress->SetPos((S32)percent);
}

void LpvsFileProgressDialog::OnMainProgress(float percent)
{
	CProgressCtrl* pProgress;
	pProgress = reinterpret_cast<CProgressCtrl *>(GetDlgItem(IDC_PROGRESS_FILE_MAIN));

	pProgress->SetPos((S32)percent);
}

void LpvsFileProgressDialog::OnSubPrintProgress(const std::string& text)
{
}

void LpvsFileProgressDialog::OnMainPrintProgress(const std::string& text)
{
}
*/

//Returning false will eventually abort the current operation. For the moment this is however
//not implemented.
bool LpvsFileProgressDialog::OnProgress(F32 percent)
{
	CProgressCtrl* pProgress;
	pProgress = reinterpret_cast<CProgressCtrl *>(GetDlgItem(IDC_PROGRESS_FILE_SUB));

	pProgress->SetPos((S32)percent);

	F32 totalWork = 100.0f * (F32)m_subProcessProgress.Size();
	F32 currentWork = (100.0f * (F32)m_currentSubProcess) + percent;
	F32 overallProgress = currentWork / totalWork * 100.0f;

	pProgress = reinterpret_cast<CProgressCtrl *>(GetDlgItem(IDC_PROGRESS_FILE_MAIN));
	
	pProgress->SetPos((S32)overallProgress);

	CStatic* pStatic;
	pStatic = reinterpret_cast<CStatic *>(GetDlgItem(IDC_STATIC_FILE_MAIN));
	char buffer[1024];
	sprintf(buffer, "Overall progress %3.2f percent", overallProgress);
	pStatic->SetWindowText(buffer);

	return true;
}

//Callback function for printing output to the user while they wait.
void LpvsFileProgressDialog::OnPrintProgress(const std::string& text)
{
	CStatic* pStatic;
	pStatic = reinterpret_cast<CStatic *>(GetDlgItem(IDC_STATIC_FILE_SUB));
	pStatic->SetWindowText(text.c_str());
}

void LpvsFileProgressDialog::OnPrintWarning(const std::string& text)
{
}

void LpvsFileProgressDialog::OnPrintError(const std::string& text)
{
}

void LpvsFileProgressDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(LpvsFileProgressDialog, CDialog)
END_MESSAGE_MAP()


// LpvsFileProgress message handlers

BOOL LpvsFileProgressDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	CProgressCtrl* pProgress;
	pProgress = reinterpret_cast<CProgressCtrl *>(GetDlgItem(IDC_PROGRESS_FILE_SUB));
	pProgress->SetRange(0, 100);
	pProgress = reinterpret_cast<CProgressCtrl *>(GetDlgItem(IDC_PROGRESS_FILE_MAIN));
	pProgress->SetRange(0, 100);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
