#include "stdafx.h"
#include <BTNexgenIPL.h>
#include <BTCImageObject.h>

namespace TexAtlas
{

#define CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, offset) m_pPixelData[(x*m_pixelSizeBytes + y * m_width*m_pixelSizeBytes) + offset]
#define CS_GEN_IMAGE_GET_PIXEL(x, y) m_pPixelData[(x*m_pixelSizeBytes + y * m_width*m_pixelSizeBytes)]

Image::Image()
{
	OneTimeInit();
}

Image::Image(const Image& rhs)
{
	OneTimeInit();
	*this = rhs;
}

Image::~Image()
{
	Clear();
}

const Image& Image::operator=(const Image& rhs)
{
	Clear();

	//Get image size.
	U32 imageSize = rhs.SizeBytes();
	//Create new pixel storage.
	m_pPixelData = new U8[imageSize];
	//Copy pixels.
	memcpy(m_pPixelData, rhs.m_pPixelData, imageSize);

	m_width = rhs.m_width;
	m_height = rhs.m_height;
	m_format = rhs.m_format;
	m_pixelSizeBytes = rhs.m_pixelSizeBytes;

	return *this;
}

Tu::Status Image::New(U32 width, U32 height, ImageFormat format)
{
	Clear();

	m_height = height;
	m_width = width;
	m_pixelSizeBytes = BytesPerPixel(format);
	U32 imageSizeBytes = width * height * m_pixelSizeBytes;
	if (imageSizeBytes > 0)
	{
		m_pPixelData = new U8[imageSizeBytes];
		ZeroMemory(m_pPixelData, imageSizeBytes);
	}
	else
	{
		m_pPixelData = 0;
	}

	m_format = format;
	return Tu::Status::SUCCESS;
}

Tu::Status Image::Read(const std::string& fileName)
{
	if (_access(fileName.c_str(),0) == -1) 
		return Tu::Status(Tu::Status::FAILURE, CS_GEN_FILE_DOES_NOT_EXIST_FAIL);

	BTCImageData* pImageData;
	BTCImageObject imageObject;
	long codecId = imageObject.Load(fileName.c_str());
	if (codecId == -1)
	{
		return Tu::Status(Tu::Status::FAILURE, CS_GEN_FIND_IMAGE_CODEC_FAIL);
	}
	pImageData = imageObject.GetObjectDataPtr();
	
	S32 bpp = pImageData->GetBitsPerPixel();
	if ((bpp < 24) || (bpp > 32))
	{
		if (!pImageData->ConvertTo24BPP())
			// we do not support this number of bits per pixel
			return Tu::Status(Tu::Status::FAILURE, CS_GEN_UNSUPPORTED_IMAGE_BPP);
	}

	CopyFromBTCImageData(pImageData);
	return Tu::Status::SUCCESS;
}

void Image::CopyToBTCImageData(BTCImageData* pImageData) const
{
	pImageData->Create(m_width, m_height, BitsPerPixel());
	for (U32 x = 0; x < (U32)m_width; x++)
	{
		for (U32 y = 0; y < (U32)m_height; y++)
		{
			if (m_format == RGB)
			{
				U8 r = CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::R);
				U8 g = CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::G);
				U8 b = CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::B);

				//BT NexGen stores image data as BGR.
				U8* pBGR = pImageData->GetBits(x,y);
				pBGR[0] = b;
				pBGR[1] = g;
				pBGR[2] = r; 
			} 
			else if (m_format == RGBA)
			{
				U8 r = CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::R);
				U8 g = CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::G);
				U8 b = CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::B);
				U8 a = CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::A);

				//BT NexGen stores image data as BGRA.
				U8* pBGRA = pImageData->GetBits(x,y);
				pBGRA[0] = b;
				pBGRA[1] = g;
				pBGRA[2] = r; 
				pBGRA[3] = a;
			}
		}
	}
}

void Image::CopyFromBTCImageData(BTCImageData* pImageData)
{
	ImageFormat format;
	if (pImageData->GetBitsPerPixel() == 24)
	{
		format = RGB;
	}
	else
	{
		format = RGBA;
	}

	New(pImageData->GetWidth(), pImageData->GetHeight(), format);
	for (U32 x = 0; x < (U32)m_width; x++)
	{
		for (U32 y = 0; y < (U32)m_height; y++)
		{
			if (format == RGB)
			{
				//BT NexGen stores image data as BGR.
				U8* pBGR = pImageData->GetBits(x,y);
				CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::R) = pBGR[2];
				CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::G) = pBGR[1];
				CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::B) = pBGR[0];
			}
			else if (format == RGBA)
			{
				//BT NexGen stores image data as BGRA.
				U8* pBGRA = pImageData->GetBits(x,y);
				CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::R) = pBGRA[2];
				CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::G) = pBGRA[1];
				CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::B) = pBGRA[0];
				CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::A) = pBGRA[3];
			}
		}
	}
}

Tu::Status Image::Write(const std::string& fileName, ImageFileType fileType) const
{
	BTCImageData imageData;
	CopyToBTCImageData(&imageData);

	std::string extension(Tu::FileExtension(fileName));
	
	BTCImageObject imageObject;
	long codecId = imageObject.GetCodecIdFromFormat( _BTTEXT(extension.c_str()), BTCODECTYPE_ENCODER);
	imageObject.Attach(imageData);

	//If TGA, then disable RLE encoding, as people may have written cheap TGA loaders that don't
	//account for this.
	if (codecId == 7)
	{
		BTCEncoderOptionsTGA options;
		options.SetOption(BTCEncoderOptionsTGA::BTEO_TGA_ENABLERLE, false);
		imageObject.Save(fileName.c_str(), codecId, &options);
	}
	else
	{
		imageObject.Save(fileName.c_str(), codecId);
	}

	return Tu::Status::SUCCESS;
}

void Image::Pixel(U32 x, U32 y, U8& r, U8& g, U8& b) const
{
	r = CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::R);
	g = CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::G);
	b = CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::B);
}

void Image::Pixel(U32 x, U32 y, U8& r, U8& g, U8& b, U8& a) const
{
	r = CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::R);
	g = CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::G);
	b = CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::B);
	a = CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, Image::A);
}

U8* Image::Pixel(U32 x, U32 y)
{
	return &CS_GEN_IMAGE_GET_PIXEL(x, y);
}

const U8* Image::Pixel(U32 x, U32 y) const
{
	return &CS_GEN_IMAGE_GET_PIXEL(x, y);
}

const U8* Image::PixelComponent(U32 x, U32 y, Image::Component component) const
{
	return &CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, component);
}

U8* Image::PixelComponent(U32 x, U32 y, Image::Component component)
{
	return &CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x, y, component);
}

void Image::FloodFill(U8 r, U8 g, U8 b)
{
	for (U32 x = 0; x < m_width; x++)
		for (U32 y = 0; y < m_height; y++)
			SetPixel(x, y, r,g,b);
}

void Image::FloodFill(U8 r, U8 g, U8 b, U8 a)
{
	for (U32 x = 0; x < m_width; x++)
		for (U32 y = 0; y < m_height; y++)
			SetPixel(x, y, r,g,b,a);
}

Image::ImageFormat Image::Format() const
{
	return m_format;
}

U32 Image::BytesPerPixel(ImageFormat format) const
{
	switch (format)
	{
	case RGB:
		return 3;
		break;
	case RGBA:
		return 4;
		break;
	default:
		return 0;
		break;
	}
}

U32 Image::BytesPerPixel() const
{
	return m_pixelSizeBytes;
}

U32 Image::BitsPerPixel() const
{
	switch (m_format)
	{
	case RGB:
		return 24;
		break;
	case RGBA:
		return 32;
		break;
	default:
		return 0;
		break;
	}
}

U32 Image::SizeBytes() const
{
	//Calculate the number of bytes to copy.
	return m_width * m_height * BytesPerPixel();
}

U32 Image::Width() const
{
	return m_width;
}

U32 Image::Height() const
{
	return m_height;
}

void Image::SetPixel(U32 x,  U32 y,  const U8* pPixel, ImageFormat format)
{
	if (format == RGB)
	{
		CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x,y, Image::R) = pPixel[0];
		CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x,y, Image::G) = pPixel[1];
		CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x,y, Image::B) = pPixel[2];
	}
	else if (format == RGBA)
	{
		CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x,y, Image::R) = pPixel[0];
		CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x,y, Image::G) = pPixel[1];
		CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x,y, Image::B) = pPixel[2];
		if (m_format == RGBA)
			CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x,y, Image::A) = pPixel[3];
		else
			CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x,y, Image::A) = 0;
	}
}

void Image::SetPixel(U32 x, U32 y, U8 r, U8 g, U8 b, U8 a)
{
	CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x,y, Image::R) = r;
	CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x,y, Image::G) = g;
	CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x,y, Image::B) = b;
	CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x,y, Image::A) = a;
}

void Image::SetPixel(U32 x, U32 y, U8 r, U8 g, U8 b)
{
	CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x,y, Image::R) = r;
	CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x,y, Image::G) = g;
	CS_GEN_IMAGE_GET_PIXEL_COMPONENT(x,y, Image::B) = b;
}

void Image::GrowBorder(U32 size, bool wrapTexture)
{
	assert(m_width != 1);
	assert(m_height != 1);
	Image newImage;
	U32 newWidth = m_width+size*2;
	U32 newHeight = m_height+size*2;
	newImage.New(newWidth, newHeight, m_format);

	U32 x,y;
	//Copy existing image in to the middle a new image.
	for (x = 0; x < m_width; x++)
		for (y = 0; y < m_height; y++)
			newImage.SetPixel(x+size, y+size, &CS_GEN_IMAGE_GET_PIXEL(x,y), m_format);

	//Copy corner pixels.
	for (x = 0; x < size; x++)
		for (y = 0; y < size; y++)
		{
			newImage.SetPixel(0+x,0+y, &CS_GEN_IMAGE_GET_PIXEL(0,0), m_format);
			newImage.SetPixel(newWidth-size+x,0+y, &CS_GEN_IMAGE_GET_PIXEL((m_width-size),0), m_format);
			newImage.SetPixel(0+x,newHeight-size+y, &CS_GEN_IMAGE_GET_PIXEL(0,(m_height-size)), m_format);
			newImage.SetPixel(newWidth-size+x,newHeight-size+y, &CS_GEN_IMAGE_GET_PIXEL((m_width-size),(m_height-size)), m_format);
		}

	if (wrapTexture)
	{
		//Copy top border pixels
		for (x = 0; x < m_width; x++)
			for (y = 0; y < size; y++)
	#ifdef _DEBUG
				newImage.SetPixel(x+size, newHeight-size+y, 0,0,255, m_format);
	#else
				newImage.SetPixel(x+size, newHeight-size+y, &CS_GEN_IMAGE_GET_PIXEL(x,(m_height-size)), m_format);
	#endif

		//Copy bottom border pixels
		for (x = 0; x < m_width; x++)
			for (y = 0; y < size; y++)
	#ifdef _DEBUG
				newImage.SetPixel(x+size, 0+y, 0,0,255);
	#else
				newImage.SetPixel(x+size, 0+y, &CS_GEN_IMAGE_GET_PIXEL(x,0), m_format);
	#endif

		//Copy left border pixels
		for (x = 0; x < size; x++)
			for (y = 0; y < m_height; y++)
	#ifdef _DEBUG
				newImage.SetPixel(newWidth-size+x, y+size, 0,0,255);
	#else
				newImage.SetPixel(newWidth-size+x, y+size, &CS_GEN_IMAGE_GET_PIXEL((m_width-size),y), m_format);
	#endif
			
		//Copy right border pixels
		for (x = 0; x < size; x++)
			for (y = 0; y < m_height; y++)
	#ifdef _DEBUG
				newImage.SetPixel(x, y+size, 0,0,255);
	#else
				newImage.SetPixel(x, y+size, &CS_GEN_IMAGE_GET_PIXEL(0,y), m_format);
	#endif
	}
	else
	{
		//Copy top border pixels
		for (x = 0; x < m_width; x++)
			for (y = 0; y < size; y++)
	#ifdef _DEBUG
				newImage.SetPixel(x+size, 0+y, 0,0,255);
	#else
				newImage.SetPixel(x+size, 0+y, &CS_GEN_IMAGE_GET_PIXEL(x,0), m_format);
	#endif

		//Copy bottom border pixels
		for (x = 0; x < m_width; x++)
			for (y = 0; y < size; y++)
	#ifdef _DEBUG
				newImage.SetPixel(x+size, newHeight-size+y, 0,0,255, m_format);
	#else
				newImage.SetPixel(x+size, newHeight-size+y, &CS_GEN_IMAGE_GET_PIXEL(x,(m_height-size)), m_format);
	#endif

		//Copy left border pixels
		for (x = 0; x < size; x++)
			for (y = 0; y < m_height; y++)
	#ifdef _DEBUG
				newImage.SetPixel(x, y+size, 0,0,255);
	#else
				newImage.SetPixel(x, y+size, &CS_GEN_IMAGE_GET_PIXEL(0,y), m_format);
	#endif
			
		//Copy right border pixels
		for (x = 0; x < size; x++)
			for (y = 0; y < m_height; y++)
	#ifdef _DEBUG
				newImage.SetPixel(newWidth-size+x, y+size, 0,0,255);
	#else
				newImage.SetPixel(newWidth-size+x, y+size, &CS_GEN_IMAGE_GET_PIXEL((m_width-size),y), m_format);
	#endif
	}

	//Clear current image.
	delete[] m_pPixelData;
	//Get a pointer to new pixel data.
	m_pPixelData = newImage.m_pPixelData;
	//Set to zero so that it isn't killed by newImage destructor.
	newImage.m_pPixelData = 0;
	//Assume new width and height.
	m_width = newWidth;
	m_height = newHeight;
}

void Image::Rotate90CW()
{
	Image newImage;
	newImage.New(m_height, m_width, m_format);
	U32 iDestRow = 0;
	for (U32 iSrcColumn = 0; iSrcColumn < m_width; iSrcColumn++)
	{
		U32 jDestColumn = m_height-1;
		for (U32 jSrcRow = 0; jSrcRow < m_height; jSrcRow++)
		{
			newImage.SetPixel(jDestColumn, iDestRow, Pixel(iSrcColumn, jSrcRow), m_format);
			jDestColumn--;
		}
		iDestRow++;
	}

	//Clear current image.
	delete[] m_pPixelData;
	//Get a pointer to new pixel data.
	m_pPixelData = newImage.m_pPixelData;
	//Set to zero so that it isn't killed by newImage destructor.
	newImage.m_pPixelData = 0;
	//Assume new width and height.
	m_width = newImage.m_width;
	m_height = newImage.m_height;
}

void Image::Rotate90CCW()
{
	Image newImage;
	newImage.New(m_height, m_width, m_format);
	U32 iDestRow = m_width-1;
	for (U32 iSrcColumn = 0; iSrcColumn < m_width; iSrcColumn++)
	{
		U32 jDestColumn = 0;
		for (U32 jSrcRow = 0; jSrcRow < m_height; jSrcRow++)
		{
			newImage.SetPixel(jDestColumn, iDestRow, Pixel(iSrcColumn, jSrcRow), m_format);
			jDestColumn++;
		}
		iDestRow--;
	}

	//Clear current image.
	delete[] m_pPixelData;
	//Get a pointer to new pixel data.
	m_pPixelData = newImage.m_pPixelData;
	//Set to zero so that it isn't killed by newImage destructor.
	newImage.m_pPixelData = 0;
	//Assume new width and height.
	m_width = newImage.m_width;
	m_height = newImage.m_height;
}

void Image::Resize(U32 newWidth, U32 newHeight)
{
	assert(newWidth < 100000);
	assert(newHeight < 100000);
	BTCImageData imageData;
	CopyToBTCImageData(&imageData);
	imageData.Resize(newWidth,newHeight);
	CopyFromBTCImageData(&imageData);
}

///Insert an image at the specified location.
void Image::Insert(U32 insertX, U32 insertY, const Image& rhs)
{
	assert((insertX) < m_width);
	assert((insertY) < m_height);
	assert((insertX + rhs.Width()) <= m_width);
	assert((insertY + rhs.Height()) <= m_height);
	//Copy existing image into this image.
	for (U32 x = 0; x < rhs.Width(); x++)
		for (U32 y = 0; y < rhs.Height(); y++)
			SetPixel(x+insertX, y+insertY, rhs.Pixel(x,y), rhs.Format());
}

void Image::Clear()
{
	if (m_pPixelData)
	{
		delete[] m_pPixelData;
		m_pPixelData = 0;
	}
	m_width = 0;
	m_height = 0;
	m_format = UNKNOWN;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

void Image::OneTimeInit()
{
	m_pPixelData = 0;
	m_pixelSizeBytes = 0;
	m_width = 0;
	m_height = 0;
	m_format = UNKNOWN;
}

void Image::Serialise(Tu::GenericDiskWriter* pWriter) const
{
	*pWriter << (U32)m_format;
	*pWriter << m_width;
	*pWriter << m_height;
	if (m_width * m_height > 0)
	{
		pWriter->BlockWrite(m_pPixelData, SizeBytes());
	}
}

void Image::Deserialise(Tu::GenericDiskReader* pReader)
{
	Clear();
	*pReader >> (U32&)m_format;
	*pReader >> m_width;
	*pReader >> m_height;
	New(m_width, m_height, m_format);
	if (m_width * m_height > 0)
	{
		pReader->BlockRead(m_pPixelData, SizeBytes());
	}
}

const U8* Image::Data() const
{
	return m_pPixelData;
}
/*
void Image::ConvertToNormalMap()
{
	static const U32 pixelSizeBytes = 4;
	#define CS_NORM_IMAGE_GET_PIXEL(x, y) pData[(x*pixelSizeBytes + y * m_width*pixelSizeBytes)]

	ImageManipulator::Pixel_A8R8G8B8* pData = new 
		ImageManipulator::Pixel_A8R8G8B8[m_width * m_height];

	for (U32 x = 0; x < m_width; x++)
	{
		for (U32 y = 0; y < m_height; y++)
		{
			Pixel(
				x,
				y,
				CS_NORM_IMAGE_GET_PIXEL(x,y).red,
				CS_NORM_IMAGE_GET_PIXEL(x,y).green,
				CS_NORM_IMAGE_GET_PIXEL(x,y).blue,
				CS_NORM_IMAGE_GET_PIXEL(x,y).alpha);
		}
	}

	ImageManipulator::GenerateNormalMap(pData, m_width, m_height);

	for (U32 x = 0; x < m_width; x++)
	{
		for (U32 y = 0; y < m_height; y++)
		{
			SetPixel(
				x,
				y,
				CS_NORM_IMAGE_GET_PIXEL(x,y).red, 
				CS_NORM_IMAGE_GET_PIXEL(x,y).green, 
				CS_NORM_IMAGE_GET_PIXEL(x,y).blue,
				CS_NORM_IMAGE_GET_PIXEL(x,y).alpha);
		}
	}

	delete pData;
	#undef CS_NORM_IMAGE_GET_PIXEL
}
*/
#endif
}

