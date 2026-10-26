#include "StdAfx.h"
#include "Display2/Context/GCMContextSPU.h"

#if DONT_INLINE_DISPLAY2
#undef INLINE
#define INLINE
#endif

#if DONT_INLINE_DISPLAY2
#include "Display2/Context/GCMContextSPU.inl"
#endif

namespace Display2
{

#if DONT_INLINE_DISPLAY2

#define GPU_CLASS C_Context
#define GPU_CHECK 1
#define GPU_TRACK 1
#include "Display2/Context/GCMFunctions.inl"
#undef GPU_TRACK
#undef GPU_CHECK
#undef GPU_CLASS

#define GPU_CLASS C_ContextNoCheck
#define GPU_TRACK 1
#include "Display2/Context/GCMFunctions.inl"
#undef GPU_TRACK
#undef GPU_CLASS

#define GPU_CLASS C_ContextNoCheckNoTrack
#include "Display2/Context/GCMFunctions.inl"
#undef GPU_CLASS

#define GPU_CLASS C_ContextNoCheckNoTrackMeasure
#define GPU_MEASURE 1
#include "Display2/Context/GCMFunctions.inl"
#undef GPU_MEASURE
#undef GPU_CLASS

#define GPU_CLASS C_ContextNoCheckNoTrackUnsafeInline
#define GPU_UNSAFE 1
#define GPU_INLINE 1
#include "Display2/Context/GCMFunctions.inl"
#undef GPU_INLINE
#undef GPU_UNSAFE
#undef GPU_CLASS

#define GPU_CLASS C_ContextNoCheckNoTrackUnsafe
#define GPU_UNSAFE 1
#include "Display2/Context/GCMFunctions.inl"
#undef GPU_UNSAFE
#undef GPU_CLASS

#endif

CellGcmConfig g_gcmConfig;

#if USE_GPU_TIMINGS2
U32 g_FreeTiming = 0;
U32 g_LastTiming = 0;
U32 g_ReadTiming = 1;
U32 g_WriteTiming = 0;
U32 g_HeadTiming[g_NumTimings] = {0, 0, 0};
U32 g_TailTiming[g_NumTimings] = {0, 0, 0};
U32 g_TimingKeys[E_Report_TimingCount / 2];
#endif

bool IsMainAddress(void* pAddress)
{
	return ((U32)pAddress >= (U32)g_gcmConfig.localAddress) && ((U32)pAddress <= (((U32)g_gcmConfig.localAddress)+g_gcmConfig.localSize)) ? false : true;
}

bool IsLocalAddress(void* pAddress)
{
	return ((U32)pAddress >= (U32)g_gcmConfig.localAddress) && ((U32)pAddress <= (((U32)g_gcmConfig.localAddress)+g_gcmConfig.localSize)) ? true : false;
}

bool IsMainOffset(const U32 offset)
{
	return offset < 1024*1024*50 ? true : false; // Assume no greater than 50mb...
}

bool IsLocalOffset(const U32 offset)
{
	return offset < 1024*1024*256 ? true : false; // Assume no greater than 256mb...
}

void* OffsetToAddress(U32 location, U32 offset)
{
	BIZ_ASSERT(location == CELL_GCM_LOCATION_MAIN || location == CELL_GCM_LOCATION_LOCAL);

	BIZ_ASSERT(offset <= 1024*1024*512);
	
	if(location == CELL_GCM_LOCATION_MAIN)
	{
		void* pAddress = NULL;

		#if PLATFORM_SPU
		BIZ_ASSERT_FAIL("cellGcmIoOffsetToAddress cannot be called from spu");
		#else
		int32_t result = cellGcmIoOffsetToAddress(offset, &pAddress);
		BIZ_ASSERT_MSG(result == CELL_OK, "cellGcmIoOffsetToAddress failed for offset %i", offset);
		#endif

		BIZ_ASSERT(pAddress);

		return pAddress;
	}
	else
	{
		BIZ_ASSERT(g_gcmConfig.localAddress);

		return (void*)((U32)g_gcmConfig.localAddress + offset);		
	}
}

U32 GetSurfacePitchFromWidth(U32 width, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample)
{
	U32 pitch = 0;

	switch(format)
	{
	case E_SurfaceFormat_L8:
		pitch = width * 1;
		break;
	case E_SurfaceFormat_D16:
		pitch = width * 2;
		break;
	case E_SurfaceFormat_A8R8G8B8:
	case E_SurfaceFormat_D24S8:
	case E_SurfaceFormat_R32F:
		pitch = width * 4;
		break;
	case E_SurfaceFormat_A16B16G16R16F:
		pitch = width * 8;
		break;
	case E_SurfaceFormat_A32B32G32R32F:
		pitch = width * 16;
		break;
	default:
		BIZ_ASSERT(0 && "Unsupported surface format");
		break;
	}

	switch(multisample)
	{
	case E_Multisample_None:
		break;
	case E_Multisample_2:
	case E_Multisample_4:
		pitch = pitch * 2;
		break;
	default:
		BIZ_ASSERT(0 && "Unsupported surface multisample");
		break;
	}

	#if PLATFORM_SPU
	BIZ_ASSERT(!(flags & E_Flags_Tiled));
	#else
	if(flags & E_Flags_Tiled)
	{
		pitch = cellGcmGetTiledPitchSize(pitch);
	}
	else
	#endif
	if(!(flags & E_Flags_Raw))
	{
		pitch = ALIGN_SIZE(pitch, 64);
	}

	return pitch;
}

U32 GetSurfaceSizeFromHeightAndPitch(U32 height, const U32 pitch, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample)
{
	switch(multisample)
	{
	case E_Multisample_None:
	case E_Multisample_2:
		break;
	case E_Multisample_4:
		height = height * 2;
		break;
	default:
		BIZ_ASSERT(0 && "Unsupported surface multisample");
		break;
	}
	
	U32 size = 0;

	#if PLATFORM_SPU
	BIZ_ASSERT(!(flags & E_Flags_Tiled));
	#else
	if(flags & E_Flags_Tiled)
	{
		if(flags & E_Flags_System)
			height = ALIGN_SIZE(height, 64);		/* align to 64 lines for main memory */
		else
			height = ALIGN_SIZE(height, 32);		/* align to 32 lines for local memory */

		size = pitch * height;
		size = ALIGN_SIZE(size, 65536);
	}
	else
		size = pitch * height;
	#endif

	return size;
}

U32 GetSurfaceSize(const U32 width, const U32 height, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample)
{
	const U32 pitch = GetSurfacePitchFromWidth(width, flags, format, multisample);
	return GetSurfaceSizeFromHeightAndPitch(height, pitch, flags, format, multisample);
}

void GetSurfaceSizeAndPitch(const U32 width, const U32 height, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample, U32& size, U32& pitch)
{
	pitch = GetSurfacePitchFromWidth(width, flags, format, multisample);
	size = GetSurfaceSizeFromHeightAndPitch(height, pitch, flags, format, multisample);
}

U32 GetTexturePitchFromWidth(U32 width, const U32 flags, const E_TextureFormat format)
{
	U32 pitch = 0;

	switch(format)
	{
	case E_TextureFormat_DXT1:
		pitch = ((width + 3) / 4) * 8;
		break;
	case E_TextureFormat_DXT3:
	case E_TextureFormat_DXT5:
		pitch = ((width + 3) / 4) * 16;	
		break;
	case E_TextureFormat_L8:
		pitch = width * 1;
		break;
	case E_TextureFormat_D16:
		pitch = width * 2;
		break;
	case E_TextureFormat_A8R8G8B8:
	case E_TextureFormat_D24S8:
	case E_TextureFormat_R32F:
		pitch = width * 4;
		break;
	case E_TextureFormat_A16B16G16R16F:
		pitch = width * 8;
		break;
	case E_TextureFormat_A32B32G32R32F:
		pitch = width * 16;
		break;
	default:
		BIZ_ASSERT(0 && "Unsupported texture format");
		break;
	}

	#if PLATFORM_SPU
	BIZ_ASSERT(!(flags & E_Flags_Tiled));
	#else
	if(flags & E_Flags_Tiled)
	{
		pitch = cellGcmGetTiledPitchSize(pitch);
	}
	else
	#endif
	if(!(flags & E_Flags_Raw))
	{
		pitch = ALIGN_SIZE(pitch, 64);
	}

	return pitch;
}

U32 GetTextureSizeFromHeightAndPitch(U32 height, const U32 pitch, const U32 flags, const E_TextureFormat format)
{
	U32 alignedPitch = pitch;

	if((flags & E_Flags_Colour) || (flags & E_Flags_DepthStencil))
	{
		alignedPitch = ALIGN_SIZE(pitch, 64);
	}

	switch(format)
	{
	case E_TextureFormat_DXT1:
	case E_TextureFormat_DXT3:
	case E_TextureFormat_DXT5:
		height = ((height + 3) / 4);
		break;
	default:
		break;
	}

	U32 size = 0;

	#if PLATFORM_SPU
	BIZ_ASSERT(!(flags & E_Flags_Tiled));
	#else
	if(flags & E_Flags_Tiled)
	{
		if(flags & E_Flags_System)
			height = ALIGN_SIZE(height, 64);		/* align to 64 lines for main memory */
		else
			height = ALIGN_SIZE(height, 32);		/* align to 32 lines for local memory */

		size = alignedPitch * height;
		size = ALIGN_SIZE(size, 65536);
	}
	else
		size = alignedPitch * height;
	#endif

	return size;
}

U32 GetTexturePitch(U32 width, const U32 flags, const E_TextureFormat format)
{
	return GetTexturePitchFromWidth(width, flags, format);
}

U32 GetTextureRowPitch(U32 width, const U32 flags, const E_TextureFormat format)
{
	return GetTexturePitchFromWidth(width, flags, format);
}

U32 GetTextureSlicePitch(U32 width, U32 height, U32 levels, const U32 flags, const E_TextureFormat format)
{
	U32 pitch = GetTexturePitchFromWidth(width, flags, format);
	U32 size = GetTextureSizeFromHeightAndPitch(height, pitch, flags, format);
	return size * levels;
}

U32 GetTexture2DSize(const U32 width, const U32 height, const U32 flags, const E_TextureFormat format)
{
	const U32 pitch = GetTexturePitchFromWidth(width, flags, format);
	return GetTextureSizeFromHeightAndPitch(height, pitch, flags, format);
}

U32 GetTexture2DSize(U32 width, U32 height, const U32 levels, const U32 flags, const E_TextureFormat format)
{
	U32 actualWidth = width;
	U32 size = 0;
	for(U32 level=0; level<levels; level++)
	{
		if(flags & E_Flags_Linear)
			size += GetTexture2DSize(actualWidth, height, flags, format); // Linear textures pad out mip widths!
		else
			size += GetTexture2DSize(width, height, flags, format);
		
		if(width > 1)
			width >>= 1;
		if(height > 1)
			height >>= 1;
	}
	return size;
}

void GetTexture2DSizeAndPitch(const U32 width, const U32 height, const U32 levels, const U32 flags, const E_TextureFormat format, U32& size, U32& pitch)
{
	pitch = GetTexturePitchFromWidth(width, flags, format);
	size = GetTextureSizeFromHeightAndPitch(height, pitch, flags, format);
}

U32 GetTexture3DSize(const U32 width, const U32 height, const U32 depth, const U32 flags, const E_TextureFormat format)
{
	return GetTexture2DSize(width, height, flags, format) * depth;
}

U32 GetTexture3DSize(U32 width, U32 height, const U32 depth, const U32 levels, const U32 flags, const E_TextureFormat format)
{
	U32 size = 0;
	for(U32 level = 0; level < levels; level++)
	{
		size += GetTexture3DSize(width, height, depth, flags, format);
		if(width > 1)
			width >>= 1;
		if(height > 1)
			height >>= 1;
	}
	return size;
}

void GetTexture3DSizeAndPitch(const U32 width, const U32 height, const U32 depth, const U32 levels, const U32 flags, const E_TextureFormat format, U32& size, U32& pitch)
{
	pitch = GetTexturePitchFromWidth(width, flags, format);
	size = GetTexture3DSize(width, height, depth, levels, flags, format);
}

U32 GetTextureCubeSize(const U32 width, const U32 flags, const E_TextureFormat format)
{
	U32 size = 0;
	for(U32 i=0; i<6; i++)
	{
		U32 faceSize = GetTexture2DSize(width, width, flags, format);

		U32 alignedFaceSize = ALIGN_SIZE(faceSize, 128);

		size += alignedFaceSize;
	}
	return size;
}

U32 GetTextureCubeSize(U32 width, const U32 levels, const U32 flags, const E_TextureFormat format)
{
	U32 size = 0;
	for(U32 i=0; i<6; i++)
	{
		U32 faceSize = GetTexture2DSize(width, width, levels, flags, format);

		U32 alignedFaceSize = ALIGN_SIZE(faceSize, 128);

		size += alignedFaceSize;
	}
	return size;
}

void GetTextureCubeSizeAndPitch(const U32 width, const U32 levels, const U32 flags, const E_TextureFormat format, U32& size, U32& pitch)
{
	pitch = GetTexturePitchFromWidth(width, flags, format);
	size = GetTextureCubeSize(width, levels, flags, format);
}

U32 GetTextureWidthFromMultisample(const U32 width, const E_Multisample multisample)
{
	switch(multisample)
	{
	case E_Multisample_2:
	case E_Multisample_4:
		return width*2;
	default:
		return width;
	}
}

U32 GetTextureHeightFromMultisample(const U32 height, const E_Multisample multisample)
{
	switch(multisample)
	{
	case E_Multisample_4:
		return height*2;
	default:
		return height;
	}
}

U32 GetTexture2DLevelOffset(const U32 level, U32 width, U32 height, const U32 levels, const U32 flags, const E_TextureFormat format)
{
	U32 offset = 0;

	for(U32 i=0; i<level; i++)
	{
		if(i >= levels)
			return offset;

		const U32 size = GetTexture2DSize(width, height, flags, format);

		offset += size;

		if(width > 1)
			width /= 2;
		if(height > 1)
			height /= 2;
	}

	return offset;
}

U32 GetTexture3DLevelOffset(const U32 level, U32 width, U32 height, U32 depth, const U32 levels, const U32 flags, const E_TextureFormat format)
{
	U32 offset = 0;

	for(U32 i=0; i<level; i++)
	{
		if(i >= levels)
			return offset;

		const U32 size = GetTexture3DSize(width, height, depth, flags, format);

		offset += size;

		if(width > 1)
			width /= 2;
		if(height > 1)
			height /= 2;
		if(depth > 1)
			depth /= 2;
	}

	return offset;
}

U32 GetTextureCubeLevelOffset(const E_CubeMapFace face, const U32 level, U32 width, const U32 levels, const U32 flags, const E_TextureFormat format)
{
	U32 offset = 0;

	for(U32 f=0; f<face; f++)
	{
		const U32 size = GetTexture2DSize(width, width, levels, flags, format);

		const U32 alignedSize = ALIGN_SIZE(size, 128); // Cube map faces need to be aligned to 128 bytes

		offset += alignedSize;
	}

	for(U32 l=0; l<level; l++)
	{
		if(l >= levels)
			return offset;

		const U32 size = GetTextureCubeSize(width, flags, format);

		offset += size;

		if(width > 1)
			width /= 2;
	}

	return offset;
}

U32 GetTexture2DLevels(U32 width, U32 height)
{
	U32 levels = 0;
	
	while(width != 1 && height != 1)
	{
		if(width > 1)
			width /= 2;
		if(height> 1)
			height /= 2;

		levels++;

		if(width < 1)
			width = 1;
		if(height < 1)
			height = 1;
	}

	levels++;

	return levels;
}

void SetTextureFormat(const E_TextureFormat format, const U32 flags, CellGcmTexture* pTexture)
{
	BIZ_ASSERT(pTexture);

	switch(format)
	{
	case E_TextureFormat_DXT1:
		pTexture->format =
			CELL_GCM_TEXTURE_COMPRESSED_DXT1 |
			CELL_GCM_TEXTURE_SZ |
			CELL_GCM_TEXTURE_NR;
		pTexture->remap = CELL_GCM_REMAP_MODE(
			CELL_GCM_TEXTURE_REMAP_ORDER_XYXY,
			CELL_GCM_TEXTURE_REMAP_FROM_A, CELL_GCM_TEXTURE_REMAP_FROM_R, CELL_GCM_TEXTURE_REMAP_FROM_G, CELL_GCM_TEXTURE_REMAP_FROM_B,
			CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP);
		break;
	
	case E_TextureFormat_DXT3:
		pTexture->format =
			CELL_GCM_TEXTURE_COMPRESSED_DXT23 |
			CELL_GCM_TEXTURE_SZ |
			CELL_GCM_TEXTURE_NR;
		pTexture->remap = CELL_GCM_REMAP_MODE(
			CELL_GCM_TEXTURE_REMAP_ORDER_XYXY,
			CELL_GCM_TEXTURE_REMAP_FROM_A, CELL_GCM_TEXTURE_REMAP_FROM_R, CELL_GCM_TEXTURE_REMAP_FROM_G, CELL_GCM_TEXTURE_REMAP_FROM_B,
			CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP);
		break;
	
	case E_TextureFormat_DXT5:
		pTexture->format =
			CELL_GCM_TEXTURE_COMPRESSED_DXT45 |
			CELL_GCM_TEXTURE_SZ |
			CELL_GCM_TEXTURE_NR;
		pTexture->remap = CELL_GCM_REMAP_MODE(
			CELL_GCM_TEXTURE_REMAP_ORDER_XYXY,
			CELL_GCM_TEXTURE_REMAP_FROM_A, CELL_GCM_TEXTURE_REMAP_FROM_R, CELL_GCM_TEXTURE_REMAP_FROM_G, CELL_GCM_TEXTURE_REMAP_FROM_B,
			CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP);
		break;
	
	case E_TextureFormat_L8:
		pTexture->format =
			CELL_GCM_TEXTURE_B8 |
			CELL_GCM_TEXTURE_LN;
		if(flags & E_Flags_Raw)
		{
			pTexture->remap = CELL_GCM_REMAP_MODE(
				CELL_GCM_TEXTURE_REMAP_ORDER_XYXY,
				CELL_GCM_TEXTURE_REMAP_FROM_B, CELL_GCM_TEXTURE_REMAP_FROM_B, CELL_GCM_TEXTURE_REMAP_FROM_B, CELL_GCM_TEXTURE_REMAP_FROM_B,
				CELL_GCM_TEXTURE_REMAP_ONE,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP);
		}
		else
		{
			pTexture->remap = CELL_GCM_REMAP_MODE(
				CELL_GCM_TEXTURE_REMAP_ORDER_XYXY,
				CELL_GCM_TEXTURE_REMAP_FROM_B, CELL_GCM_TEXTURE_REMAP_FROM_B, CELL_GCM_TEXTURE_REMAP_FROM_B, CELL_GCM_TEXTURE_REMAP_FROM_B,
				CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP);
		}
		break;
	
	case E_TextureFormat_A8R8G8B8:
		pTexture->format =
			CELL_GCM_TEXTURE_A8R8G8B8 |
			CELL_GCM_TEXTURE_LN |
			CELL_GCM_TEXTURE_NR;
		if(flags & E_Flags_Raw)
		{
			pTexture->remap = CELL_GCM_REMAP_MODE(
				CELL_GCM_TEXTURE_REMAP_ORDER_XYXY,
				CELL_GCM_TEXTURE_REMAP_FROM_B, CELL_GCM_TEXTURE_REMAP_FROM_G, CELL_GCM_TEXTURE_REMAP_FROM_R, CELL_GCM_TEXTURE_REMAP_FROM_A,
				CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP);
		}
		else
		{
			pTexture->remap = CELL_GCM_REMAP_MODE(
				CELL_GCM_TEXTURE_REMAP_ORDER_XYXY,
				CELL_GCM_TEXTURE_REMAP_FROM_A, CELL_GCM_TEXTURE_REMAP_FROM_R, CELL_GCM_TEXTURE_REMAP_FROM_G, CELL_GCM_TEXTURE_REMAP_FROM_B,
				CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP);
		}
		break;
	
	case E_TextureFormat_D16:
		pTexture->format =
			CELL_GCM_TEXTURE_DEPTH16 |
			CELL_GCM_TEXTURE_LN;
		pTexture->remap = CELL_GCM_REMAP_MODE(
			CELL_GCM_TEXTURE_REMAP_ORDER_XYXY,
			CELL_GCM_TEXTURE_REMAP_FROM_A, CELL_GCM_TEXTURE_REMAP_FROM_R, CELL_GCM_TEXTURE_REMAP_FROM_G, CELL_GCM_TEXTURE_REMAP_FROM_B,
			CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP);
		break;
	
	case E_TextureFormat_D24S8:
		pTexture->format =
			CELL_GCM_TEXTURE_DEPTH24_D8 |
			CELL_GCM_TEXTURE_LN;
		pTexture->remap = CELL_GCM_REMAP_MODE(
			CELL_GCM_TEXTURE_REMAP_ORDER_XYXY,
			CELL_GCM_TEXTURE_REMAP_FROM_A, CELL_GCM_TEXTURE_REMAP_FROM_R, CELL_GCM_TEXTURE_REMAP_FROM_G, CELL_GCM_TEXTURE_REMAP_FROM_B,
			CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP);
		break;
	
	case E_TextureFormat_R32F:
		pTexture->format =
			CELL_GCM_TEXTURE_X32_FLOAT |
			CELL_GCM_TEXTURE_LN;
		pTexture->remap = CELL_GCM_REMAP_MODE(
			CELL_GCM_TEXTURE_REMAP_ORDER_XYXY,
			CELL_GCM_TEXTURE_REMAP_FROM_A, CELL_GCM_TEXTURE_REMAP_FROM_R, CELL_GCM_TEXTURE_REMAP_FROM_G, CELL_GCM_TEXTURE_REMAP_FROM_B,
			CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP);
		break;
	
	case E_TextureFormat_A16B16G16R16F:
		pTexture->format =
			CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT |
			CELL_GCM_TEXTURE_LN;
		pTexture->remap = CELL_GCM_REMAP_MODE(
			CELL_GCM_TEXTURE_REMAP_ORDER_XYXY,
			CELL_GCM_TEXTURE_REMAP_FROM_A, CELL_GCM_TEXTURE_REMAP_FROM_R, CELL_GCM_TEXTURE_REMAP_FROM_G, CELL_GCM_TEXTURE_REMAP_FROM_B,
			CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP);
		break;
	
	case E_TextureFormat_A32B32G32R32F:
		pTexture->format =
			CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT |
			CELL_GCM_TEXTURE_LN;
		pTexture->remap = CELL_GCM_REMAP_MODE(
			CELL_GCM_TEXTURE_REMAP_ORDER_XYXY,
			CELL_GCM_TEXTURE_REMAP_FROM_A, CELL_GCM_TEXTURE_REMAP_FROM_R, CELL_GCM_TEXTURE_REMAP_FROM_G, CELL_GCM_TEXTURE_REMAP_FROM_B,
			CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP);
		break;
	
	default:
		BIZ_ASSERT(0 && "Unsupported texture format");
		break;
	}
}

U32 SetTexture2DHeader(const U32 width, const U32 height, const U32 levels, const U32 flags, const E_TextureFormat format, const U32 pitch, GPUTexture* pTexture)
{
	BIZ_ASSERT(pTexture);

	pTexture->width			= width;
	pTexture->height		= height;
	pTexture->depth			= 1;
	pTexture->mipmap		= levels;
	pTexture->cubemap		= CELL_GCM_FALSE;
	pTexture->dimension		= CELL_GCM_TEXTURE_DIMENSION_2;
	pTexture->location		= FlagsToLocation(flags);
	pTexture->pitch			= pitch;

	SetTextureFormat(format, flags, pTexture);

	return GetTexture2DSize(width, height, levels, flags, format);
}

U32 SetTexture3DHeader(const U32 width, const U32 height, const U32 depth,const U32 levels, const U32 flags, const E_TextureFormat format, const U32 pitch, GPUTexture* pTexture)
{
	BIZ_ASSERT(pTexture);

	pTexture->width			= width;
	pTexture->height		= height;
	pTexture->depth			= depth;
	pTexture->mipmap		= levels;
	pTexture->cubemap		= CELL_GCM_FALSE;
	pTexture->dimension		= CELL_GCM_TEXTURE_DIMENSION_3;
	pTexture->location		= FlagsToLocation(flags);
	pTexture->pitch			= pitch;

	SetTextureFormat(format, flags, pTexture);

	return GetTexture3DSize(width, height, depth, levels, flags, format);
}

U32 SetTextureCubeHeader(const U32 width, const U32 levels, const U32 flags, const E_TextureFormat format, const U32 pitch, GPUTexture* pTexture)
{
	BIZ_ASSERT(pTexture);

	pTexture->width			= width;
	pTexture->height		= width;
	pTexture->depth			= 1;
	pTexture->mipmap		= levels;
	pTexture->cubemap		= CELL_GCM_TRUE;
	pTexture->dimension		= CELL_GCM_TEXTURE_DIMENSION_2;
	pTexture->location		= FlagsToLocation(flags);
	pTexture->pitch			= pitch;

	SetTextureFormat(format, flags, pTexture);

	return GetTextureCubeSize(width, levels, flags, format);
}

bool DebugCheckTexture(const CellGcmTexture* pTexture)
{
	GPU_ASSERT_X(
		pTexture);
	GPU_ASSERT_X(
		pTexture->mipmap >= 1 &&
		pTexture->mipmap <= 13);
	GPU_ASSERT_X(
		pTexture->dimension == CELL_GCM_TEXTURE_DIMENSION_1 ||
		pTexture->dimension == CELL_GCM_TEXTURE_DIMENSION_2 ||
		pTexture->dimension == CELL_GCM_TEXTURE_DIMENSION_3);
	GPU_ASSERT_X(
		pTexture->cubemap == E_Bool_False ||
		pTexture->cubemap == E_Bool_True);
	GPU_ASSERT_X(
		pTexture->width >= 1 &&
		pTexture->width <= 4096);
	GPU_ASSERT_X(
		pTexture->height >= 1 &&
		pTexture->height <= 4096);
	GPU_ASSERT_X(
		pTexture->depth >= 1 &&
		pTexture->depth <= 512);
	GPU_ASSERT_X(
		pTexture->location == E_Location_Main ||
		pTexture->location == E_Location_Local);
	GPU_ASSERT_X( // Is this a valid main offset?
		pTexture->location == E_Location_Main ?
		IsMainOffset(pTexture->offset) :
		1);
	GPU_ASSERT_X( // Is this a valid local offset?
		pTexture->location == E_Location_Local ?
		IsLocalOffset(pTexture->offset) :
		1);
	GPU_ASSERT_X( // If swizzled is the offset aligned to 128b?
		pTexture->format & CELL_GCM_TEXTURE_LN ?
		1 :
		((pTexture->offset % 128) == 0));
	GPU_ASSERT_X( // Valid dimensions for 1d texture?
		pTexture->dimension == CELL_GCM_TEXTURE_DIMENSION_1 ?
		pTexture->width != 0 && pTexture->height == 1 && pTexture->depth == 1 :
		1); 
	GPU_ASSERT_X( // Valid dimensions for 2d texture?
		pTexture->dimension == CELL_GCM_TEXTURE_DIMENSION_2 ?
		pTexture->width != 0 && pTexture->height != 0 && pTexture->depth == 1 :
		1);
	GPU_ASSERT_X( // Valid dimensions for 3d texture?
		pTexture->dimension == CELL_GCM_TEXTURE_DIMENSION_3 ?
		pTexture->width != 0 && pTexture->height != 0 && pTexture->depth != 0 :
		1);
	GPU_ASSERT_X( // Valid dimensions for cube texture?
		pTexture->cubemap == E_Bool_True ?
		pTexture->width == pTexture->height && pTexture->depth == 1 && pTexture->dimension == CELL_GCM_TEXTURE_DIMENSION_2 :
		1);
	
	return true;
}

void DebugPrintContextData(const CellGcmContextData* pContextData)
{
	BIZ_ASSERT(pContextData);

	CORE_PRINT("CellGcmContextData : begin 0x%.8x end 0x%.8x current 0x%.8x free %i\n",
		pContextData->begin,
		pContextData->end,
		pContextData->current,
		(U32)pContextData->end-(U32)pContextData->current);
}

const Util::C_String GetFlagsAsString(const U32 flags)
{
	Util::C_String str = "";
	
	#define ADD_FLAG_TO_STR(_str, _flags, _flag, _name) if(_flags & _flag) { if(_str != "") _str += "|"; _str += _name; }
	
	ADD_FLAG_TO_STR(str, flags, E_Flags_Video,			"Video");
	ADD_FLAG_TO_STR(str, flags, E_Flags_System,			"System");
	ADD_FLAG_TO_STR(str, flags, E_Flags_Device,			"Device");
	ADD_FLAG_TO_STR(str, flags, E_Flags_Indirect,		"Indirect");
	ADD_FLAG_TO_STR(str, flags, E_Flags_Dynamic,		"Dynamic");
	ADD_FLAG_TO_STR(str, flags, E_Flags_Colour,			"Colour");
	ADD_FLAG_TO_STR(str, flags, E_Flags_DepthStencil,	"DepthStencil");
	ADD_FLAG_TO_STR(str, flags, E_Flags_Raw,			"Raw");
	ADD_FLAG_TO_STR(str, flags, E_Flags_Tiled,			"Tiled");
	ADD_FLAG_TO_STR(str, flags, E_Flags_Linear,			"Linear");
	ADD_FLAG_TO_STR(str, flags, E_Flags_Swizzle,		"Swizzle");
	ADD_FLAG_TO_STR(str, flags, E_Flags_SRGB,			"SRGB");
	ADD_FLAG_TO_STR(str, flags, E_Flags_DoNotTrack,		"DoNotTrack");
	ADD_FLAG_TO_STR(str, flags, E_Flags_DoNotReload,	"DoNotReload");

	return str;
}

const Util::C_String GetIndexFormatAsString(const E_IndexFormat indexFormat)
{
	switch(indexFormat)
	{
	case E_IndexFormat_16:						return "16";	
	case E_IndexFormat_32:						return "32";		
	default:									return "Unknown";
	}
}

const Util::C_String GetMultisampleAsString(const E_Multisample multisample)
{
	switch(multisample)
	{
	case E_Multisample_None:					return "1xAA";	
	case E_Multisample_2:						return "2xAA";		
	case E_Multisample_4:						return "4xAA";		
	default:									return "Unknown";
	}
}

const Util::C_String GetSurfaceFormatAsString(const E_SurfaceFormat format)
{
	switch(format)
	{
	case E_SurfaceFormat_L8:						return "L8";
	case E_SurfaceFormat_A8R8G8B8:					return "A8R8G8B8";
	case E_SurfaceFormat_R32F:						return "R32F";
	case E_SurfaceFormat_D16:						return "D16";
	case E_SurfaceFormat_D24S8:						return "D24S8";
	case E_SurfaceFormat_A16B16G16R16F: 			return "A16B16G16R16F";
	case E_SurfaceFormat_A32B32G32R32F: 			return "A32B32G32R32F";

	//case E_SurfaceFormat_X1R5G5B5_Z1R5G5B5:			return "X1R5G5B5_Z1R5G5B5";
	//case E_SurfaceFormat_X1R5G5B5_O1R5G5B5:			return "X1R5G5B5_O1R5G5B5";
	case E_SurfaceFormat_R5G6B5:					return "R5G6B5";
	case E_SurfaceFormat_X8R8G8B8_Z8R8G8B8:			return "X8R8G8B8_Z8R8G8B8";
	case E_SurfaceFormat_X8R8G8B8_O8R8G8B8:			return "X8R8G8B8_O8R8G8B8";
	case E_SurfaceFormat_A8B8G8R8:					return "A8B8G8R8";
	case E_SurfaceFormat_X8B8G8R8_Z8B8G8R8: 		return "X8B8G8R8_Z8B8G8R8";
	case E_SurfaceFormat_X8B8G8R8_O8B8G8R8: 		return "X8B8G8R8_O8B8G8R8";

	default:										return "Unknown";
	}
}

const Util::C_String GetTextureFormatAsString(const E_TextureFormat format)
{
	switch(format)
	{
	case E_TextureFormat_DXT1:						return "DXT1";
	case E_TextureFormat_DXT3:						return "DXT3";
	case E_TextureFormat_DXT5:						return "DXT5";
	case E_TextureFormat_L8:						return "L8";
	case E_TextureFormat_G16R16:					return "G16R16";
	case E_TextureFormat_G16R16F:					return "G16R16F";
	case E_TextureFormat_A8R8G8B8:					return "A8R8G8B8";
	case E_TextureFormat_R32F:						return "R32F";
	case E_TextureFormat_D16:						return "D16";
	case E_TextureFormat_D24S8:						return "D24S8";
	case E_TextureFormat_A16B16G16R16F:				return "A16B16G16R16F";
	case E_TextureFormat_A32B32G32R32F:				return "A32B32G32R32F";
	
	case E_TextureFormat_A1R5G5B5:					return "A1R5G5B5";
	case E_TextureFormat_A4R4G4B4:					return "A4R4G4B4";
	case E_TextureFormat_R5G6B5:					return "R5G6B5";
	case E_TextureFormat_G8B8:						return "G8B8";
	case E_TextureFormat_R6G5B5:					return "R6G5B5";
	case E_TextureFormat_DEPTH24_D8_FLOAT:			return "DEPTH24_D8_FLOAT";
	case E_TextureFormat_DEPTH16_FLOAT:				return "DEPTH16_FLOAT";
	case E_TextureFormat_X16:						return "X16";
	case E_TextureFormat_R5G5B5A1:					return "R5G5B5A1";
	case E_TextureFormat_COMPRESSED_HILO8:			return "COMPRESSED_HILO8";
	case E_TextureFormat_COMPRESSED_HILO_S8:		return "COMPRESSED_HILO_S8";
	case E_TextureFormat_D1R5G5B5: 					return "D1R5G5B5";
	case E_TextureFormat_D8R8G8B8: 					return "D8R8G8B8";
	case E_TextureFormat_COMPRESSED_B8R8_G8R8: 		return "D8R8G8B8";
	case E_TextureFormat_COMPRESSED_R8B8_R8G8: 		return "COMPRESSED_R8B8_R8G8";

	default:										return "Unknown";
	}
}

const Util::C_String GetTextureTypeAsString(const E_TextureType type)
{
	switch(type)
	{
	case E_TextureType_2D:						return "2D";
	case E_TextureType_3D:						return "3D";	
	case E_TextureType_Cube:					return "Cube";
	default:									return "Unknown";
	}
}

C_CommandChain::C_CommandChain() :
	m_Size(0),
	m_Offset(UINT_MAX),
	m_pAddress(NULL)
{
}
	
void C_CommandChain::Setup(CellGcmContextData* pContextData, CellGcmContextCallback pContextCallback, const S32 padding)
{
	BIZ_ASSERT(pContextData);
	BIZ_ASSERT(m_Size > 0);
	BIZ_ASSERT(m_pAddress);
	BIZ_ASSERT(((U32)m_pAddress & 3) == 0);
	BIZ_ASSERT((m_Size & 3) == 0);
	BIZ_ASSERT((padding & 3) == 0);

	cellGcmSetupContextData(pContextData, (U32*)m_pAddress, m_Size-padding, pContextCallback);
}

C_Viewport::C_Viewport() :
	m_X(0),
	m_Y(0),
	m_Width(0),
	m_Height(0),
	m_MinZ(0.f),
	m_MaxZ(1.f)
{
}

C_Viewport::C_Viewport(const U32 x, const U32 y, const U32 width, const U32 height) :
	m_X(x),
	m_Y(y),
	m_Width(width),
	m_Height(height),
	m_MinZ(0.0f),
	m_MaxZ(1.0f)
{
}
C_Viewport::C_Viewport(const U32 x, const U32 y, const U32 width, const U32 height, const F32 minZ, const F32 maxZ) :
	m_X(x),
	m_Y(y),
	m_Width(width),
	m_Height(height),
	m_MinZ(minZ),
	m_MaxZ(maxZ)
{
}

C_Viewport::~C_Viewport()
{
}

C_VertexElement::C_VertexElement() :
	m_Stream(0xFF),
	m_Offset(0),
	m_Type(0),
	m_Usage(0),
	m_UsageIndex(0),
	m_gcmSize(0),
	m_gcmType(0),
	m_gcmStride(0)
{
}

C_VertexElement::C_VertexElement(const U16 stream, const U32 offset, const U32 type, const U8 usage, const U8 usageIndex) :
	m_Stream(stream),
	m_Offset(offset),
	m_Type(type),
	m_Usage(usage),
	m_UsageIndex(usageIndex),
	m_gcmSize(0),
	m_gcmType(0),
	m_gcmStride(0)
{
	SetType(type);
}

C_VertexElement::C_VertexElement(const U16 stream, const U32 offset, const U32 type, const U8 usage, const U8 usageIndex, const U8 gcmSize, const U8 gcmType, const U8 gcmStride) :
	m_Stream(stream),
	m_Offset(offset),
	m_Type(type),
	m_Usage(usage),
	m_UsageIndex(usageIndex),
	m_gcmSize(gcmSize),
	m_gcmType(gcmType),
	m_gcmStride(gcmStride)
{
}

C_VertexElement::~C_VertexElement()
{
}

void C_VertexElement::SetStream(const U16 stream)
{
	m_Stream = stream;
}

void C_VertexElement::SetOffset(const U32 offset)
{
	m_Offset = offset;
}

void C_VertexElement::SetType(const U32 type)
{
	m_Type = type;

	switch(m_Type)
	{
	case E_VertexElementType_F32_1: 	m_gcmSize = 1;	m_gcmType = CELL_GCM_VERTEX_F;		break;
    case E_VertexElementType_F32_2:		m_gcmSize = 2; 	m_gcmType = CELL_GCM_VERTEX_F;		break;
    case E_VertexElementType_F32_3: 	m_gcmSize = 3; 	m_gcmType = CELL_GCM_VERTEX_F;		break;
    case E_VertexElementType_F32_4: 	m_gcmSize = 4; 	m_gcmType = CELL_GCM_VERTEX_F;		break;
    case E_VertexElementType_Colour:	m_gcmSize = 4; 	m_gcmType = CELL_GCM_VERTEX_UB;		break;
	case E_VertexElementType_U8_4:		m_gcmSize = 4; 	m_gcmType = CELL_GCM_VERTEX_UB256;	break;
	case E_VertexElementType_U8_4N:		m_gcmSize = 4; 	m_gcmType = CELL_GCM_VERTEX_UB;		break;
	case E_VertexElementType_S16_2: 	m_gcmSize = 2;	m_gcmType = CELL_GCM_VERTEX_S32K;	break;
	case E_VertexElementType_S16_4: 	m_gcmSize = 4;	m_gcmType = CELL_GCM_VERTEX_S32K;	break;
	case E_VertexElementType_F16_2:		m_gcmSize = 2; 	m_gcmType = CELL_GCM_VERTEX_SF;		break;
	case E_VertexElementType_F16_4:		m_gcmSize = 4; 	m_gcmType = CELL_GCM_VERTEX_SF;		break;
	case E_VertexElementType_S16_2N: 	m_gcmSize = 2;	m_gcmType = CELL_GCM_VERTEX_S1;		break;
	case E_VertexElementType_SHHD_3: 	m_gcmSize = 1;	m_gcmType = CELL_GCM_VERTEX_CMP;	break;
	case E_VertexElementType_SHHD_3N: 	m_gcmSize = 1;	m_gcmType = CELL_GCM_VERTEX_CMP;	break;
	default:
		m_gcmSize = 0;	
		m_gcmType = 0;
		BIZ_ASSERT(0 && "Unsupported vertex element type");
		break;
	}
}

void C_VertexElement::SetUsage(const U8 usage)
{
	m_Usage = usage;
}

void C_VertexElement::SetUsageIndex(const U8 usageIndex)
{
	m_UsageIndex = usageIndex;
}

U32 C_VertexElement::GetSize() const
{
	switch(m_Type)
	{
	case E_VertexElementType_F32_1: 	return sizeof(F32);
	case E_VertexElementType_F32_2: 	return sizeof(F32) * 2;
	case E_VertexElementType_F32_3: 	return sizeof(F32) * 3;
	case E_VertexElementType_F32_4: 	return sizeof(F32) * 4;
	case E_VertexElementType_Colour:	return sizeof(U32);
	case E_VertexElementType_U8_4:		return sizeof(U8) * 4;
	case E_VertexElementType_U8_4N: 	return sizeof(U8) * 4;
	case E_VertexElementType_S16_2: 	return sizeof(S16) * 2;
	case E_VertexElementType_S16_4: 	return sizeof(S16) * 4;
	case E_VertexElementType_F16_2: 	return sizeof(U16) * 2;
	case E_VertexElementType_F16_4: 	return sizeof(U16) * 4;
	case E_VertexElementType_S16_2N:	return sizeof(S16) * 2;
	case E_VertexElementType_SHHD_3: 	return sizeof(U8) * 4;
	case E_VertexElementType_SHHD_3N:	return sizeof(U8) * 4;
	default:
		BIZ_ASSERT(0 && "Unsupported vertex element type");
		return 0;
	}
}

U32 C_VertexElement::GetCount() const
{
	switch(m_Type)
	{
	case E_VertexElementType_F32_1:		return 1;
	case E_VertexElementType_F32_2:		return 2;
	case E_VertexElementType_F32_3:		return 3;
	case E_VertexElementType_F32_4:		return 4;
	case E_VertexElementType_Colour:	return 1;
	case E_VertexElementType_U8_4:		return 4;
	case E_VertexElementType_U8_4N:		return 4;
	case E_VertexElementType_S16_2:		return 2;
	case E_VertexElementType_S16_4:		return 4;
	case E_VertexElementType_F16_2:		return 2;
	case E_VertexElementType_F16_4:		return 4;
	case E_VertexElementType_S16_2N:	return 2;
	case E_VertexElementType_SHHD_3:	return 1;
	case E_VertexElementType_SHHD_3N:	return 1;
	default:
		BIZ_ASSERT(0 && "Unsupported vertex element type");
		return 0;
	}
}

C_VertexDeclaration::C_VertexDeclaration() :
	m_ElementCount(0)
{
}

U32 C_VertexDeclaration::GetStride(const U32 stream) const
{
	U32 stride = 0;
	for(U32 i=0; i<m_ElementCount; i++)
		if(stream == m_ElementArray[i].GetStream())
			stride += m_ElementArray[i].GetSize();
	return stride;
}

C_VertexBuffer::C_VertexBuffer() :
	m_Size(0),
	m_Flags(0),
	m_Offset(0)
{
	#if DISPLAY_DEBUG
	m_Locked = false;
	#endif
}

void C_VertexBuffer::SetSize(const U32 size)
{
	m_Size = size;
}

void C_VertexBuffer::SetFlags(const U32 flags)
{
	m_Flags = flags;
}

void C_VertexBuffer::SetOffset(const U32 offset)
{
	m_Offset = offset;
}

void* C_VertexBuffer::Lock(const U32 offset, const U32 size, const U32 flags)
{
	BIZ_ASSERT(IsCreated());

	#if DISPLAY_DEBUG
	BIZ_ASSERT(!m_Locked);
	m_Locked = true;
	#endif

	BIZ_ASSERT(GetAddress() && offset+size <= GetSize());
	return &((U8*)GetAddress())[offset];
}

void* C_VertexBuffer::Lock(const U32 flags)
{
	return Lock(0, GetSize(), flags);
}

void C_VertexBuffer::Unlock()
{
	BIZ_ASSERT(IsCreated());

	#if DISPLAY_DEBUG
	BIZ_ASSERT(m_Locked);
	m_Locked = false;
	#endif
}

C_IndexBuffer::C_IndexBuffer() :
	m_Size(0),
	m_Flags(0),
	m_Offset(0),
	m_Format(E_IndexFormat_Unknown)
{
	#if DISPLAY_DEBUG
	m_Locked = false;
	#endif
}

void C_IndexBuffer::SetSize(const U32 size)
{
	m_Size = size;
}

void C_IndexBuffer::SetFlags(const U32 flags)
{
	m_Flags = flags;
}

void C_IndexBuffer::SetOffset(const U32 offset)
{
	m_Offset = offset;
}

void C_IndexBuffer::SetFormat(const E_IndexFormat format)
{
	m_Format = format;
}

void* C_IndexBuffer::Lock(const U32 offset, const U32 size, const U32 flags)
{
	BIZ_ASSERT(IsCreated());

	#if DISPLAY_DEBUG
	BIZ_ASSERT(!m_Locked);
	m_Locked = true;
	#endif

	BIZ_ASSERT(GetAddress() && offset+size <= GetSize());
	return &((U8*)GetAddress())[offset];
}

void* C_IndexBuffer::Lock(const U32 flags)
{
	return Lock(0, GetSize(), flags);
}

void C_IndexBuffer::Unlock()
{
	BIZ_ASSERT(IsCreated());

	#if DISPLAY_DEBUG
	BIZ_ASSERT(m_Locked);
	m_Locked = false;
	#endif
}

C_ShaderSemantic::C_ShaderSemantic() :
	m_Usage(UCHAR_MAX),
	m_UsageIndex(UCHAR_MAX)
{
}

C_ShaderSemantic::C_ShaderSemantic(const U8 usage, const U8 usageIndex) :
	m_Usage(usage),
	m_UsageIndex(usageIndex)
{
}

C_VertexShaderParam::C_VertexShaderParam() :
	m_RegisterIndex(UINT_MAX),
	m_RegisterCount(0)
{
}

C_VertexShader::C_VertexShader() :
	m_Size(0),
	m_Flags(0),
	m_pAddressInMainMemory(NULL)
{
}

void C_VertexShader::SetSize(const U32 size)
{
	m_Size = size;
}

void C_VertexShader::SetFlags(const U32 flags)
{
	m_Flags = flags;
}

const C_VertexShaderParam C_VertexShader::GetParamFromName(const char* name) const
{
#if !PLATFORM_SPU
	S32 paramCount = cellCgbMapGetLength(&m_Program);
	for(S32 i=0; i<paramCount; i++)
	{
		U32 size = 0;
		cellCgbMapGetName((CellCgbProgram*)&m_Program, i, NULL, &size);

		char* temp = new char[size+1];
		cellCgbMapGetName((CellCgbProgram*)&m_Program, i, temp, &size);

		if(strcmp(temp, name) == 0)
		{
			SAFE_DELETE_ARY(temp);

			U16 index = USHRT_MAX;
			cellCgbMapGetVertexUniformRegister(&m_Program, i, &index, NULL);

			C_VertexShaderParam param;
			param.m_RegisterIndex = index;
			param.m_RegisterCount = 4;
			return param;
		}
		else
		{
			SAFE_DELETE_ARY(temp);
		}
	}
#endif

	return C_VertexShaderParam();
}

const C_VertexShaderParam C_VertexShader::GetParameter(const U32 index, const char* name) const
{
	return GetParamFromName(name);
}

bool C_VertexShader::GetSemantics(Btl::Vector<C_ShaderSemantic>& semantics)
{
	if(m_ProgramConfig.attributeInputMask & (1<<E_VertexElementUsage_Position))
		semantics.PushBack(C_ShaderSemantic(E_VertexElementUsage_Position, 0));
	
	if(m_ProgramConfig.attributeInputMask & (1<<E_VertexElementUsage_Normal))
		semantics.PushBack(C_ShaderSemantic(E_VertexElementUsage_Normal, 0));
	
	if(m_ProgramConfig.attributeInputMask & (1<<E_VertexElementUsage_Colour))
		semantics.PushBack(C_ShaderSemantic(E_VertexElementUsage_Colour, 0));
	
	for(U32 i=0; i<6; i++)
		if(m_ProgramConfig.attributeInputMask & (1<<E_VertexElementUsage_TexCoord+i))
			semantics.PushBack(C_ShaderSemantic(E_VertexElementUsage_TexCoord, i));
	
	if(m_ProgramConfig.attributeInputMask & (1<<E_VertexElementUsage_Tangent))
		semantics.PushBack(C_ShaderSemantic(E_VertexElementUsage_Tangent, 0));
	
	if(m_ProgramConfig.attributeInputMask & (1<<E_VertexElementUsage_Binormal))
		semantics.PushBack(C_ShaderSemantic(E_VertexElementUsage_Binormal, 0));
	
	return true;
}

C_PixelShaderParamBase::C_PixelShaderParamBase() : 
	m_pPixelShader(NULL)
{
#if TRACK_DISPLAY2_RESOURCES
	m_Name[0] = '\0';
#endif
}

C_PixelShaderParamBase::~C_PixelShaderParamBase()
{
	m_Offsets.Clear();
}

C_PixelShader::C_PixelShader() :
	m_Size(0),
	m_Flags(0),
	m_pAddressInMainMemory(NULL)
{
}

C_PixelShader::~C_PixelShader()
{
	m_Params.Clear();
}

void C_PixelShader::SetSize(const U32 size)
{
	m_Size = size;
}

void C_PixelShader::SetFlags(const U32 flags)
{
	m_Flags = flags;
}

void C_PixelShader::SetOffset(const U32 offset)
{
	m_ProgramConfig.offset = offset;
}

C_PixelShaderParam C_PixelShader::GetParamFromName(const char* name) const
{
#if !PLATFORM_SPU
	S32 paramCount = m_Params.Size();
	for(S32 i=0; i<paramCount; i++)
	{
		U32 size = 0;
		cellCgbMapGetName((CellCgbProgram*)&m_Program, i, NULL, &size);

		char* temp = new char[size+1];
		cellCgbMapGetName((CellCgbProgram*)&m_Program, i, temp, &size);

		if(strcmp(temp, name) == 0)
		{
			SAFE_DELETE_ARY(temp);
			
			return &m_Params[i];
		}
		else
		{
			SAFE_DELETE_ARY(temp);
		}
	}
#endif

	return C_PixelShaderParam();
}

C_PixelShaderParam C_PixelShader::GetParameter(const S32 index, const char* name) const
{
	return GetParamFromName(name);
}

C_PixelShaderParamArray C_PixelShader::GetParameterArray(const S32 index, const char* name, const U32 count) const
{
	C_PixelShaderParamArray paramArray;

	for(S32 i=0; i<count; i++)
	{
		Util::C_String temp;
		
		temp.Printf("%s[%i]", name, i);
		
		paramArray.PushBack(GetParameter(index+i, temp.Chars()));
	}

	return paramArray;
}

void C_PixelShader::CreateParams()
{
#if !PLATFORM_SPU
	if(m_Params.Size() == 0)
	{
		U32 paramCount = cellCgbMapGetLength(&m_Program);
		m_Params.Resize(paramCount);
	}
	
	for(S32 i=0; i<m_Params.Size(); i++)
	{
		C_PixelShaderParamBase& param = m_Params[i];

		U32 paramIndex = i;

		#if TRACK_DISPLAY2_RESOURCES
		if(strcmp(param.m_Name, "") == 0)
		{
			U32 nameSize = sizeof(param.m_Name);
			cellCgbMapGetName(&m_Program, i, param.m_Name, &nameSize);
			param.m_pPixelShader = this;
		}
		else
		{
			paramIndex = cellCgbMapLookup(&m_Program, param.m_Name);	
			BIZ_ASSERT(param.m_pPixelShader == this);
		}
		#else
		param.m_pPixelShader = this;
		#endif

		if(paramIndex != CELL_CGB_ERROR_FAILED)
		{
			U32 offsetCount = 64;
			cellCgbMapGetFragmentUniformOffsets(&m_Program, paramIndex, NULL, &offsetCount);
			if(offsetCount > 0)
			{
				param.m_Offsets.Resize(offsetCount);
				
				cellCgbMapGetFragmentUniformOffsets(&m_Program, paramIndex, &param.m_Offsets.Front(), &offsetCount);
			}
			else
			{
				param.m_Offsets.Clear();
			}
		}
		else
		{
			param.m_Offsets.Clear();
		}
	}
#endif
}

C_LockedRect::C_LockedRect() :
	pitch(0),
	pBits(NULL)
{
}

C_ConstLockedRect::C_ConstLockedRect() :
	pitch(0),
	pBits(NULL)
{
}

C_LockedBox::C_LockedBox() :
	rowPitch(0),
	slicePitch(0),
	pBits(NULL)
{
}

C_ConstLockedBox::C_ConstLockedBox() :
	rowPitch(0),
	slicePitch(0),
	pBits(NULL)
{
}

void C_SurfaceParams::OverSurface(const C_Surface* pSurface)
{
}

void C_SurfaceParams::AfterSurface(const C_Surface* pSurface)
{
}

void C_SurfaceParams::BeforeSurface(const C_Surface* pSurface)
{
}

C_Surface::C_Surface() :
	m_Width(0),
	m_Height(0),
	m_Format(E_SurfaceFormat_Unknown),
	m_Multisample(E_Multisample_Unknown),
	m_Size(0),
	m_Pitch(0),
	m_Offset(0),
	m_Flags(0),
	m_pAddress(NULL)
{
	#if !CFG_GOLD
	m_Locked = false;
	#endif
}

void C_Surface::SetSize(const U32 size)
{
	m_Size = size;
}

void C_Surface::SetFlags(const U32 flags)
{
	m_Flags = flags;
}

void C_Surface::SetOffset(const U32 offset)
{
	m_Offset = offset;
}

void C_Surface::SetAddress(U8* pAddress)
{
	m_pAddress = pAddress;
}

C_LockedRect C_Surface::LockRect(const U32 flags)
{
	BIZ_ASSERT(IsCreated());

	#if !CFG_GOLD
	BIZ_ASSERT(!m_Locked);
	m_Locked = true;
	#endif

	C_LockedRect lockedRect;
	lockedRect.pitch = GetPitch();
	lockedRect.pBits = GetAddress();
	return lockedRect;
}

C_ConstLockedRect C_Surface::LockRect(const U32 flags) const
{
	BIZ_ASSERT(IsCreated());

	#if !CFG_GOLD
	BIZ_ASSERT(!m_Locked);
	m_Locked = true;
	#endif

	C_ConstLockedRect lockedRect;
	lockedRect.pitch = GetPitch();
	lockedRect.pBits = GetAddress();
	return lockedRect;
}

void C_Surface::UnlockRect()
{
	BIZ_ASSERT(IsCreated());
	
	#if !CFG_GOLD
	BIZ_ASSERT(m_Locked);
	m_Locked = false;
	#endif
}

C_Texture::C_Texture() : m_Size(0), m_Flags(0)
{
	memset(&m_gcmTexture, 0, sizeof(m_gcmTexture));

	#if !CFG_GOLD
	memset(m_Locked, 0, sizeof(m_Locked));
	#endif
}

void C_Texture::SetSize(const U32 size)
{
	m_Size = size;
}

void C_Texture::SetFlags(const U32 flags)
{
	m_Flags = flags;
}

void C_Texture::SetOffset(const U32 offset)
{
	m_gcmTexture.offset = offset;
}

void C_Texture::SetSRGB(const bool enable)
{
	if(enable)
	{
		m_Flags |= E_Flags_SRGB;
	}
	else
	{
		m_Flags &= ~E_Flags_SRGB;
	}
}

C_LockedRect C_Texture::LockRect2D(const U32 level, const U32 flags)
{
	BIZ_ASSERT(IsCreated());
	BIZ_ASSERT(GetType() == E_TextureType_2D);

	#if !CFG_GOLD
	BIZ_ASSERT(!m_Locked[0]);
	m_Locked[0] = true;
	#endif

	U32 offset = GetTexture2DLevelOffset(level, GetWidth(), GetHeight(), GetLevels(), GetFlags(), GetFormat());
	
	C_LockedRect lockedRect;
	lockedRect.pitch = GetPitch();
	lockedRect.pBits = GetAddress()+offset;
	return lockedRect;
}

C_ConstLockedRect C_Texture::LockRect2D(const U32 level, const U32 flags) const
{
	BIZ_ASSERT(IsCreated());
	BIZ_ASSERT(GetType() == E_TextureType_2D);

	#if !CFG_GOLD
	BIZ_ASSERT(!m_Locked[0]);
	m_Locked[0] = true;
	#endif

	U32 offset = GetTexture2DLevelOffset(level, GetWidth(), GetHeight(), GetLevels(), GetFlags(), GetFormat());
	
	C_ConstLockedRect lockedRect;
	lockedRect.pitch = GetPitch();
	lockedRect.pBits = GetAddress()+offset;
	return lockedRect;
}

void C_Texture::UnlockRect2D(const U32 level) const
{
	BIZ_ASSERT(IsCreated());
	BIZ_ASSERT(GetType() == E_TextureType_2D);

	#if !CFG_GOLD
	BIZ_ASSERT(m_Locked[0]);
	m_Locked[0] = false;
	#endif
}

C_LockedBox C_Texture::LockBox3D(const U32 level, const U32 flags)
{
	BIZ_ASSERT(IsCreated());
	BIZ_ASSERT(GetType() == E_TextureType_3D);

	#if !CFG_GOLD
	BIZ_ASSERT(!m_Locked[0]);
	m_Locked[0] = true;
	#endif

	U32 rowPitch = GetTextureRowPitch(GetWidth(), GetFlags(), GetFormat());
	U32 slicePitch = GetTextureSlicePitch(GetWidth(), GetHeight(), GetLevels(), GetFlags(), GetFormat());
	U32 offset = GetTexture3DLevelOffset(level, GetWidth(), GetHeight(), GetDepth(), GetLevels(), GetFlags(), GetFormat());
	
	C_LockedBox lockedBox;
	lockedBox.rowPitch = rowPitch;
	lockedBox.slicePitch = slicePitch;
	lockedBox.pBits = GetAddress()+offset;
	return lockedBox;
}

C_ConstLockedBox C_Texture::LockBox3D(const U32 level, const U32 flags) const
{
	BIZ_ASSERT(IsCreated());
	BIZ_ASSERT(GetType() == E_TextureType_3D);

	#if !CFG_GOLD
	BIZ_ASSERT(!m_Locked[0]);
	m_Locked[0] = true;
	#endif

	U32 rowPitch = GetTextureRowPitch(GetWidth(), GetFlags(), GetFormat());
	U32 slicePitch = GetTextureSlicePitch(GetWidth(), GetHeight(), GetLevels(), GetFlags(), GetFormat());
	U32 offset = GetTexture3DLevelOffset(level, GetWidth(), GetHeight(), GetDepth(), GetLevels(), GetFlags(), GetFormat());
	
	C_ConstLockedBox lockedBox;
	lockedBox.rowPitch = rowPitch;
	lockedBox.slicePitch = slicePitch;
	lockedBox.pBits = GetAddress()+offset;
	return lockedBox;
}

void C_Texture::UnlockBox3D(const U32 level) const
{
	BIZ_ASSERT(IsCreated());
	BIZ_ASSERT(GetType() == E_TextureType_3D);

	#if !CFG_GOLD
	BIZ_ASSERT(m_Locked[0]);
	m_Locked[0] = false;
	#endif
}

C_LockedRect C_Texture::LockRectCube(const E_CubeMapFace face, const U32 level, const U32 flags)
{
	BIZ_ASSERT(IsCreated());
	BIZ_ASSERT(GetType() == E_TextureType_Cube);

	#if !CFG_GOLD
	BIZ_ASSERT(face < 6);
	BIZ_ASSERT(!m_Locked[face]);
	m_Locked[face] = true;
	#endif

	U32 offset = GetTextureCubeLevelOffset(face, level, GetWidth(), GetLevels(), GetFlags(), GetFormat());
	
	C_LockedRect lockedRect;
	lockedRect.pitch = GetPitch();
	lockedRect.pBits = GetAddress()+offset;
	return lockedRect;
}

C_ConstLockedRect C_Texture::LockRectCube(const E_CubeMapFace face, const U32 level, const U32 flags) const
{
	BIZ_ASSERT(IsCreated());
	BIZ_ASSERT(GetType() == E_TextureType_Cube);

	#if !CFG_GOLD
	BIZ_ASSERT(face < 6);
	BIZ_ASSERT(!m_Locked[face]);
	m_Locked[face] = true;
	#endif

	U32 offset = GetTextureCubeLevelOffset(face, level, GetWidth(), GetLevels(), GetFlags(), GetFormat());
	
	C_ConstLockedRect lockedRect;
	lockedRect.pitch = GetPitch();
	lockedRect.pBits = GetAddress()+offset;
	return lockedRect;
}

void C_Texture::UnlockRectCube(const E_CubeMapFace face, const U32 level) const
{
	BIZ_ASSERT(IsCreated());
	BIZ_ASSERT(GetType() == E_TextureType_Cube);

	#if !CFG_GOLD
	BIZ_ASSERT(face < 6);
	BIZ_ASSERT(m_Locked[face]);
	m_Locked[face] = false;
	#endif
}

CellGcmTexture* C_Texture::Get()
{
	return &m_gcmTexture;

}
const Util::C_String C_Texture::GetFileName() const
{
	return "";
}

C_LockedRect C_Texture::Lock(const U32 level)
{
	BIZ_ASSERT(GetType() == E_TextureType_2D);

	return LockRect2D(level);
}

C_ConstLockedRect C_Texture::Lock(const U32 level) const
{
	BIZ_ASSERT(GetType() == E_TextureType_2D);

	return LockRect2D(level);
}

void C_Texture::Unlock(const U32 level) const
{
	UnlockRect2D(level);
}

C_RenderTexture2D::C_RenderTexture2D() : m_Flags(0)
{
	m_TextureIndex = 0;
	m_SurfaceIndex = 0;
}

C_RenderTexture2D::~C_RenderTexture2D()
{
}

C_RenderTextureCube::C_RenderTextureCube() : m_Flags(0)
{
}

C_RenderTextureCube::~C_RenderTextureCube()
{
}

C_ContextBase::C_ContextBase() :
	m_pData(NULL)
{
}

C_ContextBase::C_ContextBase(C_ContextData* pData) :
	m_pData(pData)
{
	BIZ_ASSERT(pData);
}

C_Context::C_Context() :
	m_pState(NULL)
{
}

C_Context::C_Context(C_ContextData* pData) :
	C_ContextBase(pData),
	m_pState(NULL)
{
}

C_Context::C_Context(C_ContextData* pData, C_ContextState* pState) :
	C_ContextBase(pData),
	m_pState(pState)
{
}

C_ContextNoCheck::C_ContextNoCheck() :
	m_pState(NULL)
{
}

C_ContextNoCheck::C_ContextNoCheck(C_ContextData* pData) :
	C_ContextBase(pData),
	m_pState(NULL)
{
}

C_ContextNoCheck::C_ContextNoCheck(C_ContextData* pData, C_ContextState* pState) :
	C_ContextBase(pData),
	m_pState(pState)
{
}

C_ContextNoCheckNoTrack::C_ContextNoCheckNoTrack()
{
}

C_ContextNoCheckNoTrack::C_ContextNoCheckNoTrack(C_ContextData* pData) :
	C_ContextBase(pData)
{
}

C_ContextNoCheckNoTrackMeasure::C_ContextNoCheckNoTrackMeasure()
{
}

C_ContextNoCheckNoTrackMeasure::C_ContextNoCheckNoTrackMeasure(C_ContextData* pData) :
	C_ContextBase(pData)
{
}

C_ContextNoCheckNoTrackUnsafeInline::C_ContextNoCheckNoTrackUnsafeInline()
{
}

C_ContextNoCheckNoTrackUnsafeInline::C_ContextNoCheckNoTrackUnsafeInline(C_ContextData* pData) :
	C_ContextBase(pData)
{
}

C_ContextNoCheckNoTrackUnsafe::C_ContextNoCheckNoTrackUnsafe()
{
}

C_ContextNoCheckNoTrackUnsafe::C_ContextNoCheckNoTrackUnsafe(C_ContextData* pData) :
	C_ContextBase(pData)
{
}

} // namespace Display2
