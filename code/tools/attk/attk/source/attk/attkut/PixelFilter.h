#ifndef ATTKUT_PIXEL_FILTER_H
#define ATTKUT_PIXEL_FILTER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace AttkUt
{
	template<class PixelType, class Impl>
	class C_PixelFilter
	{
	public:
		C_PixelFilter(
			const U8*	pSrcPixelData,			// pointer to source image pixel data
			U32*		srcDimensions,			// width, height and depth of source image in pixels
			U32*		dstDimensions,			// width, height and depth of destination image in pixels
			U32*		dstFilterDimensions);	// width, height and depth of filter kernel in terms of destination image pixels.

		void SetSrcFilterPosition(U32 axis, S32 dstLinePos);

		void Filter(PixelType& dstPixel);
	
	private:
		FORCE_INLINE F32 Evaluate(F32 u, F32 v, F32 w) const;

		U32 GetSrcPixel(
			PixelType& srcPixel,

			const U8* pSrcPixelData,
			const S32* srcImageDimensions,
			const F32 srcFilterX, 
			const F32 srcFilterY,
			const F32 srcFilterZ);		

		//Constant data
		const U8*	m_pSrcPixelData;
		S32			m_srcDimensions[3];		// Width, height and depth of source image in pixels.
		S32			m_dstDimensions[3];		// Width, height and depth of destination image in pixels.
		F32			m_dstPixelCoverage[3];	// Number of pixels covered in our destination image.
		F32			m_srcPixelCoverage[3];	// Number of pixels covered in our source image.

		//State
		F32			m_srcOrigin[3];

		//Implementation
		Impl		m_impl;
		friend		Impl;
	};

	#include "PixelFilter.inl"
}

#endif
