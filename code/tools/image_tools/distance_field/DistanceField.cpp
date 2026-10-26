
#include "stdafx.h"

#include "system\time\Time.h"
#include "system\debuglog.h"
#include <system/debug/log/AutoConfigure.h>

static Debug::Logger& g_Log = Debug::GetLogger("NewTechSpec");

bool ThresholdPixel( const Attk::C_PixelR32G32B32A32F& pixel )
{
	if(pixel.red > 0.5f)
		return true;

	return false;
}

F32 GetDist( Attk::C_Image& image, F32 centreX, F32 centreY )
{
	S32 iX = (S32)centreX;
	S32 iY = (S32)centreY;

	Attk::C_PixelR32G32B32A32F seedPixel(0,0,0,0);
	seedPixel = image.GetPixel(iX, iY);

	bool seedPixelSet = ThresholdPixel(seedPixel);


	S32 size = 32;
	F32 maxDist = sqrtf((F32)size * (F32)size);

	F32 closestDist = maxDist;
	for(S32 y=iY - size; y<iY+size; y++)
	{
		for(S32 x=iX - size; x<iX+size; x++)
		{
			if((x >= 0) && (y >= 0) && (x < image.GetWidth()) && (y < image.GetHeight()))
			{
				Attk::C_PixelR32G32B32A32F currPixel(0,0,0,0);
				currPixel = image.GetPixel( x, y );
				
				bool currPixelSet = ThresholdPixel(currPixel);
				if( currPixelSet != seedPixelSet )
				{
					F32 dx = centreX - (F32)(x + 0.5f);
					F32 dy = centreY - (F32)(y + 0.5f);
					F32 currDist = dx * dx + dy * dy;
					if( currDist > 0.0f )
						currDist = sqrt(currDist);

					if( currDist < closestDist )
					{
						closestDist = currDist;
					}
				}

			}

		}
	}

	if(seedPixelSet)
		return -closestDist / maxDist;
	else
		return closestDist / maxDist;
}

bool ConvertImage( Attk::C_Image& srcImage, Attk::C_Image& destImage )
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

			F32 dist = GetDist( srcImage, pixelCentreX, pixelCentreY );

			destPixel.red = 0.5f + dist * 0.5f;
			destPixel.green = 0.5f + dist * 0.5f;
			destPixel.blue = 0.5f + dist * 0.5f;

			destImage.SetPixel( destPixel, x,y );
			//Attk::C_PixelR32G32B32A32F destPixel(0,0,0,0);
			//
			//Attk::C_PixelR32G32B32A32F srcPixel = srcImage.GetPixel( x,y );

			//F32 r = srcPixel.red;
			//F32 g = srcPixel.green;
			//F32 b = srcPixel.blue;
			//F32 a = srcPixel.alpha;

			//F32 redScale = 0.9f;
			//F32 redPower = 0.25f;

			//F32 greenScale = 1.1f;
			//F32 greenPower = 1.0f;

			//F32 blueScale = 0.5f;
			//F32 bluePower = 1.0f;

			//r = pow(r, redPower) * redScale;
			//g = pow(g, greenPower) * greenScale;
			//b = pow(b, bluePower) * blueScale;

			//if( r>1.0f ) r = 1.0f;
			//if( g>1.0f ) g = 1.0f;
			//if( b>1.0f ) b = 1.0f;

			//if( r<0.0f ) r = 0.0f;
			//if( g<0.0f ) g = 0.0f;
			//if( b<0.0f ) b = 0.0f;

			//destPixel.red = r;
			//destPixel.green = g;
			//destPixel.blue = b;
			//destPixel.alpha = a;

			//if( a > 0.0f )
			//{
			//	bHasAlpha = true;
			//}

			//destImage.SetPixel( destPixel, x, y );
		}
	}

	return bHasAlpha;
}


void ConvertImage( Util::C_String& inFileName, Util::C_String& outFileName )
{
	LOGINFO( g_Log, "   Converting %s\n", inFileName.Chars());
	
	Attk::C_Image srcImage;
	srcImage.Read( inFileName );

	const U32 srcWidth = srcImage.GetWidth();
	const U32 srcHeight = srcImage.GetHeight();

	const S32 xFactor = 16;
	const S32 yFactor = 16;

	const U32 dstWidth = srcWidth / xFactor;
	const U32 dstHeight = srcHeight / yFactor;

	Attk::C_Image destImage;

	destImage.Clear();
	destImage.DeletePixelData();

	destImage.NewPixelData( dstWidth, dstHeight, 1, Attk::E_PixelFormat_B8G8R8A8U );
	destImage.Fill( Attk::C_PixelR32G32B32A32F(1.0f, 1.0f, 1.0f, 0.0f) );


	bool bHasAlpha = ConvertImage(srcImage, destImage);

	destImage.ChangePixelFormat( srcImage.GetPixelFormat() );

	LOGINFO( g_Log, "Writing Output");

	bool status = destImage.Write( outFileName, Attk::E_ImageFormat_BestGuess );
	if( !status )
	{
		LOGINFO( g_Log, "   **** Error writing '%s'\n", outFileName.Chars());
	}

	
	//LOGINFO( g_Log, "   Done\n\n", outFileName.Chars());
}

void ProcessFile( Util::C_String& fileName )
{
	Util::C_String ext = Util::ExtractFileExtension(fileName).Lower();

	if( (ext == "tga") || (ext == "bmp") || (ext == "pic") )
	{
		Sys::C_Fileinfo fileInfo;
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
			outputFilename = directory + filename + "_DistanceField." +extension;

			ConvertImage( fileName, outputFilename );			
		}
	}
}

void ProcessDirectory( Util::C_String dirName )
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
		Sys::C_Fileinfo& fileInfo = dirInfo.FileInfo( fileIndex );

		switch(fileInfo.m_FileType)
		{
			case Sys::E_FileType_File:
			{
				ProcessFile(fileInfo.m_FileName);
			}
			break;

			case Sys::E_FileType_Directory:
			{
				ProcessDirectory(fileInfo.m_FileName);
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
	if(argc != 2)
	{
		LOGINFO( g_Log, "%s\nRecursively Converts _IPR0 textures to NewTech\n\n", argv[0]);
		LOGINFO( g_Log, "Usage:\n%s TexturePath\n\n", argv[0]);
	}
	else
	{	
		Util::C_String fileName = argv[1];
		//fileName = "Z:\\PGR4\\Art\\Circuits\\TestScene\\Textures";

		fileName = Util::FormatFileString(fileName);

		Sys::C_Fileinfo fileInfo;
		bool result = Sys::C_FileSystem::Instance().FileInfo(fileName, &fileInfo);

		if(result)
		{
			switch(fileInfo.m_FileType)
			{
			case Sys::E_FileType_File:
				{
					ProcessFile(fileName);
				}
				break;

			case Sys::E_FileType_Directory:
				{
					ProcessDirectory(fileName + "/");
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