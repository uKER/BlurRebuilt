#ifndef ATTK_IMAGE_RESIZE_H
#define ATTK_IMAGE_RESIZE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace AttkUt
{
	class C_ImageResize
	{
	public:
		static Attk::C_Status Resize
		(
			Attk::C_Image& img,
			U32 width, 
			U32 height,
			U32 depth,
			C_ImageFilter::E_PixelFilter filter = C_ImageFilter::E_PixelFilter_Box,
			U32 dstFilterWidth = 1,
			U32 dstFilterHeight = 1,
			U32 dstFilterDepth = 1
		);

		static void GrowBorder(Attk::C_Image& image, U32 size, bool wrapTexture = true);
	};
}

#endif
