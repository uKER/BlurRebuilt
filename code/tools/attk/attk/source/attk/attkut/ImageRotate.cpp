
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
#include <attk/attkut/ImageRotate.h>

#include "PixelFilter.h"

namespace AttkUt
{

void C_ImageRotate::Rotate90CW(Attk::C_Image& image)
{
	Attk::C_Image newImage;
	newImage.NewPixelData(image.GetHeight(), image.GetWidth(), image.GetDepth(), image.GetPixelFormat());
	U32 iDestRow = 0;
	for (U32 iSrcColumn = 0; iSrcColumn < image.GetWidth(); iSrcColumn++)
	{
		U32 jDestColumn = image.GetHeight()-1;
		for (U32 jSrcRow = 0; jSrcRow < image.GetHeight(); jSrcRow++)
		{
			newImage.SetPixel( image.GetPixel( iSrcColumn, jSrcRow), jDestColumn, iDestRow );
			jDestColumn--;
		}
		iDestRow++;
	}
	image = newImage;
}


void C_ImageRotate::Rotate90CCW(Attk::C_Image& image)
{
	Attk::C_Image newImage;
	newImage.NewPixelData(image.GetHeight(), image.GetWidth(), image.GetDepth(), image.GetPixelFormat());
	U32 iDestRow = image.GetWidth()-1;
	for (U32 iSrcColumn = 0; iSrcColumn < image.GetWidth(); iSrcColumn++)
	{
		U32 jDestColumn = 0;
		for (U32 jSrcRow = 0; jSrcRow < image.GetHeight(); jSrcRow++)
		{
			newImage.SetPixel( image.GetPixel( iSrcColumn, jSrcRow), jDestColumn, iDestRow );
			jDestColumn++;
		}
		iDestRow--;
	}
	image = newImage;
}

}// namespace attkut