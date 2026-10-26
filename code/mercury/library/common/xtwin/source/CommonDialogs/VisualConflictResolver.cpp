#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/WndCore.h"
#include "Xt/XtTool.h"
#include "XtWin/VisualConflictResolver.h"

class VisualConflictResolverWnd : public DlgWindow
{
public:
	VisualConflictResolverWnd(LarvaSchema* schema,
			Xt::Change* chgMerged, Xt::Change* chgLhs, Xt::Change* chgRhs,
			Xt::DomElement* ancestor, Xt::DomElement* lhs, Xt::DomElement* rhs
	) :
		_schema(schema),
		_base(chgMerged, ancestor, 0, 0),
		_lhs(chgLhs, lhs, 0, 0),
		_rhs(chgRhs, rhs, 0, 0),
		_tree(0)
	{
	}

private:
	struct Desc
	{
		Xt::Change*		change;
		Xt::DomElement*	dom;
		LarvaType*		type;
		LarvaField*		field;

	public:
		Desc(Xt::Change* change_ = 0, Xt::DomElement* dom_ = 0, LarvaType* type_ = 0, LarvaField* field_ = 0) :
			change(change_),
			dom(dom_),
			type(type_),
			field(field_)
		{
			if (!type && field)
				type = field->Type();
		}
	};

private:
	struct Item
	{
		Desc					base;
		Desc					lhs;
		Desc					rhs;

		Item*					parent;
		XtUt::Link<Item>		link;
		XtUt::LinkList<Item>	children;

		XtU32					colour;
		bool					conflicted;
		bool					imbalance;

	public:
		Item() :
			parent(0),
			link(this),
			colour(0),
			conflicted(false),
			imbalance(false)
		{
		}
	};

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs)
	{
		return Window::PreCreateWindow(cs);
	}

	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch(msg)
		{
			case WM_CREATE:
			{
				if (Window::WndProc(msg, wParam, lParam) < 0)
					return -1;

				_tree = CreateWindowEx(0, "XTWINTREEVIEW", "",
						WS_VISIBLE | WS_CHILD | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS,
					0, 0, 0, 0, *this, 0, 0, 0);

				Populate(TVI_ROOT, &_root, _base, _lhs, _rhs, "Root");
			}
			return 0;

			case WM_CLOSE:
				EndModal(0);
			return 0;

			case WM_SIZE:
			{
				WinRect rc(*this);

				SetWindowPos(_tree, 0, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
			}
			return 0;

			case WM_COMMAND:
				switch(LOWORD(wParam))
				{
					case IDCANCEL:
						if (HIWORD(wParam) == BN_CLICKED)
						{
							EndModal(0);
							return TRUE;
						}
					break;
				}
			break;

			case WM_NOTIFY:
			{
				NMHDR* hdr = (NMHDR*)lParam;

				if (hdr->hwndFrom == _tree)
				{
					switch(hdr->code)
					{
						case NM_CUSTOMDRAW:
						{
							NMTVCUSTOMDRAW* cd = (NMTVCUSTOMDRAW*)hdr;

							Item* item = (Item*)cd->nmcd.lItemlParam;

							if (item)
							{
								if (cd->nmcd.uItemState & CDIS_SELECTED)
									cd->clrTextBk = item->colour;
								else
									cd->clrText = item->colour;
							}
						}
						return CDRF_DODEFAULT | CDRF_NOTIFYITEMDRAW;
					}
				}
			}
			break;
		}

		return Window::WndProc(msg, wParam, lParam);
	}

private:
	void Populate(HTREEITEM treeParent, Item* itemParent, const Desc& base, const Desc& lhs, const Desc& rhs, const Sz8& indexName)
	{
		if (!base.change)
			return;

		// Create item

		Item* item = new Item;

		if (!item)
			return;

		item->base		= base;
		item->lhs		= lhs;
		item->rhs		= rhs;
		item->parent	= itemParent;

		XtUt::PushBack(&itemParent->children, &item->link);

		// Generate node label

		Sz8 changeName = "(null)";

		switch(base.change->type)
		{
			case Xt::kChangeNone:		changeName = "None";		break;

			case Xt::kChangeRemove:
			{
				Xt::DomElement*	dom		= base.dom;
				LarvaField*		field	= base.field;

				if (!dom)
				{
					dom		= lhs.dom;
					field	= lhs.field;
				}

				if (!dom)
				{
					dom		= rhs.dom;
					field	= rhs.field;
				}

				if (dom && field)
					changeName = "Remove (" + EvaluateSummaryText(0, _schema, field, dom) + ")";
				else if (Xt::DomRecord* record = AsRecord(dom))
					changeName = "Remove (" + record->name + "." + record->root.type + ")";
				else
					changeName = "Remove";
			}
			break;

			case Xt::kChangeLeaf:
			{
				Xt::ChangeLeaf* chg = (Xt::ChangeLeaf*)base.change;

				if (chg->element)
				{
					if (chg->element->kind == Xt::kDomField)
						changeName = Format("Leaf Field(%s)", AsText(chg->element).Text());
					else if (chg->element->kind == Xt::kDomRef)
						changeName = Format("Leaf Ref(#%08X)", AsId(chg->element));
					else if (chg->element->kind == Xt::kDomBlock)
						changeName = "Leaf Block";
					else if (chg->element->kind == Xt::kDomArray)
						changeName = "Leaf Array";
					else if (chg->element->kind == Xt::kDomComponent)
						changeName = "Leaf Array";
					else
						changeName = "Leaf ?";
				}
				else
					changeName = "Leaf";
			}
			break;

			case Xt::kChangeBlock:
			{
				Xt::DomBlock*	dom		= AsBlock(base.dom);
				LtStruct*		type	= AsStruct(base.type);

				if (!dom)
				{
					dom		= AsBlock(lhs.dom);
					type	= AsStruct(lhs.type);
				}

				if (!dom)
				{
					dom		= AsBlock(rhs.dom);
					type	= AsStruct(rhs.type);
				}

				if (dom && type)
					changeName = "Block (" + EvaluateSummaryText(0, _schema, type, dom, "{__type}") + ")";
				else if (type)
					changeName = "Block (" + type->name + ")";
				else
					changeName = "Block";
			}
			break;

			case Xt::kChangeArray:
			{
				Xt::DomArray*	dom		= AsArray(base.dom);
				LtStruct*		type	= AsStruct(base.type);

				if (!dom)
				{
					dom		= AsArray(lhs.dom);
					type	= AsStruct(lhs.type);
				}

				if (!dom)
				{
					dom		= AsArray(rhs.dom);
					type	= AsStruct(rhs.type);
				}

				if (type)
					changeName = "Array (" + type->name + ")";
				else
					changeName = "Array";
			}
			break;

			case Xt::kChangeComponent:
			{
				Xt::DomComponent*	dom		= AsComponent(base.dom);
				LtStruct*			type	= AsStruct(base.type);

				if (!dom)
				{
					dom		= AsComponent(lhs.dom);
					type	= AsStruct(lhs.type);
				}

				if (!dom)
				{
					dom		= AsComponent(rhs.dom);
					type	= AsStruct(rhs.type);
				}

				if (dom && type)
					changeName = "Component (" + EvaluateSummaryText(0, _schema, type, &dom->data, "{__type}") + ")";
				else if (type)
					changeName = "Component (" + type->name + ")";
				else
					changeName = "Component";
			}
			break;

			case Xt::kChangeRecord:
				if (Xt::DomRecord* record = AsRecord(base.dom))
					changeName = Format("Record (%s)", record->name.Text());
				else if (Xt::DomRecord* record = AsRecord(base.dom))
					changeName = Format("Record (%s)", record->name.Text());
				else if (Xt::DomRecord* record = AsRecord(base.dom))
					changeName = Format("Record (%s)", record->name.Text());
				else
					changeName = "Record";
			break;

			case Xt::kChangePack:		changeName = "Pack";		break;

			case Xt::kChangeId:
			{
				Xt::ChangeId* chg = (Xt::ChangeId*)base.change;

				changeName = Format("Id (#%08X)", chg->value);
			}
			break;

			case Xt::kChangeString:
			{
				Xt::ChangeString* chg = (Xt::ChangeString*)base.change;

				changeName = Format("String (%s)", chg->value.Text());
			}
			break;

			case Xt::kChangeIndex:		changeName = "Index";		break;
			case Xt::kChangeConflict:	changeName = "Conflict";	break;
		}

		// Create tree node

		TVINSERTSTRUCT is = { 0 };

		is.hParent			= treeParent;
		is.hInsertAfter		= TVI_LAST;

		is.itemex.mask		= TVIF_TEXT | TVIF_PARAM;
		is.itemex.pszText	= (LPSTR)(*changeName ? ((indexName + ": " + changeName).Text()) : indexName.Text());
		is.itemex.lParam	= (LPARAM)item;;

		HTREEITEM node = TreeView_InsertItem(_tree, &is);

		// Create child nodes

		switch(base.change->type)
		{
			case Xt::kChangeNone:
			case Xt::kChangeRemove:
			case Xt::kChangeLeaf:
			break;

			case Xt::kChangeBlock:
			{
				Xt::ChangeBlock* cb = (Xt::ChangeBlock*)base.change;
				Xt::ChangeBlock* cl = (Xt::ChangeBlock*)((lhs.change && (lhs.change->type == Xt::kChangeBlock)) ? lhs.change : 0);
				Xt::ChangeBlock* cr = (Xt::ChangeBlock*)((rhs.change && (rhs.change->type == Xt::kChangeBlock)) ? rhs.change : 0);

				for(XtUt::Dictionary<XtString, Xt::Change>::Iterator it(&cb->changes); it; ++it)
				{
					LarvaField* fb = FindFieldByName(AsStruct(base.type), it.Key());
					LarvaField* fl = FindFieldByName(AsStruct(lhs.type), it.Key());
					LarvaField* fr = FindFieldByName(AsStruct(rhs.type), it.Key());

					Xt::DomElement* eb = At(AsBlock(base.dom), it.Key());
					Xt::DomElement* el = At(AsBlock(lhs.dom), it.Key());
					Xt::DomElement* er = At(AsBlock(rhs.dom), it.Key());

					Populate(node, item,
							Desc(it, eb, TypeOf(_schema, fb, eb), fb),
							Desc(cl ? cl->changes[it.Key()] : 0, el, TypeOf(_schema, fl, el), fl),
							Desc(cr ? cr->changes[it.Key()] : 0, er, TypeOf(_schema, fr, er), fr),
						it.Key());
				}
			}
			break;

			case Xt::kChangeArray:
			{
				Xt::ChangeArray* cb = (Xt::ChangeArray*)base.change;
				Xt::ChangeArray* cl = (Xt::ChangeArray*)((lhs.change && (lhs.change->type == Xt::kChangeArray)) ? lhs.change : 0);
				Xt::ChangeArray* cr = (Xt::ChangeArray*)((rhs.change && (rhs.change->type == Xt::kChangeArray)) ? rhs.change : 0);

				Populate(node, item,
						Desc(cb->index, base.dom, 0, 0),
						Desc(cl ? cl->index : 0, lhs.dom, 0, 0),
						Desc(cr ? cr->index : 0, rhs.dom, 0, 0),
					"*index*");

				for(XtUt::Dictionary<XtId, Xt::Change>::Iterator it(&cb->changes); it; ++it)
				{
					LarvaField* fb = (base.field && (base.field->kind == kLfArray)) ? ((LfArray*)base.field)->field : 0;
					LarvaField* fl = (lhs.field && (lhs.field->kind == kLfArray)) ? ((LfArray*)lhs.field)->field : 0;
					LarvaField* fr = (rhs.field && (rhs.field->kind == kLfArray)) ? ((LfArray*)rhs.field)->field : 0;

					Xt::DomElement* eb = FindById(AsArray(base.dom), it.Key());
					Xt::DomElement* el = FindById(AsArray(lhs.dom), it.Key());
					Xt::DomElement* er = FindById(AsArray(rhs.dom), it.Key());

					Populate(node, item,
							Desc(it, eb, TypeOf(_schema, fb, eb), fb),
							Desc(cl ? cl->changes[it.Key()] : 0, el, TypeOf(_schema, fl, el), fl),
							Desc(cr ? cr->changes[it.Key()] : 0, er, TypeOf(_schema, fr, er), fr),
						Format("#%08X", it.Key()));
				}
			}
			break;

			case Xt::kChangeComponent:
			{
				Xt::ChangeComponent* cb = (Xt::ChangeComponent*)base.change;
				Xt::ChangeComponent* cl = (Xt::ChangeComponent*)((lhs.change && (lhs.change->type == Xt::kChangeComponent)) ? lhs.change : 0);
				Xt::ChangeComponent* cr = (Xt::ChangeComponent*)((rhs.change && (rhs.change->type == Xt::kChangeComponent)) ? rhs.change : 0);

				Populate(node, item,
						Desc(cb->id, base.dom, 0, 0),
						Desc(cl ? cl->id : 0, lhs.dom, 0, 0),
						Desc(cr ? cr->id : 0, rhs.dom, 0, 0),
					"*id*");

				Populate(node, item,
						Desc(cb->type, base.dom, 0, 0),
						Desc(cl ? cl->type : 0, lhs.dom, 0, 0),
						Desc(cr ? cr->type : 0, rhs.dom, 0, 0),
					"*type*");

				Populate(node, item,
						Desc(cb->data, AsComponentData(base.dom), base.type, base.field),
						Desc(cl ? cl->data : 0, AsComponentData(lhs.dom), lhs.type, lhs.field),
						Desc(cr ? cr->data : 0, AsComponentData(rhs.dom), rhs.type, rhs.field),
					"*data*");
			}
			break;

			case Xt::kChangeRecord:
			{
				Xt::ChangeRecord* cb = (Xt::ChangeRecord*)base.change;
				Xt::ChangeRecord* cl = (Xt::ChangeRecord*)((lhs.change && (lhs.change->type == Xt::kChangeRecord)) ? lhs.change : 0);
				Xt::ChangeRecord* cr = (Xt::ChangeRecord*)((rhs.change && (rhs.change->type == Xt::kChangeRecord)) ? rhs.change : 0);

				Populate(node, item,
						Desc(cb->name, base.dom, 0, 0),
						Desc(cl ? cl->name : 0, lhs.dom, 0, 0),
						Desc(cr ? cr->name : 0, rhs.dom, 0, 0),
					"*name*");

				Populate(node, item,
						Desc(cb->root, AsRecordRoot(base.dom), TypeOf(_schema, AsRecord(base.dom)), 0),
						Desc(cl ? cl->root : 0, AsRecordRoot(lhs.dom), TypeOf(_schema, AsRecord(lhs.dom)), 0),
						Desc(cr ? cr->root : 0, AsRecordRoot(rhs.dom), TypeOf(_schema, AsRecord(rhs.dom)), 0),
					"*root*");
			}
			break;

			case Xt::kChangePack:
			{
				Xt::ChangePack* cb = (Xt::ChangePack*)base.change;
				Xt::ChangePack* cl = (Xt::ChangePack*)((lhs.change && (lhs.change->type == Xt::kChangePack)) ? lhs.change : 0);
				Xt::ChangePack* cr = (Xt::ChangePack*)((rhs.change && (rhs.change->type == Xt::kChangePack)) ? rhs.change : 0);

				Populate(node, item,
						Desc(cb->name, base.dom, 0, 0),
						Desc(cl ? cl->name : 0, lhs.dom, 0, 0),
						Desc(cr ? cr->name : 0, rhs.dom, 0, 0),
					"*name*");

				Populate(node, item,
						Desc(cb->properties, AsPackProperties(base.dom), 0, 0),
						Desc(cl ? cl->properties : 0, AsPackProperties(lhs.dom), 0, 0),
						Desc(cr ? cr->properties : 0, AsPackProperties(rhs.dom), 0, 0),
					"*properties*");

				for(XtUt::Dictionary<XtId, Xt::Change>::Iterator it(&cb->changes); it; ++it)
				{
					Populate(node, item,
							Desc(it, At(AsPack(base.dom), it.Key()), 0, 0),
							Desc(cl ? cl->changes[it.Key()] : 0, At(AsPack(lhs.dom), it.Key()), 0, 0),
							Desc(cr ? cr->changes[it.Key()] : 0, At(AsPack(rhs.dom), it.Key()), 0, 0),
						Format("#%08X", it.Key()));
				}
			}
			break;

			case Xt::kChangeId:
			case Xt::kChangeString:
			case Xt::kChangeIndex:
			break;

			case Xt::kChangeConflict:
			{
				Xt::ChangeConflict* cb = (Xt::ChangeConflict*)base.change;

				Populate(node, item,
						Desc(cb->lhs, base.dom, base.type, base.field),
						lhs,
						Desc(),
					"*lhs*");

				Populate(node, item,
						Desc(cb->rhs, base.dom, base.type, base.field),
						Desc(),
						rhs,
					"*rhs*");

				item->conflicted = true;
			}
			break;
		}

		if (item->conflicted)
		{
			if (itemParent)
				itemParent->conflicted = true;

			if (item->base.change->type == Xt::kChangeConflict)
				item->colour = RGB(255, 0, 0);
			else
				item->colour = RGB(192, 0, 0);
		}
		else
		{
			if (lhs.change && rhs.change)
			{
				if (item->imbalance)
					item->colour = RGB(0, 0, 0);
				else
					item->colour = base.dom ? RGB(191, 191, 191) : RGB(127, 127, 127);
			}
			else if (lhs.change && !rhs.change)
			{
				item->imbalance = true;
				item->colour = base.dom ? RGB(0, 127, 0) : RGB(0, 255, 0);
			}
			else if (!lhs.change && rhs.change)
			{
				item->imbalance = true;
				item->colour = base.dom ? RGB(0, 0, 127) : RGB(0, 0, 255);
			}
			else
				item->colour = RGB(127, 0, 127);

			if (item->imbalance && itemParent)
				itemParent->imbalance = true;
		}
	}

private:
	LarvaSchema*	_schema;
	Desc			_base;
	Desc			_lhs;
	Desc			_rhs;

private:
	HWND _tree;
	Item _root;
};

bool VisualConflictResolver(
		HWND parentWindow,
		LarvaSchema* schema,
		Xt::Change* chgMerged, Xt::Change* chgLhs, Xt::Change* chgRhs,
		Xt::DomElement* ancestor, Xt::DomElement* lhs, Xt::DomElement* rhs
	)
{
	VisualConflictResolverWnd wnd(schema, chgMerged, chgLhs, chgRhs, ancestor, lhs, rhs);

	wnd.DoModal(WinPoint(100, 100), WinSize(600, 400), parentWindow);

	return true;
}