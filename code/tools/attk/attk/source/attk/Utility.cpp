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

using namespace Attk;

U32 Utility::GetBlockSize(
	E_PixelFormat format)
{
	switch (format)
	{
		case E_PixelFormat_Compressed_DXT1:
			return 8;
		case E_PixelFormat_Compressed_DXT2:
		case E_PixelFormat_Compressed_DXT3:
		case E_PixelFormat_Compressed_DXT4:
		case E_PixelFormat_Compressed_DXT5:
		case E_PixelFormat_Compressed_DXN:
			return 16;
		default:
			return C_BaseImage::GetBytesPerPixel(format);
	}
}

U32 Utility::GetBlockWidth(
	E_PixelFormat format)
{
	switch (format)
	{
		case E_PixelFormat_Compressed_DXT1:
		case E_PixelFormat_Compressed_DXT2:
		case E_PixelFormat_Compressed_DXT3:
		case E_PixelFormat_Compressed_DXT4:
		case E_PixelFormat_Compressed_DXT5:
		case E_PixelFormat_Compressed_DXN:
			return 4;
		default:
			return 1;
	}
}

U32 Utility::GetPitch(
	U32 width,
	E_PixelFormat format)
{
	if (width < GetBlockWidth(format))
		width = GetBlockWidth(format);

	U32 widthInBlocks = width / GetBlockWidth(format);
	
	return widthInBlocks * GetBlockSize(format);
}

U32 Utility::GetSlicePitch(
	U32 width,
	U32 height,
	E_PixelFormat format)
{
	if (width < GetBlockWidth(format))
		width = GetBlockWidth(format);

	if (height < GetBlockWidth(format))
		height = GetBlockWidth(format);

	U32 widthInBlocks = width / GetBlockWidth(format);
	U32 heightInBlocks = height / GetBlockWidth(format);

	return widthInBlocks * heightInBlocks * GetBlockSize(format);
}


