#ifndef ATTK_UTILITY_H
#define ATTK_UTILITY_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Attk
{
	FORCE_INLINE U32 GetPixelIndex(
		U32 bytesPerPixel,
		U32 width,
		U32 height,
		U32 depth,
		U32 x, 
		U32 y,
		U32 z)
	{
		return  
			x * bytesPerPixel + 
			y * width * bytesPerPixel + 
			z * width * height * bytesPerPixel;
	}

	namespace Utility
	{
		U32 GetBlockSize(
			E_PixelFormat format);

		U32 GetBlockWidth(
			E_PixelFormat format);

		U32 GetPitch(
			U32 width,
			E_PixelFormat format);

		U32 GetSlicePitch(
			U32 width,
			U32 height,
			E_PixelFormat format);
	}
}

#endif
