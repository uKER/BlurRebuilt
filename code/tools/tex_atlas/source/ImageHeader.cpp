#include "stdafx.h"
#include <tex_atlas/ImageHeader.h>
#include <tex_atlas/TGAImageHeaderBuilder.h>
#include <tex_atlas/BMPImageHeaderBuilder.h>

using namespace TexAtlas;

ImageHeader::ImageHeader()
{
}

ImageHeader::ImageHeader(const ImageHeader& rhs)
{
}


ImageHeader::~ImageHeader()
{
}

const ImageHeader& ImageHeader::operator=(const ImageHeader& rhs)
{
	m_width = rhs.m_width;
	m_height = rhs.m_height;

	return *this;
}

U32 ImageHeader::Width() const
{
	return m_width;
}

U32 ImageHeader::Height() const
{
	return m_height;
}

Attk::C_Status ImageHeader::Read(const char* pFileName)
{
	std::string fileExtension(Tu::FileExtension(pFileName));
	if (Tu::CompareNoCase(fileExtension, "tga") == 0)
	{
		return TGAImageHeaderBuilder::Build(*this, pFileName);
	}
	if (Tu::CompareNoCase(fileExtension, "bmp") == 0)
	{
		return BMPImageHeaderBuilder::Build(*this, pFileName);
	}

	return Attk::C_Status(Attk::E_Status_Failure, CS_GEN_UNSUPPORTED_FILE_TYPE);
}
