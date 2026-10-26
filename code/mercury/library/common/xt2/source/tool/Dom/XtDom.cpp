#include "Common/XtPch.h"
#include <Xt/XtTool.h>

/*
	General api syntax:

		baseType	component
		type		block
		field		element
*/
/*
	Grimoire should use unicode, however the files on disk don't have to be.

	[...]				= Expressions immediately inside brackets are data elements
	Atom				= Constant
	123					= Constant
	"abc"				= Constant
	Atom(1,2,3)			= List of unnamed expressions, can have no named elements at all
	Atom(a=1,b=2,c=3)	= Dict of named expressions, can have no unnamed elements at all (maybe {} instead of () ?)
	x => y				= Lambda function definition (args) => (body)
	() => 1,2,3			= Lambda function that takes no arguments and returns three values

	type Test
	{
		[ReadOnly] // Associated the atom "ReadOnly" with min and max
		int min, max;

		[Lines(5)] // Associated the list "Lines" with the single expression "5" as it's first argument
		wstring text;

		// Declare an array of ints

		int[] values; // The [] decorator really just adds the attribute atom [Array]

		// Nested attributes

		[Override((r, g, b, a) = [Range(1, 4), Horizontal])]
		Colour colour;

		// Everything above is really syntaxic sugar for something like the construct below, I need to implement grimoire as
		// generically as possible to make the code/data distinction as small as possible.

		[Label("Count")]
		[Source(count values)] // effectively readonly, see invertability comments below
		[Control(Sorcery.Int)];

		// To make the construct below work I'd need to write some code to flip the focus of the expression. Some expressions
		// wouldn't be invertable, but...  That just means you can't edit them :)
		//
		// For example, initially the indexof keyword probably wouldn't be invertable, but eventually it could be implemented
		// as a swap function, so you could reorder points.
		//
		// You could conceivably consider it such that you're defining either a get, or a set method for a property,
		// and the backend is doing the grunt work of converting that get/set method into the inverse.
		//
		// Given a functional programming model this shouldn't be difficult to write.

		[Line(								// Dict construct "Line"
				source	= values,			// named expr "source"	= atom "values"
				x		= p => indexof p,	// named expr "x"		= lambda p => indexof p
				y		= p => p			// named expr "y"		= lambda p => p
			)]
		control Sorcery.Chart;
	}

	// Parameters == declaration of, Arguments == use of

	enum ExpressionType
	{
		kExprConstant,
		
		// UnaryExpression (Operand)

		kExprNegate,

		// BinaryExpression (Left, Right)

		kExprAdd,
		kExprSub,
		kExprMul,
		kExprDiv,

		kExprEQ,
		kExprNE,
		kExprLT,
		kExprGT,
		kExprLE,
		kExprGE,
	};

	struct Expression
	{
		ExpressionType type;

		template<typename ReturnType>
		virtual ReturnType Accept(ExprVisitor* visitor)
		{
			return visitor->Visit(this);
		}
	};

	struct BinaryExpression : public Expression
	{
		Expression* left,
		Expression* right;

		template<typename ReturnType>
		virtual ReturnType Accept(ExprVisitor* visitor)
		{
			return visitor->Visit(this);
		}
	};

	struct UnaryExpression : public Expression
	{
		Expression* operand;
	};

	struct ExprMultiply : public BinaryExpression
	{
	};

	struct LambdaExpression : public Expression
	{
		Expression*[] parameters;
		Expression* body;
	};

	struct ParameterExpression : public Expression
	{
		string name;
	};

	struct ConstantExpression : public Expression
	{
		Variant value;
	};
	
	struct MethodCallExpr : public Expr
	{
		FuncPtr Method;
		Expression* object;
		Expression*[] parameters;
	};

	struct MemberExpr : public UnaryExpr
	{
		string member;
	};

	struct ConditionalExpr : public Expression
	{
		Expression* test;
		Expression* ifTrue;
		Expression* ifFalse;
	};

	//

	type Range<Type> // [Template(Type)]
	{
		Type min;
		Type max;
	}

	type Test
	{
		Range<int> a; // [Template(int)]
		Range<int> b;


		// To decode this I have to try to decode it one way or the other, and restart from that point if I fail
		a = Max(Frog<Dog, Cat>(123, 456));
	};

*/
/*

	Grimoire
	--------

		// [SubRecord(category)] - No category leaves uncategorised
		// [SubRef(path:model, category:parts)] - use record identified by model, 

		[Record] type Unit
		{
			sz8 name;
			Model& model;

			[Select(from:model.parts)]
			Part&& body;

			[Select(from:body.sections)]
			Section&& section;
		};

		type SpawnLayout.Base
		{
		};

		type SpawnLayout.Circle : Base
		{
			SpawnLayout.Base*& other;
		};

		using SpawnLayout;						// Bring in all of SpawnLayout
		using SpawnLayout.Circle;				// Bring in just Circle
		using SpawnLayout.Circle as MyCircle;	// Bring in just Circle as MyCircle

	Sorcery3
	--------

	*	Must be robust against schema errors, if a schema file does not load, you simply don't get those types, you will be
		warned however. A syntax highlighting editor for the schema files in sorcery would be nice, so you can double click
		an error in the list and it'll open it on that line. In addition, it should be possible to edit on the fly the schema
		without having to dump and reload everything.

	*	Must be robust against record errors, if a record does not load, the record is simply left in an "unloaded" state,
		you can click on the record to view it, and it'll try to reload, but if it fails again it'll prompt you for a resolution
		of some kind. Before loading the records sorcery should check to see if the database is under version control, if it is
		it should look to see if any files are in conflict,

	*	The Dom should be seperated off from xt, at least conceptually. It's for storing data, xt is a consumer of that data.
		There shouldn't be a link between one export item, one record. Records should be preprocessable into zero, one, or many
		export items.

	*	Anything which can be used as a component ref should have the capability of appearing as a child of the component in
		the tree view.

*/

int XtGetDbVersion()
{
	const int kXtDbVersion = 20;

	return kXtDbVersion;
}

namespace
{
	void DoStatusCallback(Xt::StatusCallback* status, Xt::StatusMessage::Severity severity, const char* msg, va_list vl)
	{
		if (status)
			status->Status(Xt::StatusMessage(severity, FormatV(msg, vl)));
	}
}

namespace Xt
{

	// Xt::StatusCallback

	StatusCallback::~StatusCallback()
	{
	}

	void StatusCallback::Status(const StatusMessage& msg)
	{
	}

	void StatusCallback::Progress(float progress)
	{
	}

	bool StatusCallback::IsCancelled()
	{
		return false;
	}

	void StatusCallback::Info(StatusCallback* status, const char* msg, ...)
	{
		va_list vl;

		va_start(vl, msg);
		XtLogV("STAT", msg, vl);
		va_end(vl);

		va_start(vl, msg);
		DoStatusCallback(status, StatusMessage::kInfo, msg, vl);
		va_end(vl);
	}

	void StatusCallback::Warning(StatusCallback* status, const char* msg, ...)
	{
		va_list vl;

		va_start(vl, msg);
		XtLogV("STAT", msg, vl);
		va_end(vl);

		va_start(vl, msg);
		DoStatusCallback(status, StatusMessage::kWarning, msg, vl);
		va_end(vl);
	}

	void StatusCallback::Error(StatusCallback* status, const char* msg, ...)
	{
		va_list vl;

		va_start(vl, msg);
		XtLogV("STAT", msg, vl);
		va_end(vl);

		va_start(vl, msg);
		DoStatusCallback(status, StatusMessage::kError, msg, vl);
		va_end(vl);
	}

	void StatusCallback::Trace(StatusCallback* status, const char* msg, ...)
	{
		va_list vl;

		va_start(vl, msg);
		XtLogV("STAT", msg, vl);
		va_end(vl);

		va_start(vl, msg);
		DoStatusCallback(status, StatusMessage::kTrace, msg, vl);
		va_end(vl);
	}

	void StatusCallback::Progress(StatusCallback* status, float progress)
	{
		if (status)
			status->Progress(progress);
	}

	//
	// Xt::Dom helpers
	//

	// Xt::Dom::FindElementByPath

	DomElement* FindElementByPath(DomElement* element, const char* path)
	{
		if (!CanCompose(element))
			return 0;

		if (!*path)
			return element;

		if ((*path == '/') || (*path == '\\'))
		{
			DomRecord* record = RecordOf(element);

			if (!record)
				return 0;

			while(((*path == '/') || (*path == '\\')) && (*path != '\0'))
				path++;

			return FindElementByPath(&record->root, path);
		}

		const char* start	= path;
		const char* end		= path;

		while((*end != '/') && (*end != '\\') && (*end != '\0'))
			end++;

		XtString name(start, end - start);

		while(((*end == '/') || (*end == '\\')) && (*end != '\0'))
			end++;

		if (name == "..")
			return FindElementByPath(element->parent, end);

		switch(element->kind)
		{
			case kDomBlock:
				if (DomBlock* block = AsBlock(element))
					return FindElementByPath(At(block, name), end);
			break;

			case kDomArray:
				if ((*name == '#') && isdigit(*(name + 1)))
				{
					if (DomArray* array = AsArray(element))
						return FindElementByPath(At(array, atoi(name + 1)), end);
				}
			break;

			case kDomComponent:
				if (DomComponent* com = AsComponent(element))
					return FindElementByPath(At(&com->data, name), end);
			break;
		}

		return 0;
	}

	// Xt::Dom::AcquireElementByPath

	DomElement* AcquireElementByPath(DomElement* element, const char* path, DomKind elementKind)
	{
		if (!element || !path)
			return 0;

		if (!CanCompose(element))
			return 0;

		if (!CanCompose(elementKind))
			return 0;

		if (!*path)
			return (elementKind == element->kind) ? element : 0;

		const char* start = path;

		while((*path != '\0') && (*path != '/') && (*path != '\\'))
			path++;

		const char* end = path;

		while((*path == '/') || (*path == '\\'))
			path++;

		const char* nextStart = path;

		while((*path != '\0') && (*path != '/') && (*path != '\\'))
			path++;

		const char* nextEnd = path;

		if (start == end)
		{
			if ((element = RecordOf(element)) == 0)
				return 0;
		}

		while(nextStart != nextEnd)
		{
			XtString token(start, end - start);

			if (token == "..")
			{
				if ((element = element->parent) == 0)
					return 0;
			}
			else if (*token == '#')
			{
				if (element->kind != kDomArray)
					return 0;

				if (*nextStart == '#')
				{
					if ((element = AcquireArray((DomArray*)element, atoi(token + 1))) == 0)
						return 0;
				}
				else
				{
					if ((element = AcquireBlock((DomArray*)element, atoi(token + 1))) == 0)
						return 0;
				}
			}
			else
			{
				if (element->kind != kDomBlock)
					return 0;

				if (*nextStart == '#')
				{
					if ((element = AcquireArray((DomBlock*)element, token)) == 0)
						return 0;
				}
				else
				{
					if ((element = AcquireBlock((DomBlock*)element, token)) == 0)
						return 0;
				}
			}

			start	= nextStart;
			end		= nextEnd;

			while((*path == '/') || (*path == '\\'))
				path++;

			nextStart = path;

			while((*path != '\0') && (*path != '/') && (*path != '\\'))
				path++;

			nextEnd = path;
		}

		XtString token(start, end - start);

		switch(elementKind)
		{
			case kDomField:
				if (*start == '#')
					return (element->kind == kDomArray) ? AcquireField((DomArray*)element, atoi(token + 1)) : 0;

			return AcquireField((DomBlock*)element, token);

			case kDomRef:
				if (*start == '#')
					return (element->kind == kDomArray) ? AcquireRef((DomArray*)element, atoi(token + 1)) : 0;

			return AcquireRef((DomBlock*)element, token);

			case kDomBlock:
				if (*start == '#')
					return (element->kind == kDomArray) ? AcquireBlock((DomArray*)element, atoi(token + 1)) : 0;

			return AcquireBlock((DomBlock*)element, token);

			case kDomArray:
				if (*start == '#')
					return (element->kind == kDomArray) ? AcquireArray((DomArray*)element, atoi(token + 1)) : 0;

			return AcquireArray((DomBlock*)element, token);
		}

		return 0;
	}

	//
	// Xt::DomMonitor
	//

	// Xt::DomMonitor::Monitor

	DomElement* Monitor(DomMonitor* monitor, DomElement* element)
	{
		assert(monitor != 0);

		if (element)
			PushBack(&element->monitors, &monitor->linkElement);
		else
			Detach(&monitor->linkElement);

		return (monitor->element = element);
	}

	//
	// Xt::Clone
	//

	bool Clone(DomField* dst, DomField* src)
	{
		if (!dst || !src)
			return false;

		return Set(dst, src->value);
	}

	bool Clone(DomRef* dst, DomRef* src)
	{
		if (!dst || !src)
			return false;

		return Set(dst, src->value);
	}

	bool Clone(DomBlock* dst, DomBlock* src)
	{
		if (!dst || !src)
			return false;

		Clear(dst);

		for(DomBlock::Iterator it(src); it; ++it)
		{
			if (!Insert(dst, it.Key(), Clone(it)))
				return false;
		}

		return true;
	}

	bool Clone(DomArray* dst, DomArray* src)
	{
		if (!dst || !src)
			return false;

		Clear(dst);

		for(DomArray::IndexIterator it(src); it; ++it) // TODO: This could be more efficent by bypassing the IndexIterator
		{
			if (!AppendWithId(dst, it.Key(), Clone(it)))
				return false;
		}

		return true;
	}

	bool Clone(DomComponent* dst, DomComponent* src)
	{
		if (!dst || !src)
			return false;

		dst->id		= src->id;
		dst->type	= src->type;

		return Clone(&dst->data, &src->data);
	}

	DomElement* Clone(DomElement* element)
	{
		if (element)
		{
			switch(element->kind)
			{
				case kDomField:
				{
					if (DomField* field = new DomField)
					{
						field->value = ((DomField*)element)->value;

						return field;
					}
				}
				break;

				case kDomRef:
				{
					if (DomRef* ref = new DomRef)
					{
						ref->value = ((DomRef*)element)->value;

						return ref;
					}
				}
				break;

				case kDomBlock:
					if (DomBlock* block = new DomBlock)
					{
						for(DomBlock::Iterator it((DomBlock*)element); it; ++it)
						{
							if (!Insert(block, it.Key(), Clone(it)))
							{
								delete block;
								return 0;
							}
						}

						return block;
					}
				break;

				case kDomArray:
					if (DomArray* array = new DomArray)
					{
						for(DomArray::IndexIterator it((DomArray*)element); it; ++it) // TODO: This could be more efficent by bypassing the IndexIterator
						{
							if (!AppendWithId(array, it.Key(), Clone(it)))
							{
								delete array;
								return 0;
							}
						}

						return array;
					}
				break;

				case kDomComponent:
					if (DomComponent* component = new DomComponent)
					{
						DomComponent* src = (DomComponent*)element;

						component->id	= src->id;
						component->type	= src->type;

						if (!Clone(&component->data, &src->data))
						{
							delete component;
							return 0;
						}

						return component;
					}
				break;

				case kDomRecord:
					if (DomRecord* record = new DomRecord)
					{
						DomRecord* src = (DomRecord*)element;

						record->id		= src->id;
						record->name	= src->name;

						if (!Clone(&record->root, &src->root))
						{
							delete record;
							return 0;
						}

						return record;
					}
				break;

				case kDomPack:
					if (DomPack* pack = new DomPack)
					{
						DomPack* src = (DomPack*)element;

						pack->id	= src->id;
						pack->name	= src->name;

						if (!Clone(&pack->properties, &src->properties))
						{
							delete pack;
							return 0;
						}

						for(DomPack::Iterator it(src); it; ++it)
						{
							if (!AddRecord(pack, (DomRecord*)Clone(it)))
							{
								delete pack;
								return 0;
							}
						}

						return pack;
					}
				break;
			}
		}

		return 0;
	}

}