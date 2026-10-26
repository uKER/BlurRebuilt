#include "stdafx.h"

#define NGL_GPU_D3D_PC 1

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>

#include "d3d9.h"  // Make sure this is the PC version.
#include "d3dx9.h"
#include "d3dx9tex.h"

#include <core/core.h>
#include <core/containers/string.h>
#include <system/scheduler/core/tl_defs.h>
#include <system/serialise/I_BinaryStream.h>
#include <system/serialise/serialise.h>
#include <system/serialise/BinarySerialise.h>
#include <core/utility/endian.h>
#include "ngl/defs.h"

#define NGL_GPU_D3D9 1
#define NGL_API
#include "ngl/dx/ngl_dx_gpu_types.h"	// output texture structures
#include "texture_export/ngl/gpu/ngl_gpu_texture.h"

#include "texture_export/texture_export.h"
#include "texture_export/Texture.h"

#include "texture_export/targets/pctex.h"

using namespace ActTools;

// Special bpp values for texel formats formats.
#define PC_BPP_DXT1 -1
#define PC_BPP_DXT2 -2

struct PCTexelFormatDesc { const char* name; D3DFORMAT format; int bpp; };

PCTexelFormatDesc theNglPcTexelFormatsDesc[] =
{
	{ "DXT1", D3DFMT_DXT1, PC_BPP_DXT1 },
	{ "DXT2", D3DFMT_DXT2, PC_BPP_DXT2 },
	{ "DXT3", D3DFMT_DXT3, PC_BPP_DXT2 },
	{ "DXT4", D3DFMT_DXT4, PC_BPP_DXT2 },
	{ "DXT5", D3DFMT_DXT5, PC_BPP_DXT2 },
	{ "R3G3B2", D3DFMT_R3G3B2, 8 },
	{ "A8", D3DFMT_A8, 8 },
	{ "L8", D3DFMT_L8, 8 },
	{ "P8", D3DFMT_P8, 8 },
	{ "A4L4", D3DFMT_A4L4, 8 },
	{ "R5G6B5", D3DFMT_R5G6B5, 16 },
	{ "X1R5G5B5", D3DFMT_X1R5G5B5, 16 },
	{ "A1R5G5B5", D3DFMT_A1R5G5B5, 16 },
	{ "A4R4G4B4", D3DFMT_A4R4G4B4, 16 },
	{ "X4R4G4B4", D3DFMT_X4R4G4B4, 16 },
	{ "A8R3G3B2", D3DFMT_A8R3G3B2, 16 },
	{ "A8P8", D3DFMT_A8P8, 16 },
	{ "A8L8", D3DFMT_A8L8, 16 },
	{ "V8U8", D3DFMT_V8U8, 16 },
	{ "L6V5U5", D3DFMT_L6V5U5, 16 },
	{ "R8G8_B8G8", D3DFMT_R8G8_B8G8, 16 },
	{ "G8R8_G8B8", D3DFMT_G8R8_G8B8, 16 },
	{ "D16_LOCKABLE", D3DFMT_D16_LOCKABLE, 16 },
	{ "D15S1", D3DFMT_D15S1, 16 },
	{ "D16", D3DFMT_D16, 16 },
	{ "L16", D3DFMT_L16, 16 },
	{ "R16F", D3DFMT_R16F, 16 },
	{ "CxV8U8", D3DFMT_CxV8U8, 16 },
	{ "R8G8B8", D3DFMT_R8G8B8, 24 },
	{ "A8R8G8B8", D3DFMT_A8R8G8B8, 32 },
	{ "X8R8G8B8", D3DFMT_X8R8G8B8, 32 },
	{ "A8B8G8R8", D3DFMT_A8B8G8R8, 32 },
	{ "X8B8G8R8", D3DFMT_X8B8G8R8, 32 },
	{ "A2R10G10B10", D3DFMT_A2R10G10B10, 32 },
	{ "A2B10G10R10", D3DFMT_A2B10G10R10, 32 },
	{ "G16R16", D3DFMT_G16R16, 32 },
	{ "X8L8V8U8", D3DFMT_X8L8V8U8, 32 },
	{ "Q8W8V8U8", D3DFMT_Q8W8V8U8, 32 },
	{ "V16U16", D3DFMT_V16U16, 32 },
	{ "A2W10V10U10", D3DFMT_A2W10V10U10, 32 },
	{ "D32", D3DFMT_D32, 32 },
	{ "D24S8", D3DFMT_D24S8, 32 },
	{ "D24X8", D3DFMT_D24X8, 32 },
	{ "D24X4S4", D3DFMT_D24X4S4, 32 },
	{ "D32F_LOCKABLE", D3DFMT_D32F_LOCKABLE, 32 },
	{ "D24FS8", D3DFMT_D24FS8, 32 },
	{ "G16R16F", D3DFMT_G16R16F, 32 },
	{ "R32F", D3DFMT_R32F, 32 },
	{ "A16B16G16R16", D3DFMT_A16B16G16R16, 64 },
	{ "Q16W16V16U16", D3DFMT_Q16W16V16U16, 64},
	{ "A16B16G16R16F", D3DFMT_A16B16G16R16F, 64 },
	{ "G32R32F", D3DFMT_G32R32F, 64 },
	{ "A32B32G32R32F", D3DFMT_A32B32G32R32F, 128 },
	{ "MULTI2_ARGB8", D3DFMT_MULTI2_ARGB8, 0 },
	{ "VERTEXDATA", D3DFMT_VERTEXDATA, 0 },
	{ "INDEX16", D3DFMT_INDEX16, 0 },
	{ "INDEX32", D3DFMT_INDEX32, 0 },
	{ "UYVY", D3DFMT_UYVY, 0 },
	{ "YUY2", D3DFMT_YUY2, 0 },
};

// Calculates the amount of memory used by a surface.
size_t NglPcTex::GetSurfaceBytes( int format, size_t w, size_t h )
{
	for ( size_t i = 0; i < tl_elementsof(theNglPcTexelFormatsDesc); ++i )
	{
		if ( theNglPcTexelFormatsDesc[i].format == format )
		{
			if ( theNglPcTexelFormatsDesc[i].bpp == PC_BPP_DXT1 )
				return tl_align( tl_max(1,w/4)*tl_max(1,h/4)*8, 4 );
			else if ( theNglPcTexelFormatsDesc[i].bpp == PC_BPP_DXT2 )
				return tl_align( tl_max(1,w/4)*tl_max(1,h/4)*16, 4 );
			else
				return tl_align(w*h*theNglPcTexelFormatsDesc[i].bpp/8, 4);
		}
	}
	assert( false );
	return 0;
}

int NglPcTex::GetD3DFormat( const char* id )
{
	for ( size_t i = 0; i < tl_elementsof(theNglPcTexelFormatsDesc); ++i )
	{
		if ( 0 == _stricmp( id, theNglPcTexelFormatsDesc[i].name ) )
			return theNglPcTexelFormatsDesc[i].format;
	}
	assert( false );
	return 0;
}

const char* NglPcTex::GetD3DFormatName( int format )
{
	for ( size_t i = 0; i < tl_elementsof(theNglPcTexelFormatsDesc); ++i )
	{
		if ( theNglPcTexelFormatsDesc[i].format == format )
			return theNglPcTexelFormatsDesc[i].name;
	}
	assert( false );
	return "invalid";
}

IDirect3D9* theD3DObject = NULL;
IDirect3DDevice9* theD3DDevice = NULL;

void InitD3DObjectDevice()
{
	// Initialize Direct3d
	if ( !theD3DObject )
		theD3DObject = ::Direct3DCreate9(D3D_SDK_VERSION);

	if ( !theD3DDevice )
	{
		D3DPRESENT_PARAMETERS params;
		ZeroMemory( &params, sizeof(params));
		params.BackBufferFormat = D3DFMT_UNKNOWN;
		params.BackBufferCount = 1;
		params.Windowed = TRUE;
		params.SwapEffect=D3DSWAPEFFECT_COPY;
		// For some reason CreateDevice does not like it when there is no focus window
		// focus window, so we have to create a nice little phony one.
		HWND hW = CreateWindow( "Button", "", 0, 0, 0, 1, 1, NULL, 0, 0, 0 );
		HRESULT err = theD3DObject->CreateDevice( D3DADAPTER_DEFAULT,
		                                          D3DDEVTYPE_NULLREF,
		                                          hW,
		                                          D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		                                          &params,
		                                          &theD3DDevice );
		DestroyWindow( hW );
	}
}

IDirect3DDevice9* GetD3DDevice() 
{
	InitD3DObjectDevice();
	return theD3DDevice;
}

class NglPcTexelFormat : public TexelFormat
{
public:
	D3DFORMAT format;
	const char* id;
	NglPcTex *driver;

	NglPcTexelFormat( NglPcTex *_driver, D3DFORMAT _format, const char* _id )
		: driver(_driver), format(_format), id(_id) {}

	size_t BlockSize() const {return 4;}

	void CopyFromA8R8G8B8( Surface* aDstSurface, const Surface* aSrcSurface ) const
	{
		IDirect3DTexture9* DstTex;
		GetD3DDevice()->CreateTexture( (UINT)aSrcSurface->GetWidth(), (UINT)aSrcSurface->GetHeight(), 1, 0, format, D3DPOOL_SCRATCH, &DstTex, NULL );
		IDirect3DSurface9* DstSurf;
		DstTex->GetSurfaceLevel( 0, &DstSurf );

		RECT rect;
		rect.left = rect.top = 0;
		rect.bottom = aSrcSurface->GetHeight();
		rect.right = aSrcSurface->GetWidth();

		D3DXLoadSurfaceFromMemory(
			DstSurf, NULL, NULL,
			aSrcSurface->GetTexels(), D3DFMT_A8R8G8B8, (UINT)aSrcSurface->GetWidth()*4, NULL, &rect, 
			D3DX_FILTER_NONE, 0 );

		D3DLOCKED_RECT lock;
		DstSurf->LockRect( &lock, NULL, 0 );
		memcpy( aDstSurface->GetTexels(), lock.pBits, Bytes(aDstSurface->GetWidth(), aDstSurface->GetHeight()) );
		DstSurf->UnlockRect();

		// hack up dest surface based on the actual # of bytes
		assert(aDstSurface->GetSize() == driver->GetSurfaceBytes(format, aDstSurface->GetWidth(), aDstSurface->GetHeight()));
		//aDstSurface->theSize = Bytes( aDstSurface->GetWidth(), aDstSurface->GetHeight() );

		DstSurf->Release();
		DstTex->Release();
	}
	void CopyToA8R8G8B8( Surface* aDstSurface, const Surface* aSrcSurface ) const
	{
		IDirect3DTexture9* DstTex;
		GetD3DDevice()->CreateTexture((UINT)aSrcSurface->GetWidth(), (UINT)aSrcSurface->GetHeight(), 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &DstTex, NULL );
		IDirect3DSurface9* DstSurf;
		DstTex->GetSurfaceLevel( 0, &DstSurf );

		RECT r = { 0, 0, aSrcSurface->GetWidth(), aSrcSurface->GetHeight() };
		UINT srcPitch = (UINT)Bytes(aSrcSurface->GetWidth(), 1);
		::D3DXLoadSurfaceFromMemory(
			DstSurf, NULL, NULL, 
			aSrcSurface->GetTexels(), format, srcPitch, NULL, &r, 
			D3DX_FILTER_NONE, 0 );

		D3DLOCKED_RECT lock;
		DstSurf->LockRect( &lock, NULL, 0 );
		size_t new_size = 4*aDstSurface->GetWidth()*aDstSurface->GetHeight();
		aDstSurface->Resize( new_size );
		memcpy( aDstSurface->GetTexels(), lock.pBits, new_size );
		DstSurf->UnlockRect();

		DstSurf->Release();
		DstTex->Release();
	}

	size_t Bytes(size_t w, size_t h) const { return driver->GetSurfaceBytes( format, w, h ); }
	const char* GetId() const { return id; }
	TexelFormat* Clone() const { return new NglPcTexelFormat(driver, format, id); }
	Class GetClass() const { return CLASS_INTEGER; }
};

TexelFormat* NglPcTex::GetTexelFormat( int format )
{
	for (size_t i = 0; i < tl_elementsof(theNglPcTexelFormatsDesc); i++)
		if ( theNglPcTexelFormatsDesc[i].format == format )
			return new NglPcTexelFormat( this, theNglPcTexelFormatsDesc[i].format, theNglPcTexelFormatsDesc[i].name );
	assert(false);
	return NULL;
}

TexelFormat* NglPcTex::GetTexelFormat( const char* format )
{
	for (size_t i = 0; i < tl_elementsof(theNglPcTexelFormatsDesc); i++)
		if ( 0 == _stricmp(theNglPcTexelFormatsDesc[i].name, format) )
			return new NglPcTexelFormat( this, theNglPcTexelFormatsDesc[i].format, theNglPcTexelFormatsDesc[i].name );
	assert(false);
	return NULL;
}

ACTRESULT NglPcTex::SerialOutTexture( Sys::I_BinaryStream* pStream, const Texture* aTexture, OutputType outputType)
{
	assert(pStream);
	Sys::BinarySerialiser<Ut::NativeToLittleEndian> ar(*pStream);

	// Check if the texture's dimensions are a power of 2.
	if ( !( IsPow2( (uint)aTexture->GetWidth() ) && IsPow2( (uint)aTexture->GetHeight() ) ) )
	{
		LOGERROR( TextureExport::g_Log, "PC Texture must have power of 2 dimensions!\n" );
	}

	int NMipmaps = aTexture->GetNumMipMaps();
	D3DFORMAT D3DFormat = (D3DFORMAT)GetD3DFormat( aTexture->GetFormat()->GetId() );

	// Textures with dimensions that are not powers of 2 cannot be mipmapped and cannot use DXT.
	if (!IsPow2(aTexture->GetWidth()) || !IsPow2(aTexture->GetHeight()))
	{
		assert(D3DFormat < D3DFMT_DXT1 || D3DFormat > D3DFMT_DXT5);
		NMipmaps = 1;
	}

	// write out header
	gpuTextureDesc desc;
	desc.Width = aTexture->GetWidth();
	desc.Height = aTexture->GetHeight();
	desc.Depth = aTexture->GetDepth();
	desc.Levels = aTexture->GetNumMipMaps();
	desc.Format = D3DFormat;

	// write out raw texture data
	if ( Texture::TEXTURE == aTexture->GetType() )
	{
		desc.Type = GPU_TEXTURE_2D;
	}
	else if ( Texture::CUBETEXTURE == aTexture->GetType() )
	{
		desc.Type = GPU_TEXTURE_CUBE;
	}
	else if ( Texture::VOLUMETEXTURE == aTexture->GetType() )
	{
		desc.Type = GPU_TEXTURE_VOLUME;
	}

	if (outputType != JUST_DATA)
		ar << desc;

	if (outputType != JUST_HEADER)
	{
		if ( Texture::TEXTURE == aTexture->GetType() )
		{
			size_t width = aTexture->GetWidth();
			size_t height = aTexture->GetHeight();

			for ( int i_mip = 0; i_mip < NMipmaps; ++i_mip )
			{
				size_t size = GetSurfaceBytes( D3DFormat, width, height );

				Surface* surface = const_cast<Texture*>(aTexture)->GetMipmap( 0, i_mip );
				//assert( width == surface->theWidth && height == surface->theHeight /*&& size == surface->theSize*/ );

				ar.Write(surface->GetTexels(), size);
				width = tl_max(1, width/2);
				height = tl_max(1, height/2);
			}
			// todo: copy multipals ngltexture
			// todo: copy multipals nglpalette
			// todo: copy multipals palette data
		}
		else if ( Texture::CUBETEXTURE == aTexture->GetType() )
		{
			for ( unsigned i_face = 0; i_face < 6; ++i_face )
			{
				size_t width = aTexture->GetWidth();
				size_t height = aTexture->GetHeight();

				for ( int i_mip = 0; i_mip < NMipmaps; ++i_mip )
				{
					size_t size = GetSurfaceBytes( D3DFormat, width, height );

					Surface* surface = const_cast<Texture*>(aTexture)->GetMipmap( i_face, i_mip );
					//assert( width == surface->GetWidth() && height == surface->GetHeight() && size == surface->GetSize() );

					ar.Write(surface->GetTexels(), size);

					width = tl_max(1, width/2);
					height = tl_max(1, height/2);
				}
			}
		}
		else if ( Texture::VOLUMETEXTURE == aTexture->GetType() )
		{
			size_t width = aTexture->GetWidth();
			size_t height = aTexture->GetHeight();
			size_t depth = aTexture->GetDepth();
			for ( int i_mip = 0; i_mip < NMipmaps; ++i_mip )
			{
				for ( unsigned i_face = 0; i_face < depth; ++i_face )
				{
					size_t size = GetSurfaceBytes( D3DFormat, width, height );

					Surface* surface = const_cast<Texture*>(aTexture)->GetMipmap( i_face, i_mip );
					assert( width == surface->GetWidth() && height == surface->GetHeight() && size == surface->GetSize() );

					ar.Write(surface->GetTexels(), size);
				}

				width = tl_max(1, width/2);
				height = tl_max(1, height/2);
				depth = tl_max(1, depth/2);
			}
		}
	}

	return ACT_OK;
}

ACTRESULT NglPcTex::SerialInTexture( Texture* aTexture, Sys::I_BinaryStream *stream ) 
{
	ACTRESULT err = ACT_OK;

	Sys::BinaryDeserialiser<Ut::LittleEndianToNative> ar(*stream);

	// read header
	gpuTextureDesc desc;
	ar >> desc;
	size_t num_mipmaps = desc.Levels;
	ActTools::TexelFormat* format = GetTexelFormat( (D3DFORMAT)desc.Format );

	assert( aTexture );
	if( desc.Depth == 1 )
		aTexture->CreateTexture( format, desc.Width, desc.Height, num_mipmaps );
	else if( desc.Depth == 6 && desc.Width == desc.Height )
		aTexture->CreateCubeTexture( format, desc.Width, num_mipmaps );
	else
		aTexture->CreateVolumeTexture( format, desc.Width, desc.Height, desc.Depth, num_mipmaps );

	// read binary data
	size_t width = aTexture->GetWidth();
	size_t height = aTexture->GetHeight();

	for( size_t i_mip = 0; i_mip < num_mipmaps; ++i_mip )
	{
		for( size_t i_depth = 0; i_depth < aTexture->GetDepth(); ++i_depth )
		{
			ActTools::Surface* surface = aTexture->GetMipmap( i_depth, i_mip );
			surface->Create( width, height );
			size_t size = surface->GetSize();

			ar.Read((void*)surface->GetTexels(), size);
		}
		width = tl_max(1, width/2);
		height = tl_max(1, height/2);
	}

	return err;
}

