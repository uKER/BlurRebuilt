#include "stdafx.h"

#include "system/xml/Xml.h"
#include "Tools_Shared/Xml/XmlDom.h"
#include "Tools_Shared/Utility/MacroExpander.h"

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorApp/Config.h>


namespace Editor
{
extern Debug::Logger& g_Log;

bool LoadConfig(const char* configPath, const char* macroPath, EditorConfig & config)
{
	Sys::C_XmlDom dom;
	if (!dom.Read(configPath))
	{
		LOGWARN(g_Log, "Unable to read config file %s", configPath);
		return false;
	}

	//crash dumps
	const Sys::C_XmlDomElement* pDump = (Sys::C_XmlDomElement*)Sys::C_XmlDomUtility::FindXmlNodeByName(
		dom.GetRootNodePtr(), "dump");
	if (pDump)
	{
		const char* pValue = pDump->GetAttributes().GetValueByName("path");
		if (pValue)
			config.dumpPath = pValue;	
	}

	//Populate macros
	{
		Sys::C_XmlDom macroDom;
		if (macroDom.Read(macroPath) && macroDom.GetRootNodePtr())
		{
			const Sys::C_XmlDomElement *pMacroGroupNode = 
				(Sys::C_XmlDomElement*)Sys::C_XmlDomUtility::FindXmlNodeByName(
					macroDom.GetRootNodePtr(), "vars");
			if (pMacroGroupNode)
				Tools::C_MacroExpander::Instance().ParseMacroGroupsXML(pMacroGroupNode, "vars");
		}
		else
			LOGWARN(g_Log, "Unable to read macro config file %s", configPath);
	}

	return true;
}

}