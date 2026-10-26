#ifndef XT_IMPL_XTDOMHELPERS_H
#define XT_IMPL_XTDOMHELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Xt
{

	//
	// Xt::Meta
	//

	enum MetaType
	{
		kMetaElement,
		kMetaId,
		kMetaString,
		kMetaIndex
	};

	struct MetaElement;
	struct MetaId;
	struct MetaString;
	struct MetaIndex;

	struct Meta
	{
		MetaType type;

	public:
		Meta(MetaType type_) : type(type_) { }
		virtual ~Meta() { }

	public:
		virtual MetaElement*	AsElement()	{ return 0; }
		virtual MetaId*			AsId()		{ return 0; }
		virtual MetaString*		AsString()	{ return 0; }
		virtual MetaIndex*		AsIndex()	{ return 0; }
	};

	//
	// Xt::MetaElement
	//

	struct MetaElement : public Meta
	{
		XtUt::Ptr<DomElement> element;

	public:
		MetaElement(DomElement* element_) :
			Meta(kMetaElement),
			element(element_)
		{
		}

	public:
		virtual MetaElement* AsElement() { return this; }
	};

	//
	// Xt::MetaId
	//

	struct MetaId : public Meta
	{
		XtId value;

	public:
		MetaId(XtId value_) :
			Meta(kMetaId),
			value(value_)
		{
		}

	public:
		virtual MetaId* AsId() { return this; }
	};

	//
	// Xt::MetaString
	//

	struct MetaString : public Meta
	{
		const XtString& value;

	public:
		MetaString(const XtString& value_) :
			Meta(kMetaString),
			value(value_)
		{
		}

	public:
		virtual MetaString* AsString() { return this; }
	};

	//
	// Xt::MetaIndex
	//

	struct MetaIndex : public Meta
	{
		const XtUt::Array<XtId>* order;

	public:
		MetaIndex(const XtUt::Array<XtId>* order_) :
			Meta(kMetaIndex),
			order(order_)
		{
		}

	public:
		virtual MetaIndex* AsIndex() { return this; }
	};

	//
	// Xt::Indexer
	//

	struct Indexer
	{
		virtual ~Indexer() { }

	public:
		virtual Indexer* Clone();

	public:
		virtual DomElement* At(DomElement* parent);

		virtual DomField* AcquireField(DomElement* parent);
		virtual DomRef* AcquireRef(DomElement* parent);
		virtual DomBlock* AcquireBlock(DomElement* parent);
		virtual DomArray* AcquireArray(DomElement* parent);
		virtual DomComponent* AcquireComponent(DomElement* parent);
		virtual DomRecord* AcquireRecord(DomElement* parent);
		virtual DomPack* AcquirePack(DomElement* parent);

		virtual bool Set(DomElement* parent, Meta* meta);

		virtual bool Remove(DomElement* parent);

	public:
		DomElement* Acquire(DomElement* parent, DomKind kind);
	};

	//
	// Xt::NullIndexer
	//

	struct NullIndexer : public Indexer
	{
	public:
		NullIndexer() { }
		virtual ~NullIndexer() { }

	public:
		virtual NullIndexer* Clone();

	public:
		virtual DomElement* At(DomElement* parent);

		virtual DomField* AcquireField(DomElement* parent);
		virtual DomRef* AcquireRef(DomElement* parent);
		virtual DomBlock* AcquireBlock(DomElement* parent);
		virtual DomArray* AcquireArray(DomElement* parent);
		virtual DomComponent* AcquireComponent(DomElement* parent);
		virtual DomRecord* AcquireRecord(DomElement* parent);
		virtual DomPack* AcquirePack(DomElement* parent);
	};

	//
	// Xt::BlockIndexer
	//

	struct BlockIndexer : public Indexer
	{
		const XtString& name;

	public:
		BlockIndexer(const XtString& name_) : name(name_) { }
		virtual ~BlockIndexer() { }

	public:
		virtual BlockIndexer* Clone();

	public:
		virtual DomElement* At(DomElement* parent);

		virtual DomField* AcquireField(DomElement* parent);
		virtual DomRef* AcquireRef(DomElement* parent);
		virtual DomBlock* AcquireBlock(DomElement* parent);
		virtual DomArray* AcquireArray(DomElement* parent);
		virtual DomComponent* AcquireComponent(DomElement* parent);

		virtual bool Set(DomElement* parent, Meta* meta);

		virtual bool Remove(DomElement* parent);
	};

	//
	// Xt::ArrayIndexer
	//

	struct ArrayIndexer : public Indexer
	{
		enum Method
		{
			kIndex,
			kElement
		};

	public:
		Method	method;
		XtId	id;

	public:
		ArrayIndexer(Method method_) : method(method_), id(0) { }
		ArrayIndexer(XtId id_) : method(kElement), id(id_) { }
		virtual ~ArrayIndexer() { }

	public:
		virtual ArrayIndexer* Clone();

	public:
		virtual DomElement* At(DomElement* parent);

		virtual DomField* AcquireField(DomElement* parent);
		virtual DomRef* AcquireRef(DomElement* parent);
		virtual DomBlock* AcquireBlock(DomElement* parent);
		virtual DomArray* AcquireArray(DomElement* parent);
		virtual DomComponent* AcquireComponent(DomElement* parent);

		virtual bool Set(DomElement* parent, Meta* meta);

		virtual bool Remove(DomElement* parent);
	};

	//
	// Xt::ComponentIndexer
	//

	struct ComponentIndexer : public Indexer
	{
		enum Method
		{
			kId,
			kType,
			kData
		};

	public:
		Method method;

	public:
		ComponentIndexer(Method method_) : method(method_) { }
		virtual ~ComponentIndexer() { }

	public:
		virtual ComponentIndexer* Clone();

	public:
		virtual DomBlock* AcquireBlock(DomElement* parent);
		virtual bool Set(DomElement* parent, Meta* meta);
		virtual bool Remove(DomElement* parent);
	};

	//
	// Xt::RecordIndexer
	//

	struct RecordIndexer : public Indexer
	{
		enum Method
		{
			kName,
			kType,
			kRoot
		};

	public:
		Method method;

	public:
		RecordIndexer(Method method_) : method(method_) { }
		virtual ~RecordIndexer() { }

	public:
		virtual RecordIndexer* Clone();

	public:
		virtual DomComponent* AcquireComponent(DomElement* parent);
		virtual bool Set(DomElement* parent, Meta* meta);
		virtual bool Remove(DomElement* parent);
	};

	//
	// Xt::PackIndexer
	//

	struct PackIndexer : public Indexer
	{
		enum Method
		{
			kName,
			kProperties,
			kRecord
		};

	public:
		Method	method;
		XtId	id;

	public:
		PackIndexer(Method method_) : method(method_), id(0) { }
		PackIndexer(XtId id_) : method(kRecord), id(id_) { }
		virtual ~PackIndexer() { }

	public:
		virtual PackIndexer* Clone();

	public:
		virtual DomElement* At(DomElement* parent);
		virtual DomBlock* AcquireBlock(DomElement* parent);
		virtual DomRecord* AcquireRecord(DomElement* parent);
		virtual bool Set(DomElement* parent, Meta* meta);
		virtual bool Remove(DomElement* parent);
	};

	//
	// Xt::DomElement helpers
	//

	int Compare(DomElement* lhs, DomElement* rhs);

	//
	// Xt Dotted Path
	//

	bool SetByDottedPath(DomBlock* block, const char* path, Xt::DomElement* element);
	bool SetByDottedPath(DomBlock* block, const char* path, const XtString& value);
	DomElement* GetByDottedPath(DomBlock* block, const char* path);

	//
	// XtLocTable helpers
	//

	DomBlock* FindLocEntryById(DomRecord* locTable, XtId id);
	XtId FindLocIdByName(DomRecord* locTable, const XtString& name);
	DomBlock* FindLocEntryByName(DomRecord* locTable, const XtString& name);
	DomBlock* CreateNewLocEntry(DomRecord* locTable, XtId withExplicitId = 0);
	DomBlock* AcquireLocEntryByName(DomRecord* locTable, const XtString& name);
	bool RemoveLocEntryById(DomRecord* locTable, XtId id);

	//
	// Schema<->Dom helpers
	//

	enum IndexMethod
	{
		kIndexSelf,		// Parent will be treated as the element without indexing
		kIndexAt,		// Parent will be indexed using At*
		kIndexAcquire	// parent will be indexed using Acquire*
	};

	LtStruct* TypeOf(LarvaSchema* schema, DomRecord* record);
	LtStruct* TypeOf(LarvaSchema* schema, LtStruct* baseType, DomComponent* component);
	LarvaType* TypeOf(LarvaSchema* schema, LarvaField* field, DomElement* element);

	DomElement* At(DomElement* parent, LarvaField* field);

	DomField* AcquireField(DomElement* parent, LarvaField* field);
	DomRef* AcquireRef(DomElement* parent, LarvaField* field);
	DomBlock* AcquireBlock(DomElement* parent, LarvaField* field);
	DomArray* AcquireArray(DomElement* parent, LarvaField* field);
	DomComponent* AcquireComponent(DomElement* parent, LarvaField* field);

	DomField* AcquireField(DomElement* parent, LarvaField* field, IndexMethod method);
	DomRef* AcquireRef(DomElement* parent, LarvaField* field, IndexMethod method);
	DomBlock* AcquireBlock(DomElement* parent, LarvaField* field, IndexMethod method);
	DomArray* AcquireArray(DomElement* parent, LarvaField* field, IndexMethod method);
	DomComponent* AcquireComponent(DomElement* parent, LarvaField* field, IndexMethod method);

	DomElement* Acquire(DomElement* parent, LarvaField* field);

	//
	// Query
	//

	class Query;

	struct QueryResult
	{
		LarvaField*	field;
		LtStruct*	type;
		DomElement*	dom;
		XtString	path;

	public:
		QueryResult(LarvaField* field_ = 0, LtStruct* type_ = 0, DomElement* dom_ = 0, const XtString& path_ = kXtNullString) :
			field(field_),
			type(type_),
			dom(dom_),
			path(path_)
		{
		}
	};

	struct QueryContext
	{
		Query*			query;
		Xt::DomDb*		db;
		LarvaSchema*	schema;

	public:
		QueryContext();
	};

	class QueryElement
	{
	public:
		QueryElement();
		virtual ~QueryElement();

	public:
		virtual void Process(QueryContext* context, const QueryResult& source);

	public:
		void Result(QueryContext* context, const QueryResult& result);

	private:
		Query*	_query;
		int		_index;

	friend class Query;
	};

	class Query
	{
	public:
		Query();
		~Query();

	public:
		void Clear();
		QueryElement* AddElement(QueryElement* el);

	public:
		void Execute(Xt::DomDb* db);

	private:
		XtUt::List<QueryElement> _elements;

	friend class QueryElement;
	};

	class QeAllRecords : public QueryElement
	{
	public:
		virtual void Process(QueryContext* context, const QueryResult& source);
	};

	class QeByRecordType : public QueryElement
	{
	public:
		QeByRecordType(LtStruct* type);

	public:
		virtual void Process(QueryContext* context, const QueryResult& source);

	private:
		LtStruct* _type;
	};

	class QeByObjectType : public QueryElement
	{
	public:
		QeByObjectType(LtStruct* type);

	public:
		virtual void Process(QueryContext* context, const QueryResult& source);

	private:
		void DoObject(QueryContext* context, LtStruct* type, Xt::DomBlock* dom);

	private:
		LtStruct* _type;

	friend class QeBotObjects;
	};

	class QeByRecordName : public QueryElement
	{
	public:
		QeByRecordName(const XtString& name);

	public:
		virtual void Process(QueryContext* context, const QueryResult& source);

	private:
		XtString _name;
	};

	class QeDependsOnRecord : public QueryElement
	{
	public:
		QeDependsOnRecord(XtId recordId);

	public:
		virtual void Process(QueryContext* context, const QueryResult& source);

	private:
		XtId _recordId;
	};

	enum FieldExpressionCmp
	{
		kFecNone,
		kFecEqual,
		kFecNotEqual,
		kFecLess,
		kFecLessEqual,
		kFecGreater,
		kFecGreaterEqual,
		kFecBetween,
		kFecIsTrue,
		kFecIsFalse,
		kFecContains,
		kFecDoesNotContain,
		kFecStartsWith,
		kFecEndsWith,
		kFecIsNull,
		kFecIsNotNull,
		kFecDisplay
	};

	class QeFeObjects;
	struct FieldPath;

	class QeFieldExpression : public QueryElement
	{
	public:
		QeFieldExpression(LtStruct* type, FieldPath* path, FieldExpressionCmp cmp, const XtString& low, const XtString& high);
		virtual ~QeFieldExpression();

	public:
		virtual void Process(QueryContext* context, const QueryResult& source);

	protected:
		void DoObject(QueryContext* context, LtStruct* type, Xt::DomBlock* dom);

	private:
		LtStruct*			_type;
		FieldPath*			_path;
		FieldExpressionCmp	_cmp;
		XtString			_low;
		XtString			_high;

	friend class QeFeObjects;
	};

	class QeUsingLocString : public QueryElement
	{
	public:
		QeUsingLocString(XtId locId);

	public:
		virtual void Process(QueryContext* context, const QueryResult& source);

	private:
		void DoObject(QueryContext* context, LtStruct* type, Xt::DomBlock* dom);

	private:
		XtId _locId;

	friend class QeUlsObjects;
	};

}

#endif // XT_IMPL_XTDOMHELPERS_H