#include "Common/XtPch.h"
#include <Xt/XtTool.h>

namespace
{

	//
	// DomToken
	//

	class DomToken
	{
	public:
		DomToken(const char* token = 0, size_t length = 0) :
			_token(token),
			_length(length)
		{
		}

	public:
		operator bool() const
		{
			return _token != 0;
		}

	public:
		bool operator==(const char* text) const
		{
			const char* token	= _token;
			const char* end		= _token + _length;

			for(; (token < end) && *text; text++, token++)
				if (*token != *text)
					return false;

			return (token == end) && (*text == '\0');
		}

		bool operator!=(const char* text) const
		{
			return !operator==(text);
		}

	public:
		XtString AsText() const
		{
			return XtString(_token, _length); // TODO: Deal with out of memory
		}

		XtString AsDecodedText() const
		{
			return XtsDecodeString(_token, _length); // TODO: Deal with out of memory
		}

		XtId AsId() const
		{
			return Xt::ParseId(_token, _token + _length);
		}

	private:
		const char*	_token;
		size_t		_length;
	};

	//
	// DomParser
	//

	class DomParser
	{
	public:
		DomParser(const char* document, int length);
		~DomParser();

	public:
		bool IsAtEof() const;
		bool IsAtEol() const;
		bool HasFailed() const;
		int Line() const;
		Xt::DomParseError Error() const;

	public:
		bool IsConflicted();

	public:
		DomToken ExpectIdentifier();
		DomToken ExpectIdentifierOrEnd();
		DomToken ExpectString();
		DomToken ExpectEol();

	private:
		void SkipToNext();
		void SkipToEol();

	private:
		static inline bool IsEol(char c)
		{
			return (c == '\r') || (c == '\n');
		}

		static inline bool IsWhiteSpace(char c)
		{
			return (c == ' ') || (c == '\t');
		}

		static inline bool IsIdentifier(char c)
		{
			return (
					((c >= 'a') && (c <= 'z')) ||
					((c >= 'A') && (c <= 'Z')) ||
					((c >= '0') && (c <= '9')) ||
					(c == '_')
				);
		}

	private:
		const char*			_document;
		const char*			_end;
		int					_line;
		bool				_failed;
		Xt::DomParseError	_error;
	};

	DomParser::DomParser(const char* document, int length) :
		_document(document),
		_line(1),
		_failed(false)
	{
		_end = _document + length;

		SkipToNext();
	}

	DomParser::~DomParser()
	{
	}

	bool DomParser::IsAtEof() const
	{
		return _document >= _end;
	}

	bool DomParser::IsAtEol() const
	{
		return IsEol(*_document);
	}

	bool DomParser::HasFailed() const
	{
		return _failed;
	}

	Xt::DomParseError DomParser::Error() const
	{
		return _error;
	}

	int DomParser::Line() const
	{
		return _line;
	}

	bool DomParser::IsConflicted()
	{
		if (_document >= _end)
			return false;

		return (*_document == '<') || (*_document == '=') || (*_document == '>');
	}

	DomToken DomParser::ExpectIdentifier()
	{
		if ((_document >= _end) || !IsIdentifier(*_document))
			return DomToken();

		const char* start = _document;

		do
		{
			_document++;
		}
		while((_document < _end) && IsIdentifier(*_document));

		if ((_document < _end) && !IsWhiteSpace(*_document) && !IsEol(*_document))
		{
			_document = _end;

			if (!_failed)
			{
				_error.type	= Xt::DomParseError::kBadCharacterInIdentifier;
				_error.line	= _line;
				_failed		= true;
			}

			return DomToken();
		}

		const char* end = _document;

		SkipToEol();

		return DomToken(start, end - start);
	}

	DomToken DomParser::ExpectIdentifierOrEnd()
	{
		if (_document >= _end)
			return DomToken();

		if (*_document == ';')
		{
			_document++;

			return DomToken(_document - 1, 1);
		}

		return ExpectIdentifier();
	}

	DomToken DomParser::ExpectString()
	{
		if ((_document >= _end) || (*_document != '\"'))
			return DomToken();

		const char* start = _document++;

		for(bool inEscape = false;; _document++)
		{
			if ((_document >= _end) || IsEol(*_document))
			{
				_document = _end;

				if (!_failed)
				{
					_error.type	= Xt::DomParseError::kStringMissingEndQuote;
					_error.line	= _line;
					_failed		= true;
				}

				return DomToken();
			}

			if (inEscape)
				inEscape = false;
			else if (*_document == '\'')
				inEscape = true;
			else if (*_document == '\"')
			{
				_document++;
				break;
			}
		}

		const char* end = _document;

		if ((_document < _end) && !IsWhiteSpace(*_document) && !IsEol(*_document))
		{
			_document = _end;

			if (!_failed)
			{
				_error.type	= Xt::DomParseError::kBadCharctersAfterString;
				_error.line	= _line;
				_failed		= true;
			}

			return DomToken();
		}

		SkipToEol();

		return DomToken(start, end - start);
	}

	DomToken DomParser::ExpectEol()
	{
		if (_failed)
			return DomToken();

		if (_document >= _end)
			return DomToken(_document, 0);

		if (!IsEol(*_document))
			return DomToken();

		const char* start = _document;

		SkipToNext();

		return DomToken(start, 0);
	}

	void DomParser::SkipToNext()
	{
		for(;;)
		{
			for(char newLine = '\0';; _document++)
			{
				if (_document >= _end)
					return;

				if (!IsEol(*_document))
					break;

				if ((newLine == '\0') || (*_document == newLine))
				{
					newLine = *_document;
					_line++;
				}
				else
					newLine = '\0';
			}

			for(;; _document++)
			{
				if (_document >= _end)
					return;

				if (!IsWhiteSpace(*_document))
					break;
			}

			if (*_document == '#')
			{
				for(;; _document++)
				{
					if (_document >= _end)
						return;

					if (IsEol(*_document))
						break;
				}
			}
			else
			{
				if (!IsEol(*_document))
					break;
			}
		}
	}

	void DomParser::SkipToEol()
	{
		for(;; _document++)
		{
			if (_document >= _end)
				return;

			if (!IsWhiteSpace(*_document))
				break;
		}

		if (*_document == '#')
		{
			for(; _document < _end; _document++)
			{
				if (IsEol(*_document))
					break;
			}
		}
	}

}

//
// Xt::DomParseError::AsText
//

const char* Xt::DomParseError::AsText()
{
	switch(type)
	{
		case kOkay:						return "Okay";
		case kIdentifierExpected:		return "Identifier expected";
		case kStringMissingEndQuote:	return "String missing end quote";
		case kBadCharacterInIdentifier:	return "Bad character in identifier";
		case kBadCharctersAfterString:	return "Bad charcters after string";
		case kMalformedBlock:			return "Malformed block";
		case kMalformedValue:			return "Malformed field";
		case kMalformedRef:				return "Malformed ref";
		case kMalformedArray:			return "Malformed array";
		case kMalformedEnd:				return "Malformed end";
		case kUnknownIdentifier:		return "Unknown identifier";
		case kNestingTooDeep:			return "Nesting too deep";
		case kUnexpectedEnd:			return "Unexpected end";
		case kRecordUnclosed:			return "Record unclosed";
		case kBlockUnclosed:			return "Block unclosed";
		case kArrayUnclosed:			return "Array unclosed";
		case kRecordOnlyValidAtRoot:	return "Record only valid at root";
		case kOnlyClassValidAsRoot:		return "Only record valid as root";
		case kOneClassPerFile:			return "One record per file";
		case kOutOfMemory:				return "Out of memory";
		case kCannotNameArrayItems:		return "Cannot name array items";
		case kBlockMustHaveValidName:	return "Block must have valid name";
		case kRecordMustHaveValidName:	return "Record must have valid name";
		case kFieldMustHaveValidName:	return "Field must have valid name";
		case kRefMustHaveValidName:		return "Ref must have valid name";
		case kArrayMustHaveValidName:	return "Array must have valid name";
		case kCouldntOpenFile:			return "Couldn't open file";
		case kErrorReadingFile:			return "Error reading file";
		case kNameTooLong:				return "Name too long";
		case kUnresolvedConflict:		return "Unresolved conflict";
		default:						return "Unknown error";
	}
}

//
// New parser
//

namespace
{
	struct ParseContext
	{
		DomParser*			xp;
		Xt::StatusCallback*	status;
		const char*			filename;

	public:
		ParseContext(DomParser* xp_, Xt::StatusCallback* status_, const char* filename_) :
			xp(xp_),
			status(status_),
			filename(filename_)
		{
		}
	};

	Xt::DomElement* DoParseElement(ParseContext* ctx);

	// ** Field **

	Xt::DomField* DoParseField(ParseContext* ctx)
	{
		DomToken value	= ctx->xp->ExpectString();
		DomToken eol	= ctx->xp->ExpectEol();

		if (!value || !eol)
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Badly formed field", ctx->filename, ctx->xp->Line());
			return 0;
		}

		Xt::DomField* field = new Xt::DomField;

		if (!field)
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Out of memory allocating DomField", ctx->filename, ctx->xp->Line());
			return 0;
		}

		field->value = value.AsDecodedText();

		return field;
	}

	// ** Ref **

	Xt::DomRef* DoParseRef(ParseContext* ctx)
	{
		DomToken value	= ctx->xp->ExpectIdentifier();
		DomToken eol	= ctx->xp->ExpectEol();

		if (!value || !eol)
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Badly formed ref", ctx->filename, ctx->xp->Line());
			return 0;
		}

		Xt::DomRef* ref = new Xt::DomRef;

		if (!ref)
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Out of memory allocating DomRef", ctx->filename, ctx->xp->Line());
			return 0;
		}

		ref->value = value.AsId();

		return ref;
	}

	// ** Block **

	bool DoParseBlockSub(Xt::DomBlock* block, ParseContext* ctx)
	{
		for(;;)
		{
			DomToken id = ctx->xp->ExpectIdentifierOrEnd();

			if (id == ";")
				break;

			Xt::DomElement* element = DoParseElement(ctx);

			if (!element)
				return false;

			if (!Set(block, id.AsText(), element))
			{
				Xt::StatusCallback::Error(ctx->status, "%s(%i): Bad id '%s'", ctx->filename, ctx->xp->Line(), id.AsText().Text());
				return false;
			}
		}

		if (!ctx->xp->ExpectEol())
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Badly formed end", ctx->filename, ctx->xp->Line());
			return false;
		}

		return true;
	}

	Xt::DomBlock* DoParseBlock(ParseContext* ctx)
	{
		if (!ctx->xp->ExpectEol())
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Badly formed block", ctx->filename, ctx->xp->Line());

			return 0;
		}

		XtUt::Ptr<Xt::DomBlock> block = new Xt::DomBlock;

		if (!block)
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Out of memory allocating DomBlock", ctx->filename, ctx->xp->Line());
			return 0;
		}

		if (!DoParseBlockSub(block, ctx))
			return 0;

		return XtUt::Pop(&block);
	}

	// ** Array **

	Xt::DomArray* DoParseArray(ParseContext* ctx)
	{
		if (!ctx->xp->ExpectEol())
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Badly formed array", ctx->filename, ctx->xp->Line());
			return 0;
		}

		XtUt::Ptr<Xt::DomArray> array = new Xt::DomArray;

		if (!array)
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Out of memory allocating DomArray", ctx->filename, ctx->xp->Line());
			return 0;
		}

		for(;;)
		{
			DomToken id = ctx->xp->ExpectIdentifierOrEnd();

			if (id == ";")
				break;

			Xt::DomElement* element = DoParseElement(ctx);

			if (!element)
				return 0;

			XtId arrayId = id.AsId();

			if (arrayId && Exists(&array->elements, arrayId)) // TODO: Hold over from before arrays had id's, can probably be removed now
			{
				Xt::StatusCallback::Warning(ctx->status, "%s(%i): Renumbering array entry", ctx->filename, ctx->xp->Line());
				arrayId = 0;
			}

			if (!arrayId)
				arrayId = GenerateUniqueArrayId(array);

			if (!AppendWithId(array, arrayId, element))
				return 0;
		}

		if (!ctx->xp->ExpectEol())
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Badly formed end", ctx->filename, ctx->xp->Line());
			return 0;
		}

		return XtUt::Pop(&array);
	}

	// ** Component **

	Xt::DomComponent* DoParseComponent(ParseContext* ctx)
	{
		DomToken id		= ctx->xp->ExpectIdentifier();
		DomToken type	= ctx->xp->ExpectString();
		DomToken eol	= ctx->xp->ExpectEol();

		if (!id || !type || !eol)
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Badly formed component", ctx->filename, ctx->xp->Line());
			return 0;
		}

		XtUt::Ptr<Xt::DomComponent> component = new Xt::DomComponent;

		if (!component)
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Out of memory allocating DomComponent", ctx->filename, ctx->xp->Line());
			return 0;
		}

		component->id	= id.AsId();
		component->type	= type.AsDecodedText();

		if (!DoParseBlockSub(&component->data, ctx))
			return 0;

		return XtUt::Pop(&component);
	}

	// ** Record **

	Xt::DomRecord* DoParseRecord(ParseContext* ctx)
	{
		DomToken id		= ctx->xp->ExpectIdentifier();
		DomToken name	= ctx->xp->ExpectString();
		DomToken type	= ctx->xp->ExpectString();
		DomToken eol	= ctx->xp->ExpectEol();

		if (!id || !name || !type || !eol)
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Badly formed record", ctx->filename, ctx->xp->Line());
			return 0;
		}

		XtUt::Ptr<Xt::DomRecord> record = new Xt::DomRecord;

		if (!record)
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Out of memory allocating DomRecord", ctx->filename, ctx->xp->Line());
			return 0;
		}

		if ((record->id = id.AsId()) == 0)
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Record id may not be zero", ctx->filename, ctx->xp->Line());
			return 0;
		}

		if ((record->name = name.AsDecodedText()).Length() <= 0)
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Record name may not be blank", ctx->filename, ctx->xp->Line());
			return 0;
		}

		record->root.type = type.AsDecodedText();

		if (!DoParseBlockSub(&record->root.data, ctx))
			return 0;

		record->dirty = false;

		return XtUt::Pop(&record);
	}

	// ** Pack **

	bool DoParsePackProperties(Xt::DomPack* pack, ParseContext* ctx)
	{
		if (!ctx->xp->ExpectEol())
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Badly formed pack properties", ctx->filename, ctx->xp->Line());
			return false;
		}

		return DoParseBlockSub(&pack->properties, ctx);
	}

	bool DoParsePackRecords(Xt::DomPack* pack, ParseContext* ctx)
	{
		if (!ctx->xp->ExpectEol())
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Badly formed pack records", ctx->filename, ctx->xp->Line());
			return false;
		}

		for(;;)
		{
			DomToken kind = ctx->xp->ExpectIdentifierOrEnd();

			if (kind == ";")
				break;

			if (kind == "record")
			{
				Xt::DomRecord* record = DoParseRecord(ctx);

				if (!record)
					return 0;

				if (!AddRecord(pack, record))
				{
					Xt::StatusCallback::Error(ctx->status, "%s(%i): Failed to add record", ctx->filename, ctx->xp->Line());
					return 0;
				}

				record->dirty = false;
			}
			else
			{
				Xt::StatusCallback::Error(ctx->status, "%s(%i): Unexpected token '%s'", ctx->filename, ctx->xp->Line(), kind.AsText().Text());
				return 0;
			}
		}

		if (!ctx->xp->ExpectEol())
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Badly formed end", ctx->filename, ctx->xp->Line());
			return false;
		}

		return true;
	}

	Xt::DomPack* DoParsePack(ParseContext* ctx)
	{
		DomToken id		= ctx->xp->ExpectIdentifier();
		DomToken name	= ctx->xp->ExpectString();
		DomToken eol	= ctx->xp->ExpectEol();

		if (!id || !name || !eol)
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Badly formed pack", ctx->filename, ctx->xp->Line());
			return 0;
		}

		XtUt::Ptr<Xt::DomPack> pack = new Xt::DomPack;

		if (!pack)
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Out of memory allocating DomPack", ctx->filename, ctx->xp->Line());
			return 0;
		}

		if ((pack->id = id.AsId()) == 0)
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Pack id may not be zero", ctx->filename, ctx->xp->Line());
			return 0;
		}

		if ((pack->name = name.AsDecodedText()).Length() <= 0)
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Pack name may not be blank", ctx->filename, ctx->xp->Line());
			return 0;
		}

		for(;;)
		{
			DomToken kind = ctx->xp->ExpectIdentifierOrEnd();

			if (kind == ";")
				break;

			if (kind == "properties")
			{
				if (!DoParsePackProperties(pack, ctx))
					return 0;
			}
			else if (kind == "records")
			{
				if (!DoParsePackRecords(pack, ctx))
					return 0;
			}
			else
			{
				Xt::StatusCallback::Error(ctx->status, "%s(%i): Unexpected token '%s'", ctx->filename, ctx->xp->Line(), kind.AsText().Text());
				return 0;
			}
		}

		if (!ctx->xp->ExpectEol())
		{
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Badly formed end", ctx->filename, ctx->xp->Line());
			return 0;
		}

		pack->dirty = false;

		return XtUt::Pop(&pack);
	}

	// ** Element **

	Xt::DomElement* DoParseElement(ParseContext* ctx)
	{
		DomToken kind = ctx->xp->ExpectIdentifier();

		if (kind)
		{
			if (kind == "field")			return DoParseField(ctx);
			else if (kind == "ref")			return DoParseRef(ctx);
			else if (kind == "block")		return DoParseBlock(ctx);
			else if (kind == "array")		return DoParseArray(ctx);
			else if (kind == "component")	return DoParseComponent(ctx);
			else if (kind == "record")		return DoParseRecord(ctx);
			else if (kind == "pack")		return DoParsePack(ctx);
			else							Xt::StatusCallback::Error(ctx->status, "%s(%i): Unexpected token '%s'", ctx->filename, ctx->xp->Line(), kind.AsText().Text());
		}
		else
			Xt::StatusCallback::Error(ctx->status, "%s(%i): Expected token", ctx->filename, ctx->xp->Line());

		return 0;
	}

}

Xt::DomElement* Xt::Parse2(XtUt::Block* mb, Xt::StatusCallback* status, const char* filename)
{
	DomParser xp((const char*)mb->start, Capacity(mb));

	DomToken xt		= xp.ExpectIdentifier();
	DomToken eol	= xp.ExpectEol();

	if (!xt || !eol || (xt != "xt"))
	{
		Xt::StatusCallback::Error(status, "%s(%i): Data missing xt tag", filename, xp.Line());
		return 0;
	}

	return DoParseElement(&ParseContext(&xp, status, filename));
}

bool Xt::Parse2(Xt::DomBlock* block, XtUt::Block* mb, Xt::StatusCallback* status, const char* filename)
{
	DomParser xp((const char*)mb->start, Capacity(mb));

	DomToken tknXt		= xp.ExpectIdentifier();
	DomToken tknEol0	= xp.ExpectEol();
	DomToken tknBlock	= xp.ExpectIdentifier();
	DomToken tknEol1	= xp.ExpectEol();

	if (!tknXt || !tknEol0 || !tknBlock || !tknEol1 || (tknXt != "xt") || (tknBlock != "block"))
	{
		Xt::StatusCallback::Error(status, "%s(%i): Data missing xt tag", filename, xp.Line());
		return 0;
	}

	return DoParseBlockSub(block, &ParseContext(&xp, status, filename));
}

bool Xt::Parse(Bundle* bundle, XtUt::Block* mb, StatusCallback* status)
{
	if (!bundle || !mb)
		return false;

	DomParser xp((const char*)mb->start, Capacity(mb));

	ParseContext ctx(&xp, status, "");

	DomToken tknXtBundle	= xp.ExpectIdentifier();
	DomToken tknEol0		= xp.ExpectEol();

	if (!tknXtBundle || !tknEol0 || (tknXtBundle != "xtbundle"))
	{
		Xt::StatusCallback::Error(ctx.status, "%s(%i): Data missing xt-bundle tag", ctx.filename, ctx.xp->Line());
		return 0;
	}

	for(;;)
	{
		DomToken id = ctx.xp->ExpectIdentifierOrEnd();

		if (id == ";")
			break;

		DomToken type	= ctx.xp->ExpectString();
		DomToken field	= ctx.xp->ExpectString();

		if (!type || !field)
		{
			Xt::StatusCallback::Error(ctx.status, "%s(%i): Badly formed item", ctx.filename, ctx.xp->Line());
			return false;
		}

		Xt::DomElement* element = DoParseElement(&ctx);

		if (!element)
			return false;

		BundleItem* item = Xt::Append(bundle, new BundleItem);

		if (!item)
		{
			delete element;
			return false;
		}

		item->type		= type.AsDecodedText();
		item->field		= field.AsDecodedText();
		item->element	= element;
	}

	if (!ctx.xp->ExpectEol())
	{
		Xt::StatusCallback::Error(ctx.status, "%s(%i): Badly formed end", ctx.filename, ctx.xp->Line());
		return false;
	}

	return true;
}