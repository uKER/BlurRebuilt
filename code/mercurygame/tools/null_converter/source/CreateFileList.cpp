#include "stdafx.h"
#include "CreateFileList.h"

bool RemoveReadOnlyFlag(const Sys::C_FilePath& filePath)
{
	Sys::C_FileInfo info;
	if (Sys::GetDefaultFileSystem()->FileInfo(filePath, &info))
	{
		//If the file exists and is not writable.
		if (info.m_FileType == Sys::E_FileType_File && !(info.m_FileAccess & Sys::E_FileAccess_Write))
		{
			if (!Sys::GetDefaultFileSystem()->FileAccess(filePath, info.m_FileAccess | Sys::E_FileAccess_Write))
			{
				LOGERROR(g_log, "ERROR: Could not set file permissions for %s.\n", filePath.Chars());
				return false;
			}
		}
	}

	return true;
}

bool ProcessFile(Tools::FileCopy::C_FileCopyList& uberCopyList, const Sys::C_FilePath& srcFilePath, S32 platformIndex, const C_Options& options)
{
	//Make src path relative to where our input is coming from
	Sys::C_FilePath srcRelativePath = srcFilePath.IReplace(options.inputAssetRootPath, "");

	Sys::C_FilePath workingRootPath = 
		Sys::C_FilePath(options.GetRootWorkingPath()) + Sys::C_FilePath("/") + Util::GetPlatformString((Util::E_Platform)platformIndex) + Sys::C_FilePath("/") +  options.outputPath;

	Sys::C_FilePath outputFilePath = workingRootPath + srcRelativePath;


	//Create output folder.
	Sys::C_FilePath outputPath = Sys::ExtractDirectory(outputFilePath);
	if (!Sys::GetDefaultFileSystem()->FileExists(outputPath))
	{
		if (!Sys::GetDefaultFileSystem()->CreateDirectory(outputPath))
		{
			LOGERROR(g_log, "Failed to create missing directory %s", outputPath.Chars());
			return false;
		}
	}

	if (!RemoveReadOnlyFlag(outputFilePath))
		return false;
	
	//Perform copy.
	if (!CopyFile(
		srcFilePath.Chars(),
		outputFilePath.Chars(),
		FALSE))
	{
		LOGERROR(g_log, "Failed to copy file %s to %s", srcFilePath.Chars(), outputFilePath.Chars());
		return false;
	}
	
	uberCopyList.AddFileCopyItem(
		(Util::E_Platform)platformIndex,
		Tools::FileCopy::C_FileCopyItem((const char*)outputFilePath.Chars(), ""));


	return true;
}

bool ProcessFolder(Tools::FileCopy::C_FileCopyList& uberCopyList, const Sys::C_FilePath& srcFilePath, S32 platformIndex, const C_Options& options)
{
	Sys::C_DirectoryInfo dirInfo;
	if (!Sys::GetDefaultFileSystem()->DirectoryInfo(srcFilePath, &dirInfo))
	{
		LOGERROR(g_log, "Failed to find folder %s", srcFilePath.Chars());
		return false;
	}
	
	const Sys::FileInfoArr& fileInfoArr = dirInfo.FileInfoArray();
	for (S32 i = 0; i < fileInfoArr.Size(); ++i)
	{
		const Sys::C_FileInfo& info = fileInfoArr[i];
		if (info.m_FileType == Sys::E_FileType_File)
		{
			if (!ProcessFile(uberCopyList, info.m_FileName, platformIndex, options))
				return false;
		}
		//Recurse folders.
		else if (info.m_FileType == Sys::E_FileType_Directory)
		{
			if (!ProcessFolder(uberCopyList, info.m_FileName, platformIndex, options))
				return false;
		}
	}


	return true;
}

bool C_CreateFileList::Process(
	Tools::FileCopy::C_FileCopyList& uberCopyList,
	const C_Options& options)
{
	if (options.inputAssetFilePath.ISearch(options.GetRootAssetPath()) != 0)
	{
		EXECUTER_ERROR("Copy input must from from the asset folder %s, the input given was %s", options.GetRootAssetPath().Chars(), options.inputAssetFilePath.Chars());
		return false;
	}

	//Write out data for our different skus.
	for(S32 platformNo=0; platformNo < Util::E_Platform_Count; platformNo++)
    {
		if (!options.exportForPlatform[platformNo])
			continue;

		if (options.copyType == E_CopyType_File)
		{
			EXECUTER_INFO("Copying file [%s]\n", options.inputAssetFilePath.Chars());
			if (!ProcessFile(
				uberCopyList, 
				options.inputAssetFilePath, 
				platformNo, 
				options))
				return false;
		}
		else if (options.copyType == E_CopyType_Folder)
		{
			Sys::C_FilePath input = options.inputAssetFilePath;
			EXECUTER_INFO("Copying folder [%s]\n", input.Chars());
			if (!ProcessFolder(
				uberCopyList, 
				input, 
				platformNo, 
				options))
				return false;
		}
	}

	return true;
}
