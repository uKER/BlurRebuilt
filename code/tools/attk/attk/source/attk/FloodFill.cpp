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
#include <attk/attkut/FloodFill.h>

#include <attk/PixelFormats.h>

using namespace Attk;
using namespace AttkUt;


Attk::C_Status C_FloodFill::FloodFill(Attk::C_Image &dest, const Attk::C_Image &src)
{

	Attk::C_Status status;

	if (src.GetChannelsPerPixel() != 4)
		return Attk::C_Status(Attk::E_Status_Failure, "Failed to flood fill texture - no alpha channel\n");

	if (src.GetDepth() > 1)
		return Attk::C_Status(Attk::E_Status_Failure, "Filed to flood fill texture - not 2 dimensional\n");

	status = dest.NewPixelData(src.GetWidth(), src.GetHeight(), 1, src.GetPixelFormat(), NULL);

	const void* srcPixels = src.GetPixelDataPtr();
	void* destPixels = dest.GetPixelDataPtr();


	switch (src.GetPixelFormat())
	{
	case Attk::E_PixelFormat_R8G8B8A8U:
		return PerformFloodFill((C_PixelR8G8B8A8U*)destPixels, (const C_PixelR8G8B8A8U*)srcPixels, src.GetWidth(), src.GetHeight(), E_PixelFormat_R8G8B8A8U);

	case Attk::E_PixelFormat_A8R8G8B8U:
		return PerformFloodFill((C_PixelA8R8G8B8U*)destPixels, (const C_PixelA8R8G8B8U*)srcPixels, src.GetWidth(), src.GetHeight(), E_PixelFormat_A8R8G8B8U);

	case Attk::E_PixelFormat_B8G8R8A8U:
		return PerformFloodFill((C_PixelB8G8R8A8U*)destPixels, (const C_PixelB8G8R8A8U*)srcPixels, src.GetWidth(), src.GetHeight(), E_PixelFormat_B8G8R8A8U);

	case Attk::E_PixelFormat_A8B8G8R8U:
		return PerformFloodFill((C_PixelA8B8G8R8U*)destPixels, (const C_PixelA8B8G8R8U*)srcPixels, src.GetWidth(), src.GetHeight(), E_PixelFormat_A8B8G8R8U);

	case Attk::E_PixelFormat_R32G32B32A32F:
		return PerformFloodFill((C_PixelR32G32B32A32F*)destPixels, (const C_PixelR32G32B32A32F*)srcPixels, src.GetWidth(), src.GetHeight(), E_PixelFormat_R32G32B32A32F);

	case Attk::E_PixelFormat_A32R32G32B32F:
		return PerformFloodFill((C_PixelA32R32G32B32F*)destPixels, (const C_PixelA32R32G32B32F*)srcPixels, src.GetWidth(), src.GetHeight(), E_PixelFormat_A32R32G32B32F);

	case Attk::E_PixelFormat_B32G32R32A32F:
		return PerformFloodFill((C_PixelB32G32R32A32F*)destPixels, (const C_PixelB32G32R32A32F*)srcPixels, src.GetWidth(), src.GetHeight(), E_PixelFormat_B32G32R32A32F);
	default:
		return Attk::C_Status(Attk::E_Status_Failure, "Unknown pixel format\n");
	}
}