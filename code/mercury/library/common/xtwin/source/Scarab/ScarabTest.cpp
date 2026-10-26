#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

class XtFormElement
{
public:
	XtFormElement() : _parentLink(this) { }
	virtual ~XtFormElement() { }

public:
	XtFormElement*					_parent;
	XtUt::Link<XtFormElement>		_parentLink;
	XtUt::LinkList<XtFormElement>	_children;

public:
	SchemaElement*	_schemaElement;
	Xt::DomMonitor	_dom;
};

void DoBuildStruct(Xt::DomDb* db, XtFormElement* parent, LtStruct* type, Xt::DomBlock* dom)
{
	if (type->base && !type->base->hidden)
	{
		DoBuildStruct(db, parent, type->base->type, dom);
	}

	for(LtStruct::FieldIterator itField(&type->fields); itField; ++itField)
	{
		if (itField->hidden)
			continue;

		XtFormElement* fe = new XtFormElement;

		fe->_parent = parent;

		XtUt::PushBack(&parent->_children, &fe->_parentLink);

		fe->_schemaElement = itField;

		Monitor(&fe->_dom, dom);
	}
}

void DoSchemaElement(Xt::DomDb* db, XtFormElement* parent, SchemaElement* se, Xt::DomElement* dom)
{
	switch(se->kind)
	{
		case kSchemaBaseType:
		case kSchemaOption:
		break;

		case kLtStruct:
			DoBuildStruct(db, parent, (LtStruct*)se, AsBlock(dom));
		break;

		case kLtAtom:
		case kLtEnum:
		case kLtFlags:
		break;

		case kLfStruct:
		case kLfInt:
		case kLfFloat:
		case kLfString:
		case kLfWideString:
		case kLfBool:
		case kLfEnum:
		case kLfFlags:
		case kLfStrictPtr:
		case kLfPtr:
		case kLfWeakPtr:
		case kLfHandle:
		case kLfArray:
		case kLfLocString:
		case kLfControl:
		break;
	}
}

void ScarabTest(Xt::DomDb* db)
{
	/*Xt::DomRecord*	record	= Xt::FindRecordById(db, 0x2F4C25ED);
	LtStruct*		type	= Xt::TypeOf(db->schema, record);

	XtFormElement root;

	DoBuildStruct(db, &root, type, &record->root.data);*/
}





class XtFormControl
{
protected:
	XtFormElement* _element;
};
