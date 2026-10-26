#include "stdafx.h"

#include "ProcessedMipmapChain.h"
#include "../Utility.h"

using namespace ResourceBundler;

C_ProcessedMipmapChain::C_ProcessedMipmapChain(
	U32 width,
	U32 height,
	U32 depth)
{
	m_width = width;
	m_height = height;
	m_depth = depth;
	m_format = Display::ResourceBundler::E_TextureFormat_Unknown;
}

C_ProcessedMipmapChain::~C_ProcessedMipmapChain()
{
	for (S32 i = 0; i < m_mipLevels.Size(); i++)
		delete m_mipLevels[i];
}

U32 C_ProcessedMipmapChain::GetNumLevels() const
{
	return (U32)m_mipLevels.Size();
}

U32 C_ProcessedMipmapChain::AddLevelPtr(
	U8* pData,
	Display::ResourceBundler::E_TextureFormat format,
	bool isBigEndian)
{
	if (m_mipLevels.Size() == 0)
	{
		m_format = format;
		m_isBigEndian = isBigEndian;
	}
	else
	{
		//All levels ought to have the same format.
		assert(m_format == format);
		assert(m_isBigEndian == isBigEndian);
	}

	m_mipLevels.PushBack(pData);
	return m_mipLevels.Size()-1;
}

U8* C_ProcessedMipmapChain::GetLevelPtr(U32 level)
{
	return m_mipLevels[level];
}

const U8* C_ProcessedMipmapChain::GetLevelPtr(U32 level) const
{
	return m_mipLevels[level];
}

U32 C_ProcessedMipmapChain::GetWidth(U32 mipLevel) const
{
	return Maths::Max((S32)(m_width >> mipLevel), 1);
}

U32 C_ProcessedMipmapChain::GetHeight(U32 mipLevel) const
{
	return Maths::Max((S32)(m_height >> mipLevel), 1);
}

U32 C_ProcessedMipmapChain::GetDepth(U32 mipLevel) const
{
	return Maths::Max((S32)(m_depth >> mipLevel), 1);
}

U32 C_ProcessedMipmapChain::GetBaseSize() const
{
	return GetLevelSize(0);
}

U32 C_ProcessedMipmapChain::GetMipSize() const
{
	U32 size = 0;
	//Accumulate mip sizes.
	for (U32 i = 1; i < (U32)m_mipLevels.Size(); i++)
		size += GetLevelSize(i);
	return size;
}

U32 C_ProcessedMipmapChain::GetLevelSize(U32 level) const
{
#ifdef USER_LLOYDW

	// Old method to calculate mip size

	U32 widthBlocks = GetWidth(level) / Utility::GetBlockWidth(m_format);
	U32 heightBlocks = GetHeight(level) / Utility::GetBlockWidth(m_format);
	U32 depthBlocks = GetDepth(level);
	U32 oldSize = widthBlocks * heightBlocks * depthBlocks* Utility::GetBytesPerBlock(m_format);

	// New method to calculate mip size

	U32 newSize = 0;

	U32 newWidth = GetWidth(level);
	U32 newHeight = GetHeight(level);

	// 0 Unknown
	// 1 DXT1
	// 2 DXT2
	// 3 DXT3
	// 4 DXT4
	// 5 DXT5
	// 6 A8R8G8B8
	// 7 L8
	// 8 A32R32G32B32
	// 9 DXN

	switch(m_format)
	{
	case 1: // DXT1
		newSize += ((newWidth + 3) / 4) * ((newHeight + 3) / 4) * 8;
		break;
	case 3: // DXT3
	case 5: // DXT5
		newSize = ((newWidth + 3) / 4) * ((newHeight + 3) / 4) * 16;
		break;
	case 6: // A8R8G8B8
		newSize = newWidth * newHeight * 4;
		break;
	default:
		assert(0 && "Unsupported texture format");
		break;
	}

	//assert(oldSize == newSize);

	return newSize;
#else
	U32 widthBlocks = GetWidth(level) / Utility::GetBlockWidth(m_format);
	U32 heightBlocks = GetHeight(level) / Utility::GetBlockWidth(m_format);
	U32 depthBlocks = GetDepth(level);
	return widthBlocks * heightBlocks * depthBlocks* Utility::GetBytesPerBlock(m_format);
#endif
}

Display::ResourceBundler::E_TextureFormat C_ProcessedMipmapChain::GetFormat() const
{
	return m_format;
}

bool C_ProcessedMipmapChain::IsBigEndian() const
{
	return m_isBigEndian;
}

