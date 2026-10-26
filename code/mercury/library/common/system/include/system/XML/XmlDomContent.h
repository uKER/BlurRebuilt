#ifndef XML_DOM_CONTENT_H
#define XML_DOM_CONTENT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Sys
{
	// callbacks for reading a file using XMLParser.cpp
	class C_XmlDomContent : public C_XmlContent
	{
	public:
		virtual ~C_XmlDomContent();

		bool StartDocument();

		bool EndDocument();

		bool StartElement(const Util::C_String& name, const Sys::C_XmlAttributeList& attributes);

		bool EndElement(const Util::C_String& name);

		bool Read(const char* filename);

	protected:
		Ut::UtList<C_XmlDomBaseNode*> m_stack;
	};
}

#endif
