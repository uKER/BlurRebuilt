#include "SystemPch.h"

#include <system/xml/XmlParser.h>
#include <system/xml/XmlDomDefinitions.h>
#include <system/xml/XmlDomBaseNode.h>
#include <system/xml/XmlDomElement.h>

using namespace Sys;

C_XmlDomBaseNode::C_XmlDomBaseNode(const C_XmlDomBaseNode& node) : m_type(E_XmlDomNodeType_Base), m_refCount(1), m_parent(0)
{
	m_type = node.m_type;
	m_name = node.m_name;

	const C_XmlDomBaseNode::Array& children = node.m_children;

	for (int i=0; i<children.Size(); i++)
	{
		C_XmlDomBaseNode* child;
		
		if (children[i]->GetType() == E_XmlDomNodeType_Base)
			child = new C_XmlDomBaseNode(*children[i]);
		else
			child = new C_XmlDomElement((C_XmlDomElement&)(*children[i]));

		this->AddChild(child);
	}
}

C_XmlDomBaseNode::~C_XmlDomBaseNode()
{
	for (int iCount=0; iCount < m_children.Size(); iCount++)
	{
		m_children[iCount]->m_parent = 0;
		m_children[iCount]->DecRef();
	}
}

int C_XmlDomBaseNode::AddRef(void)
{
	return ++m_refCount;
}

void C_XmlDomBaseNode::DecRef(void)
{
	m_refCount--;
	
	if (m_refCount == 0)
		delete this;
}

const Sys::E_XmlDomNodeType C_XmlDomBaseNode::GetType() const
{
	return m_type; 
}

Util::C_String& C_XmlDomBaseNode::GetName()
{
	return m_name;
}

const Util::C_String& C_XmlDomBaseNode::GetName() const
{
	return m_name;
}

void C_XmlDomBaseNode::AddChild(C_XmlDomBaseNode* pChild)
{
	pChild->m_parent = this;
	m_children.PushBack(pChild);
}

const C_XmlDomBaseNode::Array& C_XmlDomBaseNode::GetChildren() const
{
	return m_children;
}

const C_XmlDomBaseNode* C_XmlDomBaseNode::GetParent() const
{
	return m_parent;
}
