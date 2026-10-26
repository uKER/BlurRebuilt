#include "stdafx.h"
#include <sigslot/sigslot.h>

#include <xtwin/WndCore.h>
#include <xtwin/Scarab.h>
#include <XtWin/WndClasses.h>

#include <dialogueman/Components/Scarab_Form/WxScarabForm.h>
#include <dialogueman/Components/Scarab_Form/WxScarabFormContext.h>
#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/Row.h>
#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>
#include <DialogueMan/GlobalSignals.h>
#include <DialogueMan/App.h>

namespace DMan
{

C_WxScarabForm::C_WxScarabForm(
	wxFrame* pFrame,
	wxWindow *pParent,
	const wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxWindow(pParent, id, pos, size, style),
	m_pFrame(pFrame)
	,m_slots(this)
{
	srand(GetTickCount());

	RegisterXtWinClasses();

	m_pScarabForm = 0;
	SetBackgroundColour(*wxWHITE);

	Connect(GetId(), wxEVT_SIZE, wxSizeEventHandler(C_WxScarabForm::OnSize));
	Connect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(C_WxScarabForm::OnIdle));

	m_flags.dirty = false;

	m_slots.Connect();
}

C_WxScarabForm::~C_WxScarabForm()
{
	delete m_pScarabForm;
}

void C_WxScarabForm::Shutdown()
{
	SetDom((Xt::DomElement*)0, 0);
	m_slots.Disconnect();
}

void C_WxScarabForm::OnIdle(wxIdleEvent& e)
{
	if (!m_flags.dirty)
		return;

	m_flags.dirty = false;
	
	C_Selection selection;
	wxGetApp().GetSelection(selection);

	C_Selection::ItemArray arr = selection.GetItems();
	if (arr.Size() == 0)
	{
		SAFE_DELETE_PTR(m_pScarabForm);
		return;
	}

	C_Selection::C_Item item = arr.Back();
	if (selection.GetMode() == C_Selection::E_Mode_Row)
	{
		C_Record* pRecord = wxGetApp().GetDbManager().GetRecord(item.id.GetRecordId());

		if (item.components.Size() > 0)
		{
			U32 rowIndex = item.components.Back();

			C_RowSet* pRowSet = pRecord->GetRowSet(item.id.GetChildId());
			if (pRowSet)
			{
				C_Row* pRow = pRowSet->GetRow(rowIndex);
				if(pRow)
					SetDom(Xt::AsComponent(pRow->GetXtData()), pRow->GetXtType().AsType());
				else
					SetDom((Xt::DomElement*)0, 0);
				return;
			}
		}
	}

	SetDom((Xt::DomElement*)0, 0);
}

bool C_WxScarabForm::SetRecord(Xt::DomRecord* pRecord)
{
	if (m_pScarabForm)
		delete m_pScarabForm;

	if ((m_pScarabForm = new ScarabForm(new C_WxScarabFormContext(this))) == 0)
		return false;

	m_pScarabForm->Create((HWND)GetHandle());

	WinRect rcClient((HWND)GetHandle());
	SetWindowPos(*m_pScarabForm, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

	m_pScarabForm->SetRecord(pRecord);
	return true;
}

bool C_WxScarabForm::SetDom(Xt::DomElement* pElement, LarvaType* pLarvaType)
{
	if (!m_pScarabForm)
	{
		if ((m_pScarabForm = new ScarabForm(new C_WxScarabFormContext(this))) == 0)
			return false;

		m_pScarabForm->Create((HWND)GetHandle());
	}

	WinRect rcClient((HWND)GetHandle());
	SetWindowPos(*m_pScarabForm, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

	m_pScarabForm->SetDom(pElement, pLarvaType);

	return true;
}

bool C_WxScarabForm::SetDom(Xt::DomComponent* pComponent, LarvaType* pLarvaType)
{
	if (!m_pScarabForm)
	{
		if ((m_pScarabForm = new ScarabForm(new C_WxScarabFormContext(this))) == 0)
			return false;

		m_pScarabForm->Create((HWND)GetHandle());
	}

	WinRect rcClient((HWND)GetHandle());
	SetWindowPos(*m_pScarabForm, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

	m_pScarabForm->SetDom(&pComponent->data, pLarvaType);

	return true;
}

bool C_WxScarabForm::SetDom(Xt::DomBlock* pBlock, LarvaType* pLarvaType)
{
	if (!m_pScarabForm)
	{
		if ((m_pScarabForm = new ScarabForm(new C_WxScarabFormContext(this))) == 0)
			return false;

		m_pScarabForm->Create((HWND)GetHandle());
	}

	WinRect rcClient((HWND)GetHandle());
	SetWindowPos(*m_pScarabForm, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

	m_pScarabForm->SetDom(pBlock, pLarvaType);

	return true;
}

void C_WxScarabForm::OnDbClose()
{
}

void C_WxScarabForm::OnSelectionChanged()
{
	m_flags.dirty = true;
}

ScarabForm* C_WxScarabForm::GetScarabForm()
{
	return m_pScarabForm;
}

void C_WxScarabForm::OnSize(wxSizeEvent& e)
{
	if (!m_pScarabForm)
		return;

	WinRect rcClient((HWND)GetHandle());
	SetWindowPos(*m_pScarabForm, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}


C_WxScarabForm::C_Slots::C_Slots(C_WxScarabForm* pForm)
:
m_pForm(pForm)
{
}
	
void C_WxScarabForm::C_Slots::Connect()
{
	C_GlobalSignals& s = wxGetApp().GetGlobalSignals();

	s.appSelectionChanged.connect(this, &C_WxScarabForm::C_Slots::OnSelectionChangedSlot);
	s.appDbClose.connect(this, &C_WxScarabForm::C_Slots::OnDbCloseSlot);
}
		
void C_WxScarabForm::C_Slots::Disconnect()
{
	C_GlobalSignals& s = wxGetApp().GetGlobalSignals();

	s.appSelectionChanged.disconnect(this);
	s.appDbClose.disconnect(this);
}

void C_WxScarabForm::C_Slots::OnSelectionChangedSlot()
{
	m_pForm->OnSelectionChanged();
}

void C_WxScarabForm::C_Slots::OnDbCloseSlot()
{
	m_pForm->OnDbClose();
}

}
