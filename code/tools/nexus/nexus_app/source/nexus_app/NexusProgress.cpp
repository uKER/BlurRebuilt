#include <nexus_app/stdafx.h>
#include "../../resource.h"
#include <nexus_app/NexusProgressDialog.h>
#include <mfc_extras/BaseThread.h>

#include "..\..\include\nexus_app\nexusprogress.h"


NexusProgress::NexusProgress()
{
	m_pParent = 0;
}

NexusProgress::~NexusProgress()
{
}

void NexusProgress::SetParent(CWnd* pParent)
{
	m_pParent = pParent;
}

void NexusProgress::Show()
{
//	m_thread.SetParent(m_pParent);
//	m_thread.CreateThread();
}

void NexusProgress::Hide()
{
//	m_thread.Dialog()->PostMessage(WM_COMMAND, IDCANCEL);
}

void NexusProgress::SetTitle(const std::string& text)
{
}

//Returning false will eventually abort the current operation. For the moment this is however
//not implemented.
bool NexusProgress::OnProgress(F32 percent)
{
//	assert(m_thread.Dialog());
//	NexusProgressDialog* pDialog = (NexusProgressDialog*)m_thread.Dialog();
//	return pDialog->OnProgress(percent);
	return true;
}

//Callback function for printing output to the user while they wait.
void NexusProgress::OnPrintProgress(const std::string& text)
{
}

