#include "stdafx.h"
#include "CreateFileList.h"

bool RemoveReadOnlyFlag(const Util::C_String& filePath)
{
	Sys::C_Fileinfo info;
	if (Sys::C_FileSystem::Instance().FileInfo(filePath, &info))
	{
		//If the file exists and is not writable.
		if (info.m_FileType == Sys::E_FileType_File && !(info.m_FileAccess & Sys::E_FileAccess_Write))
		{
			if (!Sys::C_FileSystem::Instance().FileAccess(filePath, info.m_FileAccess | Sys::E_FileAccess_Write))
			{
				EXECUTER_FATAL("ERROR: Could not set file permissions for %s.\n", filePath.Chars());
				return false;
			}
		}
	}

	return true;
}

bool C_CreateFileList::Process(
	Tools::FileCopy::C_FileCopyList& uberCopyList,
	const C_Options& options)
{
	//Ordinarily our input file would be coming from our 'exportpath' and we'd use the string mangling below
	//Util::C_String relativeOutputFile = options.inputFilePath.IReplace(options.exportPath, "");

	bool success = true;
	//Write out data for our different skus.
	for(S32 platformNo=0; platformNo < Tools::E_Platform_Count; platformNo++)
    {
		//Create output folder.
		Util::C_String relativeOutputFile = options.GetOutputPath() +  Util::ExtractFilename(options.GetInputAssetFilePath());
		relativeOutputFile = relativeOutputFile.IReplace(options.GetRootTempPath(), "");

		Util::C_String outputFilePath = 
			options.GetCopyLocations()[platformNo].workingPath + relativeOutputFile;

		//Create output folder.
		success = Sys::C_FileSystem::Instance().CreateDirectory(Util::ExtractDirectory(outputFilePath));
		if (!success) goto exit;

		EXECUTER_INFO("Copying [%s] to [%s]\n", options.GetInputAssetFilePath().Chars(), outputFilePath.Chars());

		success = RemoveReadOnlyFlag(outputFilePath);
		if (!success) goto exit;
		
		//Perform copy.
		if (!CopyFile(
			options.GetInputAssetFilePath().Chars(),
			outputFilePath.Chars(),
			FALSE))
		{
			success = false;
			goto exit;
		}
		
		uberCopyList.AddFileCopyItem(
			(Tools::E_Platform)platformNo,
			Tools::FileCopy::C_FileCopyItem((const char*)outputFilePath.Chars(), ""));
	}

exit:
	return success;
}



