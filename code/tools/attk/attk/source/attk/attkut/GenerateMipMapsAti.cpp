#include "stdafx.h"

#include <attk/Definitions.h>
#include <attk/Enum.h>
#include <attk/Status.h>
#include <attk/Global.h>
#include <attk/Progress.h>
#include <attk/OptionAttribute.h>
#include <attk/Options.h>
#include <attk/BaseImage.h>
#include <attk/PixelFormats.h>
#include <attk/Image.h>
#include <attk/CubeMap.h>
#include <attk/MipmapImageChain.h>
#include <attk/MipmapPixelDataChain.h>
#include <attk/Utility.h>
#include <attk/attkut/ImageFilter.h>
#include <attk/attkut/ImageResize.h>
#include <attk/attkut/GenerateMipMaps.h>

#ifndef WIN64

#pragma push_macro("WCHAR")
//Prevent linker errors as wchar_t is not treated as a built in type /Zc:wchar_t- in the ati lib
//otherwise we'd need to disable this option on all projects using the attk
#define WCHAR unsigned __int16
#include <ati_cubemapgen/ErrorMsg.h>
#include <ati_cubemapgen/CCubeMapProcessor.h>
#pragma pop_macro("WCHAR")

#include <core/maths.h>

using namespace AttkUt;

C_GenerateMipmaps::CubeMapFilterParameters C_GenerateMipmaps::m_CubeMapFilterParams;

C_GenerateMipmaps::CubeMapFilterParameters::CubeMapFilterParameters()
	: numThreads(2)
	, baseFilterAngle(1.f)
	, initialMipAngle(1.f)
	, mipAnglePerLevelScale(2.f)
	, filterType(E_FilterType_Disc)
	, fixupType(E_EdgeFixupType_PullHermite)
	, fixupWidth(1)
	, useSolidAngle(TRUE)
	, compression(E_Compression_None)
{}

Attk::C_Status C_GenerateMipmaps::Generate(
	Attk::C_MipmapCubeMapImageChain mipmapChain,
	const Attk::C_CubeMap& cubemap,
	const U32 levels)
{
	for (U32 i=0; i<Attk::E_CubeMap_Face_Count; ++i)
		mipmapChain[i].Clear();

	for (U32 i=0; i<Attk::E_CubeMap_Face_Count; ++i)
	{
		if (!cubemap[i].GetPixelDataPtr())
			return Attk::E_Status_Failure;
	}

	Attk::C_CubeMap workingCopy(cubemap);
	workingCopy.Validate();

	for (U32 i=0; i<Attk::E_CubeMap_Face_Count; ++i)
		workingCopy[i].ChangePixelFormat(Attk::E_PixelFormat_R32G32B32A32F);

	//Pass our cubemap data into ATI's cubemapgen library for it to do its magic
	{
		CCubeMapProcessor cubeMapProcessor;
		cubeMapProcessor.Init(workingCopy[0].GetWidth(), workingCopy[0].GetHeight(), levels, 4);

		cubeMapProcessor.m_NumFilterThreads = m_CubeMapFilterParams.numThreads;

		for (U32 i=0; i<Attk::E_CubeMap_Face_Count; ++i)
			cubeMapProcessor.SetInputFaceData(
				i,										//face index (same as our index),
				CP_VAL_FLOAT32,							//source data type
				4,										//number of channels
				workingCopy[i].GetWidth()*16,			//pitch
				workingCopy[i].GetPixelDataPtr(),		//data ptr
				1000000.0f,								//max clamping value
				1.0f,									//degamma?
				1.0f);									//scale

		#ifdef _DEBUG
		std::cout << "Starting ATI cube map filtering" << std::endl;
		U32 start = GetTickCount();
		#endif
		cubeMapProcessor.InitiateFiltering(
			m_CubeMapFilterParams.baseFilterAngle,
			m_CubeMapFilterParams.initialMipAngle,
			m_CubeMapFilterParams.mipAnglePerLevelScale,
			m_CubeMapFilterParams.filterType,
			m_CubeMapFilterParams.fixupType,
			m_CubeMapFilterParams.fixupWidth,
			m_CubeMapFilterParams.useSolidAngle);

		// wait if we're running asynchronously
		while (cubeMapProcessor.GetStatus() == CP_STATUS_PROCESSING)
		{
			#ifdef _DEBUG
			//std::wcout <<  (wchar_t*)cubeMapProcessor.GetFilterProgressString() << std::endl;
			#endif
			::Sleep(200);
		}

		#ifdef _DEBUG
		U32 end = GetTickCount();
		U32 delta = end - start;
		std::cout << "Cube map filtering finished in " << (F32)(delta) / 1000.f << " seconds" << std::endl;
		#endif

		// NOTE: top level mip is also filtered! Replace with original
		for (U32 i=0; i<Attk::E_CubeMap_Face_Count; ++i)
		{
			Attk::C_Image* pMipLevel = new Attk::C_Image(cubemap[i]);
			if (!pMipLevel)
				return Attk::E_Status_OutOfMemory;
				
			mipmapChain[i].AddImagePtr(pMipLevel);
		}

		for (U32 j=1; j<(U32)cubeMapProcessor.m_NumMipLevels; ++j)
		{
			for (U32 i=0; i<Attk::E_CubeMap_Face_Count; ++i)
			{
				Attk::C_Image* pMipLevel = new Attk::C_Image();
				if (!pMipLevel)
					return Attk::E_Status_OutOfMemory;
				
				mipmapChain[i].AddImagePtr(pMipLevel);

				CImageSurface& surface = cubeMapProcessor.m_OutputSurface[j][i];

				switch (m_CubeMapFilterParams.compression)
				{
				case CubeMapFilterParameters::E_Compression_None:
					pMipLevel->NewPixelData(surface.m_Width, surface.m_Height, 1, Attk::E_PixelFormat_R32G32B32A32F, 0);
					cubeMapProcessor.GetOutputFaceData(i, j, CP_VAL_FLOAT32, 4, surface.m_Width*16, pMipLevel->GetPixelDataPtr(), 1, 1);
					break;
				case CubeMapFilterParameters::E_Compression_DXT1:
					pMipLevel->NewPixelData(surface.m_Width, surface.m_Height, 1, Attk::E_PixelFormat_Compressed_DXT1, 0);
					cubeMapProcessor.GetOutputFaceData(i, j, CP_VAL_DXT1, 4, Attk::Utility::GetPitch(surface.m_Width, Attk::E_PixelFormat_Compressed_DXT1), pMipLevel->GetPixelDataPtr(), 1, 1);
					break;
				case CubeMapFilterParameters::E_Compression_DXT3:
					pMipLevel->NewPixelData(surface.m_Width, surface.m_Height, 1, Attk::E_PixelFormat_Compressed_DXT3, 0);
					cubeMapProcessor.GetOutputFaceData(i, j, CP_VAL_DXT3, 4, Attk::Utility::GetPitch(surface.m_Width, Attk::E_PixelFormat_Compressed_DXT3), pMipLevel->GetPixelDataPtr(), 1, 1);
					break;
				case CubeMapFilterParameters::E_Compression_DXT5:
					pMipLevel->NewPixelData(surface.m_Width, surface.m_Height, 1, Attk::E_PixelFormat_Compressed_DXT5, 0);
					cubeMapProcessor.GetOutputFaceData(i, j, CP_VAL_DXT5, 4, Attk::Utility::GetPitch(surface.m_Width, Attk::E_PixelFormat_Compressed_DXT5), pMipLevel->GetPixelDataPtr(), 1, 1);
					break;
				default:
					return Attk::E_Status_Failure;
				}
			}
		}
	}

	return Attk::E_Status_Success;
}

Attk::C_Status C_GenerateMipmaps::Generate(
	Attk::C_MipmapCubeMapPixelDataChain &mipmapChain,
	const Attk::C_CubeMap &cubemap,
	const U32 levels)
{
	Attk::C_Status status;

	if (m_CubeMapFilterParams.compression != CubeMapFilterParameters::E_Compression_None)
		return Attk::E_Status_Failure;

	for (U32 i=0; i<Attk::E_CubeMap_Face_Count; ++i)
		mipmapChain[i].Clear();

	for (U32 i=0; i<Attk::E_CubeMap_Face_Count; ++i)
	{
		if (!cubemap[i].GetPixelDataPtr())
			return Attk::E_Status_Failure;
	}

	Attk::C_CubeMap workingCopy(cubemap);
	workingCopy.Validate();

	for (U32 i=0; i<Attk::E_CubeMap_Face_Count; ++i)
		workingCopy[i].ChangePixelFormat(Attk::E_PixelFormat_R32G32B32A32F);

	//Pass our cubemap data into ATI's cubemapgen library for it to do its magic
	{
		CCubeMapProcessor cubeMapProcessor;
		cubeMapProcessor.Init(workingCopy[0].GetWidth(), workingCopy[0].GetHeight(), levels, 4);

		cubeMapProcessor.m_NumFilterThreads = m_CubeMapFilterParams.numThreads;

		for (U32 i=0; i<Attk::E_CubeMap_Face_Count; ++i)
			cubeMapProcessor.SetInputFaceData(
				i,										//face index (same as our index),
				CP_VAL_FLOAT32,							//source data type
				4,										//number of channels
				workingCopy[i].GetWidth()*16,			//pitch
				workingCopy[i].GetPixelDataPtr(),		//data ptr
				1000000.0f,								//max clamping value
				1.0f,									//degamma
				1.0f);									//scale

		#ifdef _DEBUG
		std::cout << "Starting ATI cube map filtering" << std::endl;
		U32 start = GetTickCount();
		#endif
		cubeMapProcessor.InitiateFiltering(
			m_CubeMapFilterParams.baseFilterAngle,
			m_CubeMapFilterParams.initialMipAngle,
			m_CubeMapFilterParams.mipAnglePerLevelScale,
			m_CubeMapFilterParams.filterType,
			m_CubeMapFilterParams.fixupType,
			m_CubeMapFilterParams.fixupWidth,
			m_CubeMapFilterParams.useSolidAngle);

		// wait if we're running asynchronously
		while (cubeMapProcessor.GetStatus() == CP_STATUS_PROCESSING)
		{
			#ifdef _DEBUG
			//std::wcout <<  (wchar_t*)cubeMapProcessor.GetFilterProgressString() << std::endl;
			#endif
			::Sleep(200);
		}
		#ifdef _DEBUG
		U32 end = GetTickCount();
		U32 delta = end - start;
		std::cout << "Cube map filtering finished in " << (F32)(delta) / 1000.f << " seconds" << std::endl;
		#endif
		for (U32 i=0; i<Attk::E_CubeMap_Face_Count; ++i)
		{
			//Set header info.
			mipmapChain[i].SetData(
				workingCopy[0].GetWidth(),
				workingCopy[0].GetHeight(),
				1,
				cubeMapProcessor.m_NumMipLevels,
				workingCopy[0].GetPixelFormat(),
				0);

			//Alloc storage.
			U32 dataSize = mipmapChain[0].GetPixelDataSize();
			U8* pPixelData = new U8[dataSize];
			U32 offset = 0;

			if (!pPixelData)
				return Attk::E_Status_OutOfMemory;

			// NOTE: top level mip is also filtered! Replace with original
			memcpy(pPixelData + offset, workingCopy[0].GetPixelDataPtr(), workingCopy[0].GetPixelDataSize());
			offset += workingCopy[0].GetPixelDataSize();

			for (U32 j=1; j<(U32)cubeMapProcessor.m_NumMipLevels; ++j)
			{
				Attk::C_Image mipLevel;
				CImageSurface& surface = cubeMapProcessor.m_OutputSurface[j][i];

				mipLevel.NewPixelData(surface.m_Width, surface.m_Height, 1, Attk::E_PixelFormat_R32G32B32A32F, 0);
				//surface.GetImageData(CP_VAL_FLOAT32, 4, surface.m_Width*16, mipLevel.GetPixelDataPtr());
				cubeMapProcessor.GetOutputFaceData(i, j, CP_VAL_FLOAT32, 4, surface.m_Width*16, mipLevel.GetPixelDataPtr(), 1, 1);

				mipLevel.ChangePixelFormat(workingCopy[0].GetPixelFormat());
				memcpy(pPixelData + offset, mipLevel.GetPixelDataPtr(), mipLevel.GetPixelDataSize());
				offset += mipLevel.GetPixelDataSize();
			}
			mipmapChain[i].SetPixelDataPtr(pPixelData);
		}
	}

	return Attk::E_Status_Success;
}

#endif
