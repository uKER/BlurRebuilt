#include "stdafx.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <string>

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

#include <core/utility/StringProc.h>

#include <attk/DevILUtility.h>

using namespace Attk;

bool C_DevILPlugin::CanRead(const char* pFileName, C_Status* pStatus) const
{
	std::string fileExt = Tu::FileExtension(pFileName);
	if (!stricmp(fileExt.c_str(), "bmp"))
		return true;
	if (!stricmp(fileExt.c_str(), "cut"))
		return true;
	if (!stricmp(fileExt.c_str(), "dds"))
		return true;
	if (!stricmp(fileExt.c_str(), "gif"))
		return true;
	if (!stricmp(fileExt.c_str(), "ico"))
		return true;
	if (!stricmp(fileExt.c_str(), "cur"))
		return true;
	if (!stricmp(fileExt.c_str(), "jpg"))
		return true;
	if (!stricmp(fileExt.c_str(), "jpe"))
		return true;
	if (!stricmp(fileExt.c_str(), "jpeg"))
		return true;
	if (!stricmp(fileExt.c_str(), "lbm"))
		return true;
	if (!stricmp(fileExt.c_str(), "lif"))
		return true;
	if (!stricmp(fileExt.c_str(), "lmp"))
		return true;
	if (!stricmp(fileExt.c_str(), "mdl"))
		return true;
	if (!stricmp(fileExt.c_str(), "mng"))
		return true;
	if (!stricmp(fileExt.c_str(), "pcd"))
		return true;
	if (!stricmp(fileExt.c_str(), "pcx"))
		return true;
	if (!stricmp(fileExt.c_str(), "pic"))
		return true;
	if (!stricmp(fileExt.c_str(), "pix"))
		return true;
	if (!stricmp(fileExt.c_str(), "png"))
		return true;
	if (!stricmp(fileExt.c_str(), "pbm"))
		return true;
	if (!stricmp(fileExt.c_str(), "pgm"))
		return true;
	if (!stricmp(fileExt.c_str(), "ppm"))
		return true;
	if (!stricmp(fileExt.c_str(), "pnm"))
		return true;
	if (!stricmp(fileExt.c_str(), "psd"))
		return true;
	if (!stricmp(fileExt.c_str(), "pxr"))
		return true;
	if (!stricmp(fileExt.c_str(), "sgi"))
		return true;
	if (!stricmp(fileExt.c_str(), "bw"))
		return true;
	if (!stricmp(fileExt.c_str(), "rgb"))
		return true;
	if (!stricmp(fileExt.c_str(), "rgba"))
		return true;
	if (!stricmp(fileExt.c_str(), "tga"))
		return true;
	if (!stricmp(fileExt.c_str(), "tif"))
		return true;
	if (!stricmp(fileExt.c_str(), "tiff"))
		return true;
	if (!stricmp(fileExt.c_str(), "wal"))
		return true;
	if (!stricmp(fileExt.c_str(), "xpm"))
		return true;
	if (!stricmp(fileExt.c_str(), "hdr"))
		return true;

	return false;
}

bool C_DevILPlugin::CanWrite(const char* pFileName, C_Status* pStatus) const
{
	std::string fileExt = Tu::FileExtension(pFileName);
	if (!stricmp(fileExt.c_str(), "bmp"))
		return true;
	if (!stricmp(fileExt.c_str(), "dds"))
		return true;
	if (!stricmp(fileExt.c_str(), "jpg"))
		return true;
	if (!stricmp(fileExt.c_str(), "pcx"))
		return true;
	if (!stricmp(fileExt.c_str(), "png"))
		return true;
	if (!stricmp(fileExt.c_str(), "pnm"))
		return true;
	if (!stricmp(fileExt.c_str(), "psd"))
		return true;
	if (!stricmp(fileExt.c_str(), "raw"))
		return true;
	if (!stricmp(fileExt.c_str(), "sgi"))
		return true;
	if (!stricmp(fileExt.c_str(), "tga"))
		return true;
	if (!stricmp(fileExt.c_str(), "tif"))
		return true;

	return false;
}

C_Status C_DevILPlugin::ReadHeader(
	C_BaseImage& imageHeader,
	const char* pFileName)
{
	return E_Status_NotImplemented;
}

C_Status C_DevILPlugin::Read(
	C_Image& image,
	const char* pFileName, 
	const C_Options* pOptions,
	C_Progress* pProgress)
{
	C_Status status;
	DevIL::Image ilImage;

	if (!ilImage.Load(pFileName))
	{
		return E_Status_Failure;
	}

	if (!status) return status;

	status = C_DevILUtility::CopyToAttk(ilImage, image);
	if (!status) return status;

	return E_Status_Success;
}

C_Status C_DevILPlugin::Read(
	C_CubeMap& cubeMap,
	const char* pFileName, 
	const C_Options* pOptions,
	C_Progress* pProgress)
{
	C_Status status;
	DevIL::Image ilImage;

	if (!ilImage.Load(pFileName))
	{
		return E_Status_Failure;
	}

	if (!status) return status;

	U32 numImages = ilImage.NumImages() + 1;

	if (numImages != 6)
		return C_Status(E_Status_Failure, "Image is not a cube map");

	ILuint id = ilImage.GetId();

	for (U32 i=0; i<numImages; ++i)
	{
		//Force a rebind to ensure the base image is bound
		ilBindImage(ilImage.GetId());
		ILboolean active = ilImage.ActiveImage(i);

		if (!active)
			return C_Status(E_Status_Failure, "Failed to read face");

		status = C_DevILUtility::CopyToAttk(ilImage, cubeMap[i]);
		if (!status)
			return status;
	}

	//Force a rebind to ensure the base image is bound
	ilBindImage(ilImage.GetId());

	return E_Status_Success;
}

C_Status C_DevILPlugin::Write(
	const C_Image& image,
	const char* pFileName, 
	const C_Options* pOptions,
	C_Progress* pProgress)
{
	C_Status status;

	DevIL::Image ilImage;
	ILenum devILFormat = C_DevILUtility::GetDevILFormat(image);
	if (devILFormat == 0)
	{
		C_Image tmp(image);
		std::string fileExt = Tu::FileExtension(pFileName);
		if (!stricmp(fileExt.c_str(), "hdr"))
			tmp.ChangePixelFormat(Attk::E_PixelFormat_R32G32B32A32F);
		else
			tmp.ChangePixelFormat(Attk::E_PixelFormat_R8G8B8A8U);

		status = C_DevILUtility::CopyToDevIL(tmp, ilImage);
	}
 	else if (devILFormat == IL_FLOAT)
	{
		C_Image tmp(image);
		std::string fileExt = Tu::FileExtension(pFileName);
		if (stricmp(fileExt.c_str(), "hdr"))
			tmp.ChangePixelFormat(Attk::E_PixelFormat_R8G8B8A8U);

		status = C_DevILUtility::CopyToDevIL(tmp, ilImage);
	}
	else if (devILFormat == IL_COLOUR_INDEX)
	{
		C_Image tmp(image);
		std::string fileExt = Tu::FileExtension(pFileName);
		tmp.ChangePixelFormat(Attk::E_PixelFormat_R8G8B8A8U);

		status = C_DevILUtility::CopyToDevIL(tmp, ilImage);
	}
	else
	{
		status = C_DevILUtility::CopyToDevIL(image, ilImage);
	}
	if (!status) return status;

	if (!ilImage.Save(pFileName))
		return C_Status(E_Status_Failure, iluErrorString(ilGetError()));

	return E_Status_Success;
}

#endif
