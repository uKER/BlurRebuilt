#include "stdafx.h"
#include "attk/Attk.h"


using namespace AttkUt;


template <typename T>
void SetPaletteEntryData(U8* pPaletteData, const Attk::C_PixelR32G32B32A32F& colour, U32 index)
{
	T& paletteEntry = ((T*)pPaletteData)[index];
	paletteEntry.Set(colour);
}

void SetPaletteEntry(Attk::C_Image& image, const Attk::C_PixelR32G32B32A32F& colour, U32 index)
{
	assert(index<Attk::C_BaseImage::GetNumPaletteEntries(image.GetPixelFormat()));
	switch(image.GetPaletteFormat())
	{
	case Attk::E_PaletteFormat_R8G8B8A8U:
		return SetPaletteEntryData<Attk::C_PixelR8G8B8A8U>(image.GetPaletteDataPtr(), colour, index);
	case Attk::E_PaletteFormat_B8G8R8A8U:
		return SetPaletteEntryData<Attk::C_PixelB8G8R8A8U>(image.GetPaletteDataPtr(), colour, index);
	case Attk::E_PaletteFormat_A8R8G8B8U:
		return SetPaletteEntryData<Attk::C_PixelA8R8G8B8U>(image.GetPaletteDataPtr(), colour, index);
	case Attk::E_PaletteFormat_A8B8G8R8U:
		return SetPaletteEntryData<Attk::C_PixelA8B8G8R8U>(image.GetPaletteDataPtr(), colour, index);
	case Attk::E_PaletteFormat_R8G8B8U:
		return SetPaletteEntryData<Attk::C_PixelR8G8B8U>(image.GetPaletteDataPtr(), colour, index);
	case Attk::E_PaletteFormat_B8G8R8U:
		return SetPaletteEntryData<Attk::C_PixelB8G8R8U>(image.GetPaletteDataPtr(), colour, index);
	default:
		assert(false && "Image is not palettised\n");
	}
}

Attk::C_PixelR32G32B32A32F GetPaletteEntry(Attk::C_Image& image, U32 index)
{
	assert(index<Attk::C_BaseImage::GetNumPaletteEntries(image.GetPixelFormat()));
	switch(image.GetPaletteFormat())
	{
	case Attk::E_PaletteFormat_R8G8B8A8U:
		return Attk::C_PixelR32G32B32A32F(((Attk::C_PixelR8G8B8A8U*)image.GetPaletteDataPtr())[index]);
	case Attk::E_PaletteFormat_B8G8R8A8U:
		return Attk::C_PixelR32G32B32A32F(((Attk::C_PixelB8G8R8A8U*)image.GetPaletteDataPtr())[index]);
	case Attk::E_PaletteFormat_A8R8G8B8U:
		return Attk::C_PixelR32G32B32A32F(((Attk::C_PixelA8R8G8B8U*)image.GetPaletteDataPtr())[index]);
	case Attk::E_PaletteFormat_A8B8G8R8U:
		return Attk::C_PixelR32G32B32A32F(((Attk::C_PixelA8B8G8R8U*)image.GetPaletteDataPtr())[index]);
	case Attk::E_PaletteFormat_R8G8B8U:
		return Attk::C_PixelR32G32B32A32F(((Attk::C_PixelR8G8B8U*)image.GetPaletteDataPtr())[index]);
	case Attk::E_PaletteFormat_B8G8R8U:
		return Attk::C_PixelR32G32B32A32F(((Attk::C_PixelB8G8R8U*)image.GetPaletteDataPtr())[index]);
	default:
		assert(false && "Image is not palettised\n");
	}
	return Attk::C_PixelR32G32B32A32F(0.f,0.f,0.f,0.f);
}



Attk::C_Status C_ImageAdjustment::Adjust(
	Attk::C_Image& image,
	const C_ImageAdjustmentCurve<Point2>& curve,
	E_AdjustmentChannel channel)
{
	C_EvaluateImageAdjustmentCurve<Point2> curveEvaluater;
	curveEvaluater.Initialise(curve, 128);

	return AdjustSingleImage(image, curveEvaluater, channel);
}

Attk::C_Status C_ImageAdjustment::Adjust(
	Attk::C_MipmapImageChain& mipmapChain,
	const C_ImageAdjustmentCurve<Point2>& curve,
	E_AdjustmentChannel channel)
{
	Attk::C_Status status;

	C_EvaluateImageAdjustmentCurve<Point2> curveEvaluater;
	curveEvaluater.Initialise(curve, 256);
	
	for (U32 iLevel=0; iLevel<mipmapChain.GetNumLevels(); ++iLevel)
	{
		status = AdjustSingleImage(*(mipmapChain.GetLevelPtr(iLevel)), curveEvaluater, channel);
		if (!status)
			return status;
	}

	return Attk::E_Status_Success;
}

Attk::C_Status C_ImageAdjustment::AdjustSingleImage(
	Attk::C_Image& image,
	C_EvaluateImageAdjustmentCurve<Point2> curveEvaluater,
	E_AdjustmentChannel channel)
{
	Attk::E_PixelFormat pixelFormat = image.GetPixelFormat();
	if (pixelFormat == Attk::E_PixelFormat_ColourIndex8U)
	{
		// adjust palette data
		U8* pPaletteData = image.GetPaletteDataPtr();
		for (U32 i=0; i<Attk::C_BaseImage::GetNumPaletteEntries(pixelFormat); ++i)
		{
			Attk::C_PixelR32G32B32A32F paletteEntry = GetPaletteEntry(image, i);

			if (channel & E_AdjustmentChannel_Red)
				paletteEntry.red = curveEvaluater.Evaluate(paletteEntry.red).y;
			if (channel & E_AdjustmentChannel_Green)
				paletteEntry.green = curveEvaluater.Evaluate(paletteEntry.green).y;
			if (channel & E_AdjustmentChannel_Blue)
				paletteEntry.blue = curveEvaluater.Evaluate(paletteEntry.blue).y;
			if (channel & E_AdjustmentChannel_Alpha)
				paletteEntry.alpha = curveEvaluater.Evaluate(paletteEntry.alpha).y;

			SetPaletteEntry(image, paletteEntry, i);
		}
	}
	else if (pixelFormat < Attk::E_PixelFormat_Compressed_DXT1)
	{
		for (U32 d=0; d<image.GetDepth(); ++d)
			for (U32 h=0; h<image.GetHeight(); ++h)
				for (U32 w=0; w<image.GetWidth(); ++w)
				{
					Attk::C_PixelR32G32B32A32F pixel = image.GetPixel(w, h, d);

					if (channel & E_AdjustmentChannel_Red)
						pixel.red = curveEvaluater.Evaluate(pixel.red).y;
					if (channel & E_AdjustmentChannel_Green)
						pixel.green = curveEvaluater.Evaluate(pixel.green).y;
					if (channel & E_AdjustmentChannel_Blue)
						pixel.blue = curveEvaluater.Evaluate(pixel.blue).y;
					if (channel & E_AdjustmentChannel_Alpha)
						pixel.alpha = curveEvaluater.Evaluate(pixel.alpha).y;

					image.SetPixel(pixel, w, h, d);
				}
	}
	else
	{
		assert(false && "Unsupported pixel format for image adjustment\n");
		return Attk::E_Status_Failure;
	}
	
	return Attk::E_Status_Success;
}

#ifdef _DEBUG

Attk::C_Status C_ImageAdjustment::OutputDebugImage(
	const char* pFileName,
	const C_ImageAdjustmentCurve<Point2>& curve)
{
	C_EvaluateImageAdjustmentCurve<Point2> curveEvaluater;
	curveEvaluater.Initialise(curve, 256);

	Attk::C_Image image;
	image.NewPixelData(256, 256, 1, Attk::E_PixelFormat_R8G8B8A8U, 0);

	Attk::C_PixelR8G8B8A8U* pPixelData = (Attk::C_PixelR8G8B8A8U*) image.GetPixelDataPtr();

	for (U32 h=0; h<256; ++h)
	{
		for (U32 w=0; w<256; ++w)
		{
			pPixelData[256*h + w].red = w;
			pPixelData[256*h + w].green = w;
			pPixelData[256*h + w].blue = w;
			pPixelData[256*h + w].alpha = w;
		}
	}

	AdjustSingleImage(image, curveEvaluater, E_AdjustmentChannel_All);

	for (U32 w=0; w<256; ++w)
	{
		Attk::C_PixelR8G8B8A8U& pixelData = pPixelData[w];

		U32 value = pixelData.red;

		pPixelData[256*(255-value) + w].red = 255;
		pPixelData[256*(255-value) + w].green = 0;
		pPixelData[256*(255-value) + w].blue = 0;
		pPixelData[256*(255-value) + w].alpha = 255;
	}

	return image.Write(pFileName);

}

#endif

