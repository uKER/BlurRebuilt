#include "Common/XtPch.h"
#include "Xt/XtTool.h"
#include "Xt/FindDependencies.h"

namespace Xt
{

	XtString QeFeLamePath(Xt::DomElement* element)
	{
		if (!element || !CanCompose(element->kind) || !element->parent)
			return kXtNullString;

		if (Xt::DomArray* array = AsArray(element->parent))
			return QeFeLamePath(element->parent) + Format("#%i/", IndexOf(array, element->parentId));

		if (element->parent->kind == kDomComponent)
			return QeFeLamePath(element->parent);

		return QeFeLamePath(element->parent) + element->parentKey + "/";
	}

	//
	// Xt::QueryResult
	//

	QueryContext::QueryContext() :
		query(0),
		db(0),
		schema(0)
	{
	}

	//
	// Xt::QueryElement
	//

	QueryElement::QueryElement() :
		_query(0),
		_index(0)
	{
	}

	QueryElement::~QueryElement()
	{
	}

	void QueryElement::Process(QueryContext* context, const QueryResult& source)
	{
	}

	void QueryElement::Result(QueryContext* context, const QueryResult& result)
	{
		if (QueryElement* nextElement = At(&_query->_elements, _index + 1))
			nextElement->Process(context, result);
	}

	//
	// Xt::Query
	//

	Query::Query()
	{
	}

	Query::~Query()
	{
	}

	void Query::Clear()
	{
		Free(&_elements);
	}

	QueryElement* Query::AddElement(QueryElement* el)
	{
		if (!XtUt::AppendEx(&_elements, el))
			return 0;

		el->_query = this;
		el->_index = Count(&_elements) - 1;

		return el;
	}

	void Query::Execute(Xt::DomDb* db)
	{
		if (!db)
			return;

		QueryContext ctx;

		ctx.query	= this;
		ctx.db		= db;
		ctx.schema	= db->schema;

		if (QueryElement* el = At(&_elements, 0))
			el->Process(&ctx, QueryResult());
	}

	//
	// Xt::QeAllRecords
	//

	void QeAllRecords::Process(QueryContext* context, const QueryResult& source)
	{
		for(Xt::DomDb::Iterator itPack(context->db); itPack; ++itPack)
		{
			for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
			{
				Result(context, QueryResult(0, 0, itRecord, "/"));
			}
		}
	}

	//
	// Xt::QeByRecordType
	//

	QeByRecordType::QeByRecordType(LtStruct* type) :
		_type(type)
	{
	}

	void QeByRecordType::Process(QueryContext* context, const QueryResult& source)
	{
		if (DomRecord* record = RecordOf(source.dom))
		{
			if (IsBaseOf(_type, TypeOf(context->schema, record)))
				Result(context, source);
		}
	}

	//
	// Xt::QeByObjectType
	//

	class QeBotObjects : public LvObjects
	{
		QeByObjectType*	_bot;
		QueryContext*	_context;

	public:
		QeBotObjects(QeByObjectType* bot, QueryContext* context) :
			_bot(bot),
			_context(context)
		{
		}

	public:
		virtual bool Visit(LtStruct* type, Xt::DomBlock* dom)
		{
			_bot->DoObject(_context, type, dom);

			return true;
		}
	};

	QeByObjectType::QeByObjectType(LtStruct* type) :
		_type(type)
	{
	}

	void QeByObjectType::Process(QueryContext* context, const QueryResult& source)
	{
		if (Xt::DomRecord* record = AsRecord(source.dom))
		{
			VisitAllObjects(context->schema, TypeOf(context->schema, record), &record->root, &QeBotObjects(this, context), true);
			return;
		}

		if (source.field)
		{
			switch(source.field->kind)
			{
				case kLfStruct:
					DoObject(context, ((LfStruct*)source.field)->type, AsBlock(source.dom));
				break;

				case kLfStrictPtr:
				{
					if (DomComponent* com = AsComponent(source.dom))
						DoObject(context, ((LfStrictPtr*)source.field)->type, &com->data);
				}
				break;

				case kLfPtr:
				{
					LfPtr*			ptr = (LfPtr*)source.field;
					DomComponent*	com = AsComponent(source.dom);

					if (LtStruct* type = TypeOf(context->schema, ptr->baseType, com))
						DoObject(context, type, &com->data);
				}
				break;
			}
		}

		if (source.type)
			DoObject(context, source.type, AsBlock(source.dom));
	}

	void QeByObjectType::DoObject(QueryContext* context, LtStruct* type, Xt::DomBlock* dom)
	{
		if (!context || !type || !dom)
			return;

		if (!IsBaseOf(_type, type))
			return;

		Result(context, QueryResult(0, type, dom, QeFeLamePath(dom)));
	}

	//
	// Xt::QeByRecordName
	//

	QeByRecordName::QeByRecordName(const XtString& name) :
		_name(name)
	{
	}

	void QeByRecordName::Process(QueryContext* context, const QueryResult& source)
	{
		if (DomRecord* record = RecordOf(source.dom))
		{
			if (IndexOfStringI<char>(record->name, _name) >= 0)
				Result(context, source);
		}
	}

	//
	// Xt::QeDependsOnRecord
	//

	QeDependsOnRecord::QeDependsOnRecord(XtId recordId) :
		_recordId(recordId)
	{
	}

	void QeDependsOnRecord::Process(QueryContext* context, const QueryResult& source)
	{
		if (DomRecord* record = RecordOf(source.dom))
		{
			FindDependencies(context->schema, _recordId, record, this, context);
		}
	}

	//
	// Xt::QeFieldExpression
	//

	class QeFeObjects : public LvObjects
	{
		QeFieldExpression*	_expr;
		QueryContext*		_context;

	public:
		QeFeObjects(QeFieldExpression* expr, QueryContext* context) :
			_expr(expr),
			_context(context)
		{
		}

	public:
		virtual bool Visit(LtStruct* type, Xt::DomBlock* dom)
		{
			_expr->DoObject(_context, type, dom);

			return true;
		}
	};

	QeFieldExpression::QeFieldExpression(LtStruct* type, FieldPath* path, FieldExpressionCmp cmp, const XtString& low, const XtString& high) :
		_type(type),
		_path(Clone(path)),
		_cmp(cmp),
		_low(low),
		_high(high)
	{
	}

	QeFieldExpression::~QeFieldExpression()
	{
		Free(_path);
	}

	void QeFieldExpression::Process(QueryContext* context, const QueryResult& source)
	{
		if (Xt::DomRecord* record = AsRecord(source.dom))
		{
			VisitAllObjects(context->schema, TypeOf(context->schema, record), &record->root, &QeFeObjects(this, context), true);
			return;
		}

		if (source.field)
		{
			switch(source.field->kind)
			{
				case kLfStruct:
					DoObject(context, ((LfStruct*)source.field)->type, AsBlock(source.dom));
				break;

				case kLfStrictPtr:
				{
					if (DomComponent* com = AsComponent(source.dom))
						DoObject(context, ((LfStrictPtr*)source.field)->type, &com->data);
				}
				break;

				case kLfPtr:
				{
					LfPtr*			ptr = (LfPtr*)source.field;
					DomComponent*	com = AsComponent(source.dom);

					if (LtStruct* type = TypeOf(context->schema, ptr->baseType, com))
						DoObject(context, type, &com->data);
				}
				break;
			}
		}

		if (source.type)
			DoObject(context, source.type, AsBlock(source.dom));
	}

	void QeFieldExpression::DoObject(QueryContext* context, LtStruct* type, Xt::DomBlock* dom)
	{
		if (!context || !type || !dom)
			return;

		if (!IsBaseOf(_type, type))
			return;

		FieldDom fd = At(context->schema, type, dom, _path);

		if (!fd)
			return;

		bool passed = false;

		switch(fd.field->kind)
		{
			case kLfInt:
			{
				XtI64 value	= _atoi64(AsText(fd.dom));
				XtI64 low	= _atoi64(_low);

				switch(_cmp)
				{
					case Xt::kFecBetween:		passed = (value >= low) && (value <= _atoi64(_high));	break;
					case Xt::kFecEqual:			passed = value == low;	break;
					case Xt::kFecNotEqual:		passed = value != low;	break;
					case Xt::kFecLess:			passed = value < low;	break;
					case Xt::kFecLessEqual:		passed = value <= low;	break;
					case Xt::kFecGreater:		passed = value > low;	break;
					case Xt::kFecGreaterEqual:	passed = value >= low;	break;
				}
			}
			break;

			case kLfFloat:
			{
				double value = atof(AsText(fd.dom));

				switch(_cmp)
				{
					case Xt::kFecBetween:		passed = (value >= atof(_low)) && (value <= atof(_high));	break;
					case Xt::kFecEqual:			passed = value == atof(_low);	break;
					case Xt::kFecNotEqual:		passed = value != atof(_low);	break;
					case Xt::kFecLess:			passed = value < atof(_low);	break;
					case Xt::kFecLessEqual:		passed = value <= atof(_low);	break;
					case Xt::kFecGreater:		passed = value > atof(_low);	break;
					case Xt::kFecGreaterEqual:	passed = value >= atof(_low);	break;
				}
			}
			break;

			case kLfString:
			case kLfWideString:
			{
				XtString value = AsText(fd.dom);

				switch(_cmp)
				{
					case Xt::kFecContains:			passed = IndexOfStringI<char>(value, _low) >= 0;	break;
					case Xt::kFecDoesNotContain:	passed = IndexOfStringI<char>(value, _low) < 0;		break;
					case Xt::kFecStartsWith:		passed = StartsWithI<char>(value, _low);	break;
					case Xt::kFecEndsWith:			passed = EndsWithI<char>(value, _low);		break;
					case Xt::kFecEqual:				passed = _stricmp(value, _low) == 0;	break;
					case Xt::kFecNotEqual:			passed = _stricmp(value, _low) != 0;	break;
				}
			}
			break;

			case kLfBool:
			{
				bool value = atoi(AsText(fd.dom)) != 0;

				switch(_cmp)
				{
					case Xt::kFecIsTrue:	passed = value != false;	break;
					case Xt::kFecIsFalse:	passed = value == false;	break;
				}
			}
			break;

			case kLfEnum:
			{
				LfEnum*		enu		= (LfEnum*)fd.field;
				const char*	value	= AsText(fd.dom);

				if (!*value && (enu->type->options.count > 0))
					value = enu->type->options[0]->name;

				switch(_cmp)
				{
					case Xt::kFecEqual:		passed = _stricmp(value, _low) == 0; break;
					case Xt::kFecNotEqual:	passed = _stricmp(value, _low) != 0; break;
				}
			}
			break;

			case kLfHandle:
			{
				XtId id		= Xt::ParseId(AsText(fd.dom));
				XtId low	= Xt::ParseId(_low);

				if (!FindRecordById(context->db, id))
					id = 0;

				switch(_cmp)
				{
					case Xt::kFecIsNull:	passed = id == 0;	break;
					case Xt::kFecIsNotNull:	passed = id != 0;	break;
					case Xt::kFecEqual:		passed = id == low;	break;
				}
			}
			break;
		}

		if (_cmp == Xt::kFecDisplay)
			passed = true;

		if (passed)
			Result(context, QueryResult(0, type, dom, QeFeLamePath(dom)));
	}

	//
	// Xt::QeUsingLocString
	//

	class QeUlsObjects : public LvObjects
	{
		QeUsingLocString*	_uls;
		QueryContext*		_context;

	public:
		QeUlsObjects(QeUsingLocString* uls, QueryContext* context) :
			_uls(uls),
			_context(context)
		{
		}

	public:
		virtual bool Visit(LtStruct* type, Xt::DomBlock* dom)
		{
			_uls->DoObject(_context, type, dom);

			return true;
		}
	};

	QeUsingLocString::QeUsingLocString(XtId locId) :
		_locId(locId)
	{
	}

	void QeUsingLocString::Process(QueryContext* context, const QueryResult& source)
	{
		if (Xt::DomRecord* record = AsRecord(source.dom))
		{
			VisitAllObjects(context->schema, TypeOf(context->schema, record), &record->root, &QeUlsObjects(this, context), true);
			return;
		}

		if (source.field)
		{
			switch(source.field->kind)
			{
				case kLfStruct:
					DoObject(context, ((LfStruct*)source.field)->type, AsBlock(source.dom));
				break;

				case kLfStrictPtr:
				{
					if (DomComponent* com = AsComponent(source.dom))
						DoObject(context, ((LfStrictPtr*)source.field)->type, &com->data);
				}
				break;

				case kLfPtr:
				{
					LfPtr*			ptr = (LfPtr*)source.field;
					DomComponent*	com = AsComponent(source.dom);

					if (LtStruct* type = TypeOf(context->schema, ptr->baseType, com))
						DoObject(context, type, &com->data);
				}
				break;

				case kLfLocString:
				{
					if (Xt::DomBlock* blk = AsBlock(At(source.dom, source.field)))
					{
						XtId id = ParseId(AsText(At(blk, "id")));

						if (id == _locId)
							Result(context, QueryResult(0, source.type, blk, QeFeLamePath(blk)));
					}
				}
				break;
			}
		}

		if (source.type)
			DoObject(context, source.type, AsBlock(source.dom));
	}

	void QeUsingLocString::DoObject(QueryContext* context, LtStruct* type, Xt::DomBlock* dom)
	{
		if (!context || !type || !dom)
			return;

		if (type->base)
			DoObject(context, type->base->type, dom);

		for(LtStruct::FieldIterator it(&type->fields); it; ++it)
		{
			if (it->kind != kLfLocString)
				continue;

			Xt::DomBlock* blk = AsBlock(At(dom, it));

			if (!blk)
				continue;

			XtId id = ParseId(AsText(At(blk, "id")));
			
			if (id != _locId)
				continue;

			Result(context, QueryResult(0, type, blk, QeFeLamePath(blk)));
		}
	}

}