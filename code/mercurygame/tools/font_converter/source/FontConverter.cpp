#include "stdafx.h"
#include "FontConverter.h"
#include "Font.h"
#include "tools_shared/PlatformHelpers.h"
#include <converter_shared/helpers/ExportHelpers.h>

bool RemoveReadOnlyFlag(const Util::C_String& filePath)
{
	Sys::C_FileInfo info;
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

bool C_FontConverter::Process(
	Tools::FileCopy::C_FileCopyList& uberCopyList,
	const C_Options& options)
{
	//Ordinarily our input file would be coming from our 'exportpath' and we'd use the string mangling below
	//Util::C_String relativeOutputFile = options.inputFilePath.IReplace(options.exportPath, "");
	
	Font::C_Font fontResource;

	//Read Texture
	fontResource.ReadFontTexture(options.GetTextureFilePath());

	//Read Charlist
	fontResource.ReadCharList(options.GetCharlistFilePath());

	//Process
	fontResource.Process();
	


	bool success = true;
	//Write out data for our different skus.
	for(S32 platformNo=0; platformNo < Util::E_Platform_Count; platformNo++)
    {
		//Create output folder.
		Util::C_String charlistName = Util::RemoveFileExtension(Util::ExtractFilename(options.GetInputAssetFilePath()));
		U32 filePointPtr = charlistName.RevSearch('_');
		Util::C_String fontName = charlistName(0, filePointPtr);

		Util::C_String relativeOutputFile = options.GetOutputPath() +  fontName;
		relativeOutputFile = relativeOutputFile.IReplace(options.GetRootTempPath(), "");

		Util::C_String outputFilePath = 
			options.GetCopyLocations()[platformNo].workingPath + relativeOutputFile;

		success = RemoveReadOnlyFlag(outputFilePath);
		if (!success) goto exit;

		outputFilePath += ".font"; 


		//Export
		Sys::I_File* pFile = ExportHelpers::WriteExportFile( outputFilePath, (Util::E_Platform)platformNo, &uberCopyList);
	    if(pFile)
	    {	
			EXECUTER_INFO("Creating [%s]\n", outputFilePath.Chars());	    
			
			fontResource.Write(Util::E_Platform(platformNo), *pFile);
    
			SAFE_DELETE_PTR(pFile);
	    }
	}

exit:
	return success;
}



