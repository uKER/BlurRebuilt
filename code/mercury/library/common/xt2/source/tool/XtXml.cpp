#include "Common/XtPch.h"
#include <Xt/XtTool.h>

/*
struct XmlToken
{
	const char* start;
	const char* end;
};

class XmlElementIterator
{
public:
	XmlElement Next();

private:
};

class XmlAttributeIterator
{
public:
	XmlAttribute Next();
};

class XmlAttribute
{
public:
	Sz8 Name();
	Sz8 Value();

private:
	XmlToken _name;
	XmlToken _value;
};

class XmlElement
{
public:
	Sz8 Name();

public:
	XmlAttributeIterator Attributes();
	XmlElementIterator Children();

private:
	XmlToken _name;
	XmlToken _attributes;
	XmlToken _children;
};
*/
#if 0
/*

	for each record
	{
		filter type "Particle"
		{
			move "Size" "Emitter.Size"
			move "Rate" "Emitter.Rate"
		}

		filter type "LevelActorList"
		{
			for each object
			{
				filter type "CoverActor"
				{
					set "LinkedCover" ""
				}
			}

			for each component
			{
				filter type "Vector"
				{
					move "x" "vx"
					move "y" "vy"
					move "z" "vz"
				}
			}
		}
	}

*/

namespace Xt
{
	namespace Translator
	{
		struct OperationContext
		{
			Xt::DomDb*		db;
			LarvaSchema*	schema;

		public:
			OperationContext() :
				db(0),
				schema(0)
			{
			}
		};

		// Operation

		class Operation
		{
		public:
			Operation();
			virtual ~Operation();

		public:
			virtual bool Execute(OperationContext* ctx, Xt::DomElement* dom, LarvaType* type);
		};

		// SequenceOp

		class SequenceOp : public Operation
		{
		public:
			SequenceOp();
			virtual ~SequenceOp();

		public:
			virtual bool Execute(OperationContext* ctx, Xt::DomElement* dom, LarvaType* type);

		public:
			bool Append(Operation* op);
			void Clear();

		private:
			XtUt::List<Operation> _operations;
		};

		// ForEachRecordOp

		class ForEachRecordOp : public SequenceOp
		{
		public:
			ForEachRecordOp();
			virtual ~ForEachRecordOp();

		public:
			virtual bool Execute(OperationContext* ctx, Xt::DomElement* dom, LarvaType* type);

		private:
			bool DoRecord(OperationContext* ctx, Xt::DomRecord* record);
			bool DoPack(OperationContext* ctx, Xt::DomPack* pack);
			bool DoDb(OperationContext* ctx, Xt::DomDb* db);
		};

		// ForEachObjectOp

		class ForEachObjectOp : public SequenceOp
		{
		public:
			ForEachObjectOp();
			virtual ~ForEachObjectOp();

		public:
			virtual bool Execute(OperationContext* ctx, Xt::DomElement* dom, LarvaType* type);
		};

		// ForEachComponentOp

		class ForEachComponentOp : public SequenceOp
		{
		public:
			ForEachComponentOp();
			virtual ~ForEachComponentOp();

		public:
			virtual bool Execute(OperationContext* ctx, Xt::DomElement* dom, LarvaType* type);
		};

		// FilterTypeOp

		class FilterTypeOp : public SequenceOp
		{
		public:
			FilterTypeOp(LarvaType* filterType);
			virtual ~FilterTypeOp();

		public:
			virtual bool Execute(OperationContext* ctx, Xt::DomElement* dom, LarvaType* type);

		private:
			LarvaType* _filterType;
		};

		// MoveOp

		class MoveOp : public Operation
		{
		public:
			MoveOp(const XtString& from, const XtString& to);
			virtual ~MoveOp();

		public:
			virtual bool Execute(OperationContext* ctx, Xt::DomElement* dom, LarvaType* type);

		private:
			XtString _from;
			XtString _to;
		};

		// CopyOp

		class CopyOp : public Operation
		{
		public:
			CopyOp(const XtString& from, const XtString& to);
			virtual ~CopyOp();

		public:
			virtual bool Execute(OperationContext* ctx, Xt::DomElement* dom, LarvaType* type);

		private:
			XtString _from;
			XtString _to;
		};

		// SetOp

		class SetOp : public Operation
		{
		public:
			SetOp(const XtString& path, XtUt::Ptr<Xt::DomElement> element);
			virtual ~SetOp();

		public:
			virtual bool Execute(OperationContext* ctx, Xt::DomElement* dom, LarvaType* type);

		private:
			XtString					_path;
			XtUt::Ptr<Xt::DomElement>	_element;
		};

		// RemoveOp

		class RemoveOp : public Operation
		{
		public:
			RemoveOp(const XtString& path);
			virtual ~RemoveOp();

		public:
			virtual bool Execute(OperationContext* ctx, Xt::DomElement* dom, LarvaType* type);

		private:
			XtString _path;
		};
	}
}

// Operation

Xt::Translator::Operation::Operation()
{
}

Xt::Translator::Operation::~Operation()
{
}

bool Xt::Translator::Operation::Execute(OperationContext* ctx, Xt::DomElement* dom, LarvaType* type)
{
	return true;
}

// SequenceOp

Xt::Translator::SequenceOp::SequenceOp()
{
}

Xt::Translator::SequenceOp::~SequenceOp()
{
}

bool Xt::Translator::SequenceOp::Execute(OperationContext* ctx, Xt::DomElement* dom, LarvaType* type)
{
	for(XtUt::List<Operation>::Iterator it(&_operations); it; ++it)
	{
		if (!it->Execute(ctx, dom, type))
			return false;
	}

	return true;
}

bool Xt::Translator::SequenceOp::Append(Operation* op)
{
	return XtUt::AppendEx(&_operations, op) != 0;
}

void Xt::Translator::SequenceOp::Clear()
{
	XtUt::Free(&_operations);
}

// ForEachRecordOp

Xt::Translator::ForEachRecordOp::ForEachRecordOp()
{
}

Xt::Translator::ForEachRecordOp::~ForEachRecordOp()
{
}

bool Xt::Translator::ForEachRecordOp::Execute(OperationContext* ctx, Xt::DomElement* dom, LarvaType* type)
{
	if (dom)
	{
		switch(dom->kind)
		{
			case Xt::kDomRecord:	return DoRecord(ctx, (Xt::DomRecord*)dom);
			case Xt::kDomPack:		return DoPack(ctx, (Xt::DomPack*)dom);
			case Xt::kDomDb:		return DoDb(ctx, (Xt::DomDb*)dom);
		}
	}

	return true;
}

bool Xt::Translator::ForEachRecordOp::DoRecord(OperationContext* ctx, Xt::DomRecord* record)
{
	return SequenceOp::Execute(ctx, &record->root, TypeOf(ctx->schema, record));
}

bool Xt::Translator::ForEachRecordOp::DoPack(OperationContext* ctx, Xt::DomPack* pack)
{
	for(Xt::DomPack::Iterator it(pack); it; ++it)
	{
		if (!DoRecord(ctx, it))
			return false;
	}

	return true;
}

bool Xt::Translator::ForEachRecordOp::DoDb(OperationContext* ctx, Xt::DomDb* db)
{
	for(Xt::DomDb::Iterator it(db); it; ++it)
	{
		if (!DoPack(ctx, it))
			return false;
	}

	return true;
}

// FilterTypeOp

Xt::Translator::FilterTypeOp::FilterTypeOp(LarvaType* filterType) :
	_filterType(filterType)
{
}

Xt::Translator::FilterTypeOp::~FilterTypeOp()
{
}

bool Xt::Translator::FilterTypeOp::Execute(OperationContext* ctx, Xt::DomElement* dom, LarvaType* type)
{
	if (!_filterType || !type)
		return true;

	bool skip = true;

	if (_filterType != type)
	{
		if ((_filterType->behaviour == kLtStruct) && (type->behaviour == kLtStruct))
			skip = !IsBaseOf((LtStruct*)_filterType, (LtStruct*)type);
	}
	else
		skip = false;

	return skip || SequenceOp::Execute(ctx, dom, type);
}

// MoveOp

Xt::Translator::MoveOp::MoveOp(const XtString& from, const XtString& to) :
	_from(from),
	_to(to)
{
}

Xt::Translator::MoveOp::~MoveOp()
{
}

bool Xt::Translator::MoveOp::Execute(OperationContext* ctx, Xt::DomElement* dom, LarvaType* type)
{
	Xt::DomElement* from = FindElementByPath(dom, _from);

	if (!from)
		return true;

	if ((from->kind != Xt::kDomField) && (from->kind != Xt::kDomBlock) && (from->kind != Xt::kDomArray))
		return true;

	XtUt::Ptr<Xt::DomElement> cloned = Clone(from);

	if (!cloned)
	{
		DebugLn("OOM!");
		return false;
	}

	Xt::DomElement* to = AcquireElementByPath(dom, _to, from->kind); // Need a MakeIndexChainFromPath function or something...

	if (!to || (to->kind != from->kind)) // TODO: can't touch from->kind as acquire might nuke it
		return false;

	switch(from->kind)
	{
		case Xt::kDomField: if (!Clone((Xt::DomField*)to, (Xt::DomField*)&*cloned)) return false; break;
		case Xt::kDomBlock: if (!Clone((Xt::DomBlock*)to, (Xt::DomBlock*)&*cloned)) return false; break;
		case Xt::kDomArray: if (!Clone((Xt::DomArray*)to, (Xt::DomArray*)&*cloned)) return false; break;
	}

	//from->parent
	delete from;

	return true;
}

// CopyOp

Xt::Translator::CopyOp::CopyOp(const XtString& from, const XtString& to) :
	_from(from),
	_to(to)
{
}

Xt::Translator::CopyOp::~CopyOp()
{
}

bool Xt::Translator::CopyOp::Execute(OperationContext* ctx, Xt::DomElement* dom, LarvaType* type)
{
	return true;
}

// SetOp

Xt::Translator::SetOp::SetOp(const XtString& path, XtUt::Ptr<Xt::DomElement> element) :
	_path(path),
	_element(element)
{
}

Xt::Translator::SetOp::~SetOp()
{
}

bool Xt::Translator::SetOp::Execute(OperationContext* ctx, Xt::DomElement* dom, LarvaType* type)
{
	return true;
}

// RemoveOp

Xt::Translator::RemoveOp::RemoveOp(const XtString& path) :
	_path(path)
{
}

Xt::Translator::RemoveOp::~RemoveOp()
{
}

bool Xt::Translator::RemoveOp::Execute(OperationContext* ctx, Xt::DomElement* dom, LarvaType* type)
{
	return true;
}

void TransTest(Xt::DomDb* db)
{
	Xt::Translator::OperationContext ctx;

	ctx.db		= db;
	ctx.schema	= db->schema;

	Xt::Translator::SequenceOp seq;

	Xt::Translator::ForEachRecordOp* fer = new Xt::Translator::ForEachRecordOp;
	Xt::Translator::FilterTypeOp* fil = new Xt::Translator::FilterTypeOp(Xt::TypeByName(ctx.schema, "HideTestRecord"));

	seq.Append(fer);
	fer->Append(fil);
	fil->Append(new Xt::Translator::MoveOp("a/aMember", "a/renamed"));

	seq.Execute(&ctx, db, 0);
}

/*
XtTranslate

XtTrans_




enum XmlTokenKind
{
	kXmlToken_Null,
	kXmlToken_OpenDirective,
	kXmlToken_CloseDirective,
	kXmlToken_OpenElement,
	kXmlToken_CloseElement,
	kXmlToken_Text
};

struct XmlToken
{
	XmlTokenKind	kind;
	const char*		start;
	const char*		end;

public:
	inline XmlToken() :
		kind(kXmlToken_Null),
		start(0),
		end(0)
	{
	}
};

struct XmlContext
{
	const char*	start;
	const char*	end;
	const char*	ptr;

	bool		pushed;
	XmlToken	token;
};

void XmlParser_SkipWhitespace(XmlContext* ctx)
{
	for(; ctx->ptr < ctx->end; ctx->ptr++)
	{
		switch(*ctx->ptr)
		{
			case '\r':
			case '\n':
			case '\t':
			case ' ':
			continue;
		}

		break;
	}
}

XmlToken XmlParser(XmlContext* ctx)
{
	if (ctx->pushed)
	{
		ctx->pushed = false;
		return ctx->token;
	}

	while(ctx->ptr < ctx->end)
	{
	}

	return XmlToken();
}

class XmlParser
{
public:
	XmlToken

};

void ReadTranslator(XmlReader& r)
{
	r.Open();

	while(XmlElement e = r.Next())
	{
		if (e.Name() == "XtTranslator")
		{
			e.Open();


			e.Close();
		}
	}

	r.Close();
}*/
#endif