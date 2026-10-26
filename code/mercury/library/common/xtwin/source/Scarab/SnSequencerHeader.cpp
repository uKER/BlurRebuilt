#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/SendEmail.h"
#include "ScarabImpl.h"

/*

	[Sequencer(
			path:pathToArray // path to array to visualise
		)]

	[Sequencer(
			background:colour,
			backgroundField:field,

			iconField:field,

			title:summary,
			titleField:field,

			info:summary,
			infoField:field
		)]

	[Sequencer(
			expand:true // expand array as children of parent node
		)]

*/

LarvaType* FollowType(LarvaSchema* schema, LarvaField* field, Xt::DomBlock* block)
{
	if (!schema || !field)
		return 0;

	switch(field->kind)
	{
		case kLfStruct:		return ((LfStruct*)field)->type;
		case kLfInt:		return ((LfInt*)field)->type;
		case kLfFloat:		return ((LfFloat*)field)->type;
		case kLfString:		return ((LfString*)field)->type;
		case kLfWideString:	return ((LfWideString*)field)->type;
		case kLfBool:		return ((LfBool*)field)->type;
		case kLfEnum:		return ((LfEnum*)field)->type;
		case kLfFlags:		return ((LfFlags*)field)->type;
		case kLfWeakPtr:	return ((LfWeakPtr*)field)->baseType;
		case kLfHandle:		return ((LfHandle*)field)->baseType;
		case kLfArray:		return FollowType(schema, ((LfArray*)field)->field, block);
		case kLfLocString:	return ((LfLocString*)field)->type;

		case kLfPtr:
			//if (LarvaType* type = TypeOf(schema, block)) // TODO: COMPONENTS
			//	return type;
		return ((LfPtr*)field)->baseType;
	}

	return 0;
}

class SnSiItem : public SequencerItem
{
public:
	SnSiItem(SnSequencerHeader* seq, XtId id, Xt::DomElement* element, int index, LarvaField* field, LarvaType* followType) :
		_link(this),
		_id(id),
		_seq(seq),
		_dom(DomCallback, this),
		_index(index),
		_field(field),
		_followType(followType)
	{
		XtUt::PushBack(&seq->_allItems, &_link);

		Xt::Monitor(&_dom, element);

		DoLabel();

		//SetHeight(44.0f);
		//SetIndent(22.0f);
	}

public:
	virtual void OnClick()
	{
		if (_index == -1)
		{
			_seq->_sequencer->SetBody(_dom, _index, _field);
		}
		else
		{
			Xt::DomElement* element = _dom;

			if (element)
				element = element->parent;

			_seq->_sequencer->SetBody(element, _index, _field);
		}
	}

private:
	XtU32 ResolveColour(LarvaSchema* schema, LarvaField* field, Xt::DomBlock* block, int depth)
	{
		if (depth < 16)
		{
			if (LarvaType* type = FollowType(schema, field, block))
			{
				if (type->kind == kLtStruct)
				{
					XtString fieldName = type->GetString("Sequencer", "backgroundField");

					if (*fieldName)
					{
						if (LarvaField* newField = FindFieldByName((LtStruct*)type, fieldName))
							return ResolveColour(schema, newField, AsBlock(At(block, newField->name)), depth + 1);
					}
				}

				return type->GetColour("Sequencer", "background", 0xD0D0D0);
			}
		}

		return 0xD0D0D0;
	}

	XtString IconFromBase(LarvaType* type)
	{
		if (*type->icon)
			return type->icon;

		if (LtStruct* typeStruct = AsStruct(type))
		{
			if (typeStruct->base)
			{
				XtString icon = IconFromBase(typeStruct->base->type);

				if (*icon)
					return icon;
			}
		}

		return "";
	}

	XtString ResolveIcon(LarvaSchema* schema, LarvaField* field, Xt::DomBlock* block, int depth)
	{
		if (depth < 16)
		{
			if (LarvaType* type = FollowType(schema, field, block))
			{
				if (type->kind == kLtStruct)
				{
					XtString fieldName = type->GetString("Sequencer", "iconField");

					if (*fieldName)
					{
						if (LarvaField* newField = FindFieldByName((LtStruct*)type, fieldName))
							return ResolveIcon(schema, newField, AsBlock(At(block, newField->name)), depth + 1);
					}
				}

				return IconFromBase(type);
			}
		}

		return "";
	}

	XtString ResolveString(LarvaSchema* schema, LarvaField* field, Xt::DomBlock* block, const XtString& annField, const XtString& annData, const XtString& annDef, int depth)
	{
		if (depth < 16)
		{
			if (LarvaType* type = FollowType(schema, field, block))
			{
				if (type->kind == kLtStruct)
				{
					XtString fieldName = type->GetString("Sequencer", annField);

					if (*fieldName)
					{
						if (LarvaField* newField = FindFieldByName((LtStruct*)type, fieldName))
							return ResolveString(schema, newField, AsBlock(At(block, newField->name)), annField, annData, annDef, depth + 1);
					}

					return "";//DecodeSummaryText(type->GetString("Sequencer", annData, annDef), schema, (LtStruct*)type, block); // TODO: COMPONENTS
				}

				return type->GetString("Sequencer", annData, annDef);
			}
		}

		return annDef;
	}

	void DoLabel()
	{
		if (LarvaSchema* schema = _seq->Panel()->Db()->schema)
		{
			if (Xt::DomElement* element = _dom)
			{
				SetTitle(MbToWc(ResolveString(schema, _field, AsBlock(element), "titleField", "title", "{__type}", 0)));
				SetInfo(MbToWc(ResolveString(schema, _field, AsBlock(element), "infoField", "info", "", 0)));
			}
			else
			{
				SetTitle(L"(null)");
				SetInfo(L"");
			}

			SetBackground(ResolveColour(schema, _field, AsBlock(_dom), 0));
			SetIcon(CacheBitmap(ResolveIcon(schema, _field, AsBlock(_dom), 0)));
		}
		else
		{
			SetTitle(L"(schema?)");
			SetInfo(L"");
		}
	}

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnSiItem* item)
	{
		if ((event->code == Xt::kDomRemoved) && (event->from == item->_dom))
			delete item;
		else
			item->DoLabel();
	}

private:
	XtUt::Link<SnSiItem>	_link;
	XtId					_id;

private:
	SnSequencerHeader*	_seq;
	Xt::DomMonitor		_dom;
	int					_index;
	LarvaField*			_field;
	LarvaType*			_followType;

friend class SnSequencerHeader;
friend struct SnSiBuilderContext;
};

//
//
//

struct SnSiBuilderContext;

struct SnSiBuilderType : public LvTypeDom
{
	SnSiBuilderContext*	ctx;
	bool				create;
	SequencerItem*		parent;

public:
	SnSiBuilderType(SnSiBuilderContext* ctx_, bool create_, SequencerItem* parent_) :
		ctx(ctx_),
		create(create_),
		parent(parent_)
	{
	}

public:
	virtual bool Visit(struct LtStruct* type, Xt::DomBlock* dom);

};

struct SnSiBuilderField : public LvFieldDom
{
	SnSiBuilderContext*	ctx;
	bool				create;
	SequencerItem*		parent;
	int					index;

	XtUt::WeakRef<SequencerItem> lastItem;

public:
	SnSiBuilderField(SnSiBuilderContext* ctx_, bool create_, SequencerItem* parent_, int index_) :
		ctx(ctx_),
		create(create_),
		parent(parent_),
		index(index_)
	{
	}

public:
	virtual bool Visit(struct LfStruct* field, Xt::DomBlock* dom);
	virtual bool Visit(struct LfPtr* field, Xt::DomComponent* dom);
	virtual bool Visit(struct LfArray* field, Xt::DomArray* dom);
};

struct SnSiBuilderContext
{
	SnSequencerHeader*				seq;
	LarvaSchema*					schema;

public:
	SnSiBuilderContext(SnSequencerHeader* seq_, LarvaSchema* schema_) :
		seq(seq_),
		schema(schema_)
	{
	}

public:
	bool VisitType(LarvaType* type, Xt::DomElement* element, bool create, SequencerItem* parent)
	{
		return type->Accept(&SnSiBuilderType(this, create, parent), element);
	}

	bool VisitField(LarvaField* field, Xt::DomElement* element, bool create, SequencerItem* parent, int index)
	{
		return field->Accept(&SnSiBuilderField(this, create, parent, index), element, Xt::kIndexAcquire);
	}

public:
	SnSiItem* Create(SequencerItem* parent, Xt::DomElement* element, int index, LarvaField* field, SequencerItem* after)
	{
		XtId id = AsId(At(AsBlock(element), "__id"));

		if (!id)
			return 0;

		SnSiItem* newItem = seq->FindItem(id);

		LarvaType* followType = FollowType(schema, field, AsBlock(element));

		if (!newItem)
			newItem = new SnSiItem(seq, id, element, index, field, followType);

		if (newItem && (newItem->_followType != followType))
		{
			newItem->_followType = followType;
			newItem->Clear();
		}

		return (SnSiItem*)parent->Insert(newItem, after);
	}
};

bool SnSiBuilderType::Visit(LtStruct* type, Xt::DomBlock* dom)
{
	if (type->base)
		ctx->VisitType(type->base->type, dom, false, parent);

	for(XtUt::List<LarvaField>::Iterator it(&type->fields); it; ++it)
		ctx->VisitField(it, dom, false, parent, -1);

	return true;
}

bool SnSiBuilderField::Visit(LfStruct* field, Xt::DomBlock* dom)
{
	SequencerItem* newParent = parent;

	if (create)
	{
		if ((newParent = ctx->Create(parent, dom, index, field, lastItem)) == 0)
			return false;

		lastItem = newParent->Anchor();
	}

	return ctx->VisitType(field->type, dom, false, newParent);
}

bool SnSiBuilderField::Visit(LfPtr* field, Xt::DomComponent* dom)
{
	if (dom)
	{
		bool newCreate = create || field->attrs.GetBool("Sequencer", "expand", false);

		SequencerItem* newParent = parent;

		if (newCreate)
		{
			if ((newParent = ctx->Create(parent, dom, index, field, lastItem)) == 0)
				return false;

			lastItem = newParent->Anchor();
		}

		if (LarvaType* type = TypeOf(ctx->schema, field->baseType, dom))
		{
			if (!ctx->VisitType(type, dom, false, newParent))
				return false;
		}
	}

	return true;
}

bool SnSiBuilderField::Visit(LfArray* field, Xt::DomArray* dom)
{
	if (dom)
	{
		bool newCreate = create || field->attrs.GetBool("Sequencer", "expand", false);

		SnSiBuilderField v(ctx, newCreate, parent, -1);

		for(int i = 0; i < (int)Count(dom); i++)
		{
			v.index = i;

			if (!field->field->Accept(&v, At(dom, i), Xt::kIndexAcquire))
				return false;
		}
	}

	return true;
}

//
//SnSequencerHeader
//

SnSequencerHeader::SnSequencerHeader(SnSequencer* sequencer) :
	_dom(DomCallback, this),
	_ignoreNotifications(0),
	_idSequencer(this),
	_sequencer(sequencer)
{
}

SnSequencerHeader::~SnSequencerHeader()
{
}

void SnSequencerHeader::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(parentDom);
}

void SnSequencerHeader::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireBlock(parentDom, index));
}

bool SnSequencerHeader::OnBuild()
{
	if (!_idSequencer.Allocate())
		return false;

	if ((HWND)_wndSequencer == 0)
	{
		if (!_wndSequencer.Create(*Panel(), _idSequencer.IdAsHMenu()))
			return false;
	}

	return true;
}

void SnSequencerHeader::OnMeasure(HDC ic)
{
	_size.cx = 280;
	_size.cy = 400;
}

void SnSequencerHeader::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_wndSequencer, _client.left, _client.top, _client.Width(), _client.Height(), SWP_SHOWWINDOW);
}

LarvaField* FindFieldByPath(LtStruct* type, const char* path)
{
	if (!type || !*path)
		return 0;

	const char* start = path;

	while((*path != '\0') && (*path != '/') && (*path != '\\'))
		path++;

	const char* end = path;

	while((*path == '/') || (*path == '\\'))
		path++;

	XtString token(start, end - start);

	LarvaField* field = FindFieldByName(type, token);

	if (!field)
		return 0;

	if (!*path)
		return field;

	if (field->kind == kLfStruct)
		return FindFieldByPath(((LfStruct*)field)->type, path);

	return 0;
}

void SnSequencerHeader::SetDom(Xt::DomBlock* block)
{
	XtString path = _sequencer->GetString("Sequencer", "path", "");

	Monitor(&_dom, AsArray(Xt::AcquireElementByPath(block, path, Xt::kDomArray)));

	_domField = FindFieldByPath(_sequencer->_field->container, path);

	SyncControls();
}

void SnSequencerHeader::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnSequencerHeader* seq)
{
	if ((event->code == Xt::kDomRemoved) && (event->from == monitor->element))
		seq->SetDom(0);
	else
		seq->SyncControls();
}

void SnSequencerHeader::SyncControls()
{
	Xt::DomArray* arr = AsArray(_dom);

	if (!arr || !_domField)
		return;

	if (_ignoreNotifications <= 0)
	{
		_ignoreNotifications++;

		SnSiBuilderContext ctx(this, Panel()->Db()->schema);

		ctx.VisitField(_domField, arr->parent, true, _wndSequencer.Root(), -1);

		_ignoreNotifications--;
	}
}

SnSiItem* SnSequencerHeader::FindItem(XtId id)
{
	for(XtUt::LinkList<SnSiItem>::Iterator it(&_allItems); it; ++it)
	{
		if (it->_id == id)
			return it;
	}

	return 0;
}