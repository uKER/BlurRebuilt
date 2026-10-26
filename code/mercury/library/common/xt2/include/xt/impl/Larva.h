#ifndef XT_IMPL_LARVA_H
#define XT_IMPL_LARVA_H

#ifdef _MSC_VER
#pragma once
#endif

//
// Xt::Larva
//

// Xt::LarvaTypeLayout

enum LarvaTypeLayout
{
	kLtlDefault,
	kLtlVertical,
	kLtlHorizontal,
	kLtlInline
};

// Xt::LarvaTypeBorder

enum LarvaTypeBorder
{
	kLtbDefault,
	kLtbNone,
	kLtbNormal
};

// Xt::LarvaTypeKind

enum SchemaElementKind
{
	kSchemaBaseType,
	kSchemaOption,

	kLtStruct,
	kLtAtom,
	kLtEnum,
	kLtFlags,

	kLfStruct,
	kLfInt,
	kLfFloat,
	kLfString,
	kLfWideString,
	kLfBool,
	kLfEnum,
	kLfFlags,
	kLfStrictPtr,
	kLfPtr,
	kLfWeakPtr,
	kLfHandle,
	kLfArray,
	kLfLocString,
	kLfControl
};

// Xt::LarvaType prototypes

struct LarvaType;
struct LtStruct;
struct LtAtom;
struct LtEnum;
struct LtFlags;

// Xt::LarvaField prototypes

struct LarvaField;
struct LfStruct;
struct LfInt;
struct LfFloat;
struct LfString;
struct LfWideString;
struct LfBool;
struct LfEnum;
struct LfFlags;
struct LfStrictPtr;
struct LfPtr;
struct LfWeakPtr;
struct LfHandle;
struct LfArray;
struct LfLocString;
struct LfControl;

// Xt::LarvaParam

struct LarvaAttrList;

struct LarvaParam
{
	XtString key;

public:
	virtual ~LarvaParam();

public:
	virtual XtString		AsString();
	virtual double			AsFloat();
	virtual XtI64			AsInt();
	virtual bool			AsBool(bool defaultValue = false);
	virtual XtU32			AsColour(XtU32 defaultValue = 0xFFFFFF);
	virtual LarvaAttrList*	AsAttr();
};

// Xt::LarvaParamString

struct LarvaParamString : public LarvaParam
{
	XtString value;

public:
	virtual XtString	AsString();
	virtual double		AsFloat();
	virtual XtI64		AsInt();
	virtual bool		AsBool(bool defaultValue = false);
	virtual XtU32		AsColour(XtU32 defaultValue = 0xFFFFFF);
};

// Xt::LarvaAttr

struct LarvaAttr
{
	XtString				name;
	XtUt::List<LarvaParam>	params;

public:
	LarvaParam* FindParam(const char* paramName);

public:
	XtString GetString(const char* paramName, const XtString& defaultValue = kXtNullString);
	double GetFloat(const char* paramName, double defaultValue = 0.0f);
	XtI64 GetInt(const char* paramName, XtI64 defaultValue = 0);
	bool GetBool(const char* paramName, bool defaultValue = false);
	XtU32 GetColour(const char* paramName, XtU32 defaultValue = 0xFFFFFF);
};

// Xt::LarvaAttrList

struct LarvaAttrList
{
	XtUt::List<LarvaAttr> attrs;

public:
	LarvaAttr* FindAttr(const char* attrName);
	LarvaParam* FindParam(const char* attrName, const char* paramName);

public:
	XtString GetString(const char* attrName, const char* paramName, const XtString& defaultValue = kXtNullString);
	double GetFloat(const char* attrName, const char* paramName, double defaultValue = 0.0f);
	XtI64 GetInt(const char* attrName, const char* paramName, XtI64 defaultValue = 0);
	bool GetBool(const char* attrName, const char* paramName, bool defaultValue = false);
	XtU32 GetColour(const char* attrName, const char* paramName, XtU32 defaultValue = 0xFFFFFF);
};

// Xt::LarvaParamAttr

struct LarvaParamAttr : public LarvaParam
{
	LarvaAttrList attrs;

public:
	virtual LarvaAttrList* AsAttr();
};

// Xt::LvType

struct LvType
{
public:
	LvType();
	virtual ~LvType();

public:
	virtual bool Visit(LtStruct* type);
	virtual bool Visit(LtAtom* type);
	virtual bool Visit(LtEnum* type);
	virtual bool Visit(LtFlags* type);
};

// Xt::LvField

struct LvField
{
public:
	LvField();
	virtual ~LvField();

public:
	virtual bool Visit(LfStruct* field);
	virtual bool Visit(LfInt* field);
	virtual bool Visit(LfFloat* field);
	virtual bool Visit(LfString* field);
	virtual bool Visit(LfWideString* field);
	virtual bool Visit(LfBool* field);
	virtual bool Visit(LfEnum* field);
	virtual bool Visit(LfFlags* field);
	virtual bool Visit(LfStrictPtr* field);
	virtual bool Visit(LfPtr* field);
	virtual bool Visit(LfWeakPtr* field);
	virtual bool Visit(LfHandle* field);
	virtual bool Visit(LfArray* field);
	virtual bool Visit(LfLocString* field);
	virtual bool Visit(LfControl* field);
};

// Xt::LvTypeDom

struct LvTypeDom
{
public:
	LvTypeDom();
	virtual ~LvTypeDom();

public:
	virtual bool Visit(LtStruct* type, Xt::DomBlock* dom);
	virtual bool Visit(LtAtom* type, Xt::DomField* dom);
	virtual bool Visit(LtEnum* type, Xt::DomField* dom);
	virtual bool Visit(LtFlags* type, Xt::DomBlock* dom);
};

// Xt::LvFieldDom

struct LvFieldDom
{
public:
	LvFieldDom();
	virtual ~LvFieldDom();

public:
	virtual bool Visit(LfStruct* field, Xt::DomBlock* dom);
	virtual bool Visit(LfInt* field, Xt::DomField* dom);
	virtual bool Visit(LfFloat* field, Xt::DomField* dom);
	virtual bool Visit(LfString* field, Xt::DomField* dom);
	virtual bool Visit(LfWideString* field, Xt::DomField* dom);
	virtual bool Visit(LfBool* field, Xt::DomField* dom);
	virtual bool Visit(LfEnum* field, Xt::DomField* dom);
	virtual bool Visit(LfFlags* field, Xt::DomBlock* dom);
	virtual bool Visit(LfStrictPtr* field, Xt::DomComponent* dom);
	virtual bool Visit(LfPtr* field, Xt::DomComponent* dom);
	virtual bool Visit(LfWeakPtr* field, Xt::DomRef* dom);
	virtual bool Visit(LfHandle* field, Xt::DomField* dom);
	virtual bool Visit(LfArray* field, Xt::DomArray* dom);
	virtual bool Visit(LfLocString* field, Xt::DomBlock* dom);
	virtual bool Visit(LfControl* field, Xt::DomElement* dom);
};

// Xt::LvObjects

struct LvObjects
{
public:
	LvObjects();
	virtual ~LvObjects();

public:
	virtual bool Visit(LtStruct* type, Xt::DomBlock* dom);
};

// Xt::LvComponents

struct LvComponents
{
public:
	LvComponents();
	virtual ~LvComponents();

public:
	virtual bool Visit(LtStruct* type, Xt::DomComponent* dom);
};

// Xt::SchemaElement

struct SchemaElement
{
	SchemaElementKind	kind;
	Sz8					name;
	Sz8					label;
	LarvaAttrList		attrs;
	Sz8					sourceFile;
	int					sourceLine;

public:
	SchemaElement(SchemaElementKind kind_);
	virtual ~SchemaElement();
};


// Xt::LarvaField

struct LarvaField : public SchemaElement
{
	XtString		tab;
	XtString		group;
	bool			hidden;
	bool			noLabelColon;
	LarvaTypeLayout	layout;
	LarvaTypeBorder	border;
	LtStruct*		container;

public:
	LarvaField(SchemaElementKind kind_, LtStruct* container_);
	virtual ~LarvaField();

public:
	virtual bool Accept(LvField* visitor) = 0;
	virtual bool Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method) = 0;

public:
	virtual LarvaType* Type() = 0;
};

// Xt::LarvaOption

struct LarvaOption : public SchemaElement
{
	XtString defValue;
	XtString summary;

public:
	LarvaOption();
	~LarvaOption();
};

// Xt::LarvaBaseType

struct LarvaBaseType : public SchemaElement
{
	LtStruct*	type;
	bool		hidden;

public:
	LarvaBaseType();
	~LarvaBaseType();
};

// Xt::LarvaType

struct LarvaType : public SchemaElement
{
	XtString		category;
	LarvaTypeLayout	layout;
	LarvaTypeBorder	border;
	bool			hidden;
	XtString		icon;

public:
	LarvaType(SchemaElementKind kind_);
	virtual ~LarvaType();

public:
	virtual bool Accept(LvType* visitor) = 0;
	virtual bool Accept(LvTypeDom* visitor, Xt::DomElement* element) = 0;

public:
	virtual LarvaAttr* FindAttr(const char* attrName);
	virtual LarvaParam* FindParam(const char* attrName, const char* paramName);

public:
	XtString GetString(const char* attrName, const char* paramName, const XtString& defaultValue = kXtNullString);
	double GetFloat(const char* attrName, const char* paramName, double defaultValue = 0.0f);
	XtI64 GetInt(const char* attrName, const char* paramName, XtI64 defaultValue = 0);
	bool GetBool(const char* attrName, const char* paramName, bool defaultValue = false);
	XtU32 GetColour(const char* attrName, const char* paramName, XtU32 defaultValue = 0xFFFFFF);
};

// Xt::LarvaSchema

struct LarvaSchema
{
	XtUt::Dictionary<XtString, LarvaType> types;

public:
	typedef XtUt::Dictionary<XtString, LarvaType>::Iterator TypeIterator;

public:
	LarvaSchema();
	~LarvaSchema();
};

//
// Xt::LarvaTypes
//

// Xt::LtStruct

struct LtStruct : public LarvaType
{
	LarvaBaseType*			base;
	XtUt::List<LarvaField>	fields;
	bool					isRecord;
	bool					hasExport;

public:
	typedef XtUt::List<LarvaField>::Iterator FieldIterator;

public:
	LtStruct();
	virtual ~LtStruct();

public:
	virtual bool Accept(LvType* visitor);
	virtual bool Accept(LvTypeDom* visitor, Xt::DomElement* element);

public:
	virtual LarvaAttr* FindAttr(const char* attrName);
	virtual LarvaParam* FindParam(const char* attrName, const char* paramName);
};

// Xt::LtAtom

struct LtAtom : public LarvaType
{
	XtAtomType atomType;

public:
	LtAtom(XtAtomType atomType_);
	virtual ~LtAtom();

public:
	virtual bool Accept(LvType* visitor);
	virtual bool Accept(LvTypeDom* visitor, Xt::DomElement* element);
};

// Xt::LtEnum

struct LtEnum : public LarvaType
{
	XtUt::List<LarvaOption> options;

public:
	typedef XtUt::List<LarvaOption>::Iterator OptionIterator;

public:
	LtEnum();
	virtual ~LtEnum();

public:
	virtual bool Accept(LvType* visitor);
	virtual bool Accept(LvTypeDom* visitor, Xt::DomElement* element);
};

// Xt::LtFlags

struct LtFlags : public LarvaType
{
	XtUt::List<LarvaOption> options;

public:
	typedef XtUt::List<LarvaOption>::Iterator OptionIterator;

public:
	LtFlags();
	virtual ~LtFlags();

public:
	virtual bool Accept(LvType* visitor);
	virtual bool Accept(LvTypeDom* visitor, Xt::DomElement* element);
};

//
// Xt::LarvaFields
//

// Xt::LfStruct

struct LfStruct : public LarvaField
{
	LtStruct* type;

public:
	LfStruct(LtStruct* container_, LtStruct* type_);
	virtual ~LfStruct();

public:
	virtual bool Accept(LvField* visitor);
	virtual bool Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method);

public:
	virtual LarvaType* Type();
};

// Xt::LfInt

struct LfInt : public LarvaField
{
	LtAtom*		type;
	XtI64		radix;
	XtString	defValue;

public:
	LfInt(LtStruct* container_, LtAtom* type_);
	virtual ~LfInt();

public:
	virtual bool Accept(LvField* visitor);
	virtual bool Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method);

public:
	virtual LarvaType* Type();
};

// Xt::LfFloat

struct LfFloat : public LarvaField
{
	LtAtom* type;
	XtString defValue;

public:
	LfFloat(LtStruct* container_, LtAtom* type_);
	virtual ~LfFloat();

public:
	virtual bool Accept(LvField* visitor);
	virtual bool Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method);

public:
	virtual LarvaType* Type();
};

// Xt::LfString

struct LfString : public LarvaField
{
	LtAtom*		type;
	int			lines;
	XtString	defValue;

public:
	LfString(LtStruct* container_, LtAtom* type_);
	virtual ~LfString();

public:
	virtual bool Accept(LvField* visitor);
	virtual bool Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method);

public:
	virtual LarvaType* Type();
};

// Xt::LfWideString

struct LfWideString : public LarvaField
{
	LtAtom*		type;
	int			lines;
	XtString	defValue;

public:
	LfWideString(LtStruct* container_, LtAtom* type_);
	virtual ~LfWideString();

public:
	virtual bool Accept(LvField* visitor);
	virtual bool Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method);

public:
	virtual LarvaType* Type();
};

// Xt::LfBool

struct LfBool : public LarvaField
{
	LtAtom*		type;
	XtString	defValue;

public:
	LfBool(LtStruct* container_, LtAtom* type_);
	virtual ~LfBool();

public:
	virtual bool Accept(LvField* visitor);
	virtual bool Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method);

public:
	virtual LarvaType* Type();
};

// Xt::LfEnum

struct LfEnum : public LarvaField
{
	LtEnum*		type;
	XtString	defValue;

public:
	LfEnum(LtStruct* container_, LtEnum* type_);
	virtual ~LfEnum();

public:
	virtual bool Accept(LvField* visitor);
	virtual bool Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method);

public:
	virtual LarvaType* Type();
};

// Xt::LfFlags

struct LfFlags : public LarvaField
{
	LtFlags* type;

public:
	LfFlags(LtStruct* container_, LtFlags* type_);
	virtual ~LfFlags();

public:
	virtual bool Accept(LvField* visitor);
	virtual bool Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method);

public:
	virtual LarvaType* Type();
};

// Xt::LfStrictPtr

struct LfStrictPtr : public LarvaField
{
	LtStruct* type;

public:
	LfStrictPtr(LtStruct* container_, LtStruct* type_);
	virtual ~LfStrictPtr();

public:
	virtual bool Accept(LvField* visitor);
	virtual bool Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method);

public:
	virtual LarvaType* Type();
};

// Xt::LfPtr

struct LfPtr : public LarvaField
{
	LtStruct*	baseType;
	XtString	defValue;

public:
	LfPtr(LtStruct* container_, LtStruct* baseType_);
	virtual ~LfPtr();

public:
	virtual bool Accept(LvField* visitor);
	virtual bool Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method);

public:
	virtual LarvaType* Type();
};

// Xt::LfWeakPtr

struct LfWeakPtr : public LarvaField
{
	LtStruct* baseType;

public:
	LfWeakPtr(LtStruct* container_, LtStruct* baseType_);
	virtual ~LfWeakPtr();

public:
	virtual bool Accept(LvField* visitor);
	virtual bool Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method);

public:
	virtual LarvaType* Type();
};

// Xt::LfHandle

struct LfHandle : public LarvaField
{
	LtStruct* baseType;

public:
	LfHandle(LtStruct* container_, LtStruct* baseType_);
	virtual ~LfHandle();

public:
	virtual bool Accept(LvField* visitor);
	virtual bool Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method);

public:
	virtual LarvaType* Type();
};

// Xt::LfArray

struct LfArray : public LarvaField
{
	LarvaField* field;

public:
	LfArray(LtStruct* container_);
	virtual ~LfArray();

public:
	virtual bool Accept(LvField* visitor);
	virtual bool Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method);

public:
	virtual LarvaType* Type();
};

// Xt::LfLocString

struct LfLocString : public LarvaField
{
	LtAtom* type;

public:
	LfLocString(LtStruct* container_, LtAtom* type_);
	virtual ~LfLocString();

public:
	virtual bool Accept(LvField* visitor);
	virtual bool Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method);

public:
	virtual LarvaType* Type();
};

// Xt::LfControl

struct LfControl : public LarvaField
{
	XtString type;

public:
	LfControl(LtStruct* container_);
	virtual ~LfControl();

public:
	virtual bool Accept(LvField* visitor);
	virtual bool Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method);

public:
	virtual LarvaType* Type();
};

// Xt::FieldDom

namespace Xt
{
	struct FieldDom
	{
		LarvaField* field;
		DomElement* dom;

	public:
		FieldDom(LarvaField* field_ = 0, DomElement* dom_ = 0) :
			field(field_),
			dom(dom_)
		{
		}

	public:
		operator bool()
		{
			return field && dom;
		}
	};
}

// Xt::TypeComponent

namespace Xt
{
	struct TypeComponent
	{
		LtStruct*		type;
		DomComponent*	component;

	public:
		TypeComponent(LtStruct* type_ = 0, DomComponent* component_ = 0) :
			type(type_),
			component(component_)
		{
		}

	public:
		operator bool()
		{
			return type && component;
		}
	};
}

// Xt::FieldPath

namespace Xt
{
	struct FieldPath
	{
		LarvaField*	field;
		FieldPath*	next;

	public:
		FieldPath(LarvaField* field_ = 0, FieldPath* next_ = 0) :
			field(field_),
			next(next_)
		{
		}
	};

	bool Equal(FieldPath* lhs, FieldPath* rhs);

	FieldPath* Clone(FieldPath* path);
	void Free(FieldPath* path);

	XtString ToString(FieldPath* path);

	FieldDom At(LarvaSchema* schema, LtStruct* type, DomBlock* dom, FieldPath* path);
	FieldDom Acquire(LarvaSchema* schema, LtStruct* type, DomBlock* dom, FieldPath* path);
}

namespace Grimoire { struct Dom; }

class EstFormatter
{
public:
	EstFormatter();
	virtual ~EstFormatter();

public:
	virtual bool GetFormatByType(LtStruct* type, XtString& format);
	virtual bool GetFormatByOption(LarvaOption* option, XtString& format);

public:
	virtual void CacheComponentText(Xt::DomRecord* record, XtId id, const char* text);
	virtual bool FindComponentText(Xt::DomRecord* record, XtId id, XtString& text);
};

LarvaSchema* CreateLarvaSchema(Grimoire::Dom* grim, Xt::StatusCallback* status);

bool IsBaseOf(const LtStruct* base, const LtStruct* derived);

LarvaType* FindTypeByName(LarvaSchema* schema, const XtString& typeName);
LarvaField* FindFieldByName(LtStruct* type, const char* fieldName);
LarvaOption* FindOptionByName(LtEnum* enumType, const char* optionName);
LarvaOption* FindOptionByName(LtFlags* flagsType, const char* optionName);
LtStruct* FindCommonBase(LtStruct* lhs, LtStruct* rhs);

bool SetDefaults(LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component);

LtStruct* AsStruct(LarvaType* type);
LfStruct* AsStruct(LarvaField* field);

LtAtom* AsAtom(LarvaType* type);

LtEnum* AsEnum(LarvaType* type);
LfEnum* AsEnum(LarvaField* field);

LtFlags* AsFlags(LarvaType* type);
LfFlags* AsFlags(LarvaField* field);

namespace Xt
{
	XtId ParseId(const char* text);
	XtId ParseId(const char* start, const char* end);

	FieldDom GetByDottedPath(LtStruct* type, DomBlock* dom, const char* path);

	typedef XtUt::Dictionary<XtId, XtId> IdMap;
}

bool VisitMembers(LtStruct* type, Xt::DomBlock* dom, LvFieldDom* visitor, Xt::IndexMethod method);
bool VisitArrayItems(LfArray* field, Xt::DomArray* dom, LvFieldDom* visitor, Xt::IndexMethod method);

bool GenerateIdForComponent(Xt::DomComponent* component, bool force);
bool GenerateIdForComponentAndChildren(LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component, bool force);

// Generate string given a format string (of style "Free Text {fieldname} Free Text") and a root element

XtString EvaluateStructuredText(Xt::DomDb* db, LarvaSchema* schema, LtStruct* type, Xt::DomBlock* block, const char* format, EstFormatter* formatter);
XtString EvaluateStructuredText(Xt::DomDb* db, LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component, const char* format, EstFormatter* formatter);
XtString EvaluateStructuredText(Xt::DomDb* db, LarvaSchema* schema, LarvaField* field, Xt::DomElement* element, EstFormatter* formatter);

// Generate the text summary for an element

XtString EvaluateSummaryText(Xt::DomDb* db, LarvaSchema* schema, LtStruct* type, Xt::DomBlock* block, const XtString& defaultFormat);
XtString EvaluateSummaryText(Xt::DomDb* db, LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component, const XtString& defaultFormat);
XtString EvaluateSummaryText(Xt::DomDb* db, LarvaSchema* schema, LarvaField* field, Xt::DomElement* element);

// Remap all weak pointers for an element and its children

void MapWeakPtrs(LarvaSchema* schema, LtStruct* type, Xt::DomBlock* block, Xt::IdMap* map);
void MapWeakPtrs(LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component, Xt::IdMap* map);
void MapWeakPtrs(LarvaSchema* schema, LarvaField* field, Xt::DomElement* element, Xt::IdMap* map);

// Visit all the objects within the given element

bool VisitAllObjects(LarvaSchema* schema, LtStruct* type, Xt::DomBlock* block, LvObjects* visitor, bool useAcquire);
bool VisitAllObjects(LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component, LvObjects* visitor, bool useAcquire);
bool VisitAllObjects(LarvaSchema* schema, LarvaField* field, Xt::DomElement* element, LvObjects* visitor, bool useAcquire);

// Visit all the components within the given element

bool VisitAllComponents(LarvaSchema* schema, LtStruct* type, Xt::DomBlock* block, LvComponents* visitor);
bool VisitAllComponents(LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component, LvComponents* visitor);
bool VisitAllComponents(LarvaSchema* schema, LarvaField* field, Xt::DomElement* element, LvComponents* visitor);

// Find a component within a record with the given id

Xt::TypeComponent FindComponentById(LarvaSchema* schema, Xt::DomRecord* record, XtId id);

// Find a field and dom which has an attribute parameter equal to value

Xt::FieldDom FindBySemantic(LtStruct* type, Xt::DomBlock* block, const char* attr, const char* param, const char* value);
Xt::FieldDom AcquireBySemantic(LtStruct* type, Xt::DomBlock* block, const char* attr, const char* param, const char* value);

// Regenerate all component ids from this element down, optionally recording a map of old id -> new id

bool RegenerateComponentIds(LarvaSchema* schema, LtStruct* type, Xt::DomBlock* block, Xt::IdMap* map);
bool RegenerateComponentIds(LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component, Xt::IdMap* map);
bool RegenerateComponentIds(LarvaSchema* schema, LarvaField* field, Xt::DomElement* element, Xt::IdMap* map);

// Correctly duplicate any element, including remapping any internal cross references within child components

Xt::DomElement* Duplicate(LarvaSchema* schema, LtStruct* type, Xt::DomBlock* block);
Xt::DomElement* Duplicate(LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component);
Xt::DomElement* Duplicate(LarvaSchema* schema, LarvaField* field, Xt::DomElement* element);

struct EuuParams
{
	bool ensureGloballyUnique;

	int missingCount;
	int collisionCount;
	int totalCount;

	XtUt::Set<XtU64> globalIds;

public:
	EuuParams() :
		ensureGloballyUnique(false),
		missingCount(0),
		collisionCount(0),
		totalCount(0)
	{
	}
};

bool EnsureUniqueUids(LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component, EuuParams* params);

void RegenerateUniqueIds(LarvaSchema* schema, LtStruct* type, Xt::DomBlock* block);
void RegenerateUniqueIds(LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component);
void RegenerateUniqueIds(LarvaSchema* schema, LarvaField* field, Xt::DomElement* element);

#endif // XT_IMPL_LARVA_H