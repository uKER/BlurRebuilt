#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/SendEmail.h"
#include "XtWin/NodeGraph.h"
#include "../ScarabImpl.h"
#include "SnNgImpl.h"

// Add "Out" pins for each contained struct... So one for the top level object, then one for any internal ones. Don't create nodes for these ones
// So- Any field marked "Node" becomes a node, any field marked "Pin" is a pin

// TODO: Need some kind of Query() interface to upcast safely

// TODO: Do [Tab] view properly, do it as an annotation on the type, rather than a hidden control in the form (so the hacks can be removed)

// TODO: Add [NodeGraph(action:zoom)] to allow nested nodegraphs, also add breadcrumb trail to allow navigation back

SnNgFormatter::SnNgFormatter(const char* paramName) :
	_paramName(paramName)
{
}

bool SnNgFormatter::GetFormatByType(LtStruct* type, XtString& format)
{
	if (LarvaParam* param = type->FindParam("NodeGraph", _paramName))
	{
		format = param->AsString();
		return true;
	}

	if (LarvaParam* param = type->FindParam("Summary", "0"))
	{
		format = param->AsString();
		return true;
	}

	return false;
}

bool SnNgFormatter::GetFormatByOption(LarvaOption* option, XtString& format)
{
	if (LarvaParam* param = option->attrs.FindParam("NodeGraph", _paramName))
	{
		format = param->AsString();
		return true;
	}

	format = option->summary;

	return true;
}

//
// SnNgContext
//

class ScarabDomNodeGraphContext : public NodeGraphContext
{
public:
	ScarabDomNodeGraphContext(ScarabDomNodeGraph* ng) :
		_ng(ng)
	{
	}

public:
	void AddToTypes(XtUt::Array<LtStruct*>* types, LtStruct* type)
	{
		if (!type || type->hidden)
			return;

		for(XtUt::Array<LtStruct*>::Iterator it(types); it; ++it)
		{
			if (it == type)
				return;
		}

		XtUt::AppendEx(types, type);
	}

	void AddDerivedTypes(XtUt::Array<LtStruct*>* types, LtStruct* baseType)
	{
		Xt::DomDb* db = _ng->_form->Db();

		for(XtUt::Dictionary<XtString, LarvaType>::Iterator it(&db->schema->types); it; ++it)
		{
			if (it->kind != kLtStruct)
				continue;

			LtStruct* type = (LtStruct*)(LarvaType*)it;

			if (!IsBaseOf(baseType, type))
				continue;

			AddToTypes(types, type);
		}
	}

	XtString GetCategory(LtStruct* type)
	{
		if (!type)
			return kXtNullString;

		if (*type->category)
			return type->category;

		if (type->base)
		{
			XtString result = GetCategory(type->base->type);

			if (*result)
				return result;
		}

		return kXtNullString;
	}

	static bool SortTypes(LtStruct* lhs, LtStruct* rhs)
	{
		return _stricmp(lhs->label, rhs->label) < 0;
	}

	virtual void ContextMenu(const WinPoint& pt)
	{
		_ng->MegaUndoHack_Store();

		NgDiagram*	diagram	= _ng->Diagram();
		NgPin*		pin		= 0;

		if (diagram)
		{
			NgVector p = diagram->ToCanvas(pt.ScreenToClient(*_ng));

			pin = diagram->HitTestPin(p);

			NgNode*	node = diagram->HitTestNode(p);

			if (pin && node)
			{
				if (pin->Node()->IsBehind(node))
					pin = 0;
			}
		}

		if (pin)
			ContextMenu_Pin(pt, pin->Anchor());
		else
			ContextMenu_Standard(pt);
	}

	void ContextMenu_Pin(const WinPoint& pt, XtUt::WeakRef<NgPin> pinRef)
	{
		HMENU menu = CreatePopupMenu();

		SnNgPin* snPin = (SnNgPin*)(NgPin*)pinRef;

		if (snPin && snPin->ConnectionCount())
			AppendMenu(menu, MF_STRING, 50, "Disconnect");
		else
			AppendMenu(menu, MF_STRING | MF_GRAYED | MF_DISABLED, 50, "Disconnect");

		if (snPin && snPin->ConnectionCount())
			AppendMenu(menu, MF_STRING, 51, "Toggle Connections");
		else
			AppendMenu(menu, MF_STRING | MF_GRAYED | MF_DISABLED, 51, "Toggle Connections");

		int result = TrackPopupMenuEx(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY, pt.x, pt.y, *_ng, 0);

		DestroyMenu(menu);

		snPin = (SnNgPin*)(NgPin*)pinRef;

		if (snPin)
		{
			if (result == 50)
			{
				for(XtUt::LinkList<NgConnection>::Iterator it(snPin->Connections()); it; ++it)
				{
					SnNgPin* p0 = (SnNgPin*)it->From();
					SnNgPin* p1 = (SnNgPin*)it->To();

					if (p0 && p0->_weak)
					{
						if (Xt::DomRef* ref = AsRef(p0->_dom))
							Set(ref, 0);
					}

					if (p1 && p1->_weak)
					{
						if (Xt::DomRef* ref = AsRef(p1->_dom))
							Set(ref, 0);
					}
				}

				_ng->MegaUndoHack_Store();
			}
			else if (result == 51)
			{
				for(XtUt::LinkList<NgConnection>::Iterator it(snPin->Connections()); it; ++it)
				{
					SnNgPin* p0 = (SnNgPin*)it->From();
					SnNgPin* p1 = (SnNgPin*)it->To();

					bool isVirtual = it->Virtual() != kNgVirtualNone;

					it->SetVirtual(isVirtual ? kNgVirtualNone : kNgVirtualFrom);
				}
			}
		}
	}

	void ContextMenu_Standard(const WinPoint& pt)
	{
		// Selected name

		XtString selectedName;

		for(XtUt::LinkList<NgNode>::Iterator it(_ng->Diagram()->Selection()); it; ++it)
		{
			SnNgNode* node = (SnNgNode*)(NgNode*)it;

			if (!*selectedName)
				selectedName = WcToMb(node->Label());
			else
			{
				selectedName = "(multiple)";
				break;
			}
		}

		// Types

		XtUt::Array<LtStruct*> types;

		for(XtUt::List<ScarabDomNodeGraph::RootElement>::Iterator itRoot(&_ng->_rootElements); itRoot; ++itRoot)
		{
			switch(itRoot->field->kind)
			{
				case kLfPtr:
				{
					LfPtr* field = ((LfPtr*)itRoot->field);

					// TODO: Check to see if already used?

					AddToTypes(&types, field->baseType);
					AddDerivedTypes(&types, field->baseType);
				}
				break;

				case kLfArray:
				{
					LfArray* field = (LfArray*)itRoot->field;

					if (LtStruct* type = AsStruct(field->Type()))
					{
						AddToTypes(&types, type);

						if (field->field->kind != kLfStrictPtr)
							AddDerivedTypes(&types, type);
					}
				}
				break;
			}
		}

		XtUt::MergeSort(&types, SortTypes);

		// Categories

		XtUt::Array<XtString> categories;

		bool emptyCat = false;

		for(XtUt::Array<LtStruct*>::Iterator itType(&types); itType; ++itType)
		{
			XtString category = GetCategory(*itType);

			if (!*category)
			{
				emptyCat = true;
				continue;
			}

			bool present = false;

			for(XtUt::Array<XtString>::Iterator itCat(&categories); itCat; ++itCat)
			{
				if (itCat == category)
				{
					present = true;
					break;
				}
			}

			if (!present)
				XtUt::AppendEx(&categories, category);
		}

		XtUt::MergeSort(&categories, XtUt::LessThan<XtString>());

		if (emptyCat)
			XtUt::AppendEx(&categories, kXtNullString);

		// Menu

		HMENU menu		= CreatePopupMenu();
		HMENU newMenu	= CreatePopupMenu();

		AppendMenu(menu, MF_STRING | MF_POPUP, (UINT_PTR)newMenu, "New");

		if (*selectedName)
			AppendMenu(menu, MF_STRING, 53, "Duplicate '" + selectedName + "'");
		else
			AppendMenu(menu, MF_STRING | MF_GRAYED | MF_DISABLED, 53, "Duplicate");

		if (*selectedName)
			AppendMenu(menu, MF_STRING, 50, "Delete\tDel");
		else
			AppendMenu(menu, MF_STRING | MF_GRAYED | MF_DISABLED, 50, "Delete\tDel");

		AppendMenu(menu, MF_SEPARATOR, 0, 0);

		if (*selectedName)
			AppendMenu(menu, MF_STRING, 58, "Cut\tCtrl+X");
		else
			AppendMenu(menu, MF_STRING | MF_GRAYED | MF_DISABLED, 58, "Cut\tCtrl+X");

		if (*selectedName)
			AppendMenu(menu, MF_STRING, 56, "Copy\tCtrl+C");
		else
			AppendMenu(menu, MF_STRING | MF_GRAYED | MF_DISABLED, 56, "Copy\tCtrl+C");

		AppendMenu(menu, MF_STRING, 57, "Paste\tCtrl+V");

		AppendMenu(menu, MF_SEPARATOR, 0, 0);

		if (*selectedName)
			AppendMenu(menu, MF_STRING, 54, "Flip");
		else
			AppendMenu(menu, MF_STRING | MF_GRAYED | MF_DISABLED, 54, "Flip");

		if (*selectedName)
			AppendMenu(menu, MF_STRING, 55, "Auto Layout");
		else
			AppendMenu(menu, MF_STRING | MF_GRAYED | MF_DISABLED, 55, "Auto Layout");

		AppendMenu(menu, MF_SEPARATOR, 0, 0);

		if (!XtUt::IsEmpty(&_ng->_undo))
			AppendMenu(menu, MF_STRING, 51, "Undo\tCtrl+Z");
		else
			AppendMenu(menu, MF_STRING | MF_GRAYED | MF_DISABLED, 51, "Undo\tCtrl+Z");

		if (!XtUt::IsEmpty(&_ng->_redo))
			AppendMenu(menu, MF_STRING, 52, "Redo\tCtrl+Shift+Z");
		else
			AppendMenu(menu, MF_STRING | MF_GRAYED | MF_DISABLED, 52, "Redo\tCtrl+Shift+Z");

		for(XtUt::Array<XtString>::Iterator itCat(&categories); itCat; ++itCat)
		{
			HMENU catMenu = newMenu;

			if (categories.count > 1)
			{
				catMenu = CreatePopupMenu();

				XtString label = CleanUpLabel(*itCat);

				if (!*label)
					label = "(misc)";

				AppendMenu(newMenu, MF_STRING | MF_POPUP, (UINT_PTR)catMenu, label);
			}

			for(XtUt::Array<LtStruct*>::Iterator it(&types); it; ++it)
			{
				if (GetCategory(*it) ==  itCat)
					AppendMenu(catMenu, MF_STRING, 1000 + it.index, (*it)->label);
			}
		}

		int result		= TrackPopupMenuEx(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY, pt.x, pt.y, *_ng, 0);
		int typeResult	= result - 1000;

		DestroyMenu(menu);

		if (result == 50)
			Delete();
		else if (result == 51)
			Undo();
		else if (result == 52)
			Redo();
		else if (result == 53)
			Duplicate(_ng->_wndGraph->Diagram()->ToCanvas(pt.ScreenToClient(*_ng->_wndGraph)));
		else if (result == 54)
		{
			for(XtUt::LinkList<NgNode>::Iterator it(_ng->Diagram()->Selection()); it; ++it)
			{
				SnNgNode* node = (SnNgNode*)(NgNode*)it;

				bool def = node->_type->GetBool("NodeGraph", "nodeFlip", false);
				bool mirror = !(node->Mirrored() ^ def);

				if (Xt::DomComponent* com = AsComponent(node->_dom))
					Set(&com->data, "__flipped", mirror ? "1" : "0");

				node->SetMirrored(mirror ^ def);
			}			

			_ng->MegaUndoHack_Store();
		}
		else if (result == 55)
		{
			NgVector centre = _ng->_wndGraph->Diagram()->ToCanvas(pt.ScreenToClient(*_ng->_wndGraph));

			int count = 0;

			float cellWidth		= 0.0f;
			float cellHeight	= 0.0f;

			for(XtUt::LinkList<NgNode>::Iterator it(_ng->Diagram()->Selection()); it; ++it)
			{
				cellWidth = Max(cellWidth, it->Location().sz.vx);
				cellHeight += it->Location().sz.vy;

				count++;
			}

			if (count > 0)
				cellHeight /= (float)count;

			cellWidth += 10.0f;
			cellHeight += 10.0f;

			int sqCount = Max((int)sqrt((float)count), 1);

			int x = 0;
			int y = 0;

			centre.vx -= (cellWidth * sqCount) / 2.0f;
			centre.vy -= (cellHeight * sqCount) / 2.0f;

			float rowY = 0.0f;

			for(XtUt::LinkList<NgNode>::Iterator it(_ng->Diagram()->Selection()); it; ++it)
			{
				it->SetPosition(centre + NgVector(x * cellWidth, 0.0f));

				rowY = Max(rowY, it->Location().sz.vy);

				x++;

				if (x > sqCount)
				{
					centre.vy += rowY + 10.0f;

					x		= 0;
					rowY	= 0.0f;
				}
			}

			_ng->MegaUndoHack_Store();
		}
		else if (result == 58)
			Cut();
		else if (result == 56)
			Copy();
		else if (result == 57)
			Paste(_ng->_wndGraph->Diagram()->ToCanvas(pt.ScreenToClient(*_ng->_wndGraph)));
		else if ((typeResult >= 0) && (typeResult < (int)types.count))
		{
			LtStruct*							want = types[typeResult];
			ScarabDomNodeGraph::RootElement*	best = _ng->FindBestRoot(want);

			if (best)
			{
				if (Xt::DomArray* array = AsArray(best->dom))
				{
					if (Xt::DomComponent* component = new Xt::DomComponent)
					{
						LarvaSchema* schema = _ng->_form->Db()->schema;

						SetType(component, want->name);
						SetDefaults(schema, want, component);

						GenerateIdForComponentAndChildren(schema, want, component, true);

						NgVector canvasPt = _ng->_wndGraph->Diagram()->ToCanvas(pt.ScreenToClient(*_ng->_wndGraph));

						Set(&component->data, "__x", Format("%g", canvasPt.vx));
						Set(&component->data, "__y", Format("%g", canvasPt.vy));

						Xt::Append(array, component);
					}
				}
			}
		}
	}

	virtual void SelectionChanged()
	{
		if (*_ng && !_ng->_resyncSelection)
		{
			_ng->_resyncSelection = true;

			PostMessage(*_ng, WM_APP + 1, 0, 0);
		}
	}

	virtual void CheckPoint()
	{
		_ng->MegaUndoHack_CheckPoint();
	}

	virtual void Undo()
	{
		_ng->MegaUndoHack_Undo();
	}

	virtual void Redo()
	{
		_ng->MegaUndoHack_Redo();
	}

	virtual void Delete()
	{
		if (*_ng && !_ng->_resyncSelection)
		{
			_ng->_resyncSelection = true;

			PostMessage(*_ng, WM_APP + 1, 0, 0);
		}

		XtUt::Array<XtUt::WeakRef<NgNode>> refs;

		for(XtUt::LinkList<NgNode>::Iterator it(_ng->Diagram()->Selection()); it; ++it)
			XtUt::AppendEx(&refs, XtUt::WeakRef<NgNode>(it->Anchor()));

		for(XtUt::Array<XtUt::WeakRef<NgNode>>::Iterator it(&refs); it; ++it)
		{
			SnNgNode* node = (SnNgNode*)it->Get();

			if (!node)
				continue;

			if (node->_indexer && node->_parentDom)
				node->_indexer->Remove(node->_parentDom);
		}
	}

	virtual void Cut()
	{
		Copy();
		Delete();
	}

	virtual void Copy()
	{
		Xt::Bundle bundle;

		for(XtUt::LinkList<NgNode>::Iterator it(_ng->Diagram()->Selection()); it; ++it)
		{
			Xt::DomComponent* com = AsComponent(((SnNgNode*)&*it)->_dom);

			if (!com)
				continue;

			ScarabDomNodeGraph::RootElement* best = _ng->FindBestRoot(((SnNgNode*)&*it)->_type);

			if (!best)
				continue;

			Xt::Append(&bundle, best->field, com);
		}

		XtString data;

		if (Xt::Serialise(&bundle, data))
			SetClipboardText(*_ng, data);
	}

	virtual void Paste(const NgVector& targetPt)
	{
		_ng->MegaUndoHack_Store();

		XtString data;
		Xt::Bundle bundle;

		if (!GetClipboardText(*_ng, data))
			return;

		if (!Xt::Parse(&bundle, &XtUt::WeakBlock((XtU8*)data.Text(), (size_t)data.Length()), 0))
			return;

		// Extract components

		LarvaSchema* schema = _ng->_form->Db()->schema;

		Xt::IdMap map;
		XtUt::List<DupItem> newComs;

		for(Xt::Bundle::Iterator it(&bundle); it; ++it)
		{
			LtStruct*			type	= AsStruct(FindTypeByName(schema, it->type));
			LarvaField*			field	= FindFieldByName(type, it->field);
			Xt::DomComponent*	srcCom	= AsComponent(it->element);
			LtStruct*			comType	= AsStruct(TypeOf(schema, field, srcCom));

			if (!type || !field || !srcCom || !comType)
				continue;

			if (ScarabDomNodeGraph::RootElement* best = _ng->FindBestRoot(comType))
			{
				if (Xt::DomArray* array = AsArray(best->dom))
				{
					if (Xt::DomComponent* dstCom = (Xt::DomComponent*)::Duplicate(schema, comType, srcCom))
					{
						if (srcCom->id && dstCom->id)
							XtUt::Insert(&map, srcCom->id, new XtId(dstCom->id));

						Xt::Append(array, dstCom);

						XtUt::AppendEx(&newComs, new DupItem(comType, dstCom));
					}
				}
			}
		}

		// Map connections

		Xt::DomDb* db = _ng->_form->Db();

		for(XtUt::List<DupItem>::Iterator itCom(&newComs); itCom; ++itCom)
		{
			Xt::DomComponent* com = AsComponent(itCom->dom);

			if (com)
				MapWeakPtrs(schema, Xt::TypeOf(schema, itCom->type, com), com, &map);
		}

		// Select new items

		_ng->SyncControls();

		_ng->Diagram()->ClearSelection();

		for(XtUt::List<DupItem>::Iterator itCom(&newComs); itCom; ++itCom)
		{
			if (Xt::DomComponent* com = AsComponent(itCom->dom))
			{
				if (SnNgNode* node = _ng->FindNode(com->id))
				{
					node->Select();
				}
			}
		}

		// Find centre point of duplication

		NgVector min;
		NgVector max;

		int count = 0;

		for(XtUt::LinkList<NgNode>::Iterator itNode(_ng->Diagram()->Selection()); itNode; ++itNode)
		{
			const NgRect& l = itNode->Location();

			if (count > 0)
			{
				min.vx = Min(min.vx, l.TopLeft().vx);
				min.vy = Min(min.vy, l.TopLeft().vy);

				max.vx = Max(max.vx, l.BottomRight().vx);
				max.vy = Max(max.vy, l.BottomRight().vy);
			}
			else
			{
				min = l.TopLeft();
				max = l.BottomRight();
			}

			count++;
		}

		// Move to new position

		NgVector delta = targetPt - ((min + max) * 0.5f);

		for(XtUt::List<DupItem>::Iterator itCom(&newComs); itCom; ++itCom)
		{
			if (Xt::DomComponent* com = AsComponent(itCom->dom))
			{
				Set(&com->data, "__x", Format("%g", atof(AsText(At(&com->data, "__x"))) + delta.vx));
				Set(&com->data, "__y", Format("%g", atof(AsText(At(&com->data, "__y"))) + delta.vy));
			}
		}
	}

	struct DupItem
	{
		LtStruct*		type;
		Xt::DomMonitor	dom;

	public:
		DupItem(LtStruct* type_, Xt::DomComponent* dom_) :
			type(type_)
		{
			Monitor(&dom, dom_);
		}
	};

	virtual void Duplicate(const NgVector& targetPt)
	{
		LarvaSchema* schema = _ng->_form->Db()->schema;

		if (*_ng && !_ng->_resyncSelection)
		{
			_ng->_resyncSelection = true;

			PostMessage(*_ng, WM_APP + 1, 0, 0);
		}

		// Make ref list

		XtUt::Array<XtUt::WeakRef<NgNode>> refs;

		for(XtUt::LinkList<NgNode>::Iterator it(_ng->Diagram()->Selection()); it; ++it)
			XtUt::AppendEx(&refs, XtUt::WeakRef<NgNode>(it->Anchor()));

		// Find centre point of duplication

		NgVector min;
		NgVector max;

		int count = 0;

		for(XtUt::Array<XtUt::WeakRef<NgNode>>::Iterator itNode(&refs); itNode; ++itNode)
		{
			const NgRect& l = (*itNode)->Location();

			if (count > 0)
			{
				min.vx = Min(min.vx, l.TopLeft().vx);
				min.vy = Min(min.vy, l.TopLeft().vy);

				max.vx = Max(max.vx, l.BottomRight().vx);
				max.vy = Max(max.vy, l.BottomRight().vy);
			}
			else
			{
				min = l.TopLeft();
				max = l.BottomRight();
			}

			count++;
		}

		NgVector delta = targetPt - ((min + max) * 0.5f);

		// Duplicate

		Xt::IdMap map;
		XtUt::List<DupItem> newComs;

		for(XtUt::Array<XtUt::WeakRef<NgNode>>::Iterator it(&refs); it; ++it)
		{
			SnNgNode* node = (SnNgNode*)it->Get();

			if (!node)
				continue;

			if (ScarabDomNodeGraph::RootElement* best = _ng->FindBestRoot(node->_type))
			{
				if (Xt::DomArray* array = AsArray(best->dom))
				{
					if (Xt::DomComponent* srcCom = AsComponent(node->_dom))
					{
						if (Xt::DomComponent* dstCom = (Xt::DomComponent*)::Duplicate(schema, node->_type, srcCom))
						{
							if (srcCom->id && dstCom->id)
								XtUt::Insert(&map, srcCom->id, new XtId(dstCom->id));

							Set(&dstCom->data, "__x", Format("%g", atof(AsText(At(&dstCom->data, "__x"))) + delta.vx));
							Set(&dstCom->data, "__y", Format("%g", atof(AsText(At(&dstCom->data, "__y"))) + delta.vy));

							Xt::Append(array, dstCom);

							XtUt::AppendEx(&newComs, new DupItem(node->_type, dstCom));
						}
					}
				}
			}
		}

		// Map connections

		for(XtUt::List<DupItem>::Iterator itCom(&newComs); itCom; ++itCom)
		{
			Xt::DomComponent* com = AsComponent(itCom->dom);

			if (com)
				MapWeakPtrs(schema, Xt::TypeOf(schema, itCom->type, com), com, &map);
		}

		// Select new items

		_ng->SyncControls();

		_ng->Diagram()->ClearSelection();

		for(XtUt::List<DupItem>::Iterator itCom(&newComs); itCom; ++itCom)
		{
			if (Xt::DomComponent* com = AsComponent(itCom->dom))
			{
				if (SnNgNode* node = _ng->FindNode(com->id))
				{
					node->Select();
				}
			}
		}
	}

private:
	ScarabDomNodeGraph* _ng;
};

//
// SfcNull
//

class SfcNull : public ScarabFormContext
{
public:
	SfcNull(ScarabFormContext* sfc) :
		_sfc(sfc)
	{
	}

public:
	virtual void OpenRecord(Xt::DomRecord* record)
	{
		_sfc->OpenRecord(record);
	}

private:
	ScarabFormContext* _sfc;
};

//
// ScarabDomNodeGraph
//

ScarabDomNodeGraph::ScarabDomNodeGraph() :
	_domType(0),
	_form(0),
	_splitter(0),
	_firstLayout(true),
	_resyncSelection(false),
	_resyncControls(false),
	_megaUndoHack(false),
	_wndGraph(0),
	_wndForm(0),
	_snapshot(0)
{
	_domRoot.SetCallback(DomCallback, this);
}

ScarabDomNodeGraph::~ScarabDomNodeGraph()
{
	delete _wndGraph;
	delete _wndForm;
	delete _snapshot;
}

bool ScarabDomNodeGraph::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!ChildWindow::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_CLIPCHILDREN;
	cs->dwExStyle	|= WS_EX_CONTROLPARENT;

	return true;
}

LRESULT ScarabDomNodeGraph::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
			if (ChildWindow::WndProc(msg, wParam, lParam) != 0)
				return -1;

			if ((_wndForm = new ScarabForm(new SfcNull(_form->FormContext()))) == 0)
				return -1;

			if ((_wndGraph = new NodeGraph(new ScarabDomNodeGraphContext(this))) == 0)
				return -1;

			if (!_wndForm->Create(*this))
				return -1;

			if (!_wndGraph->Create(*this))
				return -1;

			_host.SetWindow(*this);

			_splitter = (DwSplit*)_host.Root()->SetChild(0, new DwSplit(&_host));
			_splitter->Create();
			_splitter->SetVertical(false);

			DwPane* pa0 = (DwPane*)_splitter->SetChild(0, new DwPane(&_host));
			DwPane* pa1 = (DwPane*)_splitter->SetChild(1, new DwPane(&_host));

			pa0->Add(new DwTab(&_host, *_wndGraph));
			pa1->Add(new DwTab(&_host, *_wndForm));
		}
		return 0;

		case WM_DESTROY:
			_host.SetWindow(0);

			delete _wndGraph;
			delete _wndForm;
		break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(_hwnd, &ps);
			_host.Paint(ps.hdc, ps.rcPaint);
			EndPaint(_hwnd, &ps);
		}
		return 0;

		case WM_SIZE:
		{
			WinRect rcClient(*this);

			_host.SetLocation(rcClient);
			_host.PerformLayout(true);
		}
		return 0;

		case WM_APP:
			SyncControls();
		return 0;

		case (WM_APP + 1):
			SyncSelection();

			MegaUndoHack_Store();
		return 0;

		case (WM_APP + 2):
			MegaUndoHack_Store();
		return 0;
	}

	return ChildWindow::WndProc(msg, wParam, lParam);
}

void ScarabDomNodeGraph::SetForm(ScarabForm* form)
{
	_form = form;
}

void ScarabDomNodeGraph::SetDom(LtStruct* rootType, Xt::DomBlock* rootDom)
{
	Diagram()->Clear();

	if (rootType && rootDom)
	{
		Monitor(&_domRoot, rootDom);

		_domType = rootType;
	}
	else
	{
		Monitor(&_domRoot, 0);
		_domType = 0;
	}

	SyncControls();
	SyncSelection();

	MegaUndoHack_Clear();

	Diagram()->Frame();
}

void ScarabDomNodeGraph::SetWatermark(XtString watermark)
{
	if (Diagram())
		Diagram()->SetWatermark(MbToWc(watermark));
}

void ScarabDomNodeGraph::Layout()
{
	if (_firstLayout && _splitter)
	{
		WinRect rc(*this);

		if (rc.Height() > 0)
		{
			_splitter->SetPosition((rc.Height() * 3) / 4);

			_host.PerformLayout(true);

			_firstLayout = false;
		}
	}
}

void ScarabDomNodeGraph::SyncControls()
{
	// TODO: Need to keep track of what's connected to what...  So I don't need to clear the entire thing each time, just the deleted bits, etc...

	_resyncControls = false;

	LarvaSchema* schema = _form->Db()->schema;

	if (_wndGraph)
	{
		if (Xt::DomBlock* dom = AsBlock(_domRoot))
		{
			if (_domType)
			{
				PopulateRoots(_domType, dom);
				PopulateNodes();
				PopulatePins();
			}
			else
				Diagram()->Clear();
		}
		else
			Diagram()->Clear();

		Diagram()->SetDirty(true);
	}
}

void ScarabDomNodeGraph::SyncSelection()
{
	_resyncSelection = false;

	if (_wndForm)
	{
		SnNgNode* selected = 0;

		for(XtUt::LinkList<NgNode>::Iterator it(Diagram()->Selection()); it; ++it)
		{
			SnNgNode* node = (SnNgNode*)(NgNode*)it;

			if (!selected)
				selected = node;
			else
			{
				selected = 0;
				break;
			}
		}

		if (selected)
		{
			if (Xt::DomComponent* component = AsComponent(selected->_dom))
			{
				LarvaSchema* schema = _form->Db()->schema;

				_wndForm->SetDom(component, selected->_field);
			}
			else
				_wndForm->SetDom(0, (LarvaType*)0);
		}
		else
			_wndForm->SetDom(0, (LarvaType*)0);
	}
}

NgDiagram* ScarabDomNodeGraph::Diagram()
{
	return _wndGraph ? _wndGraph->Diagram() : 0;
}

SnNgNode* ScarabDomNodeGraph::FindNode(XtId id)
{
	if (!id)
		return 0;

	for(XtUt::LinkList<SnNgNode>::Iterator itNode(&_nodes); itNode; ++itNode)
	{
		if (itNode->_id == id)
			return itNode;
	}

	return 0;
}

ScarabDomNodeGraph::RootElement* ScarabDomNodeGraph::FindBestRoot(LtStruct* want)
{
	RootElement*	bestRoot	= 0;
	LtStruct*		bestType	= 0;

	for(XtUt::List<ScarabDomNodeGraph::RootElement>::Iterator itRoot(&_rootElements); itRoot; ++itRoot)
	{
		switch(itRoot->field->kind)
		{
			case kLfPtr:
			{
				LfPtr* field = ((LfPtr*)itRoot->field);

				if (IsBaseOf(field->baseType, want))
				{
					if (!IsBaseOf(field->baseType, bestType))
					{
						bestRoot	= itRoot;
						bestType	= field->baseType;
					}
				}
			}
			break;

			case kLfArray:
			{
				LfArray* field = (LfArray*)itRoot->field;

				if (LtStruct* type = AsStruct(field->Type()))
				{
					if (IsBaseOf(type, want))
					{
						if (field->field->kind == kLfStrictPtr)
							return itRoot;

						if (!IsBaseOf(type, bestType))
						{
							bestRoot	= itRoot;
							bestType	= type;
						}
					}
				}
			}
			break;
		}
	}

	return bestRoot;
}

ScarabDomNodeGraph::RootElement* ScarabDomNodeGraph::FindPossiblePlug(LtStruct* want)
{
	RootElement*	bestRoot	= 0;
	LtStruct*		bestType	= 0;

	for(XtUt::List<ScarabDomNodeGraph::RootElement>::Iterator itRoot(&_rootElements); itRoot; ++itRoot)
	{
		switch(itRoot->field->kind)
		{
			case kLfPtr:
			{
				LfPtr* field = ((LfPtr*)itRoot->field);

				if (IsBaseOf(want, field->baseType) || IsBaseOf(field->baseType, want))
				{
					if (!IsBaseOf(bestType, field->baseType))
					{
						bestRoot	= itRoot;
						bestType	= field->baseType;
					}
				}
			}
			break;

			case kLfArray:
			{
				LfArray* field = (LfArray*)itRoot->field;

				if (LtStruct* type = AsStruct(field->Type()))
				{
					if (IsBaseOf(want, type) || IsBaseOf(type, want))
					{
						if (field->field->kind == kLfStrictPtr)
							return itRoot;

						if (!IsBaseOf(bestType, type))
						{
							bestRoot	= itRoot;
							bestType	= type;
						}
					}
				}
			}
			break;
		}
	}

	return bestRoot;
}

void ScarabDomNodeGraph::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, ScarabDomNodeGraph* ng)
{
	bool refresh = true;

	if (event->code == Xt::kDomFieldChanged)
	{
		if (((Xt::DomFieldChanged*)event)->from->parentKey == "__x")
			refresh = false;
		else if (((Xt::DomFieldChanged*)event)->from->parentKey == "__y")
			refresh = false;
	}

	if (refresh)
	{
		if (*ng && !ng->_resyncControls)
		{
			ng->_resyncControls = true;

			PostMessage(*ng, WM_APP, 0, 0);
		}
	}
}

void ScarabDomNodeGraph::MegaUndoHack_CheckPoint()
{
	if (*this && !_megaUndoHack)
	{
		_megaUndoHack = true;

		PostMessage(*this, WM_APP + 2, 0, 0);
	}
}

void ScarabDomNodeGraph::MegaUndoHack_Clear()
{
	XtUt::Free(&_undo);
	XtUt::Free(&_redo);

	if (_snapshot)
		delete _snapshot;

	_snapshot = Clone(AsBlock(_domRoot));

	_megaUndoHack = false;
}

void ScarabDomNodeGraph::MegaUndoHack_Store()
{
	_megaUndoHack = false;

	Xt::DomBlock* block = AsBlock(_domRoot);

	if (!block || !_snapshot)
		return;

	XtUt::Ptr<UndoHack> uh = new UndoHack;

	if (!uh)
		return;

	uh->undo = Xt::Diff(block, _snapshot);
	uh->redo = Xt::Diff(_snapshot, block);

	if (!uh->undo)
		return;

	if (!Xt::Patch(_snapshot, uh->redo))
		DebugLn("MegaUndoHack_Store: Patch failed!");

	//void Dump(Xt::Change* change, const char* name);
	//Dump(uh->undo, "STORE");

	XtUt::Free(&_redo);

	XtUt::PushBack(&_undo, &XtUt::Pop(&uh)->link);
}

void ScarabDomNodeGraph::MegaUndoHack_Undo()
{
	_megaUndoHack = false;

	Xt::DomBlock* block = AsBlock(_domRoot);

	if (!block || !_snapshot)
		return;

	XtUt::Ptr<Xt::Change> revert = Xt::Diff(block, _snapshot);

	if (revert)
	{
		XtUt::Ptr<UndoHack> uh = new UndoHack;

		uh->undo = XtUt::Pop(&revert);
		uh->redo = Xt::Diff(_snapshot, block);

		if (!Xt::Patch(block, uh->undo))
			DebugLn("MegaUndoHack_Undo: Revert failed!");

		XtUt::PushBack(&_redo, &XtUt::Pop(&uh)->link);
	}
	else
	{
		XtUt::Ptr<UndoHack> uh = XtUt::PopBack(&_undo);

		if (!uh)
			return;

		if (!Xt::Patch(block, uh->undo))
			DebugLn("MegaUndoHack_Undo: Undo failed!");

		if (!Xt::Patch(_snapshot, uh->undo))
			DebugLn("MegaUndoHack_Undo: Undo snapshot failed!");

		XtUt::PushBack(&_redo, &XtUt::Pop(&uh)->link);
	}

	SyncControls();
	SyncSelection();
}

void ScarabDomNodeGraph::MegaUndoHack_Redo()
{
	_megaUndoHack = false;

	Xt::DomBlock* block = AsBlock(_domRoot);

	if (!block || !_snapshot)
		return;

	XtUt::Ptr<Xt::Change> revert = Xt::Diff(block, _snapshot);

	if (revert)
	{
		XtUt::Free(&_redo);
		return;
	}

	XtUt::Ptr<UndoHack> uh = XtUt::PopBack(&_redo);

	if (!uh)
		return;

	if (!Xt::Patch(block, uh->redo))
		DebugLn("MegaUndoHack_Undo: Redo failed!");

	if (!Xt::Patch(_snapshot, uh->redo))
		DebugLn("MegaUndoHack_Undo: Redo snapshot failed!");

	XtUt::PushBack(&_undo, &XtUt::Pop(&uh)->link);

	SyncControls();
	SyncSelection();
}
