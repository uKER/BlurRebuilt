#include "stdafx.h"

#include <Aftk/Aftk.h>
#include <tools_shared/utility/Misc.h>
#include <File_Tag_Db/FileTagDB.h>

#include <maya/MGlobal.h>
#include <maya/MCommandResult.h>
#include <maya/MStringArray.h>

#include "Definitions.h"
#include "Meta.h"
#include <AftkConv/Version.h>

bool SaveMeta(const Sys::C_FilePath& mainInput, const Sys::C_FilePath& output)
{
	Ut::Vector<FileTag::C_MetaTag::C_Input> inputs;

	//Add main input file.
	{
		inputs.PushBack();
		FileTag::C_MetaTag::C_Input& input = inputs.Back();
		input.file = mainInput;
		WIN32_FILE_ATTRIBUTE_DATA attribs;
		if (!GetFileAttributesEx(input.file.Chars(), GetFileExInfoStandard, &attribs))
		{
			LOGERROR(g_log, "Failed to query file time for %s", input.file.Chars());
			return false;
		}
		input.fileTime = (S64)(attribs.ftLastWriteTime.dwHighDateTime)<<32;
		input.fileTime |= attribs.ftLastWriteTime.dwLowDateTime;
	}

	//Add references.
	Aftk::E_FileType fileType;
	Aftk::C_Global::GetFileType(mainInput.Chars(), fileType);
	if ((fileType == Aftk::E_FileType_Mb || fileType == Aftk::E_FileType_Ma))
	{

		Btl::Set<Sys::C_FilePath> uniqueFiles;
		{
			MStringArray arrResult;
			MCommandResult result;
			MStatus mStatus = MGlobal::executeCommand("file -q -l", result, true, false);
			if (!mStatus) 
			{
				LOGERROR(g_log, "Failed execute file -q -l");
				return false;
			}
			mStatus = result.getResult(arrResult);
			if (!mStatus) 
			{
				LOGERROR(g_log, "Failed to get result array");
				return false;
			}
			for (U32 i = 0; i < arrResult.length(); ++i)
				if (mainInput != Sys::C_FilePath(arrResult[i].asChar()))
					uniqueFiles.Insert(Btl::String(arrResult[i].asChar()));
		}
		{
			MStringArray arrResult;
			MCommandResult result;
			MStatus mStatus = MGlobal::executeCommand("biz_mb_listDependencies();", result, true, false);
			if (!mStatus)
			{
				LOGERROR(g_log, "Failed to execute biz_DO_ListInstancePaths");
				return false;
			}
			mStatus = result.getResult(arrResult);
			if (!mStatus) 
			{
				LOGERROR(g_log, "Failed to get result array");
				return false;
			}
			for (U32 i = 0; i < arrResult.length(); ++i)
				if (mainInput != Sys::C_FilePath(arrResult[i].asChar()))
					uniqueFiles.Insert(Btl::String(arrResult[i].asChar()));
		}


		for (Btl::Set<Sys::C_FilePath>::Iterator it = uniqueFiles.Begin(); it != uniqueFiles.End(); ++it)
		{
			const Sys::C_FilePath& str = *it;
			Aftk::C_Global::GetFileType(str.Chars(), fileType);
			if ((fileType == Aftk::E_FileType_Mb || fileType == Aftk::E_FileType_Ma))
			{
				inputs.PushBack();
				FileTag::C_MetaTag::C_Input& input = inputs.Back();
				input.file = str.Chars();

				WIN32_FILE_ATTRIBUTE_DATA attribs;
				if (!GetFileAttributesEx(input.file.Chars(), GetFileExInfoStandard, &attribs)) 
				{
					LOGWARN(g_log, "MISSING REFERENCE %s ! An artist needs to fix this! Mising ref filetime set to zero.", input.file.Chars());
					input.fileTime = 0;
				}
				else
				{
					input.fileTime = (S64)(attribs.ftLastWriteTime.dwHighDateTime)<<32;
					input.fileTime |= attribs.ftLastWriteTime.dwLowDateTime;
				}
			}
		}
	}


	FileTag::C_MetaTag metaTag;
	if (FileTag::C_MetaManager::Instance().AddTag(metaTag, output.Chars()))
	{
		metaTag.SetBuildTime(AFTK_CONV_CURRENT_VERSION);
		metaTag.SetInputs(inputs);
	}

	//Flush databases to disk.
	if (!FileTag::C_MetaManager::Instance().Write())
	{
		LOGERROR(g_log, "Failed to write tag db!");
		return false;
	}
	return true;
}