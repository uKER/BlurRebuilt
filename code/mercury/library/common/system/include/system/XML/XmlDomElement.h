#ifndef XML_DOM_ELEMENT_H
#define XML_DOM_ELEMENT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Sys
{
	class C_XmlDomElement : public C_XmlDomBaseNode
	{
	public:
		C_XmlDomElement(const C_XmlDomElement& node);
		C_XmlDomElement();

		C_XmlAttributeList& GetAttributes();
		
		const C_XmlAttributeList& GetAttributes() const;

	private:
		C_XmlAttributeList m_attributes;
	};
}

#endif
