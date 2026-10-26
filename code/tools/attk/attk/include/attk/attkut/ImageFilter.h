#ifndef ATTK_IMAGE_FILTER_H
#define ATTK_IMAGE_FILTER_H

#ifdef _MSC_VER_
#pragma once
#endif

#include "../../../source/attk/attkut/PixelFilter.h"
#include "../../../source/attk/attkut/BoxFilter.h"
#include "../../../source/attk/attkut/EmbossFilter.h"
#include "../../../source/attk/attkut/SharpnessFilter.h"
#include "../../../source/attk/attkut/BartlettFilter.h"

namespace AttkUt
{
	class C_ImageFilter
	{
	public:
		enum E_PixelFilter
		{
			E_PixelFilter_Box,
			E_PixelFilter_Emboss,
			E_PixelFilter_Sharpness,
			E_PixelFilter_Bartlett,

			E_PixelFilter_Count,
			E_PixelFilter_Unknown
		};

		// Old Filter2D functions (to remain compatible with anything that was previously using them)

		static Attk::C_Status Filter2D(
			Attk::C_Image& img,
			E_PixelFilter filter,
			U32 dstFilterWidth,
			U32 dstFilterHeight);

		static Attk::C_Status Filter2D(
			Attk::C_Image& img,
			U32 width, 
			U32 height,
			U32 depth,
			E_PixelFilter filter,
			U32 dstFilterWidth,
			U32 dstFilterHeight);

		template<class PixelType>
		static Attk::C_Status Filter2D(
			U8* pDstPixelData, 
			Attk::C_Image& img, 
			U32 newWidth, 
			U32 newHeight,
			E_PixelFilter filter,
			U32 dstFilterWidth,
			U32 dstFilterHeight);


		static Attk::C_Status Filter(
			Attk::C_Image& img,
			E_PixelFilter filter,
			U32 dstFilterWidth,
			U32 dstFilterHeight,
			U32 dstFilterDepth);

		static Attk::C_Status Filter(
			Attk::C_Image& img,
			U32 width, 
			U32 height,
			U32 depth,
			E_PixelFilter filter,
			U32 dstFilterWidth,
			U32 dstFilterHeight,
			U32 dstFilterDepth);

		template<class PixelType>
		static Attk::C_Status Filter(
			U8* pDstPixelData, 
			Attk::C_Image& img, 
			U32 newWidth, 
			U32 newHeight,
			U32 newDepth,
			E_PixelFilter filter,
			U32 dstFilterWidth,
			U32 dstFilterHeight,
			U32 dstFilterDepth);

	private:
		template<class PixelType, class FilterType>
		static Attk::C_Status Filter(
			U8* pNewPixelData, 
			Attk::C_Image& img, 
			U32 newWidth, 
			U32 newHeight,
			U32 newDepth,
			U32 dstFilterWidth,
			U32 dstFilterHeight,
			U32 dstFilterDepth);
	};

	#include "../../../source/attk/attkut/ImageFilter.inl"
}

#endif
