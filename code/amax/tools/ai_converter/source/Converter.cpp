#include "stdafx.h"
#include "Converter.h"

#include <tools_shared/utility/misc.h>
#include <tools_shared/shim/ShimErrorHelpers.h>

Sys::C_FilePath ExpandToolPath(const C_Options& options, const Sys::C_FilePath& toolPath)
{
	const char* launchPlatformMacro = "%TOOL_PLATFORM%";

	Sys::C_FilePath commandFilePath = toolPath;

	//Attempt to figure our our launch platform.
	//This is a hack really, we should have platform enums and a mapping table.
	//Default to win32.
	Btl::String launchPlatform = "win32";
	if (Tools::GetPlatformString() != "win32")
	{
		Sys::C_FilePath candidateFilePath = 
			commandFilePath.Replace(launchPlatformMacro, Tools::GetPlatformString().Chars());

		if (Sys::GetDefaultFileSystem()->FileExists(candidateFilePath))
			//Use whatever our preferred platform is.
			launchPlatform = Tools::GetPlatformString().Chars();
	}

	LOGDEBUG(Tools::g_pMyApplication->GetLogger(), "GetFullToolPath using platform - '%s'.\n", launchPlatform.Chars());

	commandFilePath = commandFilePath.Replace(launchPlatformMacro, launchPlatform);

	Tools::C_MacroExpander::Instance().Expand(commandFilePath);

	//Set launch command and working folders.
	return commandFilePath;
}

bool RemoveReadOnlyFlag(const Btl::String& filePath)
{
	Sys::C_FileInfo info;
	if (Sys::GetDefaultFileSystem()->FileInfo(filePath, &info))
	{
		//If the file exists and is not writable.
		if (info.m_FileType == Sys::E_FileType_File && !(info.m_FileAccess & Sys::E_FileAccess_Write))
		{
			if (!Sys::GetDefaultFileSystem()->FileAccess(filePath, info.m_FileAccess | Sys::E_FileAccess_Write))
			{
				ERROR_FATAL("ERROR: Could not set file permissions for %s.\n", filePath.Chars());
				return false;
			}
		}
	}

	return true;
}

bool C_AIConverter::Process(Tools::FileCopy::C_FileCopyList& uberCopyList,
							const C_Options& options,
							const Sys::C_XmlDom& dom)
{
	//Ordinarily our input file would be coming from our 'exportpath' and we'd use the string mangling below
	//Btl::String relativeOutputFile = options.inputFilePath.IReplace(options.exportPath, "");

	bool success = true;

	if (options.ShouldExport())
	{
		ERROR_INFO("Generating MEL file for export");

		BEGIN_CHECK_CUSTOM (!Sys::GetDefaultFileSystem()->FileExists(options.GetMayaSceneValue()))
			ERROR_ERROR("Input file: (%s) doesn't exist!", options.GetMayaSceneValue().Chars())
		END_RETURN_FALSE

		Sys::C_FilePath tempOutputDirectory = options.GetRootTempPath() + "/" + options.GetOutputPath() + "/" + options.GetTrackName() + "/";
		Sys::C_FilePath melFilename = tempOutputDirectory + "AIExport.mel";
		Sys::C_FilePath tempOutputLocation = tempOutputDirectory + options.GetOutputFilename();
		Btl::String tempOutputLocationUnix = tempOutputLocation.Chars();
		tempOutputLocationUnix = tempOutputLocationUnix.Replace("\\", "/");
		Btl::String mayaSceneUnix = options.GetMayaSceneValue();
		mayaSceneUnix = mayaSceneUnix.Replace("\\", "/");

		Sys::GetDefaultFileSystem()->CreateDirectory(tempOutputDirectory);

		Btl::String convertScript;

		convertScript += "global int $biz_TM_skipGetLatest = 1;\n";
		convertScript += "source biz_batchModeSetup.mel;\n";
		convertScript += "if (!`pluginInfo -q -l racingAITools`) loadPlugin racingAITools.mll;\n";
		convertScript += "biz_mb_loadSceneAndReloadRefs(\"";
		convertScript = convertScript + mayaSceneUnix + " \", \"\");\n";
		convertScript += "source biz_dw_AITools.mel;biz_dw_exportAIDataAuto(\"";
		convertScript = convertScript + tempOutputLocationUnix;
		convertScript += "\");\n";

		FILE* fh = 0;
		fopen_s(&fh, melFilename.Chars(), "w");
		if (fh)
		{
			fwrite(convertScript.Chars(), convertScript.Length(), 1, fh);
			fclose(fh);
		}

		ERROR_INFO("Executing mayabatch");

		Btl::String convertCommandLine = Btl::String("-script \"") + melFilename.String() + Btl::String("\"");

		BEGIN_CHECK_CUSTOM(Sys::ShellExec::ExecuteCommandAndWait(
			"mayabatch", 
			convertCommandLine.Chars(),
			tempOutputDirectory.Chars(),
			Sys::ShellExec::E_LaunchState_Minimise,
			&Tools::g_pMyApplication->GetLogger(),
			0, 
			"",
			0,
			true, 
			true) != 0)
			ERROR_ERROR("Nexus conversion failed");
		END_RETURN_FALSE

		//Bail if failed to create output.
		BEGIN_CHECK_CUSTOM (!Sys::GetDefaultFileSystem()->FileExists(tempOutputLocation))
			ERROR_ERROR("Output file: (%s) doesn't exist!", tempOutputLocation.Chars())
		END_RETURN_FALSE

		ERROR_INFO("Copying output to working");

		//Write out data for our different skus.
		for(S32 platformNo=0; platformNo < Util::E_Platform_Count; platformNo++)
		{
			//Create output folder.
			Btl::String relativeOutputFile = options.GetOutputPath() + "/" + options.GetTrackName() + "/" + options.GetOutputFilename();

			Btl::String inputFilePath = options.GetRootTempPath().Chars();
			inputFilePath += "/";
			inputFilePath += relativeOutputFile;
			Btl::String outputFilePath = options.GetCopyLocations()[platformNo].workingPath.Chars();
			outputFilePath += "/";
			outputFilePath += options.GetLevelPath() + "/";
			outputFilePath += relativeOutputFile;

			//Create output folder.
			success = Sys::GetDefaultFileSystem()->CreateDirectory(Sys::ExtractDirectory(outputFilePath));
			if (!success) goto exit;

			ERROR_INFO("Copying [%s] to [%s]\n", inputFilePath.Chars(), outputFilePath.Chars());

			success = RemoveReadOnlyFlag(outputFilePath);
			if (!success) goto exit;

			//Perform copy.
			if (!CopyFile(
				inputFilePath.Chars(),
				outputFilePath.Chars(),
				FALSE))
			{
				success = false;
				goto exit;
			}

			uberCopyList.AddFileCopyItem(
				(Util::E_Platform)platformNo,
				Tools::FileCopy::C_FileCopyItem((const char*)outputFilePath.Chars(), ""));
		}
	} else {
		ERROR_INFO("Job marked as do not export");
	}
exit:
	return success;
}


