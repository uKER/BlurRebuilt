
#include "stdafx.h"

#include "system\time\Time.h"
#include "system\debuglog.h"
#include <system/debug/log/AutoConfigure.h>

Time::C_Time kMagicFileTime = 1232470000000000;

U32 kMagic = 'NTCH';

static Debug::Logger& g_Log = Debug::GetLogger("NewTechSpec");

bool CopyFile( const Util::C_String& src, const Util::C_String& dst )
{
	Sys::I_File* pInfile = Sys::C_FileSystem::Instance().Open( src );
	if(pInfile == NULL)
	{
		LOGINFO( g_Log, "*** Failed to open %s for reading", src.Chars());
		return false;
	}
	Sys::I_File* pOutfile = Sys::C_FileSystem::Instance().Open( dst, Sys::E_FileAccess_Write );
	if(pOutfile == NULL)
	{
		LOGINFO( g_Log, "*** Failed to open %s for writing", dst.Chars());
		return false;
	}

	Sys::C_StreamSize fileLength = pInfile->Length();

	U8* pBuffer = new U8[(S32)fileLength];

	pInfile->Read( pBuffer, (S32)fileLength );
	pOutfile->Write( pBuffer, (S32)fileLength );

	SAFE_DELETE_ARY( pBuffer );

	SAFE_DELETE_PTR( pInfile );
	SAFE_DELETE_PTR( pOutfile );

	// copy file time
	Sys::C_Fileinfo fileInfo;
	Sys::C_FileSystem::Instance().FileInfo(src, &fileInfo);
	Sys::C_FileSystem::Instance().FileTime( dst, fileInfo.m_FileTime );

	return true;
}

bool IsNewTech( const Util::C_String& fileName )
{
	Sys::C_Fileinfo fileInfo;
	Sys::C_FileSystem::Instance().FileInfo(fileName, &fileInfo);

	Sys::C_StreamSize fileSize = fileInfo.m_FileSize;
	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open( fileName );
	if(pFile == NULL)
	{
		LOGINFO( g_Log, "Failed to open %s for reading", fileName.Chars());
		return true;
	}
	pFile->SetPosition( fileSize - 4 );
	U32 data;
	pFile->Read(&data, 4);
	SAFE_DELETE_PTR( pFile );

	//if(fileInfo.m_FileTime == kMagicFileTime)
	//	return true;

	if( data == kMagic )
		return true;

	return false;
}

bool SetNewTech( const Util::C_String& fileName )
{
	Util::C_String tempFile = "Z:\\temp.dat";
	CopyFile( fileName, tempFile );

	Sys::I_File* pInfile = Sys::C_FileSystem::Instance().Open( tempFile, Sys::E_FileAccess_Read );
	if(pInfile == NULL)
	{
		LOGINFO( g_Log, "*** Failed to open %s for reading", tempFile.Chars());
		return false;
	}

	Sys::I_File* pOutfile = Sys::C_FileSystem::Instance().Open( fileName, Sys::E_FileAccess_Write );
	if(pInfile == NULL)
	{
		LOGINFO( g_Log, "*** Failed to open %s for writing", fileName.Chars());
		return false;
	}
	Sys::C_StreamSize fileLength = pInfile->Length();

	U8* pBuffer = new U8[(S32)fileLength + 4];

	pInfile->Read( pBuffer, (S32)fileLength );

	*((U32*)&pBuffer[fileLength]) = kMagic;
	pOutfile->Write( pBuffer, (S32)fileLength + 4);

	SAFE_DELETE_ARY( pBuffer );

	SAFE_DELETE_PTR( pInfile );
	SAFE_DELETE_PTR( pOutfile );

/*

	Sys::C_Fileinfo fileInfo;
	Sys::C_FileSystem::Instance().FileInfo(fileName, &fileInfo);

	Sys::C_StreamSize fileSize = fileInfo.m_FileSize;
	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open( fileName, Sys::E_FileAccess_Append );

	pFile->SetPosition( fileSize );
	pFile->Write(&kMagic, 4);
	SAFE_DELETE_PTR( pFile );
*/
	/*
	Time::C_Time currentTime = Time::GetCurrentTime();
	//LOGINFO( g_Log, "   Current time %I64d '%s'\n", currentTime, Time::FormatDate_TimeString(currentTime).Chars());

	Time::C_Time newFileTime = kMagicFileTime;

	//LOGINFO( g_Log, "   Setting file time to '%s'\n", Time::FormatDate_TimeString(newFileTime).Chars());

	Sys::C_FileSystem::Instance().FileTime(fileName, newFileTime);

	*/
	return true;
}

bool ConvertImage( Attk::C_Image& srcImage, Attk::C_Image& destImage )
{
	bool bHasAlpha = false;
	//LOGINFO( g_Log, "   Converting\n");

	const S32 width = srcImage.GetWidth();
	const S32 height = srcImage.GetHeight();

	const F32 scale = 1.0f;

	for(S32 y=0; y<height; y++)
	{
		for(S32 x=0; x<width; x++)
		{
			Attk::C_PixelR32G32B32A32F destPixel(0,0,0,0);
			
			Attk::C_PixelR32G32B32A32F srcPixel = srcImage.GetPixel( x,y );

			F32 r = srcPixel.red;
			F32 g = srcPixel.green;
			F32 b = srcPixel.blue;
			F32 a = srcPixel.alpha;

			F32 redScale = 0.9f;
			F32 redPower = 0.25f;

			F32 greenScale = 1.1f;
			F32 greenPower = 1.0f;

			F32 blueScale = 0.5f;
			F32 bluePower = 1.0f;

			r = pow(r, redPower) * redScale;
			g = pow(g, greenPower) * greenScale;
			b = pow(b, bluePower) * blueScale;

			if( r>1.0f ) r = 1.0f;
			if( g>1.0f ) g = 1.0f;
			if( b>1.0f ) b = 1.0f;

			if( r<0.0f ) r = 0.0f;
			if( g<0.0f ) g = 0.0f;
			if( b<0.0f ) b = 0.0f;

			destPixel.red = r;
			destPixel.green = g;
			destPixel.blue = b;
			destPixel.alpha = a;

			if( a > 0.0f )
			{
				bHasAlpha = true;
			}

			destImage.SetPixel( destPixel, x, y );
		}
	}
	return bHasAlpha;
}


void BackupFile( const Util::C_String& inFileName )
{
	Util::C_String outFileName = inFileName + ".oldtech_backup";

	CopyFile( inFileName, outFileName );
}

void ConvertImage( Util::C_String& inFileName, Util::C_String& outFileName )
{
	BackupFile(inFileName);

	LOGINFO( g_Log, "   Converting %s\n", inFileName.Chars());
	
	Attk::C_Image srcImage;
	srcImage.Read( inFileName );

	const U32 width = srcImage.GetWidth();
	const U32 height = srcImage.GetHeight();

	//LOGINFO( g_Log, "   Creating destination image\n");
	Attk::C_Image destImage;

	destImage.Clear();
	destImage.DeletePixelData();

	//destImage.AddFrame();
	//destImage.AddLayer(width, height, 1);
	//destImage.AddMipLevel();
	destImage.NewPixelData( width, height, 1, Attk::E_PixelFormat_B8G8R8A8U );
	destImage.Fill( Attk::C_PixelR32G32B32A32F(1.0f, 1.0f, 1.0f, 0.0f) );


	bool bHasAlpha = ConvertImage(srcImage, destImage);

	destImage.ChangePixelFormat( srcImage.GetPixelFormat() );
	//LOGINFO( g_Log, "   Writing output %s\n", outFileName.Chars());
	//Attk::C_Options options;
	//options.AddAttribute();
	bool status = destImage.Write( outFileName, Attk::E_ImageFormat_BestGuess );
	if( !status )
	{
		LOGINFO( g_Log, "   **** Error writing '%s'\n", outFileName.Chars());
	}

	SetNewTech( outFileName );

	//LOGINFO( g_Log, "   Done\n\n", outFileName.Chars());
}

void ProcessFile( Util::C_String& fileName )
{
	Util::C_String ext = Util::ExtractFileExtension(fileName).Lower();

	if( (ext == "tga") || (ext == "bmp") || (ext == "pic") )
	{
		if( (fileName.ISearch("_IPR0") != -1) || (Util::ExtractFilename(fileName).ISearch("S0_") == 0))
		{
			Sys::C_Fileinfo fileInfo;
			Sys::C_FileSystem::Instance().FileInfo(fileName, &fileInfo);

			if( !(fileInfo.m_FileAccess & Sys::E_FileAccess_Read) )
			{
				LOGINFO( g_Log, "   *** No read access for %s - FILE NOT CONVERTED\n", fileName.Chars());
			
			}
			else
			if( !(fileInfo.m_FileAccess & Sys::E_FileAccess_Write) )
			{
				LOGINFO( g_Log, "   *** No write access for %s - FILE NOT CONVERTED\n", fileName.Chars());
			}
			else
			{
				if(IsNewTech(fileName))
				{
					LOGINFO( g_Log, "   **** Already Converted %s\n", fileName.Chars());
				}
				else
				{

					/*
					Util::C_String outputFilename;

					Util::C_String directory = Util::ExtractDirectory(fileName);
					Util::C_String filename = Util::RemoveFileExtension( Util::ExtractFilename(fileName) );
					Util::C_String extension = Util::ExtractFileExtension(fileName);
					outputFilename = directory + filename + "_NewTech." +extension;

					ConvertImage( fileName, outputFilename );
					*/
					ConvertImage( fileName, fileName );
				
				}
			}
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

