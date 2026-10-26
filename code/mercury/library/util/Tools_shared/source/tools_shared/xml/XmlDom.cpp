#include "stdafx.h"
#include <tools_shared/xml/XmlDom.h>

using namespace Tools;

XmlBaseNode::~XmlBaseNode()
{
	for (int iCount=0; iCount < m_children.Size(); iCount++)
		m_children[iCount]->DecRef();
}

int XmlBaseNode::AddRef(void)
{
	return ++m_refCount;
}

void XmlBaseNode::DecRef(void)
{
	m_refCount--;
	
	if (m_refCount == 0)
		delete this;
}

const XmlBaseNode::E_NodeType XmlBaseNode::GetType() const
{
	return m_type; 
}

Util::C_String& XmlBaseNode::GetName()
{
	return m_name;
}

const Util::C_String& XmlBaseNode::GetName() const
{
	return m_name;
}

void XmlBaseNode::AddChild(XmlBaseNode* pChild)
{
	m_children.PushBack(pChild);
}

const XmlBaseNode::Array& XmlBaseNode::GetChildren() const
{
	return m_children;
}


XmlElement::XmlElement() : XmlBaseNode(E_NodeType_Element)
{
	m_type = E_NodeType_Element;
}

XmlContent::~XmlContent()
{
	while (m_stack.Size())
	{
		XmlElement* node = (XmlElement*)m_stack.Back();
		m_stack.PopBack();
		node->DecRef();
	}
}

bool XmlContent::StartDocument() 
{ 
	return true;
}

bool XmlContent::EndDocument() 
{
	return true; 
}

bool XmlContent::StartElement(const Util::C_String& _name, const Sys::C_XmlAttributeList& attributes)
{
	Util::C_String name = _name.Lower();
	
	XmlElement* newElement = new XmlElement();
	newElement->GetName() = name;

	newElement->attributes = attributes;

	XmlBaseNode *bn = (XmlBaseNode*)newElement;

	// If item already on stack, add new item as a child to it
	if (m_stack.Size() > 0)
	{
		XmlElement& node = (XmlElement&)*m_stack.Back();

		bn->AddRef();
		node.AddChild(bn);
	}

	m_stack.PushBack(bn);

	return true; 
}

bool XmlContent::EndElement(const Util::C_String& _name)
{
	XmlElement* node = (XmlElement*)m_stack.Back();

	Util::C_String name = _name.Lower();

	assert(name == node->GetName());

	m_stack.PopBack();

	node->DecRef();

	return true;
}


XmlDom::XmlDom() : XmlContent()
{
	m_pRootNode = 0;
}

bool XmlDom::Read(const char* filename)
{
	Sys::I_File *pFile = Sys::C_FileSystem::Instance().Open(filename);
	
	if (!pFile)
	{
//		LOGERROR(g_Log, "XmlDom::Read - Unable to load file '%s'", filename);
		return false;
	}

	//Read file into buffer
	U32 c = (U32)pFile->Length();
	char *pBuffer = new char[c];
	pFile->Read(pBuffer, c);

	//Create a root node
	m_pRootNode = new XmlBaseNode();
	m_pRootNode->GetName() = "root";
	m_pRootNode->AddRef();
	m_stack.PushBack(m_pRootNode);

	//Parse xml data
	XmlParse(*this, pBuffer, c);

	//Cleanup
	delete pBuffer;
	delete pFile;

	return true;
}

XmlBaseNode* XmlDom::GetRootNodePtr()
{
	return m_pRootNode;
}

const XmlBaseNode* XmlDom::GetRootNodePtr() const
{
	return m_pRootNode;
}



//Recursive node search.
XmlBaseNode* XmlUtility::FindXmlNodeByName(XmlBaseNode* pNode, const Util::C_String& name)
{
	if ((pNode->GetName() == name) && (pNode->GetType() == XmlBaseNode::E_NodeType_Element))
		return pNode;

	for (int i = 0; i < pNode->GetChildren().Size(); i++)
	{
		XmlBaseNode* pResult = FindXmlNodeByName(pNode->GetChildren()[i], name);
		if (pResult)
			return pResult;
	}

	return 0;
}

//Recursive node search.
const XmlBaseNode* XmlUtility::FindXmlNodeByName(const XmlBaseNode* pNode, const Util::C_String& name)
{
	return FindXmlNodeByName(const_cast<XmlBaseNode*>(pNode), name);
}


XmlBaseNode* XmlUtility::FindXmlNodeByNameAttribute(XmlBaseNode* pNode, const Util::C_String& name)
{
	if ((pNode->GetType() == XmlBaseNode::E_NodeType_Element))
	{
		const Sys::C_XmlAttribute* pAttr = 
			((XmlElement*)pNode)->attributes.GetAttributeByName("name");
		if (pAttr && pAttr->value == name)
			return pNode;
	}

	for (int i = 0; i < pNode->GetChildren().Size(); i++)
	{
		XmlBaseNode* pResult = FindXmlNodeByNameAttribute(pNode->GetChildren()[i], name);
		if (pResult)
			return pResult;
	}

	return 0;
}

XmlBaseNode* XmlUtility::FindXmlNodeByTypeNameAttribute(XmlBaseNode* pNode, const Util::C_String& name)
{
	if ((pNode->GetType() == XmlBaseNode::E_NodeType_Element))
	{
		const Sys::C_XmlAttribute* pAttr = 
			((XmlElement*)pNode)->attributes.GetAttributeByName("type_name");
		if (pAttr && pAttr->value == name)
			return pNode;
	}

	for (int i = 0; i < pNode->GetChildren().Size(); i++)
	{
		XmlBaseNode* pResult = FindXmlNodeByTypeNameAttribute(pNode->GetChildren()[i], name);
		if (pResult)
			return pResult;
	}

	return 0;
}

XmlBaseNode* XmlUtility::FindXmlNodeByTypeNameAttributeSubStr(XmlBaseNode* pNode, const Util::C_String& name)
{
	if ((pNode->GetType() == XmlBaseNode::E_NodeType_Element))
	{
		const Sys::C_XmlAttribute* pAttr = 
			((XmlElement*)pNode)->attributes.GetAttributeByName("type_name");
		if (pAttr && pAttr->value.ISearch(name) != -1)
			return pNode;
	}

	for (int i = 0; i < pNode->GetChildren().Size(); i++)
	{
		XmlBaseNode* pResult = FindXmlNodeByTypeNameAttributeSubStr(pNode->GetChildren()[i], name);
		if (pResult)
			return pResult;
	}

	return 0;
}
