#include "SystemPch.h"

#include <system/xml/XmlDom.h>
#include <system/xml/XmlDomUtility.h>

using namespace Sys;

//Recursive node search.
C_XmlDomBaseNode* C_XmlDomUtility::FindXmlNodeByName(
	C_XmlDomBaseNode* pNode, 
	const Util::C_String& name)
{
	if ((pNode->GetName() == name) && (pNode->GetType() == Sys::E_XmlDomNodeType_Element))
		return pNode;

	for (int i = 0; i < pNode->GetChildren().Size(); i++)
	{
		C_XmlDomBaseNode* pResult = FindXmlNodeByName(pNode->GetChildren()[i], name);
		if (pResult)
			return pResult;
	}

	return 0;
}

//Recursive node search.
const C_XmlDomBaseNode* C_XmlDomUtility::FindXmlNodeByName(
	const C_XmlDomBaseNode* pNode, 
	const Util::C_String& name)
{
	return FindXmlNodeByName(const_cast<C_XmlDomBaseNode*>(pNode), name);
}

C_XmlDomBaseNode* C_XmlDomUtility::FindXmlNodeByNameAttribute(
	C_XmlDomBaseNode* pNode, 
	const Util::C_String& name)
{
	if ((pNode->GetType() == Sys::E_XmlDomNodeType_Element))
	{
		const Sys::C_XmlAttribute* pAttr = 
			((C_XmlDomElement*)pNode)->GetAttributes().GetAttributeByName("name");
		if (pAttr && pAttr->value == name)
			return pNode;
	}

	for (int i = 0; i < pNode->GetChildren().Size(); i++)
	{
		C_XmlDomBaseNode* pResult = FindXmlNodeByNameAttribute(pNode->GetChildren()[i], name);
		if (pResult)
			return pResult;
	}

	return 0;
}

const C_XmlDomBaseNode* C_XmlDomUtility::FindXmlNodeByNameAttribute(
	const C_XmlDomBaseNode* pNode, 
	const Util::C_String& name)
{
	return FindXmlNodeByNameAttribute(const_cast<C_XmlDomBaseNode*>(pNode), name);
}

const C_XmlDomBaseNode* C_XmlDomUtility::FindXmlNodeByTypeNameAttributeSubStr(
	const C_XmlDomBaseNode* pNode, 
	const Util::C_String& name)
{
	return FindXmlNodeByTypeNameAttributeSubStr(const_cast<C_XmlDomBaseNode*>(pNode), name);
}

C_XmlDomBaseNode* C_XmlDomUtility::FindXmlNodeByTypeNameAttribute(
	C_XmlDomBaseNode* pNode, 
	const Util::C_String& name)
{
	if ((pNode->GetType() == Sys::E_XmlDomNodeType_Element))
	{
		const Sys::C_XmlAttribute* pAttr = 
			((C_XmlDomElement*)pNode)->GetAttributes().GetAttributeByName("type_name");
		if (pAttr && pAttr->value == name)
			return pNode;
	}

	for (int i = 0; i < pNode->GetChildren().Size(); i++)
	{
		C_XmlDomBaseNode* pResult = FindXmlNodeByTypeNameAttribute(pNode->GetChildren()[i], name);
		if (pResult)
			return pResult;
	}

	return 0;
}

C_XmlDomBaseNode* C_XmlDomUtility::FindXmlNodeByTypeNameAttributeSubStr(
	C_XmlDomBaseNode* pNode, 
	const Util::C_String& name)
{
	if ((pNode->GetType() == Sys::E_XmlDomNodeType_Element))
	{
		const Sys::C_XmlAttribute* pAttr = 
			((C_XmlDomElement*)pNode)->GetAttributes().GetAttributeByName("type_name");
		if (pAttr && pAttr->value.ISearch(name) != -1)
			return pNode;
	}

	for (int i = 0; i < pNode->GetChildren().Size(); i++)
	{
		C_XmlDomBaseNode* pResult = FindXmlNodeByTypeNameAttributeSubStr(pNode->GetChildren()[i], name);
		if (pResult)
			return pResult;
	}

	return 0;
}

bool Sys::C_XmlDomUtility::XpathEvaluate(const Sys::C_XmlDomBaseNode* node, const Util::C_String& expression, Ut::Vector<const Sys::C_XmlDomBaseNode*>& results)
{
	Ut::Vector<Util::C_String> splitExpression;
	expression.Tokenise(splitExpression, '/');

	return XpathEvaluate(node, splitExpression, 0, results);
}

bool Sys::C_XmlDomUtility::XpathEvaluate(const Sys::C_XmlDomBaseNode* node, Ut::Vector<Util::C_String> splitExpression, U32 index, Ut::Vector<const Sys::C_XmlDomBaseNode*>& results)
{
	if (node == 0)
		return false;

	if (index == splitExpression.Size())
		return true;

	Util::C_String currentExpression = splitExpression[index];

	if (currentExpression == "..")
	{
		return XpathEvaluate(node->GetParent(), splitExpression, index+1, results);
	}

	int lBracket = currentExpression.Search("[");
	int rBracket = currentExpression.Search("]",lBracket);
	
	Util::C_String nodeName;
	Util::C_String arrayIndex;

	if (lBracket >= 0)
	{
		nodeName = currentExpression(0,lBracket);
		arrayIndex = currentExpression(lBracket+1,rBracket);
	}
	else
	{
		nodeName = currentExpression;
		arrayIndex = "";
	}

	bool nodeMatch = true;

	if (nodeName == "**" || nodeName == "*" || nodeName == node->GetName())
	{
		if (arrayIndex != "")
		{
			if (arrayIndex[0] == '@')
			{
				int equals = arrayIndex.Search("=");
				Util::C_String attribute = arrayIndex(1,equals);
				Util::C_String value = arrayIndex(equals+2,arrayIndex.Length()-1);

				const char* attributeValue = ((Sys::C_XmlDomElement*)node)->GetAttributes().GetValueByName(attribute);

				if (!(value && attributeValue && stricmp(value.Chars(), attributeValue)==0))
					nodeMatch = false;
			}
			else
			{
				// We only support attribute indexing at the moment, not collections, so continue
				return true;
			}
		}
	}
	else
		nodeMatch = false;

	if (nodeMatch)
	{
		const Sys::C_XmlDomBaseNode::Array& children = node->GetChildren();
		for (Sys::C_XmlDomBaseNode::Array::ConstIterator it = children.Begin(); it != children.End(); ++it)
		{
			// If we find our version of the descendant wildcard, recurse into two paths -- one maintaining the **
			// and one trying to find the remainder of the expression
			if (currentExpression == "**")
			{
				if (!XpathEvaluate(*it, splitExpression, index, results))
					return false;

				if (!XpathEvaluate(*it, splitExpression, index+1, results))
					return false;
				
				continue;
			}

			if (!XpathEvaluate(*it, splitExpression, index+1, results))
				return false;
		}

		if (splitExpression.Size() == index+1)
			results.PushBack(node);
	}

	return true;
}
