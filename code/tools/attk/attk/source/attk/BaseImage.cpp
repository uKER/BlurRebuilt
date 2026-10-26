#include "stdafx.h"

#include <string>

#include <attk/Definitions.h>
#include <attk/Enum.h>
#include <attk/Status.h>
#include <attk/Global.h>
#include <attk/OptionAttribute.h>
#include <attk/Options.h>
#include <attk/BaseImage.h>

using namespace Attk;


C_BaseImage::C_BaseImage()
{
	_OneTimeInit();
	C_BaseImage::Clear();
}

C_BaseImage::C_BaseImage(const C_BaseImage& rhs)
{
	*this = rhs;
}

C_BaseImage::~C_BaseImage()
{
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
void C_BaseImage::_OneTimeInit()
{
}
#endif

const C_BaseImage& C_BaseImage::operator=(const C_BaseImage& rhs)
{
	m_pixelFormat =  rhs.m_pixelFormat;
	m_paletteFormat =  rhs.m_paletteFormat;

	m_width = rhs.m_width;
	m_height = rhs.m_height;
	m_depth = rhs.m_depth;

	return *this;
}

U32 C_BaseImage::GetBytesPerPixel(E_PixelFormat format)
{
	switch(format)
	{
	case E_PixelFormat_B32G32R32A32F:
	case E_PixelFormat_R32G32B32A32F:
	case E_PixelFormat_A32R32G32B32F:
		return 16;

	case E_PixelFormat_R32G32B32F:
		return 12;

	case E_PixelFormat_A8B8G8R8U:
	case E_PixelFormat_R8G8B8A8U:
	case E_PixelFormat_A8R8G8B8U:
	case E_PixelFormat_B8G8R8A8U:
		return 4;

	case E_PixelFormat_B8G8R8U:
	case E_PixelFormat_R8G8B8U:
		return 3;

	case E_PixelFormat_Luminance16U:
		return 2;

	case E_PixelFormat_Luminance8U:
	case E_PixelFormat_ColourIndex8U:
		return 1;
	};

	ATTK_ASSERT(false);
	return 0;
}

U32 C_BaseImage::GetBitsPerPixel(E_PixelFormat format)
{
	return GetBytesPerPixel(format)*8;
}

U32 C_BaseImage::GetBitsPerPaletteEntry(E_PaletteFormat format)
{
	return GetBytesPerPaletteEntry(format)*8;
}

U32 C_BaseImage::GetChannelsPerPixel() const
{
	switch (m_pixelFormat)
	{
	case E_PixelFormat_Unknown:
		return 0;

	case E_PixelFormat_Luminance8U:
		return 1;

	case E_PixelFormat_Luminance16U:
		return 1;

	case E_PixelFormat_B32G32R32A32F:
	case E_PixelFormat_R32G32B32A32F:
	case E_PixelFormat_A32R32G32B32F:
	case E_PixelFormat_R8G8B8A8U:
	case E_PixelFormat_A8R8G8B8U:
	case E_PixelFormat_B8G8R8A8U:
		return 4;

	case E_PixelFormat_R8G8B8U:
	case E_PixelFormat_B8G8R8U:
	case E_PixelFormat_R32G32B32F:
	case E_PixelFormat_B32G32R32F:
		return 3;
	case E_PixelFormat_ColourIndex8U:
		return GetChannelsPerPaletteEntry();
	};

	ATTK_ASSERT(false);
	return 0;
}

U32 C_BaseImage::GetBitsPerPixel() const
{
	return GetBytesPerPixel()*8;
}

U32 C_BaseImage::GetBytesPerPixel() const
{
	return GetBytesPerPixel(m_pixelFormat);
}

U32 C_BaseImage::GetChannelsPerPaletteEntry() const
{
	return GetChannelsPaletteEntry(m_paletteFormat);
}

U32 C_BaseImage::GetChannelsPaletteEntry(E_PaletteFormat format)
{
	switch(format)
	{
	case E_PaletteFormat_Unknown:
	case E_PaletteFormat_None:
		return 0;

	case E_PaletteFormat_R8G8B8A8U:
	case E_PaletteFormat_B8G8R8A8U:
	case E_PaletteFormat_A8R8G8B8U:
	case E_PaletteFormat_A8B8G8R8U:
		return 4;

	case E_PaletteFormat_R8G8B8U:
	case E_PaletteFormat_B8G8R8U:
		return 3;
	}
	ATTK_ASSERT(false);
	return 0;
}

U32 C_BaseImage::GetBitsPerPaletteEntry() const
{
	return GetBytesPerPaletteEntry()*8;
}

U32 C_BaseImage::GetBytesPerPaletteEntry(E_PaletteFormat paletteFormat)
{
	switch (paletteFormat)
	{
		case E_PaletteFormat_R8G8B8A8U:
			return 4;

		case E_PaletteFormat_B8G8R8A8U:
			return 4;

		case E_PaletteFormat_A8R8G8B8U:
			return 4;

		case E_PaletteFormat_R8G8B8U:
			return 3;

		case E_PaletteFormat_B8G8R8U:
			return 3;
	}

	return 0;
}

U32 C_BaseImage::GetBytesPerPaletteEntry() const
{
	return GetBytesPerPaletteEntry(m_paletteFormat);
}

U32 C_BaseImage::GetNumPaletteEntries(E_PixelFormat pixelFormat)
{
	switch (pixelFormat)
	{
		case E_PixelFormat_ColourIndex8U:
			return 256;

		default:
			ATTK_ASSERT(false);
	}
	return 0;
}

U32 C_BaseImage::GetPaletteDataSize(E_PixelFormat pixelFormat, E_PaletteFormat paletteFormat)
{
	return GetNumPaletteEntries(pixelFormat) * GetBytesPerPaletteEntry(paletteFormat);
}

U32 C_BaseImage::GetPaletteDataSize() const
{
	return GetNumPaletteEntries() * GetBytesPerPaletteEntry();
}
U32 C_BaseImage::GetNumPaletteEntries() const
{
	return GetNumPaletteEntries(m_pixelFormat);
}

E_PixelFormat C_BaseImage::GetPixelFormat() const
{
	return m_pixelFormat;
}

E_PaletteFormat C_BaseImage::GetPaletteFormat() const
{
	return m_paletteFormat;
}

U32 C_BaseImage::GetWidth() const
{
	return m_width;
}

U32 C_BaseImage::GetHeight() const
{
	return m_height;
}

U32 C_BaseImage::GetDepth() const
{
	return m_depth;
}

void C_BaseImage::Clear()
{
	m_pixelFormat = E_PixelFormat_Unknown;
	m_paletteFormat = E_PaletteFormat_None;
	m_width = 0;
	m_height = 0;
	m_depth = 0;
}

C_Status C_BaseImage::Read(
	const char* pFileName,
	E_ImageFormat imageFormat, 
	const C_Options* pOptions)
{
	return E_Status_NotImplemented;
}

C_Status C_BaseImage::Read(
	const void* pSource, 
	U32 size, 
	E_ImageFormat imageFormat,
	const C_Options* pOptions)
{
	return E_Status_NotImplemented;
}
