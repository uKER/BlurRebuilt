#include "stdafx.h"

#include "system/misc/StringUtil.h"
#include "wx/treelistctrl/treelistctrl.h"

#include <EditorLib/Aftk/AftkRefHelpers.h>

#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/IDgNode.h>

#include <EditorLib/Misc/Selection.h>

#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Xt/XtHelpers.h>
#include <EditorLib/Xt/XtManager.h>

#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>
#include <EditorLib/Level/XtLevel.h>

#include <EditorApp/Components/HypergraphDialog.h>

using namespace Editor;

C_HypergraphPanel::C_Slots::C_Slots(C_HypergraphPanel *pDialog)
: m_pDialog(pDialog)
{
}

void C_HypergraphPanel::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.coreEnableViews.connect(this, &Editor::C_HypergraphPanel::C_Slots::OnCoreEnableViewsSlot);
	s.coreSelectionChanged.connect(this, &Editor::C_HypergraphPanel::C_Slots::OnCoreSelectionChangedSlot);
	s.dgNodeUpdated.connect(this, &Editor::C_HypergraphPanel::C_Slots::OnDgNodeUpdatedSlot);
	s.dgNodeDestroy.connect(this, &Editor::C_HypergraphPanel::C_Slots::OnDgNodeDestroyedSlot);
	s.levelOpening.connect(this, &Editor::C_HypergraphPanel::C_Slots::OnLevelOpeningSlot);
	s.levelOpened.connect(this, &Editor::C_HypergraphPanel::C_Slots::OnLevelOpenedSlot);
	s.levelClosing.connect(this, &Editor::C_HypergraphPanel::C_Slots::OnLevelClosingSlot);
	s.levelClosed.connect(this, &Editor::C_HypergraphPanel::C_Slots::OnLevelClosedSlot);
}

void C_HypergraphPanel::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.coreEnableViews.disconnect(this);
	s.coreSelectionChanged.disconnect(this);
	s.dgNodeUpdated.disconnect(this);
	s.dgNodeDestroy.disconnect(this);
	s.levelOpening.disconnect(this);
	s.levelOpened.disconnect(this);
	s.levelClosing.disconnect(this);
	s.levelClosed.disconnect(this);
}

void C_HypergraphPanel::C_Slots::OnCoreEnableViewsSlot(bool enable)
{
	m_pDialog->OnCoreEnableViews(enable);
}

void C_HypergraphPanel::C_Slots::OnCoreSelectionChangedSlot()
{
	m_pDialog->OnCoreSelectionChanged();
}

void C_HypergraphPanel::C_Slots::OnDgNodeUpdatedSlot(const EditorLib::I_DgNode& dgNode, const EditorLib::C_UpdateHintCollection* pHc)
{
	m_pDialog->OnDgNodeUpdated(dgNode);
}

void C_HypergraphPanel::C_Slots::OnDgNodeDestroyedSlot(const EditorLib::I_DgNode& dgNode)
{
	m_pDialog->OnDgNodeDestroyed(dgNode);
}

void C_HypergraphPanel::C_Slots::OnLevelOpeningSlot(const EditorLib::I_Level& level)
{
	m_pDialog->OnLevelOpening(level);
}
			
void C_HypergraphPanel::C_Slots::OnLevelOpenedSlot(const EditorLib::I_Level& level)
{
	m_pDialog->OnLevelOpened(level);
}

void C_HypergraphPanel::C_Slots::OnLevelClosingSlot(const EditorLib::I_Level& level)
{
	m_pDialog->OnLevelClosing(level);
}

void C_HypergraphPanel::C_Slots::OnLevelClosedSlot(const EditorLib::I_Level& level)
{
	m_pDialog->OnLevelClosed(level);
}

IMPLEMENT_CLASS(C_HypergraphPanel, wxPanel)

C_HypergraphPanel::C_HypergraphPanel(wxWindow *parent,  wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: wxPanel(parent, id, pos, size, style)
, m_Slots(this)
, m_connected(false)
{
	wxBoxSizer* pSizer0;
	pSizer0 = new wxBoxSizer( wxVERTICAL );

	m_pTreeCtrl = new wxTreeCtrl( 
		this, 
		wxID_ANY,
		wxDefaultPosition, 
		wxDefaultSize, 
		wxTR_DEFAULT_STYLE|
		wxTR_HAS_BUTTONS|
		wxTR_EDIT_LABELS|
		wxTR_EXTENDED|
		wxTR_FULL_ROW_HIGHLIGHT|
		wxTR_HAS_VARIABLE_ROW_HEIGHT|
		wxTR_HIDE_ROOT|
		wxTR_TWIST_BUTTONS|
		wxHSCROLL|
		wxNO_BORDER|
		wxTAB_TRAVERSAL|
		wxVSCROLL);

	pSizer0->Add( m_pTreeCtrl, 1, wxALL|wxEXPAND, 0 );

	SetSizer(pSizer0);
	Layout();

	Connect(wxID_ANY, wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler(C_HypergraphPanel::OnNodeSelect));
	Connect(wxID_ANY, wxEVT_SHOW, wxShowEventHandler(C_HypergraphPanel::OnShow));
	Connect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(C_HypergraphPanel::OnIdle));

	m_Flags.pendingSync = false;
	m_Flags.dirty = false;
	m_viewEnabled = 1;

	m_Slots.Connect();
	m_connected = true;
}

C_HypergraphPanel::~C_HypergraphPanel()
{
	Shutdown();
}

void C_HypergraphPanel::Shutdown()
{
	if (m_connected)
	{
		Disconnect(wxID_ANY, wxEVT_SHOW, wxShowEventHandler(C_HypergraphPanel::OnShow));

		m_Slots.Disconnect();
		m_connected = false;
	}
}

void C_HypergraphPanel::OnShow(wxShowEvent& e)
{
	if (e.GetShow())
	{
		m_Flags.dirty = true;
		m_viewEnabled++;
	} else {
		m_viewEnabled--;
		EditorLib::C_NodeId invalidId;
		SelectItem(invalidId);
	}
}

void C_HypergraphPanel::OnIdle(wxIdleEvent& e)
{
	if (m_Flags.pendingSync)
		return;

	if (m_Flags.dirty && m_viewEnabled)
	{
		m_Flags.dirty = false;
		m_pTreeCtrl->Freeze();
		SelectItem(m_SelectedItem);
		m_pTreeCtrl->Thaw();
	}
}

void C_HypergraphPanel::OnDgNodeUpdated(const EditorLib::I_DgNode& dgNode)
{
	m_Flags.dirty = true;
}

void C_HypergraphPanel::OnDgNodeDestroyed(const EditorLib::I_DgNode& dgNode)
{
	m_Flags.dirty = true;
}

void C_HypergraphPanel::OnCoreEnableViews(bool enable)
{
	m_viewEnabled += enable ? 1 : -1;
}

void C_HypergraphPanel::OnCoreSelectionChanged()
{
	m_pTreeCtrl->Freeze();

	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	//workaround compiler error obtaining a ConstReverseIterator from a const object
	EditorLib::C_Selection::ItemArray& items = const_cast<EditorLib::C_Selection::ItemArray&>(selection.GetItems());
	EditorLib::C_Selection::ItemArray::ConstReverseIterator it = items.ReverseBegin();

	EditorLib::I_DgNode *pNode = 0;

	EditorLib::C_NodeId invalidId;
	m_SelectedItem = invalidId;
	m_Flags.dirty = true;

	while (it != items.ReverseEnd())
	{
		pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(it->id);
		if (!pNode)
		{
			it++;
		}
		else
		{
			m_SelectedItem = it->id;
			break;
		}
	}

	m_pTreeCtrl->Thaw();
}

void C_HypergraphPanel::OnLevelOpening(const EditorLib::I_Level& level)
{
	m_Flags.pendingSync = true;
}

void C_HypergraphPanel::OnLevelOpened(const EditorLib::I_Level& level)
{
	m_Flags.pendingSync = false;
}

void C_HypergraphPanel::OnLevelClosing(const EditorLib::I_Level& level)
{
	m_Flags.pendingSync = true;
	m_pTreeCtrl->Freeze();
	EditorLib::C_NodeId invalidId;
	SelectItem(invalidId);
	m_pTreeCtrl->Thaw();
}

void C_HypergraphPanel::OnLevelClosed(const EditorLib::I_Level& level)
{
	assert(m_Flags.pendingSync);
	m_Flags.pendingSync = false;
}

void C_HypergraphPanel::SelectItem(const EditorLib::C_NodeId& nodeId)
{
	EditorLib::C_XtObject* pXtObject;
	EditorLib::I_DgNode* pNode;
	
	pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(nodeId);

	m_pTreeCtrl->Freeze();

	m_pTreeCtrl->UnselectAll();
	m_pTreeCtrl->DeleteAllItems();

	if (pNode)
		pXtObject = pNode->AsXtObject();
	else
		pXtObject = 0;
		
	if (!pXtObject || !pXtObject->GetXtData() || pNode->IsA(EditorLib::E_ObjType_SceneAftkNode) || pXtObject->GetXtData()->kind != Xt::kDomComponent || 
		!pXtObject->GetXtType() || pXtObject->GetXtType()->kind != kLtStruct)
	{
		m_pTreeCtrl->Thaw();
		return;
	}

	Xt::DomComponent* pComponent = Xt::AsComponent(pXtObject->GetXtData());
	LtStruct* type = AsStruct(pXtObject->GetXtType());

	for (size_t i=0; i < type->fields.count; i++)
	{
		LarvaField* pField = type->fields[i];
		
		Xt::DomElement* element = Xt::At(&pComponent->data, pField->name);
	}

	m_pTreeCtrl->AddRoot("");
	wxTreeItemId root = m_pTreeCtrl->GetRootItem();
	m_pRootItem = m_pTreeCtrl->AppendItem(root, pNode->GetName().Chars());
	m_pRootSourceItem = m_pTreeCtrl->AppendItem(m_pRootItem, "Outgoing Messages");
	m_pRootDestinationItem = m_pTreeCtrl->AppendItem(m_pRootItem, "Incoming Messages");
	m_pRootChildOfItem = m_pTreeCtrl->AppendItem(m_pRootItem, "Child Of");
	m_pRootParentOfItem = m_pTreeCtrl->AppendItem(m_pRootItem, "Parent Of");
	
	ParseDatabase(pXtObject);

	m_pTreeCtrl->Expand(m_pRootItem);
	m_pTreeCtrl->Expand(m_pRootSourceItem);
	m_pTreeCtrl->Expand(m_pRootDestinationItem);
	m_pTreeCtrl->Expand(m_pRootChildOfItem);
	m_pTreeCtrl->Expand(m_pRootParentOfItem);

	m_pTreeCtrl->Thaw();
}

C_HypergraphPanel::HypergraphVisitor::HypergraphVisitor(Btl::Vector<Btl::Pair<LtStruct*,Xt::DomBlock*> >& dom, const XtString& value) : m_Dom(dom), m_Value(value)
{
}

C_HypergraphPanel::HypergraphVisitor::~HypergraphVisitor()
{
}

bool C_HypergraphPanel::HypergraphVisitor::Visit(struct LtStruct* type, Xt::DomBlock* dom)
{
	XtString value = GetAttr(type, "SceneView", "hypergraph");

	if (value != m_Value)
		return true;

	m_Dom.PushBack(Btl::MakePair(type, dom));

	return true;
}

XtString C_HypergraphPanel::GetAttr(struct LtStruct* type, XtString attr, XtString param)
{
	XtString value = type->attrs.GetString(attr, param);
	if (value != kXtNullString)
		return value;

	if (type->base)
	{
		XtString value = GetAttr(type->base->type, attr, param);
		if (value != kXtNullString)
			return value;
	}

	return kXtNullString;
}

void C_HypergraphPanel::ParseDatabase(EditorLib::C_XtObject* selectedItem)
{
	if (!selectedItem)
		return;

	LarvaSchema* schema = EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema;
	Xt::DomElement* data = EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetXtData();
	LarvaType* type = EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetXtType();

	Btl::Vector<Btl::Pair<LtStruct*,Xt::DomBlock*> > messages;
	HypergraphVisitor messageVisitor(messages, "message");
	VisitAllObjects(schema, (LtStruct*)type, Xt::AsComponent(data), &messageVisitor, false);

	for (Btl::Vector<Btl::Pair<LtStruct*,Xt::DomBlock*> >::Iterator it = messages.Begin(); it != messages.End(); ++it)
	{
		ParseMessage(selectedItem, it->first, it->second);
	}

	Btl::Vector<Btl::Pair<LtStruct*,Xt::DomBlock*> > hasParent;
	HypergraphVisitor hasParentVisitor(hasParent, "hasparent");
	VisitAllObjects(schema, (LtStruct*)type, Xt::AsComponent(data), &hasParentVisitor, false);

	for (Btl::Vector<Btl::Pair<LtStruct*,Xt::DomBlock*> >::Iterator it = hasParent.Begin(); it != hasParent.End(); ++it)
	{
		ParseHasParent(selectedItem, it->first, it->second);
	}
}

XtId GetXtId(Xt::DomComponent* pComponent, LarvaField* field)
{
	if (field->kind == kLfWeakPtr)
		return Xt::AsId(Xt::At(&pComponent->data, field));

	if (field->kind == kLfStruct)
		return pComponent->id;

	return 0;
}

LarvaType* GetType(Xt::DomComponent* pBlock)
{
	XtString typeName = pBlock->type;
	
	if (typeName == kXtNullString)
		return 0;

	return EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema->types[typeName];
}

Xt::DomComponent* C_HypergraphPanel::FindParentActor(Xt::DomBlock* dom)
{
	Xt::DomElement* pDom = dom;
	while (pDom)
	{
		pDom = pDom->parent;

		Xt::DomComponent* pBlock = Xt::AsComponent(pDom);
		if (pBlock)
		{
			LarvaType* type = GetType(pBlock);
			if (type && type->kind == kLtStruct && GetAttr((LtStruct*)type, "SceneView", "hypergraph") == "actor")
				return pBlock;
		}
	}

	return 0;
}

XtString C_HypergraphPanel::GetName(Xt::DomComponent* block)
{
	Xt::DomField* nameField = Xt::AsField(Xt::At(&block->data, "Name"));
	
	if (nameField)
		return nameField->value;
	else
		return kXtNullString;
}

XtString C_HypergraphPanel::GetSummaryText(Xt::DomComponent* block)
{
	if (!block)
		return kXtNullString;

	return EvaluateSummaryText(EditorLib::C_Core::Instance().GetXtManager().GetDb(), EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, AsStruct(GetType(block)), block, "{__type}");
}

wxTreeItemId C_HypergraphPanel::GetTreeItem(wxTreeItemId root, wxString text)
{
	wxTreeItemId item;

	wxTreeItemIdValue cookie;
	wxTreeItemId child = m_pTreeCtrl->GetFirstChild(root, cookie);
	while (child)
	{
		if (m_pTreeCtrl->GetItemText(child) == text)
		{
			item = child;
			break;
		}
		child = m_pTreeCtrl->GetNextChild(root, cookie);
	}

	if (!item.IsOk())
		item = m_pTreeCtrl->AppendItem(root, text);

	return item;
}

void C_HypergraphPanel::ParseMessage(EditorLib::C_XtObject* selectedItem, LtStruct* type, Xt::DomBlock* dom)
{
	bool messageDataFound = false;
	Btl::Pair<Btl::Pair<LarvaField*, LarvaType*>, Xt::DomComponent*> messageData;
	
	bool paramActorDestinationFound = false;
	bool actorDestinationFound = false;
	Btl::Pair<Btl::Pair<LarvaField*, LarvaType*>, Xt::DomComponent*> messageDestination;

	paramActorDestinationFound = GetField(type, dom, "destination", "paramactor", messageDestination);
	actorDestinationFound = GetField(type, dom, "destination", "actor", messageDestination);

	messageDataFound = GetField(type, dom, "data", "messagedata", messageData);

	Xt::DomComponent* parentActor = FindParentActor(dom);

	if ((actorDestinationFound || paramActorDestinationFound) && parentActor)
	{
		Util::C_String messageType;
		if (messageDataFound)
		{
			messageType = GetSummaryText(messageData.second).Text();
		} else 
		{
			Xt::DomComponent* pComponent = Xt::AsComponent(dom->parent);
			if (pComponent)
				messageType = pComponent->type;
			else
				return;
		}

		XtId destinationActor = 0;

		if (paramActorDestinationFound)
		{
			if (messageDestination.first.second->kind != kLtStruct)
				return;

			LtStruct* structFields = AsStruct(messageDestination.first.second);

			for (size_t i=0; i<structFields->fields.count; i++)
			{
				XtString value = structFields->fields[i]->attrs.GetString("SceneView", "hypergraph");
				if (value == "destination")
				{
					destinationActor = GetXtId(messageDestination.second, structFields->fields[i]);
				}
			}
		}
		else
		{
			destinationActor = messageDestination.second->id;
		}

		if (destinationActor != 0)
		{
			if (Xt::AsComponent(selectedItem->GetXtData())->id == destinationActor)
			{
				wxTreeItemId message = GetTreeItem(m_pRootDestinationItem, messageType.Chars());
				
				EditorLib::C_NodeId nodeId(parentActor->id, EditorLib::kNoChildId);
				EditorLib::I_DgNode* pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(nodeId);
				
				m_pTreeCtrl->AppendItem(message, GetSummaryText(parentActor).Text(), -1, -1, new C_TreeItemData(pNode));

				m_pTreeCtrl->Expand(message);
			}
		}

		if (Xt::AsComponent(selectedItem->GetXtData())->id == parentActor->id)
		{
			wxTreeItemId message = GetTreeItem(m_pRootSourceItem, messageType.Chars());
			
			EditorLib::C_NodeId nodeId(destinationActor, EditorLib::kNoChildId);
			EditorLib::I_DgNode* pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(nodeId);

			XtId recordId = EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetXtRecordId();
			Xt::DomRecord* pLevelRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(recordId);
			m_pTreeCtrl->AppendItem(message, GetSummaryText(FindComponentById(EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, pLevelRecord, destinationActor).component).Text(), -1, -1, new C_TreeItemData(pNode));

			m_pTreeCtrl->Expand(message);
		}
	}
}

bool C_HypergraphPanel::GetField(LtStruct* type, Xt::DomBlock* dom, const Btl::String& fieldValue, const Btl::String& fieldTypeValue, Btl::Pair<Btl::Pair<LarvaField*, LarvaType*>, Xt::DomComponent*>& component)
{
	for (size_t i=0; i<type->fields.count; i++)
	{
		if (type->fields[i]->Type()->kind != kLtStruct)
			continue;

		XtString value = type->fields[i]->attrs.GetString("SceneView", "hypergraph");

		if (value == fieldValue.Chars())
		{
			if (GetAttr((LtStruct*)type->fields[i]->Type(), "SceneView", "hypergraph") != fieldTypeValue.Chars())
				continue;

			LarvaField* field = type->fields[i];

			Xt::DomElement* element = Xt::At(dom, field);
			if (!element || element->kind != Xt::kDomComponent && element->kind != Xt::kDomRef)
				continue;

			Xt::DomComponent* pComponent;

			if (element->kind == Xt::kDomRef)
			{
				XtId refId = Xt::AsId(Xt::AsRef(element));
				if (refId == 0)
					continue;

				EditorLib::C_XtLevel* pLevel = EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel();
				if (!pLevel)
					continue;

				XtId recordId = pLevel->GetXtRecordId();
				Xt::DomRecord* pLevelRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(recordId);

				pComponent = FindComponentById(EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, pLevelRecord, refId).component;
				if (!pComponent)
					continue;
			}
			else
			{
				pComponent = Xt::AsComponent(element);
			}

			LarvaType* fieldType = EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema->types[pComponent->type];
			if (fieldType == 0)
				continue;

			component = Btl::MakePair(Btl::MakePair(field, fieldType), pComponent);
			return true;
		}
	}

	if (type->base)
	{
		if (GetField(type->base->type, dom, fieldValue, fieldTypeValue, component))
			return true;
	}

	return false;
}

void C_HypergraphPanel::ParseHasParent(EditorLib::C_XtObject* selectedItem, LtStruct* type, Xt::DomBlock* dom)
{
	bool messageDestinationFound = false;
	Btl::Pair<Btl::Pair<LarvaField*, LarvaType*>, Xt::DomComponent*> messageDestination;

	messageDestinationFound = GetField(type, dom, "parent", "actor", messageDestination);

	Xt::DomComponent* parentActor = FindParentActor(dom);

	if (messageDestinationFound && parentActor)
	{
		XtId destinationActor = 0;

		if (messageDestination.first.second->kind != kLtStruct)
			return;

		LtStruct* structFields = AsStruct(messageDestination.first.second);

		destinationActor = messageDestination.second->id;

		if (destinationActor != 0)
		{
			if (Xt::AsComponent(selectedItem->GetXtData())->id == destinationActor)
			{
				EditorLib::C_NodeId nodeId(parentActor->id, EditorLib::kNoChildId);
				EditorLib::I_DgNode* pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(nodeId);
				
				m_pTreeCtrl->AppendItem(m_pRootParentOfItem, GetSummaryText(parentActor).Text(), -1, -1, new C_TreeItemData(pNode));
			}
		}

		if (Xt::AsComponent(selectedItem->GetXtData())->id == parentActor->id)
		{
			EditorLib::C_NodeId nodeId(destinationActor, EditorLib::kNoChildId);
			EditorLib::I_DgNode* pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(nodeId);

			XtId recordId = EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetXtRecordId();
			Xt::DomRecord* pLevelRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(recordId);
			m_pTreeCtrl->AppendItem(m_pRootChildOfItem, GetSummaryText(FindComponentById(EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, pLevelRecord, destinationActor).component).Text(), -1, -1, new C_TreeItemData(pNode));
		}
	}
}

void C_HypergraphPanel::OnNodeSelect(wxTreeEvent& e)
{
	if (!e.GetItem().IsOk())
		return;

	C_TreeItemData* pData = (C_TreeItemData*) m_pTreeCtrl->GetItemData(e.GetItem());

	if (!pData || !pData->GetData())
		return;

	EditorLib::C_Status status;
	EditorLib::C_Core::Instance().GetCommandProcessor().Execute("select", pData->GetData()->GetId().AsString());
}
