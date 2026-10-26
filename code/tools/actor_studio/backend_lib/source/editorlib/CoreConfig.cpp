#include "stdafx.h"

#include <System/Xml/Xml.h>

#include <Tools_Shared/Utility/MacroExpander.h>

#include <EditorLib/CoreConfig.h>
#include <EditorLib/Core.h>

namespace EditorLib
{

bool C_CoreConfig::Load(const char* configFilePath_)
{
	configPath = Misc::ExtractDirectory(configFilePath_);

	Sys::C_XmlDom dom;
	if (!dom.Read(configFilePath_))
	{
		LOGWARN(C_Core::Instance().GetLogger(), "Unable to read config file %s", configFilePath_);
		return false;
	}

	const Sys::C_XmlDomElement *pXtDbNode = (Sys::C_XmlDomElement*) Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "xtdb");
	if (pXtDbNode)
	{
		const char* pValue = pXtDbNode->GetAttributes().GetValueByName("path");
		if (pValue)
			xtDbPath = pValue;	
	}
	else
		LOGWARN(C_Core::Instance().GetLogger(), "No xtdb node found config file...");


	const Sys::C_XmlDomElement *pA3dCacheNode = 
		(Sys::C_XmlDomElement*)Sys::C_XmlDomUtility::FindXmlNodeByName(
			dom.GetRootNodePtr(), "a3d_cache");
	if (pA3dCacheNode)
	{
		const char* pValue = pA3dCacheNode->GetAttributes().GetValueByName("path");
		if (pValue)
			a3dCachePath = pValue;	

		pValue = pA3dCacheNode->GetAttributes().GetValueByName("tool");
		if (pValue)
			conversionToolPath = pValue;

		pValue = pA3dCacheNode->GetAttributes().GetValueByName("params");
		if (pValue)
			conversionToolParams = pValue;
		
		pValue = pA3dCacheNode->GetAttributes().GetValueByName("metaqueryparams");
		if (pValue)
			conversionToolMetaQueryParams = pValue;

		conversionToolParams = conversionToolParams.Replace("&quot;", "\"");
		conversionToolMetaQueryParams = conversionToolMetaQueryParams.Replace("&quot;", "\"");
	}
	else
		LOGWARN(C_Core::Instance().GetLogger(), "No a3dCache node was found in the config file...");

	//Ubercopy
	const Sys::C_XmlDomElement* pUbercopy = (Sys::C_XmlDomElement*)Sys::C_XmlDomUtility::FindXmlNodeByName(
		dom.GetRootNodePtr(), "ubercopy");
	if (!pUbercopy) return false;
	{
		const char* pValue = pUbercopy->GetAttributes().GetValueByName("path");
		if (pValue)
			uberCopyToolPath = pValue;	
	}

	//alienbrain
	const Sys::C_XmlDomElement* pAlienbrain = (Sys::C_XmlDomElement*)Sys::C_XmlDomUtility::FindXmlNodeByName(
		dom.GetRootNodePtr(), "alienbrain");
	if (!pAlienbrain) return false;
	{
		const char* pValue = pAlienbrain->GetAttributes().GetValueByName("path");
		if (pValue)
			alienbrainConfigPath = pValue;	
	}

	//crash dumps
	const Sys::C_XmlDomElement* pDump = (Sys::C_XmlDomElement*)Sys::C_XmlDomUtility::FindXmlNodeByName(
		dom.GetRootNodePtr(), "dump");
	if (pDump)
	{
		const char* pValue = pDump->GetAttributes().GetValueByName("path");
		if (pValue)
			dumpPath = pValue;	
	}

	//Paths
	{
		const Sys::C_XmlDomElement* pPaths = (Sys::C_XmlDomElement*)Sys::C_XmlDomUtility::FindXmlNodeByName(
			dom.GetRootNodePtr(), "paths");
		if (!pPaths) return false;

		const char* pValue = pPaths->GetAttributes().GetValueByName("root");
		if (!pValue) return false;
		rootProjectPath = pValue;

		pValue = pPaths->GetAttributes().GetValueByName("root_temp");
		if (!pValue) return false;
		rootTempPath = pValue;

		pValue = pPaths->GetAttributes().GetValueByName("root_working");
		if (!pValue) return false;
		rootWorkingPath = pValue;
	}

	//Get local machine name.
	char tmp[128];
	DWORD size = sizeof(tmp);
	GetComputerName(tmp, &size);
	computerName = tmp;

	//Macro expansion
	{
		projectName = "%BIZARRE_PROJECT%";
		Tools::C_MacroExpander::Instance().Expand(projectName);
		Tools::C_MacroExpander::Instance().Expand(xtDbPath);
		Tools::C_MacroExpander::Instance().Expand(a3dCachePath);
		Tools::C_MacroExpander::Instance().Expand(conversionToolPath);
		Tools::C_MacroExpander::Instance().Expand(conversionToolParams);

		Tools::C_MacroExpander::Instance().Expand(uberCopyToolPath);
		Tools::C_MacroExpander::Instance().Expand(alienbrainConfigPath);

		Tools::C_MacroExpander::Instance().Expand(rootProjectPath);
		Tools::C_MacroExpander::Instance().Expand(rootTempPath);
		Tools::C_MacroExpander::Instance().Expand(rootWorkingPath);
	}

	return true;
}

}

