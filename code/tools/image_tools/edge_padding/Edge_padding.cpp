
#include "stdafx.h"

#include "system\time\Time.h"
#include "system\debuglog.h"
#include <system/debug/log/AutoConfigure.h>

static Debug::Logger& g_Log = Debug::GetLogger("Edge_Padding");

bool ThresholdPixel( const Attk::C_PixelR32G32B32A32F& pixel )
{
	if(pixel.red > 0.5f)
		return true;

	return false;
}

Attk::C_PixelR32G32B32A32F GetNearestPixel( Attk::C_Image& image, S32 centreX, S32 centreY )
{
	S32 iX = (S32)centreX;
	S32 iY = (S32)centreY;

	const S32 radius = 16;
	F32	maxDistant = 100000.f;
	Attk::C_PixelR32G32B32A32F nearestPixel(0,0,0,0);

	for(S32 y=iY - radius; y<iY+radius; y++)
	{
		for(S32 x=iX - radius; x<iX+radius; x++)
		{
			if((x >= 0) && (y >= 0) && (x < image.GetWidth()) && (y < image.GetHeight()))
			{
				Attk::C_PixelR32G32B32A32F currPixel(0,0,0,0);
				currPixel = image.GetPixel( x, y );
				
				if (currPixel.red != 0.f || currPixel.green != 0.f || currPixel.blue != 0.f)
				{
					//It is not black
					//Calculate distant
					F32 distantX = fabsf(F32(x - centreX));
					F32 distantY = fabsf(F32(y - centreY));
					F32 distant = sqrtf(distantX * distantX + distantY * distantY);

					if (distant < maxDistant)
					{
						nearestPixel = currPixel;
						maxDistant = distant;
					}
				}
				
			}

		}
	}

	return nearestPixel;
}

bool ConvertImage( Attk::C_Image& srcImage, Attk::C_Image& destImage, F32 factor )
{	
	bool bHasAlpha = false;

	LOGINFO( g_Log, "Creating Image");

	const U32 srcWidth = srcImage.GetWidth();
	const U32 srcHeight = srcImage.GetHeight();

	const S32 dstWidth = destImage.GetWidth();
	const S32 dstHeight = destImage.GetHeight();

	const F32 scale = 1.0f;

	for(S32 y=0; y<dstHeight; y++)
	{
		if((y % 32) == 0)
		{
			LOGINFO( g_Log, "%d Percent Complete", y * 100 / dstHeight);
		}

		for(S32 x=0; x<dstWidth; x++)
		{
			Attk::C_PixelR32G32B32A32F destPixel(0,0,0,0);

			F32 pixelCentreX = ((F32)x + 0.5f) * (F32)srcWidth / (F32)dstWidth;
			F32 pixelCentreY = ((F32)y + 0.5f) * (F32)srcHeight / (F32)dstHeight;

			//Get Pixel
			Attk::C_PixelR32G32B32A32F srcPixel(0,0,0,0);
			srcPixel = srcImage.GetPixel(x, y);

			//If black
			if (srcPixel.red == 0.f && srcPixel.green == 0.f && srcPixel.blue == 0.f)
			{
				//Expand to nearest not black pixel
				destImage.SetPixel( GetNearestPixel(srcImage, x, y) * factor, x, y);
			}
			else
			{
				//Copy
				destImage.SetPixel( srcPixel * factor, x,y );
			}
		}
	}

	return bHasAlpha;
}


void ConvertImage( Util::C_String& inFileName, Util::C_String& outFileName, F32 factor)
{
	LOGINFO( g_Log, "   Converting %s\n", inFileName.Chars());
	
	Attk::C_Image srcImage;
	srcImage.Read( inFileName );

	const U32 srcWidth = srcImage.GetWidth();
	const U32 srcHeight = srcImage.GetHeight();

	const U32 dstWidth = srcWidth;
	const U32 dstHeight = srcHeight;

	Attk::C_Image destImage;

	destImage.Clear();
	destImage.DeletePixelData();

	destImage.NewPixelData( dstWidth, dstHeight, 1, Attk::E_PixelFormat_B8G8R8A8U );
	destImage.Fill( Attk::C_PixelR32G32B32A32F(0.0f, 0.0f, 0.0f, 0.0f) );


	bool bHasAlpha = ConvertImage(srcImage, destImage, factor);

	destImage.ChangePixelFormat( srcImage.GetPixelFormat() );

	LOGINFO( g_Log, "Writing Output");

	bool status = destImage.Write( outFileName, Attk::E_ImageFormat_BestGuess );
	if( !status )
	{
		LOGINFO( g_Log, "   **** Error writing '%s'\n", outFileName.Chars());
	}

	
	//LOGINFO( g_Log, "   Done\n\n", outFileName.Chars());
}

void ProcessFile( Util::C_String& fileName, F32 factor)
{
	Util::C_String ext = Util::ExtractFileExtension(fileName).Lower();

	if( (ext == "tga") || (ext == "bmp") || (ext == "pic") )
	{
		Sys::C_FileInfo fileInfo;
		Sys::C_FileSystem::Instance().FileInfo(fileName, &fileInfo);

		if( !(fileInfo.m_FileAccess & Sys::E_FileAccess_Read) )
		{
			LOGINFO( g_Log, "   *** No read access for %s - FILE NOT CONVERTED\n", fileName.Chars());
		
		}
		else
		{
			Util::C_String outputFilename;

			Util::C_String directory = Util::ExtractDirectory(fileName);
			Util::C_String filename = Util::RemoveFileExtension( Util::ExtractFilename(fileName) );
			Util::C_String extension = Util::ExtractFileExtension(fileName);
			outputFilename = directory + filename + "_edge_padding." +extension;

			ConvertImage( fileName, outputFilename, factor );			
		}
	}
}

void ProcessDirectory( Util::C_String dirName, F32 factor )
{
	LOGINFO( g_Log, "Scanning '%s'\n", dirName.Chars() );

	Sys::C_DirectoryInfo dirInfo;
	bool result = Sys::C_FileSystem::Instance().DirectoryInfo( dirName, &dirInfo );

	if(!result)
	{
		LOGINFO( g_Log, "Problem Reading Directory Info '%s'\n", dirName.Chars() );
		return;
	}

	const U32 numFiles = dirInfo.NumFiles();

	//LOGINFO( g_Log, "%d files\n", numFiles );

	for(U32 fileIndex=0; fileIndex<numFiles; fileIndex++)
	{
		Sys::C_FileInfo& fileInfo = dirInfo.FileInfo( fileIndex );

		switch(fileInfo.m_FileType)
		{
			case Sys::E_FileType_File:
			{
				ProcessFile(Util::C_String(fileInfo.m_FileName.Chars()), factor);
			}
			break;

			case Sys::E_FileType_Directory:
			{
				ProcessDirectory(Util::C_String(fileInfo.m_FileName.Chars()), factor);
			}
			break;

			default:
			break;
		}
	}

	LOGINFO( g_Log,  "\n" );

}



int main(int argc, char* argv[])
//int _tmain(int argc, _TCHAR* argv[])
{
	Debug::AutoConfigureDebugLog();
	if((argc != 2) && (argc != 3))
	{
		LOGINFO( g_Log, "%s\nExpand borders in lightmap textures\n\n", argv[0]);
		LOGINFO( g_Log, "Usage:\n%s TexturePath [multifactor]\n\n", argv[0]);
	}
	else
	{	
		Util::C_String fileName = argv[1];
		//fileName = "Z:\\PGR4\\Art\\Circuits\\TestScene\\Textures";

		fileName = Util::FormatFileString(fileName);

		F32 factor = 1.f;
		if (argc == 3)
		{
			factor = atof(argv[2]);
			LOGINFO( g_Log, "Factor %f\n", factor);
		}

		Sys::C_FileInfo fileInfo;
		bool result = Sys::C_FileSystem::Instance().FileInfo(fileName, &fileInfo);

		if(result)
		{
			switch(fileInfo.m_FileType)
			{
			case Sys::E_FileType_File:
				{
					ProcessFile(fileName, factor);
				}
				break;

			case Sys::E_FileType_Directory:
				{
					ProcessDirectory(fileName + "/", factor);
				}
				break;

			case Sys::E_FileType_NotPresent:
				{
					LOGINFO( g_Log, "***Could not find file or directory '%s'\n", fileName.Chars());
				}
				break;
			}
		}


		LOGINFO( g_Log, "Press any key...\n");
	}

	getch();

	return 0;
}