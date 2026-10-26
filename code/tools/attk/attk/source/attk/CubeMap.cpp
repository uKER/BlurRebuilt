#include "stdafx.h"

#include <attk/Definitions.h>
#include <attk/Enum.h>
#include <attk/Status.h>
#include <attk/Global.h>
#include <attk/Utility.h>
#include <attk/Progress.h>
#include <attk/OptionAttribute.h>
#include <attk/Options.h>
#include <attk/BaseImage.h>
#include <attk/PixelFormats.h>
#include <attk/Image.h>
#include <attk/CubeMap.h>
#include <attk/PixelConversion.h>
#include <attk/PaletteConversion.h>
#include <attk/PixelBlend.h>
#include <attk/Utility.h>

#include <attk/attkut/ImageFilter.h>
#include <attk/attkut/ImageResize.h>

//Plugins
#include <attk_plugins/IPlugin.h>
#include <attk_plugins/DevILPlugin.h>

#include <il/DevIL.h>
#include <attk/DevILUtility.h>

#include <system/Serialise.h>

using namespace Attk;

C_CubeMap::C_CubeMap()
{
}

C_CubeMap::~C_CubeMap()
{
}

C_CubeMap::C_CubeMap(const C_CubeMap& rhs)
{
	for (U32 i=0; i<E_CubeMap_Face_Count; ++i)
		m_Faces[i] = rhs.m_Faces[i];
}

const C_CubeMap& C_CubeMap::operator =(const C_CubeMap& rhs)
{
	for (U32 i=0; i<E_CubeMap_Face_Count; ++i)
		m_Faces[i] = rhs.m_Faces[i];

	return *this;
}

C_Image& C_CubeMap::operator [](U32 face)
{
	ATTK_ASSERT(face < E_CubeMap_Face_Count);
	return m_Faces[face];
}

const C_Image& C_CubeMap::operator [](U32 face) const
{
	ATTK_ASSERT(face < E_CubeMap_Face_Count);
	return m_Faces[face];
}

void C_CubeMap::Clear()
{
	for (U32 face = E_CubeMap_Face_X_Pos;
		face < E_CubeMap_Face_Count;
		++face)
	{
		m_Faces[face].Clear();
	}
}

C_Status C_CubeMap::Validate()
{
	const U32 pixelFormatOrder[E_PixelFormat_Count] = 
	{
		7,	//E_PixelFormat_R8G8B8A8U
		8,	//E_PixelFormat_A8R8G8B8U
		9,	//E_PixelFormat_A8B8G8R8U
		3,	//E_PixelFormat_R8G8B8U
		4,	//E_PixelFormat_B8G8R8U
		10,	//E_PixelFormat_B8G8R8A8U
		11,	//E_PixelFormat_R32G32B32A32F
		12,	//E_PixelFormat_A32R32G32B32F
		5,	//E_PixelFormat_R32G32B32F
		6,	//E_PixelFormat_B32G32R32F
		13,	//E_PixelFormat_B32G32R32A32F
		0,	//E_PixelFormat_ColourIndex8U
		1,	//E_PixelFormat_Luminance8U
		2,	//E_PixelFormat_Luminance16U
	};

	//Convert pixel formats
	Attk::E_PixelFormat pixelFormat = m_Faces[0].GetPixelFormat();
	bool conversionRequired = false;

	for (U32 i=1; i<E_CubeMap_Face_Count; ++i)
	{
		Attk::E_PixelFormat currentPixelFormat = m_Faces[i].GetPixelFormat();
		if (currentPixelFormat != pixelFormat)
		{
			conversionRequired = true;
			// Figure out which format to convert to
			if (pixelFormatOrder[currentPixelFormat] > pixelFormatOrder[pixelFormat])
				pixelFormat = currentPixelFormat;
		}
	}

	if (conversionRequired)
	{
		for (U32 i=0; i<E_CubeMap_Face_Count; ++i)
			m_Faces[i].ChangePixelFormat(pixelFormat);
	}

	//Resize all faces so that they are all of equal size
	U32 maxWidth=m_Faces[0].GetWidth(), maxHeight=m_Faces[0].GetWidth();
	bool resizeRequired = false;

	for (U32 i=1; i<E_CubeMap_Face_Count; ++i)
	{
		if (m_Faces[i].GetWidth()!=maxWidth || m_Faces[i].GetHeight()!=maxHeight)
			resizeRequired = true;

		maxWidth = Maths::Max(maxWidth, m_Faces[i].GetWidth());
		maxHeight = Maths::Max(maxHeight, m_Faces[i].GetHeight());
	}

	if (resizeRequired)
		return ResizeAll(maxWidth, maxHeight);

	return E_Status_Success;
}

C_Status C_CubeMap::ResizeAll(U32 width, U32 height)
{
	for (U32 i=0; i<E_CubeMap_Face_Count; ++i)
		AttkUt::C_ImageResize::Resize(m_Faces[i], width, height, 1, AttkUt::C_ImageFilter::E_PixelFilter_Box);

	return E_Status_Success;
}


C_Status C_CubeMap::Read(
	const char* pFileName,
	E_ImageFormat imageFormat,
	const C_Options* pOptions)
{
	C_Status status;
	bool canRead;

	if (!Sys::C_FileSystem::Instance().FileExists(pFileName))
		return E_Status_Failure;

	C_DevILPlugin plugin;
	canRead = plugin.CanRead(pFileName, &status);
	if (!status) return status;

	if (canRead)
	{
		status = plugin.Read(*this, pFileName, pOptions);
		if (!status) return status;
	}

	return E_Status_Success;
}

C_Status C_CubeMap::Write(
	const char* pFileName,
	E_ImageFormat imageFormat,
	const C_Options* pOptions) const
{
	C_Status status;
	bool canWrite;
	char fileName[256];

	C_DevILPlugin plugin;
	canWrite = plugin.CanWrite(pFileName, &status);
	if (!status) return status;

	if (canWrite)
	{
		for (U32 i=0; i<E_CubeMap_Face_Count; ++i)
		{
		if (m_Faces[i].GetPixelDataPtr())
		{
			sprintf(fileName, pFileName, i);
			status = plugin.Write(m_Faces[i], fileName, pOptions);
			if (!status) return status;
		}
		}
	}

	return Attk::E_Status_Success;
}