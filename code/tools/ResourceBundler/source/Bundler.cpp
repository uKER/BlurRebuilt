#include "stdafx.h"

#include <system/misc/StringUtil.h>
#include <display/bundler/BundleHeader.h>

#include "BundlerOutputOptions.h"
#include "Bundler.h"
#include "RDFParse.h"


#include <export_resource/ExportTextureOptions.h>
#include <export_resource/OutputResourceHeader.h>
#include <export_resource_header/IExportResourceHeader.h>
#include <tools_shared/versionstrings.inl>
#include <tools_shared/Argument/Syntax.h>
#include <tools_shared/Argument/ArgumentDataBase.h>

#include <zlib.h>


using namespace ResourceBundler;

C_Bundler::C_Bundler(
	const char *szName,			// the name of the application
	const char *szSubtitle,		// subtitle e.g. "bizarre creations lighting tool"
	const char *szLogger,		// name of the base logger for this application
	const char *szDmpPath,		// path where .dmp files are to be written out to
	bool bExecuter)				// true if the application needs to send executer feedback
	:
Tools::C_ToolsApplication(
	szName,
	szSubtitle,
	szLogger,
	szDmpPath,
	false)
{
	//Allocate a small area of reserved storage at the start of our graphics memory block
	//just in case anyone want's to ninja-in some data there.
	m_pRawData				= (U8*)malloc(RESERVED_DATA_SIZE);
	m_rawDataSize			= 0;
	m_dataReserveSize		= 256;
	memset(&m_outputOptions, 0, sizeof(m_outputOptions));
}

C_Bundler::~C_Bundler()
{
	for (int iCount=0; iCount < m_Resources.Size(); iCount++)
	{
		delete m_Resources[iCount];
	}

	if (m_pRawData)
		free(m_pRawData);
}

void C_Bundler::_ShowUsage()
{
	printf("Copyright (C) Bizarre Creations 2006.\n\n");
	printf("Usage:\n\n");
	printf("  BizarreBundler resource_file [-f <x360 | pc | psgl | psgcm | wii>] [-c] [-o <packed resource filename>]\n");
	printf("  e.g. BizarreBundler resources.xml -f x360 -c -o resource_bundle.bpr\n\n");
	printf("  -c     Compress output\n");
	printf("  -f     Target platform\n");
	printf("                x360  = XBox 360\n");
	printf("                pc    = PC\n");
	printf("                ps3   = PS3 (LibGCM)\n");
	printf("                wii   = Wii\n");
	printf("  -o     Output file location (else put in folder with resource file)\n");
	printf("\n");
}

bool C_Bundler::_ParseArguments(int argc, char *argv[])
{
	Btl::String args;
	for (S32 i = 1; i < argc; ++i)
	{
		args += "\"";
		args += argv[i];
		args += "\"";
	}

	Tools::C_Syntax* pSyntax = new Tools::C_Syntax;
	bool status = true;

	{
		Tools::C_Argument arg(
			'c', 
			"compress",
			"compress output",
		Tools::C_Argument::E_Required_False);
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'u', 
			"uncompress",
			"uncompress output",
		Tools::C_Argument::E_Required_False);
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'n', 
			"individualcompress",
			"compress outputs individually",
		Tools::C_Argument::E_Required_False);
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'f', 
			"target",
			"target platform: \n\
	x360  = XBox 360\n\
	pc    = PC\n\
	ps3   = PS3 (LibGCM)\n\
	wii   = Wii\n",
			Tools::C_Argument::E_Required_True);
		arg.GetDataTypes().PushBack(Util::E_VarType_String);
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'o', 
			"output",
			"output file",
			Tools::C_Argument::E_Required_True);
		arg.GetDataTypes().PushBack(Util::E_VarType_String);
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'i', 
			"input",
			"files to convert",
			Tools::C_Argument::E_Required_True);
		arg.GetDataTypes().PushBack(Tools::C_ArgType(Util::E_VarType_String, true));
		pSyntax->AddArg(arg);
	}

	Tools::C_ArgumentDatabase argDb;
	Tools::C_ArgumentParser parser(pSyntax);
	if (!parser.Parse(argDb, args))
	{
		printf("Failed to parse arguments!");
		return false;
	}

	{
		const Tools::ArgDataArray* pArgData = argDb.GetArgData('i');
		if (pArgData)
		{
			for (S32 i = 0; i < pArgData->Size(); ++i)
			{
				Btl::String str;
				(*pArgData)[i].GetValue(str);
				m_Inputs.PushBack(str);
			}
		}	
	}

	{
		const Tools::ArgDataArray* pArgData = argDb.GetArgData('o');
		BIZ_ASSERT(pArgData);
		Btl::String str;
		if (pArgData->Size())
			pArgData->Back().GetValue(str);
		m_OutputFileName = str;
	}

	{
		int compressionFlags = 0;

		if (argDb.IsSet('c'))
		{
			m_outputOptions.dataCompressionType = Display::ResourceBundler::E_HeaderFlag_Compressed;
			compressionFlags++;
		}

		if (argDb.IsSet('u'))
		{		
			m_outputOptions.dataCompressionType = Display::ResourceBundler::E_HeaderFlag_Uncompressed;
			compressionFlags++;
		}

		if (argDb.IsSet('n'))
		{
			m_outputOptions.dataCompressionType = Display::ResourceBundler::E_HeaderFlag_CompressedIndividually;
			compressionFlags++;
		}

		if (compressionFlags == 0)
		{
			m_outputOptions.dataCompressionType = Display::ResourceBundler::E_HeaderFlag_Uncompressed;
		}
		else if (compressionFlags > 1)
		{
			printf("Compression options are mutually exclusive!\n");
			return false;
		}
	}

	{
		const Tools::ArgDataArray* pArgData = argDb.GetArgData('f');
		BIZ_ASSERT(pArgData);
		Btl::String str;
		if (pArgData->Size())
			pArgData->Back().GetValue(str);
		
		if (str == "x360")
			_InitialiseOutputOptions(Display::ResourceBundler::E_Platform_Xbox360);
		else if (str == "pc")
			_InitialiseOutputOptions(Display::ResourceBundler::E_Platform_PC);
		else if (str == "psgl")
			_InitialiseOutputOptions(Display::ResourceBundler::E_Platform_PS3_OpenGL);
		else if (str == "libgcm")
			_InitialiseOutputOptions(Display::ResourceBundler::E_Platform_PS3_LibGCM);
		else if (str == "ps3")
			_InitialiseOutputOptions(Display::ResourceBundler::E_Platform_PS3_LibGCM);
		else if (str == "wii")
			_InitialiseOutputOptions(Display::ResourceBundler::E_Platform_Wii);
		else
		{
			printf("Unknown platform!\n");
			return false;
		}
	}

	printf("Bundling for platform: %s\n", Display::ResourceBundler::g_platformName[m_outputOptions.platform]);
	printf("\tZLib compression: %s\n", m_outputOptions.dataCompressionType ? "true" : "false");
	printf("\tFile Endianness: %s\n", m_outputOptions.bigEndianFile ? "big endian" : "little endian");
	printf("\tTexture Endianness: %s\n", m_outputOptions.bigEndianTexture ? "big endian" : "little endian");
	printf("\tTile textures: %s\n", m_outputOptions.tileTextures ? "true" : "false");

	if (m_outputOptions.dataCompressionType == Display::ResourceBundler::E_HeaderFlag_CompressedIndividually)
		m_outputOptions.resourceAlignment = 1;
	printf("\tTexture Alignment: %i byte(s)\n\n", m_outputOptions.resourceAlignment);

	return true;
}

int C_Bundler::Main(int argc, char *argv[], Progress::I_ProgressBar *pProgress)
{
	printf("\n");

	if (!_ParseArguments(argc, argv))
	{
		_ShowUsage();
		return 1;
	}

	C_RDFParser rdf;

	if (!rdf.LoadRDF(this))
		return 1;

	if (!_WriteXPRFile())
		return 1;

	return 0;
}

void C_Bundler::_InitialiseOutputOptions(Display::ResourceBundler::E_Platform platform)
{
	m_outputOptions.platform = platform;

	switch (platform)
	{
	case Display::ResourceBundler::E_Platform_Xbox360:
		m_outputOptions.bigEndianFile		= true;
		m_outputOptions.bigEndianTexture	= true;
		m_outputOptions.tileTextures		= true;
		m_outputOptions.resourceAlignment	= 1;
		break;

	case Display::ResourceBundler::E_Platform_PS3_OpenGL:
	case Display::ResourceBundler::E_Platform_PS3_LibGCM:
		m_outputOptions.bigEndianFile		= true;
		m_outputOptions.bigEndianTexture	= false;
		m_outputOptions.tileTextures		= false;
		m_outputOptions.resourceAlignment	= 128;
		break;

	case Display::ResourceBundler::E_Platform_Wii:
		m_outputOptions.bigEndianFile		= true;
		m_outputOptions.bigEndianTexture	= false;// Only "small" because we do the endian
													// conversion ourselves later in the pipeline.	
		m_outputOptions.tileTextures		= true;
		m_outputOptions.resourceAlignment	= 32;
		break;

	case Display::ResourceBundler::E_Platform_PC:
		m_outputOptions.bigEndianFile		= false;
		m_outputOptions.bigEndianTexture	= false;
		m_outputOptions.tileTextures		= false;
		m_outputOptions.resourceAlignment	= 1;
		break;
	
	default:
		m_outputOptions.bigEndianTexture	= false;
		m_outputOptions.tileTextures		= false;
		m_outputOptions.resourceAlignment	= 1;
		assert(false);
	};
}

bool C_Bundler::GetPath(Btl::String& path) const
{
	Sys::I_File *pFile = Sys::GetDefaultFileSystem()->Open(path);
	if (pFile == NULL)
	{
		Btl::String fileName = m_BasePath + path;
		pFile = Sys::GetDefaultFileSystem()->Open(fileName);
		if (pFile == NULL)
			return false;

		path = fileName;
	}

	delete pFile;

	return true;
}

const C_BundlerOutputOptions& C_Bundler::GetOutputOptions() const
{
	return m_outputOptions;
}

void C_Bundler::AddResource(I_ExportResourceHeader * resource)
{
	m_Resources.PushBack(resource);
}

U32 C_Bundler::GetDataOffset() const
{
	return m_rawDataSize;
}

U32 C_Bundler::_EstimateCompressedDataSize(	const U32 inputDataSize) const
{
	//calculate size of buffer required: (dataSizeInBytes*0.001)(+1 for rounding)+4
	return 4+(U32)((F32)inputDataSize * 0.001f) + 5 + inputDataSize;
}

bool C_Bundler::_CompressData(
	U8*& pCompressedData,
	U32& compressedDataSize,
	const U8* pInputData,
	const U32 inputDataSize) const
{
	unsigned long destLen = compressedDataSize;

	//compress existing data into new buffer
	if (compress(pCompressedData, &destLen, pInputData, inputDataSize) != Z_OK)
	{
		fprintf(stderr, "BUNDLER: error : Couldn't compress data");
		return false;
	}

	compressedDataSize = destLen;
	return true;
}

bool C_Bundler::_WriteXPRFile()
{
	U8* pOutputData;
	U8* pCompressedOutputData;
	U32 outputDataSize;

	U32 uncompressedDataSize = m_rawDataSize;
	U32 compressedDataSize;
	
	switch (m_outputOptions.dataCompressionType)
	{
		case Display::ResourceBundler::E_HeaderFlag_CompressedIndividually:
		case Display::ResourceBundler::E_HeaderFlag_Uncompressed:
		{
			pOutputData = m_pRawData;
			outputDataSize = uncompressedDataSize;
			pCompressedOutputData = 0;
			compressedDataSize = 0;
			break;
		}

		case Display::ResourceBundler::E_HeaderFlag_Compressed:
		{
			compressedDataSize = _EstimateCompressedDataSize(uncompressedDataSize);
			pCompressedOutputData = (U8*)malloc(compressedDataSize);
			if (!_CompressData(
				pCompressedOutputData,
				compressedDataSize,
				m_pRawData,
				uncompressedDataSize))
				return false;

			pOutputData = pCompressedOutputData;
			outputDataSize = compressedDataSize;
			break;
		}

		default:
			return false;
	};

	//Pad the data buffer out to be a multiple of the DVD sector size (2k).
	//This is needed in order to DMA the file off the DVD drive.
	//TODO: Media specified padding should be performed the as-yet uncompleted
	//Bizarre Pak file system.
	_PadToAlignment(
		pOutputData,
		outputDataSize,
		2048);

	{ // Create out output directory.
		Sys::C_FilePath path = Sys::ExtractDirectory(m_OutputFileName);
		if (!Sys::GetDefaultFileSystem()->FileExists(path))
			Sys::GetDefaultFileSystem()->CreateDirectory(path);
	}
	// Create the XPR file for writing
	Sys::I_File *pFile = Sys::C_FileSystem::Instance().Open(
		m_OutputFileName,
		Sys::E_FileAccess_Write);
	if (!pFile)
	{
		printf("Error: Unable to open XPR file '%s'\n", m_OutputFileName.Chars());
		return false;
	}
	pFile->SetEndianSwapFlag(m_outputOptions.bigEndianFile);

	//Create our serialiser object.
	Sys::BinarySerialiserAutoEndian ar(*pFile);

	//Build and write our bundle header.
	{
		Display::ResourceBundler::C_BundleHeader bundleHeader;
		bundleHeader.magicID		= m_magicNumber;
		bundleHeader.version		= 3;
		bundleHeader.platformCode	= m_outputOptions.platform;
		bundleHeader.flags			= 0;
		bundleHeader.flags |= m_outputOptions.dataCompressionType;
		bundleHeader.resourceInfoSize = 0;
		// TODO: Fix up to correct platform

		U32 offsetToStringTable(0);

		// Run through the resources and work out how much space we need to take up
		for (S32 iCount=0; iCount < m_Resources.Size(); iCount++)
		{
			// Calculate the size of the resource data we need to store
			U32 resourceSize = m_Resources[iCount]->CalculateHeaderSize();

			bundleHeader.resourceInfoSize	+= U16(resourceSize);
			offsetToStringTable				+= resourceSize;

			// Add string space and trailing zero's
			bundleHeader.resourceInfoSize += m_Resources[iCount]->GetName().Length()+1;
		}

		bundleHeader.stringTableOffset		= U16(offsetToStringTable);
		bundleHeader.uncompressedDataSize	= uncompressedDataSize;
		bundleHeader.compressedDataSize		= compressedDataSize;

		//Write out our bundle header.
		ar << bundleHeader;
	}

	U32 gfxDataSize = 0;

	// Write out resource blocks
	U32 offsetToString = 0;
	for (S32 i=0; i < m_Resources.Size(); i++)
	{
		const I_ExportResourceHeader& exportResource = *(m_Resources[i]);
		
		// Offset to next resource - or Zero for last
		const U32 resourceSize = exportResource.CalculateHeaderSize();

		U16 offsetToNextResource;
		if (i != m_Resources.Size()-1) 
			offsetToNextResource = U16(resourceSize);
		else 
			offsetToNextResource = 0;

		// Offset to string from start of this resource
		U16 resourceStringOffset = U16(offsetToString);
		// We reference the string from the start of each block
		offsetToString += exportResource.GetName().Length()+1;

		// Offset to gfx data from base of data
		U32 offsetToResourceData = exportResource.GetOffset();

		//Write out our resource header.
		exportResource.Serialise(
			ar,
			offsetToNextResource,
			resourceStringOffset,
			offsetToResourceData);

		gfxDataSize += exportResource.GetUncompressedDataSize();
	}

	printf("\nTotal gfx data size (%d bytes - %.1fk)\n", gfxDataSize, float(gfxDataSize) / 1024.f);

	//Write out our string table! :D
	Sys::C_StreamPos pos = pFile->GetPosition();
	for (S32 i=0; i < m_Resources.Size(); i++)
	{
		const I_ExportResourceHeader& resource = *(m_Resources[i]);

		if (!pFile->Write(resource.GetName().Chars(), resource.GetName().Length()+1))
			return false;
	}

	//Write out our platform specific data chunk.
	if (!pFile->Write(pOutputData, outputDataSize))
		return false;

	if (pCompressedOutputData)
		free(pCompressedOutputData);

	delete pFile;

	return true;
}

bool C_Bundler::WriteData(
	U32& compressedSize,
	const U8 *_pData, 
	U32 inputSize)
{
	U8* pData = const_cast<U8*>(_pData);
	if (inputSize % m_outputOptions.resourceAlignment)
	{
		if (!_GetAlignedCopy(
			&pData,
			_pData,
			inputSize,
			m_outputOptions.resourceAlignment))
			return false;
	}

	const U8* pDataToWrite;
	U32 dataSizeToWrite;
	U8* pCompressedData;
	if (m_outputOptions.dataCompressionType == Display::ResourceBundler::E_HeaderFlag_CompressedIndividually)
	{
		//Compress an individual resource.
		dataSizeToWrite = _EstimateCompressedDataSize(inputSize);

		//allocate buffer for compressed data
		pCompressedData = (U8*)malloc(dataSizeToWrite);
		if (!pCompressedData)
			return false;

		if (!_CompressData(
			pCompressedData,
			dataSizeToWrite, //This will be modified by _CompressData to be the true storage required.
			pData,
			inputSize))
			return false;

		pDataToWrite = pCompressedData;
		compressedSize = dataSizeToWrite;
		//We'll have to free pCompressedData later.
	}
	else
	{
		//No compression here.
		pDataToWrite = pData;
		dataSizeToWrite = inputSize;
		pCompressedData = 0;
		compressedSize = 0;
	}

	assert(pData);
	// Realloc if necessary
	if (m_rawDataSize + dataSizeToWrite > m_dataReserveSize)
	{
		// Find the next power-of-2 size that is big enough to hold the data
		while (m_rawDataSize + dataSizeToWrite > m_dataReserveSize)
			m_dataReserveSize *= 2;

		m_pRawData = (U8*) realloc(m_pRawData, m_dataReserveSize);
		if (!m_pRawData)
		{
			printf("Error: Out of memory\n");
			return false;
		}
	}

	// Copy over the data
	memcpy(m_pRawData + m_rawDataSize, pDataToWrite, dataSizeToWrite);
	m_rawDataSize += dataSizeToWrite;
	
	//Free any temporary memory that we've allocated.
	if (pCompressedData)
		free(pCompressedData);

	//Free aligned buffer, if any.
	if (pData != _pData)
		free(pData);

	return true;
}

bool C_Bundler::_WriteData(
	U8* pOutputData, 
	U32& outputSize,
	const U8* pInputData, 
	U32 inputSize) const
{
	assert(pInputData);
	// Realloc if necessary
	if (outputSize + inputSize > m_dataReserveSize)
	{
		// Find the next power-of-2 size that is big enough to hold the data
		while (m_rawDataSize + outputSize > outputSize)
			outputSize *= 2;

		pOutputData = (U8*) realloc(pOutputData, outputSize);
		if (!pOutputData)
		{
			printf("Error: Out of memory\n");
			return false;
		}
	}

	// Copy over the data
	memcpy(pOutputData + m_rawDataSize, pInputData, inputSize);
	outputSize += inputSize;

	return true;
}

void C_Bundler::_PadToAlignment(
	U8* pOutputData, 
	U32& outputSize,
	U32 align)
{
	if (outputSize % align)
	{
		U32 bytesToFill = align - (outputSize % align);

		if (bytesToFill)
		{
			pOutputData = (U8*)realloc(pOutputData, outputSize + bytesToFill);
			memset(pOutputData+outputSize, 0, bytesToFill);
			outputSize = outputSize + bytesToFill;
		}
	}
}

bool C_Bundler::_GetAlignedCopy(
	U8** ppOutputData,
	const U8* pInputData, 
	U32& outputSize,
	U32 align)
{
	if (outputSize % align)
	{
		U32 bytesToFill = align - (outputSize % align);

		if (bytesToFill)
		{
			*ppOutputData = (U8*)malloc(outputSize + bytesToFill);
			memcpy(*ppOutputData, pInputData, outputSize);
			memset(*ppOutputData+outputSize, 0, bytesToFill);
			outputSize = outputSize + bytesToFill;
		}
		return true;
	}

	assert(!"Already aligned!");
	*ppOutputData = 0;
	return false;
}