#include "stdafx.h"
#include "FontConverter.h"
#include "Font.h"
#include "tools_shared/PlatformHelpers.h"

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
	for(S32 platformNo=0; platformNo < Tools::E_Platform_Count; platformNo++)
    {
		//Create output folder.
		Util::C_String charlistName = Util::RemoveFileExtension(Util::ExtractFilename(options.GetInputAssetFilePath()));
		U32 filePointPtr = charlistName.RevSearch('_');
		Util::C_String fontName = charlistName(0, filePointPtr);

		Util::C_String relativeOutputFile = options.GetOutputPath() +  fontName;
		relativeOutputFile = relativeOutputFile.IReplace(options.GetRootTempPath(), "");

		Util::C_String outputFilePath = 
			options.GetCopyLocations()[platformNo].workingPath + relativeOutputFile;

		//Create output folder.
		success = Sys::C_FileSystem::Instance().CreateDirectory(Util::ExtractDirectory(outputFilePath));
		if (!success) goto exit;

		success = RemoveReadOnlyFlag(outputFilePath);
		if (!success) goto exit;

		outputFilePath += ".font"; 


		bool swizzle = Tools::C_PlatformHelper::ShouldEndianSwap(Tools::E_Platform(platformNo));
		
		//Export
		Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open( outputFilePath, Sys::E_FileAccess_Write );
	    if(pFile)
	    {	
			EXECUTER_INFO("Creating [%s]\n", outputFilePath.Chars());

			pFile->SetEndianSwapFlag(swizzle);		    
			
			fontResource.Write(Tools::E_Platform(platformNo), *pFile);
    
			SAFE_DELETE_PTR(pFile);
	    }
		
		uberCopyList.AddFileCopyItem(
			(Tools::E_Platform)platformNo,
			Tools::FileCopy::C_FileCopyItem((const char*)outputFilePath.Chars(), ""));
	}

exit:
	return success;
}



