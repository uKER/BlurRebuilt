#ifndef ATTK_ENUM_H
#define ATTK_ENUM_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Attk
{
	enum E_PixelFormat
	{
		E_PixelFormat_Unknown,

		//unsigned formats
		E_PixelFormat_R8G8B8A8U,
		E_PixelFormat_A8R8G8B8U,
		E_PixelFormat_A8B8G8R8U,

		E_PixelFormat_R8G8B8U,
		E_PixelFormat_B8G8R8U,
		E_PixelFormat_B8G8R8A8U,

		//floating point formats
		E_PixelFormat_R32G32B32A32F,

		E_PixelFormat_A32R32G32B32F,

		E_PixelFormat_R32G32B32F,
		E_PixelFormat_B32G32R32F,
		E_PixelFormat_B32G32R32A32F,

		E_PixelFormat_ColourIndex8U,
		E_PixelFormat_Luminance8U,
		E_PixelFormat_Luminance16U,

		E_PixelFormat_Compressed_DXT1,
		E_PixelFormat_Compressed_DXT2,
		E_PixelFormat_Compressed_DXT3,
		E_PixelFormat_Compressed_DXT4,
		E_PixelFormat_Compressed_DXT5,
		E_PixelFormat_Compressed_DXN,

		E_PixelFormat_Count
	};

	enum E_PaletteFormat
	{
		E_PaletteFormat_Unknown,
		E_PaletteFormat_None,
		E_PaletteFormat_R8G8B8A8U,
		E_PaletteFormat_B8G8R8A8U,
		E_PaletteFormat_A8R8G8B8U,
		E_PaletteFormat_A8B8G8R8U,

		E_PaletteFormat_R8G8B8U,
		E_PaletteFormat_B8G8R8U,

		E_PaletteFormat_Count
	};

	enum E_BlendMode
	{
		E_BlendMode_Additive,
		E_BlendMode_Multiply,
		E_BlendMode_None
	};

	enum E_ImageFormat
	{
		E_ImageFormat_BestGuess,

		//Attk native image format.
		E_ImageFormat_Attk,

		//DevIL formats.
		E_ImageFormat_Bmp,
		E_ImageFormat_Cut,
		E_ImageFormat_Dds,
		E_ImageFormat_Gif,
		E_ImageFormat_Ico,
		E_ImageFormat_Cur,
		E_ImageFormat_Jpeg,
		E_ImageFormat_Lif,
		E_ImageFormat_Lmp,
		E_ImageFormat_Mdl,
		E_ImageFormat_Mng,
		E_ImageFormat_Pcd,
		E_ImageFormat_Pcx,
		E_ImageFormat_Pic,
		E_ImageFormat_Pix,
		E_ImageFormat_Png,
		E_ImageFormat_Pbm,
		E_ImageFormat_Pgm,
		E_ImageFormat_Ppm,
		E_ImageFormat_Pnm,
		E_ImageFormat_Psd,
		E_ImageFormat_Pxr,
		E_ImageFormat_Sgi,
		E_ImageFormat_Bw,
		E_ImageFormat_Rgb,
		E_ImageFormat_Rgba,
		E_ImageFormat_Tga,
		E_ImageFormat_Tif,
		E_ImageFormat_Tiff,
		E_ImageFormat_Wal,
		E_ImageFormat_Xpm,
		E_ImageFormat_Raw
	};

	enum E_Status
	{
		E_Status_Success,
		E_Status_Failure,
		E_Status_NotImplemented,
		E_Status_OutOfMemory,
		E_Status_FailedToOpenFile,
		E_Status_OutOfDiskSpace,
		E_Status_Cancelled
	};

	enum E_Channel
	{
		E_Channel_0 = 1,
		E_Channel_1 = 2,
		E_Channel_2 = 4,
		E_Channel_3 = 8,
	};

	enum E_CubeMap_Face
	{
		E_CubeMap_Face_X_Pos=0,
		E_CubeMap_Face_X_Neg,
		E_CubeMap_Face_Y_Pos,
		E_CubeMap_Face_Y_Neg,
		E_CubeMap_Face_Z_Pos,
		E_CubeMap_Face_Z_Neg,

		E_CubeMap_Face_Count
	};
}

#endif
