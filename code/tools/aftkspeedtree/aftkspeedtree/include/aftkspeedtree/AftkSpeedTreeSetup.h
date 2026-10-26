#ifdef _MSC_VER_
#pragma once
#endif

#ifndef AFTK_SPEEDTREE_SETUP_H
#define AFTK_SPEEDTREE_SETUP_H

//#define SHOW_TREE_DEBUG_MESSAGES

#include "System/Serialise/TextSerialise.h"
#include "System/xml/XmlDom.h"

namespace AftkSpeedTree
{
	struct Flag
	{
		Util::C_String name;
		Util::C_String value;	
	};

	typedef Ut::UtVector< Ut::DynamicVector<Flag> > FlagArray;

	struct SpeedTreeSetup
	{
		SpeedTreeSetup() :
			m_bGenerateBranches(true),
			m_bDoubleSidedFronds(false),
			m_bGenerateFronds(true), 
			m_bGenerateLeaves(true) {};
		~SpeedTreeSetup() {};

		const SpeedTreeSetup& operator =(const SpeedTreeSetup& r)
		{
			m_bDoubleSidedFronds = r.m_bDoubleSidedFronds;

			m_bGenerateBranches = r.m_bGenerateBranches;
			m_bGenerateFronds = r.m_bGenerateFronds;
			m_bGenerateLeaves = r.m_bGenerateLeaves;
			m_flagOverrides = r.m_flagOverrides;

			return *this;
		}

		bool			m_bDoubleSidedFronds;

		bool			m_bGenerateBranches;
		bool			m_bGenerateFronds;
		bool			m_bGenerateLeaves;
		FlagArray		m_flagOverrides;
	};

	static bool WriteSetupXmlFile(const char* pFileName, const SpeedTreeSetup& setup)
	{
		std::ofstream stream(pFileName);

		if (!stream.is_open())
		{
			//LOGERROR( g_Log, "C_TreeRenderer::WriteSettingsToXml() - Could not open file %s\n", pFileName);
			return false;
		}

		stream << "<aftkspeedtreesetup>\n";
		stream << "\t<doublesidedfronds value=\"" << ((setup.m_bDoubleSidedFronds)  ? "true" : "false") << "\"/>\n";
		stream << "\t<generatebranches value=\"" << ((setup.m_bGenerateBranches)  ? "true" : "false") << "\"/>\n";
		stream << "\t<generatefronds value=\"" << ((setup.m_bGenerateFronds)  ? "true" : "false") << "\"/>\n";
		stream << "\t<generateleaves value=\"" << ((setup.m_bGenerateLeaves)  ? "true" : "false") << "\"/>\n";
		stream << "\t<flags>\n";
		for (S32 i=0; i<setup.m_flagOverrides.Size(); ++i)
			stream << "\t\t<flag name=\"" << setup.m_flagOverrides[i].name.Chars() << "\" value=\"" << setup.m_flagOverrides[i].value.Chars() << "\"/>\n";
		stream << "\t</flags>\n";
		stream << "</aftkspeedtreesetup>\n";

		stream.close();
		return true;

	}

	static bool ReadXmlAttrS32(const Sys::C_XmlDom& dom, const char *pNodeName, const char *pAttrName, S32& value)
	{
		const Sys::C_XmlDomBaseNode* pXmlNode = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), pNodeName);
		if (pXmlNode)
		{
			const Sys::C_XmlAttribute *pValueAttr = ((Sys::C_XmlDomElement*)(pXmlNode))->GetAttributes().GetAttributeByName(pAttrName);
			if (pValueAttr)
			{
				if (pValueAttr->value[0] == '-')
				{
					const char* pValueChars = pValueAttr->value.Chars();
					value = -1 * atoi(++pValueChars);
				}
				else
					value = atoi(pValueAttr->value.Chars());

				return true;
			}
		}
		return false;
	}

	static bool ReadXmlAttrBool(const Sys::C_XmlDom& dom, const char *pNodeName, const char *pAttrName, bool& value)
	{
		const Sys::C_XmlDomBaseNode* pXmlNode = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), pNodeName);
		if (pXmlNode)
		{
			const Sys::C_XmlAttribute *pValueAttr = ((Sys::C_XmlDomElement*)(pXmlNode))->GetAttributes().GetAttributeByName(pAttrName);
			if (pValueAttr)
			{
				value = (pValueAttr->value == "true");
				return true;
			}
		}
		return false;
	}

	static bool ReadXmlAttrString(const Sys::C_XmlDom& dom, const char *pNodeName, const char *pAttrName, std::string& value)
	{
		const Sys::C_XmlDomBaseNode* pXmlNode = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), pNodeName);
		if (pXmlNode)
		{
			const Sys::C_XmlAttribute *pValueAttr = ((Sys::C_XmlDomElement*)(pXmlNode))->GetAttributes().GetAttributeByName(pAttrName);
			if (pValueAttr)
			{
				value = pValueAttr->value.Chars();
				return true;
			}
		}
		return false;

	}

	static bool ReadSetupXmlFile(const char* pFileName, SpeedTreeSetup& setup)
	{
		Sys::C_XmlDom dom;
		if (!dom.Read(pFileName))
			return false;

		ReadXmlAttrBool(dom, "doublesidedfronds", "value", setup.m_bDoubleSidedFronds);
		ReadXmlAttrBool(dom, "generatebranches", "value", setup.m_bGenerateBranches);
		ReadXmlAttrBool(dom, "generatefronds", "value", setup.m_bGenerateFronds);
		ReadXmlAttrBool(dom, "generateleaves", "value", setup.m_bGenerateLeaves);

		const Sys::C_XmlDomBaseNode* pXmlNode = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "flags");
		if (pXmlNode)
		{
			for (S32 i=0; i<pXmlNode->GetChildren().Size(); ++i)
			{
				const Sys::C_XmlDomElement* pChild = (const Sys::C_XmlDomElement*) pXmlNode->GetChildren()[i];
				const Sys::C_XmlAttribute *pNameAttr = pChild->GetAttributes().GetAttributeByName("name");
				const Sys::C_XmlAttribute *pValueAttr = pChild->GetAttributes().GetAttributeByName("value");

				if (!pNameAttr || !pValueAttr)
					continue;

				Flag flag = {pNameAttr->value, pValueAttr->value};
				setup.m_flagOverrides.PushBack(flag);
			}
		}
		return true;
	};
}

#endif


