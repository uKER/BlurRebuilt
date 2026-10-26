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
#include <attk/PixelConversion.h>

using namespace Attk;


C_Status C_PixelConversion::CopyToInterchangeFormat(
	const U8* pSourcePixels,
	const C_PixelR32G32B32A32F* pInterchangePalette,
	E_PixelFormat sourceFormat,
	C_PixelR32G32B32A32F* pDestination,
	U32 nPixels)
{
	switch(sourceFormat)
	{
		//Integer formats.
		case E_PixelFormat_R8G8B8A8U:
		{
			const C_PixelR8G8B8A8U* pPixelsToCopy = (const C_PixelR8G8B8A8U*)pSourcePixels;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestination[i].Set(pPixelsToCopy[i]);
		}
		break;

		case E_PixelFormat_A8R8G8B8U:
		{
			const C_PixelA8R8G8B8U* pPixelsToCopy = (const C_PixelA8R8G8B8U*)pSourcePixels;

			for (U32 i = 0; i < nPixels; i++)
				pDestination[i].Set(pPixelsToCopy[i]);
		}
		break;

		case E_PixelFormat_A8B8G8R8U:
		{
			const C_PixelA8B8G8R8U* pPixelsToCopy = (const C_PixelA8B8G8R8U*)pSourcePixels;

			for (U32 i = 0; i < nPixels; i++)
				pDestination[i].Set(pPixelsToCopy[i]);
		}
		break;

		case E_PixelFormat_B8G8R8U:
		{
			const C_PixelB8G8R8U* pPixelsToCopy = (const C_PixelB8G8R8U*)pSourcePixels;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestination[i].Set(pPixelsToCopy[i]);
		}
		break;

		case E_PixelFormat_B8G8R8A8U:
		{
			const C_PixelB8G8R8A8U* pPixelsToCopy = (const C_PixelB8G8R8A8U*)pSourcePixels;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestination[i].Set(pPixelsToCopy[i]);
		}
		break;

		case E_PixelFormat_R8G8B8U:
		{
			const C_PixelR8G8B8U* pPixelsToCopy = (const C_PixelR8G8B8U*)pSourcePixels;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestination[i].Set(pPixelsToCopy[i]);
		}
		break;

		case E_PixelFormat_ColourIndex8U:
		{
			const C_PixelColourIndex8U* pPixelsToCopy = (const C_PixelColourIndex8U*)pSourcePixels;

			for (U32 i = 0; i < nPixels; i++)
				pDestination[i].Set(pPixelsToCopy[i], pInterchangePalette);
		}
		break;

		//Floating point formats.
		case E_PixelFormat_R32G32B32A32F:
		{
			const C_PixelR32G32B32A32F* pPixelsToCopy = (const C_PixelR32G32B32A32F*)pSourcePixels;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestination[i] = pPixelsToCopy[i];
		}
		break;

		case E_PixelFormat_A32R32G32B32F:
		{
			const C_PixelA32R32G32B32F* pPixelsToCopy = (const C_PixelA32R32G32B32F*)pSourcePixels;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestination[i].Set(pPixelsToCopy[i]);
		}
		break;

		case E_PixelFormat_R32G32B32F:
		{
			const C_PixelR32G32B32F* pPixelsToCopy = (const C_PixelR32G32B32F*)pSourcePixels;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestination[i].Set(pPixelsToCopy[i]);
		}
		break;

		case E_PixelFormat_B32G32R32F:
		{
			const C_PixelB32G32R32F* pPixelsToCopy = (const C_PixelB32G32R32F*)pSourcePixels;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestination[i].Set(pPixelsToCopy[i]);
		}
		break;

		case E_PixelFormat_B32G32R32A32F:
		{
			const C_PixelB32G32R32A32F* pPixelsToCopy = (const C_PixelB32G32R32A32F*)pSourcePixels;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestination[i].Set(pPixelsToCopy[i]);
		}
		break;

		case E_PixelFormat_Luminance8U:
		{
			const C_PixelLuminance8U* pPixelsToCopy = (const C_PixelLuminance8U*)pSourcePixels;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestination[i].Set(pPixelsToCopy[i]);
		}
		break;

		case E_PixelFormat_Luminance16U:
		{
			const C_PixelLuminance16U* pPixelsToCopy = (const C_PixelLuminance16U*)pSourcePixels;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestination[i].Set(pPixelsToCopy[i]);
		}
		break;

		case E_PixelFormat_Unknown:
		default:
			return E_Status_Failure;
	};


	return E_Status_Success;
}

C_Status C_PixelConversion::CopyFromInterchangeFormat(
	const C_PixelR32G32B32A32F* pSource,
	U8* pDestination,
	E_PixelFormat destinationFormat,
	U32 nPixels)
{
	switch(destinationFormat)
	{
		case E_PixelFormat_R8G8B8A8U:
		{
			C_PixelR8G8B8A8U* pDestinationPixels = (C_PixelR8G8B8A8U*)pDestination;
		
			for (U32 i = 0; i < nPixels; i++)
				pDestinationPixels[i].Set(pSource[i]);
		}
		break;

		case E_PixelFormat_A8R8G8B8U:
		{
			C_PixelA8R8G8B8U* pDestinationPixels = (C_PixelA8R8G8B8U*)pDestination;
		
			for (U32 i = 0; i < nPixels; i++)
				pDestinationPixels[i].Set(pSource[i]);
		}
		break;

		case E_PixelFormat_A8B8G8R8U:
		{
			C_PixelA8B8G8R8U* pDestinationPixels = (C_PixelA8B8G8R8U*)pDestination;
		
			for (U32 i = 0; i < nPixels; i++)
				pDestinationPixels[i].Set(pSource[i]);
		}
		break;

		case E_PixelFormat_B8G8R8U:
		{
			C_PixelB8G8R8U* pDestinationPixels = (C_PixelB8G8R8U*)pDestination;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestinationPixels[i].Set(pSource[i]);
		}
		break;


		case E_PixelFormat_B8G8R8A8U:
		{
			C_PixelB8G8R8A8U* pDestinationPixels = (C_PixelB8G8R8A8U*)pDestination;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestinationPixels[i].Set(pSource[i]);
		}
		break;

		case E_PixelFormat_R8G8B8U:
		{
			C_PixelR8G8B8U* pDestinationPixels = (C_PixelR8G8B8U*)pDestination;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestinationPixels[i].Set(pSource[i]);
		}
		break;


		case E_PixelFormat_R32G32B32A32F:
		{
			C_PixelR32G32B32A32F* pDestinationPixels = (C_PixelR32G32B32A32F*)pDestination;

			for (U32 i = 0; i < nPixels; i++)
				pDestinationPixels[i] = pSource[i];
		}
		break;

		case E_PixelFormat_A32R32G32B32F:
		{
			C_PixelA32R32G32B32F* pDestinationPixels = (C_PixelA32R32G32B32F*)pDestination;

			for (U32 i = 0; i < nPixels; i++)
				pDestinationPixels[i].Set(pSource[i]);
		}
		break;

		case E_PixelFormat_R32G32B32F:
		{
			C_PixelR32G32B32F* pDestinationPixels = (C_PixelR32G32B32F*)pDestination;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestinationPixels[i].Set(pSource[i]);
		}
		break;

		case E_PixelFormat_B32G32R32A32F:
		{
			C_PixelB32G32R32A32F* pDestinationPixels = (C_PixelB32G32R32A32F*)pDestination;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestinationPixels[i].Set(pSource[i]);
		}
		break;

		case E_PixelFormat_Luminance8U:
		{
			C_PixelLuminance8U* pDestinationPixels = (C_PixelLuminance8U*)pDestination;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestinationPixels[i].Set(pSource[i]);
		}
		break;

		case E_PixelFormat_Luminance16U:
		{
			C_PixelLuminance16U* pDestinationPixels = (C_PixelLuminance16U*)pDestination;
			
			for (U32 i = 0; i < nPixels; i++)
				pDestinationPixels[i].Set(pSource[i]);
		}
		break;

		case E_PixelFormat_ColourIndex8U:
		{
			C_PixelColourIndex8U* pDestinationPixels = (C_PixelColourIndex8U*)pDestination;

//			for (U32 i = 0; i < nPixels; i++)
//				pDestination[i].Set(*pPixelsToCopy, pSourcePalette);
		}
		break;

		case E_PixelFormat_Unknown:
		default:
			return E_Status_Failure;
	};

	return E_Status_Success;
}

