#include "stdafx.h"
#include <system/serialise.h>

#include <display/bundler/ResourceBundleReader.h>
#include <display/bundler/BundleHeader.h>
#include <display/bundler/ResourceHeader.h>
#include <display/bundler/ResourceTexture.h>
#include <display/bundler/ResourceTexture2D.h>
#include <display/bundler/ResourceTextureCubeMap.h>
#include <display/bundler/ResourceTextureArray2D.h>
#include <display/bundler/ResourceTexture3D.h>
#include <Display/Device/Device.h>
#include <Display/Texture/Texture.h>
#include <Display/Texture/Texture2D.h>
#include <Display/Texture/Texture3D.h>
#include <Display/Texture/TextureCube.h>
#include <Display/Bundler/I_ResourceFactory.h>
#include <Display/Bundler/I_ResourceBundle.h>
#include <Display/Bundler/I_ResourceBundleReaderImpl.h>
#include <Display/Bundler/ResourceBundleReaderImpl1.h>
#include <Display/Bundler/ResourceBundleReaderImpl2.h>
#include <Display/Bundler/ResourceBundleReaderImpl3.h>


using namespace Display::ResourceBundler;

bool C_ResourceBundleReader::Read(
	U32& sysMemorySize,
	U8*& pSysMemory,
	U32& vidMemorySize,
	U8*& pVidMemory,
	I_ResourceBundle& bundle,
	const Sys::C_FilePath& fileName)
{
	pSysMemory = NULL;
	pVidMemory = NULL;
	sysMemorySize = 0;
	vidMemorySize = 0;

	Sys::I_File* pFile = Display::gFileSystem->Open(fileName,Sys::E_FileAccess_Read);
	if (!pFile)
	{
		LOGWARN(g_Log, "Failed to open file '%s'\n", fileName.Chars());
		return false;
	}

	bool success = Read(sysMemorySize, pSysMemory, vidMemorySize, pVidMemory, bundle, *pFile);

	delete pFile;

	return success;
}

bool C_ResourceBundleReader::Read(
	U32& sysMemorySize,
	U8*& pSysMemory,
	U32& vidMemorySize,
	U8*& pVidMemory,
	I_ResourceBundle& bundle,
	Sys::I_File& file)
{
	pSysMemory = NULL;
	pVidMemory = NULL;
	sysMemorySize = 0;
	vidMemorySize = 0;

	U32 magicNumber;
	if (!file.Read(&magicNumber, sizeof(U32)))
		return false;

	//Examine our magic number to determine how we process this file.
	if (magicNumber == GetHeader("BZR1"))
	{
		C_ResourceBundleReaderImpl1 impl;
		if (!((I_ResourceBundleReaderImpl&)impl).Read(
			1,
			sysMemorySize,
			pSysMemory,
			vidMemorySize,
			pVidMemory,
			bundle,
			file))
			return false;
	}
	else if (magicNumber == GetHeader("BZR2"))
	{
		U8 version;
		file.Read(&version, sizeof(version));

		switch(version)
		{
			case 2:
			{
				C_ResourceBundleReaderImpl2 impl;
				if (!((I_ResourceBundleReaderImpl&)impl).Read(
					version,
					sysMemorySize,
					pSysMemory,
					vidMemorySize,
					pVidMemory,
					bundle,
					file))
					return false;
				break;
			}
			case 3:
			{
				C_ResourceBundleReaderImpl3 impl;
				if (!((I_ResourceBundleReaderImpl&)impl).Read(
					version,
					sysMemorySize,
					pSysMemory,
					vidMemorySize,
					pVidMemory,
					bundle,
					file))
					return false;
				break;
			}
			default:
				assert(!"Unknown bundle version!");
		}
	}
	else
	{
		BIZ_WARNING_FAIL("%s: Invalid file type", file.Name().Chars());

		//Invalid file type.
		return false;
	}

	return true;
}

U32 C_ResourceBundleReader::GetHeader(const char* headerString)
{
	U32 a = headerString[3];
	U32 b = headerString[2] << 8;
	U32 c = headerString[1] << 16;
	U32 d = headerString[0] << 24;

	return a | b | c | d;
}
