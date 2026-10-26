#include "stdafx.h"
#include <system/misc/BtlStringUtil.h>
#include <file_tag_db/FileTagDB.h>
#include <tools_shared/utility/AssetMeta.h>

namespace Tools
{

bool MetaMismatch(
	const Sys::C_FilePath& outputFilePath,
	const Sys::C_FilePath& sourceFilePath,
	S64 version)
{
	if (!Sys::GetDefaultFileSystem()->FileExists(outputFilePath))
		return true;

	// If file exists.
	FileTag::C_MetaTag metaTag;
	if(!FileTag::C_MetaManager::Instance().GetTag(metaTag, outputFilePath.Chars()))
		//No meta-data - mismatch.
		return true;
	
	if (version != metaTag.GetBuildTime())
		return true;

	Ut::Vector<FileTag::C_MetaTag::C_Input> inputs;
	metaTag.GetInputs(inputs);
	if (inputs.Size() == 0)
		return true;
	
	//Bail if source file different.
	Sys::C_FilePath a = sourceFilePath;
	Sys::C_FilePath b = inputs[0].file.Chars();
	if (stricmp(a.Chars(), b.Chars()))
		return true;

	for (S32 i = 0; i < inputs.Size(); ++i)
	{
		const FileTag::C_MetaTag::C_Input& input = inputs[i];

		//Check if dest filetime == input filetime.
		FILETIME metaInputFileTime;
		metaInputFileTime.dwLowDateTime = (DWORD)(input.fileTime & 0x00000000ffffffff);
		metaInputFileTime.dwHighDateTime = (DWORD)((input.fileTime >> 32) & 0x00000000ffffffff);

		WIN32_FILE_ATTRIBUTE_DATA destDad;
		if (!GetFileAttributesEx(input.file.Chars(), GetFileExInfoStandard, &destDad))
		{
			//Query failed.
			if (input.fileTime == 0)
				//File was missing in the first place, should never have been added as an input.
				continue;
			else
				//File's missing now but wasn't originally.
				return true;
		}

		LONG result = CompareFileTime(&metaInputFileTime, &destDad.ftLastWriteTime);
		if (result != 0)
			//Mismatch
			return true;
	}

	//No mismatch
	return false;
}

bool MetaMismatch(
	const Sys::C_FilePath& outputFilePath,
	const Sys::C_FilePath& sourceFilePath,
	const Sys::C_FilePath& toolFilePath)
{

	// Obtain the modification time for the exe
	WIN32_FILE_ATTRIBUTE_DATA exeFileAttribs;
	if (!GetFileAttributesEx(toolFilePath.Chars(), GetFileExInfoStandard, &exeFileAttribs)) 
		return true;
	

	S64 version = exeFileAttribs.ftLastWriteTime.dwLowDateTime;
	version += S64(exeFileAttribs.ftLastWriteTime.dwHighDateTime) << 32;
	
	return MetaMismatch(outputFilePath, sourceFilePath, version);
}

}
