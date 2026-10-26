#include "stdafx.h"
#include "TreeRendererSettings.h"

using namespace SpeedtreeBillboard;

C_TreeRendererSettings::C_TreeRendererSettings()
:
	m_AtlasWidth(0),
	m_AtlasHeight(0),
	m_MinBillboardWidth(0),
	m_MinBillboardHeight(0),
	m_Lighting(E_Lighting_None)
{
	m_Filename = "billboard";
	m_FileFormat = "png";
	m_DefinitionFilename = "billboard.bbd";
}

C_TreeRendererSettings::~C_TreeRendererSettings()
{
}

bool C_TreeRendererSettings::ReadSettingsFromXml(
	C_TreeRendererSettings& settings,
	const Util::C_String& filename)
{
	Sys::C_XmlDom dom;
	if (!dom.Read(filename.Chars()))
		return false;

	{
		Sys::C_XmlDomBaseNode* pAtlasNode = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "atlas");
		if (!pAtlasNode)
		{
			LOGFATAL(g_Log, "Failed to find atlas information in %s\n", filename.Chars());
			return false;
		}

		const Sys::C_XmlAttribute *pWidthAttribute = ((Sys::C_XmlDomElement*)(pAtlasNode))->GetAttributes().GetAttributeByName("width");
		const Sys::C_XmlAttribute *pHeightAttribute = ((Sys::C_XmlDomElement*)(pAtlasNode))->GetAttributes().GetAttributeByName("height");

		if (!pWidthAttribute || !pHeightAttribute)
		{
			LOGFATAL(g_Log, "Failed to find atlas dimensions in %s\n", filename.Chars());
		}

		settings.m_AtlasWidth = atoi(pWidthAttribute->value.Chars());
		settings.m_AtlasHeight = atoi(pHeightAttribute->value.Chars());

		if ((settings.m_AtlasWidth & (settings.m_AtlasWidth-1)) || (settings.m_AtlasHeight & (settings.m_AtlasHeight-1)))
			LOGWARN(SpeedtreeBillboard::g_Log, "Texture atlas has a non power of 2 size\n");

		const Sys::C_XmlAttribute *pFilenameAttribute = ((Sys::C_XmlDomElement*)(pAtlasNode))->GetAttributes().GetAttributeByName("name");
		if (pFilenameAttribute)
			settings.m_Filename = pFilenameAttribute->value;

		const Sys::C_XmlAttribute *pFormatAttribute = ((Sys::C_XmlDomElement*)(pAtlasNode))->GetAttributes().GetAttributeByName("format");
		if (pFormatAttribute)
			settings.m_FileFormat = pFormatAttribute->value;

		const Sys::C_XmlAttribute *pDefinitionFileAttribute = ((Sys::C_XmlDomElement*)(pAtlasNode))->GetAttributes().GetAttributeByName("definition");
		if (pDefinitionFileAttribute)
			settings.m_DefinitionFilename = pDefinitionFileAttribute->value;
	}

	{
		Sys::C_XmlDomBaseNode* pBillboardNode = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "billboard");
		if (!pBillboardNode)
		{
			LOGFATAL(g_Log, "Failed to find billboard information in %s\n", filename.Chars());
			return false;
		}

		const Sys::C_XmlAttribute *pWidthAttribute = ((Sys::C_XmlDomElement*)(pBillboardNode))->GetAttributes().GetAttributeByName("minwidth");
		const Sys::C_XmlAttribute *pHeightAttribute = ((Sys::C_XmlDomElement*)(pBillboardNode))->GetAttributes().GetAttributeByName("minheight");

		if (!pWidthAttribute || !pHeightAttribute)
		{
			LOGFATAL(g_Log, "Failed to find atlas dimensions in %s\n", filename.Chars());
			return false;
		}

		settings.m_MinBillboardWidth = atoi(pWidthAttribute->value.Chars());
		settings.m_MinBillboardHeight = atoi(pHeightAttribute->value.Chars());

		if (settings.m_MinBillboardWidth>settings.m_AtlasWidth || settings.m_MinBillboardHeight>settings.m_AtlasHeight)
		{
			LOGFATAL(g_Log, "The billboard size is greater than the atlas size\n", filename.Chars());
			return false;
		}
	}

	{
		Sys::C_XmlDomBaseNode* pLightingNode = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "lighting");
		if (!pLightingNode)
		{
			LOGWARN(g_Log, "Failed to find lighting node in %s\n", filename.Chars());
		}
		else
		{
			const Sys::C_XmlAttribute *pLightingTypeAttribute = ((Sys::C_XmlDomElement*)(pLightingNode))->GetAttributes().GetAttributeByName("type");
			if (!pLightingTypeAttribute)
				LOGWARN(g_Log, "Failed to find lighting type attribute in %s\n", filename.Chars());

			if (pLightingTypeAttribute->value == "none")
				settings.m_Lighting = E_Lighting_None;
			else if (pLightingTypeAttribute->value == "vertexcolour")
				settings.m_Lighting = E_Lighting_VertexColours;
			else
				LOGWARN(g_Log, "Unknown lighting mode %s", pLightingTypeAttribute->value.Chars());
		}
	}

	{
		Sys::C_XmlDomBaseNode* pConditionsNode = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "conditions");
		if (!pConditionsNode)
		{
			LOGWARN(g_Log, "Failed to find conditions information in %s\n", filename.Chars());
			LOGWARN(g_Log, "Using 'sunny' condition\n");
			settings.m_Conditions.PushBack((U32)E_Condition_Sunny);
		}
		else
		{
			const Sys::C_XmlDomBaseNode::Array& childNodes = pConditionsNode->GetChildren();
			for (U32 i=0; i<(U32)childNodes.Size(); ++i)
			{
				const Sys::C_XmlAttribute *pWeatherTypeNode = ((Sys::C_XmlDomElement*)(childNodes[i]))->GetAttributes().GetAttributeByName("type");

				if (!pWeatherTypeNode)
				{
					LOGWARN(g_Log, "Failed to read weather type information\n");
					continue;
				}
				if (pWeatherTypeNode->value == "sunny")
					settings.m_Conditions.PushBack((U32)E_Condition_Sunny);
				else if (pWeatherTypeNode->value == "night")
					settings.m_Conditions.PushBack((U32)E_Condition_Night);
				else if (pWeatherTypeNode->value == "overcast")
					settings.m_Conditions.PushBack((U32)E_Condition_Overcast);
				else if (pWeatherTypeNode->value == "snow")
					settings.m_Conditions.PushBack((U32)E_Condition_Snow);
				else
					LOGWARN(g_Log, "Unknown weather type %s", pWeatherTypeNode->value.Chars());
			}
		}
	}

	{
		Sys::C_XmlDomBaseNode* pViewsNode = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "views");
		if (!pViewsNode)
		{
			LOGFATAL(g_Log, "Failed to find view information in %s\n", filename.Chars());
			return false;
		}

		const Sys::C_XmlDomBaseNode::Array& childNodes = pViewsNode->GetChildren();
		settings.m_Views.Resize(childNodes.Size());
		for (U32 i=0; i<(U32)childNodes.Size(); ++i)
		{
			const Sys::C_XmlAttribute *pAzimuthNode = ((Sys::C_XmlDomElement*)(childNodes[i]))->GetAttributes().GetAttributeByName("azimuth");
			const Sys::C_XmlAttribute *pElevationNode = ((Sys::C_XmlDomElement*)(childNodes[i]))->GetAttributes().GetAttributeByName("elevation");

			if (!pAzimuthNode || !pElevationNode)
			{
				LOGWARN(g_Log, "Found incomplete view in %s\n", filename.Chars());
				settings.m_Views[i].m_Azimuth = 0;
				settings.m_Views[i].m_Elevation = 0;
			}

			if (pAzimuthNode->value.Chars()[0] == '-')
				settings.m_Views[i].m_Azimuth = -atoi(&(pAzimuthNode->value.Chars()[1]));
			else
				settings.m_Views[i].m_Azimuth = atoi(pAzimuthNode->value.Chars());

			if (pElevationNode->value.Chars()[0] == '-')
				settings.m_Views[i].m_Elevation = -atoi(&(pElevationNode->value.Chars()[1]));
			else
				settings.m_Views[i].m_Elevation = atoi(pElevationNode->value.Chars());
		}

		if (childNodes.Size() == 0)
		{
			LOGFATAL(g_Log, "Failed to find view information in %s\n", filename.Chars());
			return false;
		}
	}

	return true;
}


bool C_TreeRendererSettings::WriteSettingsToXml(
	const C_TreeRendererSettings& settings,
	const Util::C_String& filename)
{
	Sys::C_FileSystem& fileSystem = Sys::C_FileSystem::Instance();

	Sys::I_File *xmlOutputFile = fileSystem.Open(
		filename,
		Sys::E_FileAccess_Write);
	if (!xmlOutputFile)
	{
		LOGERROR( g_Log, "C_TreeRenderer::WriteSettingsToXml() - Could not open file %s\n", filename.Chars());
		return false;
	}

	Sys::TextSerialiser stream(*xmlOutputFile);

	stream << "<sptbillboardsettings>\n";
	stream << "\t<atlas width=\"" << Util::unsignedIntegerToCString(settings.m_AtlasWidth) << "\" height=\"" << Util::unsignedIntegerToCString(settings.m_AtlasHeight) << "\" name=\""
	       << settings.m_Filename << "\" format=\"" << settings.m_FileFormat << "\" definition=\"" << settings.m_DefinitionFilename << "\"/>\n";
	stream << "\t<billboard minWidth=\"" << Util::unsignedIntegerToCString(settings.m_MinBillboardWidth) << "\" minHeight=\"" << Util::unsignedIntegerToCString(settings.m_MinBillboardHeight) << "\"/>\n";

	{
		Util::C_String lightingString =
			(settings.m_Lighting == E_Lighting_None) ? "none" :
			(settings.m_Lighting == E_Lighting_VertexColours) ? "vertexcolour" : "none";
		
		stream << "\t<lighting type=\"" << lightingString << "\"/>\n";
	}

	stream << "\t<conditions>\n";
	for (U32 i=0; i<(U32)settings.m_Conditions.Size(); ++i)
	{
		Util::C_String weatherString =
			(settings.m_Conditions[i] == E_Condition_Sunny) ? "sunny" :
			(settings.m_Conditions[i] == E_Condition_Night) ? "night" :
			(settings.m_Conditions[i] == E_Condition_Overcast) ? "overcast" :
			(settings.m_Conditions[i] == E_Condition_Snow) ? "snow" : "sunny";

		stream << "\t\t<weather type=\"" << weatherString << "\"/>\n";
	}
	stream << "\t</conditions>\n";

	stream << "\t<views>\n";
	for (U32 i=0; i<(U32)settings.m_Views.Size(); ++i)
		stream << "\t\t<angle azimuth=\"" << Util::signedIntegerToCString((S32)settings.m_Views[i].m_Azimuth) << "\" elevation=\"" << Util::signedIntegerToCString((S32)settings.m_Views[i].m_Elevation) << "\"/>\n";

	stream << "\t</views>\n";
	stream << "</sptbillboardsettings>\n";

	xmlOutputFile->Overflow();
	xmlOutputFile->Flush();

	delete xmlOutputFile;

	return true;
}