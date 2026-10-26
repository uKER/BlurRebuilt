#include "stdafx.h"

#include "Definitions.h"
#include <attk/Enum.h>
#include <attk/Status.h>
#include <attk/Global.h>
#include <attk/Progress.h>
#include <attk/OptionAttribute.h>
#include <attk/Options.h>
#include <attk/BaseImage.h>
#include <attk/PixelFormats.h>
#include <attk/Image.h>
#include "PaletteConversion.h"

using namespace Attk;

C_Status C_PaletteConversion::CopyToInterchangeFormat(
	const U8* pSource,
	E_PaletteFormat sourceFormat,
	C_PixelR32G32B32A32F* pDestination,
	U32 nPixels)
{
	switch(sourceFormat)
	{
		//Integer formats.
		case E_PaletteFormat_R8G8B8A8U:
		{
			const C_PixelR8G8B8A8U* pDataToCopy = (const C_PixelR8G8B8A8U*)pSource;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestination[i].Set(pDataToCopy[i]);
		}
		break;

		case E_PaletteFormat_B8G8R8A8U:
		{
			const C_PixelB8G8R8A8U* pDataToCopy = (const C_PixelB8G8R8A8U*)pSource;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestination[i].Set(pDataToCopy[i]);
		}
		break;

		case E_PaletteFormat_A8R8G8B8U:
		{
			const C_PixelA8R8G8B8U* pDataToCopy = (const C_PixelA8R8G8B8U*)pSource;

			for (U32 i = 0; i < nPixels; i++)
				pDestination[i].Set(pDataToCopy[i]);
		}
		break;

		case E_PaletteFormat_A8B8G8R8U:
		{
			const C_PixelA8B8G8R8U* pDataToCopy = (const C_PixelA8B8G8R8U*)pSource;

			for (U32 i = 0; i < nPixels; i++)
				pDestination[i].Set(pDataToCopy[i]);
		}
		break;

		case E_PaletteFormat_R8G8B8U:
		{
			const C_PixelR8G8B8U* pDataToCopy = (const C_PixelR8G8B8U*)pSource;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestination[i].Set(pDataToCopy[i]);
		}
		break;

		case E_PaletteFormat_B8G8R8U:
		{
			const C_PixelB8G8R8U* pDataToCopy = (const C_PixelB8G8R8U*)pSource;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestination[i].Set(pDataToCopy[i]);
		}
		break;

		case E_PaletteFormat_Unknown:
		case E_PaletteFormat_None:
		default:
			return E_Status_Failure;
	};

	return E_Status_Success;		
}

C_Status C_PaletteConversion::CopyFromInterchangeFormat(
	const C_PixelR32G32B32A32F* pSource,
	U8* pDestination,
	E_PaletteFormat destinationFormat,
	U32 nPixels)
{
	switch(destinationFormat)
	{
		case E_PaletteFormat_R8G8B8A8U:
		{
			C_PixelR8G8B8A8U* pDestinationPixels = (C_PixelR8G8B8A8U*)pDestination;
		
			for (U32 i = 0; i < nPixels; i++)
				pDestinationPixels[i].Set(pSource[i]);
		}
		break;

		case E_PaletteFormat_B8G8R8A8U:
		{
			C_PixelB8G8R8A8U* pDestinationPixels = (C_PixelB8G8R8A8U*)pDestination;
		
			for (U32 i = 0; i < nPixels; i++)
				pDestinationPixels[i].Set(pSource[i]);
		}
		break;

		case E_PaletteFormat_A8R8G8B8U:
		{
			C_PixelA8R8G8B8U* pDestinationPixels = (C_PixelA8R8G8B8U*)pDestination;
		
			for (U32 i = 0; i < nPixels; i++)
				pDestinationPixels[i].Set(pSource[i]);
		}
		break;

		case E_PaletteFormat_A8B8G8R8U:
		{
			C_PixelA8B8G8R8U* pDestinationPixels = (C_PixelA8B8G8R8U*)pDestination;
		
			for (U32 i = 0; i < nPixels; i++)
				pDestinationPixels[i].Set(pSource[i]);
		}
		break;

		case E_PaletteFormat_R8G8B8U:
		{
			C_PixelR8G8B8U* pDestinationPixels = (C_PixelR8G8B8U*)pDestination;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestinationPixels[i].Set(pSource[i]);
		}
		break;

		case E_PaletteFormat_B8G8R8U:
		{
			C_PixelB8G8R8U* pDestinationPixels = (C_PixelB8G8R8U*)pDestination;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestinationPixels[i].Set(pSource[i]);
		}
		break;

		case E_PaletteFormat_Unknown:
		case E_PaletteFormat_None:
			default:
			return E_Status_Failure;
	};

	return E_Status_Success;
}


