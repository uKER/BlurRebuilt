#include "SystemPch.h"
#include <core/core.h>
#include <System/XML/XmlParser.h>
#include <System/DebugLog.h>

namespace Sys
{

using namespace Util;

static Debug::Logger &g_Logger = Debug::GetLogger("system.XMLParser");

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// XmlAttributes

C_XmlAttributeList::C_XmlAttributeList() :
		_noAttributes(0),
		_lineNo(0)
{
}

C_XmlAttributeList::~C_XmlAttributeList()
{
}

int C_XmlAttributeList::Elements() const
{
	return _noAttributes;
}

const C_XmlAttribute* C_XmlAttributeList::GetAttributeByIndex(int index) const
{
	assert(_noAttributes<=kMaxAttributes);
	if ((index >= 0) && (index < _noAttributes))
		return &_attributes[index];

	return 0;
}

const C_XmlAttribute* C_XmlAttributeList::GetAttributeByName(const char* name) const
{
	for(int i = 0; i < _noAttributes; i++)
		if (stricmp(_attributes[i].name, name) == 0)
			return &_attributes[i];

	return 0;
}

C_XmlAttribute* C_XmlAttributeList::GetAttributeByIndex(int index)
{
	assert(_noAttributes<=kMaxAttributes);
	if ((index >= 0) && (index < _noAttributes))
		return &_attributes[index];

	return 0;
}

C_XmlAttribute* C_XmlAttributeList::GetAttributeByName(const char* name)
{
	for(int i = 0; i < _noAttributes; i++)
		if (stricmp(_attributes[i].name, name) == 0)
			return &_attributes[i];

	return 0;
}

const char* C_XmlAttributeList::GetKeyByIndex(int index) const
{
	if ((index >= 0) && (index < _noAttributes))
		return _attributes[index].name;

	return 0;
}

const char* C_XmlAttributeList::GetValueByIndex(int index) const
{
	if ((index >= 0) && (index < _noAttributes))
		return _attributes[index].value;

	return 0;
}

const char* C_XmlAttributeList::GetValueByName(const char* name) const
{
	for(int i = 0; i < _noAttributes; i++)
		if (stricmp(_attributes[i].name, name) == 0)
			return _attributes[i].value;

	return 0;
}

void C_XmlAttributeList::Add(const C_String& name, const C_String& value)
{
	if (_noAttributes >= kMaxAttributes)
		return;

	C_XmlAttribute& attribute = _attributes[_noAttributes++];

	attribute.name	= name;
	attribute.value	= value;
}

void C_XmlAttributeList::Clear()
{
	_noAttributes = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// XmlParse

bool XttXmlIsWhiteSpace(char c)
{
	return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n');
}

bool XttXmlIsIdentifierStart(char c)
{
	return (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '_') || (c == '.'));
}

bool XttXmlIsIdentifierBody(char c)
{
	return (((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'Z')) || (c == '_') || (c == '-') || (c == '.'));
}

E_XmlResult XmlParse(C_XmlContent& content, const char* document, int length)
{
	enum E_XmlParseState
	{
		E_XmlParseState_Characters,
		E_XmlParseState_StartElement,
		E_XmlParseState_StartElementBody,
		E_XmlParseState_ElementBody,
		E_XmlParseState_EndElementNoContent,
		E_XmlParseState_EndElementBodyStart,
		E_XmlParseState_EndElementBody,
		E_XmlParseState_EndElementBodyEnd,
		E_XmlParseState_AttributeNameBody,
		E_XmlParseState_AttributeEquals,
		E_XmlParseState_AttributeValueStart,
		E_XmlParseState_AttributeValueBody,
		E_XmlParseState_AttributeValueEnd,
		E_XmlParseState_CommentStart,
		E_XmlParseState_CommentStart2,
		E_XmlParseState_CommentBody,
		E_XmlParseState_CommentEnd,
		E_XmlParseState_CommentEnd2,
		E_XmlParseState_HeaderBody
	};

	const int kXmlMaxStack = 64;
	C_String stack[kXmlMaxStack];
	E_XmlParseState state = E_XmlParseState_Characters;
	C_XmlAttributeList attributes;
	int stackPtr = 0;
	int line = 1;
	char newLine = '\0';
	const char* start = 0;
	C_String name;
	C_String value;

	if (!content.StartDocument())
		return E_XmlResult_Cancelled;

	attributes.SetLineNo(line);

	for(; length > 0; document++, length--)
	{
		if (((*document == '\r') || (*document == '\n')) && (*document != newLine))
		{
			newLine = (*document == '\r') ? '\n' : '\r';
			line++;

			attributes.SetLineNo(line);
		}
		else
			newLine = '\0';

		switch(state)
		{
		case E_XmlParseState_Characters:
			if (*document == '<')
				state = E_XmlParseState_StartElement;
			else if (!XttXmlIsWhiteSpace(*document))
			{
				LOGWARN(g_Logger, "Xml(%i): Only whitespace allowed outside of elements!\n", line);
				return E_XmlResult_Failed;
			}
			break;

		case E_XmlParseState_StartElement:
			if (XttXmlIsIdentifierStart(*document))
			{
				start = document;
				state = E_XmlParseState_StartElementBody;
				attributes.Clear();
			}
			else if (*document == '/')
				state = E_XmlParseState_EndElementBodyStart;
			else if (*document == '!')
				state = E_XmlParseState_CommentStart;
			else if (*document == '?')
				state = E_XmlParseState_HeaderBody;
			else
			{
				LOGWARN(g_Logger, "Xml(%i): Identifier name contains invalid character!\n", line);
				return E_XmlResult_Failed;
			}
			break;

		case E_XmlParseState_StartElementBody:
			if (!XttXmlIsIdentifierBody(*document))
			{
				assert(start);
				if (XttXmlIsWhiteSpace(*document))
				{
					//						stack[stackPtr] = C_String((CHAR8*)start, (int)(size_t)(document - start));
					stack[stackPtr].CopyFrom((CHAR8*)start, (int)(size_t)(document - start));

					state = E_XmlParseState_ElementBody;
				}
				else if (*document == '/')
				{
					//						stack[stackPtr] = C_String((CHAR8*)start, (int)(size_t)(document - start));
					stack[stackPtr].CopyFrom((CHAR8*)start, (int)(size_t)(document - start));
					state = E_XmlParseState_EndElementNoContent;
				}
				else if (*document == '>')
				{
					//						stack[stackPtr] = C_String((CHAR8*)start, (int)(size_t)(document - start));
					stack[stackPtr].CopyFrom((CHAR8*)start, (int)(size_t)(document - start));

					if (!content.StartElement(stack[stackPtr], attributes))
						return E_XmlResult_Cancelled;

					stackPtr++;
					state = E_XmlParseState_Characters;
				}
				else
				{
					LOGWARN(g_Logger, "Xml(%i): Identifier name contains invalid character!\n", line);
					return E_XmlResult_Failed;
				}
			}
			break;

		case E_XmlParseState_EndElementBodyStart:
			assert(start);
			if (XttXmlIsIdentifierStart(*document))
			{
				start = document;
				state = E_XmlParseState_EndElementBody;
			}
			else
			{
				LOGWARN(g_Logger, "Xml(%i): Identifier name contains invalid character!\n", line);
				return E_XmlResult_Failed;
			}
			break;

		case E_XmlParseState_EndElementBody:
			if (!XttXmlIsIdentifierBody(*document))
			{
				assert(start);
				if (*document == '>')
				{
					//						name = C_String((CHAR8*)start, (int)(size_t)(document - start));
					name.CopyFrom((CHAR8*)start, (int)(size_t)(document - start));
					stackPtr--;

					assert(stackPtr>=0);
					if (name == stack[stackPtr])
					{
						if (!content.EndElement(stack[stackPtr]))
							return E_XmlResult_Cancelled;

						state = E_XmlParseState_Characters;
					}
					else
					{
						LOGWARN(g_Logger, "Xml(%i): End element name doesn't match start element name!\n", line);
						return E_XmlResult_Failed;
					}
				}
				else if (XttXmlIsWhiteSpace(*document))
				{
					//						name = C_String((CHAR8*)start, (int)(size_t)(document - start));
					name.CopyFrom((CHAR8*)start, (int)(size_t)(document - start));
					state = E_XmlParseState_EndElementBodyEnd;
				}
				else
				{
					LOGWARN(g_Logger, "Xml(%i): Identifier name contains invalid character!\n", line);
					return E_XmlResult_Failed;
				}
			}
			break;

		case E_XmlParseState_EndElementBodyEnd:
			if (*document == '>')
			{
				if (name == stack[stackPtr])
				{
					if (!content.EndElement(stack[stackPtr]))
						return E_XmlResult_Cancelled;

					stackPtr--;
					state = E_XmlParseState_Characters;
				}
				else
				{
					LOGWARN(g_Logger, "Xml(%i): End element name doesn't match start element name!\n", line);
					return E_XmlResult_Failed;
				}
			}
			else if (!XttXmlIsWhiteSpace(*document))
			{
				LOGWARN(g_Logger, "Xml(%i): Expected '>'!\n", line);
				return E_XmlResult_Failed;
			}
			break;

		case E_XmlParseState_ElementBody:
			if (XttXmlIsIdentifierStart(*document))
			{
				start = document;
				state = E_XmlParseState_AttributeNameBody;
			}
			else if (*document == '/')
				state = E_XmlParseState_EndElementNoContent;
			else if (*document == '>')
			{
				if (!content.StartElement(stack[stackPtr], attributes))
					return E_XmlResult_Cancelled;

				stackPtr++;
				state = E_XmlParseState_Characters;
			}
			else if (!XttXmlIsWhiteSpace(*document))
			{
				LOGWARN(g_Logger, "Xml(%i): Expected attribute or '>'!\n", line);
				return E_XmlResult_Failed;
			}
			break;

		case E_XmlParseState_EndElementNoContent:
			if (*document == '>')
			{
				state = E_XmlParseState_Characters;

				if (!content.StartElement(stack[stackPtr], attributes))
					return E_XmlResult_Cancelled;

				if (!content.EndElement(stack[stackPtr]))
					return E_XmlResult_Cancelled;
			}
			else
			{
				LOGWARN(g_Logger, "Xml(%i): Expected '>'!\n", line);
				return E_XmlResult_Failed;
			}
			break;

		case E_XmlParseState_AttributeNameBody:
			if (!XttXmlIsIdentifierBody(*document))
			{
				assert(start);
				if (*document == '=')
				{
					//						name = C_String((CHAR8*)start, (int)(size_t)(document - start));
					name.CopyFrom((CHAR8*)start, (int)(size_t)(document - start));
					state = E_XmlParseState_AttributeValueStart;
				}
				else if (XttXmlIsWhiteSpace(*document))
				{
					//						name = C_String((CHAR8*)start, (int)(size_t)(document - start));
					name.CopyFrom((CHAR8*)start, (int)(size_t)(document - start));
					state = E_XmlParseState_AttributeEquals;
				}
				else
				{
					LOGWARN(g_Logger, "Xml(%i): Expected '='!\n", line);
					return E_XmlResult_Failed;
				}
			}
			break;

		case E_XmlParseState_AttributeEquals:
			if (*document == '=')
				state = E_XmlParseState_AttributeValueStart;
			else if (!XttXmlIsWhiteSpace(*document))
			{
				LOGWARN(g_Logger, "Xml(%i): Expected '='!\n", line);
				return E_XmlResult_Failed;
			}
			break;

		case E_XmlParseState_AttributeValueStart:
			if (*document == '\"')
			{
				start = document + 1;
				state = E_XmlParseState_AttributeValueBody;
			}
			else if (!XttXmlIsWhiteSpace(*document))
			{
				LOGWARN(g_Logger, "Xml(%i): Expected '\"'!\n", line);
				return E_XmlResult_Failed;
			}
			break;

		case E_XmlParseState_AttributeValueBody:
			if (*document == '\"')
			{
				assert(start);

				//					value = C_String((CHAR8*)start, (int)(size_t)(document - start));
				value.CopyFrom((CHAR8*)start, (int)(size_t)(document - start));
				attributes.Add(name, value);
				state = E_XmlParseState_AttributeValueEnd;
			}
			else if ((*document == '\r') || (*document == '\n'))
			{
				LOGWARN(g_Logger, "Xml(%i): Attribute value contains newline!\n", line);
				return E_XmlResult_Failed;
			}
			break;

		case E_XmlParseState_AttributeValueEnd:
			if (XttXmlIsWhiteSpace(*document))
				state = E_XmlParseState_ElementBody;
			else if (*document == '/')
				state = E_XmlParseState_EndElementNoContent;
			else if (*document == '>')
			{
				if (!content.StartElement(stack[stackPtr], attributes))
					return E_XmlResult_Cancelled;

				stackPtr++;
				state = E_XmlParseState_Characters;
			}
			else
			{
				LOGWARN(g_Logger, "Xml(%i): Expected whitespace between attributes!\n", line);
				return E_XmlResult_Failed;
			}
			break;

		case E_XmlParseState_HeaderBody:
			if (*document == '?')
				state = E_XmlParseState_CommentEnd2;
			break;

		case E_XmlParseState_CommentStart:
			if (*document == '-')
				state = E_XmlParseState_CommentStart2;
			else
			{
				LOGWARN(g_Logger, "Xml(%i): Identifier name contains invalid character!  (Malformed comment maybe?)\n", line);
				return E_XmlResult_Failed;
			}
			break;

		case E_XmlParseState_CommentStart2:
			if (*document == '-')
				state = E_XmlParseState_CommentBody;
			else
			{
				LOGWARN(g_Logger, "Xml(%i): Identifier name contains invalid character!  (Malformed comment maybe?)\n", line);
				return E_XmlResult_Failed;
			}
			break;

		case E_XmlParseState_CommentBody:
			if (*document == '-')
				state = E_XmlParseState_CommentEnd;
			break;

		case E_XmlParseState_CommentEnd:
			if (*document == '-')
				state = E_XmlParseState_CommentEnd2;
			else
				state = E_XmlParseState_CommentBody;
			break;

		case E_XmlParseState_CommentEnd2:
			if (*document == '>')
				state = E_XmlParseState_Characters;
			else
				state = E_XmlParseState_CommentBody;
			break;
		}
	}

	if (stackPtr > 0)
	{
		LOGWARN(g_Logger, "Xml(%i): End of file, but elements are still open!\n", line);
		return E_XmlResult_Failed;
	}

	if (state != E_XmlParseState_Characters)
	{
		LOGWARN(g_Logger, "Xml(%i): Unexpected end of file!\n", line);
		return E_XmlResult_Failed;
	}

	if (!content.EndDocument())
		return E_XmlResult_Cancelled;

	return E_XmlResult_Success;
}


}; // namespace Util
