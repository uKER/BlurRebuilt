#include "stdafx.h"

#include <EditorLib/Dg/SceneXtNodeBase.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Xt/XtTypeDatabaseHelpers.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

#include <EditorApp/Toolbars/ActorToolPanel.h>


using namespace Editor;

C_ActorToolPanel::C_Slots::C_Slots(C_ActorToolPanel *pToolPanel)
: m_pToolPanel(pToolPanel)
{
}

void C_ActorToolPanel::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.coreSelectionChanged.connect(this, &Editor::C_ActorToolPanel::C_Slots::OnCoreSelectionChangedSlot);
	s.databaseEvent.connect(this, &Editor::C_ActorToolPanel::C_Slots::OnDatabaseEventSlot);
}

void C_ActorToolPanel::C_Slots::OnCoreSelectionChangedSlot()
{
	m_pToolPanel->OnSelectionChanged();
}

void C_ActorToolPanel::C_Slots::OnDatabaseEventSlot(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt)
{
	m_pToolPanel->OnDatabaseEvent(db, evt);
}

void C_ActorToolPanel::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();
	s.coreSelectionChanged.disconnect(this);
	s.databaseEvent.disconnect(this);
}


C_ActorToolPanel::C_ActorToolPanel(
	wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: wxPanel(parent, id, pos, size, style)
, m_Slots(this)
{
	wxBoxSizer* pSizer;
	pSizer = new wxBoxSizer(wxHORIZONTAL);

	m_pActorTypeComboBox = new wxComboBox(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_DROPDOWN|wxCB_READONLY|wxCB_SORT); 
	pSizer->Add(m_pActorTypeComboBox, 0, wxALL, 0);
	
	pSizer->Fit(this);
	SetSizer(pSizer);
	Layout();

	m_pActorTypeComboBox->Enable(false);
	Connect(wxID_ANY, wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(C_ActorToolPanel::OnActorTypeChanged));
	m_Slots.Connect();
}

C_ActorToolPanel::~C_ActorToolPanel()
{
//	m_Slots.Disconnect();
}

void C_ActorToolPanel::OnDatabaseEvent(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt)
{
	if (evt == EditorLib::E_DatabaseEvent_Opened)
	{
		typedef Ut::UtMap<Btl::String, const LtStruct*> ActorTypeMap;
		ActorTypeMap actorTypes;
		{
			Btl::Vector<const EditorLib::I_XtType*> types;
			EditorLib::C_Core::Instance().GetXtTypeDatabase().BuildTypeList(types);

			for (S32 i = 0; i < types.Size(); i++)
			{
				const EditorLib::I_XtType* pType = types[i];
				if (pType->IsA(EditorLib::E_XtType_SceneXtActorNode) ||
					pType->IsA(EditorLib::E_XtType_SceneXtNode))
				{
					assert(pType->GetXtType()->kind == kLtStruct);
					const LtStruct* pLtStruct = (LtStruct*)pType->GetXtType();
					//Ignore types that are hidden.
					if (pLtStruct->hidden)
						continue;
					actorTypes.Insert(
						ActorTypeMap::DataType(
							Btl::String((const char*)pLtStruct->label), pLtStruct));
				}
			}
		}

		wxArrayString actorTypeStrings;

		for (ActorTypeMap::Iterator it = actorTypes.Begin(); 
			it != actorTypes.End(); ++it)
		{
			const LtStruct* pLtStruct = it->GetValue();
			actorTypeStrings.Add((const char*)pLtStruct->label);
		}

		m_pActorTypeComboBox->Append(actorTypeStrings);
	} else if (evt == EditorLib::E_DatabaseEvent_Closing)
	{
		m_pActorTypeComboBox->Clear();
	}
}

void C_ActorToolPanel::OnSelectionChanged()
{
	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	//workaround compiler error obtaining a ConstReverseIterator from a const object
	EditorLib::C_Selection::ItemArray& items = const_cast<EditorLib::C_Selection::ItemArray&>(selection.GetItems());
	EditorLib::C_Selection::ItemArray::ConstReverseIterator it = items.ReverseBegin();

	while (it != items.ReverseEnd())
	{
		EditorLib::I_DgNode *pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(it->id);
		if (!pNode || (pNode && pNode->IsA(EditorLib::E_ObjType_SceneAftkNode)))
			it++;
		else
			break;
	}

	if (it == items.ReverseEnd())
	{
		m_pActorTypeComboBox->Enable(false);
		return;
	}

	//Select last item in list.
	EditorLib::I_DgNode* pNode = 
		EditorLib::C_Core::Instance().GetDgManager().FindNodeById(it->id);

	if (!pNode)
	{
		m_pActorTypeComboBox->Enable(false);
		return;
	}

	EditorLib::C_SceneXtNodeBase* pSceneXtNode =
		pNode->IsA(EditorLib::E_ObjType_SceneXtNodeBase) ? (EditorLib::C_SceneXtNodeBase*)pNode : 0;
	if (!pSceneXtNode || !pSceneXtNode->GetXtType())
	{
		m_pActorTypeComboBox->Enable(false);
		return;
	}

	BIZ_ASSERT(pSceneXtNode->GetXtType()->kind == kLtStruct);
	const LtStruct* pLtStruct = (LtStruct*)pSceneXtNode->GetXtType();
	m_pActorTypeComboBox->SetValue((const char*)pLtStruct->label);
	m_pActorTypeComboBox->Enable(true);
}

void C_ActorToolPanel::OnActorTypeChanged(wxCommandEvent& e)
{
	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	//workaround compiler error obtaining a ConstReverseIterator from a const object
	EditorLib::C_Selection::ItemArray& items = const_cast<EditorLib::C_Selection::ItemArray&>(selection.GetItems());
	EditorLib::C_Selection::ItemArray::ConstReverseIterator it = items.ReverseBegin();

	while (it != items.ReverseEnd())
	{
		EditorLib::I_DgNode *pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(it->id);
		if (!pNode || (pNode && pNode->IsA(EditorLib::E_ObjType_SceneAftkNode)))
			it++;
		else
			break;
	}

	if (it == items.ReverseEnd())
		return;

	//Select last item in list.
	EditorLib::I_DgNode* pNode = 
		EditorLib::C_Core::Instance().GetDgManager().FindNodeById(it->id);

	if (!pNode)
		return;

	EditorLib::C_SceneXtNodeBase* pSceneXtNode =
		pNode->IsA(EditorLib::E_ObjType_SceneXtNodeBase) ? (EditorLib::C_SceneXtNodeBase*)pNode : 0;

	if (!pSceneXtNode)
		return;
	
	{
		Btl::Vector<const EditorLib::I_XtType*> types;
		EditorLib::C_Core::Instance().GetXtTypeDatabase().BuildTypeList(types);

		for (S32 i = 0; i < types.Size(); i++)
		{
			const EditorLib::I_XtType* pType = types[i];
			if (pType->IsA(EditorLib::E_XtType_SceneXtActorNode) ||
				pType->IsA(EditorLib::E_XtType_SceneXtNode))
			{
				assert(pType->GetXtType()->kind == kLtStruct);
				const LtStruct* pLtStruct = (LtStruct*)pType->GetXtType();
				//Ignore types that are hidden.
				if (pLtStruct->hidden)
					continue;

				if (m_pActorTypeComboBox->GetValue() == (const char*)pLtStruct->label)
					Xt::SetType(Xt::AsComponent(pSceneXtNode->GetXtData()), pLtStruct->name);
			}
		}
	}
}
