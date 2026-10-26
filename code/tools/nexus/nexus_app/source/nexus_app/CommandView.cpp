#include <nexus_app/stdafx.h>
#include <nexus_app/Nexus.h>
#include "../../resource.h"
#include <nexus_app/CommandEdit.h>
#include <mfc_extras/ChatRichEd.h>
#include <nexus_app/CommandEdit.h>
#include <nexus_app/CommandView.h>
#include <nexus_app/NexusSystem.h>
#include "..\..\include\nexus_app\commandview.h"
#include <nexus_app/MainFrm.h>
#include <nexus_app/DockingPanelBottom.h>
#include <nexus_app/NexusDoc.h>

using namespace Nexus;

IMPLEMENT_DYNAMIC(CommandView, CExtResizableDialog)
CommandView::CommandView(CWnd* pParent /*=NULL*/)
	: CExtResizableDialog(CommandView::IDD, pParent)
{
}

CommandView::~CommandView()
{
}

void CommandView::DoDataExchange(CDataExchange* pDX)
{
	CExtResizableDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMMAND_VIEW_INPUT, m_commandEdit);
	DDX_Control(pDX, IDC_COMMAND_VIEW_REDIT, m_outputREdit);
}


BEGIN_MESSAGE_MAP(CommandView, CExtResizableDialog)
END_MESSAGE_MAP()


// CommandView message handlers

BOOL CommandView::OnInitDialog()
{
	CExtResizableDialog::OnInitDialog();

	if (!AddAnchor(IDC_COMMAND_VIEW_REDIT, __RDA_KEEP, __RDA_BOTH))
		return FALSE;
	if (!AddAnchor(IDC_COMMAND_VIEW_INPUT, __RDA_Y, __RDA_BOTH))
		return FALSE;
	if (!AddAnchor(IDOK, __RDA_BOTH, __RDA_BOTH))
		return FALSE;

	ShowSizeGrip(FALSE);
	Nexus::g_pNexusSystem->SetLogCallback(PrintStatic);
	return TRUE;
}

void CommandView::OnOK()
{
	Core* pCore = g_pNexusSystem->GetCorePtr();

	char buffer[32768];
	m_commandEdit.GetLine(0, buffer, 32768);
	m_commandEdit.AddHistory(buffer);
	m_commandEdit.ClearCommand();

	Print(buffer);
	Print("\n");
	Tu::Status status = pCore->ExecuteString(buffer);
	if (!status)
	{
		std::string err("*** Could not execute command: " + status.ErrorString() + std::string("***\n"));
		Print(err);
	}
	
	//Refresh our document view if needed.
	if (pCore->IsSceneViewDirty())
	{
		g_pMainFrame->Update();
		g_pDocument->UpdateAllViews(0);
		pCore->SetIsSceneViewDirty(false);
	}
}

void CommandView::Clear()
{
	m_outputREdit.Clear();
	m_outputText.Empty();
}

void CommandView::Print(const std::string& text)
{
	m_outputREdit.AppendText(text.c_str());
}

void CommandView::PrintStatic(const std::string& text)
{
	g_pMainFrame->GetDockingPanelBottomPtr()->GetCommandViewPtr()->Print(text);
}
