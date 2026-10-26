#include "stdafx.h"

#include "system/xml/Xml.h"
#include <dialogueman/Config.h>
#include <dialogueman/Misc/StringUtil.h>
#include <Tools_Shared/Utility/MacroExpander.h>

namespace DMan
{

Voice Voice::defaultVoice = Voice();

Voice::Voice() :
	speaker(defaultVoice.speaker),
	speed(defaultVoice.speed),
	volume(defaultVoice.volume)
{}

bool LoadConfig(const char* configPath, const char* macroPath, DialogueManConfig& config)
{
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
			LOGWARN(g_Log, "Unable to read macro config file %s", macroPath);
	}

	//Read config vars
	Voice::defaultVoice.speaker = "LH Michael";
	Voice::defaultVoice.speed = 0;
	Voice::defaultVoice.volume = 100;
					   
	Sys::C_XmlDom dom;
	if (!dom.Read(configPath))
	{
		LOGWARN(g_Log, "Unable to read config file %s", configPath);
		return false;
	}

	const Sys::C_XmlDomElement* pXtDbNode = 
		(Sys::C_XmlDomElement*) Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "xtdb");
	if (pXtDbNode)
	{
		const char* pValue = pXtDbNode->GetAttributes().GetValueByName("path");
		if (pValue)
			config.XtDbPath = pValue;	
	}
	else
	{
		LOGWARN(g_Log, "No xtdb node found in config file...");
		return false;
	}
		
	const Sys::C_XmlDomElement* pLocIdNode = 
		(Sys::C_XmlDomElement*) Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "locid");
	if (pLocIdNode)
	{
		const char* pValue = pLocIdNode->GetAttributes().GetValueByName("name");
		if (pValue)
			config.locId = pValue;	
	}
	else
	{
		LOGWARN(g_Log, "No locid node found in config file...");
		return false;
	}

	const Sys::C_XmlDomElement* pExportNode = 
		(Sys::C_XmlDomElement*) Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "export");
	if (pExportNode)
	{
		const char* pValue = pExportNode->GetAttributes().GetValueByName("path");
		if (pValue)
			config.exportPath = pValue;	
	}
	else
	{
		LOGWARN(g_Log, "No export node found in config file...");
		return false;
	}

	const Sys::C_XmlDomElement* pAudioXMLNode = 
		(Sys::C_XmlDomElement*) Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "audioxml");
	if (pAudioXMLNode)
	{
		const char* pValue = pAudioXMLNode->GetAttributes().GetValueByName("path");
		if (pValue)
			config.audioXMLPath = pValue;	
	}
	else
	{
		LOGWARN(g_Log, "No audioXML node found in config file...");
		return false;
	}

	const Sys::C_XmlDomElement* pBankPCXMLNode = 
		(Sys::C_XmlDomElement*) Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "bankpc");
	if (pBankPCXMLNode)
	{
		const char* pValue = pBankPCXMLNode->GetAttributes().GetValueByName("path");
		if (pValue)
			config.bankPCPath = pValue;	
	}
	else
	{
		LOGWARN(g_Log, "No bankpc node found in config file...");
		return false;
	}

	const Sys::C_XmlDomElement* pBank360XMLNode = 
		(Sys::C_XmlDomElement*) Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "bank360");
	if (pBank360XMLNode)
	{
		const char* pValue = pBank360XMLNode->GetAttributes().GetValueByName("path");
		if (pValue)
			config.bank360Path = pValue;	
	}
	else
	{
		LOGWARN(g_Log, "No bank360 node found in config file...");
		return false;
	}

	const Sys::C_XmlDomElement* pBankPS3XMLNode = 
		(Sys::C_XmlDomElement*) Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "bankps3");
	if (pBankPS3XMLNode)
	{
		const char* pValue = pBankPS3XMLNode->GetAttributes().GetValueByName("path");
		if (pValue)
			config.bankPS3Path = pValue;	
	}
	else
	{
		LOGWARN(g_Log, "No bankps3 node found in config file...");
		return false;
	}

	const Sys::C_XmlDomElement* pBuild360XMLNode = 
		(Sys::C_XmlDomElement*) Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "build360");
	if (pBuild360XMLNode)
	{
		const char* pValue = pBuild360XMLNode->GetAttributes().GetValueByName("path");
		if (pValue)
			config.build360Path = pValue;	
	}
	else
	{
		LOGWARN(g_Log, "No build360 node found in config file...");
		return false;
	}

	const Sys::C_XmlDomElement* pTTSNode = 
		(Sys::C_XmlDomElement*) Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "tts");
	if (pTTSNode)
	{
		//Get the Path value
		const char* pPath = pTTSNode->GetAttributes().GetValueByName("path");
		if (pPath)
			config.tts.path = pPath;

		//Get the Filename value
		const char* pFileName = pTTSNode->GetAttributes().GetValueByName("fileName");
		if (pFileName)
			config.tts.fileName = pFileName;

		//Get the Default Voice values
		const char* pDefaultSpeaker = pTTSNode->GetAttributes().GetValueByName("defaultSpeaker");
		if (pDefaultSpeaker)
			Voice::defaultVoice.speaker = pDefaultSpeaker;

		const char* pDefaultSpeed = pTTSNode->GetAttributes().GetValueByName("defaultSpeed");
		if (pDefaultSpeed)
			Voice::defaultVoice.speed = ToS32(pDefaultSpeed);

		const char* pDefaultVolume = pTTSNode->GetAttributes().GetValueByName("defaultVolume");
		if (pDefaultVolume)
			Voice::defaultVoice.volume = ToU32(pDefaultVolume);

		//Get the voice definition of each character
		for (int i = 0; i < pTTSNode->GetChildren().Size(); i++)
		{
			const Sys::C_XmlDomBaseNode* pChild = pTTSNode->GetChildren()[i];
			if ((pChild->GetName() == "voice") && (pChild->GetType() == Sys::E_XmlDomNodeType_Element))
			{
				Voice voice;
				Sys::C_XmlDomElement* pChildAsEl = (Sys::C_XmlDomElement*)pChild;
				Util::C_String character = pChildAsEl->GetAttributes().GetValueByName("character");
										
				voice.speaker = pChildAsEl->GetAttributes().GetValueByName("speaker");
				voice.speed = ToS32(pChildAsEl->GetAttributes().GetValueByName("speed"));
				voice.volume = ToU32(pChildAsEl->GetAttributes().GetValueByName("volume"));

				config.tts.voices[character.Upper()] = voice;		
			}
		}

	}
	else
	{
		LOGWARN(g_Log, "No tts node found in config file...");
		return false;
	}

	//Macro expansion
	{
		Tools::C_MacroExpander::Instance().Expand(config.XtDbPath);
		XtString dbPath = config.XtDbPath.Chars();
		CleanUpPath(dbPath, false, true);
		config.XtDbPath = dbPath;
	}

	return true;
}

bool LoadPreferences(const char* prefsPath, DialogueManPreferences& prefs)
{
	return true;
}


}