#include "stdafx.h"
#include "TextureConverter.h"
#include "attk/attk.h"
#include "tools_shared/PlatformHelpers.h"
#include "texture_export/texture.h"
#include "texture_export/targets/pctex.h"
#include "texture_export/targets/ps3tex.h"
#include "texture_export/targets/xetex.h"
#include "texture_export/converters/atidxt.h"
#include "texture_export/converters/generatemipmaps.h"
#include "texture_export/utility.h"
#include <system/perf.h>


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

bool C_TextureConverter::Process(
	Tools::FileCopy::C_FileCopyList& uberCopyList,
	const C_Options& options)
{
	PERF_EVENT("Process");
	bool success = true;
	Attk::C_Image* pImage = NULL;

	//Read Texture
	if (Sys::C_FileSystem::Instance().FileExists(options.GetInputAssetFilePath()))
	{
		pImage = new Attk::C_Image();
		if (!pImage->Read(options.GetTextureFilePath()))
		{
			EXECUTER_FATAL("Error reading texture %s", options.GetInputAssetFilePath());
			return false;
		}
	}
	else
	{
		EXECUTER_FATAL("Texture %s doesn't exist", options.GetInputAssetFilePath());
		return false;
	}

	//Process Texture

	U32 widthResize, heightResize;
	if (options.GetResize(widthResize, heightResize))
	{
		
		AttkUt::C_ImageResize::Resize(*pImage, widthResize, heightResize, pImage->GetDepth());
		EXECUTER_INFO("Image Resized to %i,%i", widthResize, heightResize);
	}

	if (options.GetColourNormalize())
	{
		ColourNormalize(pImage);
	}

	//Write out data for our different skus.
	for(S32 platformNo=0; platformNo < Tools::E_Platform_Count; platformNo++)
    {
		//Create output folder.
		Util::C_String textureName = Util::RemoveFileExtension(Util::ExtractFilename(options.GetInputAssetFilePath()));
	
		Util::C_String relativeOutputFile = options.GetOutputPath() +  textureName;
		relativeOutputFile = relativeOutputFile.IReplace(options.GetRootTempPath(), "");

		Util::C_String outputFilePath = 
			options.GetCopyLocations()[platformNo].workingPath + relativeOutputFile;

		//Create output folder.
		success = Sys::C_FileSystem::Instance().CreateDirectory(Util::ExtractDirectory(outputFilePath));
		if (!success) goto exit;

		success = RemoveReadOnlyFlag(outputFilePath);
		if (!success) goto exit;

		outputFilePath += ".texture"; 


		bool swizzle = Tools::C_PlatformHelper::ShouldEndianSwap(Tools::E_Platform(platformNo));
		
		//Export
		Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open( outputFilePath, Sys::E_FileAccess_Write );
	    if(pFile)
	    {	
			EXECUTER_INFO("Creating [%s]\n", outputFilePath.Chars());

			pFile->SetEndianSwapFlag(swizzle);		    
			
			WriteTexture(pImage, options, Tools::E_Platform(platformNo), *pFile);
    
			SAFE_DELETE_PTR(pFile);
	    }
		
		uberCopyList.AddFileCopyItem(
			(Tools::E_Platform)platformNo,
			Tools::FileCopy::C_FileCopyItem((const char*)outputFilePath.Chars(), ""));
	}

exit:
	SAFE_DELETE_PTR(pImage);
	return success;
}

enum E_TextureFileVersion
{
	E_TextureFileVersion_Initial = 0,
	E_TextureFileVersion_Current = E_TextureFileVersion_Initial
};


void C_TextureConverter::WriteTexture( Attk::C_Image* pImage, const C_Options& options, Tools::E_Platform platform, Sys::I_File& file )
{
	PERF_EVENT("Write Texture");
	//Write blocks and convert
	Sys::C_FileBlock fileBlock( &file, "Texture" );

	{
		Sys::C_FileBlock headerBlock( &fileBlock, "Header" );
		Sys::BinarySerialiserAutoEndian ar( headerBlock );

		ar << U32(E_TextureFileVersion_Current);
		ar << U32((options.GetGamma())?1:0); //Gamma
	}

	{
		Sys::C_FileBlock dataBlock( &fileBlock, "Data" );

		// convert from Attk to export_texture format
		ActTools::Texture *pTexture = TextureExport::TextureFromAttkImage(pImage);

		//Check if needs generate mipmaps
		if (options.GetGenerateMipmaps())
		{
			Texutils::GenerateMipmaps(pTexture, ActTools::FT_TRIANGLE, 0.f, ActTools::FT_TRIANGLE, 0.f, ActTools::EDGE_CLAMP, ActTools::EDGE_CLAMP, 0);
		}

		// convert to DXT5 using ATI library first
		if (options.GetTextureFormat() == "DXT1" || options.GetTextureFormat() == "DXT3" || options.GetTextureFormat() == "DXT5")
		{
			ActTools::TexelFormat *p = AtiDXT::GetTexelFormat(options.GetTextureFormat());
			pTexture->ConvertTo(p);
		}
		if (options.GetGamma())
		{
			pTexture->SetSRGBFlag(true);
		}
		else
		{
			pTexture->SetSRGBFlag(false);
		}
		switch (platform)
		{
		case Tools::E_Platform_360:
			{
				NglXeTex texWriter;
				ActTools::TexelFormat *pFormat = texWriter.GetTexelFormat(options.GetTextureFormat());
				pTexture->ConvertTo(pFormat);
				delete pFormat;
		
				texWriter.SerialOutTexture(&dataBlock, pTexture);
			}
			break;
		case Tools::E_Platform_PC:
			{
				NglPcTex texWriter;
				ActTools::TexelFormat *pFormat = texWriter.GetTexelFormat(options.GetTextureFormat());
				pTexture->ConvertTo(pFormat);
				delete pFormat;

				texWriter.SerialOutTexture(&dataBlock, pTexture);
			}
			break;
		case Tools::E_Platform_PS3:
			{
				NglPs3Tex texWriter;
				ActTools::TexelFormat *pFormat = texWriter.GetTexelFormat(options.GetTextureFormat());
				pTexture->ConvertTo(pFormat);
				delete pFormat;

				texWriter.SerialOutTexture(&dataBlock, pTexture);
			}
			break;
		}
		delete pTexture;
	}
}

void C_TextureConverter::ColourNormalize( Attk::C_Image* pImage )
{
	//Each pixel, normalize
	PERF_EVENT("ColourNormalize");
	EXECUTER_INFO("Colour Normalize, Alpha channel deleted");
	for (U32 x = 0; x < pImage->GetWidth(); x++)
		for (U32 y = 0; y < pImage->GetWidth(); y++)
		{
			Attk::C_PixelR32G32B32A32F pixelColour = pImage->GetPixel(x,y);

			//Normalize
			F32 maxColour = Maths::Max(pixelColour.red, Maths::Max(pixelColour.blue, pixelColour.green));
			
			//Lost Alpha
			pixelColour.Set(Attk::C_PixelR32G32B32A32F(pixelColour.red / maxColour, pixelColour.green / maxColour, pixelColour.blue / maxColour, maxColour));
			pImage->SetPixel(pixelColour, x, y);
		}
}

