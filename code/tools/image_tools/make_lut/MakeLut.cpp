
#include "stdafx.h"

#include "system\time\Time.h"
#include "system\debuglog.h"
#include <system/debug/log/AutoConfigure.h>

#include "Lut.h"

static Debug::Logger& g_Log = Debug::GetLogger("MakeLut");

IDirect3D9 * g_pD3d = NULL;
IDirect3DDevice9* g_pD3dDevice = NULL;

//void ConvertImage( Attk::C_Image& srcImage, Util::C_String& outFileName )
//{	
//	LOGINFO( g_Log, "Creating Image");
//
//	const U32 srcWidth = srcImage.GetWidth();
//	const U32 srcHeight = srcImage.GetHeight();
//
//	//Lut::C_Lut lut;
//
//	//lut.ExtractStrip( srcImage );
//	//lut.WriteDDS( g_pD3dDevice, outFileName );
//
//	Lut::C_Lut idLut;
//	idLut.SetIdentity();
//
//	idLut.WriteDDS( g_pD3dDevice, outFileName );
//	idLut.EmbedStrip(srcImage);
//	srcImage.Write("TestId.bmp");
//
//
//	LPDIRECT3DVOLUMETEXTURE9 p3dTexture;
//	idLut.CreateVolumeTexture( g_pD3dDevice, &p3dTexture);
//
//	Lut::C_Lut ddsLut;
//	//ddsLut.ReadDDS( g_pD3dDevice, outFileName );
//	ddsLut.SetupFromVolumeTexture( g_pD3dDevice, p3dTexture );
//	//ddsLut.WriteDDS( g_pD3dDevice, "OutDDS2.dds" );
//
//	ddsLut.EmbedStrip(srcImage);
//	srcImage.Write("TestDds.bmp");
//
//
//	ddsLut.Apply(srcImage);
//	srcImage.Write("AppliedDDSLut.bmp");
//
//	//ddsLut.ReadDDS( g_pD3dDevice, outFileName );
//	//ddsLut.WriteDDS( g_pD3dDevice, "OutDDS2.dds" );
//	//ddsLut.EmbedStrip(srcImage);
//	//srcImage.Write("TestDDSEmbed.bmp");
//}
//
//
//void ConvertImage( Util::C_String& inFileName, Util::C_String& outFileName )
//{
//	LOGINFO( g_Log, "   Converting %s\n", inFileName.Chars());
//	
//	Attk::C_Image srcImage;
//	srcImage.Read( inFileName );
//
//	ConvertImage(srcImage, outFileName);
//	
//	//LOGINFO( g_Log, "   Done\n\n", outFileName.Chars());
//}
//
//void ProcessFile( Util::C_String& fileName )
//{
//	Util::C_String ext = Util::ExtractFileExtension(fileName).Lower();
//
//	if( (ext == "tga") || (ext == "bmp") || (ext == "pic") )
//	{
//		Sys::C_Fileinfo fileInfo;
//		Sys::C_FileSystem::Instance().FileInfo(fileName, &fileInfo);
//
//		if( !(fileInfo.m_FileAccess & Sys::E_FileAccess_Read) )
//		{
//			LOGINFO( g_Log, "   *** No read access for %s - FILE NOT CONVERTED\n", fileName.Chars());
//		
//		}
//		else
//		{
//			Util::C_String outputFilename;
//
//			Util::C_String directory = Util::ExtractDirectory(fileName);
//			Util::C_String filename = Util::RemoveFileExtension( Util::ExtractFilename(fileName) );
//			Util::C_String extension = Util::ExtractFileExtension(fileName);
//			outputFilename = directory + filename + ".dds";
//
//			ConvertImage( fileName, outputFilename );			
//		}
//	}
//}
//
//void ProcessDirectory( Util::C_String dirName )
//{
//	LOGINFO( g_Log, "Scanning '%s'\n", dirName.Chars() );
//
//	Sys::C_DirectoryInfo dirInfo;
//	bool result = Sys::C_FileSystem::Instance().DirectoryInfo( dirName, &dirInfo );
//
//	if(!result)
//	{
//		LOGINFO( g_Log, "Problem Reading Directory Info '%s'\n", dirName.Chars() );
//		return;
//	}
//
//	const U32 numFiles = dirInfo.NumFiles();
//
//	//LOGINFO( g_Log, "%d files\n", numFiles );
//
//	for(U32 fileIndex=0; fileIndex<numFiles; fileIndex++)
//	{
//		Sys::C_Fileinfo& fileInfo = dirInfo.FileInfo( fileIndex );
//
//		switch(fileInfo.m_FileType)
//		{
//			case Sys::E_FileType_File:
//			{
//				ProcessFile(fileInfo.m_FileName);
//			}
//			break;
//
//			case Sys::E_FileType_Directory:
//			{
//				ProcessDirectory(fileInfo.m_FileName);
//			}
//			break;
//
//			default:
//			break;
//		}
//	}
//
//	LOGINFO( g_Log,  "\n" );
//
//}


class C_Options
{
public:
	bool ParseCommandLine( int argc, char* argv[] );
	void Dump();

public:
	bool m_EmbedLut;
	bool m_ApplyLut;
	Util::C_String m_SrcImageFilename;
	Util::C_String m_SrcLutFilename;
	Util::C_String m_OutputFilename;
};

bool C_Options::ParseCommandLine( int argc, char* argv[] )
{
	Util::C_String executableFileName;

	//Get Executable FileName
	executableFileName = argv[0];

	//Define the command line object.
	TCLAP::CmdLine cmd("Lut Converter.", ' ', "1.00");

	// Wrap everything in a try block.  Do this every time, 
	// because exceptions will be thrown for problems. 
	try
	{  
		TCLAP::SwitchArg argApply("a","apply","Apply LUT to image",false);
		cmd.add(argApply);

		TCLAP::SwitchArg argEmbed("e","embed","Embed LUT into image",false);
		cmd.add(argEmbed);

		//std::vector<TCLAP::Arg*> xorArgList;
		//xorArgList.push_back(&argExtract);
		//xorArgList.push_back(&argEmbed);
		//xorArgList.push_back(&argApply);
		//cmd.xorAdd(xorArgList);

		TCLAP::ValueArg<std::string> argSrcImageFilename("i","image","Source Image",false,"","Source Image");
		cmd.add(argSrcImageFilename);

		TCLAP::ValueArg<std::string> argSrcLutFilename("l","lut","Source Lut",false,"","Source Lut");
		cmd.add(argSrcLutFilename);

		TCLAP::ValueArg<std::string> argOutputFilename("o","output","Output Filename",false,"","Output");
		cmd.add(argOutputFilename);

		//Parse the args.
		cmd.parse(argc, argv);

		m_ApplyLut = false;
		if(argApply.getValue() == true)
		{
			m_ApplyLut = true;
		}

		m_EmbedLut = false;
		if(argEmbed.getValue() == true)
		{
			m_EmbedLut = true;
		}		

		m_SrcImageFilename = argSrcImageFilename.getValue().c_str();
		m_SrcLutFilename = argSrcLutFilename.getValue().c_str();
		m_OutputFilename = argOutputFilename.getValue().c_str();
	} 
	catch (TCLAP::ArgException &e)  // catch any exceptions
	{ 
		LOGERROR( g_Log, "Error: %s %s", e.error().c_str(), e.argId().c_str() );
		return false;
	}

	return true;
}

void C_Options::Dump()
{
	LOGINFO( g_Log, "Options:\n");

	LOGINFO( g_Log, "Apply Lut: %d\n", m_ApplyLut);
	LOGINFO( g_Log, "Embed Lut: %d\n", m_EmbedLut);	

	LOGINFO( g_Log, "Source Image Filename: %s\n", m_SrcImageFilename.Chars());
	LOGINFO( g_Log, "Source Lut Filename: %s\n", m_SrcLutFilename.Chars());
	LOGINFO( g_Log, "Output Filename: %s\n", m_OutputFilename.Chars());
	LOGINFO( g_Log, "\n");
}


bool CreateDevice()
{
	// create d3d device
	g_pD3d = Direct3DCreate9( D3D_SDK_VERSION );

	if( g_pD3d == NULL )
	{
		LOGERROR( g_Log, "Failed to create d3d" );
		return false;
	}

	D3DPRESENT_PARAMETERS d3dpp; 

	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed   = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;

	HRESULT hr;
	
	HWND hWnd = GetConsoleWindow();

	hr = g_pD3d->CreateDevice( D3DADAPTER_DEFAULT, 
						D3DDEVTYPE_HAL, hWnd,
									D3DCREATE_SOFTWARE_VERTEXPROCESSING,
									&d3dpp, &g_pD3dDevice );

	if( FAILED(hr) )
	{
		LOGERROR( g_Log, "Failed to create d3d device" );
		return false;
	}

	return true;
}

bool IsDdsFile( Util::C_String& filename )
{
	Util::C_String extension = Util::ExtractFileExtension(filename).Lower();

	if(extension == "dds")
	{
		return true;
	}
	
	return false;
}

bool LoadLut(Lut::C_Lut& lut, C_Options& options)
{
	bool success = true;
	if(options.m_SrcLutFilename == "")
	{
		LOGINFO( g_Log, "No Source LUT, using identity");
		lut.SetIdentity();
	}
	else
	{
		if(IsDdsFile(options.m_SrcLutFilename))
		{
			LOGINFO( g_Log, "Reading LUT from DDS");
			success = lut.ReadDDS( g_pD3dDevice, options.m_SrcLutFilename );
			if(!success)
			{
				return false;
			}
		}
		else
		{
			LOGINFO( g_Log, "Reading LUT from image %s", options.m_SrcLutFilename.Chars());
			Attk::C_Image srcImage;
			success = srcImage.Read( options.m_SrcLutFilename );
			if(!success)
			{
				LOGERROR( g_Log, "Error reading image %s", options.m_SrcLutFilename.Chars());
				return false;
			}
			success = lut.ExtractStrip( srcImage );
			if(!success)
			{
				LOGERROR( g_Log, "Problem extracting strip");
				return false;
			}
		}
	}
	return true;
}

bool Process(C_Options& options)
{
	Lut::C_Lut lut;
	bool success = true;

	success = LoadLut(lut, options);
	if(!success)
	{
		LOGERROR( g_Log, "Problem loading LUT");
		return false;
	}

	Util::C_String outputFilename;
	if(options.m_OutputFilename.Length() <= 0)
	{
		if(options.m_SrcLutFilename.Length() <= 0)
		{
			outputFilename = "IdentityLut.dds";
		}
		else
		{
			// derive output filename from input
			Util::C_String directory = Util::ExtractDirectory(options.m_SrcLutFilename);
			Util::C_String filename = Util::RemoveFileExtension( Util::ExtractFilename(options.m_SrcLutFilename) );
			Util::C_String extension = Util::ExtractFileExtension(options.m_SrcLutFilename);
			if(IsDdsFile( options.m_SrcLutFilename ) )
			{
				outputFilename = directory + filename + ".bmp";
			}
			else
			{
				outputFilename = directory + filename + ".dds";
			}
		}
	}
	else
	{
		outputFilename = options.m_OutputFilename;
	}

	if(IsDdsFile( outputFilename ) )
	{
		LOGINFO( g_Log, "Writing output: %s", outputFilename.Chars());
		success = lut.WriteDDS( g_pD3dDevice, outputFilename );
		if(!success)
		{
			LOGERROR( g_Log, "Problem writing dds %s", outputFilename.Chars() );
			return false;
		}
	}
	else
	{
		Attk::C_Image image;
		if(options.m_SrcImageFilename.Length() > 0)
		{
			// read image and embed strip
			success = image.Read(options.m_SrcImageFilename);
			if(!success)
			{
				LOGERROR( g_Log, "Problem reading source image %s", options.m_SrcImageFilename.Chars() );
				return false;
			}

			if(options.m_ApplyLut)
			{
				lut.Apply( image );
			}

			if(options.m_EmbedLut)
			{
				lut.EmbedStrip(image);
			}
		}
		else
		{
			// just write strip into ouput
			success = lut.MakeStrip(image);
			if(!success)
			{
				LOGERROR( g_Log, "Problem creating image strip" );
				return false;
			}
		}
		
		LOGINFO( g_Log, "Writing output %s", outputFilename.Chars() );
		success = image.Write(outputFilename.Chars());
		if(!success)
		{
			LOGERROR( g_Log, "Problem writing image %s", outputFilename.Chars() );
			return false;
		}
	}


	return true;
}

/*
bool ApplyLut(C_Options& options)
{
	Lut::C_Lut lut;
	bool success = true;

	success = LoadLut(lut, options);
	if(!success)
	{
		LOGERROR( g_Log, "Problem loading LUT");
		return false;
	}

	Attk::C_Image image;
	if(options.m_SrcImageFilename.Length() <= 0)
	{
		LOGERROR( g_Log, "Source image required for Apply");
		return false;
	}

	// read image and embed strip
	success = image.Read(options.m_SrcImageFilename);
	if(!success)
	{
		LOGERROR( g_Log, "Problem reading source image %s", options.m_SrcImageFilename.Chars() );
		return false;
	}

	lut.Apply( image );

	Util::C_String outputFilename;
	if(options.m_OutputFilename.Length() <= 0)
	{
		Util::C_String directory = Util::ExtractDirectory(options.m_SrcImageFilename);
		Util::C_String filename = Util::RemoveFileExtension( Util::ExtractFilename(options.m_SrcImageFilename) );
		Util::C_String extension = Util::ExtractFileExtension(options.m_SrcImageFilename);

		outputFilename = directory + filename + "_Graded" + "." + extension;
	}
	else
	{
		outputFilename = options.m_OutputFilename;		
	}
		
	LOGINFO( g_Log, "Writing output %s", outputFilename.Chars() );
	success = image.Write(outputFilename.Chars());
	if(!success)
	{
		LOGERROR( g_Log, "Problem writing image %s", outputFilename.Chars() );
		return false;
	}

	return true;
}
*/

int main(int argc, char* argv[])
//int _tmain(int argc, _TCHAR* argv[])
{
	Debug::AutoConfigureDebugLog();
	//if(argc != 2)
	//{
	//	LOGINFO( g_Log, "%s\nRecursively Converts _IPR0 textures to NewTech\n\n", argv[0]);
	//	LOGINFO( g_Log, "Usage:\n%s TexturePath\n\n", argv[0]);

	//	getch();
	//	return 0;

	//}

	bool result;
	result = CreateDevice();
	if(!result)
	{
		LOGERROR( g_Log, "Problem creating d3d device" );
		return 0;
	}
	
	C_Options options;

	result = options.ParseCommandLine(argc, argv);
	if(result == FALSE)
	{
		LOGERROR( g_Log, "Problem parsing command line" );
		return 0;
	}

	options.Dump();

	result = Process(options);

	/*
	switch(options.m_Mode)
	{
		case E_Mode_Extract:
		break;
		case E_Mode_Apply:
			result = ApplyLut(options);
		break;		
	}
	*/


	//LOGINFO( g_Log, "Press any key...\n");

	//getch();

	SAFE_RELEASE(g_pD3dDevice);

	return 0;
}