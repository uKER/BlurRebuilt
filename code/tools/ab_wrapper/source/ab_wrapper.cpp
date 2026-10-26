#include "stdafx.h"
#include <Ab_Wrapper.h>

#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Serialise.h>
#include <System/Xml/Xml.h>
#include <System/Serialise/TextSerialise.h>
#include <system/misc/BtlStringUtil.h>

#include <Tools_Shared/Utility/MacroExpander.h>

namespace Vcs
{

C_Ab::C_Ab(
	Debug::Logger& logger,
	bool(*pAbScriptExecCallback)(const char* cmdline, const char* cmdParams, const char* workingPath))
: m_logger(logger), m_pAbScriptExecCallback(pAbScriptExecCallback)
{
	BIZ_ASSERT(m_pAbScriptExecCallback);
}

C_Ab::~C_Ab()
{
}

bool C_Ab::Initialise(const char* config)
{
	Sys::C_XmlDom dom;
	if (!dom.Read(config))
	{
		LOGERROR(m_logger, "Failed to read alienbrain configuration file");
		return false;
	}

	const Sys::C_XmlDomElement* pScriptElement = (Sys::C_XmlDomElement*)Sys::C_XmlDomUtility::FindXmlNodeByName(
		dom.GetRootNodePtr(), "shim_ab_script");
	
	if (!pScriptElement)
	{
		LOGERROR(m_logger, "Failed to obtain script path from alienbrain configuration file");
		return false;
	}

	m_config.scriptPath = pScriptElement->GetAttributes().GetValueByName("value");
	Tools::C_MacroExpander::Instance().Expand(m_config.scriptPath);

	const Sys::C_XmlDomElement* pConfigRoot = (Sys::C_XmlDomElement*)Sys::C_XmlDomUtility::FindXmlNodeByName(
												dom.GetRootNodePtr(), "shim_ab_config");

	if (!pConfigRoot)
	{
		LOGERROR(m_logger, "Failed to find 'shim_ab_config' tag in alienbrain configuration file");
		return false;
	}

	const Sys::C_XmlDomBaseNode::Array& domChildren = pConfigRoot->GetChildren();
	for (S32 i=0; i<domChildren.Size(); ++i)
	{
		Sys::C_XmlDomElement *pChild = (Sys::C_XmlDomElement*) domChildren[i];
		if (pChild->GetName() == "shim_ab_mapping")
		{
			Btl::String root = pChild->GetAttributes().GetValueByName("local");
			root = Misc::FormatFileString(Misc::RemoveTrailingSlash(root)).Lower();
			root.Replace("\\","/");

			C_Config::FolderMapping mapping;
			mapping.first = root;
			mapping.second = pChild->GetAttributes().GetValueByName("server");
			m_config.mappings.PushBack(mapping);
		}
	}

	//get alienbrain credentials
	const Sys::C_XmlDomElement* pUserElement = (Sys::C_XmlDomElement*)Sys::C_XmlDomUtility::FindXmlNodeByName(
			dom.GetRootNodePtr(), "shim_ab_user");
	if (pUserElement)
		m_config.user = pUserElement->GetAttributes().GetValueByName("name");

	const Sys::C_XmlDomElement* pPasswordElement = (Sys::C_XmlDomElement*)Sys::C_XmlDomUtility::FindXmlNodeByName(
			dom.GetRootNodePtr(), "shim_ab_pass");
	if (pUserElement)
		m_config.password = pPasswordElement->GetAttributes().GetValueByName("name");

	return true;
}

bool C_Ab::Update(const Btl::Vector<Btl::String>& filePaths) const
{
	return PerformAction(filePaths, false);
}

bool C_Ab::Sync(const Btl::Vector<Btl::String>& filePaths) const
{
	return PerformAction(filePaths, true);
}

bool C_Ab::PerformAction(const Btl::Vector<Btl::String>& filePaths, bool sync) const
{
	bool status = true;
	for (S32 i=0; i < m_config.mappings.Size(); ++i)
	{
		Btl::Vector<Btl::String> filePathsToGrab;
		for (S32 j=0; j < filePaths.Size(); ++j)
		{
			Btl::String formattedPath = filePaths[j].Lower();
			formattedPath = formattedPath.Replace("\\","/");

			if (formattedPath.Search(m_config.mappings[i].first) == 0)
				filePathsToGrab.PushBack(formattedPath);
		}
		if (filePathsToGrab.Size())
			if (!LaunchAlienbrainScript(m_config.mappings[i], filePathsToGrab, sync))
				status = false;
	}

	return status;
}

bool C_Ab::LaunchAlienbrainScript(
	const C_Config::FolderMapping& mapping,
	const Btl::Vector<Btl::String>& foldersToGrab, 
	bool sync) const
{
	bool status = true;

	if (foldersToGrab.Size())
	{
		//alienbrain project is not guaranteed to be the same as the project in CoreConfig.
		Btl::String project;
		{
			S32 start = mapping.first.Search('/', 0);
			S32 end = -1;
			if (start != -1)
			{
				end = mapping.first.Search('/', start+1);
				if (end != -1)
					project = mapping.first(start+1, end);
				else
					project = mapping.first(start+1, mapping.first.Length());
			}
		}

		Sys::C_FilePath tmpFile = "%TEMP%/alienbraingetlatest.xml";
		Tools::C_MacroExpander::Instance().Expand(tmpFile);

		Sys::C_File *pFile = (Sys::C_File*) Sys::C_FileSystem::Instance().Open(tmpFile, Sys::E_FileAccess_Write);
		if (!pFile)
		{
			LOGERROR(m_logger, "Failed to create alienbrain get latest file");
			return false;
		}

		Sys::TextSerialiser serialiser(*pFile);
		serialiser << "<folders>";
		for (S32 j=0; j<foldersToGrab.Size(); ++j)
			serialiser << "<folder>" << foldersToGrab[j].Chars() << "</folder>";

		serialiser << "</folders>";			
		serialiser.m_stream.Overflow();
		pFile->Flush();
		SAFE_DELETE_PTR(pFile);

		{
			Btl::String tmp;
			tmp.Printf("\"%s\" \"%s\" \"%s\" \"%s\" \"%s\" -m \"%s\" %s", 
				m_config.scriptPath.Chars(),
				m_config.user.Chars(),
				m_config.password.Chars(),
				project.Chars(), 
				mapping.second.Chars(), 
				tmpFile.Chars(),
				sync ? "--sync" : "--getlatest");

			if (!m_pAbScriptExecCallback(
				"cscript.exe", 
				tmp.Chars(),
				Sys::C_FileSystem::Instance().GetWorkingDirectory().Chars()))
			{
				status = false;
				LOGERROR(m_logger, "Failed to get latest references");
			}
		}
	}

	return status;
}

}
