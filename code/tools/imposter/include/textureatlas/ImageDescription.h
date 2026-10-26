#ifndef TEXTUREATLAS_IMAGELIST_H
#define TEXTUREATLAS_IMAGELIST_H

#ifdef _MSC_VER
#pragma once
#endif

namespace TextureAtlas
{
	struct ImageSize
	{
		U32 x, y;

		ImageSize()
		: x(0),y(0)
		{ }

		ImageSize(U32 xx, U32 yy)
		: x(xx),y(yy)
		{ }
	};

	struct ImageRect
	{
		U32 x, y, w, h;

		ImageRect()
		: x(0),y(0),w(0),h(0)
		{ }

		ImageRect(U32 xx, U32 yy, U32 ww, U32 hh)
		: x(xx),y(yy),w(ww),h(hh)
		{ }

	};

	struct C_InputImageDescription
	{
		Btl::String		fileName;
		ImageSize		size;

		C_InputImageDescription()
		{ }

		C_InputImageDescription(const Btl::String& f, const ImageSize& s)
		: fileName(f), size(s)
		{ }
	};


	struct C_OutputImageDescription
	{
		Btl::String		fileName;
		ImageRect		rect;
		U32				atlas;

		C_OutputImageDescription()
		{ }

		C_OutputImageDescription(const Btl::String& f, const ImageRect& r, S32 a=0)
		: fileName(f),rect(r),atlas(a)
		{ }
	};

	typedef ImageSize								ImagePosition;
	typedef Btl::Vector<C_InputImageDescription>	InputImageDescArray;
	typedef Btl::Vector<C_OutputImageDescription>	OutputImageDescArray;
	typedef Btl::Vector<ImageSize>					ImageSizeArray;
}

#endif