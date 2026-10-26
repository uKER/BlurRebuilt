#include "Common/XtPch.h"
#include "Xt/XtTool.h"

bool IsBaseOf(const LtStruct* base, const LtStruct* derived)
{
	if (!base)
		return false;

	for(; derived; derived = derived->base->type)
	{
		if (base == derived)
			return true;

		if (!derived->base)
			break;
	}

	return false;
}

LarvaType* FindTypeByName(LarvaSchema* schema, const XtString& typeName)
{
	return schema ? At(&schema->types, typeName) : 0;
}

LarvaField* FindFieldByName(LtStruct* type, const char* fieldName)
{
	for(; type; type = type->base->type)
	{
		for(LtStruct::FieldIterator it(&type->fields); it; ++it)
		{
			if (strcmp(it->name, fieldName) == 0)
				return it;
		}

		if (!type->base)
			break;
	}

	return 0;
}

LarvaOption* FindOptionByName(LtEnum* enumType, const char* optionName)
{
	if (!enumType)
		return 0;

	for(LtEnum::OptionIterator it(&enumType->options); it; ++it)
	{
		if (strcmp(it->name, optionName) == 0)
			return it;
	}

	return 0;
}

LarvaOption* FindOptionByName(LtFlags* flagsType, const char* optionName)
{
	if (!flagsType)
		return 0;

	for(LtEnum::OptionIterator it(&flagsType->options); it; ++it)
	{
		if (strcmp(it->name, optionName) == 0)
			return it;
	}

	return 0;
}

LtStruct* FindCommonBase(LtStruct* lhs, LtStruct* rhs)
{
	if (!rhs)
		return 0;

	for(; lhs; lhs = lhs->base->type)
	{
		if (IsBaseOf(lhs, rhs))
			return lhs;

		if (!lhs->base)
			break;
	}

	return 0;
}

//
// Misc helpers
//

LtStruct* AsStruct(LarvaType* type)
{
	return (type && (type->kind == kLtStruct)) ? (LtStruct*)type : 0;
}

LfStruct* AsStruct(LarvaField* field)
{
	return (field && (field->kind == kLfStruct)) ? (LfStruct*)field : 0;
}

LtAtom* AsAtom(LarvaType* type)
{
	return (type && (type->kind == kLtAtom)) ? (LtAtom*)type : 0;
}

LtEnum* AsEnum(LarvaType* type)
{
	return (type && (type->kind == kLtEnum)) ? (LtEnum*)type : 0;
}

LfEnum* AsEnum(LarvaField* field)
{
	return (field && (field->kind == kLfEnum)) ? (LfEnum*)field : 0;
}

LtFlags* AsFlags(LarvaType* type)
{
	return (type && (type->kind == kLtFlags)) ? (LtFlags*)type : 0;
}

LfFlags* AsFlags(LarvaField* field)
{
	return (field && (field->kind == kLfFlags)) ? (LfFlags*)field : 0;
}

namespace Xt
{
	XtId ParseId(const char* text)
	{
		return ParseId(text, text + strlen(text));
	}

	XtId ParseId(const char* start, const char* end)
	{
		XtId id = 0;

		for(; start < end; start++)
		{
			if ((*start >= '0') && (*start <= '9'))
				id = (id << 4) + (XtId)(*start - '0');
			else if ((*start >= 'A') && (*start <= 'F'))
				id = (id << 4) + 10 + (XtId)(*start - 'A');
			else if ((*start >= 'a') && (*start <= 'f'))
				id = (id << 4) + 10 + (XtId)(*start - 'a');
			else
				return 0;
		}

		return id;
	}
}

//
// GenerateIdForComponent
//

bool GenerateIdForComponent(Xt::DomComponent* component, bool force)
{
	if (!component)
		return false;

	if (!force && component->id)
		return true;

	return SetId(component, Xt::GenerateUniqueComponentId());
}

//
// GenerateIdForComponentAndChildren
//

struct GenerateIdLc : public LvComponents
{
	bool result;
	bool force;

public:
	GenerateIdLc(bool force_) :
		result(true),
		force(force_)
	{
	}

public:
	virtual bool Visit(LtStruct* type, Xt::DomComponent* dom)
	{
		if (!GenerateIdForComponent(dom, force))
			result = false;

		return true;
	}
};

bool GenerateIdForComponentAndChildren(LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component, bool force)
{
	GenerateIdLc gi(force);

	return VisitAllComponents(schema, type, component, &gi) && gi.result;
}

// Xt::GetByDottedPath

namespace Xt
{
	FieldDom GetByDottedPath(FieldDom fd, const char* path)
	{
		if (!path || !*path)
			return fd;

		if (!fd.field)
			return FieldDom();

		const char* start	= path;
		const char* end		= path;

		while((*end != '\0') && (*end != '.'))
			end++;

		XtString token(start, end - start);

		if (*end == '.')
			end++;

		switch(fd.field->kind)
		{
			case kLfStruct:
			{
				LarvaField* field = FindFieldByName(AsStruct(fd.field)->type, token);

				return GetByDottedPath(FieldDom(field, At(AsBlock(fd.dom), field)), end);
			}
			break;

			case kLfHandle:
			{
				if (Xt::DomDb* db = DbOf(fd.dom))
				{
					if (Xt::DomRecord* record = FindRecordById(db, ParseId(AsText(fd.dom))))
					{
						if (LtStruct* type = TypeOf(db->schema, record))
						{
							if (LarvaField* field = FindFieldByName(type, token))
								return GetByDottedPath(FieldDom(field, At(&record->root.data, field)), end);
						}
					}
				}
			}

			break;

			// TODO: StrictPtr, Ptr, WeakPtr

			case kLfArray:
			return GetByDottedPath(FieldDom(((LfArray*)fd.field)->field, At(AsArray(fd.dom), atoi(token))), end);
		}

		return FieldDom();
	}

	FieldDom GetByDottedPath(LtStruct* type, DomBlock* dom, const char* path)
	{
		if (!type || !dom || !path || !*path)
			return FieldDom();

		const char* start	= path;
		const char* end		= path;

		while((*end != '\0') && (*end != '.'))
			end++;

		XtString token(start, end - start);

		LarvaField* field = FindFieldByName(type, token);

		if (!field)
			return FieldDom();

		if (*end == '\0')
			return FieldDom(field, At(dom, field));

		return GetByDottedPath(FieldDom(field, At(dom, field)), end + 1);
	}
}

//
// FindComponentById
//

namespace
{
	struct FindComponentVisitor : public LvComponents
	{
		XtId				_id;
		Xt::TypeComponent	_tc;

	public:
		FindComponentVisitor(XtId id) :
			_id(id)
		{
		}

	public:
		virtual bool Visit(LtStruct* type, Xt::DomComponent* dom)
		{
			if (!dom || (dom->id != _id))
				return true;

			_tc.type		= type;
			_tc.component	= dom;

			return false;
		}
	};
}

Xt::TypeComponent FindComponentById(LarvaSchema* schema, Xt::DomRecord* record, XtId id)
{
	if (!schema || !record || !id)
		return Xt::TypeComponent();

	FindComponentVisitor v(id);

	VisitAllComponents(schema, TypeOf(schema, record), &record->root, &v);

	return v._tc;
}

//
// FindFieldBySemantic / AcquireFieldBySemantic
//

struct FindFieldVisitor : public LvFieldDom
{
	const char*		_attr;
	const char*		_param;
	const char*		_value;
	Xt::IndexMethod	_method;
	Xt::FieldDom	_fd;

public:
	FindFieldVisitor(const char* attr, const char* param, const char* value, bool acquire) :
		_attr(attr),
		_param(param),
		_value(value),
		_method(acquire ? Xt::kIndexAcquire : Xt::kIndexAt)
	{
	}

public:
	bool DoCheck(LarvaField* field, Xt::DomElement* element)
	{
		if (field->attrs.GetString(_attr, _param) != _value)
			return true;

		if (element)
			_fd = Xt::FieldDom(field, element);

		return false;
	}

public:
	virtual bool Visit(LfStruct* field, Xt::DomBlock* dom)			{ return DoCheck(field, dom) && VisitMembers(field->type, dom, this, _method); }
	virtual bool Visit(LfInt* field, Xt::DomField* dom)				{ return DoCheck(field, dom); }
	virtual bool Visit(LfFloat* field, Xt::DomField* dom)			{ return DoCheck(field, dom); }
	virtual bool Visit(LfString* field, Xt::DomField* dom)			{ return DoCheck(field, dom); }
	virtual bool Visit(LfWideString* field, Xt::DomField* dom)		{ return DoCheck(field, dom); }
	virtual bool Visit(LfBool* field, Xt::DomField* dom)			{ return DoCheck(field, dom); }
	virtual bool Visit(LfEnum* field, Xt::DomField* dom)			{ return DoCheck(field, dom); }
	virtual bool Visit(LfFlags* field, Xt::DomBlock* dom)			{ return DoCheck(field, dom); }
	virtual bool Visit(LfStrictPtr* field, Xt::DomComponent* dom)	{ return DoCheck(field, dom); }
	virtual bool Visit(LfPtr* field, Xt::DomComponent* dom)			{ return DoCheck(field, dom); }
	virtual bool Visit(LfWeakPtr* field, Xt::DomRef* dom)			{ return DoCheck(field, dom); }
	virtual bool Visit(LfHandle* field, Xt::DomField* dom)			{ return DoCheck(field, dom); }
	virtual bool Visit(LfArray* field, Xt::DomArray* dom)			{ return DoCheck(field, dom); }
	virtual bool Visit(LfLocString* field, Xt::DomBlock* dom)		{ return DoCheck(field, dom); }
	virtual bool Visit(LfControl* field, Xt::DomElement* dom)		{ return DoCheck(field, dom); }
};

Xt::FieldDom FindBySemantic(LtStruct* type, Xt::DomBlock* block, const char* attr, const char* param, const char* value)
{
	if (!type || !attr || !param || !value)
		return Xt::FieldDom();

	FindFieldVisitor v(attr, param, value, false);

	VisitMembers(type, block, &v, v._method);

	return v._fd;
}

Xt::FieldDom AcquireBySemantic(LtStruct* type, Xt::DomBlock* block, const char* attr, const char* param, const char* value)
{
	if (!type || !attr || !param || !value)
		return Xt::FieldDom();

	FindFieldVisitor v(attr, param, value, true);

	VisitMembers(type, block, &v, v._method);

	return v._fd;
}

//
// RegenerateComponentIds
//

namespace
{
	struct RegenerateComponentIdsVisitor : public LvComponents
	{
		Xt::IdMap* map;

	public:
		RegenerateComponentIdsVisitor(Xt::IdMap* map_) :
			map(map_)
		{
		}

	public:
		virtual bool Visit(LtStruct* type, Xt::DomComponent* dom)
		{
			if (dom)
			{
				XtId newId = Xt::GenerateUniqueComponentId();

				if (!newId || (map && !XtUt::Insert(map, dom->id, new XtId(newId))))
					return false;

				SetId(dom, newId);
			}

			return true;
		}
	};
}

bool RegenerateComponentIds(LarvaSchema* schema, LtStruct* type, Xt::DomBlock* block, Xt::IdMap* map)
{
	return VisitAllComponents(schema, type, block, &RegenerateComponentIdsVisitor(map));
}

bool RegenerateComponentIds(LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component, Xt::IdMap* map)
{
	return VisitAllComponents(schema, type, component, &RegenerateComponentIdsVisitor(map));
}

bool RegenerateComponentIds(LarvaSchema* schema, LarvaField* field, Xt::DomElement* element, Xt::IdMap* map)
{
	return VisitAllComponents(schema, field, element, &RegenerateComponentIdsVisitor(map));
}

//
// Duplicate
//

Xt::DomElement* Duplicate(LarvaSchema* schema, LtStruct* type, Xt::DomBlock* block)
{
	if (!schema || !type || !block)
		return 0;

	XtUt::Ptr<Xt::DomBlock> newBlock = (Xt::DomBlock*)Clone(block);

	Xt::IdMap map;

	if (!RegenerateComponentIds(schema, type, newBlock, &map))
		return 0;

	MapWeakPtrs(schema, type, newBlock, &map);

	RegenerateUniqueIds(schema, type, newBlock);

	return Pop(&newBlock);
}

Xt::DomElement* Duplicate(LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component)
{
	if (!schema || !type || !component)
		return 0;

	XtUt::Ptr<Xt::DomComponent> newComponent = (Xt::DomComponent*)Clone(component);

	Xt::IdMap map;

	if (!RegenerateComponentIds(schema, type, newComponent, &map))
		return 0;

	MapWeakPtrs(schema, type, newComponent, &map);

	RegenerateUniqueIds(schema, type, newComponent);

	return Pop(&newComponent);
}

Xt::DomElement* Duplicate(LarvaSchema* schema, LarvaField* field, Xt::DomElement* element)
{
	if (!schema || !field || !element)
		return 0;

	XtUt::Ptr<Xt::DomElement> newElement = Clone(element);

	Xt::IdMap map;

	if (!RegenerateComponentIds(schema, field, newElement, &map))
		return 0;

	MapWeakPtrs(schema, field, newElement, &map);

	RegenerateUniqueIds(schema, field, newElement);

	return Pop(&newElement);
}

namespace Xt
{
	bool Equal(FieldPath* lhs, FieldPath* rhs)
	{
		if (!lhs || !rhs)
			return false;

		if (lhs->field != rhs->field)
			return false;

		if (!lhs->next && !rhs->next)
			return true;

		return Equal(lhs->next, rhs->next);
	}

	FieldPath* Clone(FieldPath* path)
	{
		if (!path)
			return 0;

		FieldPath* newPath = new FieldPath(path->field);

		if (newPath)
			newPath->next = Clone(path->next);

		return newPath;
	}

	void Free(FieldPath* path)
	{
		if (path)
		{
			Free(path->next);

			delete path;
		}
	}

	void ToStringSub(XtString& text, FieldPath* path)
	{
		if (!path)
			return;

		static XtString kDot(".");

		Append(text, kDot);
		Append(text, path->field->label);

		ToStringSub(text, path->next);
	}

	XtString ToString(FieldPath* path)
	{
		if (!path)
			return kXtNullString;

		XtString result = path->field->label;

		ToStringSub(result, path->next);

		return result;
	}

	FieldDom At(LarvaSchema* schema, LtStruct* type, DomBlock* dom, FieldPath* path)
	{
		if (!schema || !type || !dom || !path || !path->field)
			return FieldDom();

		DomElement* element = At(dom, path->field);

		if (!path->next)
			return FieldDom(path->field, element);

		return At(schema, AsStruct(path->field->Type()), AsBlock(element), path->next);
	}

	FieldDom Acquire(LarvaSchema* schema, LtStruct* type, DomBlock* dom, FieldPath* path)
	{
		if (!schema || !type || !dom || !path || !path->field)
			return FieldDom();

		DomElement* element = Acquire(dom, path->field);

		if (!path->next)
			return FieldDom(path->field, element);

		return At(schema, AsStruct(path->field->Type()), AsBlock(element), path->next);
	}
}
