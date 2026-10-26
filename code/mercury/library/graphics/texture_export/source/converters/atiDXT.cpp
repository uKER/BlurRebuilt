#include "stdafx.h"

#include <core/containers/string.h>
#include <windows.h>

#include "ATI_Compress/ATI_Compress.h"

#include "texture_export/Texture.h"

#include "texture_export/converters/atiDXT.h"

using ActTools::TexelFormat;
using ActTools::Texture;
using ActTools::Surface;

namespace AtiDXT
{

size_t theBits[] = {
	32, // ATI_TC_FORMAT_ARGB_8888
	32, // ATI_TC_FORMAT_ARGB_2101010
    16, // ATI_TC_FORMAT_ARGB_16
    128, // ATI_TC_FORMAT_ARGB_32F
    4, // ATI_TC_FORMAT_DXT1
    8, // ATI_TC_FORMAT_DXT3
    8, // ATI_TC_FORMAT_DXT5
	8, // ATI_TC_FORMAT_DXT5_xGBR
	8, // ATI_TC_FORMAT_DXT5_RxBG
	8, // ATI_TC_FORMAT_DXT5_RBxG
	8, // ATI_TC_FORMAT_DXT5_xRBG
	8, // ATI_TC_FORMAT_DXT5_RGxB
	8, // ATI_TC_FORMAT_DXT5_xGxR
	0, // ATI_TC_FORMAT_ATI1N
    0, // ATI_TC_FORMAT_ATI2N
    0, // ATI_TC_FORMAT_ATI2N_XY
    8, // ATI_TC_FORMAT_ATI2N_DXT5
};

class AtiDXTTexelFormat : public TexelFormat
{
	int theAtiFmtId; 
	Util::C_String theId;

public:
	AtiDXTTexelFormat( int aAtiFmtId, const char* aId ) :
		theAtiFmtId( aAtiFmtId ), theId( aId )
	{
		assert( aAtiFmtId <= ATI_TC_FORMAT_MAX && aId && *aId );
	}

	size_t Bits() const
	{
		return theBits[theAtiFmtId];
	}

	size_t Bytes( size_t aWidth, size_t aHeight ) const
	{
		size_t n;
#if 0
		// ignoring DXT format for now, overallocates by several times.
		// would be nice if nvdxtlib had a function to return the size of a texture.
		n = max( 4, aWidth ) * max( 4, aHeight ) * 4; 
#else
		ATI_TC_Texture tex;
		tex.dwSize = sizeof(tex);
		tex.dwWidth = aWidth;
		tex.dwHeight = aHeight;
		tex.dwPitch = 0;
		tex.format = (ATI_TC_FORMAT)theAtiFmtId;
		n = ATI_TC_CalculateBufferSize( &tex );
#endif
		return n;
	}

	size_t BlockSize() const {return 4;}

	virtual const char* GetId() const 
	{
		return theId;
	}

	void CopyFromA8R8G8B8( Surface* aDstSurface, const Surface* aSrcSurface ) const
	{
		assert( aSrcSurface->GetWidth() == aDstSurface->GetWidth() &&
		        aSrcSurface->GetHeight() == aDstSurface->GetHeight() );
		ATI_TC_Texture src;
		src.dwSize = sizeof(src);
		src.dwWidth = aSrcSurface->GetWidth();
		src.dwHeight = aSrcSurface->GetHeight();
		src.dwPitch = aSrcSurface->GetFormat()->Bytes( aSrcSurface->GetWidth(), 1 );
		src.format = ATI_TC_FORMAT_ARGB_8888;
		src.dwDataSize = ATI_TC_CalculateBufferSize( &src );
		assert( src.dwDataSize <= aSrcSurface->GetSize() );
		src.pData = (ATI_TC_BYTE*)aSrcSurface->GetTexels();

		ATI_TC_Texture dst;
		dst.dwSize = sizeof(dst);
		dst.dwWidth = aDstSurface->GetWidth();
		dst.dwHeight = aDstSurface->GetHeight();
		dst.dwPitch = 0;
		dst.format = (ATI_TC_FORMAT)theAtiFmtId;
		dst.dwDataSize = ATI_TC_CalculateBufferSize( &dst );
		assert( ATI_TC_CalculateBufferSize(&dst) <= aDstSurface->GetSize() );
		dst.pData = (ATI_TC_BYTE*)aDstSurface->GetTexels();

		ATI_TC_CompressOptions options = { 0 };
		options.dwSize = sizeof(options);
		if ( ATI_TC_FORMAT_DXT1 == dst.format )
		{
			const int alpha = aSrcSurface->GetAlphaPresence();
			if ( Texture::ALPHA_GREYSCALE == alpha || Texture::ALPHA_BLACKWHITE == alpha )
			{
				options.bDXT1UseAlpha = TRUE;
				options.nAlphaThreshold = 127;
			}
		}
		ATI_TC_ERROR ati_err = ATI_TC_ConvertTexture( &src, &dst, &options, NULL, NULL, NULL );
		assert( ATI_TC_OK == ati_err );
	}
	void CopyToA8R8G8B8( Surface* aDstSurface, const Surface* aSrcSurface ) const
	{
		assert( aSrcSurface->GetWidth() == aDstSurface->GetWidth() &&
		        aSrcSurface->GetHeight() == aDstSurface->GetHeight() );

		ATI_TC_Texture src;
		src.dwSize = sizeof(src);
		src.dwWidth = aSrcSurface->GetWidth();
		src.dwHeight = aSrcSurface->GetHeight();
		src.dwPitch = 0;
		src.format = (ATI_TC_FORMAT)theAtiFmtId;
		src.dwDataSize = ATI_TC_CalculateBufferSize( &src );
		assert( ATI_TC_CalculateBufferSize(&src) <= aSrcSurface->GetSize() );
		src.pData = (ATI_TC_BYTE*)aSrcSurface->GetTexels();

		ATI_TC_Texture dst;
		dst.dwSize = sizeof(dst);
		dst.dwWidth = aDstSurface->GetWidth();
		dst.dwHeight = aDstSurface->GetHeight();
		dst.dwPitch = aDstSurface->GetFormat()->Bytes( aDstSurface->GetWidth(), 1 );
		dst.format = ATI_TC_FORMAT_ARGB_8888;
		dst.dwDataSize = ATI_TC_CalculateBufferSize( &dst );
		assert( dst.dwDataSize <= aDstSurface->GetSize() );
		dst.pData = (ATI_TC_BYTE*)aDstSurface->GetTexels();

		ATI_TC_CompressOptions options = { 0 };
		options.dwSize = sizeof(options);
		ATI_TC_ConvertTexture( &src, &dst, &options, NULL, NULL, NULL );
	}

	TexelFormat* Clone() const
	{
		return new AtiDXTTexelFormat( theAtiFmtId, theId );
	}
	Class GetClass() const { return CLASS_INTEGER; }
};

TexelFormat* GetTexelFormat( const char* aFormatId )
{
	if ( !_stricmp( aFormatId, ATIDXT_ARGB_8888    ) ) return new AtiDXTTexelFormat( ATI_TC_FORMAT_ARGB_8888,    aFormatId );
	if ( !_stricmp( aFormatId, ATIDXT_ARGB_2101010 ) ) return new AtiDXTTexelFormat( ATI_TC_FORMAT_ARGB_2101010, aFormatId );
	if ( !_stricmp( aFormatId, ATIDXT_ARGB_16      ) ) return new AtiDXTTexelFormat( ATI_TC_FORMAT_ARGB_16,      aFormatId );
	if ( !_stricmp( aFormatId, ATIDXT_ARGB_32F     ) ) return new AtiDXTTexelFormat( ATI_TC_FORMAT_ARGB_32F,     aFormatId );
	if ( !_stricmp( aFormatId, ATIDXT_DXT1         ) ) return new AtiDXTTexelFormat( ATI_TC_FORMAT_DXT1,         aFormatId );
	if ( !_stricmp( aFormatId, ATIDXT_DXT3         ) ) return new AtiDXTTexelFormat( ATI_TC_FORMAT_DXT3,         aFormatId );
	if ( !_stricmp( aFormatId, ATIDXT_DXT5         ) ) return new AtiDXTTexelFormat( ATI_TC_FORMAT_DXT5,         aFormatId );
	if ( !_stricmp( aFormatId, ATIDXT_DXT5_xGBR    ) ) return new AtiDXTTexelFormat( ATI_TC_FORMAT_DXT5_xGBR,    aFormatId );
	if ( !_stricmp( aFormatId, ATIDXT_DXT5_RxBG    ) ) return new AtiDXTTexelFormat( ATI_TC_FORMAT_DXT5_RxBG,    aFormatId );
	if ( !_stricmp( aFormatId, ATIDXT_DXT5_RBxG    ) ) return new AtiDXTTexelFormat( ATI_TC_FORMAT_DXT5_RBxG,    aFormatId );
	if ( !_stricmp( aFormatId, ATIDXT_DXT5_xRBG    ) ) return new AtiDXTTexelFormat( ATI_TC_FORMAT_DXT5_xRBG,    aFormatId );
	if ( !_stricmp( aFormatId, ATIDXT_DXT5_RGxB    ) ) return new AtiDXTTexelFormat( ATI_TC_FORMAT_DXT5_RGxB,    aFormatId );
	if ( !_stricmp( aFormatId, ATIDXT_DXT5_xGxR    ) ) return new AtiDXTTexelFormat( ATI_TC_FORMAT_DXT5_xGxR,    aFormatId );
	if ( !_stricmp( aFormatId, ATIDXT_ATI1N        ) ) return new AtiDXTTexelFormat( ATI_TC_FORMAT_ATI1N,        aFormatId );
	if ( !_stricmp( aFormatId, ATIDXT_ATI2N        ) ) return new AtiDXTTexelFormat( ATI_TC_FORMAT_ATI2N,        aFormatId );
	if ( !_stricmp( aFormatId, ATIDXT_ATI2N_XY     ) ) return new AtiDXTTexelFormat( ATI_TC_FORMAT_ATI2N_XY,     aFormatId );
	if ( !_stricmp( aFormatId, ATIDXT_ATI2N_DXT5   ) ) return new AtiDXTTexelFormat( ATI_TC_FORMAT_ATI2N_DXT5,   aFormatId );

	assert(false);
	return 0;
}

};

