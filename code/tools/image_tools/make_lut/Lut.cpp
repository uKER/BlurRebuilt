
#include "stdafx.h"

#include "system\time\Time.h"
#include "system\debuglog.h"
#include <system/debug/log/AutoConfigure.h>

#include "Lut.h"

namespace Lut
{

static Debug::Logger& g_Log = Debug::GetLogger("Lut");

// C_Value

C_Value::C_Value()
{
}

C_Value::C_Value(F32 r, F32 g, F32 b)
		: m_R(r)
		, m_G(g)
		, m_B(b)
{
}

void C_Value::SetX8R8G8B8(U32 u32value)
{
	U8 r = (U8)((u32value >> 16) & 0xff);
	U8 g = (U8)((u32value >> 8) & 0xff);
	U8 b = (U8)((u32value >> 0) & 0xff);

	m_R = (F32)r / 255.0f;
	m_G = (F32)g / 255.0f;
	m_B = (F32)b / 255.0f;
}

U32 C_Value::GetX8R8G8B8() const
{
	const U8 r = (U8)(m_R * 255.0f);
	const U8 g = (U8)(m_G * 255.0f);
	const U8 b = (U8)(m_B * 255.0f);
	const U8 a = 255;

	const U32 result = (a << 24) + (r << 16) + (g << 8) + (b << 0);
	return result;
}

C_Value operator+ (const C_Value& a, const C_Value& b)
{
	C_Value result;
	result.m_R = a.m_R + b.m_R;
	result.m_G = a.m_G + b.m_G;
	result.m_B = a.m_B + b.m_B;
	return result;
}

C_Value operator* (const C_Value& a, const F32 x)
{
	C_Value result;
	result.m_R = a.m_R * x;
	result.m_G = a.m_G * x;
	result.m_B = a.m_B * x;
	return result;
}

// C_Lut

C_Lut::C_Lut()
	: m_Size( 32 )
{
	m_Data.Resize( m_Size * m_Size * m_Size );

	SetIdentity();
}

void C_Lut::SetIdentity()
{
	for(S32 z=0; z<m_Size; z++)
	{
		for(S32 y=0; y<m_Size; y++)
		{
			for(S32 x=0; x<m_Size; x++)
			{
				F32 r = (F32)x / ((F32)m_Size-1.0f);
				F32 g = (F32)y / ((F32)m_Size-1.0f);
				F32 b = (F32)z / ((F32)m_Size-1.0f);

				GetValue(x, y, z) = C_Value(r, g, b);
			}
		}
	}
}

const C_Value& C_Lut::GetValue(S32 x, S32 y, S32 z) const
{
	x = Maths::Clamp(x, 0, m_Size-1);
	y = Maths::Clamp(y, 0, m_Size-1);
	z = Maths::Clamp(z, 0, m_Size-1);

	return m_Data[ x + y * m_Size + z * m_Size * m_Size ];
}

C_Value& C_Lut::GetValue(S32 x, S32 y, S32 z)
{
	x = Maths::Clamp(x, 0, m_Size-1);
	y = Maths::Clamp(y, 0, m_Size-1);
	z = Maths::Clamp(z, 0, m_Size-1);

	return m_Data[ x + y * m_Size + z * m_Size * m_Size ];
}

bool C_Lut::ExtractStrip(const Attk::C_Image& image)
{
	LOGINFO( g_Log, "Extracting strip from image");

	const S32 width = image.GetWidth();
	const S32 height = image.GetHeight();

	const S32 stripWidth = m_Size * m_Size;
	const S32 stripHeight = m_Size;

	if( (width<stripWidth) || (height<stripHeight))
	{
		LOGERROR( g_Log, "Image to small to read LUT strip" );
		SetIdentity();
		return false;
	}

	const S32 stripX = 0;
	const S32 stripY = height - m_Size;
	for(S32 z=0; z<m_Size; z++)
	{
		for(S32 y=0; y<m_Size; y++)
		{
			for(S32 x=0; x<m_Size; x++)
			{
				S32 ix = stripX + x + z * m_Size;
				S32 iy = stripY + y;
				Attk::C_PixelR32G32B32A32F pixel = image.GetPixel(ix, iy);
				GetValue(x,y,z)=C_Value(pixel.red, pixel.green, pixel.blue);
			}
		}
	}
	return true;
}

bool C_Lut::EmbedStrip(Attk::C_Image& image)
{
	LOGINFO( g_Log, "Embedding strip into image");

	const S32 width = image.GetWidth();
	const S32 height = image.GetHeight();

	const S32 stripWidth = m_Size * m_Size;
	const S32 stripHeight = m_Size;

	if( (width<stripWidth) || (height<stripHeight))
	{
		LOGERROR( g_Log, "Image to small to embed LUT strip" );
		return false;
	}

	const S32 stripX = 0;
	const S32 stripY = height - m_Size;
	for(S32 z=0; z<m_Size; z++)
	{
		for(S32 y=0; y<m_Size; y++)
		{
			for(S32 x=0; x<m_Size; x++)
			{
				S32 ix = stripX + x + z * m_Size;
				S32 iy = stripY + y;

				const C_Value& value = GetValue(x,y,z);

				Attk::C_PixelR32G32B32A32F pixel = image.GetPixel(ix, iy);
				pixel.red = value.m_R;
				pixel.green = value.m_G;
				pixel.blue = value.m_B;
				image.SetPixel(pixel, ix, iy);
			}
		}
	}
	return true;
}

bool C_Lut::MakeStrip(Attk::C_Image& image)
{
	LOGINFO( g_Log, "Creating strip image");

	image.NewPixelData(m_Size * m_Size, m_Size, 1, Attk::E_PixelFormat_A8R8G8B8U);
	return EmbedStrip( image );

}

bool C_Lut::CreateVolumeTexture(IDirect3DDevice9* pD3dDevice, IDirect3DVolumeTexture9** ppVolumeTexture )
{
	HRESULT hr;

	const S32 width = m_Size;
	const S32 height = m_Size;
	const S32 depth = m_Size;
	const S32 mipLevels = 1;

	hr=pD3dDevice->CreateVolumeTexture( width, height, depth, mipLevels, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, ppVolumeTexture, NULL);

	// D3DPOOL_SYSTEMMEM

	if( FAILED(hr) )
	{
		LOGERROR( g_Log, "Failed to create volume texture" );
		return false;
	}

	U32 mipLevel = 0;

	IDirect3DVolume9* pVolumeLevel = NULL;
	hr = (*ppVolumeTexture)->GetVolumeLevel(mipLevel, &pVolumeLevel);
	if( FAILED(hr) )
	{
		printf( "Failed to get volume level\n" );
		return false;
	}

#if 1
	// lock and fill texture
	D3DLOCKED_BOX lockedVolume;
	hr = pVolumeLevel->LockBox(&lockedVolume, NULL, D3DLOCK_DISCARD);

	if( FAILED(hr) )
	{
		LOGERROR( g_Log, "Failed to lock box" );
		return false;
	}

	U8* pVolume = (U8*)lockedVolume.pBits;
	for(S32 z=0; z<depth; z++)
	{
		U8* pSlice = pVolume + lockedVolume.SlicePitch * z;
		for(S32 y=0; y<height; y++)
		{
			U8* pRow = pSlice + lockedVolume.RowPitch * y;
			for(S32 x=0; x<width; x++)
			{
				U32* pPixel = (U32*)(pRow + (x * 4));
				*pPixel = GetValue(x,y,z).GetX8R8G8B8();
			}
		}
	}

	hr = pVolumeLevel->UnlockBox();

	if( FAILED(hr) )
	{
		LOGERROR( g_Log, "Failed to unlock box" );
		return false;
	}
#else
	for(S32 z=0; z<depth; z++)
	{
		D3DBOX box;

		box.Left = 0;
		box.Right = width;
		box.Top = 0;
		box.Bottom = height;
		box.Front = z;
		box.Back = z+1;

		// lock and fill texture
		D3DLOCKED_BOX lockedVolume;
		hr = pVolumeLevel->LockBox(&lockedVolume, &box, 0);

		if( FAILED(hr) )
		{
			LOGERROR( g_Log, "Failed to lock box" );
			return false;
		}

		U8* pVolume = (U8*)lockedVolume.pBits;
		U8* pSlice = pVolume + lockedVolume.SlicePitch * z;
		for(S32 y=0; y<height; y++)
		{
			U8* pRow = pSlice + lockedVolume.RowPitch * y;
			for(S32 x=0; x<width; x++)
			{
				U32* pPixel = (U32*)(pRow + (x * 4));
				*pPixel = GetValue(x,y,z).GetX8R8G8B8();
			}
		}

		hr = pVolumeLevel->UnlockBox();
		if( FAILED(hr) )
		{
			LOGERROR( g_Log, "Failed to unlock box" );
			return false;
		}
	}
#endif

	if(pVolumeLevel)
	{
		pVolumeLevel->Release();
		pVolumeLevel = NULL;
	}

	return true;
}

bool C_Lut::SetupFromVolumeTexture(IDirect3DDevice9* pD3dDevice, IDirect3DVolumeTexture9* pVolumeTexture )
{
	HRESULT hr;

	const S32 width = m_Size;
	const S32 height = m_Size;
	const S32 depth = m_Size;
	const S32 mipLevels = 1;

	D3DVOLUME_DESC volumeDesc;
	pVolumeTexture->GetLevelDesc(0, &volumeDesc);

	if(volumeDesc.Width != width)
	{
		LOGERROR( g_Log, "Width is %d, must be %d", volumeDesc.Width, width );
		return false;
	}

	if(volumeDesc.Height != height)
	{
		LOGERROR( g_Log, "Height is %d, must be %d", volumeDesc.Height, height );
		return false;
	}

	if(volumeDesc.Depth != depth)
	{
		LOGERROR( g_Log, "Depth is %d, must be %d", volumeDesc.Depth, depth );
		return false;
	}

	if(volumeDesc.Format != D3DFMT_X8R8G8B8)
	{
		LOGERROR( g_Log, "Format is not D3DFMT_A8R8G8B8, format is %d", volumeDesc.Format );
		return false;
	}
	

	U32 mipLevel = 0;
	IDirect3DVolume9* pVolumeLevel = NULL;
	hr = pVolumeTexture->GetVolumeLevel(mipLevel, &pVolumeLevel);
	if( FAILED(hr) )
	{
		printf( "Failed to get volume level\n" );
		return false;
	}

#if 1
	// lock and read texture
	D3DLOCKED_BOX lockedVolume;
	hr = pVolumeLevel->LockBox(&lockedVolume, NULL, D3DLOCK_READONLY);

	if( FAILED(hr) )
	{
		LOGERROR( g_Log, "Failed to lock box" );
		return false;
	}

	U8* pVolume = (U8*)lockedVolume.pBits;
	for(S32 z=0; z<depth; z++)
	{
		U8* pSlice = pVolume + lockedVolume.SlicePitch * z;
		for(S32 y=0; y<height; y++)
		{
			U8* pRow = pSlice + lockedVolume.RowPitch * y;
			for(S32 x=0; x<width; x++)
			{
				U32* pPixel = (U32*)(pRow + (x * 4));
				GetValue(x,y,z).SetX8R8G8B8(*pPixel);
			}
		}
	}

	hr = pVolumeLevel->UnlockBox();
	if( FAILED(hr) )
	{
		LOGERROR( g_Log, "Failed to unlock box" );
		return false;
	}

#else
	for(S32 z=0; z<depth; z++)
	{
		D3DBOX box;

		box.Left = 0;
		box.Right = width;
		box.Top = 0;
		box.Bottom = height;
		box.Front = z;
		box.Back = z+1;

		D3DLOCKED_BOX lockedVolume;
		hr = pVolumeLevel->LockBox(&lockedVolume, &box, D3DLOCK_READONLY);
		if( FAILED(hr) )
		{
			LOGERROR( g_Log, "Failed to lock box" );
			return false;
		}
	
		U8* pVolume = (U8*)lockedVolume.pBits;
		U8* pSlice = pVolume + lockedVolume.SlicePitch * z;
		for(S32 y=0; y<height; y++)
		{
			U8* pRow = pSlice + lockedVolume.RowPitch * y;
			for(S32 x=0; x<width; x++)
			{
				U32* pPixel = (U32*)(pRow + (x * 4));
				GetValue(x,y,z).SetX8R8G8B8(*pPixel);
			}
		}
		pVolumeLevel->UnlockBox();
	}
#endif

	if(pVolumeLevel)
	{
		pVolumeLevel->Release();
		pVolumeLevel = NULL;
	}

	return true;
}

bool C_Lut::WriteDDS(IDirect3DDevice9* pD3dDevice, const Util::C_String& fileName)
{
	HRESULT hr;	
	bool result;

	LPDIRECT3DVOLUMETEXTURE9 p3dTexture;

	result = CreateVolumeTexture( pD3dDevice, &p3dTexture );
	if(!result)
	{
		return false;
	}

	LOGINFO( g_Log, "Writing Output File %s", fileName.Chars());

	hr = D3DXSaveTextureToFile(fileName.Chars(), D3DXIFF_DDS, p3dTexture, NULL);
	if( FAILED(hr) )
	{
		LOGERROR( g_Log, "Failed to save output file %s", fileName.Chars() );
		return false;
	}

	SAFE_RELEASE(p3dTexture);
	return true;
}

bool C_Lut::ReadDDS(IDirect3DDevice9* pD3dDevice, const Util::C_String& fileName)
{
	HRESULT hr;
	
	LOGINFO( g_Log, "Reading input File %s", fileName.Chars());

	LPDIRECT3DVOLUMETEXTURE9 p3dTexture;
	hr = D3DXCreateVolumeTextureFromFile( pD3dDevice, fileName.Chars(), &p3dTexture);

	if(FAILED(hr))
	{
		LOGERROR( g_Log, "Failed to read input file %s", fileName.Chars() );
		return false;
	}


	return SetupFromVolumeTexture( pD3dDevice, p3dTexture );
}

C_Value C_Lut::Apply(const C_Value& src) const
{
	F32 x = src.m_R * (F32)(m_Size-1);
	F32 y = src.m_G * (F32)(m_Size-1);
	F32 z = src.m_B * (F32)(m_Size-1);

	S32 ix = (S32)floor(x);
	S32 iy = (S32)floor(y);
	S32 iz = (S32)floor(z);

	F32 fx = x - (F32)ix;
	F32 fy = y - (F32)iy;
	F32 fz = z - (F32)iz;

	F32 omfx = 1.0f - fx;
	F32 omfy = 1.0f - fy;
	F32 omfz = 1.0f - fz;

	C_Value result(0,0,0);

	result = result + GetValue(ix,   iy,   iz  ) * (omfx * omfy * omfz);
	result = result + GetValue(ix+1, iy,   iz  ) * (fx   * omfy * omfz);
	result = result + GetValue(ix,   iy+1, iz  ) * (omfx * fy   * omfz);
	result = result + GetValue(ix+1, iy+1, iz  ) * (fx   * fy   * omfz);

	result = result + GetValue(ix,   iy,   iz+1) * (omfx * omfy * fz);
	result = result + GetValue(ix+1, iy,   iz+1) * (fx   * omfy * fz);
	result = result + GetValue(ix,   iy+1, iz+1) * (omfx * fy   * fz);
	result = result + GetValue(ix+1, iy+1, iz+1) * (fx   * fy   * fz);

	return result;
}

void C_Lut::Apply(Attk::C_Image& image)
{
	LOGINFO( g_Log, "Applying LUT");

	const S32 width = image.GetWidth();
	const S32 height = image.GetHeight();

	for(S32 y=0; y<height; y++)
	{
		for(S32 x=0; x<width; x++)
		{	
			Attk::C_PixelR32G32B32A32F pixel = image.GetPixel(x,y);

			C_Value value( pixel.red, pixel.green, pixel.blue );
			value = Apply(value);

			pixel.red = value.m_R;
			pixel.green = value.m_G;
			pixel.blue = value.m_B;

			image.SetPixel(pixel, x, y);
		}
	}
}


} // namespace Lut

