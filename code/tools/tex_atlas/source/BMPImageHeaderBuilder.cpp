#include "stdafx.h"
#include <tex_atlas/ImageHeader.h>
#include <tex_atlas/BMPImageHeaderBuilder.h>

using namespace TexAtlas;

Attk::C_Status BMPImageHeaderBuilder::Build(ImageHeader& header, const char* pFileName)
{
	header.m_height = 0;
	header.m_width = 0;

	std::ifstream fileStream;
	fileStream.open(pFileName, std::ios::binary);
	if (!fileStream.is_open())
	{
		return Attk::E_Status_Failure;
	}

	BITMAPFILEHEADER bmpHeader; //our bitmap file header
	BITMAPINFOHEADER bmpImageInfo;
	//Attempt to read header
	try
	{
		fileStream.seekg(0, std::ios_base::beg);
		fileStream.read((char*)&bmpHeader, sizeof(BITMAPFILEHEADER));
		fileStream.read((char*)&bmpImageInfo, sizeof(BITMAPINFOHEADER));
	}
	catch (std::ifstream::failure e)
	{
		fileStream.close();
		return Attk::E_Status_Failure;
	}

	header.m_height = bmpImageInfo.biHeight;
	header.m_width = bmpImageInfo.biWidth;

	fileStream.close();
	return Attk::E_Status_Success;
}

