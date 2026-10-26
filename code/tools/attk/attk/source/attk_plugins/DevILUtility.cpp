#include "stdafx.h"

#include <il/DevIL.h>

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
#include <attk/CubeMap.h>
#include <attk_plugins/IPlugin.h>
#include <attk_plugins/DevILPlugin.h>

#include <attk/DevILUtility.h>

#ifndef DOXYGEN_SHOULD_SKIP_THIS

namespace Attk
{
	ILenum C_DevILUtility::GetDevILType(const Attk::C_Image& img)
	{
		switch(img.GetPixelFormat())
		{
		case E_PixelFormat_B8G8R8A8U:
		case E_PixelFormat_R8G8B8A8U:
		case E_PixelFormat_A8R8G8B8U:
		case E_PixelFormat_R8G8B8U:
		case E_PixelFormat_B8G8R8U:
		case E_PixelFormat_Luminance8U:
		case E_PixelFormat_ColourIndex8U:
			return IL_UNSIGNED_BYTE;

		case E_PixelFormat_Luminance16U:
			return IL_UNSIGNED_SHORT;

		case E_PixelFormat_B32G32R32A32F:
		case E_PixelFormat_R32G32B32A32F:
		case E_PixelFormat_A32R32G32B32F:
		case E_PixelFormat_R32G32B32F:
		case E_PixelFormat_B32G32R32F:
			return IL_FLOAT;
		}

		ATTK_ASSERT(false);
		return 0;
	}

	ILenum C_DevILUtility::GetDevILFormat(const Attk::C_Image& img)
	{
		switch(img.GetPixelFormat())
		{
		case E_PixelFormat_R8G8B8U:
			return IL_RGB;

		case E_PixelFormat_B8G8R8U:
			return IL_BGR;

		case E_PixelFormat_B8G8R8A8U:
			return IL_BGRA;

		case E_PixelFormat_R32G32B32A32F:
			return IL_RGBA;

		case E_PixelFormat_B32G32R32A32F:
			return IL_BGRA;

		case E_PixelFormat_R32G32B32F:
			return IL_RGB;

		case E_PixelFormat_B32G32R32F:
			return IL_BGR;

		case E_PixelFormat_ColourIndex8U:
			return IL_COLOUR_INDEX;

		case E_PixelFormat_Luminance8U:
		case E_PixelFormat_Luminance16U:
			return IL_LUMINANCE;

		case E_PixelFormat_R8G8B8A8U:
			return IL_RGBA;

		case E_PixelFormat_A32R32G32B32F:
		case E_PixelFormat_A8R8G8B8U:
			return 0;
		};
		ATTK_ASSERT(false);
		return 0;
	}

	C_Status C_DevILUtility::CopyToDevIL(
		const Attk::C_Image& source, 
		DevIL::Image& destination)
	{
		//Aquire DeviL format.
		ILenum ilFormat = GetDevILFormat(source);
		ATTK_ASSERT(ilFormat);
		ILenum ilType = GetDevILType(source);
		ATTK_ASSERT(ilType);
		if (!ilFormat || !ilType)
			return E_Status_Failure;

		if (!destination.TexImage(
			source.GetWidth(),
			source.GetHeight(),
			1,
			source.GetChannelsPerPixel(),
			ilFormat,
			ilType,
			(ILvoid*)source.GetPixelDataPtr()))
			return E_Status_Failure;

		/*
		ILenum ilPaletteFormat;
		switch(source.GetPaletteFormat())
		{
		case PALETTE_FORMAT_NONE:
			break;
		case PALETTE_FORMAT_R8G8B8U:
			ilPaletteFormat = IL_PAL_RGB24;
			break;
		case PALETTE_FORMAT_R8G8B8A8U:
			ilPaletteFormat = IL_PAL_RGBA32;
			break;
		case PALETTE_FORMAT_B8G8R8U:
			ilPaletteFormat = IL_PAL_BGR24;
			break;
		case PALETTE_FORMAT_R8G8B8A8U:
			ilPaletteFormat = IL_PAL_BGRA32;
			break;
		default:
			break;
		};
*/
		//Copy palette from DevIL image.
//		destination.SetPalette(pIlPalette, paletteFormat);

		return E_Status_Success;
	}

	C_Status C_DevILUtility::CopyToAttk(
		const DevIL::Image& source,
		Attk::C_Image& destination)
	{
		C_Status status;

		destination.Clear();
		ILenum ilFormat = source.GetFormat();
		ILenum ilType = source.GetType();
		
		//If DevIL screws up and doesn't understand it should be using 2 bytes per pixel...
		if (ilType == IL_UNSIGNED_BYTE && source.GetBpp() == 2)
		{
			ilType = IL_UNSIGNED_SHORT;
		}

		Attk::E_PixelFormat format;
		if (ilFormat == IL_RGBA && ilType == IL_UNSIGNED_BYTE)
		{
			format = Attk::E_PixelFormat_R8G8B8A8U;
		}
		else if (ilFormat == IL_RGB && ilType == IL_UNSIGNED_BYTE)
		{
			format = Attk::E_PixelFormat_R8G8B8U;
		}

		else if (ilFormat == IL_BGRA && ilType == IL_UNSIGNED_BYTE)
		{
			format = Attk::E_PixelFormat_B8G8R8A8U;
		}
		else if (ilFormat == IL_BGR && ilType == IL_UNSIGNED_BYTE)
		{
			format = Attk::E_PixelFormat_B8G8R8U;
		}
		else if ((ilFormat == IL_BGRA) && ilType == IL_FLOAT)
		{
			format = Attk::E_PixelFormat_B32G32R32A32F;
		}
		else if ((ilFormat == IL_RGBA) && ilType == IL_FLOAT)
		{
			format = Attk::E_PixelFormat_R32G32B32A32F;
		}
		else if ((ilFormat == IL_RGB) && ilType == IL_FLOAT)
		{
			format = Attk::E_PixelFormat_R32G32B32F;
		}
		else if ((ilFormat == IL_LUMINANCE) && ilType == IL_UNSIGNED_BYTE)
		{
			format = Attk::E_PixelFormat_Luminance8U;
		}
		else if ((ilFormat == IL_LUMINANCE) && ilType == IL_UNSIGNED_SHORT)
		{
			format = Attk::E_PixelFormat_Luminance16U;
		}
		else if ((ilFormat == IL_COLOUR_INDEX) && ilType == IL_UNSIGNED_BYTE)
		{
			format = Attk::E_PixelFormat_ColourIndex8U;
		}
		else
		{
			return E_Status_Failure;
		}

		destination.DeletePixelData();
		destination._ResizeDevIL(
			source.GetWidth(),
			source.GetHeight(),
			source.GetDepth());

		status = destination.NewPixelData(
			source.GetWidth(),
			source.GetHeight(), 
			source.GetDepth(),
			format,
			0);
		if (!status) return status;

		ILenum ilTargetFormat;
		switch(format)
		{
		//Integer formats.
		case E_PixelFormat_R8G8B8A8U:
			ilTargetFormat = IL_RGBA;
			break;

		case E_PixelFormat_B8G8R8A8U:
			ilTargetFormat = IL_BGRA;
			break;

		case E_PixelFormat_B8G8R8U:
			ilTargetFormat = IL_BGR;
			break;

		case E_PixelFormat_R8G8B8U:
			ilTargetFormat = IL_RGB;
			break;

		case E_PixelFormat_Luminance8U:
		case E_PixelFormat_Luminance16U:
			ilTargetFormat = IL_LUMINANCE;
			break;

		case E_PixelFormat_ColourIndex8U:
			ilTargetFormat = IL_COLOUR_INDEX;
			break;

		//Floating point formats.
		case E_PixelFormat_R32G32B32A32F:
			ilTargetFormat = IL_RGBA;
			break;

		case E_PixelFormat_B32G32R32A32F:
			ilTargetFormat = IL_BGRA;
			break;

		case E_PixelFormat_B32G32R32F:
			ilTargetFormat = IL_BGR;
			break;

		case E_PixelFormat_R32G32B32F:
			ilTargetFormat = IL_RGB;
			break;

		//We cannot convert directly to the following formats.
		case E_PixelFormat_A8R8G8B8U:
		case E_PixelFormat_A32R32G32B32F:
		default:
			return E_Status_Failure;
		};

		assert(destination.GetBytesPerPixel() == source.GetBpp());

		source.Bind();
		ilCopyPixels(
			0,
			0,
			0,
			source.GetWidth(),
			source.GetHeight(),
			1,
			ilTargetFormat,
			ilType,
			destination.GetPixelDataPtr());

		const ILubyte* pIlPalette = source.GetPalette();
		ILenum ilPaletteType = source.GetPaletteType();

		E_PaletteFormat paletteFormat;
		switch(ilPaletteType)
		{
			case IL_PAL_NONE:
				paletteFormat = E_PaletteFormat_None;
				break;
			case IL_PAL_RGB24:
				paletteFormat = E_PaletteFormat_B8G8R8U;
				break;
			case IL_PAL_RGB32:
				paletteFormat = E_PaletteFormat_R8G8B8A8U;
				break;
			case IL_PAL_RGBA32:
				paletteFormat = E_PaletteFormat_R8G8B8A8U;
				break;
			case IL_PAL_BGR24:
				paletteFormat = E_PaletteFormat_B8G8R8U;
				break;
			case IL_PAL_BGR32:
				paletteFormat = E_PaletteFormat_B8G8R8A8U;
				break;
			case IL_PAL_BGRA32:
				paletteFormat = E_PaletteFormat_B8G8R8A8U;
				break;
			default:
				break;
		}

		if (paletteFormat != E_PaletteFormat_None)
		{
			//Copy palette from DevIL image.
			destination.SetPaletteData(pIlPalette, paletteFormat);
		}
		return E_Status_Success;
	}

	C_Status C_DevILUtility::Resize(
		Attk::C_Image& image, 
		U32 width, 
		U32 height)
	{
		Attk::E_PixelFormat originalPixelFormat = image.GetPixelFormat();

		if (!image.GetPixelDataPtr())
			return Attk::E_Status_Failure;

		Attk::C_Status status;
		DevIL::Image ilImage;

		//Change to intermediate format if we're using a format unsupported by DevIL.
		if (image.GetPixelFormat() == E_PixelFormat_A32R32G32B32F ||
			image.GetPixelFormat() == E_PixelFormat_A8R8G8B8U)
			image.ChangePixelFormat(Attk::E_PixelFormat_R32G32B32A32F);

		status = CopyToDevIL(
			image, 
			ilImage);
		if (!status) return status;


		iluImageParameter(ILU_FILTER, ILU_BILINEAR);
	
		if (!ilImage.Resize(width, height, 1))
			return E_Status_Failure;

		status = CopyToAttk(
			ilImage, 
			image);
		if (!status) return status;

		//Change back from intermediate format if we're using a format unsupported by DevIL.
		if (originalPixelFormat != image.GetPixelFormat())
			image.ChangePixelFormat(originalPixelFormat);

		return E_Status_Success;
	}
}

#endif
