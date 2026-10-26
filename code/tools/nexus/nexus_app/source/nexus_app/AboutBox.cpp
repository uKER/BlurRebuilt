#include <nexus_app/stdafx.h>
#include <nexus_app/NexusSystem.h>

#include "../../resource.h"

#include <nexus_app/AboutBox.h>
#include "..\..\include\nexus_app\aboutbox.h"

using namespace Nexus;


IMPLEMENT_DYNAMIC(AboutBox, CDialog)
AboutBox::AboutBox(CWnd* pParent /*=NULL*/)
	: CDialog(AboutBox::IDD, pParent)
{
}

AboutBox::~AboutBox()
{
}

void AboutBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ABOUTVERSION, m_versionTextCtrl);
	DDX_Control(pDX, IDC_ABOUTAFTK, m_aftkTextCtrl);	
}


BEGIN_MESSAGE_MAP(AboutBox, CDialog)
END_MESSAGE_MAP()


// AboutBox message handlers

BOOL AboutBox::OnInitDialog()
{
	CDialog::OnInitDialog();

	{
		std::string temp;
		temp += "Nexus: ";
		temp += __DATE__;
		temp += " ";
		temp += __TIME__;
		m_versionTextCtrl.SetWindowText(temp.c_str());
	}
	std::stringstream ss;
	U32 versionHigh;
	U32 versionLow;
	Aftk::C_Global::GetHighestSupportedFileVersion(Aftk::E_FileType_A3d, versionHigh);
	Aftk::C_Global::GetLowestSupportedFileVersion(Aftk::E_FileType_A3d, versionLow);

	ss << "AFTK version [" << Aftk::C_Global::GetVersion() << "]\n";
	ss << "AFTK build stamp [" << Aftk::C_Global::GetBuildStamp() << "]\n";
	ss << "AFTK build PC [" << Aftk::C_Global::GetBuildPc() << "]\n";
	ss << "AFTK SVN revision(s) [" << Aftk::C_Global::GetSvnRevision() << "]\n";
	ss << "A3D exported file version [" << versionHigh << "]\n";
	ss << "A3D importable file versions [" << versionLow << "] to [" << versionHigh << "]\n";
	m_aftkTextCtrl.SetWindowText(ss.str().c_str());

	return TRUE; 
}
