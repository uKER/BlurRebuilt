#include <nexus_app/stdafx.h>
#include <nexus_app/Nexus.h>

#include <nexus_app/MainFrm.h>

#include "../../resource.h"
#include <nexus_app/NexusSystem.h>
#include <nexus_app/NexusDoc.h>
#include <nexus_app/CommandEdit.h>
#include "..\..\include\nexus_app\commandedit.h"

using namespace Nexus;

CommandEdit::CommandEdit() : CEdit()
{
	m_pHistoryItem = 0;
}

CommandEdit::~CommandEdit()
{
}

void CommandEdit::ClearCommand()
{
	m_pHistoryItem = 0;
	SendMessage(WM_SETTEXT, 0, 0);
}

void CommandEdit::AddHistory(char* history)
{
	if (strlen(history) == 0)
		return;

	if (m_commandHistory.Size() != 0)
	{
		if (strcmp(m_commandHistory.End()->GetData().c_str(), history))
			m_commandHistory.Add(std::string(history));
	}
	else
	{
		m_commandHistory.Add(std::string(history));
	}

	if (m_commandHistory.Size() > 50)
		m_commandHistory.Remove(m_commandHistory.Begin());
}



BEGIN_MESSAGE_MAP(Nexus::CommandEdit, CEdit)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
END_MESSAGE_MAP()

void CommandEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	KeyboardEvent(nChar, nRepCnt, nFlags, true);

	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CommandEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CommandEdit::KeyboardEvent(UINT nChar, UINT nRepCnt, UINT nFlags, bool pressed)
{
	bool commandHistoryBrowse = false;
	switch (nChar)
	{
		case VK_DOWN:
		{
			commandHistoryBrowse = true;
			if (!m_pHistoryItem)
				m_pHistoryItem = m_commandHistory.Begin();
			else if (m_pHistoryItem->GetNext())
				m_pHistoryItem = m_pHistoryItem->GetNext();
			break;
		}

		case VK_UP:
		{
			commandHistoryBrowse = true;
			if (!m_pHistoryItem)
				m_pHistoryItem = m_commandHistory.End();
			else if (m_pHistoryItem->GetPrev())
					m_pHistoryItem = m_pHistoryItem->GetPrev();
			break;
		}
	}

	if (m_pHistoryItem && commandHistoryBrowse)
	{
		SendMessage(WM_SETTEXT, 0, (LPARAM)m_pHistoryItem->GetData().c_str());
		UpdateData();
	}
}