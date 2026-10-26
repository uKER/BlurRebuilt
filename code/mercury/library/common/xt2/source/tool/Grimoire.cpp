#include "Common/XtPch.h"
#include "Xt/XtTool.h"
#include "Xt/Impl/Grimoire.h"

namespace Grimoire
{
	const Atom kXtNullAtom;

	bool operator==(const Token& lhs, const char* rhs)
	{
		const char* s = lhs.start;

		for(; s < lhs.end; s++)
		{
			if (*s != *rhs)
				break;

			rhs++;
		}

		return (s >= lhs.end) && (*rhs == '\0');
	}

	bool operator==(const Token& lhs, const Token& rhs)
	{
		const char* l = lhs.start;
		const char* r = rhs.start;

		for(; (l < lhs.end) && (r < rhs.end); l++, r++)
			if (*l != *r)
				break;

		return (l >= lhs.end) && (r >= rhs.end);
	}
};

namespace
{
	using namespace Grimoire;

	// LexState

	enum LexState
	{
		// Symbol

		kLexNone,
		kLexIdentifier,
		kLexLiteralInteger,
		kLexLiteralFloat,
		kLexLiteralString,
		kLexLiteralStringEscape,
		kLexNotEqual,
		kLexOr,
		kLexAnd,
		kLexEqual,
		kLexLessEqual,
		kLexGreaterEqual,

		// Whitespace

		kLexLineComment,
		kLexComment,
		kLexCommentEnd
	};

	// Context

	struct Context
	{
		const char*	document;
		int			length;
		int			line;

		bool		pushed;
		Token		token;

		const char*			filename;
		Grimoire::Document*	grimDoc;
		Xt::StatusCallback*	status;

	public:
		inline Context(const char* document_, int length_, const char* filename_, Grimoire::Document* grimDoc_, Xt::StatusCallback* status_) :
			document(document_),
			length(length_),
			line(1),
			pushed(false),
			filename(filename_),
			grimDoc(grimDoc_),
			status(status_)
		{
		}
	};

	// Helpers

	bool IsIdentifier(char c)
	{
		return (((c >= 'a') && (c <= 'z')) ||
				((c >= 'A') && (c <= 'Z')) ||
				((c >= '0') && (c <= '9')) ||
				(c == '_')
			);
	}

	bool IsNumber(char c)
	{
		return (((c >= 'a') && (c <= 'z')) ||
				((c >= 'A') && (c <= 'Z')) ||
				((c >= '0') && (c <= '9')) ||
				(c == '.')
			);
	}

	// ReportError

	void ReportError(Context* ctx, int line, const char* text, ...)
	{
		if (ctx->status)
		{
			char buffer[512];
			va_list vl;

			va_start(vl, text);
			_vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, text, vl);
			va_end(vl);

			Xt::StatusCallback::Error(ctx->status, "%s(%i): %s", ctx->filename, line, buffer);
		}
	}

	// LexNext

	Symbol LexNext_Classify(Context* ctx, char c, char n, LexState* state)
	{
		switch(*state)
		{
			case kLexNone:
				switch(c)
				{
					case 'a': case 'b': case 'c': case 'd': case 'e':
					case 'f': case 'g': case 'h': case 'i': case 'j':
					case 'k': case 'l': case 'm': case 'n': case 'o':
					case 'p': case 'q': case 'r': case 's': case 't':
					case 'u': case 'v': case 'w': case 'x': case 'y':
					case 'z': case 'A': case 'B': case 'C': case 'D':
					case 'E': case 'F': case 'G': case 'H': case 'I':
					case 'J': case 'K': case 'L': case 'M': case 'N':
					case 'O': case 'P': case 'Q': case 'R': case 'S':
					case 'T': case 'U': case 'V': case 'W': case 'X':
					case 'Y': case 'Z': case '_':
						if (!IsIdentifier(n))
							return kSymIdentifier;

						*state = kLexIdentifier;
					break;

					case '0': case '1': case '2': case '3': case '4':
					case '5': case '6': case '7': case '8': case '9':
						if (!IsNumber(n))
							return kSymLiteralInteger;

						*state = kLexLiteralInteger;
					break;

					case '\"':
						*state = kLexLiteralString;
					break;

					case '!':
						if (n != '=')
							return kSymNot;

						*state = kLexNotEqual;
					break;

					case '|':
						if (n != '|')
						{
							ReportError(ctx, ctx->line, "Illegal token |");
							return kSymInvalid;
						}

						*state = kLexOr;
					break;

					case '&':
						if (n != '&')
							return kSymRef;

						*state = kLexAnd;
					break;

					case '=':
						if (n != '=')
							return kSymAssign;

						*state = kLexEqual;
					break;

					case '<':
						if (n != '=')
							return kSymLess;

						*state = kLexLessEqual;
					break;

					case '>':
						if (n != '=')
							return kSymGreater;

						*state = kLexGreaterEqual;
					break;

					case '.':
						if ((n < '0') || (n > '9'))
							return kSymDot;

						*state = kLexLiteralFloat;
					break;

					case '[': return kSymOpenBracket;
					case ']': return kSymCloseBracket;

					case '(': return kSymOpenParen;
					case ')': return kSymCloseParen;

					case '{': return kSymOpenBrace;
					case '}': return kSymCloseBrace;

					case ',': return kSymComma;
					case ':': return kSymColon;
					case ';': return kSymSemicolon;

					case '+': return kSymPlus;
					case '-': return kSymMinus;
					case '/': return kSymDivide;
					case '*': return kSymMultiply;
					case '%': return kSymModulo;

					default:
						ReportError(ctx, ctx->line, "Illegal character '%c'", c);
					return kSymInvalid;
				}
			break;

			case kLexIdentifier:
				if (!IsIdentifier(n))
					return kSymIdentifier;
			break;

			case kLexLiteralInteger:
			case kLexLiteralFloat:
				if (c == '.')
					*state = kLexLiteralFloat;

				if (!IsNumber(n))
					return (*state == kLexLiteralInteger) ? kSymLiteralInteger : kSymLiteralFloat;
			break;

			case kLexLiteralString:
				switch(c)
				{
					case '\r':
					case '\n':
						ReportError(ctx, ctx->token.line, "Newline found in string literal");
					return kSymInvalid;

					case '\0':
						ReportError(ctx, ctx->token.line, "Unexpected end of file found in string literal");
					return kSymInvalid;

					case '\"':
					return kSymLiteralString;

					case '\'':
						*state = kLexLiteralStringEscape;
					break;
				}
			break;

			case kLexLiteralStringEscape:
				switch(c)
				{
					case '\r':
					case '\n':
						ReportError(ctx, ctx->token.line, "Newline found in string literal");
					return kSymInvalid;

					case '\0':
						ReportError(ctx, ctx->token.line, "Unexpected end of file found in string literal");
					return kSymInvalid;

					default:
						*state = kLexLiteralString;
					break;
				}
			break;

			case kLexNotEqual:
			return kSymNotEqual;

			case kLexOr:
			return kSymOr;

			case kLexAnd:
			return kSymAnd;

			case kLexEqual:
			return kSymEqual;

			case kLexLessEqual:
			return kSymLessEqual;

			case kLexGreaterEqual:
			return kSymGreaterEqual;
		}

		return kSymNone;
	}

	Symbol LexNext_SkipWhitespace(Context* ctx)
	{
		LexState	state			= kLexNone;
		char		newline			= '\0';
		int			commentStart	= 0;

		// Whitespace and comments

		for(; ctx->length > 0; ctx->document++, ctx->length--)
		{
			char c = *ctx->document;

			// Count lines

			if (((c == '\r') || (c == '\n')) && (c != newline))
			{
				newline = (c == '\n') ? '\r' : '\n';
				ctx->line++;
			}
			else
				newline = '\0';

			if (state == kLexNone)
			{
				if (c == '/')
				{
					char n = (ctx->length > 1) ? ctx->document[1] : '\0';

					commentStart = ctx->line;

					if (n == '*')
						state = kLexComment;
					else if (n == '/')
						state = kLexLineComment;
					else
						break;
				}
				else if ((c != ' ') && (c != '\t') && (c != '\r') && (c != '\n'))
					break;
			}
			else if (state == kLexLineComment)
			{
				if ((c == '\r') || (c == '\n'))
					state = kLexNone;
			}
			else if (state == kLexComment)
			{
				if (c == '*')
					state = kLexCommentEnd;
			}
			else if (state == kLexCommentEnd)
			{
				if (c == '/')
					state = kLexNone;
				else if (c != '*')
					state = kLexComment;
			}
		}

		ctx->token.line = ctx->line;

		// End of file and unclosed comments

		if (ctx->length <= 0)
		{
			if ((state == kLexNone) || (state == kLexLineComment))
				return (ctx->token.symbol = kSymEof);

			ReportError(ctx, commentStart, "Unexpected end of file found in comment");

			return (ctx->token.symbol = kSymInvalid);
		}

		return (ctx->token.symbol = kSymNone);
	}

	Symbol LexNext_ParseSymbol(Context* ctx)
	{
		const char*	start	= ctx->document;
		LexState	state	= kLexNone;
		char		newline	= '\0';

		while(ctx->length > 0)
		{
			char c = *ctx->document;
			char n = (ctx->length > 1) ? ctx->document[1] : '\0';

			ctx->document++;
			ctx->length--;

			// Count lines

			if (((c == '\r') || (c == '\n')) && (c != newline))
			{
				newline = (c == '\n') ? '\r' : '\n';
				ctx->line++;
			}
			else
				newline = '\0';

			if ((ctx->token.symbol = LexNext_Classify(ctx, c, n, &state)) != kSymNone)
			{
				ctx->token.start	= start;
				ctx->token.end		= ctx->document;

				return ctx->token.symbol;
			}
		}

		if ((ctx->token.symbol = LexNext_Classify(ctx, '\0', '\0', &state)) != kSymNone)
		{
			ctx->token.start	= start;
			ctx->token.end		= ctx->document;

			return ctx->token.symbol;
		}

		ReportError(ctx, ctx->line, "Unexpected end of file");

		return (ctx->token.symbol = kSymInvalid);
	}

	Symbol LexNext(Context* ctx)
	{
		ctx->token.start	= 0;
		ctx->token.end		= 0;

		if (LexNext_SkipWhitespace(ctx) != kSymNone)
			return ctx->token.symbol;

		return LexNext_ParseSymbol(ctx);
	}

	// ParseNext

	Symbol ParseNext(Context* ctx)
	{
		if (ctx->pushed)
		{
			ctx->pushed = false;

			return ctx->token.symbol;
		}

		return LexNext(ctx);
	}

	// Expect

	bool Expect(Context* ctx, Symbol expecting)
	{
		if (ParseNext(ctx) == expecting)
			return true;

		ctx->pushed = true;

		return false;
	}

	// Check

	bool Check(Context* ctx, Symbol expecting)
	{
		ParseNext(ctx);

		ctx->pushed = true;

		return ctx->token.symbol == expecting;
	}

	// Push

	void Push(Context* ctx)
	{
		ctx->pushed = true;
	}

	// ParseExpression

	struct ExpressionPrecedence
	{
		Symbol	symbol;
		int		level;
	};

	ExpressionPrecedence gExpressionPrecedence[] =
	{
		{ kSymColon,		9 },
		{ kSymOr,			8 },
		{ kSymAnd,			7 },
		{ kSymEqual,		6 },
		{ kSymNotEqual,		6 },
		{ kSymLess,			5 },
		{ kSymGreater,		5 },
		{ kSymLessEqual,	5 },
		{ kSymGreaterEqual,	5 },
		{ kSymPlus,			4 },
		{ kSymMinus,		4 },
		{ kSymMultiply,		3 },
		{ kSymDivide,		3 },
		{ kSymModulo,		3 },
		{ kSymDot,			1 },
		{ kSymOpenParen,	1 },
		{ kSymOpenBracket,	1 },
		{ kSymNone }
	};

	template<typename ExpType>
	ExpType* ParseExpression_Setup(Context* ctx, Expression** result)
	{
		ExpType* expr = new ExpType;

		if (!expr)
		{
			ReportError(ctx, ctx->token.line, "Out of memory");
			return 0;
		}

		expr->token = ctx->token;

		if (result)
			*result = expr;

		return expr;
	}

	bool ParseAttributes(Context* ctx, XtUt::List<Attribute>* attributes);

	bool ParseExpression_Recursive(Context* ctx, int level, Expression** result)
	{
		if (level == 0)
		{
			if (
					Expect(ctx, kSymLiteralInteger) ||
					Expect(ctx, kSymLiteralFloat) ||
					Expect(ctx, kSymLiteralString) ||
					Expect(ctx, kSymIdentifier)
				)
			{
				return ParseExpression_Setup<Expression>(ctx, result) != 0;
			}
			else if (Expect(ctx, kSymOpenParen))
			{
				if (!ParseExpression_Recursive(ctx, gExpressionPrecedence[1].level, result))
					return true;

				if (!Expect(ctx, kSymCloseParen))
				{
					ReportError(ctx, ctx->token.line, "Expected )");
					return false;
				}

				return true;
			}
			else if (Expect(ctx, kSymOpenBracket))
			{
				ExpAttributes* expAttr = ParseExpression_Setup<ExpAttributes>(ctx, result);

				if (!expAttr)
					return false;

				expAttr->token.symbol = kSymAttributes;

				Push(ctx);

				return ParseAttributes(ctx, &expAttr->attributes);
			}
			else
			{
				ReportError(ctx, ctx->token.line, "Unexpected token %.*s", ctx->token.end - ctx->token.start, ctx->token.start);
				return false;
			}
		}
		else if (level == 2)
		{
			if (Expect(ctx, kSymNot) || Expect(ctx, kSymPlus) || Expect(ctx, kSymMinus))
			{
				if (Expression* expr = ParseExpression_Setup<Expression>(ctx, result))
					return ParseExpression_Recursive(ctx, level - 1, &expr->right);

				return false;
			}
			else
				return ParseExpression_Recursive(ctx, level - 1, result);
		}
		else
		{
			Expression* left = 0;

			if (!ParseExpression_Recursive(ctx, level - 1, &left))
			{
				delete left;
				return false;
			}

			for(;;)
			{
				ExpressionPrecedence* ep = gExpressionPrecedence;

				for(; ep->symbol != kSymNone; ep++)
					if ((ep->level == level) && Expect(ctx, ep->symbol))
						break;

				if (ep->symbol == kSymNone)
				{
					*result = left;
	 				return true;
				}

				Expression* expr = ParseExpression_Setup<Expression>(ctx, 0);

				if (!expr)
				{
					delete left;
					return false;
				}

				expr->left = left;

				if (ep->symbol == kSymOpenParen)
				{
					if (!ParseExpression_Recursive(ctx, gExpressionPrecedence[1].level, &expr->right))
					{
						delete expr;
						return false;
					}

					if (!Expect(ctx, kSymCloseParen))
					{
						delete expr;
						ReportError(ctx, ctx->token.line, "Expected )");
						return false;
					}
				}
				else if (ep->symbol == kSymOpenBracket)
				{
					if (!ParseExpression_Recursive(ctx, gExpressionPrecedence[1].level, &expr->right))
					{
						delete expr;
						return false;
					}

					if (!Expect(ctx, kSymCloseBracket))
					{
						delete expr;
						ReportError(ctx, ctx->token.line, "Expected ]");
						return false;
					}
				}
				else
				{
					if (!ParseExpression_Recursive(ctx, level - 1, &expr->right))
					{
						delete expr;
						return false;
					}
				}

				left = expr;
			}
		}
	}

	bool ParseExpression(Context* ctx, Parameter* parameter)
	{
		return ParseExpression_Recursive(ctx, gExpressionPrecedence[0].level, &parameter->expression);
	}

	// ParseParameter

	bool ParseParameter(Context* ctx, Parameter* parameter)
	{
		if (!parameter)
		{
			ReportError(ctx, ctx->line, "Out of memory");
			return false;
		}

		return ParseExpression(ctx, parameter);
	}

	// ParseAttribute

	bool ParseAttribute(Context* ctx, Attribute* attribute)
	{
		if (!attribute)
		{
			ReportError(ctx, ctx->line, "Out of memory");
			return false;
		}

		if (!Expect(ctx, kSymIdentifier))
		{
			ReportError(ctx, ctx->token.line, "Expected attribute name");
			return false;
		}

		attribute->name = ctx->token;

		if (!Expect(ctx, kSymOpenParen))
			return true;

		for(;;)
		{
			if (!ParseParameter(ctx, AppendEx(&attribute->parameters, new Parameter)))
				return false;

			if (!Expect(ctx, kSymComma))
				break;
		}

		if (!Expect(ctx, kSymCloseParen))
		{
			ReportError(ctx, ctx->token.line, "Expected )");
			return false;
		}

		return true;
	}

	// ParseDefaults

	bool ParseDefaults(Context* ctx, XtUt::List<Attribute>* attributes)
	{
		Attribute* attr = AppendEx(attributes, new Attribute);

		if (!attr)
		{
			ReportError(ctx, ctx->line, "Out of memory");
			return false;
		}

		static const char* kDefault = "Default";

		attr->name.symbol	= kSymIdentifier;
		attr->name.start	= kDefault;
		attr->name.end		= kDefault + 7;
		attr->name.line		= ctx->token.line;

		if (Expect(ctx, kSymOpenBrace))
		{
			for(;;)
			{
				if (!ParseParameter(ctx, AppendEx(&attr->parameters, new Parameter)))
					return false;

				if (!Expect(ctx, kSymComma))
					break;
			}

			if (!Expect(ctx, kSymCloseBrace))
			{
				ReportError(ctx, ctx->token.line, "Expected }");
				return false;
			}
		}
		else
		{
			if (!ParseParameter(ctx, AppendEx(&attr->parameters, new Parameter)))
				return false;
		}

		return true;
	}

	// ParseAttributes

	bool ParseAttributes(Context* ctx, XtUt::List<Attribute>* attributes)
	{
		for(;;)
		{
			if (!Expect(ctx, kSymOpenBracket))
				return true;

			for(;;)
			{
				if (!ParseAttribute(ctx, AppendEx(attributes, new Attribute)))
					return false;

				if (!Expect(ctx, kSymComma))
					break;
			}

			if (!Expect(ctx, kSymCloseBracket))
			{
				ReportError(ctx, ctx->token.line, "Expected ]");
				return false;
			}
		}

		return true;
	}

	// ParseAtom

	bool ParseAtom(Context* ctx, Type* type, XtUt::List<Attribute>* attributes)
	{
		if (!type)
		{
			ReportError(ctx, ctx->line, "Out of memory");
			return false;
		}

		Replace(&type->attributes, attributes);

		type->document = ctx->grimDoc;

		if (!Expect(ctx, kSymIdentifier))
		{
			ReportError(ctx, ctx->token.line, "Expected type name");
			return false;
		}

		type->name = ctx->token;

		if (!Expect(ctx, kSymColon))
		{
			ReportError(ctx, ctx->token.line, "Expected :");
			return false;
		}

		if (!Expect(ctx, kSymIdentifier))
		{
			ReportError(ctx, ctx->token.line, "Expected atom type");
			return false;
		}

		if (Attribute* attr = AppendEx(&type->attributes, new Attribute))
		{
			static const char* kAtom = "Atom";

			attr->name.symbol	= kSymIdentifier;
			attr->name.start	= kAtom;
			attr->name.end		= kAtom + 4;
			attr->name.line		= ctx->token.line;

			if (Parameter* param = AppendEx(&attr->parameters, new Parameter))
			{
				if ((param->expression = new Expression) != 0)
				{
					param->expression->token = ctx->token;
				}
				else
				{
					ReportError(ctx, ctx->line, "Out of memory");
					return false;
				}
			}
			else
			{
				ReportError(ctx, ctx->line, "Out of memory");
				return false;
			}
		}
		else
		{
			ReportError(ctx, ctx->line, "Out of memory");
			return false;
		}

		if (!Expect(ctx, kSymSemicolon))
		{
			ReportError(ctx, ctx->token.line, "Expected ;");
			return false;
		}

		return true;
	}

	// ParseField

	bool ParseField(Context* ctx, Member* member, XtUt::List<Attribute>* attributes)
	{
		if (!member)
		{
			ReportError(ctx, ctx->line, "Out of memory");
			return false;
		}

		Replace(&member->attributes, attributes);

		if (!Expect(ctx, kSymIdentifier))
		{
			ReportError(ctx, ctx->token.line, "Expected type name");
			return false;
		}

		member->type = ctx->token;

		if (Expect(ctx, kSymMultiply))
		{
			Attribute* attr = AppendEx(&member->attributes, new Attribute);

			if (!attr)
			{
				ReportError(ctx, ctx->line, "Out of memory");
				return false;
			}

			static const char* kPtr = "Ptr";

			attr->name.symbol	= kSymIdentifier;
			attr->name.start	= kPtr;
			attr->name.end		= kPtr + 3;
			attr->name.line		= ctx->token.line;

			if (Expect(ctx, kSymRef))
			{
				Attribute* attr = AppendEx(&member->attributes, new Attribute);

				if (!attr)
				{
					ReportError(ctx, ctx->line, "Out of memory");
					return false;
				}

				static const char* kWeak = "Weak";

				attr->name.symbol	= kSymIdentifier;
				attr->name.start	= kWeak;
				attr->name.end		= kWeak + 4;
				attr->name.line		= ctx->token.line;
			}
		}
		else if (Expect(ctx, kSymRef))
		{
			Attribute* attr = AppendEx(&member->attributes, new Attribute);

			if (!attr)
			{
				ReportError(ctx, ctx->line, "Out of memory");
				return false;
			}

			static const char* kHandle = "Handle";

			attr->name.symbol	= kSymIdentifier;
			attr->name.start	= kHandle;
			attr->name.end		= kHandle + 6;
			attr->name.line		= ctx->token.line;
		}

		if (Expect(ctx, kSymOpenBracket))
		{
			if (!Expect(ctx, kSymCloseBracket))
			{
				ReportError(ctx, ctx->token.line, "Expected ]");
				return false;
			}

			Attribute* attr = AppendEx(&member->attributes, new Attribute);

			if (!attr)
			{
				ReportError(ctx, ctx->line, "Out of memory");
				return false;
			}

			static const char* kArray = "Array";

			attr->name.symbol	= kSymIdentifier;
			attr->name.start	= kArray;
			attr->name.end		= kArray + 5;
			attr->name.line		= ctx->token.line;
		}

		if (!Expect(ctx, kSymIdentifier))
		{
			ReportError(ctx, ctx->token.line, "Expected member name");
			return false;
		}

		member->name = ctx->token;

		if (Expect(ctx, kSymAssign))
		{
			if (!ParseDefaults(ctx, &member->attributes))
				return false;
		}

		return true;
	}

	// ParseControl

	bool ParseControl(Context* ctx, Member* member, XtUt::List<Attribute>* attributes)
	{
		if (!member)
		{
			ReportError(ctx, ctx->line, "Out of memory");
			return false;
		}

		Replace(&member->attributes, attributes);

		if (!Expect(ctx, kSymIdentifier))
		{
			ReportError(ctx, ctx->token.line, "Expected control type");
			return false;
		}

		member->type = ctx->token;

		return true;
	}

	// ParseType

	bool ParseType(Context* ctx, Type* type, XtUt::List<Attribute>* attributes)
	{
		if (!type)
		{
			ReportError(ctx, ctx->line, "Out of memory");
			return false;
		}

		Replace(&type->attributes, attributes);

		type->document = ctx->grimDoc;

		if (!Expect(ctx, kSymIdentifier))
		{
			ReportError(ctx, ctx->token.line, "Expected type name");
			return false;
		}

		type->name = ctx->token;

		if (Expect(ctx, kSymColon))
		{
			XtUt::List<Attribute> baseAttr;

			if (!ParseAttributes(ctx, &baseAttr))
				return false;

			if (!Expect(ctx, kSymIdentifier))
			{
				ReportError(ctx, ctx->token.line, "Expected base type");
				return false;
			}

			if (BaseType* baseType = (type->base = new BaseType))
			{
				baseType->type = ctx->token;

				Replace(&baseType->attributes, &baseAttr);
			}
			else
			{
				ReportError(ctx, ctx->line, "Out of memory");
				return false;
			}
		}

		if (!Expect(ctx, kSymOpenBrace))
		{
			ReportError(ctx, ctx->token.line, "Expected {");
			return false;
		}

		for(;;)
		{
			if (Check(ctx, kSymCloseBrace))
				break;

			XtUt::List<Attribute> attributes;

			if (!ParseAttributes(ctx, &attributes))
				return false;

			if (!Expect(ctx, kSymIdentifier))
			{
				ReportError(ctx, ctx->token.line, "Expected type name");
				return false;
			}

			if (ctx->token == "field")
			{
				if (!ParseField(ctx, XtUt::AppendEx(&type->members, new Member(kMbField)), &attributes))
					return false;
			}
			else if (ctx->token == "control")
			{
				if (!ParseControl(ctx, XtUt::AppendEx(&type->members, new Member(kMbControl)), &attributes))
					return false;
			}
			else
			{
				Push(ctx);

				if (!ParseField(ctx, XtUt::AppendEx(&type->members, new Member(kMbField)), &attributes))
					return false;
			}

			if (!Expect(ctx, kSymSemicolon))
			{
				ReportError(ctx, ctx->token.line, "Expected ;");
				return false;
			}
		}

		if (!Expect(ctx, kSymCloseBrace))
		{
			ReportError(ctx, ctx->token.line, "Expected }");
			return false;
		}

		// Eat semicolons on end

		Expect(ctx, kSymSemicolon);

		return true;
	}

	// ParseLabel

	bool ParseLabel(Context* ctx, Member* member, XtUt::List<Attribute>* attributes)
	{
		if (!member)
		{
			ReportError(ctx, ctx->line, "Out of memory");
			return false;
		}

		Replace(&member->attributes, attributes);

		if (!Expect(ctx, kSymIdentifier))
		{
			ReportError(ctx, ctx->token.line, "Expected label name");
			return false;
		}

		member->name = ctx->token;

		return true;
	}

	// ParseEnum

	bool ParseEnum(Context* ctx, Type* type, XtUt::List<Attribute>* attributes)
	{
		if (!type)
		{
			ReportError(ctx, ctx->line, "Out of memory");
			return false;
		}

		Replace(&type->attributes, attributes);

		type->document = ctx->grimDoc;

		if (!Expect(ctx, kSymIdentifier))
		{
			ReportError(ctx, ctx->token.line, "Expected type name");
			return false;
		}

		type->name = ctx->token;

		if (!Expect(ctx, kSymOpenBrace))
		{
			ReportError(ctx, ctx->token.line, "Expected {");
			return false;
		}

		for(;;)
		{
			if (Check(ctx, kSymCloseBrace))
				break;

			XtUt::List<Attribute> attributes;

			if (!ParseAttributes(ctx, &attributes))
				return false;

			Member* member = XtUt::AppendEx(&type->members, new Member(kMbLabel));

			if (!ParseLabel(ctx, member, &attributes))
				return false;

			if (Expect(ctx, kSymAssign))
			{
				if (!ParseDefaults(ctx, &member->attributes))
					return false;
			}

			if (!Expect(ctx, kSymComma))
				break;
		}

		if (!Expect(ctx, kSymCloseBrace))
		{
			ReportError(ctx, ctx->token.line, "Expected }");
			return false;
		}

		// Eat semicolons on end

		Expect(ctx, kSymSemicolon);

		return true;
	}

	// ParseRoot

	bool ParseRoot(Context* ctx, Dom* dom)
	{
		if (!dom)
		{
			ReportError(ctx, ctx->line, "Out of memory");
			return false;
		}

		for(;;)
		{
			if (Check(ctx, kSymEof))
				break;

			XtUt::List<Attribute> attributes;

			if (!ParseAttributes(ctx, &attributes))
				return false;

			if (!Expect(ctx, kSymIdentifier))
			{
				ReportError(ctx, ctx->token.line, "Expected keyword 'type', 'enum', or 'flags'");
				return false;
			}

			if (ctx->token == "atom")
			{
				if (!ParseAtom(ctx, XtUt::AppendEx(&dom->types, new Type(kTbAtom)), &attributes))
					return false;
			}
			else if (ctx->token == "type")
			{
				if (!ParseType(ctx, XtUt::AppendEx(&dom->types, new Type(kTbStruct)), &attributes))
					return false;
			}
			else if (ctx->token == "enum")
			{
				if (!ParseEnum(ctx, XtUt::AppendEx(&dom->types, new Type(kTbEnum)), &attributes))
					return false;
			}
			else if (ctx->token == "flags")
			{
				if (!ParseEnum(ctx, XtUt::AppendEx(&dom->types, new Type(kTbFlags)), &attributes))
					return false;
			}
			else
			{
				ReportError(ctx, ctx->token.line, "Expected keyword 'type', 'enum', or 'flags'");
				return false;
			}
		}

		return true;
	}
}

bool GrimoireParse(const char* document, int length, Grimoire::Dom* dom, const char* filename, Grimoire::Document* grimDoc, Xt::StatusCallback* status)
{
	Context ctx(document, length, filename, grimDoc, status);

	return ParseRoot(&ctx, dom);
}

bool GrimoireImport(const char* filename, Grimoire::Dom* dom, Xt::StatusCallback* status)
{
	XtSimpleFile f;

	if (!f.Open(filename))
		return false;

	int		length	= f.Length();
	char*	data	= new char[length];

	if (data == 0)
		return false;

	Grimoire::Document* doc;

	if ((doc = AppendEx(&dom->documents, new Grimoire::Document)) == 0)
	{
		delete [] data;
		return false;
	}

	Sz8 fname, ext;

	SplitPath(filename, doc->path, fname, ext);

	doc->filename	= filename;
	doc->document	= data;
	doc->length		= length;

	f.Read(data, length);

	f.Close();

	return GrimoireParse(data, length, dom, filename, doc, status);
}

namespace Grimoire
{
	Attribute* FindAttributeByName(XtUt::List<Attribute>* attributes, const char* name)
	{
		for(size_t i = 0; i < attributes->count; i++)
			if ((*attributes)[i]->name == name)
				return (*attributes)[i];

		return 0;
	}

	Attribute* FindAttributeByName(Member* member, const char* name)
	{
		return FindAttributeByName(&member->attributes, name);
	}

	Attribute* FindAttributeByName(Type* type, const char* name)
	{
		return FindAttributeByName(&type->attributes, name);
	}
};

bool TokenToString(XtString* str, const Grimoire::Token* token)
{
	return str->Set(token->start, token->end - token->start);
}

XtString GetParameterString(XtUt::List<Grimoire::Attribute>* attrs, const char* attributeName, int parameterIndex)
{
	if (Grimoire::Attribute* ga = FindAttributeByName(attrs, attributeName))
	{
		if (Grimoire::Parameter* gp = At(&ga->parameters, parameterIndex))
			return ToSz8(Evaluate(gp->expression));
	}

	return kXtNullString;
}

XtI64 GetParameterInt(XtUt::List<Grimoire::Attribute>* attrs, const char* attributeName, int parameterIndex)
{
	if (Grimoire::Attribute* ga = FindAttributeByName(attrs, attributeName))
	{
		if (Grimoire::Parameter* gp = At(&ga->parameters, parameterIndex))
			return ToI64(Evaluate(gp->expression));
	}

	return 0;
}

double GetParameterFloat(XtUt::List<Grimoire::Attribute>* attrs, const char* attributeName, int parameterIndex)
{
	if (Grimoire::Attribute* ga = FindAttributeByName(attrs, attributeName))
	{
		if (Grimoire::Parameter* gp = At(&ga->parameters, parameterIndex))
			return ToF64(Evaluate(gp->expression));
	}

	return 0;
}

namespace Grimoire
{

	// Variant

	void Variant::Set(const Variant& v)
	{
		if (this != &v)
		{
			switch(v._type)
			{
				case kVariantI64:	Set(v.AsI64());		break;
				case kVariantF64:	Set(v.AsF64());		break;
				case kVariantSz8:	Set(v.AsSz8());		break;
				case kVariantAtom:	Set(v.AsAtom());	break;
				case kVariantAttr:	Set(v.AsAttr());	break;

				default:
					Clear();
				break;
			}
		}
	}

	void Variant::Set(XtI64 i64)
	{
		if (_type != kVariantI64)
		{
			Clear();

			_type = kVariantI64;

			new(XtPlacementNew(), &Data()->i64) XtI64(i64);
		}
		else
			Data()->i64 = i64;
	}

	void Variant::Set(XtF64 f64)
	{
		if (_type != kVariantF64)
		{
			Clear();

			_type = kVariantF64;

			new(XtPlacementNew(), &Data()->f64) XtF64(f64);
		}
		else
			Data()->f64 = f64;
	}

	void Variant::Set(const XtString& sz8)
	{
		if (_type != kVariantSz8)
		{
			Clear();

			_type = kVariantSz8;

			new(XtPlacementNew(), &Data()->sz8) XtString(sz8);
		}
		else
			Data()->sz8 = sz8;
	}

	void Variant::Set(const Atom& atom)
	{
		if (_type != kVariantAtom)
		{
			Clear();

			_type = kVariantAtom;

			new(XtPlacementNew(), &Data()->atom) Atom(atom);
		}
		else
			Data()->atom = atom;
	}

	void Variant::Set(ExpAttributes* attr)
	{
		if (_type != kVariantAttr)
		{
			Clear();

			_type = kVariantAttr;

			new(XtPlacementNew(), &Data()->attr) ExpAttributes*(attr);
		}
		else
			Data()->attr = attr;
	}

	void Variant::Clear()
	{
		if (_type == kVariantSz8)
			Data()->sz8.~Sz();
		else if (_type == kVariantAtom)
			Data()->atom.~Atom();

		_type = kVariantNone;
	}

	// Resolve

	Variant Resolve(const Variant& v)
	{
		if (v.Type() == kVariantAtom)
		{
			const Atom& a = v.AsAtom();

			if (a.name == "false")
				return Variant((XtI64)0);
			else if (a.name == "true")
				return Variant((XtI64)1);
			else if (a.name == "pi")
				return Variant(3.1415926536);
		}

		return v;
	}

	// DetermineExpressionResult

	VariantType DetermineExpressionResult(const Variant& lhs, const Variant& rhs)
	{
		if ((lhs.Type() == kVariantAtom) && (rhs.Type() == kVariantAtom))
			return kVariantAtom;

		if ((lhs.Type() == kVariantAtom) || (rhs.Type() == kVariantAtom))
			return kVariantSz8;

		if ((lhs.Type() == kVariantSz8) || (rhs.Type() == kVariantSz8))
			return kVariantSz8;

		if ((lhs.Type() == kVariantF64) || (rhs.Type() == kVariantF64))
			return kVariantF64;

		if ((lhs.Type() == kVariantI64) || (rhs.Type() == kVariantI64))
			return kVariantI64;

		return kVariantNone;
	}

	// ToI64

	XtI64 ToI64(const Variant& v)
	{
		switch(v.Type())
		{
			case kVariantI64: return v.AsI64();
			case kVariantF64: return (XtI64)v.AsF64();
			case kVariantSz8: return _atoi64(v.AsSz8());
		}

		return 0;
	}

	// ToF64

	XtF64 ToF64(const Variant& v)
	{
		switch(v.Type())
		{
			case kVariantI64: return (double)v.AsI64();
			case kVariantF64: return v.AsF64();
			case kVariantSz8: return atof(v.AsSz8());
		}

		return 0.0;
	}

	// ToSz8

	XtString ToSz8(const Variant& v)
	{
		switch(v.Type())
		{
			case kVariantI64:	return Format("%I64i", v.AsI64());
			case kVariantF64:	return Format("%g", v.AsF64());
			case kVariantSz8:	return v.AsSz8();
			case kVariantAtom:	return v.AsAtom().name;
		}

		return kXtNullString;
	}

	// ToAtom

	Atom ToAtom(const Variant& v)
	{
		return v.AsAtom();
	}

	// ToAttr

	ExpAttributes* ToAttr(const Variant& v)
	{
		return v.AsAttr();
	}

	// Add

	Variant Add(const Variant& lhs, const Variant& rhs)
	{
		switch(DetermineExpressionResult(lhs, rhs))
		{
			case kVariantI64:	return Variant(ToI64(lhs) + ToI64(rhs));
			case kVariantF64:	return Variant(ToF64(lhs) + ToF64(rhs));
			case kVariantSz8:	return Variant(ToSz8(lhs) + ToSz8(rhs));
		}

		return Variant();
	}

	// Sub

	Variant Sub(const Variant& lhs, const Variant& rhs)
	{
		switch(DetermineExpressionResult(lhs, rhs))
		{
			case kVariantI64: return Variant(ToI64(lhs) - ToI64(rhs));
			case kVariantF64: return Variant(ToF64(lhs) - ToF64(rhs));
		}

		return Variant();
	}

	// Mul

	Variant Mul(const Variant& lhs, const Variant& rhs)
	{
		switch(DetermineExpressionResult(lhs, rhs))
		{
			case kVariantI64: return Variant(ToI64(lhs) * ToI64(rhs));
			case kVariantF64: return Variant(ToF64(lhs) * ToF64(rhs));
		}

		return Variant();
	}

	// Div

	Variant Div(const Variant& lhs, const Variant& rhs)
	{
		switch(DetermineExpressionResult(lhs, rhs))
		{
			case kVariantI64: return Variant(ToI64(lhs) / ToI64(rhs));
			case kVariantF64: return Variant(ToF64(lhs) / ToF64(rhs));
		}

		return Variant();
	}

	// Mod

	Variant Mod(const Variant& lhs, const Variant& rhs)
	{
		switch(DetermineExpressionResult(lhs, rhs))
		{
			case kVariantI64: return Variant(ToI64(lhs) % ToI64(rhs));
			case kVariantF64: return Variant(fmod(ToF64(lhs), ToF64(rhs)));
		}

		return Variant();
	}

	// Access

	Variant Access(const Variant& lhs, const Variant& rhs)
	{
		switch(DetermineExpressionResult(lhs, rhs))
		{
			case kVariantAtom: return Variant(Atom(ToAtom(lhs).name + "." + ToAtom(rhs).name));
		}

		return Variant();
	}

	// Call

	Variant Call(const Atom& func, const Variant& rhs)
	{
		if (!func)
			return Variant();

		if (func.name == "sin")
			return Variant(sin(ToF64(rhs)));
		else if (func.name == "asin")
			return Variant(asin(ToF64(rhs)));
		else if (func.name == "cos")
			return Variant(cos(ToF64(rhs)));
		else if (func.name == "acos")
			return Variant(acos(ToF64(rhs)));
		else if (func.name == "tan")
			return Variant(tan(ToF64(rhs)));
		else if (func.name == "atan")
			return Variant(atan(ToF64(rhs)));
		else if (func.name == "sqrt")
			return Variant(sqrt(ToF64(rhs)));
		else if (func.name == "abs")
			return Variant(fabs(ToF64(rhs)));

		return Variant();
	}

	// Evaluate

	Variant EvaluateWithoutResolve(Expression* expr)
	{
		if (!expr)
			return Variant();

		switch(expr->token.symbol)
		{
			// Identifiers

			case kSymIdentifier:
			{
				XtString str;

				if (TokenToString(&str, &expr->token))
					return Variant(Atom(str));
			}
			break;

			// Constants

			case kSymLiteralInteger:
			{
				XtString str;

				if (TokenToString(&str, &expr->token))
					return Variant(_atoi64(str));
			}
			break;

			case kSymLiteralFloat:
			{
				XtString str;

				if (TokenToString(&str, &expr->token))
					return Variant(atof(str));
			}
			break;

			case kSymLiteralString:
			return Variant(XtsDecodeString(expr->token.start, expr->token.end - expr->token.start));

			case kSymAttributes:
			return Variant((ExpAttributes*)expr);

			// Math operators

			case kSymPlus:
			return Add(Evaluate(expr->left), Evaluate(expr->right));

			case kSymMinus:
			return Sub(Evaluate(expr->left), Evaluate(expr->right));
			
			case kSymMultiply:
			return Mul(Evaluate(expr->left), Evaluate(expr->right));

			case kSymDivide:
			return Div(Evaluate(expr->left), Evaluate(expr->right));

			case kSymModulo:
			return Mod(Evaluate(expr->left), Evaluate(expr->right));

			// Named parameter

			case kSymColon:
			return EvaluateWithoutResolve(expr->right);

			// Member accessor

			case kSymDot:
			return Access(EvaluateWithoutResolve(expr->left), EvaluateWithoutResolve(expr->right));

			// Function call

			case kSymOpenParen:
			return Call(EvaluateWithoutResolve(expr->left).AsAtom(), Evaluate(expr->right));

			default:
				DebugLn("Evaluate: Unsupported Symbol %i", expr->token.symbol);
			break;
		}

		return Variant();
	}

	Variant Evaluate(Expression* expr)
	{
		return Resolve(EvaluateWithoutResolve(expr));
	}

}