#include "stdafx.h"

#include <tools_shared/shim/ShimErrorHelpers.h>
#include <Helpers/XmlHelper.h>

#include <system/xml/XmlDom.h>

bool XmlHelper::ParseDependsOn(const Sys::C_XmlDomBaseNode* node)
{
	if (node->GetType() != Sys::E_XmlDomNodeType_Element)
		return false;
	
	Sys::C_XmlDomElement* element = (Sys::C_XmlDomElement*)node;

	const char* dependsOn = element->GetAttributes().GetValueByName("shim_depends_on");

	if (!dependsOn)
		return true;

	Ut::Vector<const Sys::C_XmlDomBaseNode*> results;

	if (dependsOn[0] == '!')
		Sys::C_XmlDomUtility::XpathEvaluate(node, dependsOn+1, results);
	else
		Sys::C_XmlDomUtility::XpathEvaluate(node, dependsOn, results);

	if (!results.Size())
		return true;

	if (results.Size()>1)
	{
		BEGIN_ALWAYS ERROR_ERROR("Too many results for dependancy search") END_RETURN_FALSE;
	}

	if (!ParseDependsOn(results[0]))
		return false;

	const char* value = ((Sys::C_XmlDomElement*)results[0])->GetAttributes().GetValueByName("value");

	if (dependsOn[0] == '!')
	{
		if (value && stricmp(value,"true")==0)
			return false;
		else
			return true;
	}
	else
	{
		if (value && stricmp(value,"true")==0)
			return true;
		else
			return false;
	}
}

const Sys::C_XmlDomBaseNode* XmlHelper::FindXmlNodeByShimTag(const Sys::C_XmlDomBaseNode* node, const Util::C_String& tag)
{
	if (node->GetType() == Sys::E_XmlDomNodeType_Element)
	{
		const Sys::C_XmlAttribute* xmlTag = ((const Sys::C_XmlDomElement*)node)->GetAttributes().GetAttributeByName("shim_tag");

		if (xmlTag && xmlTag->value == tag)
		{
			const Sys::C_XmlAttribute* value = ((const Sys::C_XmlDomElement*)node)->GetAttributes().GetAttributeByName("value");
			
			return node;
		}
	}

	const Sys::C_XmlDomBaseNode::Array& children = node->GetChildren();

	for (Sys::C_XmlDomBaseNode::Array::ConstIterator it=children.Begin(); it != children.End(); ++it)
	{
		const Sys::C_XmlDomBaseNode* childNode = FindXmlNodeByShimTag(*it, tag);
		if (childNode)
			return childNode;
	}

	return 0;
}
