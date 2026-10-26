#include "SystemPch.h"

//Include everything we need to start using our XML DOM
#include <system/xml/XmlParser.h>

#include <system/xml/XmlDomDefinitions.h>
#include <system/xml/XmlDomBaseNode.h>
#include <system/xml/XmlDomElement.h>

using namespace Sys;

C_XmlDomElement::C_XmlDomElement() : C_XmlDomBaseNode(E_XmlDomNodeType_Element)
{
	m_type = E_XmlDomNodeType_Element;
}

C_XmlDomElement::C_XmlDomElement(const C_XmlDomElement& element) : C_XmlDomBaseNode(element)
{
	const Sys::C_XmlAttributeList& attributes = element.m_attributes;
	for (int i=0; i<attributes.Elements(); i++)
		m_attributes.Add(attributes.GetKeyByIndex(i), attributes.GetValueByIndex(i));
}

C_XmlAttributeList& C_XmlDomElement::GetAttributes()
{
	return m_attributes;
}

const C_XmlAttributeList& C_XmlDomElement::GetAttributes() const
{
	return m_attributes;
}
