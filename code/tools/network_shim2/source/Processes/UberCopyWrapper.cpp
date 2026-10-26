#include "stdafx.h"
#include "Utility.h"
#include <tools_shared/shim/ShimErrorHelpers.h>

#include <Helpers/XmlHelper.h>
#include <tools_shared/PlatformHelpers.h>
#include <tools_shared/file_copy/FileCopyUtils.h>
#include <processes/UberCopyWrapper.h>

Ut::OpenHashSet<Util::C_String, Ut::HashKeyOpString> UberCopyWrapper::m_DestinationSet;
Ut::OpenHashSet<Util::C_String, Ut::HashKeyOpString> UberCopyWrapper::m_TaskSet;

bool UberCopyWrapper::PostExportCopy(const C_Options& options, const Util::C_String& nodeName)
{
	ERROR_INFO("Performing post-export copy.\n");
	//Okay, so using the output name of the asset we've processed we need to determine what
	//folder contains the XML files describing our files to copy.

	Util::C_String taskDescriptionXmlFilePath = 
		GetTaskDescriptionXmlFilePath(options, nodeName);

	Util::C_String working;
	BEGIN_CHECK_CUSTOM (!GetCopyToWorkingXmlFilePath(working, taskDescriptionXmlFilePath))
		ERROR_ERROR("Failed to get 'working' file path");
	END_RETURN_FALSE;

	bool isNetworkExport = (options.GetExportInfo() & Tools::Shim::E_ExportInfoMask_Location) == Tools::Shim::E_ExportInfo_LocationNetwork;
	bool isLocalData = (options.GetExportInfo() & Tools::Shim::E_ExportInfoMask_DataSource) == Tools::Shim::E_ExportInfo_DataSourceNetwork;
	//Copy working folder for network exports only.
	if (isNetworkExport && !isLocalData)
	{
		Util::C_String game;
		BEGIN_CHECK_CUSTOM(!GetCopyToGameXmlFilePath(game, taskDescriptionXmlFilePath))
			ERROR_ERROR("Failed to get game path")
		END_RETURN_FALSE;

		if (!CopySingle(options, taskDescriptionXmlFilePath))
			return false;

		//Copy our xml config files
		{
			if (!CopySingle(options, working))
				return false;

			if (!CopySingle(options, game))
				return false;
		}

		//Copy to working folder.
		if (!Copy(options, working))
			return false;
	}

	Util::C_String platform = "";
	CopyToGame(options, platform, taskDescriptionXmlFilePath);

	return true;
}

bool UberCopyWrapper::CopyToGame(const C_Options& options, const Util::C_String& platform, const Util::C_String& taskDescriptionXmlFilePath)
{
	if (platform.Length() == 0)
		ERROR_INFO("Performing copy to all platforms.\n")
	else
		ERROR_INFO("Performing copy to %s.\n", platform.Chars());

	const Util::C_String* pPlatform;
	if (platform.Length() == 0)
		pPlatform = 0;
	else
		pPlatform = &platform;

	Util::C_String xmlFilePath;
	if (!GetCopyToGameXmlFilePath(
		xmlFilePath,
		taskDescriptionXmlFilePath))
		return false;

	bool result = Copy(
		options,
		xmlFilePath,
		pPlatform);
	if (!result)
	{
		BEGIN_ALWAYS
		ERROR_ERROR("*** Problems Copying! ***\n\
One or more files have failed to copy. If this is just because your devkit is switched off you can attempt the copy again at a later time.\n\
To do this, select the relevant copy option by right-clicking on the Executer node you processed.");
		END_RETURN_FALSE;
	}

	Util::C_String text = "All files have been successfully copied for ";
	if (!pPlatform)
		text += "all platforms";
	else
		text += *pPlatform;
	text += ".";
	ERROR_INFO("*** Copy Successful! ***\n%s", text.Chars());

	return true;
}

bool UberCopyWrapper::CopySingle(const C_Options& options, const Util::C_String& fileToCopy)
{
	if (!Sys::C_FileSystem::Instance().FileExists(fileToCopy))
	{
		LOGINFO(Tools::g_pMyApplication->GetLogger(), "Skipping copy working as %s does not exist", fileToCopy.Chars());
		return false;
	}

	Util::C_String src = fileToCopy;
	Util::C_String dst = Util::ConvertLocalPathToABNetworkPath(fileToCopy, options.GetLaunchComputerName());

	if (src.Lower() != dst.Lower())
		return Tools::FileCopy::FileCopyUtils::CopyFile(src, dst);
	else
		return true;
}

bool UberCopyWrapper::Copy(const C_Options& options, const Util::C_String& xmlFilePath, const Util::C_String* pPlatform)
{
	BEGIN_CHECK_CUSTOM(!Sys::C_FileSystem::Instance().FileExists(xmlFilePath))
		ERROR_INFO("Skipping copy working as %s does not exist", xmlFilePath.Chars())
	END_RETURN_FALSE

	BEGIN_CHECK_CUSTOM (!ValidityCheck(options, xmlFilePath)) ERROR_ERROR("Export contains duplicate filename!") END_RETURN_FALSE

	Util::C_String localUberCopyExe = ExpandToolPath(
		options,
		options.GetUberCopyFilePath());

	Util::C_String args = Util::C_String("\"") + xmlFilePath + "\"";
	if (pPlatform)
	{
		args += " --platform " + *pPlatform;
		if (*pPlatform == "xenon" && options.GetCopyLocations()[Util::E_Platform_360].devKit != "**UNINITIALISED**")
			args += " --consoles \"" + options.GetCopyLocations()[Util::E_Platform_360].devKit + "\"";
	}
	else
	{
		if (options.GetCopyLocations()[Util::E_Platform_360].devKit == "**UNINITIALISED**")
			args += " --platform pc,ps3";
		else
			args += " --platform pc,ps3,xenon --consoles \"" + options.GetCopyLocations()[Util::E_Platform_360].devKit + "\"";
	}

	BEGIN_CHECK_CUSTOM (Sys::ShellExec::ExecuteCommandAndWait(
		localUberCopyExe, 
		args,
		Util::ExtractDirectory(localUberCopyExe),
		options.GetShellExecLaunchState(),
		&Tools::g_pMyApplication->GetLogger()) != 0)
		ERROR_ERROR("UberCopy failed.")
	END_RETURN_FALSE

	return true;
}

bool UberCopyWrapper::ValidityCheck(const C_Options& options, const Util::C_String& xmlFilePath)
{
	Sys::C_XmlDom dom;
	dom.Read(xmlFilePath.Chars());

	Ut::Vector<const Sys::C_XmlDomBaseNode*> files;
	Sys::C_XmlDomUtility::XpathEvaluate(dom.GetRootNodePtr(), "**/file", files);

	for (int i=0; i<files.Size(); i++)
	{
		const char* dst = ((Sys::C_XmlDomElement*)files[i])->GetAttributes().GetValueByName("dst");

		if (dst)
		{
			BEGIN_CHECK_CUSTOM(m_DestinationSet.Find(dst) != m_DestinationSet.End()) ERROR_ERROR("Asset (%s) is copied to from multiple locations", dst) END_RETURN_FALSE
			m_DestinationSet.Insert(dst);
		}
	}

	// Loop through all platform restrictions

	//Ut::Vector<const Sys::C_XmlDomBaseNode*> platforms;
	//Sys::C_XmlDomUtility::XpathEvaluate(options.GetActionNode(), "**/shim_config/shim_path_match/platform", platforms);

	//for (int i=0; i<platforms.Size(); i++)
	//{
	//	if (!((Sys::C_XmlDomElement*)platforms[i])->GetAttributes().GetValueByName("name"))
	//		continue;

	//	Util::C_String platform = ((Sys::C_XmlDomElement*)platforms[i])->GetAttributes().GetValueByName("name");

	//	int fileLength = -1;
	//	int pathLength = -1;

	//	if (((Sys::C_XmlDomElement*)platforms[i])->GetAttributes().GetValueByName("file_length"))
	//		fileLength = atoi(((Sys::C_XmlDomElement*)platforms[i])->GetAttributes().GetValueByName("file_length"));

	//	if (((Sys::C_XmlDomElement*)platforms[i])->GetAttributes().GetValueByName("path_length"))
	//		pathLength = atoi(((Sys::C_XmlDomElement*)platforms[i])->GetAttributes().GetValueByName("path_length"));

	//	Sys::C_XmlDomUtility::XpathEvaluate(dom.GetRootNodePtr(), "**/platform[@type=\""+platform+"\"]/file", files);

	//	for (int j=0; j<files.Size(); j++)
	//	{
	//		const char* dst = ((Sys::C_XmlDomElement*)files[j])->GetAttributes().GetValueByName("dst");

	//		if (dst)
	//		{
	//			BEGIN_CHECK_CUSTOM(m_DestinationSet.Find(dst) != m_DestinationSet.End()) ERROR_ERROR("asset (%s) is copied to from multiple locations", dst) END_RETURN_FALSE
	//			m_DestinationSet.Insert(dst);
	//		}
	//	}
	//}

	return true;
}

bool UberCopyWrapper::AddTask(const Util::C_String& nodeName)
{
	if (m_TaskSet.Find(nodeName) != m_TaskSet.End())
		return false;

	m_TaskSet.Insert(nodeName);
	return true;
}

const Ut::OpenHashSet<Util::C_String, Ut::HashKeyOpString>& UberCopyWrapper::GetTasks()
{
	return m_TaskSet;
}

Util::C_String UberCopyWrapper::GetTaskDescriptionXmlFilePath(const C_Options& options, const Util::C_String& nodeName)
{
	Util::C_String filePath;
	filePath = options.GetRootTempPath() + "\\ubercopy\\";
	filePath = filePath + nodeName + Util::C_String(".ubercopy");
	return filePath;
}

bool UberCopyWrapper::GetCopyToGameXmlFilePath(Util::C_String& copyToGameFilePath, const Util::C_String& taskFilePath)
{
	Sys::C_XmlDom dom;
	BEGIN_CHECK_CUSTOM(!dom.Read(taskFilePath)) ERROR_ERROR("Couldn't read ubercopy path (%s)", taskFilePath.Chars()) END_RETURN_FALSE;

	const Sys::C_XmlDomBaseNode* pNode = 		
		Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "ubercopy");

	copyToGameFilePath = 
		((Sys::C_XmlDomElement*)(pNode))->GetAttributes().GetAttributeByName("copy_to_game")->value;

	return true;
}

bool UberCopyWrapper::GetCopyToWorkingXmlFilePath(Util::C_String& copyToWorkingFilePath, const Util::C_String& taskFilePath)
{
	Sys::C_XmlDom dom;
	BEGIN_CHECK_CUSTOM(!dom.Read(taskFilePath)) ERROR_ERROR("Couldn't read ubercopy path (%s)", taskFilePath.Chars()) END_RETURN_FALSE;

	const Sys::C_XmlDomBaseNode* pNode =
		Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "ubercopy");

	copyToWorkingFilePath =
		((Sys::C_XmlDomElement*)(pNode))->GetAttributes().GetAttributeByName("copy_to_working")->value;

	return true;
}