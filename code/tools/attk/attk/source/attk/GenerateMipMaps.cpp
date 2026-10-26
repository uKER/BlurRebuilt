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
#include <attk/CubeMap.h>
#include <attk/MipmapImageChain.h>
#include <attk/MipmapPixelDataChain.h>
#include <attk/Utility.h>
#include <attk/attkut/ImageFilter.h>
#include <attk/attkut/ImageResize.h>
#include <attk/attkut/GenerateMipMaps.h>

#include <core/maths.h>

using namespace AttkUt;


Attk::C_Status C_GenerateMipmaps::Generate(
	Attk::C_MipmapImageChain& mipmapChain,
	const Attk::C_Image& image,
	const U32 levels)
{
	Attk::C_Status status;

	mipmapChain.Clear();

	if (!image.GetPixelDataPtr())
		return Attk::E_Status_Failure;

	Attk::C_Image workingCopy(image);

	S32 mipWidth = image.GetWidth();
	S32 mipHeight = image.GetHeight();
	S32 mipDepth = image.GetDepth();
	for (U32 iMipLevel = 0; iMipLevel < levels || levels == 0; iMipLevel++)
	{
		status = AttkUt::C_ImageResize::Resize(
			workingCopy,
			Maths::Max(mipWidth, 1), 
			Maths::Max(mipHeight, 1), 
			Maths::Max(mipDepth, 1));
		if (!status) return status;

		//Upload to mip chain.
		Attk::C_Image* pMipLevel = new Attk::C_Image(workingCopy);
		if (!pMipLevel)
			return Attk::E_Status_OutOfMemory;
		mipmapChain.AddImagePtr(pMipLevel);

		mipWidth >>= 1;
		mipHeight >>= 1;
		mipDepth >>=1;

		if (mipHeight == 0 && mipWidth == 0 && mipDepth == 0)
			break;
	}

	return Attk::E_Status_Success;
}

Attk::C_Status C_GenerateMipmaps::Generate(
	Attk::C_MipmapPixelDataChain& mipmapChain,
	const Attk::C_Image& image,
	const U32 levels)
{
	Attk::C_Status status;
	mipmapChain.Clear();

	if (!image.GetPixelDataPtr())
		return Attk::E_Status_Failure;

	Attk::C_Image workingCopy(image);
	//Perform operation in floating point for precision.
	workingCopy.ChangePixelFormat(Attk::E_PixelFormat_R32G32B32A32F);

	U32 nMipLevels;
	{
		S32 mipWidth = image.GetWidth();
		S32 mipHeight = image.GetHeight();
		S32 mipDepth = image.GetDepth();
		U32 iMipLevel = 1;
		for (; iMipLevel < levels || levels == 0; iMipLevel++)
		{
			mipWidth >>= 1;
			mipHeight >>= 1;
			mipDepth >>= 1;

			if (mipHeight == 0 && mipWidth == 0 && mipDepth == 0)
				break;
		}
		nMipLevels = iMipLevel;
	}
	//Set header info.
	mipmapChain.SetData(
		image.GetWidth(),
		image.GetHeight(),
		image.GetDepth(),
		nMipLevels,
		image.GetPixelFormat(),
		0);

	//Alloc storage.
	U32 dataSize = mipmapChain.GetPixelDataSize();
	U8* pPixelData = new U8[dataSize];

	//Build mip levels.
	{
		S32 mipWidth = image.GetWidth();
		S32 mipHeight = image.GetHeight();
		S32 mipDepth = image.GetDepth();

		U32 offset = 0;
		for (U32 iMipLevel = 0; iMipLevel < levels || levels == 0; iMipLevel++)
		{
			//Upload to mip chain.
			Attk::C_Image mipLevel(workingCopy);
			mipLevel.ChangePixelFormat(image.GetPixelFormat());

			memcpy(pPixelData + offset, mipLevel.GetPixelDataPtr(), mipLevel.GetPixelDataSize());
			
			//Update our data offset.
			offset += mipLevel.GetPixelDataSize();

			mipWidth >>= 1;
			mipHeight >>= 1;
			mipDepth >>=1;

			if (mipHeight == 0 && mipWidth == 0 && mipDepth == 0)
				break;

			status = AttkUt::C_ImageResize::Resize(
				workingCopy,
				Maths::Max(mipWidth, 1),
				Maths::Max(mipHeight, 1),
				Maths::Max(mipDepth, 1));
			if (!status) return status;
		}
		assert(dataSize == offset);
	}

	mipmapChain.SetPixelDataPtr(pPixelData);
	return Attk::E_Status_Success;
}


