#include "stdafx.h"

#include <attk/Enum.h>
#include <attk/Status.h>
#include <attk/Global.h>
#include <attk/Progress.h>
#include <attk/OptionAttribute.h>
#include <attk/Options.h>
#include <attk/BaseImage.h>
#include <attk/PixelFormats.h>
#include <attk/Image.h>
#include <attk/Utility.h>
#include <attk/attkut/ImageFilter.h>
#include <attk/attkut/ImageResize.h>

#include "PixelFilter.h"

using namespace AttkUt;

Attk::C_Status C_ImageResize::Resize(
	Attk::C_Image& img,
	U32 dstWidth, 
	U32 dstHeight,
	U32 dstDepth,
	C_ImageFilter::E_PixelFilter filter,
	U32 dstFilterWidth,
	U32 dstFilterHeight,
	U32 dstFilterDepth)
{
	if (dstWidth == img.GetWidth() && dstHeight == img.GetHeight() && dstDepth == img.GetDepth())
		return Attk::E_Status_Success;

	return AttkUt::C_ImageFilter::Filter(
		img,
		dstWidth,
		dstHeight,
		dstDepth,
		filter,
		dstFilterWidth,
		dstFilterHeight,
		dstFilterDepth);
}



void C_ImageResize::GrowBorder(Attk::C_Image& image, U32 size, bool wrapTexture)
{
	assert(image.GetWidth() != 1);
	assert(image.GetHeight() != 1);
	Attk::C_Image newImage;
	U32 newWidth = image.GetWidth()+size*2;
	U32 newHeight = image.GetHeight()+size*2;
	newImage.NewPixelData(newWidth, newHeight, 1, image.GetPixelFormat());

	U32 x,y;
	//Copy existing image in to the middle a new image.
	for (x = 0; x < image.GetWidth(); x++)
		for (y = 0; y < image.GetHeight(); y++)
			newImage.SetPixel(image.GetPixel(x,y), x+size, y+size );

	//Copy corner pixels.
	for (x = 0; x < size; x++)
		for (y = 0; y < size; y++)
		{
			newImage.SetPixel(image.GetPixel(0,0), 0+x,0+y);
			newImage.SetPixel(image.GetPixel((image.GetWidth()-size),0), newWidth-size+x,0+y);
			newImage.SetPixel(image.GetPixel(0,(image.GetHeight()-size)), 0+x,newHeight-size+y);
			newImage.SetPixel(image.GetPixel((image.GetWidth()-size),(image.GetHeight()-size)), newWidth-size+x,newHeight-size+y);
		}

	if (wrapTexture)
	{
		//Copy top border pixels
		for (x = 0; x < image.GetWidth(); x++)
			for (y = 0; y < size; y++)
	#ifdef _DEBUG
				newImage.SetPixel(Attk::C_PixelR32G32B32A32F(0,0,255,0),x+size, newHeight-size+y);
	#else
				newImage.SetPixel(image.GetPixel(x,(image.GetHeight()-size)), x+size, newHeight-size+y);
	#endif

		//Copy bottom border pixels
		for (x = 0; x < image.GetWidth(); x++)
			for (y = 0; y < size; y++)
	#ifdef _DEBUG
				newImage.SetPixel(Attk::C_PixelR32G32B32A32F(0,0,255,0),x+size, 0+y);
	#else
				newImage.SetPixel(image.GetPixel(x,0), x+size, 0+y);
	#endif

		//Copy left border pixels
		for (x = 0; x < size; x++)
			for (y = 0; y < image.GetHeight(); y++)
	#ifdef _DEBUG
				newImage.SetPixel(Attk::C_PixelR32G32B32A32F(0,0,255,0),newWidth-size+x, y+size);
	#else
				newImage.SetPixel(image.GetPixel((image.GetWidth()-size),y), newWidth-size+x, y+size);
	#endif
			
		//Copy right border pixels
		for (x = 0; x < size; x++)
			for (y = 0; y < image.GetHeight(); y++)
	#ifdef _DEBUG
				newImage.SetPixel(Attk::C_PixelR32G32B32A32F(0,0,255,0),x, y+size);
	#else
				newImage.SetPixel(image.GetPixel(0,y), x, y+size);
	#endif
	}
	else
	{
		//Copy top border pixels
		for (x = 0; x < image.GetWidth(); x++)
			for (y = 0; y < size; y++)
	#ifdef _DEBUG
				newImage.SetPixel(Attk::C_PixelR32G32B32A32F(0,0,255,0),x+size, 0+y);
	#else
				newImage.SetPixel(image.GetPixel(x,0), x+size, 0+y);
	#endif

		//Copy bottom border pixels
		for (x = 0; x < image.GetWidth(); x++)
			for (y = 0; y < size; y++)
	#ifdef _DEBUG
				newImage.SetPixel(Attk::C_PixelR32G32B32A32F(0,0,255,0),x+size, newHeight-size+y);
	#else
				newImage.SetPixel(image.GetPixel(x,(image.GetHeight()-size)), x+size, newHeight-size+y);
	#endif

		//Copy left border pixels
		for (x = 0; x < size; x++)
			for (y = 0; y < image.GetHeight(); y++)
	#ifdef _DEBUG
				newImage.SetPixel(Attk::C_PixelR32G32B32A32F(0,0,255,0),x, y+size);
	#else
				newImage.SetPixel(image.GetPixel(0,y), x, y+size);
	#endif
			
		//Copy right border pixels
		for (x = 0; x < size; x++)
			for (y = 0; y < image.GetHeight(); y++)
	#ifdef _DEBUG
				newImage.SetPixel(Attk::C_PixelR32G32B32A32F(0,0,255,0),newWidth-size+x, y+size);
	#else
				newImage.SetPixel(image.GetPixel((image.GetWidth()-size),y), newWidth-size+x, y+size);
	#endif
	}

	//Clear current image.
	image.DeletePixelData();
	image.NewPixelData( newWidth, newHeight, 1, newImage.GetPixelFormat(), newImage.GetPixelDataPtr());
}
