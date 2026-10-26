#include "stdafx.h"
#include <tools_shared/shim/ShimErrorHelpers.h>

Util::C_String GetLeft(const Util::C_String& string, int s)
{
	return string(0, s);
}

Util::C_String GetRight(const Util::C_String& string, int s)
{
	S32 lo = string.Length() - s;
	S32 hi = lo + s;
	return string(lo, hi);
}

Util::C_String ExpandToolPath(const C_Options& options, const Util::C_String& toolPath)
{
	const char* launchPlatformMacro = "%TOOL_PLATFORM%";

	Util::C_String commandFilePath = toolPath;

	//Attempt to figure our our launch platform.
	//This is a hack really, we should have platform enums and a mapping table.
	//Default to win32.
	Util::C_String launchPlatform = "win32";
	if (options.GetPlatformName() != "win32")
	{
		Util::C_String candidateFilePath = 
			commandFilePath.Replace(launchPlatformMacro, options.GetPlatformName());

		if (Sys::C_FileSystem::Instance().FileExists(candidateFilePath))
			//Use whatever our preferred platform is.
			launchPlatform = options.GetPlatformName();
	}

	LOGDEBUG(Tools::g_pMyApplication->GetLogger(), "GetFullToolPath using platform - '%s'.\n", launchPlatform.Chars());

	commandFilePath = commandFilePath.IReplace(launchPlatformMacro, launchPlatform);

	Tools::C_MacroExpander::Instance().Expand(commandFilePath);

	//Set launch command and working folders.
	return Util::FormatWindowsFileString(commandFilePath);
}

bool RemoveReadOnlyFlag(const Util::C_String& filePath)
{
	Sys::C_FileInfo info;
	if (Sys::C_FileSystem::Instance().FileInfo(filePath, &info))
	{
		//If the file exists and is not writable.
		if (info.m_FileType == Sys::E_FileType_File && !(info.m_FileAccess & Sys::E_FileAccess_Write))
		{
			BEGIN_CHECK_CUSTOM(!Sys::C_FileSystem::Instance().FileAccess(filePath, info.m_FileAccess | Sys::E_FileAccess_Write))
				ERROR_FATAL("ERROR: Could not set file permissions for %s.\n", filePath.Chars());
			END_RETURN_FALSE;
		}
	}

	return true;
}

Util::C_String GetAssetFullPath(
	const Util::C_String& assetPath,
	U32 exportInfo,
	const Util::C_String& launchComputerName)
{
	if (exportInfo & Tools::Shim::E_ExportInfo_DataSourceLocal)
	{
		//Convert from network path to local if required, otherwise returns local path.
		return Util::ConvertABNetworkPathToLocalPath(assetPath);
	}

	//We need to convert a network path to point to our alienbrain drive.
	bool isNetworkExport = (exportInfo & Tools::Shim::E_ExportInfoMask_Location) == 
		Tools::Shim::E_ExportInfo_LocationNetwork;
	bool isSCMData = (exportInfo & Tools::Shim::E_ExportInfoMask_DataSource) == Tools::Shim::E_ExportInfo_DataSourceSCM;
	bool isNetworkData = (exportInfo & Tools::Shim::E_ExportInfoMask_DataSource) == Tools::Shim::E_ExportInfo_DataSourceNetwork;

	if (isNetworkExport && isSCMData)
	{
		int pos = assetPath.ISearch(ALIEN_BRAIN_FOLDER);
		if (pos == -1)
		{
			ERROR_WARN("Could not find alienbrain path.\n");
			return assetPath;
		}
		return Util::ConvertABNetworkPathToLocalPath(assetPath);
	}
	
	if (isNetworkExport && isNetworkData)
	{
		return Util::ConvertABNetworkPathToLocalPath(assetPath);
	}

	ERROR_ERROR("Unknown export type in GetAssetFullPath");
	return assetPath;
}