#ifndef XTTFLASKHELPERS_H
#define XTTFLASKHELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Xt/Impl/Grimoire.h"
#include "../../Common/XtFlask.h"

// ExportString

enum ExportStringEncoding
{
	kEseAnsi,
	kEseUnicode
};

struct ExportString
{
	XtString				text;
	size_t					offset;
	ExportStringEncoding	encoding;
	Sz16					uniText;
};

struct ExportStringKey
{
	XtString				text;
	ExportStringEncoding	encoding;

public:
	ExportStringKey(const XtString& text_, ExportStringEncoding encoding_) :
		text(text_),
		encoding(encoding_)
	{
	}

public:
	bool operator<(const ExportStringKey& rhs) const
	{
		if (encoding < rhs.encoding)
			return true;

		if (encoding > rhs.encoding)
			return false;

		return text < rhs.text;
	}
};

struct ExportStringTable
{
	XtUt::Dictionary<ExportStringKey, ExportString>	entries;
	size_t											offset;

public:
	inline ExportStringTable() :
		offset(0)
	{
	}
};

struct ExportType;

struct ExportFieldPrep
{
	virtual ~ExportFieldPrep();
	virtual XtString Eval(Xt::DomElement* dom);
};

struct EfpMul : public ExportFieldPrep
{
	XtString fieldName;

public:
	virtual ~EfpMul();
	virtual XtString Eval(Xt::DomElement* dom);
};

struct EfpRecordName : public ExportFieldPrep
{
	XtString fieldName;

public:
	virtual ~EfpRecordName();
	virtual XtString Eval(Xt::DomElement* dom);
};

struct ExportBase
{
	XtU32		index;
	ExportType*	type;
	XtU32		offset;
	XtU32		fieldOffset;

public:
	inline ExportBase(XtU32 index_) :
		index(index_),
		type(0),
		offset(0),
		fieldOffset(0)
	{
	}
};

struct ExportField
{
	XtU32				index;
	XtString			name;
	ExportType*			type;
	bool				array;
	XtFieldBehaviour	behaviour;
	XtU32				offset;
	XtU32				fieldIndex;
	int					radix;
	ExportFieldPrep*	prep;

public:
	inline ExportField(XtU32 index_) :
		index(index_),
		type(0),
		array(false),
		behaviour(kXtFbImplInfer),
		offset(0),
		fieldIndex(0),
		radix(10),
		prep(0)
	{
	}

	inline ~ExportField()
	{
		delete prep;
	}
};

struct ExportType
{
	XtU32				index;
	XtTypeBehaviour		behaviour;
	XtString			name;
	XtU32				size;
	XtU32				totalFieldCount;
	XtU32				totalHandleCount;
	XtAtomType			atom;

	XtUt::List<ExportBase>	bases;
	XtUt::List<ExportField>	fields;

public:
	inline ExportType(XtU32 index_) :
		index(index_),
		behaviour(kXtTbStruct),
		size(0),
		totalFieldCount(0),
		totalHandleCount(0),
		atom(kXtNotAtom)
	{
	}
};

struct ExportSchema
{
	XtUt::List<ExportType> types;
	XtUt::Dictionary<XtString, ExportType*> lookup;
};

struct ExportComponent
{
	ExportType*				type;
	size_t					count;
	XtComponentBehaviour	behaviour;
	XtU32					offset;

public:
	inline ExportComponent() :
		type(0),
		count(0),
		behaviour(kXtCbObject),
		offset(0)
	{
	}
};

struct ExportHandleFixup
{
	XtU32 offset;

public:
	ExportHandleFixup() :
		offset(-1)
	{
	}
};

struct ExportRecord
{
	XtU32							id;
	XtUt::Array<ExportComponent>	components;
	XtUt::Array<ExportHandleFixup>	handleFixups;
	XtUt::Block						data;
	XtUt::Block						strings;

public:
	inline ExportRecord() :
		id(0)
	{
	}
};

struct CompileElement;
struct CompileComponent;

enum CompileElementBehaviour
{
	kCeNone,
	kCeWeak
};

struct CompileField
{
	CompileElement* element;

public:
	inline CompileField() :
		element(0)
	{
	}
};

struct CompileElement
{
	int							index;
	CompileComponent*			component;
	Xt::DomElement*				dom;
	XtUt::Array<CompileField>	fields;
	CompileElementBehaviour		behaviour;

public:
	inline CompileElement() :
		index(0),
		component(0),
		dom(0),
		behaviour(kCeNone)
	{
	}
};

struct CompileId
{
	XtId			id;
	CompileElement*	element;

public:
	inline CompileId() :
		element(0)
	{
	}
};

struct CompileComponent
{
	int							index;
	ExportType*					type;
	XtComponentBehaviour		behaviour;
	XtUt::List<CompileElement>	elements;
	int							offset;

public:
	inline CompileComponent() :
		index(0),
		type(0),
		behaviour(kXtCbObject),
		offset(0)
	{
	}
};

struct CompileContext
{
	ExportSchema*						schema;
	XtUt::List<CompileComponent>		components;
	XtUt::Array<CompileComponent*>		maps[kXtComponentBehaviourCount];
	XtUt::Dictionary<XtId, CompileId>	ids;
	ExportStringTable					strings;
	ExportRecord*						record;

public:
	inline CompileContext() :
		schema(0),
		record(0)
	{
	}
};

const ExportString* Add(ExportStringTable* st, const XtString& string, ExportStringEncoding encoding);

ExportType* FindTypeByName(ExportSchema* schema, const char* name);
ExportType* FindTypeByToken(ExportSchema* schema, const Grimoire::Token* token);
ExportField* FindFieldByName(ExportType* type, const char* name);

bool CreateExportSchema(Grimoire::Dom* grim, ExportSchema* schema, Xt::StatusCallback* status);

XtU32 SizeOf(ExportComponent* component);
XtU32 SizeOf(CompileComponent* component);
XtU32 HandleCountOf(CompileComponent* component);

void SwizzleRecord(ExportRecord* record);

#include "XtFlaskExport.h"

#endif // XTTFLASKHELPERS_H