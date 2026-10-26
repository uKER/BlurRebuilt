#include <nexus_app/stdafx.h>
#include <nexus_app/Nexus.h>

#include <nexus_app/DockingPanelRight.h>
#include <nexus_app/MainFrm.h>

#include "../../resource.h"
#include <nexus_app/NexusSystem.h>
#include <nexus_app/NexusDoc.h>
#include <mfc_extras/ChatRichEd.h>
#include <nexus_app/CommandEdit.h>
#include <nexus_app/CommandView.h>
#include <nexus_app/DockingPanelBottom.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Nexus;

bool DockingPanelBottom::Create(CWnd* pMainFrame)
{
	if(	!CExtControlBar::Create(
			_T("Output and Command Panel"),
			pMainFrame,
			ID_VIEW_OUTPUT_TOOLBAR))
		return false;

	if( !m_tabPageContainerCtrl.Create(
			this,
			CRect(0,0,0,0),
			this->GetDlgCtrlID()))
		return false;

	if (!m_commandViewDlg.Create(
		m_commandViewDlg.IDD,
		&m_tabPageContainerCtrl))
		return false;

	if(	!m_tabPageContainerCtrl.PageInsert(
		&m_commandViewDlg,
		"Output"))
		return false;

	return true;
}

void DockingPanelBottom::Destroy()
{
}

void DockingPanelBottom::Update()
{
}

void DockingPanelBottom::Clear()
{
	m_commandViewDlg.Clear();
}

CommandView* DockingPanelBottom::GetCommandViewPtr()
{
	return &m_commandViewDlg;
}

const CommandView* DockingPanelBottom::GetCommandViewPtr() const
{
	return &m_commandViewDlg;
}

