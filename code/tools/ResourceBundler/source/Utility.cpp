#include "stdafx.h"

#include "Utility.h"

const char* ResourceBundler::Utility::GetTextureFormatString(Display::ResourceBundler::E_TextureFormat format)
{
	return Display::ResourceBundler::g_textureFormat[format];
}

Display::ResourceBundler::E_TextureFormat ResourceBundler::Utility::GetTextureFormatType(const char* format)
{
	for (U32 i = 0; true; i++)
	{
		if (Display::ResourceBundler::g_textureFormat[i][0] == '\0')
			break;
		
		if (!_stricmp(Display::ResourceBundler::g_textureFormat[i], format))
			return Display::ResourceBundler::E_TextureFormat(i);
	}

	return Display::ResourceBundler::E_TextureFormat_Unknown;
}

U32 ResourceBundler::Utility::GetBytesPerBlock(Display::ResourceBundler::E_TextureFormat format)
{
	return Display::ResourceBundler::g_textureFormatBytesPerBlock[format];
}

U32 ResourceBundler::Utility::GetBlockWidth(Display::ResourceBundler::E_TextureFormat format)
{
	return Display::ResourceBundler::g_textureFormatBlockWidth[format];
}

U32 ResourceBundler::Utility::GetPitch(U32 width, Display::ResourceBundler::E_TextureFormat format)
{
	if (width < Display::ResourceBundler::g_textureFormatBlockWidth[format])
		width = Display::ResourceBundler::g_textureFormatBlockWidth[format];

	U32 widthInBlocks = width / Display::ResourceBundler::g_textureFormatBlockWidth[format];
	
	return widthInBlocks * Display::ResourceBundler::g_textureFormatBytesPerBlock[format];
}

U32 ResourceBundler::Utility::GetSlicePitch(U32 width, U32 height, Display::ResourceBundler::E_TextureFormat format)
{
	if (width < Display::ResourceBundler::g_textureFormatBlockWidth[format])
		width = Display::ResourceBundler::g_textureFormatBlockWidth[format];

	if (height < Display::ResourceBundler::g_textureFormatBlockWidth[format])
		height = Display::ResourceBundler::g_textureFormatBlockWidth[format];

	U32 widthInBlocks = width / Display::ResourceBundler::g_textureFormatBlockWidth[format];
	U32 heightInBlocks = height / Display::ResourceBundler::g_textureFormatBlockWidth[format];

	return widthInBlocks * heightInBlocks * Display::ResourceBundler::g_textureFormatBytesPerBlock[format];
}

bool ResourceBundler::Utility::RequiresPow2Size(Display::ResourceBundler::E_TextureFormat format)
{
	return Display::ResourceBundler::g_requiresPow2Size[format];
}

bool ResourceBundler::Utility::IsPow2Size(U32 width, U32 height, U32 depth)
{
	if (!(width & (width - 1)) && 
		!(height & (height - 1)) && 
		!(depth & (depth - 1)))
		return true;

	return false;
}

S32 ResourceBundler::Utility::MakePow2(S32 val)
{
	S32 i = 1;
	for (; (i<<1) <= val; i<<=1);
	return i;
}

Display::ResourceBundler::E_TextureFormat ResourceBundler::Utility::GetEquivalentTextureFormat(
	Attk::E_PixelFormat format)
{
	switch (format)
	{
	case Attk::E_PixelFormat_Compressed_DXT1:
		return Display::ResourceBundler::E_TextureFormat_DXT1;
	case Attk::E_PixelFormat_Compressed_DXT2:
		return Display::ResourceBundler::E_TextureFormat_DXT2;
	case Attk::E_PixelFormat_Compressed_DXT3:
		return Display::ResourceBundler::E_TextureFormat_DXT3;
	case Attk::E_PixelFormat_Compressed_DXT4:
		return Display::ResourceBundler::E_TextureFormat_DXT4;
	case Attk::E_PixelFormat_Compressed_DXT5:
		return Display::ResourceBundler::E_TextureFormat_DXT5;
	case Attk::E_PixelFormat_A8B8G8R8U:
		return Display::ResourceBundler::E_TextureFormat_A8R8G8B8;
	case Attk::E_PixelFormat_Luminance8U:
		return Display::ResourceBundler::E_TextureFormat_L8;
	case Attk::E_PixelFormat_A32R32G32B32F:
		return Display::ResourceBundler::E_TextureFormat_A32R32G32B32;
	case Attk::E_PixelFormat_Compressed_DXN:
		return Display::ResourceBundler::E_TextureFormat_DXN;
	default:
		return Display::ResourceBundler::E_TextureFormat_Unknown;
	}
}