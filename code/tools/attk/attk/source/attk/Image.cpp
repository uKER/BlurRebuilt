#include "stdafx.h"

#include <attk/Definitions.h>
#include <attk/Enum.h>
#include <attk/Status.h>
#include <attk/Global.h>
#include <attk/Utility.h>
#include <attk/Progress.h>
#include <attk/OptionAttribute.h>
#include <attk/Options.h>
#include <attk/BaseImage.h>
#include <attk/PixelFormats.h>
#include <attk/Image.h>
#include <attk/CubeMap.h>
#include <attk/PixelConversion.h>
#include <attk/PaletteConversion.h>
#include <attk/PixelBlend.h>
#include <attk/Utility.h>

//Plugins
#include <attk_plugins/IPlugin.h>
#include <attk_plugins/DevILPlugin.h>

#include <il/DevIL.h>
#include <attk/DevILUtility.h>

#include <system/Serialise.h>

using namespace Attk;

C_Image::C_Image()
{
	_OneTimeInit();
}

C_Image::C_Image(const C_Image& rhs)
{
	_OneTimeInit();
	*this = rhs;
}

C_Image::~C_Image()
{
	Clear();
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
void C_Image::_OneTimeInit()
{
	C_BaseImage::_OneTimeInit();

	m_pPixelData = 0;
	m_pPaletteData = 0;
}
#endif

const C_Image& C_Image::operator=(const C_Image& rhs)
{
	//Clear pixel data.
	delete[] m_pPixelData;

	///Clear palette data.
	delete[] m_pPaletteData;
	m_pPaletteData = 0;

	//Copy base class contents.
	C_BaseImage::operator =(rhs);

	///Copy pixel data.

	//Allocate storage.
	{
		U32 pixelDataSize = rhs.GetPixelDataSize();
		//Allocate pixel storage.
		m_pPixelData = new U8[pixelDataSize];
		//Copy pixel data.
		memcpy(m_pPixelData, rhs.m_pPixelData, pixelDataSize);
	}

	//Copy palette data
	if (rhs.m_pPaletteData)
	{
		U32 paletteDataSize = rhs.GetPaletteDataSize();
		m_pPaletteData = new U8[paletteDataSize];
		memcpy(m_pPaletteData, rhs.m_pPaletteData, paletteDataSize);
	}
	
	return *this;
}

U8* C_Image::GetPixelDataPtr(
	U32 x, 
	U32 y,
	U32 z)
{
	U32 bytesPerPixel = GetBytesPerPixel();

	U32 index = Attk::GetPixelIndex(
		bytesPerPixel,
		GetWidth(),
		GetHeight(),
		GetDepth(),
		x, 
		y,
		z);
	ATTK_ASSERT(index < GetPixelDataSize());

	return &(m_pPixelData[index]);
}

const U8* C_Image::GetPixelDataPtr(
	U32 x, 
	U32 y,
	U32 z) const
{
	return (const_cast<C_Image*>(this))->GetPixelDataPtr(
		x,
		y,
		z);
}

U32 C_Image::GetPixelDataSize() const
{
	if (m_pixelFormat == E_PixelFormat_Compressed_DXT1 ||
		m_pixelFormat == E_PixelFormat_Compressed_DXT2 ||
		m_pixelFormat == E_PixelFormat_Compressed_DXT3 ||
		m_pixelFormat == E_PixelFormat_Compressed_DXT4 ||
		m_pixelFormat == E_PixelFormat_Compressed_DXT5 ||
		m_pixelFormat == E_PixelFormat_Compressed_DXN)
		return Utility::GetSlicePitch(GetWidth(), GetHeight(), m_pixelFormat) * GetDepth();
	else
		return GetWidth() * GetHeight() * GetDepth() * GetBytesPerPixel();
}

void C_Image::SetPixel(
	const C_PixelR32G32B32A32F& pixel,
	U32 x, 
	U32 y,
	U32 z)
{
	switch(m_pixelFormat)
	{
	case E_PixelFormat_R8G8B8A8U:
	{
		C_PixelR8G8B8A8U* pPixel = 
			(C_PixelR8G8B8A8U*)GetPixelDataPtr(x, y, z);
		pPixel->Set(pixel);
		break;
	}
	case E_PixelFormat_A8R8G8B8U:
	{
		C_PixelA8R8G8B8U* pPixel = 
			(C_PixelA8R8G8B8U*)GetPixelDataPtr(x, y, z);
		pPixel->Set(pixel);
		break;
	}
	case E_PixelFormat_A8B8G8R8U:
	{
		C_PixelA8B8G8R8U* pPixel = 
			(C_PixelA8B8G8R8U*)GetPixelDataPtr(x, y, z);
		pPixel->Set(pixel);
		break;
	}
	case E_PixelFormat_R8G8B8U:
	{
		C_PixelR8G8B8U* pPixel = 
			(C_PixelR8G8B8U*)GetPixelDataPtr(x, y, z);
		pPixel->Set(pixel);
		break;
	}
	case E_PixelFormat_B8G8R8U:
	{
		C_PixelB8G8R8U* pPixel = 
			(C_PixelB8G8R8U*)GetPixelDataPtr(x, y, z);
		pPixel->Set(pixel);
		break;
	}
	case E_PixelFormat_B8G8R8A8U:
	{
		C_PixelB8G8R8A8U* pPixel = 
			(C_PixelB8G8R8A8U*)GetPixelDataPtr(x, y, z);
		pPixel->Set(pixel);
		break;
	}

	case E_PixelFormat_R32G32B32A32F:
	{
		C_PixelR32G32B32A32F* pPixel = 
			(C_PixelR32G32B32A32F*)GetPixelDataPtr(x, y, z);
		pPixel->Set(pixel);
		break;
	}
	case E_PixelFormat_A32R32G32B32F:
	{
		C_PixelA32R32G32B32F* pPixel = 
			(C_PixelA32R32G32B32F*)GetPixelDataPtr(x, y, z);
		pPixel->Set(pixel);
		break;
	}
	case E_PixelFormat_R32G32B32F:
	{
		C_PixelR32G32B32F* pPixel = 
			(C_PixelR32G32B32F*)GetPixelDataPtr(x, y, z);
		pPixel->Set(pixel);
		break;
	}
	case E_PixelFormat_B32G32R32F:
	{
		C_PixelB32G32R32F* pPixel = 
			(C_PixelB32G32R32F*)GetPixelDataPtr(x, y, z);
		pPixel->Set(pixel);
		break;
	}
	case E_PixelFormat_B32G32R32A32F:
	{
		C_PixelB32G32R32A32F* pPixel = 
			(C_PixelB32G32R32A32F*)GetPixelDataPtr(x, y, z);
		pPixel->Set(pixel);
		break;
	}

	case E_PixelFormat_Luminance8U:
	{
		C_PixelLuminance8U* pPixel = 
			(C_PixelLuminance8U*)GetPixelDataPtr(x, y, z);
		pPixel->Set(pixel);
		break;
	}
	case E_PixelFormat_Luminance16U:
	{
		C_PixelLuminance16U* pPixel = 
			(C_PixelLuminance16U*)GetPixelDataPtr(x, y, z);
		pPixel->Set(pixel);
		break;
	}
	default:
		break;
	};
}

C_Status C_Image::SetPixelData(
	const U8* pData, 
	E_PixelFormat format)
{
	if (format == m_pixelFormat)
	{
		memcpy(
			m_pPixelData, 
			pData, 
			GetPixelDataSize());
		return E_Status_Success;
	}

	U32 nPixels = GetWidth() * GetHeight();
	C_PixelR32G32B32A32F* pInterchangePixels = 
		new C_PixelR32G32B32A32F[nPixels];

	C_Status status;
	//Copy data to intermediate format.
	status = C_PixelConversion::CopyToInterchangeFormat(
		pData,	// source pixels
		0,		// source palette
		format,	// source format
		pInterchangePixels,		// pixel data store
		nPixels); // num pixels.
	if (!status) return status;

	//Free old pixel data.
	delete[] m_pPixelData;
	m_pPixelData = 0;

	//Initialise our new pixel array.
	status = NewPixelData(
		GetWidth(),
		GetHeight(),
		GetDepth(),
		format,
		0);
	if (!status) return status;

	//Copy from intermediate format to current format.
	status = C_PixelConversion::CopyFromInterchangeFormat(
		pInterchangePixels,
		GetPixelDataPtr(),
		m_pixelFormat,
		nPixels);
	if (!status) return status;
	delete[] pInterchangePixels;

	return E_Status_Success;
}

C_Status C_Image::SetPaletteData(const U8* pPalette, E_PaletteFormat format)
{
	m_paletteFormat = format;
	if (!m_pPaletteData)
		m_pPaletteData = new U8[GetPaletteDataSize(m_pixelFormat, format)];

	if (format == m_paletteFormat)
	{
		//Simply overwrite existing palette
		memcpy(
			m_pPaletteData, 
			pPalette,
			GetPaletteDataSize());
		return E_Status_Success;
	}

	delete[] m_pPaletteData;

	U32 nPaletteEntries = GetNumPaletteEntries(m_pixelFormat);

	//Allocate enough temporary storage for our palette.
	C_PixelR32G32B32A32F* pInterchangePixels = 
		new C_PixelR32G32B32A32F[nPaletteEntries];

	C_Status status;
	//Copy data to intermediate format.
	status = C_PaletteConversion::CopyToInterchangeFormat(
		m_pPaletteData,
		format,
		pInterchangePixels, 
		nPaletteEntries);
	if (!status) return status;

	//Copy from intermediate format to current format.
	status = C_PaletteConversion::CopyFromInterchangeFormat(
		pInterchangePixels,
		m_pPaletteData,
		m_paletteFormat,
		nPaletteEntries);
	if (!status) return status;
	delete[] pInterchangePixels;

	return E_Status_Success;
}

C_Status C_Image::NewPixelData(
	U32 width, 
	U32 height, 
	U32 depth,
	E_PixelFormat format,
	const U8* pData)
{
	return _NewPixelData(
		width,
		height,
		depth,
		format, 
		pData);
}

C_Status C_Image::MovePixelData(
	U32 width, 
	U32 height, 
	U32 depth,
	E_PixelFormat format,
	U8* pData)
{
	return _MovePixelData(
		width,
		height, 
		depth,
		format, 
		pData);
}

void C_Image::DisownPixelData()
{
	m_pPixelData = 0;
	Clear();
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

C_Status C_Image::_MovePixelData(
	U32 width, 
	U32 height, 
	U32 depth,
	E_PixelFormat format,
	U8* pData)
{
	//If we've a valid data ptr, and we're already handling that data, then we have nothing
	//further to do here.
	if (pData && pData == m_pPixelData)
		return E_Status_Success;

	C_Status status;

	DeletePixelData();

	//Cannot create an image if we don't know its format.
	if (format == E_PixelFormat_Unknown)
		return E_Status_Failure;

	if (m_pixelFormat == E_PixelFormat_Unknown)
	{
		//Set image format. If the incomming format differs then we'll need to perform data conversion.
		m_pixelFormat = format;
	}

	m_width = width;
	m_height = height;
	m_depth = depth;

	m_pPixelData = pData;

	return E_Status_Success;
}
#endif

C_Status C_Image::_NewPixelData(
	U32 width, 
	U32 height, 
	U32 depth,
	E_PixelFormat format,
	const U8* pData)
{
	DeletePixelData();

	//We call this purely for the sanity checks.
	//We don't actually move anything.
	C_Status status = _MovePixelData(
		width,
		height,
		depth,
		format,
		0);
	if (!status) return status;

	//Allocate memory.
	U32 bytes = GetPixelDataSize();
	m_pPixelData = new U8[bytes];
	if (!m_pPixelData)
		return E_Status_OutOfMemory;

	//Copy data, performing format conversion as needed.
	if (pData)
	{
		status = SetPixelData(
			pData, 
			format);
		if (!status) return status;
	}
	return E_Status_Success;
}

C_PixelR32G32B32A32F C_Image::GetPixel(
	U32 x, 
	U32 y,
	U32 z) const
{
	C_PixelR32G32B32A32F outPixel;

	switch(m_pixelFormat)
	{
	case E_PixelFormat_R8G8B8A8U:
	{
		C_PixelR8G8B8A8U* pPixel = 
			(C_PixelR8G8B8A8U*)GetPixelDataPtr(x, y, z);
		outPixel.Set(*pPixel);
		break;
	}
	case E_PixelFormat_A8R8G8B8U:
	{
		C_PixelA8R8G8B8U* pPixel = 
			(C_PixelA8R8G8B8U*)GetPixelDataPtr(x, y, z);
		outPixel.Set(*pPixel);
		break;
	}
	case E_PixelFormat_A8B8G8R8U:
	{
		C_PixelA8B8G8R8U* pPixel = 
			(C_PixelA8B8G8R8U*)GetPixelDataPtr(x, y, z);
		outPixel.Set(*pPixel);
		break;
	}
	case E_PixelFormat_R8G8B8U:
	{
		C_PixelR8G8B8U* pPixel = 
			(C_PixelR8G8B8U*)GetPixelDataPtr(x, y, z);
		outPixel.Set(*pPixel);
		break;
	}
	case E_PixelFormat_B8G8R8U:
	{
		C_PixelB8G8R8U* pPixel = 
			(C_PixelB8G8R8U*)GetPixelDataPtr(x, y, z);
		outPixel.Set(*pPixel);
		break;
	}
	case E_PixelFormat_B8G8R8A8U:
	{
		C_PixelB8G8R8A8U* pPixel = 
			(C_PixelB8G8R8A8U*)GetPixelDataPtr(x, y, z);
		outPixel.Set(*pPixel);
		break;
	}

	case E_PixelFormat_R32G32B32A32F:
	{
		C_PixelR32G32B32A32F* pPixel = 
			(C_PixelR32G32B32A32F*)GetPixelDataPtr(x, y, z);
		outPixel.Set(*pPixel);
		break;
	}
	case E_PixelFormat_A32R32G32B32F:
	{
		C_PixelA32R32G32B32F* pPixel = 
			(C_PixelA32R32G32B32F*)GetPixelDataPtr(x, y, z);
		outPixel.Set(*pPixel);
		break;
	}
	case E_PixelFormat_R32G32B32F:
	{
		C_PixelR32G32B32F* pPixel = 
			(C_PixelR32G32B32F*)GetPixelDataPtr(x, y, z);
		outPixel.Set(*pPixel);
		break;
	}
	case E_PixelFormat_B32G32R32F:
	{
		C_PixelB32G32R32F* pPixel = 
			(C_PixelB32G32R32F*)GetPixelDataPtr(x, y, z);
		outPixel.Set(*pPixel);
		break;
	}
	case E_PixelFormat_B32G32R32A32F:
	{
		C_PixelB32G32R32A32F* pPixel = 
			(C_PixelB32G32R32A32F*)GetPixelDataPtr(x, y, z);
		outPixel.Set(*pPixel);
		break;
	}

	case E_PixelFormat_Luminance8U:
	{
		C_PixelLuminance8U* pPixel = 
			(C_PixelLuminance8U*)GetPixelDataPtr(x, y, z);
		outPixel.Set(*pPixel);
		break;
	}
	case E_PixelFormat_Luminance16U:
	{
		C_PixelLuminance16U* pPixel = 
			(C_PixelLuminance16U*)GetPixelDataPtr(x, y, z);
		outPixel.Set(*pPixel);
		break;
	}
	case E_PixelFormat_ColourIndex8U:
	{
		outPixel = GetPalettisedPixel(x, y, z);
	}
	default:
		break;
	};

	return outPixel;
}

C_PixelR32G32B32A32F C_Image::GetPalettisedPixel(
	U32 x, 
	U32 y,
	U32 z) const
{
	C_PixelR32G32B32A32F outPixel(0.f, 0.f, 0.f, 0.f);
	U8 paletteIndex = *GetPixelDataPtr(x, y, z);
	switch(m_paletteFormat)
	{
	case E_PaletteFormat_R8G8B8A8U:
	{
		assert((S32)paletteIndex < GetNumPaletteEntries());
		C_PixelR8G8B8A8U *pColour = (C_PixelR8G8B8A8U*) GetPaletteDataPtr();
		pColour += paletteIndex;
		outPixel.Set(*pColour);
	}
	case E_PaletteFormat_B8G8R8A8U:
	{
		U8 paletteIndex = *GetPixelDataPtr(x, y, z);
		assert((S32)paletteIndex < GetNumPaletteEntries());
		C_PixelB8G8R8A8U *pColour = (C_PixelB8G8R8A8U*) GetPaletteDataPtr();
		pColour += paletteIndex;
		outPixel.Set(*pColour);
	}
	case E_PaletteFormat_A8R8G8B8U:
	{
		assert((S32)paletteIndex < GetNumPaletteEntries());
		C_PixelA8R8G8B8U *pColour = (C_PixelA8R8G8B8U*) GetPaletteDataPtr();
		pColour += paletteIndex;
		outPixel.Set(*pColour);
	}
	case E_PaletteFormat_A8B8G8R8U:
	{
		assert((S32)paletteIndex < GetNumPaletteEntries());
		C_PixelA8B8G8R8U *pColour = (C_PixelA8B8G8R8U*) GetPaletteDataPtr();
		pColour += paletteIndex;
		outPixel.Set(*pColour);
	}
	case E_PaletteFormat_R8G8B8U:
	{
		assert((S32)paletteIndex < GetNumPaletteEntries());
		C_PixelR8G8B8U *pColour = (C_PixelR8G8B8U*) GetPaletteDataPtr();
		pColour += paletteIndex;
		outPixel.Set(*pColour);
	}
	case E_PaletteFormat_B8G8R8U:
	{
		assert((S32)paletteIndex < GetNumPaletteEntries());
		C_PixelB8G8R8U *pColour = (C_PixelB8G8R8U*) GetPaletteDataPtr();
		pColour += paletteIndex;
		outPixel.Set(*pColour);
	}
	}

	return outPixel;
}

C_Status C_Image::Fill(U8* pPixel)
{
	U32 bytesPerPixel = GetBytesPerPixel();

	U32 nPixels = GetWidth() * GetHeight();
	for (U32 l = 0; l < nPixels; l++, m_pPixelData+=bytesPerPixel)
		memcpy(m_pPixelData, pPixel, bytesPerPixel);

	return E_Status_Success;
}

C_Status C_Image::Fill(E_Channel channel, F32 value)
{
	return Fill(channel, C_PixelR32G32B32A32F(value, value, value, value));
}

C_Status C_Image::Fill(const C_PixelR32G32B32A32F& pixel)
{
	return Fill(E_Channel_0 | E_Channel_1 | E_Channel_2 | E_Channel_3, pixel);
}

C_Status C_Image::Fill(U32 channels, const C_PixelR32G32B32A32F& pixel)
{
	Attk::E_PixelFormat oldFormat = m_pixelFormat;
	ChangePixelFormat(E_PixelFormat_R32G32B32A32F);

	U32 nPixels = GetWidth() * GetHeight();
	C_PixelR32G32B32A32F* pPixel = (C_PixelR32G32B32A32F*)m_pPixelData;

	for (U32 l = 0; l < nPixels; l++, pPixel++)
	{
		if (channels & Attk::E_Channel_0)
			pPixel->red = pixel.red;
		if (channels & Attk::E_Channel_1)
			pPixel->green = pixel.green;
		if (channels & Attk::E_Channel_2)
			pPixel->blue = pixel.blue;
		if (channels & Attk::E_Channel_3)
			pPixel->alpha = pixel.alpha;
	}

	ChangePixelFormat(oldFormat);
	return E_Status_Success;
}

void C_Image::DeletePixelData()
{
	delete[] m_pPixelData;
	m_pPixelData = 0;
}

void C_Image::DeletePaletteData()
{
	delete[] m_pPaletteData;
	m_pPaletteData = 0;
	
	m_paletteFormat = E_PaletteFormat_None;
}

C_Status C_Image::ChangePixelFormat(E_PixelFormat pixelFormat)
{
	if (pixelFormat == m_pixelFormat)
		return E_Status_Success;

	C_Status status;
	{
		C_Image tmp;
		status = tmp.NewPixelData(
			m_width,
			m_height,
			m_depth,
			pixelFormat);
		if (!status) return status;

		for (U32 width = 0; width < m_width; width++)
			for (U32 height = 0; height < m_height; height++)
				for (U32 depth = 0; depth < m_depth; depth++)
					tmp.SetPixel(GetPixel(width, height, depth), width, height, depth);

		//Free original pixel data.
		delete[] m_pPixelData;

		//Guerilla access of tmp internal data. 
		m_pPixelData = tmp.m_pPixelData;
		tmp.m_pPixelData = 0;
	}

	//Free original palette data.
	if (m_pPaletteData)
	{
		delete[] m_pPaletteData;
		m_pPaletteData = 0;
		m_paletteFormat = E_PaletteFormat_None;
	}
	m_pixelFormat = pixelFormat;
	return E_Status_Success;
}

C_Status C_Image::ChangePaletteFormat(E_PaletteFormat paletteFormat)
{
	if (
		(m_paletteFormat == E_PaletteFormat_Unknown) 
		|| 
		(m_paletteFormat == E_PaletteFormat_None)
		)
		return E_Status_Failure;

	if (paletteFormat == m_paletteFormat)
		return E_Status_Success;

	U32 nPaletteEntriesToCopy = GetNumPaletteEntries(m_pixelFormat);

	//Perform format conversion as source data is in a foreign format.
	C_PixelR32G32B32A32F* pInterchangePixels = 
		new C_PixelR32G32B32A32F[nPaletteEntriesToCopy];

	C_Status status;
	//Copy data to intermediate format.
	status = C_PaletteConversion::CopyToInterchangeFormat(
		m_pPaletteData,
		m_paletteFormat,
		pInterchangePixels, 
		nPaletteEntriesToCopy);
	if (!status) return status;

	//Free original pixel data.
	delete[] m_pPaletteData;

	//Allocate storage for copy of new pixel data.
	{
		U32 nBytesToCopy = nPaletteEntriesToCopy * GetBytesPerPaletteEntry(paletteFormat);
		m_pPaletteData = new U8[nBytesToCopy];
	}

	//Copy from interchange format into our new format.
	status = C_PaletteConversion::CopyFromInterchangeFormat(
		pInterchangePixels,
		m_pPaletteData,
		paletteFormat,
		nPaletteEntriesToCopy);
	if (!status) return status;
	delete[] pInterchangePixels;

	return E_Status_Success;
}

void C_Image::Clear()
{
	//Clear pixel data.
	delete[] m_pPixelData;
	m_pPixelData = 0;

	//Clear palette
	delete[] m_pPaletteData;
	m_pPaletteData = 0;
	m_paletteFormat = E_PaletteFormat_None;

	//Clear header data.
	C_BaseImage::Clear();
}

///Insert an image at the specified location
void C_Image::Insert(U32 insertX, U32 insertY, const Attk::C_Image& rhs)
{
	assert((insertX) < m_width);
	assert((insertY) < m_height);
	assert((insertX + rhs.GetWidth()) <= m_width);
	assert((insertY + rhs.GetHeight()) <= m_height);

	//Copy existing image into this image.
	for (U32 x = 0; x < rhs.GetWidth(); x++)
		for (U32 y = 0; y < rhs.GetHeight(); y++)
			SetPixel(rhs.GetPixel(x,y), x+insertX, y+insertY );
}


C_Status C_Image::Read(
	const char* pFileName,
	E_ImageFormat imageFormat,
	const C_Options* pOptions)
{
	if (!Sys::C_FileSystem::Instance().FileExists(pFileName))
		return E_Status_Failure;

	C_Status status;
	bool canRead;

	//Check if file is in the Attk's native format...
	{
	}
	//Else, try to use DevIL to process it.
	C_DevILPlugin plugin;
	canRead = plugin.CanRead(pFileName, &status);
	if (!canRead) 
		return Attk::E_Status_FailedToOpenFile;

	if (canRead)
	{
		status = plugin.Read(*this, pFileName, pOptions);
		if (!status) 
			return status;
	}

	return E_Status_Success;
}

C_Status C_Image::Write(
	const char* pFileName, 
	E_ImageFormat imageFormat,
	const C_Options* pOptions) const
{
	C_Status status;
	bool canWrite;

	//Check if file is in the Attk's native format...
	{
	}
	//Else, try to use DevIL to process it.
	C_DevILPlugin plugin;
	canWrite = plugin.CanWrite(pFileName, &status);
	if (!status) return status;

	if (canWrite)
	{
		status = plugin.Write(*this, pFileName, pOptions);
		if (!status) return status;
	}
	return E_Status_Success;
}
/*
Status C_Image::Read(
	const void* pSource, 
	U32 size, 
	EImageFormat imageFormat,
	const Options* pOptions)
{
	return Status::NOT_IMPLEMENTED;
}

Status C_Image::Write(
	void*& pDestination, 
	U32& size, 
	EImageFormat imageFormat,
	const Options* pOptions) const
{
	return Status::NOT_IMPLEMENTED;
}
*/

C_Status C_Image::_ResizeDevIL(
	U32 width, 
	U32 height,
	U32 depth)
{
	C_Status status = C_DevILUtility::Resize(*this, width, height);
	if (!status) return status;
		
	//Resize dimensions now that our pixel data has been processed.
	m_width = width;
	m_height = height;

	return E_Status_Success;
};


U8* C_Image::GetPaletteDataPtr()
{
	return m_pPaletteData;
}

const U8* C_Image::GetPaletteDataPtr() const
{
	return m_pPaletteData;
}

U8* C_Image::GetPixelDataPtr()
{
	return m_pPixelData;
}

const U8* C_Image::GetPixelDataPtr() const
{
	return m_pPixelData;
}