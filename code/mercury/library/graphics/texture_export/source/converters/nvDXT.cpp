#include "stdafx.h"

#define EXCLUDE_LIBS

#include "dxtlib/dxtlib.h"

#include "texture_export/Texture.h"
#include "texture_export/converters/nvDXT.h"

using namespace ActTools;

namespace NvDXT
{

class nvDXTTexelFormat : public TexelFormat
{
public:
	// holds value from enum TextureFormats { kDXT1, kDXT1a, ... } declared in ddsTypes.h
	int theNvidiaTextureFormat; 
	int theQuality;
	const char* theId;

	nvDXTTexelFormat(int nvidiafmt, int quality, const char* id) 
		: theNvidiaTextureFormat(nvidiafmt), theQuality(quality), theId(id) {}

	void CopyFromA8R8G8B8( Surface* aDstSurface, const Surface* aSrcSurface ) const;
	void CopyToA8R8G8B8( Surface* aDstSurface, const Surface* aSrcSurface ) const;

	TexelFormat* Clone() const { return new nvDXTTexelFormat(theNvidiaTextureFormat, theQuality, theId); }

	size_t Bits() const 
	{
		switch (theNvidiaTextureFormat)
		{
			case kDXT1: return 4;
			case kDXT3:
			case kDXT5:
			case kDXT5_NM:  return 8;
			case kA8: return 8;
			case kA8L8: return 16;
			default: assert(false); return 0;
		}
	}
	size_t Bytes( size_t aWidth, size_t aHeight ) const
	{
		// ignoring DXT format for now, overallocates by several times.
		// would be nice if nvdxtlib had a function to return the size of a texture.
		return max(4,aWidth) * max(4,aHeight) * 4; 
	}

	size_t BlockSize() const {return 4;}

	const char* GetId() const { return theId; }

	Class GetClass() const { return CLASS_INTEGER; }
};

struct DxtlibUserData
{
	Surface* surface;
	const TexelFormat* format;
};

static NV_ERROR_CODE StoreMip( const void* aData, size_t aSize, const MIPMapData *mipMapData, void* aUserData )
{
	int aMipLevel = mipMapData->mipLevel;
	int aWidth = mipMapData->width;
	int aHeight = mipMapData->height;

	DxtlibUserData* user_data = reinterpret_cast<DxtlibUserData*>(aUserData);
	user_data->surface->Resize( aSize );
	user_data->surface->theSize = aSize;
	memcpy( user_data->surface->theTexels, aData, aSize );
	return NV_ERROR_CODE(0);
}

void nvDXTTexelFormat::CopyFromA8R8G8B8( Surface* aDstSurface, const Surface* aSrcSurface ) const
{
	assert( kDXT1 <= theNvidiaTextureFormat && theNvidiaTextureFormat <= k3Dc );

	nvCompressionOptions nvidia_options;
	nvidia_options.textureFormat = static_cast<nvTextureFormats>(theNvidiaTextureFormat);
	// check if kDXT1->kDXT1a since if we use kDXT1 it will turn the texture to opaque even if it is punch through(B&W alpha)
	if ( kDXT1 == nvidia_options.textureFormat )
	{
		const int alpha = aSrcSurface->theTexture->GetAlphaPresence();
		if ( Texture::ALPHA_GREYSCALE == alpha || Texture::ALPHA_BLACKWHITE == alpha )
			nvidia_options.textureFormat = kDXT1a;
	}
	nvidia_options.SetDefaultOptions();
	nvidia_options.quality = (nvQualitySetting)theQuality;
	nvidia_options.mipMapGeneration = kNoMipMaps;
	nvidia_options.bBinaryAlpha = false;
	nvidia_options.bSwapRGB = false;
	DxtlibUserData user_data = { aDstSurface, this };
	nvidia_options.user_data = &user_data;

	// A bug in nvDXT prevents it from converting DDS surfaces with dimensions that aren't even multiples of 4.
	// So, we pad the surface to 4x4 by repeating the bottom and right texels so the DXT interpolation works correctly.
	const Surface* surface = aSrcSurface;
	Surface tmp_surface;
	if ( (nvidia_options.textureFormat >= kDXT1 && nvidia_options.textureFormat <= kDXT5) ||
		 nvidia_options.textureFormat == kDXT5_NM )
	{
		if ( aSrcSurface->GetWidth() % 4 || aSrcSurface->GetHeight() % 4 )
		{
//#pragma TODO( "Consider using ActTools::ResizeSurface()" )
			tmp_surface.theTexture = aSrcSurface->theTexture;
			tmp_surface.Create( align(aSrcSurface->GetWidth(),4), align(aSrcSurface->GetHeight(),4) );
			for ( size_t y = 0; y < tmp_surface.GetHeight(); ++y ) 
				for ( size_t x = 0; x < tmp_surface.GetWidth(); ++x )
					((u32*)tmp_surface.theTexels)[y*tmp_surface.GetWidth()+x] = 
						((u32*)aSrcSurface->GetTexels())[min(y,aSrcSurface->GetHeight()-1)*aSrcSurface->GetWidth()+min(x,aSrcSurface->GetWidth()-1)];
			surface = &tmp_surface;
		}
	}

	HRESULT h_result = nvDDS::nvDXTcompress( (U8*)surface->theTexels,
	                                      (uint)surface->GetWidth(), (uint)surface->GetHeight(), (uint)surface->GetWidth()*4, 
										  nvBGRA,
	                                      &nvidia_options, StoreMip, (const RECT*)NULL );
	assert(h_result == 0);

	if ( surface == &tmp_surface )
		tmp_surface.Destroy();
}

void nvDXTTexelFormat::CopyToA8R8G8B8( Surface* aDstSurface, const Surface* aSrcSurface ) const
{
	assert(0);
}

TexelFormat* GetTexelFormat( const char* aFormatId, const char* aQuality )
{
	int quality = kQualityNormal;
	if ( !_stricmp( aQuality, "fastest" ) ) quality = kQualityFastest;
	if ( !_stricmp( aQuality, "highest" ) ) quality = kQualityHighest;
	if ( !_stricmp( aFormatId, "DXT1" ) ) return new nvDXTTexelFormat( kDXT1, quality, aFormatId );
	if ( !_stricmp( aFormatId, "DXT3" ) ) return new nvDXTTexelFormat( kDXT3, quality, aFormatId );
	if ( !_stricmp( aFormatId, "DXT5" ) ) return new nvDXTTexelFormat( kDXT5, quality, aFormatId );
	if ( !_stricmp( aFormatId, "V8U8" ) ) return new nvDXTTexelFormat( kV8U8, quality, aFormatId ); // DuDv 
	if ( !_stricmp( aFormatId, "CxV8U8" ) ) return new nvDXTTexelFormat( kCxV8U8, quality, aFormatId ); // normal map
	if ( !_stricmp( aFormatId, "A8" ) ) return new nvDXTTexelFormat( kA8, quality, aFormatId ); // alpha only
	if ( !_stricmp( aFormatId, "A8L8" ) ) return new nvDXTTexelFormat( kA8L8, quality, aFormatId ); // luminance
	if ( !_stricmp( aFormatId, "DXT5_NM" ) ) return new nvDXTTexelFormat( kDXT5_NM, quality, "DXT5" ); // DXT5 normal map compression. G = Y, alpha = X
	if ( !_stricmp( aFormatId, "3Dc" ) ) return new nvDXTTexelFormat( k3Dc, quality, aFormatId ); // ATI's normal map scheme
	return NULL;
}

} //namespace NvDXT


