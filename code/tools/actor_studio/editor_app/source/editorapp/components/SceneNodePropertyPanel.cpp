#include "stdafx.h"

#include <wx/aui/aui.h>
#include <wx/wxflatnotebook/wxFlatNotebook.h>

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/propdev.h>
#include <wx/propgrid/advprops.h>
#include <wx/propgrid/manager.h>

#include <EditorApp/Components/SceneNodePropertyPanel.h>
#include <EditorApp/Properties/SceneNodeProperties.h>

#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

namespace Editor
{
C_SceneNodePropertyPanel::C_Slots::C_Slots(C_SceneNodePropertyPanel *pPanel)
: m_pSceneNodePropertyPanel(pPanel)
{
}

void C_SceneNodePropertyPanel::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();
	s.coreSelectionChanged.connect(this, &Editor::C_SceneNodePropertyPanel::C_Slots::OnCoreSelectionChangedSlot);
	s.dgNodeUpdated.connect(this, &Editor::C_SceneNodePropertyPanel::C_Slots::OnDgNodeUpdatedSlot);
	s.dgNodeDestroy.connect(this, &Editor::C_SceneNodePropertyPanel::C_Slots::OnDgNodeDestroySlot);
}

void C_SceneNodePropertyPanel::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();
	s.coreSelectionChanged.disconnect(this);
	s.dgNodeUpdated.disconnect(this);
	s.dgNodeDestroy.disconnect(this);
}

void C_SceneNodePropertyPanel::C_Slots::OnCoreSelectionChangedSlot()
{
	m_pSceneNodePropertyPanel->OnSelectionChanged();
}

void C_SceneNodePropertyPanel::C_Slots::OnDgNodeUpdatedSlot(const EditorLib::I_DgNode &dgNode, const EditorLib::C_UpdateHintCollection* pHc)
{
	m_pSceneNodePropertyPanel->OnDgNodeUpdated(dgNode);
}

void C_SceneNodePropertyPanel::C_Slots::OnDgNodeDestroySlot(const EditorLib::I_DgNode& dgNode)
{
	m_pSceneNodePropertyPanel->OnDgNodeDestroy(dgNode);
}

BEGIN_EVENT_TABLE(C_SceneNodePropertyPanel,wxPanel)
END_EVENT_TABLE()

C_SceneNodePropertyPanel::C_SceneNodePropertyPanel(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: wxPanel(parent, id, pos, size, style)
, m_Slots(this)
{
	SetBackgroundColour(wxColour(255, 255, 255));

	wxBoxSizer* pBoxSizer;
	pBoxSizer = new wxBoxSizer(wxVERTICAL);

	m_pPropertyGridManager = new wxPropertyGridManager(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	m_pProperties = new C_SceneNodeProperties(m_pPropertyGridManager);

	const wxColour lightgray(212,208,200);
	const wxColour darkgray(113,111,100);

	m_pPropertyGridManager->GetGrid()->SetWindowStyle(wxNO_BORDER);
	m_pPropertyGridManager->GetGrid()->SetMarginColour(*wxWHITE);
	m_pPropertyGridManager->GetGrid()->SetCaptionBackgroundColour(*wxWHITE);
	m_pPropertyGridManager->GetGrid()->SetCellBackgroundColour(*wxWHITE);
	m_pPropertyGridManager->GetGrid()->SetCellTextColour(darkgray);
	m_pPropertyGridManager->GetGrid()->SetCellDisabledTextColour(lightgray);
	m_pPropertyGridManager->GetGrid()->SetLineColour(lightgray);

	pBoxSizer->Add(m_pPropertyGridManager, 1, wxALL|wxEXPAND, 0 );

	m_pLabel = new wxStaticText(this, wxID_ANY, "No Transform Data Available", wxDefaultPosition, wxDefaultSize, 0);
	m_pLabel->SetForegroundColour(lightgray);
	pBoxSizer->Add(m_pLabel, 0, wxALL|wxEXPAND, 5);

	m_pPropertyGridManager->GetGrid()->SetInternalFlag(wxPG_FL_SPLITTER_PRE_SET);
	SetSizer(pBoxSizer);
	Layout();
	pBoxSizer->Fit(this);

	Connect(wxID_ANY, wxID_ANY, wxEVT_PG_CHANGED, wxObjectEventFunction(&C_SceneNodePropertyPanel::OnPropertyChange), 0, this);

	m_Slots.Connect();
}

C_SceneNodePropertyPanel::~C_SceneNodePropertyPanel()
{
	//m_Slots.Disconnect();
	delete m_pProperties;
	delete m_pPropertyGridManager;
}

void C_SceneNodePropertyPanel::OnSelectionChanged()
{
	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	//workaround compiler error obtaining a ConstReverseIterator from a const object
	EditorLib::C_Selection::ItemArray& items = const_cast<EditorLib::C_Selection::ItemArray&>(selection.GetItems());
	EditorLib::C_Selection::ItemArray::ConstReverseIterator it = items.ReverseBegin();

	while (it != items.ReverseEnd())
	{
		EditorLib::I_DgNode *pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(it->id);
		if (!pNode)
			it++;
		else
			break;
	}

	//Select last item in list.
	EditorLib::I_DgNode* pNode = (it == items.ReverseEnd())
									? 0
									: EditorLib::C_Core::Instance().GetDgManager().FindNodeById(it->id);

	Btl::String name = (pNode) ? pNode->GetName() : "Current";

	if (GetParent() && GetParent()->IsKindOf(CLASSINFO(wxFlatNotebook)))
	{
		wxFlatNotebook *pParent = (wxFlatNotebook*)GetParent();
		S32 pageIndex = pParent->GetPageIndex(this);
		pParent->SetPageText(pageIndex, name.Chars());
	}
	else if (GetParent() && GetParent()->IsKindOf(CLASSINFO(wxAuiNotebook)))
	{
		wxAuiNotebook *pParent = (wxAuiNotebook*)GetParent();
		S32 pageIndex = pParent->GetPageIndex(this);
		pParent->SetPageText(pageIndex, name.Chars());
	}

	m_pProperties->SetDgNode(pNode);

	m_pLabel->Show(m_pPropertyGridManager->GetSelectedPage()==-1);
	Layout();
}

void C_SceneNodePropertyPanel::OnDgNodeUpdated(const EditorLib::I_DgNode& dgNode)
{
	if (&dgNode == m_pProperties->GetDgNode())
	{
		if (GetParent() && GetParent()->IsKindOf(CLASSINFO(wxFlatNotebook)))
		{
			wxFlatNotebook *pParent = (wxFlatNotebook*)GetParent();
			S32 pageIndex = pParent->GetPageIndex(this);
			pParent->SetPageText(pageIndex, dgNode.GetName().Chars());
		}
		else if (GetParent() && GetParent()->IsKindOf(CLASSINFO(wxAuiNotebook)))
		{
			wxAuiNotebook *pParent = (wxAuiNotebook*)GetParent();
			S32 pageIndex = pParent->GetPageIndex(this);
			pParent->SetPageText(pageIndex, dgNode.GetName().Chars());
		}

		m_pProperties->Refresh();
	}
}

void C_SceneNodePropertyPanel::OnDgNodeDestroy(const EditorLib::I_DgNode& dgNode)
{
	if (&dgNode == m_pProperties->GetDgNode())
	{
		m_pProperties->SetDgNode(0);

		if (GetParent() && GetParent()->IsKindOf(CLASSINFO(wxFlatNotebook)))
		{
			wxFlatNotebook *pParent = (wxFlatNotebook*)GetParent();
			S32 pageIndex = pParent->GetPageIndex(this);
			pParent->SetPageText(pageIndex, _T(""));
		}
		else if (GetParent() && GetParent()->IsKindOf(CLASSINFO(wxAuiNotebook)))
		{
			wxAuiNotebook *pParent = (wxAuiNotebook*)GetParent();
			S32 pageIndex = pParent->GetPageIndex(this);
			pParent->SetPageText(pageIndex, _T(""));
		}
	}
}

void C_SceneNodePropertyPanel::OnPropertyChange(wxPropertyGridEvent& e)
{
	m_pProperties->OnPropertyChange(e);
}

C_SceneNodeProperties* C_SceneNodePropertyPanel::GetSceneNodePropertiesPtr()
{
	return m_pProperties;
}

const C_SceneNodeProperties* C_SceneNodePropertyPanel::GetSceneNodePropertiesPtr() const
{
	return m_pProperties;
}

}
