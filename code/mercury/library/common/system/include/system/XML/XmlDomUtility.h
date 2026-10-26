#ifndef XML_DOM_UTILITY_H
#define XML_DOM_UTILITY_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Sys
{
	class C_XmlDomUtility
	{
	public:
		//Recursive node search.
		static const C_XmlDomBaseNode* FindXmlNodeByName(
			const C_XmlDomBaseNode* pNode, 
			const Util::C_String& name);

		static const C_XmlDomBaseNode* FindXmlNodeByNameAttribute(
			const C_XmlDomBaseNode* pNode, 
			const Util::C_String& name);

		static const C_XmlDomBaseNode* FindXmlNodeByTypeNameAttributeSubStr(
			const C_XmlDomBaseNode* pNode, 
			const Util::C_String& name);

		//Recursive node search.
		static C_XmlDomBaseNode* FindXmlNodeByName(
			C_XmlDomBaseNode* pNode,
			const Util::C_String& name);

		static C_XmlDomBaseNode* FindXmlNodeByNameAttribute(
			C_XmlDomBaseNode* pNode, 
			const Util::C_String& name);

		static C_XmlDomBaseNode* FindXmlNodeByTypeNameAttribute(
			C_XmlDomBaseNode* pNode, 
			const Util::C_String& name);

		static C_XmlDomBaseNode* FindXmlNodeByTypeNameAttributeSubStr(
			C_XmlDomBaseNode* pNode, 
			const Util::C_String& name);

		static bool XpathEvaluate(const Sys::C_XmlDomBaseNode* node, const Util::C_String& expression, Ut::Vector<const Sys::C_XmlDomBaseNode*>& results);

	private:
		static bool XpathEvaluate(const Sys::C_XmlDomBaseNode* node, Ut::Vector<Util::C_String> splitExpression, U32 index, Ut::Vector<const Sys::C_XmlDomBaseNode*>& results);
	};
}

#endif
