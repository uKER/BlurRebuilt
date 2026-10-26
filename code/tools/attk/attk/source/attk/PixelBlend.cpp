#include "stdafx.h"

#include <attk/Definitions.h>
#include <attk/Enum.h>
#include <attk/Status.h>
#include <attk/Global.h>
#include <attk/Progress.h>
#include <attk/OptionAttribute.h>
#include <attk/Options.h>
#include <attk/BaseImage.h>
#include <attk/PixelFormats.h>
#include <attk/Image.h>
#include <attk/PixelBlend.h>

using namespace Attk;

C_Status C_PixelBlend::Blend(
	const C_PixelR32G32B32A32F* pSource,
	E_BlendMode blendMode,
	F32 alphaMultiplier,
	C_PixelR32G32B32A32F* pDestination,
	U32 nPixels)
{
	//Get a function pointer to our blending function to avoid branch logic within our pixel
	//processing loop.
	PixelBlendFunc pPixelBlendFunc;

	switch (blendMode)
	{
	case E_BlendMode_Additive:
		pPixelBlendFunc = &AdditiveBlend;
		break;

	case E_BlendMode_Multiply:
		pPixelBlendFunc = &MultiplyBlend;
		break;

	case E_BlendMode_None:
		pPixelBlendFunc = &Copy;
		break;

	default:
		return E_Status_Failure;
	};

	for (U32 iPixel = 0; iPixel < nPixels; iPixel++)
	{
		pPixelBlendFunc(
			pSource[iPixel],
			alphaMultiplier,
			pDestination[iPixel]);
	}

	return E_Status_Success;
}

void C_PixelBlend::AdditiveBlend(
	const C_PixelR32G32B32A32F& source,
	F32 opacity,
	C_PixelR32G32B32A32F& destination)
{
	destination.alpha = source.alpha * opacity;
	destination.red = destination.red + source.red * opacity;
	destination.green = destination.green + source.green * opacity;
	destination.blue = destination.blue + source.blue * opacity;
	if (destination.red > 1.f)
		destination.red = 1.f;
	if (destination.green > 1.f)
		destination.green = 1.f;
	if (destination.blue > 1.f)
		destination.blue = 1.f;
	if (destination.alpha > 1.f)
		destination.alpha = 1.f;
}

void C_PixelBlend::MultiplyBlend(
	const C_PixelR32G32B32A32F& source,
	F32 opacity,
	C_PixelR32G32B32A32F& destination)
{
	F32 alpha = source.alpha * opacity;
	destination.red = destination.red * source.red * alpha;
	destination.green = destination.green * source.green * alpha;
	destination.blue = destination.blue * source.blue * alpha;
	destination.alpha = 1.0f;
}

void C_PixelBlend::Copy(
	const C_PixelR32G32B32A32F& source,
	F32 alphaMultiplier,
	C_PixelR32G32B32A32F& destination)
{
	destination.red = source.red;
	destination.green = source.green;
	destination.blue = source.blue;
	destination.alpha = 1.f;
}
