#ifndef ATTK_PIXEL_CONVERSION_H
#define ATTK_PIXEL_CONVERSION_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Attk
{
	typedef void(*PixelBlendFunc)(
		const C_PixelR32G32B32A32F& pSource,
		F32 alphaMultiplier,
		C_PixelR32G32B32A32F& pDestination);

	class C_PixelConversion
	{
	public:
		static C_Status CopyToInterchangeFormat(
			const U8* pSourcePixels,
			const C_PixelR32G32B32A32F* pInterchangePalette,
			E_PixelFormat sourceFormat,
			C_PixelR32G32B32A32F* pDestination,
			U32 nPixels);

		static C_Status CopyFromInterchangeFormat(
			const C_PixelR32G32B32A32F* pSource,
			U8* pDestination,
			E_PixelFormat destinationFormat,
			U32 nPixels);
	};
};

#endif
