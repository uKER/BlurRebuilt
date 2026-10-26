#ifndef ATTK_PIXEL_BLEND_H
#define ATTK_PIXEL_BLEND_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Attk
{
	typedef void(*PixelBlendFunc)(
		const C_PixelR32G32B32A32F& pSource,
		F32 alphaMultiplier,
		C_PixelR32G32B32A32F& pDestination);

	class C_PixelBlend
	{
	public:
		static C_Status Blend(
			const C_PixelR32G32B32A32F* pSource,
			E_BlendMode blendMode,
			F32 opacity,
			C_PixelR32G32B32A32F* pDestination,
			U32 nPixels);
	private:
		static void AdditiveBlend(
			const C_PixelR32G32B32A32F& pSource,
			F32 opacity,
			C_PixelR32G32B32A32F& pDestination);

		static void MultiplyBlend(
			const C_PixelR32G32B32A32F& pSource,
			F32 opacity,
			C_PixelR32G32B32A32F& pDestination);

		static void Copy(
			const C_PixelR32G32B32A32F& pSource,
			F32 opacity,
			C_PixelR32G32B32A32F& pDestination);
	};
};

#endif
