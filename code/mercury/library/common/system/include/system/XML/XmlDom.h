#ifndef XML_DOM_H
#define XML_DOM_H

#ifdef _MSC_VER
#pragma once
#endif

//Include everything we need to start using our XML DOM
#include <system/xml/XmlParser.h>

#include <system/xml/XmlDomDefinitions.h>
#include <system/xml/XmlDomBaseNode.h>
#include <system/xml/XmlDomElement.h>
#include <system/xml/XmlDomContent.h>
#include <system/xml/XmlDomUtility.h>

namespace Sys
{
	class I_BinaryStream;
	class TextSerialiser;

	class C_XmlDom : public C_XmlDomContent
	{
	public:
		C_XmlDom();

		virtual ~C_XmlDom();

		void SetRootNodePtr(C_XmlDomBaseNode* pRoot);

		C_XmlDomBaseNode* GetRootNodePtr();

		const C_XmlDomBaseNode* GetRootNodePtr() const;

		bool Read(const char* filename);

		bool Write(const char* filename) const;

		bool Serialise(Sys::I_BinaryStream& xml, Sys::C_XmlDomBaseNode& node) const;

	private:
		void XmlSerialiseIndent(Sys::TextSerialiser& xml, U32 depth) const;

		void XmlSerialiseNewLine(Sys::TextSerialiser& xml) const;

		void XmlSerialiseAttribute(Sys::TextSerialiser& xml, const Sys::C_XmlAttribute& attr) const;

		bool XmlSerialiseElement(Sys::TextSerialiser& xml, U32 depth, Sys::C_XmlDomElement& node) const;

		bool XmlSerialise(Sys::TextSerialiser& xml, U32 depth, Sys::C_XmlDomBaseNode& node) const;

		C_XmlDomBaseNode*	m_pRootNode;
	};
}

#endif
