#include "SystemPch.h"

#include <system/xml/XmlDom.h>

using namespace Sys;

C_XmlDomContent::~C_XmlDomContent()
{
	while (m_stack.Size())
	{
		C_XmlDomElement* node = (C_XmlDomElement*)m_stack.Back();
		m_stack.PopBack();
		node->DecRef();
	}
}

bool C_XmlDomContent::StartDocument() 
{ 
	return true;
}

bool C_XmlDomContent::EndDocument() 
{
	return true; 
}

bool C_XmlDomContent::StartElement(
	const Util::C_String& _name, 
	const Sys::C_XmlAttributeList& attributes)
{
	Util::C_String name = _name.Lower();
	
	C_XmlDomElement* newElement = new C_XmlDomElement();
	newElement->GetName() = name;

	newElement->GetAttributes() = attributes;

	C_XmlDomBaseNode *bn = (C_XmlDomBaseNode*)newElement;

	// If item already on stack, add new item as a child to it
	if (m_stack.Size() > 0)
	{
		C_XmlDomElement& node = (C_XmlDomElement&)*m_stack.Back();

		bn->AddRef();
		node.AddChild(bn);
	}

	m_stack.PushBack(bn);

	return true; 
}

bool C_XmlDomContent::EndElement(const Util::C_String& _name)
{
	C_XmlDomElement* node = (C_XmlDomElement*)m_stack.Back();

	Util::C_String name = _name.Lower();

	assert(name == node->GetName());

	m_stack.PopBack();

	node->DecRef();

	return true;
}
