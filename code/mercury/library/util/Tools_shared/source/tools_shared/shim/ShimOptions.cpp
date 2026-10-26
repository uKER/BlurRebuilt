#include "stdafx.h"

#include <core/utility/Singleton.h>
#include <core/utility/HashOps.h>

#include <tools_shared/PlatformHelpers.h>
#include <tools_shared/shim/ShimDefinitions.h>
#include <tools_shared/file_copy/FileCopy.h>
#include <tools_shared/shim/Shim.h>
#include <tools_shared/shim/ShimSharedXmlTags.h>
#include <tools_shared/utility/MacroExpander.h>

#include <system/misc/StringUtil.h>
#include <system/xml/XmlDom.h>

//Shim configuration
#define SHIMXML_CONFIG											"shim_config"
#define SHIMXML_CONFIG_MACROS									"shim_macros"
#define SHIMXML_CONFIG_EXECUTER_MACROS							"vars"
#define SHIMXML_CONFIG_ROOT_PROJECT_PATH						"shim_root_project_path"
#define SHIMXML_CONFIG_ROOT_TOOLS_PATH							"shim_root_tools_path"
#define SHIMXML_CONFIG_SERVER_ROOT_TOOLS_PATH					"shim_server_root_tools_path"
#define SHIMXML_CONFIG_ROOT_ASSET_PATH							"shim_root_asset_path"
#define SHIMXML_CONFIG_ROOT_TEMP_PATH							"shim_root_temp_path"
#define	SHIMXML_CONFIG_ROOT_WORKING_PATH						"shim_root_working_path"
#define SHIMXML_CONFIG_ROOT_GAME_PATHS							"shim_root_game_paths"
#define SHIMXML_CONFIG_ROOT_GAME_PATH							"shim_root_game_path"

#define SHIMXML_CONFIG_SERVER_ROOT_CONTENT_PATH					"shim_server_root_tools_path"
#define SHIMXML_CONFIG_BOOTSTRAP_FILEPATH						"shim_bootstrap_filepath"
#define SHIMXML_CONFIG_MAYA_PLUG_IN_PATH						"shim_maya_plug_in_path"
#define SHIMXML_CONFIG_UBERCOPY_FILEPATH						"shim_ubercopy_filepath"
#define SHIMXML_CONFIG_ROBOCOPY_FILEPATH						"shim_robocopy_filepath"

//Devkit info
#define	SHIMXML_DEVKIT_360										"shim_devkit_360"

namespace Tools
{
namespace Shim
{

bool C_Options::ParseXml(const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom)
{
	m_shimXmlFilePath = xmlFilePath;

	//Aquire macros
	if (!Tools::C_MacroExpander::Instance().ParseMacroGroupsXML(dom.GetRootNodePtr(), 
		SHIMXML_CONFIG_EXECUTER_MACROS))
		return false;

	if (!Tools::C_MacroExpander::Instance().ParseMacroGroupsXML(dom.GetRootNodePtr(), 
		SHIMXML_CONFIG_MACROS))
		return false;

	const Sys::C_XmlDomBaseNode* pExportTypeNode = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "export_type");
	if (!pExportTypeNode)
	{
		LOGFATAL( Tools::g_log, "Failed to find export type in XML.\n");
        return false;
	}

	const Sys::C_XmlDomBaseNode* pTypeEntry = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "type_entry");
	if (!pTypeEntry)
	{
		LOGFATAL( Tools::g_log, "Failed to find type entry type in XML.\n");
        return false;
	}

	m_shimExportName = m_shimExportName.Format("%s_%x", ((Sys::C_XmlDomElement*)(pTypeEntry))->GetAttributes().GetAttributeByName("name")->value.Chars(),
					Ut::HashKeyOpString::GetHash(((Sys::C_XmlDomElement*)(pTypeEntry))->GetAttributes().GetAttributeByName("type_name")->value.Chars()));

	Util::C_String exportType = ((Sys::C_XmlDomElement*)(pExportTypeNode))->GetAttributes().GetAttributeByName("type")->value;
	exportType.ConvertToLower();
	if (exportType == "local")
		m_shimExportInfo = Shim::E_ExportInfo_LocationLocal;
	else if (exportType == "network")
		m_shimExportInfo = Shim::E_ExportInfo_LocationNetwork;
	else
	{
		m_shimExportInfo = Shim::E_ExportInfo_LocationUnknown;
		LOGFATAL( Tools::g_log, "Failed to find valid export type in XML.\n");
		return false;
	}

	Util::C_String dataType = ((Sys::C_XmlDomElement*)(pExportTypeNode))->GetAttributes().GetAttributeByName("data")->value;
	dataType.ConvertToLower();
	if (dataType == "local")
		m_shimExportInfo |= Shim::E_ExportInfo_DataSourceLocal;
	else if (dataType == "network")
		m_shimExportInfo |= Shim::E_ExportInfo_DataSourceNetwork;
	else if (dataType == "scm")
		m_shimExportInfo |= Shim::E_ExportInfo_DataSourceSCM;
	else
	{
		m_shimExportInfo |= Shim::E_ExportInfo_DataSourceUnknown;
		LOGFATAL( Tools::g_log, "Failed to find valid data source in XML.\n");
		return false;
	}
	
	LOGDEBUG( Tools::g_log, "Performing %s export.\n", exportType.Chars());

	// Scan through the node hierachy looking for export info

	const Sys::C_XmlDomBaseNode* pRootAssetPathXml = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), SHIMXML_CONFIG_ROOT_ASSET_PATH);
	if (!pRootAssetPathXml) return false;

	const Sys::C_XmlDomBaseNode* pRootTempPathXml = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), SHIMXML_CONFIG_ROOT_TEMP_PATH);
	if (!pRootTempPathXml) return false;

	const Sys::C_XmlDomBaseNode* pRootWorkingPathXml = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), SHIMXML_CONFIG_ROOT_WORKING_PATH);
	if (!pRootWorkingPathXml) return false;

	const Sys::C_XmlDomBaseNode* pRootGamePathsXml = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), SHIMXML_CONFIG_ROOT_GAME_PATHS);
	if (!pRootGamePathsXml) return false;

	const Sys::C_XmlDomBaseNode* p360Name = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(dom.GetRootNodePtr(), SHIMXML_DEVKIT_360);
	if (!p360Name) return false;

	m_shimRootAssetPath = 
		Util::FormatWindowsFileString(((Sys::C_XmlDomElement*)(pRootAssetPathXml))->GetAttributes().GetAttributeByName("name")->value);
	m_shimRootTempPath = 
		Util::FormatWindowsFileString(((Sys::C_XmlDomElement*)(pRootTempPathXml))->GetAttributes().GetAttributeByName("name")->value);
	m_shimRootWorkingPath = 
		Util::FormatWindowsFileString(((Sys::C_XmlDomElement*)(pRootWorkingPathXml))->GetAttributes().GetAttributeByName("name")->value);
	Util::C_String x360DevKitName = 
		((Sys::C_XmlDomElement*)(p360Name))->GetAttributes().GetAttributeByName("value")->value;

	//Use the same working path for all platforms
	for (S32 i = 0; i < m_shimCopyLocations.Size(); i++)
	{
		m_shimCopyLocations[i].workingPath = m_shimRootWorkingPath;
		const char* platformName = Util::GetPlatformString((Util::E_Platform)i);
		if (!platformName)
			return false;

		m_shimCopyLocations[i].workingPath = m_shimCopyLocations[i].workingPath + "\\" + platformName;
	}

	//Set devkit paths
	m_shimCopyLocations[Util::E_Platform_360].devKit = x360DevKitName;

	//Set game paths.
	for (S32 i = 0; i < pRootGamePathsXml->GetChildren().Size(); i++)
	{
		const Sys::C_XmlDomElement* pNode = (Sys::C_XmlDomElement*)pRootGamePathsXml->GetChildren()[i];
		if (pNode->GetName().Lower() != SHIMXML_CONFIG_ROOT_GAME_PATH)
			//Ignore.
			continue;

		Util::C_String platformName = pNode->GetAttributes().GetAttributeByName("platform")->value;
		Util::C_String platformPath = Util::FormatWindowsFileString(
			pNode->GetAttributes().GetAttributeByName("name")->value);

		Util::E_Platform platform = Util::GetPlatform(platformName);
		if (platform == Util::E_Platform_Unknown)
			//User specified unknown export platform.
			return false;

		m_shimCopyLocations[platform].copyPath = platformPath;
	}

	//We need to grab the name of the computer that called us.
	if (!SetLaunchComputerName())
		return false;

	// Scan through the node hierachy looking for export info
	const Sys::C_XmlDomBaseNode* pShimConfigNode = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), SHIMXML_CONFIG);
	if (!pShimConfigNode) return false;
	const Sys::C_XmlDomBaseNode* pLocalRootToolsPath = Sys::C_XmlDomUtility::FindXmlNodeByName(pShimConfigNode, SHIMXML_CONFIG_ROOT_TOOLS_PATH);
	if (!pLocalRootToolsPath) return false;
	const Sys::C_XmlDomBaseNode* pLocalRootProjectPath = Sys::C_XmlDomUtility::FindXmlNodeByName(pShimConfigNode, SHIMXML_CONFIG_ROOT_PROJECT_PATH);
	if (!pLocalRootProjectPath) return false;
	const Sys::C_XmlDomBaseNode* pServerRootToolsPath = Sys::C_XmlDomUtility::FindXmlNodeByName(pShimConfigNode, SHIMXML_CONFIG_SERVER_ROOT_TOOLS_PATH);
	if (!pServerRootToolsPath) return false;

	m_shimRootToolsPath = 
		Util::FormatWindowsFileString(((Sys::C_XmlDomElement*)(pLocalRootToolsPath))->GetAttributes().GetAttributeByName("name")->value);
	m_shimRootProjectPath = 
		Util::FormatWindowsFileString(((Sys::C_XmlDomElement*)(pLocalRootProjectPath))->GetAttributes().GetAttributeByName("name")->value);
	m_shimServerRootToolsPath = 
		Util::FormatWindowsFileString(((Sys::C_XmlDomElement*)(pServerRootToolsPath))->GetAttributes().GetAttributeByName("name")->value);

	const Sys::C_XmlDomBaseNode* pBootstrapFilePath = Sys::C_XmlDomUtility::FindXmlNodeByName(pShimConfigNode, SHIMXML_CONFIG_BOOTSTRAP_FILEPATH);
	if (pBootstrapFilePath)
		m_shimBootstrapFilePath = 
			Util::FormatWindowsFileString(((Sys::C_XmlDomElement*)(pBootstrapFilePath))->GetAttributes().GetAttributeByName("name")->value);
	const Sys::C_XmlDomBaseNode* pAftkConv = Sys::C_XmlDomUtility::FindXmlNodeByName(pShimConfigNode, "shim_aftkconv_filepath");
	if (pAftkConv)
		m_shimAftkConvFilePath = 
			Util::FormatWindowsFileString(((Sys::C_XmlDomElement*)(pAftkConv))->GetAttributes().GetAttributeByName("name")->value);
	const Sys::C_XmlDomBaseNode* pSvn = Sys::C_XmlDomUtility::FindXmlNodeByName(pShimConfigNode, "svn");
	if (pSvn)
	{
		const Sys::C_XmlDomBaseNode* pSvnCmd = Sys::C_XmlDomUtility::FindXmlNodeByName(pSvn, "get_latest_cmd");
		if (pSvnCmd)
		{
			m_svnOptions.svnGetLatestCmd = ((Sys::C_XmlDomElement*)(pSvnCmd))->GetAttributes().GetAttributeByName("path")->value; 
			Tools::C_MacroExpander::Instance().Expand(m_svnOptions.svnGetLatestCmd);
		}

		const Sys::C_XmlDomBaseNode* pCreds = Sys::C_XmlDomUtility::FindXmlNodeByName(pSvn, "credentials");
		if (pCreds) 
		{
			m_svnOptions.user = ((Sys::C_XmlDomElement*)(pCreds))->GetAttributes().GetAttributeByName("user")->value.Chars();
			m_svnOptions.pass = ((Sys::C_XmlDomElement*)(pCreds))->GetAttributes().GetAttributeByName("pass")->value.Chars();
		}

		const Sys::C_XmlDomBaseNode* pSvnArtTools = Sys::C_XmlDomUtility::FindXmlNodeByName(pShimConfigNode, "art_tools_svn");
		if (pSvnArtTools) 
		{
			m_shimSvnArtToolsSvn.remotePath = ((Sys::C_XmlDomElement*)(pSvnArtTools))->GetAttributes().GetAttributeByName("remotePath")->value.Chars();
			m_shimSvnArtToolsSvn.localPath = ((Sys::C_XmlDomElement*)(pSvnArtTools))->GetAttributes().GetAttributeByName("localPath")->value.Chars();
			Tools::C_MacroExpander::Instance().Expand(m_shimSvnArtToolsSvn.localPath);
		}
	}
	const Sys::C_XmlDomBaseNode* pUberCopy = Sys::C_XmlDomUtility::FindXmlNodeByName(pShimConfigNode, SHIMXML_CONFIG_UBERCOPY_FILEPATH);
	if (pUberCopy)
		m_shimUberCopyFilePath = 
			Util::FormatWindowsFileString(((Sys::C_XmlDomElement*)(pUberCopy))->GetAttributes().GetAttributeByName("name")->value);
	const Sys::C_XmlDomBaseNode* pRoboCopyPath = Sys::C_XmlDomUtility::FindXmlNodeByName(pShimConfigNode, SHIMXML_CONFIG_ROBOCOPY_FILEPATH);
	if (pRoboCopyPath)
		m_shimRobocopyFilePath = 
			Util::FormatWindowsFileString(((Sys::C_XmlDomElement*)(pRoboCopyPath))->GetAttributes().GetAttributeByName("name")->value);
	
	//Perform macro expansion.
	for (S32 i = 0; i < m_shimCopyLocations.Size(); i++)
	{
		C_MacroExpander::Instance().Expand(m_shimCopyLocations[i].copyPath);
		C_MacroExpander::Instance().Expand(m_shimCopyLocations[i].devKit);
		C_MacroExpander::Instance().Expand(m_shimCopyLocations[i].workingPath);
	}

	Tools::C_MacroExpander::Instance().Expand(m_shimRootToolsPath);
	Tools::C_MacroExpander::Instance().Expand(m_shimServerRootToolsPath);

	Tools::C_MacroExpander::Instance().Expand(m_shimRootProjectPath);

	Tools::C_MacroExpander::Instance().Expand(m_shimRootAssetPath);
	Tools::C_MacroExpander::Instance().Expand(m_shimRootTempPath);
	Tools::C_MacroExpander::Instance().Expand(m_shimRootWorkingPath);

	Tools::C_MacroExpander::Instance().Expand(m_shimLaunchComputerName);
	
	Tools::C_MacroExpander::Instance().Expand(m_shimRobocopyFilePath);
	Tools::C_MacroExpander::Instance().Expand(m_shimUberCopyFilePath);
	Tools::C_MacroExpander::Instance().Expand(m_shimAftkConvFilePath);
	Tools::C_MacroExpander::Instance().Expand(m_shimBootstrapFilePath);
	return true;
}

bool C_Options::SetLaunchComputerName()
{
	if (m_shimExportInfo & E_ExportInfo_LocationNetwork)
	{
		int pos = m_shimXmlFilePath.ISearch(ALIEN_BRAIN_FOLDER);

		if (pos == -1)
			return false;

		// Is this a network path... just to make sure... :)
		if (m_shimXmlFilePath(0, 2) == "\\\\")
		{
			int pos = m_shimXmlFilePath.ISearch(ALIEN_BRAIN_FOLDER);
			m_shimLaunchComputerName = m_shimXmlFilePath(2, pos-1);
			return true;
		}
	}
	else
	{
		//Get local machine name.
		char tmp[128];
		DWORD sizeComputerName = sizeof(tmp);
		GetComputerName(tmp, &sizeComputerName);
		m_shimLaunchComputerName = tmp;
		return true;
	}

	return false;
}


const Util::C_String& C_Options::GetExportName() const
{
	return m_shimExportName;
}

const Util::C_String& C_Options::GetXmlFilePath() const
{
	return m_shimXmlFilePath;
}

const Util::C_String& C_Options::GetRootToolsPath() const
{
	return m_shimRootToolsPath;
}

const Util::C_String& C_Options::GetServerRootToolsPath() const
{
	return m_shimServerRootToolsPath;
}

const Util::C_String& C_Options::GetRootProjectPath() const
{
	return m_shimRootProjectPath;
}

const Util::C_String& C_Options::GetRootAssetPath() const
{
	return m_shimRootAssetPath;
}

const Util::C_String& C_Options::GetRootTempPath() const
{
	return m_shimRootTempPath;
}

const Util::C_String& C_Options::GetRootWorkingPath() const
{
	return m_shimRootWorkingPath;
}

const U32 C_Options::GetExportInfo() const
{
	return m_shimExportInfo;
}

const Util::C_String& C_Options::GetLaunchComputerName() const
{
	return m_shimLaunchComputerName;
}

const ShimCopyLocations& C_Options::GetCopyLocations() const
{
	return m_shimCopyLocations;
}

const Util::C_String& C_Options::GetBootstrapFilePath() const
{
	return m_shimBootstrapFilePath;
}

const Util::C_String& C_Options::GetRobocopyFilePath() const
{
	return m_shimRobocopyFilePath;
}

const Util::C_String& C_Options::GetUberCopyFilePath() const
{
	return m_shimUberCopyFilePath;
}

const Util::C_String& C_Options::GetAftkConvFilePath() const
{
	return m_shimAftkConvFilePath;
}

const C_SvnFolder& C_Options::GetArtToolsSvn() const
{
	return m_shimSvnArtToolsSvn;
}

const C_SvnOtions& C_Options::GetSvnOptions() const
{
	return m_svnOptions;
}

}
}