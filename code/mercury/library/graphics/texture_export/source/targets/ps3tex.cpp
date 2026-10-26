#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>
#include <core/containers/string.h>

#include "Cg/cg.h"
#include "cell/gcm.h"
#include "sdk_version.h"

#include <core/core.h>
#include <core/containers/string.h>
#include <system/scheduler/core/tl_defs.h>
#include <system/serialise/I_BinaryStream.h>
#include <system/serialise/BinarySerialise.h>
#include <core/utility/endian.h>
#include "ngl/defs.h"

#define NGL_API
#include "texture_export/ngl/ps3/ngl_ps3_gpu_types.h"
#include "texture_export/ngl/gpu/ngl_gpu_texture.h"

#include "texture_export/texture_export.h"
#include "texture_export/Texture.h"

#include "texture_export/targets/ps3tex.h"

#include <d3d9.h>
#include <d3dx9tex.h>

#include <misc_shared/types/float16.h>

using namespace ActTools;

struct Ps3TexelFormatDesc
{
	const char* name;
	uint format; 
	D3DFORMAT d3d_format;
};

Ps3TexelFormatDesc Ps3TexelFormats[] =
{
	{ "DXT1", GPU_TEXTURE_DXT1, D3DFMT_DXT1 },
	{ "DXT1_RGB", GPU_TEXTURE_DXT1_RGB, D3DFMT_DXT1 },
	{ "DXT3", GPU_TEXTURE_DXT3, D3DFMT_DXT3 },
	{ "DXT5", GPU_TEXTURE_DXT5, D3DFMT_DXT5 },
	{ "LIN_L8", GPU_TEXTURE_LIN_L8, D3DFMT_L8 },
	{ "LIN_A8", GPU_TEXTURE_LIN_A8, D3DFMT_A8 },
	{ "L8", GPU_TEXTURE_L8, D3DFMT_L8 },
	{ "A8", GPU_TEXTURE_A8, D3DFMT_A8 },
	{ "LIN_A1R5G5B5", GPU_TEXTURE_LIN_A1R5G5B5, D3DFMT_A1R5G5B5 },
	{ "LIN_X1R5G5B5", GPU_TEXTURE_LIN_X1R5G5B5, D3DFMT_X1R5G5B5 },
	{ "LIN_A1B5G5R5", GPU_TEXTURE_LIN_A1B5G5R5, D3DFMT_A1R5G5B5 },
	{ "LIN_X1B5G5R5", GPU_TEXTURE_LIN_X1B5G5R5, D3DFMT_UNKNOWN },
	{ "LIN_R5G6B5", GPU_TEXTURE_LIN_R5G6B5, D3DFMT_R5G6B5 },
	{ "LIN_R6G5B5", GPU_TEXTURE_LIN_R6G5B5, D3DFMT_UNKNOWN },
	{ "LIN_A4R4G4B4", GPU_TEXTURE_LIN_A4R4G4B4, D3DFMT_A4R4G4B4 },
	{ "LIN_A8L8", GPU_TEXTURE_LIN_A8L8, D3DFMT_A8L8 },
	{ "LIN_G8B8", GPU_TEXTURE_LIN_G8B8, D3DFMT_UNKNOWN },
	{ "LIN_X16", GPU_TEXTURE_LIN_X16, D3DFMT_INDEX16 },
	{ "LIN_HILO8", GPU_TEXTURE_LIN_HILO8, D3DFMT_UNKNOWN },
	{ "LIN_HILOS8", GPU_TEXTURE_LIN_HILOS8, D3DFMT_UNKNOWN },
	{ "LIN_D16", GPU_TEXTURE_LIN_D16, D3DFMT_D16 },
	{ "LIN_D16F", GPU_TEXTURE_LIN_D16F, D3DFMT_UNKNOWN },
	{ "A1R5G5B5", GPU_TEXTURE_A1R5G5B5, D3DFMT_A1R5G5B5 },
	{ "X1R5G5B5", GPU_TEXTURE_X1R5G5B5, D3DFMT_X1R5G5B5 },
	{ "A1B5G5R5", GPU_TEXTURE_A1B5G5R5, D3DFMT_UNKNOWN },
	{ "X1B5G5R5", GPU_TEXTURE_X1B5G5R5, D3DFMT_UNKNOWN },
	{ "R5G6B5", GPU_TEXTURE_R5G6B5, D3DFMT_R5G6B5 },
	{ "R6G5B5", GPU_TEXTURE_R6G5B5, D3DFMT_UNKNOWN },
	{ "A4R4G4B4", GPU_TEXTURE_A4R4G4B4, D3DFMT_A4R4G4B4 },
	{ "A8L8", GPU_TEXTURE_A8L8, D3DFMT_A8L8 },
	{ "G8B8", GPU_TEXTURE_G8B8, D3DFMT_UNKNOWN },
	{ "X16", GPU_TEXTURE_X16, D3DFMT_INDEX16 },
	{ "HILO8", GPU_TEXTURE_HILO8, D3DFMT_UNKNOWN },
	{ "HILOS8", GPU_TEXTURE_HILOS8, D3DFMT_UNKNOWN },
	{ "D16", GPU_TEXTURE_D16, D3DFMT_D16 },
	{ "D16F", GPU_TEXTURE_D16F, D3DFMT_UNKNOWN },
	{ "LIN_A8R8G8B8", GPU_TEXTURE_LIN_A8R8G8B8, D3DFMT_A8R8G8B8 },
	{ "LIN_X8R8G8B8", GPU_TEXTURE_LIN_X8R8G8B8, D3DFMT_X8R8G8B8 },
	{ "LIN_A8B8G8R8", GPU_TEXTURE_LIN_A8B8G8R8, D3DFMT_A8B8G8R8 },
	{ "LIN_X8B8G8R8", GPU_TEXTURE_LIN_X8B8G8R8, D3DFMT_X8B8G8R8 },
	{ "LIN_R8G8B8A8", GPU_TEXTURE_LIN_R8G8B8A8, D3DFMT_UNKNOWN },
	{ "LIN_R8G8B8X8", GPU_TEXTURE_LIN_R8G8B8X8, D3DFMT_UNKNOWN },
	{ "LIN_X32", GPU_TEXTURE_LIN_X32, D3DFMT_INDEX32 },
	{ "LIN_Y16X16", GPU_TEXTURE_LIN_Y16X16, D3DFMT_UNKNOWN },
	{ "LIN_Y16X16F", GPU_TEXTURE_LIN_Y16X16F, D3DFMT_UNKNOWN },
	{ "LIN_D24S8", GPU_TEXTURE_LIN_D24S8, D3DFMT_D24S8 },
	{ "LIN_D24FS8", GPU_TEXTURE_LIN_D24FS8, D3DFMT_D24FS8 },
	{ "A8R8G8B8", GPU_TEXTURE_A8R8G8B8, D3DFMT_A8R8G8B8 },
	{ "X8R8G8B8", GPU_TEXTURE_X8R8G8B8, D3DFMT_X8R8G8B8 },
	{ "A8B8G8R8", GPU_TEXTURE_A8B8G8R8, D3DFMT_A8B8G8R8 },
	{ "X8B8G8R8", GPU_TEXTURE_X8B8G8R8, D3DFMT_X8B8G8R8 },
	{ "R8G8B8A8", GPU_TEXTURE_R8G8B8A8, D3DFMT_UNKNOWN },
	{ "R8G8B8X8", GPU_TEXTURE_R8G8B8X8, D3DFMT_UNKNOWN },
	{ "X32", GPU_TEXTURE_X32, D3DFMT_INDEX32 },
	{ "Y16X16", GPU_TEXTURE_Y16X16, D3DFMT_UNKNOWN },
	{ "Y16X16F", GPU_TEXTURE_Y16X16F, D3DFMT_UNKNOWN },
	{ "D24S8", GPU_TEXTURE_D24S8, D3DFMT_D24S8 },
	{ "D24FS8", GPU_TEXTURE_D24FS8, D3DFMT_D24FS8 },
	{ "LIN_B16G16R16A16F", GPU_TEXTURE_LIN_B16G16R16A16F, D3DFMT_A16B16G16R16F },
	{ "B16G16R16A16F", GPU_TEXTURE_B16G16R16A16F, D3DFMT_A16B16G16R16F },
	{ "LIN_B32G32R32A32F", GPU_TEXTURE_LIN_B32G32R32A32F, D3DFMT_A32B32G32R32F },
	{ "B32G32R32A32F", GPU_TEXTURE_B32G32R32A32F, D3DFMT_A32B32G32R32F },
};

uint GetGCMFormat( const char* id )
{
	for (size_t i = 0; i < tl_elementsof(Ps3TexelFormats); i++)
		if (_stricmp(id, Ps3TexelFormats[i].name) == 0)
			return Ps3TexelFormats[i].format;
	assert( false );
	return 0;
}

const char* GetGCMFormatName( uint format )
{
	for (size_t i = 0; i < tl_elementsof(Ps3TexelFormats); i++)
		if (Ps3TexelFormats[i].format == format)
			return Ps3TexelFormats[i].name;
	assert( false );
	return "invalid";
}

D3DFORMAT GetD3DFormat( gpuTextureFormat format )
{
	for (size_t i = 0; i < tl_elementsof(Ps3TexelFormats); i++)
		if (Ps3TexelFormats[i].format == format)
			return Ps3TexelFormats[i].d3d_format;
	assert( false );
	return D3DFMT_UNKNOWN;
}

bool GCMIsTextureFormatLinear(gpuTextureFormat aFormat)
{
	return (GPU_TEXTURE_FORMAT_FORMAT(aFormat) & CELL_GCM_TEXTURE_LN) != 0;
}

bool GCMIsTextureFormatSwizzled(gpuTextureFormat aFormat)
{
	return (GPU_TEXTURE_FORMAT_FORMAT(aFormat) & CELL_GCM_TEXTURE_LN) == 0;
}

bool GCMIsTextureFormatDXT(gpuTextureFormat aFormat)
{
	return (GPU_TEXTURE_FORMAT_PIXELFORMAT(aFormat) == CELL_GCM_TEXTURE_COMPRESSED_DXT1) || 
	       (GPU_TEXTURE_FORMAT_PIXELFORMAT(aFormat) == CELL_GCM_TEXTURE_COMPRESSED_DXT23) ||
	       (GPU_TEXTURE_FORMAT_PIXELFORMAT(aFormat) == CELL_GCM_TEXTURE_COMPRESSED_DXT45);
}

uint GCMGetTextureFormatBPP(gpuTextureFormat aFormat)
{
	switch (GPU_TEXTURE_FORMAT_PIXELFORMAT(aFormat))
	{
	case CELL_GCM_TEXTURE_B8:					return 1;
	case CELL_GCM_TEXTURE_A1R5G5B5:				return 2;
	case CELL_GCM_TEXTURE_A4R4G4B4:				return 2;
	case CELL_GCM_TEXTURE_R5G6B5:				return 2;
	case CELL_GCM_TEXTURE_A8R8G8B8:				return 4;
	case CELL_GCM_TEXTURE_COMPRESSED_DXT1:		return 8;
	case CELL_GCM_TEXTURE_COMPRESSED_DXT23:		return 16;
	case CELL_GCM_TEXTURE_COMPRESSED_DXT45:		return 16;
	case CELL_GCM_TEXTURE_G8B8:					return 2;
	case CELL_GCM_TEXTURE_R6G5B5:				return 2;
	case CELL_GCM_TEXTURE_DEPTH24_D8:			return 4;
	case CELL_GCM_TEXTURE_DEPTH24_D8_FLOAT:		return 4;
	case CELL_GCM_TEXTURE_DEPTH16:				return 2;
	case CELL_GCM_TEXTURE_DEPTH16_FLOAT:		return 2;
	case CELL_GCM_TEXTURE_X16:					return 2;
	case CELL_GCM_TEXTURE_Y16_X16:				return 4;
	case CELL_GCM_TEXTURE_R5G5B5A1:				return 2;
	case CELL_GCM_TEXTURE_COMPRESSED_HILO8:		return 2;
	case CELL_GCM_TEXTURE_COMPRESSED_HILO_S8:	return 2;
	case CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT: return 8;
	case CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT: return 16;
	case CELL_GCM_TEXTURE_X32_FLOAT:			return 4;
	case CELL_GCM_TEXTURE_D1R5G5B5:				return 2;
	case CELL_GCM_TEXTURE_D8R8G8B8:				return 4;
	case CELL_GCM_TEXTURE_Y16_X16_FLOAT:		return 4;
	default: return 0;
	};
}

// Depth and 16bit channel formats are considered float even if they are not actually floating point data,
//  since they are converted from/to floating point intermediate formats when processing.
bool GCMIsTextureFormatFloat(gpuTextureFormat aFormat)
{
	switch (GPU_TEXTURE_FORMAT_PIXELFORMAT(aFormat))
	{
	case CELL_GCM_TEXTURE_B8:					return false;
	case CELL_GCM_TEXTURE_A1R5G5B5:				return false;
	case CELL_GCM_TEXTURE_A4R4G4B4:				return false;
	case CELL_GCM_TEXTURE_R5G6B5:				return false;
	case CELL_GCM_TEXTURE_A8R8G8B8:				return false;
	case CELL_GCM_TEXTURE_COMPRESSED_DXT1:		return false;
	case CELL_GCM_TEXTURE_COMPRESSED_DXT23:		return false;
	case CELL_GCM_TEXTURE_COMPRESSED_DXT45:		return false;
	case CELL_GCM_TEXTURE_G8B8:					return false;
	case CELL_GCM_TEXTURE_R6G5B5:				return false;
	case CELL_GCM_TEXTURE_DEPTH24_D8:			return true;
	case CELL_GCM_TEXTURE_DEPTH24_D8_FLOAT:		return true;
	case CELL_GCM_TEXTURE_DEPTH16:				return true;
	case CELL_GCM_TEXTURE_DEPTH16_FLOAT:		return true;
	case CELL_GCM_TEXTURE_X16:					return true;
	case CELL_GCM_TEXTURE_Y16_X16:				return true;
	case CELL_GCM_TEXTURE_R5G5B5A1:				return false;
	case CELL_GCM_TEXTURE_COMPRESSED_HILO8:		return false;
	case CELL_GCM_TEXTURE_COMPRESSED_HILO_S8:	return false;
	case CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT: return true;
	case CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT: return true;
	case CELL_GCM_TEXTURE_X32_FLOAT:			return true;
	case CELL_GCM_TEXTURE_D1R5G5B5:				return false;
	case CELL_GCM_TEXTURE_D8R8G8B8:				return false;
	case CELL_GCM_TEXTURE_Y16_X16_FLOAT:		return true;
	default: return 0;
	};
}

size_t GCMCalcTextureSize(gpuTextureType type, size_t width, size_t height, size_t depth, size_t levels, gpuTextureFormat format)
{
	bool sz = GCMIsTextureFormatSwizzled(format);
	size_t bpp = GCMGetTextureFormatBPP(format);

	if (GCMIsTextureFormatDXT(format))
	{
		width = tl_max(1, (width+3)/4);
		height = tl_max(1, (height+3)/4);
	}

	size_t size = 0;
	for (size_t d = 0; d < depth; d++)
	{
		uint w = width;
		uint h = height;
		for (uint l = 0; l < levels; l++)
		{
			size += w*h*bpp;
			if (sz) w = tl_max(1, (w+1)/2);
			h = tl_max(1, (h+1)/2);
		}
		if (type == GPU_TEXTURE_CUBE) 
			size = align(size, 128);
	}

	return size;
}

size_t GCMSetTextureHeader(gpuTextureType aType, size_t aWidth, size_t aHeight, size_t aDepth, size_t aLevels, gpuTextureFormat aFormat, CellGcmTexture* tex)
{
	tex->format    = GPU_TEXTURE_FORMAT_FORMAT(aFormat);
	tex->mipmap    = aLevels;
	tex->dimension = aType == GPU_TEXTURE_VOLUME ? CELL_GCM_TEXTURE_DIMENSION_3 : CELL_GCM_TEXTURE_DIMENSION_2;
	tex->cubemap   = aType == GPU_TEXTURE_CUBE;
	tex->remap     = GPU_TEXTURE_FORMAT_REMAP(aFormat);
	tex->width     = aWidth;
	tex->height    = aHeight;
	tex->depth     = aDepth;

	// Pitch is always 0 for swizzled textures.  
	if (GCMIsTextureFormatSwizzled(aFormat))
		tex->pitch = 0;
	else
	{
		// The libgcm Reference release 1.0.1 incorrectly states that pitch should always be 0 for DXTC textures.  The reality is that
		//  pitch should be 0 when a DXTC texture is marked swizzled (eg its dimensions are powers of 2).  When the DXTC texture is marked
		//  linear, the pitch should be the size of one row of 4x4 blocks.
		if (GCMIsTextureFormatDXT(aFormat))
			tex->pitch = GCMGetTextureFormatBPP(aFormat) * tl_max(1, (aWidth+3)/4); 
		else
			tex->pitch = GCMGetTextureFormatBPP(aFormat) * aWidth; 
	}

	tex->location = 0;
	tex->offset = 0;

	return GCMCalcTextureSize(aType, aWidth, aHeight, aDepth, aLevels, aFormat);
}

template<typename T>
T EndianSwap32(T &data)
{
	T out;
	Ut::DoEndianSwap::EndianSwapD(&out, &data);
	return out;
}

template<typename T>
T EndianSwap16(T &data)
{
	T out;
	Ut::DoEndianSwap::EndianSwapW(&out, &data);
	return out;
}

void GCMEndianSwapSurface(void* data, uint width, uint height, uint pitch, gpuTextureFormat aFormat)
{
	u8* dest_data = (u8*)data;

	uint bpp = GCMGetTextureFormatBPP(aFormat);

	for (uint y = 0; y < height; y++)
	{
		for (uint x = 0; x < width; x++)
		{
			u8* dest_ptr = dest_data + y*pitch+x*bpp;
			u16* dest16 = (u16*)dest_ptr;
			u32* dest32 = (u32*)dest_ptr;
			switch (GPU_TEXTURE_FORMAT_PIXELFORMAT(aFormat))
			{
			case CELL_GCM_TEXTURE_B8: 
				break;
			case CELL_GCM_TEXTURE_A1R5G5B5:
			case CELL_GCM_TEXTURE_A4R4G4B4:
			case CELL_GCM_TEXTURE_R5G6B5:
				*dest16 = EndianSwap16(*dest16);
				break;
			case CELL_GCM_TEXTURE_A8R8G8B8:
				*dest32 = EndianSwap32(*dest32);
				break;
			case CELL_GCM_TEXTURE_COMPRESSED_DXT1:
			case CELL_GCM_TEXTURE_COMPRESSED_DXT23:
			case CELL_GCM_TEXTURE_COMPRESSED_DXT45:
				// No endian swapping needed.
				return;
			case CELL_GCM_TEXTURE_G8B8:	
			case CELL_GCM_TEXTURE_R6G5B5:
				*dest16 = EndianSwap16(*dest16);
				break;
			case CELL_GCM_TEXTURE_DEPTH24_D8:
			case CELL_GCM_TEXTURE_DEPTH24_D8_FLOAT:
				*dest32 = EndianSwap32(*dest32);
				break;
			case CELL_GCM_TEXTURE_DEPTH16:
			case CELL_GCM_TEXTURE_DEPTH16_FLOAT:
				*dest16 = EndianSwap16(*dest16);
				break;
			case CELL_GCM_TEXTURE_X16:
				*dest16 = EndianSwap16(*dest16);
				break;
			case CELL_GCM_TEXTURE_Y16_X16:
				dest16[0] = EndianSwap16(dest16[0]);
				dest16[1] = EndianSwap16(dest16[1]);
				break;
			case CELL_GCM_TEXTURE_R5G5B5A1:
				*dest16 = EndianSwap16(*dest16);
				break;
			case CELL_GCM_TEXTURE_COMPRESSED_HILO8:
			case CELL_GCM_TEXTURE_COMPRESSED_HILO_S8:
				assert(false && "HILO textures not supported.");
				break;
			case CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT:
				dest16[0] = EndianSwap16(dest16[0]);
				dest16[1] = EndianSwap16(dest16[1]);
				dest16[2] = EndianSwap16(dest16[2]);
				dest16[3] = EndianSwap16(dest16[3]);
				break;
			case CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT:
				dest32[0] = EndianSwap32(dest32[0]);
				dest32[1] = EndianSwap32(dest32[1]);
				dest32[2] = EndianSwap32(dest32[2]);
				dest32[3] = EndianSwap32(dest32[3]);
				break;
			case CELL_GCM_TEXTURE_X32_FLOAT:
				*dest32 = EndianSwap32(*dest32);
				break;
			case CELL_GCM_TEXTURE_D1R5G5B5:
				*dest16 = EndianSwap16(*dest16);
				break;
			case CELL_GCM_TEXTURE_D8R8G8B8:
				*dest16 = EndianSwap16(*dest16);
				break;
			case CELL_GCM_TEXTURE_Y16_X16_FLOAT:
				dest16[0] = EndianSwap16(dest16[0]);
				dest16[1] = EndianSwap16(dest16[1]);
				break;
			default:
				assert(false && "Unknown format.");
				break;
			}
		}
	}
}

void GCMSwizzleSurface(u8* dest, const u8* src, uint width, uint height, gpuTextureFormat aFormat)
{
	uint bpp = GCMGetTextureFormatBPP(aFormat);

	// 128bit and 64bit formats are swizzled as if they were wide 32bit formats.
	if (bpp > 4)
	{
		width *= bpp/4;
		bpp = 4;
	}

	// Non-square swizzled textures are treated as an array of square swizzled textures.
	uint dimension = tl_min(width, height);
	uint steps = tl_max(width, height) / dimension;

	uint step_offset = 0;
	uint step_x = 0;
	uint step_y = 0;
	for (uint s = 0; s < steps; s++)
	{
		for (uint offset = 0; offset < dimension*dimension; offset++)
		{
			// Extract interleaved x and y source coords from swizzled offset.
			uint x = 0, y = 0;
			for (uint b = 0; b < 16; b++)
			{
				uint xbit = (offset>>(2*b+0))&1;
				uint ybit = (offset>>(2*b+1))&1;
				x |= xbit<<b;
				y |= ybit<<b;
			}
			x += step_x;
			y += step_y;
			assert(x < width && y < height);
			for (uint i = 0; i < bpp; i++)
				dest[step_offset+offset*bpp+i] = src[(y*width+x)*bpp+i];
		}
		step_offset += dimension*dimension*bpp;
		if (dimension == height)
			step_x += dimension;
		else
			step_y += dimension;
	}
	assert(step_offset == width*height*bpp);
}
void GCMUnSwizzleSurface(u8* dest, const u8* src, uint width, uint height, gpuTextureFormat aFormat)
{
	uint bpp = GCMGetTextureFormatBPP(aFormat);

	// 128bit and 64bit formats are swizzled as if they were wide 32bit formats.
	if (bpp > 4)
	{
		width *= bpp/4;
		bpp = 4;
	}

	// Non-square swizzled textures are treated as an array of square swizzled textures.
	uint dimension = tl_min(width, height);
	uint steps = tl_max(width, height) / dimension;

	uint step_offset = 0;
	uint step_x = 0;
	uint step_y = 0;
	for (uint s = 0; s < steps; s++)
	{
		for (uint offset = 0; offset < dimension*dimension; offset++)
		{
			// Extract interleaved x and y source coords from swizzled offset.
			uint x = 0, y = 0;
			for (uint b = 0; b < 16; b++)
			{
				uint xbit = (offset>>(2*b+0))&1;
				uint ybit = (offset>>(2*b+1))&1;
				x |= xbit<<b;
				y |= ybit<<b;
			}
			x += step_x;
			y += step_y;
			assert(x < width && y < height);
			for (uint i = 0; i < bpp; i++)
				dest[(y*width+x)*bpp+i] = src[step_offset+offset*bpp+i];
		}
		step_offset += dimension*dimension*bpp;
		if (dimension == height)
			step_x += dimension;
		else
			step_y += dimension;
	}
	assert(step_offset == width*height*bpp);
}

u32 GCMPackField32(u32 pos, u32 width, u8 data)
{
	uint range = (1<<width)-1;
	u32 scaled = (u32)data * range / 255;
	return scaled << pos;
}

// Note- swizzling is ignored, call GCMSwizzleSurface afterwards if needed.
void GCMCopySurfaceFromA8R8G8B8(void* data, uint width, uint height, gpuTextureFormat aFormat, const void* src)
{
	u32* src_data = (u32*)src;
	u8* dest_data = (u8*)data;

	uint bpp = GCMGetTextureFormatBPP(aFormat);

	for (uint y = 0; y < height; y++)
	{
		for (uint x = 0; x < width; x++)
		{
			u8* dest_ptr = dest_data + (y*width+x)*bpp;
			u8* dest8 = dest_ptr;
			u16* dest16 = (u16*)dest_ptr;
			u32* dest32 = (u32*)dest_ptr;

			u32 texel = src_data[y*width+x];

			u32 remap = GPU_TEXTURE_FORMAT_REMAP(aFormat);
			u8 texel_fields[4] = { 0, 0, 0, 0 };
			texel_fields[(remap>>0)&3] = (texel>>24)&0xff; // a
			texel_fields[(remap>>2)&3] = (texel>>16)&0xff; // r
			texel_fields[(remap>>4)&3] = (texel>> 8)&0xff; // g
			texel_fields[(remap>>6)&3] = (texel>> 0)&0xff; // b

			u8 a = texel_fields[0];
			u8 r = texel_fields[1];
			u8 g = texel_fields[2];
			u8 b = texel_fields[3];

			switch (GPU_TEXTURE_FORMAT_PIXELFORMAT(aFormat))
			{
			case CELL_GCM_TEXTURE_B8: 
				*dest8 = b; 
				break;
			case CELL_GCM_TEXTURE_A1R5G5B5:
				*dest16 = GCMPackField32(15, 1, a) | GCMPackField32(10, 5, r) | GCMPackField32(5, 5, g) | GCMPackField32(0, 5, b);
				break;
			case CELL_GCM_TEXTURE_A4R4G4B4:
				*dest16 = GCMPackField32(12, 4, a) | GCMPackField32(8, 4, r) | GCMPackField32(4, 4, g) | GCMPackField32(0, 4, b);
				break;
			case CELL_GCM_TEXTURE_R5G6B5:
				*dest16 = GCMPackField32(11, 5, r) | GCMPackField32(5, 6, g) | GCMPackField32(0, 5, b);
				break;
			case CELL_GCM_TEXTURE_A8R8G8B8:
				*dest32 = GCMPackField32(24, 8, a) | GCMPackField32(16, 8, r) | GCMPackField32(8, 8, g) | GCMPackField32(0, 8, b);
				break;
			case CELL_GCM_TEXTURE_COMPRESSED_DXT1:
			case CELL_GCM_TEXTURE_COMPRESSED_DXT23:
			case CELL_GCM_TEXTURE_COMPRESSED_DXT45:
				assert(false && "DXT formats not supported.");
				break;
			case CELL_GCM_TEXTURE_G8B8:	
				*dest16 = GCMPackField32(8, 8, g) | GCMPackField32(0, 8, b);
				break;
			case CELL_GCM_TEXTURE_R6G5B5:
				*dest16 = GCMPackField32(10, 6, r) | GCMPackField32(5, 5, g) | GCMPackField32(0, 5, b);
				break;
			case CELL_GCM_TEXTURE_R5G5B5A1:
				*dest16 = GCMPackField32(11, 5, r) | GCMPackField32(6, 5, g) | GCMPackField32(1, 5, b) | GCMPackField32(0, 1, a);
				break;
			case CELL_GCM_TEXTURE_COMPRESSED_HILO8:
			case CELL_GCM_TEXTURE_COMPRESSED_HILO_S8:
				assert(false && "HILO formats not supported.");
				break;
			case CELL_GCM_TEXTURE_D1R5G5B5:
				*dest16 = GCMPackField32(15, 1, 1) | GCMPackField32(10, 5, r) | GCMPackField32(5, 5, g) | GCMPackField32(0, 5, b);
				break;
			case CELL_GCM_TEXTURE_D8R8G8B8:
				*dest32 = GCMPackField32(24, 8, 255) | GCMPackField32(16, 8, r) | GCMPackField32(8, 8, g) | GCMPackField32(0, 8, b);
				break;
			default:
				assert(false && "Format not supported.");
				break;
			}
		}
	}
}

// Note- swizzling is ignored, call GCMSwizzleSurface afterwards if needed.
void GCMCopySurfaceFromA32R32G32B32(void* data, uint width, uint height, gpuTextureFormat aFormat, const void* src)
{
	float* src_data = (float*)src;
	u8* dest_data = (u8*)data;

	uint bpp = GCMGetTextureFormatBPP(aFormat);

	for (uint y = 0; y < height; y++)
	{
		for (uint x = 0; x < width; x++)
		{
			u8* dest_ptr = dest_data + (y*width+x)*bpp;
			u16* dest16 = (u16*)dest_ptr;
			u32* dest32 = (u32*)dest_ptr;
			float* destf = (float*)dest_ptr;

			float texel[4];
			texel[0] = src_data[(y*width+x)*4+0];
			texel[1] = src_data[(y*width+x)*4+1];
			texel[2] = src_data[(y*width+x)*4+2];
			texel[3] = src_data[(y*width+x)*4+3];

			u32 remap = GPU_TEXTURE_FORMAT_REMAP(aFormat);
			float texel_fields[4] = { 0, 0, 0, 0 };
			texel_fields[(remap>>0)&3] = texel[0];
			texel_fields[(remap>>2)&3] = texel[1];
			texel_fields[(remap>>4)&3] = texel[2];
			texel_fields[(remap>>6)&3] = texel[3];

			float a = texel_fields[0];
			float r = texel_fields[1];
			float g = texel_fields[2];
			float b = texel_fields[3];

			switch (GPU_TEXTURE_FORMAT_PIXELFORMAT(aFormat))
			{
			case CELL_GCM_TEXTURE_DEPTH24_D8:
			case CELL_GCM_TEXTURE_DEPTH24_D8_FLOAT:
			case CELL_GCM_TEXTURE_DEPTH16:
			case CELL_GCM_TEXTURE_DEPTH16_FLOAT:
				assert(false && "Depth formats supported.");
				break;
			case CELL_GCM_TEXTURE_X16:
				*dest16 = (int)r;
				break;
			case CELL_GCM_TEXTURE_Y16_X16:
				dest16[0] = (int)r;
				dest16[1] = (int)g;
				break;
			case CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT:
				dest16[0] = Half::F16(r).m_U16;
				dest16[1] = Half::F16(g).m_U16;
				dest16[2] = Half::F16(b).m_U16;
				dest16[3] = Half::F16(a).m_U16;
				break;
			case CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT:
				destf[0] = r;
				destf[1] = g;
				destf[2] = b;
				destf[3] = a;
				break;
			case CELL_GCM_TEXTURE_X32_FLOAT:
				*destf = r;
				break;
			case CELL_GCM_TEXTURE_Y16_X16_FLOAT:
				dest16[0] = Half::F16(r).m_U16;
				dest16[1] = Half::F16(g).m_U16;
				break;
			default:
				assert(false && "Format not supported.");
				break;
			}
		}
	}
}

void GCMCopySurfaceToA8R8G8B8(void* data, uint width, uint height, gpuTextureFormat aFormat, const void* src)
{
	u8* src_data = (u8*)src;
	u32* dest_data = (u32*)data;

	uint bpp = GCMGetTextureFormatBPP(aFormat);

	for (uint y = 0; y < height; y++)
	{
		for (uint x = 0; x < width; x++)
		{
			u8* src_ptr = src_data + (y*width+x)*bpp;
			u8* src8 = src_ptr;
			u16* src16 = (u16*)src_ptr;
			u32* src32 = (u32*)src_ptr;

			u32 remap = GPU_TEXTURE_FORMAT_REMAP(aFormat);
			u8 texel_fields[4] = { 0, 0, 0, 0 };

			switch (GPU_TEXTURE_FORMAT_PIXELFORMAT(aFormat))
			{
			case CELL_GCM_TEXTURE_G8B8: 
			{
				texel_fields[3] = src8[0]; // b
				texel_fields[2] = src8[1]; // g
				texel_fields[1] = 0; // r
				texel_fields[0] = 255; // a
				break;
			}

			case CELL_GCM_TEXTURE_D1R5G5B5:
			case CELL_GCM_TEXTURE_R6G5B5:
			case CELL_GCM_TEXTURE_COMPRESSED_HILO8:
			case CELL_GCM_TEXTURE_COMPRESSED_HILO_S8:
			case CELL_GCM_TEXTURE_DEPTH16_FLOAT:
			case CELL_GCM_TEXTURE_Y16_X16:
			case CELL_GCM_TEXTURE_Y16_X16_FLOAT:

			default:
				assert(false && "Format not supported.");
				break;
			}


			u8* dest8 = (u8*)&dest_data[y*width+x];

			u8 a = texel_fields[(remap>>0)&3]; // a
			u8 r = texel_fields[(remap>>2)&3]; // r
			u8 g = texel_fields[(remap>>4)&3]; // g
			u8 b = texel_fields[(remap>>6)&3]; // b

			dest8[0] = b;
			dest8[1] = g;
			dest8[2] = r;
			dest8[3] = a;

		}
	}
}

size_t GetSurfaceBytes( gpuTextureFormat aFormat, size_t aWidth, size_t aHeight )
{
	if (GCMIsTextureFormatDXT(aFormat))
	{
		aWidth = tl_max(1, (aWidth+3)/4);
		aHeight = tl_max(1, (aHeight+3)/4);
	}

	return aWidth * aHeight * GCMGetTextureFormatBPP(aFormat); 
}

IDirect3D9* thePs3D3DObject = NULL;
IDirect3DDevice9* thePs3D3DDevice = NULL;

void InitPs3D3DObjectDevice()
{
	// Initialize Direct3d
	if ( !thePs3D3DObject )
		thePs3D3DObject = ::Direct3DCreate9(D3D_SDK_VERSION);

	if ( !thePs3D3DDevice )
	{
		D3DPRESENT_PARAMETERS params;
		ZeroMemory( &params, sizeof(params));
		params.BackBufferFormat = D3DFMT_UNKNOWN;
		params.BackBufferCount = 1;
		params.Windowed = TRUE;
		params.SwapEffect=D3DSWAPEFFECT_COPY;
		// For some reason CreateDevice does not like it when there is no focus window
		// focus window, so we have to create a nice little phony one.
		HWND hW = CreateWindow( "Button", "", 0, 0, 0, 1, 1, NULL, 0, 0, 0 );
		HRESULT err = thePs3D3DObject->CreateDevice( D3DADAPTER_DEFAULT,
			                                         D3DDEVTYPE_NULLREF,
													 hW,
													 D3DCREATE_SOFTWARE_VERTEXPROCESSING,
													 &params,
													 &thePs3D3DDevice );
		DestroyWindow( hW );
	}
}

IDirect3DDevice9* GetPs3D3DDevice() 
{
	InitPs3D3DObjectDevice();
	return thePs3D3DDevice;
}

class Ps3TexelFormat : public TexelFormat
{
public:
	gpuTextureFormat format;
	const char* id;
	NglPs3Tex* driver;

	Ps3TexelFormat( NglPs3Tex* _driver, uint _format, const char* _id ) 
		: driver(_driver), format((gpuTextureFormat)_format), id(_id) 
	{
	}

	void CopyFromA8R8G8B8( Surface* aDstSurface, const Surface* aSrcSurface ) const
	{
		assert(GetClass() == CLASS_INTEGER && "Only supported for integer formats.");
		GCMCopySurfaceFromA8R8G8B8(aDstSurface->GetTexels(), aDstSurface->GetWidth(), aDstSurface->GetHeight(), format, aSrcSurface->GetTexels());
	}
	void CopyToA8R8G8B8( Surface* aDstSurface, const Surface* aSrcSurface ) const
	{
		assert(GetClass() == CLASS_INTEGER && "Only supported for integer formats.");

		D3DFORMAT d3dformat = GetD3DFormat(format);
		if( D3DFMT_UNKNOWN == d3dformat )
		{
			GCMCopySurfaceToA8R8G8B8(aDstSurface->GetTexels(), aDstSurface->GetWidth(), aDstSurface->GetHeight(), format, aSrcSurface->GetTexels() );
		}
		else
		{
			// format is known to d3d, let it handle the conversion

			IDirect3DTexture9* DstTex;
			GetPs3D3DDevice()->CreateTexture((UINT)aSrcSurface->GetWidth(), (UINT)aSrcSurface->GetHeight(), 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &DstTex, NULL );
			IDirect3DSurface9* DstSurf;
			DstTex->GetSurfaceLevel( 0, &DstSurf );

			RECT r = { 0, 0, aSrcSurface->GetWidth(), aSrcSurface->GetHeight() };
			UINT srcPitch = (UINT)GetSurfaceBytes(format, aSrcSurface->GetWidth(), 1);
			HRESULT err = D3DXLoadSurfaceFromMemory(
				DstSurf, NULL, NULL, 
				aSrcSurface->GetTexels(), d3dformat, srcPitch, NULL, &r,
				D3DX_DEFAULT, 0 );

			D3DLOCKED_RECT lock;
			DstSurf->LockRect( &lock, NULL, 0 );
			size_t new_size = 4*aDstSurface->GetWidth()*aDstSurface->GetHeight();
			aDstSurface->Resize( new_size );
			memcpy( aDstSurface->GetTexels(), lock.pBits, new_size );
			DstSurf->UnlockRect();

			DstSurf->Release();
			DstTex->Release();
		}
	}

	void CopyFromA32R32G32B32F( Surface* aDstSurface, const Surface* aSrcSurface ) const
	{
		assert(GetClass() == CLASS_FLOAT && "Only supported for floating point formats.");
		GCMCopySurfaceFromA32R32G32B32(aDstSurface->GetTexels(), aDstSurface->GetWidth(), aDstSurface->GetHeight(), format, aSrcSurface->GetTexels());
	}
	void CopyToA32R32G32B32F( Surface* aDstSurface, const Surface* aSrcSurface ) const
	{
		assert(GetClass() == CLASS_FLOAT && "Only supported for floating point formats.");
		// todo
		assert(false);
	}

	size_t Bytes(size_t w, size_t h) const 
	{ 
		return GetSurfaceBytes( format, w, h ); 
	}

	const char* GetId() const { return id; }
	TexelFormat* Clone() const { return new Ps3TexelFormat(driver, format, id); }

	Class GetClass() const 
	{ 
		return GCMIsTextureFormatFloat(format) ? CLASS_FLOAT : CLASS_INTEGER;
	}
};

TexelFormat* NglPs3Tex::GetTexelFormat( int format )
{
	for (size_t i = 0; i < tl_elementsof(Ps3TexelFormats); i++)
		if (Ps3TexelFormats[i].format == format)
			return new Ps3TexelFormat( NULL, Ps3TexelFormats[i].format, Ps3TexelFormats[i].name );
	return NULL;
}

TexelFormat* NglPs3Tex::GetTexelFormat( const char* format )
{
	for (size_t i = 0; i < tl_elementsof(Ps3TexelFormats); i++)
		if (_stricmp(Ps3TexelFormats[i].name, format) == 0)
			return new Ps3TexelFormat( NULL, Ps3TexelFormats[i].format, Ps3TexelFormats[i].name );
	return NULL;
}

template<typename T>
T &Serialise(T &ar, const CellGcmTexture &t)
{
	ar << t.width << t.height << t.mipmap << t.format << t.dimension << t.cubemap << t.remap << t.depth << t.pitch << t.offset;
	return ar;
}

template<typename T>
T &Deserialise(T &ar, CellGcmTexture &t)
{
	ar >> t.width >> t.height >> t.mipmap >> t.format >> t.dimension >> t.cubemap >> t.remap >> t.depth >> t.pitch >> t.offset;
	return ar;
}

ACTRESULT NglPs3Tex::SerialOutTexture( Sys::I_BinaryStream *pStream, const Texture* aTexture, OutputType outputType ) 
{
	Texture tmpTexture;
	Sys::BinarySerialiser<Ut::NativeToBigEndian> ar(*pStream);

	/*if (Texture::ANIMATED == aTexture->GetType())
	{	
		//return SerialOutAnimTexture( aPackFile, aTexture, NGL_APK_TEXTURE_VERSION, IFL_PLAT_PS3 | IFL_ENDIANSWAP );
		return ACT_ERROR;
	}*/

	// Volume textures are not yet supported by this module.  Their layout is similar to cubemaps, with the exception that
	//  all layers for the top mip are written, followed by all layers for the next mip, etc.  The 128byte alignment needs 
	//  are also slightly different.
	// And then, there is VTC (volume texture compression), which is basically DXT layers that are interleaved 4 at a time.
	if (Texture::VOLUMETEXTURE == aTexture->GetType())
	{	
		LOGERROR( TextureExport::g_Log, "Volume textures are not yet supported by this module!");
		return ACT_ERROR;
	}

	gpuTextureFormat GCMFormat;
	if ( !_stricmp( "R8G8B8", aTexture->GetFormat()->GetId() ) )
	{
		// 2006.10.18 KGT: a 24bit R8G8B8 is not supported on Xenon.
		//                 to handle properly DDS or other 24bpp RGB texture we will convert the surfaces to A8R8G8B8 and
		//                 serailize them as a 32bpp D3DFMT_X8R8G8B8 Xenon format
		tmpTexture.CopyFrom( *aTexture );
		ActTools::A8R8G8B8 a8r8g8b8;
		tmpTexture.ConvertTo( &a8r8g8b8 );

		aTexture = &tmpTexture;
		GCMFormat = GPU_TEXTURE_X8R8G8B8;
	}
	else
	{
		GCMFormat = (gpuTextureFormat)GetGCMFormat( aTexture->GetFormat()->GetId() );
	}

	// DXT textures are neither swizzled or linear.  Their data is always laid out as linear, but the texture format flag depends solely on
	// whether or not they are power of 2.  Also, the pitch is always 0.
	if (GCMIsTextureFormatDXT(GCMFormat))
	{
		if (IsPow2(aTexture->GetWidth()) && IsPow2(aTexture->GetHeight()))
			GCMFormat = GPU_TEXTURE_FORMAT(GPU_TEXTURE_FORMAT_PIXELFORMAT(GCMFormat), GPU_TEXTURE_FORMAT_REMAP(GCMFormat));
		else
			GCMFormat = GPU_TEXTURE_FORMAT(GPU_TEXTURE_FORMAT_PIXELFORMAT(GCMFormat)|CELL_GCM_TEXTURE_LN, GPU_TEXTURE_FORMAT_REMAP(GCMFormat));
	}

	if (GCMIsTextureFormatSwizzled(GCMFormat))
	{
		//tex->Flags |= NGLTEX_SWIZZLED;
		if (!IsPow2(aTexture->GetWidth()) || !IsPow2(aTexture->GetHeight()))
		{
			LOGERROR( TextureExport::g_Log, "Swizzled textures must have power of 2 dimensions.  Texture will not be swizzled.\n");
			GCMFormat = GPU_TEXTURE_FORMAT(GPU_TEXTURE_FORMAT_PIXELFORMAT(GCMFormat)|CELL_GCM_TEXTURE_LN, GPU_TEXTURE_FORMAT_REMAP(GCMFormat));
			//tex->Flags &= ~NGLTEX_SWIZZLED;
		}
	}

	int flags = 0;
	CellGcmTexture tex;

	// Fill in the texture header.
	unsigned int num_faces=0;
	size_t size = 0;
	if ( Texture::TEXTURE == aTexture->GetType() )
	{
		size = GCMSetTextureHeader(GPU_TEXTURE_2D, aTexture->GetWidth(), aTexture->GetHeight(), aTexture->GetDepth(), aTexture->GetNumMipMaps(), GCMFormat, &tex);
		num_faces = 1;
	}
	else if ( Texture::CUBETEXTURE == aTexture->GetType() )
	{
		size = GCMSetTextureHeader(GPU_TEXTURE_CUBE, aTexture->GetWidth(), aTexture->GetHeight(), aTexture->GetDepth(), aTexture->GetNumMipMaps(), GCMFormat, &tex);
		num_faces = 6;
		flags |= NGLTEX_CUBE;
	}
	else if ( Texture::VOLUMETEXTURE == aTexture->GetType() )
	{
		size = GCMSetTextureHeader(GPU_TEXTURE_VOLUME, aTexture->GetWidth(), aTexture->GetHeight(), aTexture->GetDepth(), aTexture->GetNumMipMaps(), GCMFormat, &tex);
		num_faces = aTexture->GetDepth();
		flags |= NGLTEX_VOLUME;
	}
	else
	{
		LOGERROR( TextureExport::g_Log, "Unknown texture type!");
		return ACT_ERROR;
	}

	// TODO: save out CellGcmTexture tex;

	size_t bpp = GCMGetTextureFormatBPP(GCMFormat);
	u8* tmp_buffer = (u8*)BizAlignedMalloc(size, 128);
	u8* dest_ptr = tmp_buffer;
	U8* pRawTextureData = dest_ptr;

	for (size_t d = 0; d < aTexture->GetDepth(); d++)
	{
		size_t pitch_width = aTexture->GetWidth();

		size_t width = aTexture->GetWidth();
		size_t height = aTexture->GetHeight();

		if (GCMIsTextureFormatDXT(GCMFormat))
		{
			pitch_width =  tl_max(1, (pitch_width+3)/4);
			width = tl_max(1, (width+3)/4);
			height = tl_max(1, (height+3)/4);
		}

		for (size_t l = 0; l < aTexture->GetNumMipMaps(); l++)
		{
			const u8* src_ptr = aTexture->GetMipmap(d, l)->GetTexels();
			u8* mip_ptr = dest_ptr;

			size_t pitch = pitch_width * bpp;

			// DXT textures never have their data swizzled, even though their format is marked as swizzled if they are 
			//  a power of 2 in both dimensions.
			if (GCMIsTextureFormatSwizzled(GCMFormat) && !GCMIsTextureFormatDXT(GCMFormat))
			{
				GCMSwizzleSurface(mip_ptr, src_ptr, width, height, GCMFormat);
				dest_ptr += width * height * bpp;
			}
			else
			{
				size_t bytes = width * bpp;
				for (size_t y = 0; y < height; y++)
				{
					memcpy(dest_ptr, src_ptr, bytes);
					src_ptr += bytes;
					dest_ptr += pitch;
				}
			}

			GCMEndianSwapSurface(mip_ptr, width, height, pitch, GCMFormat);

			// For linear textures, each level has the same pitch as the base level (so pitch_width stays constant).  
			// For swizzled textures, the pitch decreases with the width.
			if (GCMIsTextureFormatSwizzled(GCMFormat))
				pitch_width = tl_max(1, (pitch_width+1)/2);

			width = tl_max(1, (width+1)/2);
			height = tl_max(1, (height+1)/2);
		}

		// Cubemaps have 128 byte alignment between levels.
		if (flags & NGLTEX_CUBE)
			dest_ptr = tl_align(dest_ptr, 128);
	}

	if (outputType != JUST_DATA)
	{
		ar << tex;
		ar << (U32)size;
	}

	if (outputType != JUST_HEADER)
		ar.Write(pRawTextureData, size);

	BizAlignedFree(tmp_buffer);

	return ACT_OK;
}

ACTRESULT NglPs3Tex::SerialInTexture( ActTools::Texture* aTexture, Sys::I_BinaryStream* pStream )
{
	Sys::BinaryDeserialiser<Ut::BigEndianToNative> ar(*pStream);

	ACTRESULT err = ACT_OK;

	// read the header
	CellGcmTexture header;
	ar >> header;

	size_t num_mipmaps = header.mipmap;
	gpuTextureFormat GCMFormat = GPU_TEXTURE_FORMAT(header.format, header.remap);
	ActTools::TexelFormat* format = GetTexelFormat( GCMFormat );

	assert( aTexture );
	if( header.dimension == CELL_GCM_TEXTURE_DIMENSION_2 )
		aTexture->CreateTexture( format, header.width, header.height, num_mipmaps );
	else if( header.depth == 6 && header.width == header.height )
		aTexture->CreateCubeTexture( format, header.width, num_mipmaps );
	else
		aTexture->CreateVolumeTexture( format, header.width, header.height, header.depth, num_mipmaps );

	// read the texture data
	int size;
	ar >> size;

	U8 *readbuffer = new U8[size];
	u8* src = readbuffer;
	ar.Read(src, size);

	size_t bpp = GCMGetTextureFormatBPP(GCMFormat);
	for( size_t d = 0; d < aTexture->GetDepth(); d++ )
	{
		size_t pitch_width = aTexture->GetWidth();

		size_t width = aTexture->GetWidth();
		size_t height = aTexture->GetHeight();

		if( GCMIsTextureFormatDXT(GCMFormat))
		{
			pitch_width =  tl_max(1, (pitch_width+3)/4);
			width = tl_max(1, (width+3)/4);
			height = tl_max(1, (height+3)/4);
		}

		for( size_t l = 0; l < aTexture->GetNumMipMaps(); l++ )
		{
			ActTools::Surface* surface = aTexture->GetMipmap( d, l );
			surface->Create( aTexture->GetWidth() >> l, aTexture->GetHeight() >> l );
			u8* dst = (U8*)aTexture->GetMipmap(d, l)->GetTexels();
			u8* mip_ptr = dst;

			size_t pitch = pitch_width * bpp;

			if( GCMIsTextureFormatSwizzled(GCMFormat) && !GCMIsTextureFormatDXT(GCMFormat))
			{
				GCMUnSwizzleSurface( mip_ptr, src, width, height, GCMFormat );
				src += width * height * bpp;
			}
			else
			{
				size_t bytes = width * bpp;
				for( size_t y = 0; y < height; y++ )
				{
					memcpy( dst, src, bytes );
					dst += bytes;
					src += pitch;
				}
			}

			GCMEndianSwapSurface(mip_ptr, width, height, pitch, GCMFormat);

			// For linear textures, each level has the same pitch as the base level (so pitch_width stays constant).  
			// For swizzled textures, the pitch decreases with the width.
			if (GCMIsTextureFormatSwizzled(GCMFormat))
				pitch_width = tl_max(1, (pitch_width+1)/2);

			width = tl_max(1, (width+1)/2);
			height = tl_max(1, (height+1)/2);
		}

		// Cubemaps have 128 byte alignment between levels.
//		if( ngl_texture->Flags & NGLTEX_CUBE )
//			dst = tl_align( dst, 128 );
	}

	delete readbuffer;

	return err;
}


