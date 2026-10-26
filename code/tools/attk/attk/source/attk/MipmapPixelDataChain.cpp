#include "stdafx.h"

#include <attk/Enum.h>
#include <attk/Status.h>
#include <attk/Global.h>
#include <attk/Progress.h>
#include <attk/OptionAttribute.h>
#include <attk/Options.h>
#include <attk/BaseImage.h>
#include <attk/PixelFormats.h>
#include <attk/Image.h>

#include <attk/MipmapPixelDataChain.h>
#include <core/maths.h>

using namespace Attk;

Attk::C_MipmapPixelDataChain::C_MipmapPixelDataChain()
{
}

Attk::C_MipmapPixelDataChain::~C_MipmapPixelDataChain()
{
	Clear();
}

void C_MipmapPixelDataChain::SetData(
	U32 width,
	U32 height,
	U32 depth,
	U32 numLevels,
	E_PixelFormat format,
	U8* pPixelData)
{
	m_width = width;
	m_height = height;
	m_depth = depth;
	m_numLevels = numLevels;
	m_format = format;
	m_pPixelDataChain = pPixelData;
}

void C_MipmapPixelDataChain::SetPixelDataPtr(
	U8* pPixelData)
{
	m_pPixelDataChain = pPixelData;
}

U32 C_MipmapPixelDataChain::GetNumLevels() const
{
	return m_numLevels;
}

U32 C_MipmapPixelDataChain::GetPixelDataSize() const
{
	U32 pixelDataSize = 0;
	for (U32 i = 0; i < m_numLevels; i++)
	{
		pixelDataSize += 
			GetWidth(i) * GetHeight(i) * GetDepth(i) * C_BaseImage::GetBytesPerPixel(m_format);
	}

	return pixelDataSize;
}

U8* C_MipmapPixelDataChain::GetPixelDataPtr()
{
	return m_pPixelDataChain;
}

const U8* C_MipmapPixelDataChain::GetPixelDataPtr() const
{
	return m_pPixelDataChain;
}

U32 C_MipmapPixelDataChain::GetWidth(U32 mipLevel) const
{
	return Maths::Max((S32)(m_width >> mipLevel), 1);
}

U32 C_MipmapPixelDataChain::GetHeight(U32 mipLevel) const
{
	return Maths::Max((S32)(m_height >> mipLevel), 1);
}

U32 C_MipmapPixelDataChain::GetDepth(U32 mipLevel) const
{
	return Maths::Max((S32)(m_depth >> mipLevel), 1);
}

E_PixelFormat C_MipmapPixelDataChain::GetPixelDataFormat() const
{
	return m_format;
}

void C_MipmapPixelDataChain::Clear()
{
	m_width = 0;
	m_height = 0;
	m_depth = 0;
	m_numLevels = 0;
	m_format = E_PixelFormat_Unknown;
	m_pPixelDataChain = 0;
}
