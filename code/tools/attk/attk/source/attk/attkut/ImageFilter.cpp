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

Attk::C_Status C_ImageFilter::Filter2D(
	Attk::C_Image& img,
	E_PixelFilter filter,
	U32 dstFilterWidth,
	U32 dstFilterHeight)
{
	return Filter(img, filter, dstFilterWidth, dstFilterHeight, 1);
}

Attk::C_Status C_ImageFilter::Filter2D(
		Attk::C_Image& img,
		U32 width,
		U32 height,
		U32 depth,
		E_PixelFilter filter,
		U32 dstFilterWidth,
		U32 dstFilterHeight)
{
	return Filter(img, width, height, depth, filter, dstFilterWidth, dstFilterHeight, 1);
}

Attk::C_Status C_ImageFilter::Filter(
	Attk::C_Image& img,
	E_PixelFilter filter,
	U32 dstFilterWidth,
	U32 dstFilterHeight,
	U32 dstFilterDepth)
{
	return Filter(
		img,
		img.GetWidth(),
		img.GetHeight(),
		img.GetDepth(),
		filter,
		dstFilterWidth,
		dstFilterHeight,
		dstFilterDepth);
}

Attk::C_Status C_ImageFilter::Filter(
	Attk::C_Image& img,
	U32 dstWidth, 
	U32 dstHeight,
	U32 dstDepth,
	E_PixelFilter filter,
	U32 dstFilterWidth,
	U32 dstFilterHeight,
	U32 dstFilterDepth)
{
	if (!img.GetPixelDataPtr())
		return Attk::E_Status_Failure;

	Attk::C_Status status;
	Attk::E_PixelFormat srcPixelFormat = img.GetPixelFormat();

	U32 bytesPerPixel = img.GetBytesPerPixel();
	{
		{
			if (!img.GetPixelDataPtr())
				return Attk::E_Status_Failure;

			U32 newSize = dstWidth * dstHeight * dstDepth * bytesPerPixel;
			U8* pNewPixelData = new U8[newSize];
			if (!pNewPixelData)
				return Attk::E_Status_OutOfMemory;

			switch(img.GetPixelFormat())
			{
			//unsigned formats
			case Attk::E_PixelFormat_R8G8B8A8U:
				status = Filter<Attk::C_PixelR8G8B8A8U>(
					pNewPixelData, 
					img,
					dstWidth,
					dstHeight, 
					dstDepth,
					filter,
					dstFilterWidth,
					dstFilterHeight,
					dstFilterDepth);
				if (!status) return status;
				break;

			case Attk::E_PixelFormat_A8R8G8B8U:
				status = Filter<Attk::C_PixelA8R8G8B8U>(
					pNewPixelData, 
					img,
					dstWidth,
					dstHeight, 
					dstDepth,
					filter,
					dstFilterWidth,
					dstFilterHeight,
					dstFilterDepth);
				if (!status) return status;
				break;

			case Attk::E_PixelFormat_A8B8G8R8U:
				status = Filter<Attk::C_PixelA8B8G8R8U>(
					pNewPixelData, 
					img,
					dstWidth,
					dstHeight, 
					dstDepth,
					filter,
					dstFilterWidth,
					dstFilterHeight,
					dstFilterDepth);
				if (!status) return status;
				break;

		case Attk::E_PixelFormat_R8G8B8U:
				status = Filter<Attk::C_PixelR8G8B8U>(
					pNewPixelData, 
					img,
					dstWidth,
					dstHeight, 
					dstDepth,
					filter,
					dstFilterWidth,
					dstFilterHeight,
					dstFilterDepth);
				if (!status) return status;
				break;

		case Attk::E_PixelFormat_B8G8R8U:
				status = Filter<Attk::C_PixelB8G8R8U>(
					pNewPixelData, 
					img,
					dstWidth,
					dstHeight, 
					dstDepth,
					filter,
					dstFilterWidth,
					dstFilterHeight,
					dstFilterDepth);
				if (!status) return status;
				break;

		case Attk::E_PixelFormat_B8G8R8A8U:
			status = Filter<Attk::C_PixelB8G8R8A8U>(
				pNewPixelData, 
				img,
				dstWidth,
				dstHeight, 
				dstDepth,
				filter,
				dstFilterWidth,
				dstFilterHeight,
				dstFilterDepth);
			if (!status) return status;
			break;

		//floating point formats
		case Attk::E_PixelFormat_R32G32B32A32F:
			status = Filter<Attk::C_PixelR32G32B32A32F>(
				pNewPixelData, 
				img,
				dstWidth,
				dstHeight, 
				dstDepth,
				filter,
				dstFilterWidth,
				dstFilterHeight,
				dstFilterDepth);
			if (!status) return status;
			break;

		case Attk::E_PixelFormat_A32R32G32B32F:
				status = Filter<Attk::C_PixelA32R32G32B32F>(
					pNewPixelData, 
					img,
					dstWidth,
					dstHeight, 
					dstDepth,
					filter,
					dstFilterWidth,
					dstFilterHeight,
					dstFilterDepth);
				if (!status) return status;
				break;

			case Attk::E_PixelFormat_R32G32B32F:
				status = Filter<Attk::C_PixelR32G32B32F>(
					pNewPixelData, 
					img,
					dstWidth,
					dstHeight, 
					dstDepth,
					filter,
					dstFilterWidth,
					dstFilterHeight,
					dstFilterDepth);
				if (!status) return status;
				break;

			case Attk::E_PixelFormat_B32G32R32F:
				status = Filter<Attk::C_PixelB32G32R32F>(
					pNewPixelData, 
					img,
					dstWidth,
					dstHeight, 
					dstDepth,
					filter,
					dstFilterWidth,
					dstFilterHeight,
					dstFilterDepth);
				if (!status) return status;
				break;

			case Attk::E_PixelFormat_B32G32R32A32F:
				status = Filter<Attk::C_PixelB32G32R32A32F>(
					pNewPixelData, 
					img,
					dstWidth,
					dstHeight, 
					dstDepth,
					filter,
					dstFilterWidth,
					dstFilterHeight,
					dstFilterDepth);
				if (!status) return status;
				break;
/*
			case Attk::E_PixelFormat_ColourIndex8U:
				status = Filter<Attk::C_PixelColourIndex8U>(
					pNewPixelData, 
					img,
					dstWidth,
					dstHeight, 
					dstDepth,
					filter,
					dstFilterWidth,
					dstFilterHeight,
					dstFilterDepth);
				if (!status) return status;
				break;
*/
			case Attk::E_PixelFormat_Luminance8U:
				status = Filter<Attk::C_PixelLuminance8U>(
					pNewPixelData, 
					img,
					dstWidth,
					dstHeight, 
					dstDepth,
					filter,
					dstFilterWidth,
					dstFilterHeight,
					dstFilterDepth);
				if (!status) return status;
				break;

			case Attk::E_PixelFormat_Luminance16U:
				status = Filter<Attk::C_PixelLuminance16U>(
					pNewPixelData, 
					img,
					dstWidth,
					dstHeight, 
					dstDepth,
					filter,
					dstFilterWidth,
					dstFilterHeight,
					dstFilterDepth);
				if (!status) return status;
				break;

			default:
				assert("!Unsupported format for filtering");
				return Attk::E_Status_Failure;
			};
			
			status = img.MovePixelData(
				dstWidth,
				dstHeight,
				dstDepth,
				img.GetPixelFormat(),
				pNewPixelData);
			if (!status) return status;
		}
	}

	return Attk::E_Status_Success;
}
