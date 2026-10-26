#include "stdafx.h"
#include "Display/Texture/TGA/TGATexture2DUtils.h"
#include "Display/Utils/File.h"

namespace NewDisplay
{
namespace TGA
{

#define RED 0X000000FF
#define GREEN 0X0000FF00
#define BLUE 0X00FF0000
#define ALPHA 0XFF000000

class Image
{
public:
	int m_size;
	unsigned char * m_pData; //Imaga Data
	int m_width; //Image Width
	int m_height; //Image Height
	int m_bytesPerPixel; //Image bytes per pixel
	char* m_sName; //Image Name

	Image();
	Image(Image& in_image); //copy constructor
	Image(BYTE* in_pImgData, int in_width, int in_height, int in_bytesPerPixel);
	virtual ~Image();
	virtual void Set(BYTE* in_pImgData, int in_width, int in_height, int in_bytesPerPixel, char* in_sName = NULL);
	virtual void Unload(); //delete Image data
	virtual bool Load(LPCTSTR in_sName, bool in_bColorKey = false);
	//color key is black

private:
	bool LoadTGA();
};

Image::Image()
{
	m_pData = NULL;
	m_sName = NULL;
	m_height = 0;
	m_width = 0;
	m_bytesPerPixel = 0;
}

Image::~Image()
{
	Unload();
}

Image::Image(BYTE* in_pImgData, int in_width, int in_height, int in_bytesPerPixel)
{
	m_pData = in_pImgData;
	m_sName = NULL;
	m_height = in_height;
	m_width = in_width;
	m_bytesPerPixel = in_bytesPerPixel;
}

Image::Image(Image& in_image)
{
	if(in_image.m_sName)
	{
		const int temp = strlen(in_image.m_sName) + 1;
		m_sName = new char[temp];
		strcpy_s(m_sName,temp,in_image.m_sName);
	}
	m_height = in_image.m_height;
	m_width = in_image.m_width;
	m_bytesPerPixel = in_image.m_bytesPerPixel;
	
	//copy buffer
	int length = m_bytesPerPixel*m_height*m_width;
	m_pData = new unsigned char[length];
	memcpy(m_pData,in_image.m_pData,length);
}

void Image::Unload()
{
	//if(m_pData)
		//delete [] m_pData;

	m_pData = NULL;
	
	if(m_sName)
		delete [] m_sName;
	
	m_sName = NULL;
}

void Image::Set(BYTE* in_pImgData, int in_width, int in_height, int in_bytesPerPixel, char* in_sName)
{
	Unload();
	m_pData = in_pImgData;
	if(in_sName)
	{
		const int temp = strlen(in_sName)+1;
		m_sName = new char[temp];
		strcpy_s(m_sName,temp,in_sName);
	}
	m_height = in_height;
	m_width = in_width;
	m_bytesPerPixel = in_bytesPerPixel;
}

bool Image::Load(LPCTSTR in_sName, bool in_bColorKey)
{
	// Make Sure A Filename Was Given and no image is already loaded
	if (!in_sName || m_pData)
		return false;
	
	const int size = strlen(in_sName) + 1;
	m_sName = new char[size];
	strcpy_s(m_sName,size,in_sName);
	
	//is image .bmp or .tga?
	char * pDot = strrchr(m_sName,'.');
	bool bOK = false;

	if (strcmp(pDot,".tga") == 0)
	{
		bOK = LoadTGA();
	}
	else
		return false;//Error("Incorrect Image Format: %s",m_sName);

	return bOK;
}

// Load uncompressed TGA's only
// tga files created in paintshop pro will not load
bool Image::LoadTGA()
{
#if 1
	return false;
#else
/* 
TGA Header Description
id (byte) 0
colour map type (byte) 1
image type (byte) 2
colour map first entry (byte) 3,4
colour map length (2 bytes) 5,6
map entry size (byte) 7
horizontal origin (2 bytes) 8,9
vertical origin (2 bytes) 10,11
width (2 bytes) 12,13
height (2 bytes) 14,15
pixel depth (byte) 16
image descriptor (byte) 17

  image type:
  1 - colour map image
  2 - RGB(A) uncompressed
  3 - greyscale uncompressed
  9 - greyscale RLE (Runlength encoded) not supported
  10 - RGB(A) RLE (Runlength encoded) not supported
  11 - Compressed, black and white images. not supported
  32 - Compressed color-mapped data, using Huffman, Delta, and
  runlength encoding. not supported
  33 - Compressed color-mapped data, using Huffman, Delta, and
  runlength encoding. 4-pass quadtree-type process. not
  supported
  
	 more information at
	 http://astronomy.swin.edu.au/pbourke/dataformats/tga/
	*/
	
	BYTE header[18];
	int imageSize;
	//int temp;
	FILE *File = fopen(m_sName, "rb");
		
	if( File == NULL || fread(header,1,sizeof(header),File)!= sizeof(header) ||
	(header[2] < 0 && header[2] > 3) )
	{
		if(File)
			fclose(File);
		return false;
	}
		
	m_width = header[13] * 256 + header[12];
	m_height = header[15] * 256 + header[14];

	if( m_width <= 0 || m_height <= 0)
	{
		fclose(File);
		return false;//return Error("Incorrect TGA: %s:", m_sName);
	}
	
	m_bytesPerPixel = header[16]/8;
	imageSize = m_width*m_height*m_bytesPerPixel;
				
	m_pData = new unsigned char[imageSize];
	m_size = imageSize;
				
	if(fread(m_pData, 1, imageSize, File) != imageSize)
	{
		delete [] m_pData;
		fclose (File);
		return false;//return Error("Incorrect TGA: %s:", m_sName);
	}

	//// Swap Red and Blue
	//for(int i=0; i<int(imageSize); i+=m_bytesPerPixel)
	//{
	//	temp=m_pData[i];
	//	m_pData[i] = m_pData[i + 2];
	//	m_pData[i + 2] = temp;
	//}
					
	fclose (File);
	return true;
#endif
}

bool LoadTexture2DFromFile(const Util::C_String& fileName, C_Texture2DParams* pParams)
{
	assert(pParams);

	Image image;
	image.Load(fileName.Chars());

	pParams->m_Width = image.m_width;
	pParams->m_Height = image.m_height;
	pParams->m_Format = E_Format_A8R8G8B8;
	pParams->m_Levels = 1;
	pParams->m_Size = image.m_size;
	pParams->m_pData = image.m_pData;

	return true;
}

}; // namespace TGA
}; // namespace NewDisplay
