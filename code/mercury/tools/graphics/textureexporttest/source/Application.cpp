#include "stdafx.h"
#include <system/xml/XmlDom.h>

#include <tools_shared/shim/Shim.h>
#include <tools_shared/ToolsApplication.h>
#include <tools_shared/VersionStrings.inl>
#include <texture_export/texture.h>
#include <texture_export/targets/pctex.h>
#include <texture_export/targets/ps3tex.h>
#include <texture_export/targets/xetex.h>
#include <texture_export/converters/generatemipmaps.h>
#include <texture_export/converters/atidxt.h>
#include <texture_export/converters/hdrcompression.h>
#include <texture_export/converters/heightmaptonormalmap.h>
#include <texture_export/converters/normalisecolour.h>
#include <attk/attk.h>
#include <system/perf/profiler.h>
#include <system/perf/profilerbackend.h>
#include <core/threads.h>
#include <system/memory/memorytracker.h>

#include <texture_export/texturemanager.h>
#include <texture_export/textureprocessing.h>
#include <texture_export/utility.h>
#include <misc_shared/cache/cacheentry.h>
#include "cachetest.h"
//#include <texture_export/targets/pcfx.h>
//#include <texture_export/targets/xefx.h>
#include <system/serialise.h>
#include <system/serialise/binaryserialise.h>

#define NGL_GPU_D3D_PC 1

#include "ngl/mercury_to_ngl.h"
#include "ngl/defs.h"
#include "ngl/ngl_meshdefs.h"
//#include "texture_export/ngl/ngl_fx.h"

#include "Application.h"

C_Application::C_Application(
	const char *szName,
	const char *szSubtitle,
	const char *szLogger,
	const char *szDmpPath,
	bool bExecuter)
	:
	Tools::C_ToolsApplication(
		szName,
		szSubtitle,
		szLogger,
		szDmpPath,
		bExecuter)
{
}

C_Application::~C_Application()
{
}

//#define WRITE_OUT_PS3_FORMAT
#define WRITE_OUT_XENON_FORMAT
//#define WRITE_OUT_PC_FORMAT

S32 C_Application::Main(S32 argc, char **argv, Progress::I_ProgressBar *pProgress)
{
	C_Options options;
	bool success = true;

//	Memory::Tracker::Instance().DumpStatsOnExit(false);
	Memory::Tracker::Instance().DumpBlocksOnExit(true);

	// Parse our command-line options.
	success = ParseCommandLineOptions(argc, argv, options);
	if (!success)
		return 0;

	// Load the input file using the ATTK
	Attk::C_Image attkImage;
	attkImage.Read(options.inFile);

	// convert from Attk to export_texture format
	Attk::E_PixelFormat attkFormat = attkImage.GetPixelFormat();

	ActTools::Texture *pTexture = 0;
	ActTools::Texture *pTexture2 = 0;
	if (attkFormat == Attk::E_PixelFormat_A32R32G32B32F ||
		attkFormat == Attk::E_PixelFormat_B32G32R32A32F ||
		attkFormat == Attk::E_PixelFormat_B32G32R32F ||
		attkFormat == Attk::E_PixelFormat_R32G32B32A32F ||
		attkFormat == Attk::E_PixelFormat_R32G32B32F)
	{
		pTexture = TextureExport::TextureFromAttkImageF(&attkImage);
	}
	else
	{
		pTexture = TextureExport::TextureFromAttkImage(&attkImage);
	}
	
	// test flags for conversions
	if (options.flag == "height")
	{
		Texutils::HeightMapToNormalMap(pTexture);
	}
	else if (options.flag == "normalise")
	{
		Texutils::NormaliseColour(pTexture);
	}

	// perform the conversion
	if (options.format == "dxt1")
	{
		ActTools::TexelFormat *p = AtiDXT::GetTexelFormat("DXT1");
		pTexture->ConvertTo(p);
		delete p;
	}
	else if (options.format == "dxt3")
	{
		ActTools::TexelFormat *p = AtiDXT::GetTexelFormat("DXT3");
		pTexture->ConvertTo(p);
		delete p;
	}
	else if (options.format == "dxt5")
	{
		ActTools::TexelFormat *p = AtiDXT::GetTexelFormat("DXT5");
		pTexture->ConvertTo(p);
		delete p;
	}
	else if (options.format == "hdr")
	{
		NglXeTex texWriter;
		ActTools::TexelFormat *pFormat = texWriter.GetTexelFormat("A16B16G16R16F");
		pTexture->ConvertTo(pFormat);
		delete pFormat;
	}
	else if (options.format == "hdr5")
	{
		options.outFile2	= Util::RemoveFileExtension(options.outFile) + "_HDR." + Util::ExtractFileExtension(options.outFile);
		options.outFile		= Util::RemoveFileExtension(options.outFile) + "_LDR." + Util::ExtractFileExtension(options.outFile);

		ActTools::Texture* pTextureTemp = pTexture;

		Texutils::HDRCompress5BPP(pTextureTemp, pTexture, pTexture2);

		delete pTextureTemp;

		ActTools::TexelFormat *p = AtiDXT::GetTexelFormat("DXT1");
		pTexture->ConvertTo(p);
		delete p;
	}
	else if (options.format == "hdr5b")
	{
		options.outFile2	= Util::RemoveFileExtension(options.outFile) + "_HDR." + Util::ExtractFileExtension(options.outFile);
		options.outFile		= Util::RemoveFileExtension(options.outFile) + "_LDR." + Util::ExtractFileExtension(options.outFile);

		ActTools::Texture* pTextureTemp = pTexture;

		Texutils::HDRCompress5BPPBlur(pTextureTemp, pTexture, pTexture2);

		delete pTextureTemp;

		ActTools::TexelFormat *p = AtiDXT::GetTexelFormat("DXT1");
		pTexture->ConvertTo(p);
		delete p;
	}
	else if (options.format == "hdr8")
	{
		options.outFile2	= Util::RemoveFileExtension(options.outFile) + "_HDR." + Util::ExtractFileExtension(options.outFile);
		options.outFile		= Util::RemoveFileExtension(options.outFile) + "_LDR." + Util::ExtractFileExtension(options.outFile);

		ActTools::Texture* pTextureTemp = pTexture;

		Texutils::HDRCompress8BPP(pTextureTemp, pTexture, pTexture2);

		delete pTextureTemp;

		ActTools::TexelFormat *p = AtiDXT::GetTexelFormat("DXT1");
		pTexture->ConvertTo(p);
		delete p;
	}
	else
	{
		NglXeTex t;
		ActTools::TexelFormat *p = t.GetTexelFormat("A8R8G8B8");
		pTexture->ConvertTo(p);
		delete p;
	}

#ifdef WRITE_OUT_PS3_FORMAT
	// write out using texture export plugin
	Sys::I_File *pFile = Sys::C_FileSystem::Instance().Open(options.outFile, Sys::E_FileAccess_Write );
	NglPs3Tex texWriter;
	texWriter.SerialOutTexture(pFile, pTexture);
	delete pFile;
#endif
#ifdef WRITE_OUT_PC_FORMAT
	// write out using texture export plugin
	Sys::I_File *pFile = Sys::C_FileSystem::Instance().Open(options.outFile, Sys::E_FileAccess_Write );
	NglPcTex texWriter;
	texWriter.SerialOutTexture(pFile, pTexture);
#endif
#ifdef WRITE_OUT_XENON_FORMAT
	// write out using texture export plugin
	Sys::I_File *pFile = Sys::C_FileSystem::Instance().Open(options.outFile, Sys::E_FileAccess_Write );
	NglXeTex texWriter;
	texWriter.SerialOutTexture(pFile, pTexture);
	delete pFile;

	if (pTexture2)
	{
		Sys::I_File *pFile = Sys::C_FileSystem::Instance().Open(options.outFile2, Sys::E_FileAccess_Write );
		NglXeTex texWriter;
		texWriter.SerialOutTexture(pFile, pTexture2);
		delete pFile;
	}
#endif

	delete pTexture;
	
	return 1;
}

bool C_Application::ParseCommandLineOptions(
	int argc, 
	char* argv[], 
	C_Options& options)
{
	// Wrap everything in a try block.  Do this every time, 
	// because exceptions will be thrown for problems. 
	try
	{  
		//Define the command line object.
		TCLAP::CmdLine cmd("Example Command-line Tool.", ' ', "1.00");

		//Define the input and output files
		TCLAP::UnlabeledValueArg<std::string> inFileArg("inname",   "Input image filename (.bmp, .png, etc.)", "", "input file", cmd );
		TCLAP::UnlabeledValueArg<std::string> outFileArg("outname", "Output binary image", "", "output file", cmd );

		//Define the options
		TCLAP::ValueArg<std::string> flagArg("c", "flags", "Texture converter options", false, "", "string", cmd);
		TCLAP::ValueArg<std::string> formatArg("f", "format", "Texture output format", false, "", "string", cmd);

		//Parse the args.
		cmd.parse(argc, argv);

		//Get the values
		options.inFile = inFileArg.getValue().c_str();
		options.outFile = outFileArg.getValue().c_str();
		options.format = formatArg.getValue().c_str();
		options.flag = flagArg.getValue().c_str();
		
		options.format.ConvertToLower();
		options.flag.ConvertToLower();

		options.outFile2 = Util::RemoveFileExtension(options.outFile) + "_2." + Util::ExtractFileExtension(options.outFile);
	} 
	catch (TCLAP::CmdLineParseException &e)
	{
		std::cerr << "Failed to parse arguments: " << e.argId() << std::endl;
		return false;
	}
	catch (TCLAP::ArgException &e)  // catch any exceptions
	{ 
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
		return false;
	}

	return true;
}
