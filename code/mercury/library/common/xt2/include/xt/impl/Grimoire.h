#ifndef XT_TOOL_GRIMOIRE_H
#define XT_TOOL_GRIMOIRE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Grimoire
{
	// Symbol

	enum Symbol
	{
		kSymEof		= -2,
		kSymInvalid	= -1,
		kSymNone	= 0,

		// Delimiters

		kSymOpenBracket,	// [
		kSymCloseBracket,	// ]
		kSymOpenParen,		// (
		kSymCloseParen,		// )
		kSymOpenBrace,		// {
		kSymCloseBrace,		// }
		kSymComma,			// ,
		kSymColon,			// :
		kSymSemicolon,		// ;

		// Constants

		kSymLiteralInteger,	// 123
		kSymLiteralFloat,	// 123.456
		kSymLiteralString,	// "string"

		// Identifiers

		kSymIdentifier,		// abcde

		// Access operators

		kSymDot,			// .

		// Boolean logic

		kSymNot,			// !
		kSymOr,				// ||
		kSymAnd,			// &&

		// Comparison

		kSymEqual,			// ==
		kSymNotEqual,		// !=
		kSymLess,			// <
		kSymGreater,		// >
		kSymLessEqual,		// <=
		kSymGreaterEqual,	// >=

		// Maths

		kSymAssign,			// =
		kSymPlus,			// +
		kSymMinus,			// -
		kSymMultiply,		// *
		kSymDivide,			// /
		kSymModulo,			// %

		// Qualifiers

		kSymRef,			// &

		// Special

		kSymAttributes
	};

	// Token

	struct Token
	{
		Symbol		symbol;
		const char*	start;
		const char*	end;
		int			line;

	public:
		Token() :
			symbol(kSymInvalid),
			start(0),
			end(0),
			line(0)
		{
		}
	};

	bool operator==(const Token& lhs, const char* rhs);
	bool operator==(const Token& lhs, const Token& rhs);

	inline bool operator!=(const Token& lhs, const char* rhs)
	{
		return !(lhs == rhs);
	}

	inline bool operator!=(const Token& lhs, const Token& rhs)
	{
		return !(lhs == rhs);
	}

	// Document

	struct Document
	{
		XtString	filename;
		XtString	path;
		const char*	document;
		int			length;

	public:
		Document() :
			document(0),
			length(0)
		{
		}

		~Document()
		{
			delete [] document;
		}
	};

	// Expression

	struct Expression
	{
		Token		token;
		Expression*	left;
		Expression*	right;

	public:
		Expression() :
			left(0),
			right(0)
		{
		}

		virtual ~Expression()
		{
			delete left;
			delete right;
		}
	};

	struct Attribute;

	struct ExpAttributes : public Expression
	{
		XtUt::List<Attribute> attributes;

	public:
		ExpAttributes()
		{
		}

		virtual ~ExpAttributes()
		{
		}
	};

	// Parameter

	struct Parameter
	{
		Expression* expression;

	public:
		Parameter() :
			expression(0)
		{
		}

		~Parameter()
		{
			delete expression;
		}
	};

	// Attribute

	struct Attribute
	{
		Token					name;
		XtUt::List<Parameter>	parameters;
	};

	// Member

	enum MemberBehaviour
	{
		kMbField,
		kMbControl,
		kMbLabel
	};

	struct Member
	{
		MemberBehaviour			behaviour;
		Token					type;
		Token					name;
		XtUt::List<Attribute>	attributes;

	public:
		Member(MemberBehaviour behaviour_) :
			behaviour(behaviour_)
		{
		}
	};

	// BaseType

	struct BaseType
	{
		Token					type;
		XtUt::List<Attribute>	attributes;
	};

	// Type

	enum TypeBehaviour
	{
		kTbStruct,
		kTbAtom,
		kTbEnum,
		kTbFlags
	};

	struct Type
	{
		TypeBehaviour			behaviour;
		Token					name;
		BaseType*				base;
		XtUt::List<Member>		members;
		XtUt::List<Attribute>	attributes;
		Document*				document;

	public:
		Type(TypeBehaviour behaviour_) :
			behaviour(behaviour_),
			base(0),
			document(0)
		{
		}

		~Type()
		{
			delete base;
		}
	};

	// Dom

	struct Dom
	{
		XtString				path;
		XtUt::List<Type>		types;
		XtUt::List<Document>	documents;
	};

	// Helpers

	Attribute* FindAttributeByName(XtUt::List<Attribute>* attributes, const char* name);
	Attribute* FindAttributeByName(Member* member, const char* name);
	Attribute* FindAttributeByName(Type* type, const char* name);

	// Variant

	enum VariantType
	{
		kVariantNone,
		kVariantI64,
		kVariantF64,
		kVariantSz8,
		kVariantAtom,
		kVariantAttr
	};

	struct Atom
	{
		XtString name;

	public:
		Atom(const XtString& name_ = "") :
			name(name_)
		{
		}

	public:
		operator bool() const
		{
			return *name != '\0';
		}
	};

	extern const Atom kXtNullAtom;

	class Variant
	{
	public:
		Variant() :
			_type(kVariantNone)
		{
		}

		Variant(const Variant& rhs) :
			_type(kVariantNone)
		{
			Set(rhs);
		}

		template<typename Type>
		explicit Variant(const Type& rhs) :
			_type(kVariantNone)
		{
			Set(rhs);
		}

		~Variant()
		{
			Clear();
		}

	public:
		Variant& operator=(const Variant& rhs)
		{
			Set(rhs);
			return *this;
		}

		template<typename Type>
		Variant& operator=(const Type& rhs)
		{
			Set(rhs);
			return *this;
		}

	public:
		void Set(const Variant& v);
		void Set(XtI64 i64);
		void Set(XtF64 f64);
		void Set(const XtString& sz8);
		void Set(const Atom& atom);
		void Set(ExpAttributes* attr);
		void Clear();

	public:
		VariantType Type() const
		{
			return _type;
		}

	public:
		XtI64 AsI64() const				{ return (_type == kVariantI64) ? Data()->i64 : 0; }
		XtF64 AsF64() const				{ return (_type == kVariantF64) ? Data()->f64 : 0.0; }
		const XtString& AsSz8() const	{ return (_type == kVariantSz8) ? Data()->sz8 : kXtNullString; }
		const Atom& AsAtom() const		{ return (_type == kVariantAtom) ? Data()->atom : kXtNullAtom; }
		ExpAttributes* AsAttr() const	{ return (_type == kVariantAttr) ? Data()->attr : 0; }

	private:
		union VariantData
		{
			XtI64				i64;
			XtF64				f64;
			struct { XtString	sz8;	};
			struct { Atom		atom;	};
			ExpAttributes*		attr;
		};

	private:
		VariantData*		Data()			{ return (VariantData*)_data; }
		const VariantData*	Data() const	{ return (VariantData*)_data; }

	private:
		VariantType	_type;
		XtU8		_data[sizeof(VariantData)];
	};

	XtI64 ToI64(const Variant& v);
	XtF64 ToF64(const Variant& v);
	XtString ToSz8(const Variant& v);
	Atom ToAtom(const Variant& v);
	ExpAttributes* ToAttr(const Variant& v);

	// Evaluate

	Variant Resolve(const Variant& v);

	VariantType DetermineExpressionResult(const Variant& lhs, const Variant& rhs);

	Variant Add(const Variant& lhs, const Variant& rhs);
	Variant Sub(const Variant& lhs, const Variant& rhs);
	Variant Mul(const Variant& lhs, const Variant& rhs);
	Variant Div(const Variant& lhs, const Variant& rhs);
	Variant Mod(const Variant& lhs, const Variant& rhs);

	Variant EvaluateWithoutResolve(Expression* expr);
	Variant Evaluate(Expression* expr);
}

bool GrimoireParse(const char* document, int length, Grimoire::Dom* dom, const char* filename, Grimoire::Document* grimDoc, Xt::StatusCallback* status);
bool GrimoireImport(const char* filename, Grimoire::Dom* dom, Xt::StatusCallback* status);

bool TokenToString(XtString* str, const Grimoire::Token* token);

XtString GetParameterString(XtUt::List<Grimoire::Attribute>* attrs, const char* attributeName, int parameterIndex);
XtI64 GetParameterInt(XtUt::List<Grimoire::Attribute>* attrs, const char* attributeName, int parameterIndex);
double GetParameterFloat(XtUt::List<Grimoire::Attribute>* attrs, const char* attributeName, int parameterIndex);

#endif // XT_TOOL_GRIMOIRE_H