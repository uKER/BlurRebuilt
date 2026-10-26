#include "stdafx.h"
#include <tex_atlas/ImageHeader.h>
#include <tex_atlas/TGAImageHeaderBuilder.h>

#define TGA_IMAGE_SPECIFICATION_OFFSET 8

using namespace TexAtlas;

struct TGAImageSpecification
{
	U16 xOrigin;
	U16 yOrigin;
	U16 width;
	U16 height;
	U8 pixelDepth;
	U8 imageDescriptor;
};

Attk::C_Status TGAImageHeaderBuilder::Build(ImageHeader& header, const char* pFileName)
{
	header.m_height = 0;
	header.m_width = 0;

	std::ifstream fileStream;
	fileStream.open(pFileName, std::ios::binary);
	if (!fileStream.is_open())
	{
		return Attk::E_Status_Failure;
	}

	//Attempt to read header
	TGAImageSpecification imageSpecification;
	try
	{
		fileStream.exceptions( std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit );
		fileStream.seekg(TGA_IMAGE_SPECIFICATION_OFFSET, std::ios_base::beg);
		fileStream.read((char*)&imageSpecification, sizeof(TGAImageSpecification));
	}
	catch (std::ifstream::failure)
	{
		fileStream.close();
		return Attk::E_Status_Failure;
	}

	header.m_height = imageSpecification.height;
	header.m_width = imageSpecification.width;

	fileStream.close();
	return Attk::E_Status_Success;
}

