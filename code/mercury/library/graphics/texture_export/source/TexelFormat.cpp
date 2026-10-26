#include "stdafx.h"

#include "texture_export/TexelFormat.h"

#include "texture_export/Texture.h"
#include "texture_export/Surface.h"

namespace ActTools
{

int TexelFormat::Compare( const TexelFormat* aSF ) const
{
	return _stricmp( GetId(), aSF->GetId() );
}

void TexelFormat::Get( const void* ACTT_UNUSED(pixel), int* ACTT_UNUSED(r), int* ACTT_UNUSED(g), int* ACTT_UNUSED(b), int* ACTT_UNUSED(a) ) const
{
	assert( 0 );
}

void TexelFormat::Set( void* ACTT_UNUSED(pixel), int ACTT_UNUSED(r), int ACTT_UNUSED(g), int ACTT_UNUSED(b), int ACTT_UNUSED(a) ) const
{
	assert( 0 );
}

void TexelFormat::CopyFromA8R8G8B8( ActTools::Surface* ACTT_UNUSED(aDstSurface), const ActTools::Surface* ACTT_UNUSED(aSrcSurface) ) const
{
	assert(false);
}

void TexelFormat::CopyToA8R8G8B8( ActTools::Surface* ACTT_UNUSED(aDstSurface), const ActTools::Surface* ACTT_UNUSED(aSrcSurface) ) const
{
	assert(false);
}

ACTRESULT TexelFormat::CopyToP8( ActTools::Surface* ACTT_UNUSED(aDstSurface), const ActTools::Surface* ACTT_UNUSED(aSrcSurface) ) const
{
	return ACT_ERROR;
}

ACTRESULT TexelFormat::CopyFromP8( ActTools::Surface* ACTT_UNUSED(aDstSurface), const ActTools::Surface* ACTT_UNUSED(aSrcSurface) ) const
{
	return ACT_ERROR;
}

void TexelFormat::CopyToA32R32G32B32F( ActTools::Surface* ACTT_UNUSED(aDstSurface), const ActTools::Surface* ACTT_UNUSED(aSrcSurface) ) const
{
	assert(false);
}

void TexelFormat::CopyFromA32R32G32B32F( ActTools::Surface* ACTT_UNUSED(aDstSurface), const ActTools::Surface* ACTT_UNUSED(aSrcSurface) ) const
{
	assert(false);
}


void NullTexelFormat::CopyFromA8R8G8B8( ActTools::Surface* ACTT_UNUSED(aDstSurface), const ActTools::Surface* ACTT_UNUSED(aSrcSurface) ) const
{
}

void NullTexelFormat::CopyToA8R8G8B8( ActTools::Surface* ACTT_UNUSED(aDstSurface), const ActTools::Surface* ACTT_UNUSED(aSrcSurface) ) const 
{
}


TexelFormat* GetTexelFormat( const char* format )
{
	// 8bit
	if (_stricmp(format,"A8") == 0) return new A8;
	if (_stricmp(format,"L8") == 0) return new L8;
	// 16bit
	if (_stricmp(format,"R5G5B5") == 0) return new R5G5B5;
	if (_stricmp(format,"G8R8")==0) return new G8R8;
	if (_stricmp(format,"G8B8")==0) return new G8B8;
	if (_stricmp(format,"A8L8") == 0) return new A8L8;
	// 32bit
	if (_stricmp(format,"A8R8G8B8") == 0) return new A8R8G8B8;
	if (_stricmp(format,"A8B8G8R8") == 0) return new A8B8G8R8;
	if (_stricmp(format,"B8G8R8A8") == 0) return new B8G8R8A8;
	if (_stricmp(format,"R8G8B8A8") == 0) return new R8G8B8A8;
	if (_stricmp(format,"R8G8B8") == 0) return new R8G8B8;
	if (_stricmp(format,"B8G8R8") == 0) return new B8G8R8;
	// floating point
	if (_stricmp(format,"A32R32G32B32F") == 0) return new A32R32G32B32F;
	if (_stricmp(format,"A32B32G32R32F") == 0) return new A32B32G32R32F;
	return NULL;
}

// The order of argb8 is backwards because on little-endian intel architectures, the most significant byte comes last.
// When treated as a 32bit value, the layout is 0xAARRGGBB.
struct argb8 { u8 b, g, r, a; };
struct argb32f { float a, r, g, b; };
struct abgr32f { float a, b, g, r; };

void L8::CopyFromA8R8G8B8( Surface* aDstSurface, const Surface* aSrcSurface ) const
{
	u8* first = reinterpret_cast<u8*>(aDstSurface->GetTexels());
	u8* last = first + aDstSurface->GetWidth()*aDstSurface->GetHeight();
	u8* dst = first;
	const argb8* src = reinterpret_cast<const argb8*>(aSrcSurface->GetTexels());
	for ( dst = first; dst != last; ++dst, ++src )
	{
		int sum = (int)src->r + (int)src->g + (int)src->b;
		*dst = static_cast<u8>(sum / 3);
	}
}

void L8::CopyToA8R8G8B8( Surface* aDstSurface, const Surface* aSrcSurface ) const
{
	argb8* first = reinterpret_cast<argb8*>(aDstSurface->GetTexels());
	argb8* last = first + aDstSurface->GetWidth()*aDstSurface->GetHeight();
	argb8* dst = first;
	const u8* src = reinterpret_cast<const u8*>(aSrcSurface->GetTexels());
	for ( dst = first; dst != last; ++dst, ++src )
	{
		dst->a = 255;
		dst->r = *src;
		dst->g = *src;
		dst->b = *src;
	}
}

void A8::CopyFromA8R8G8B8( Surface* aDstSurface, const Surface* aSrcSurface ) const
{
	u8* first = reinterpret_cast<u8*>(aDstSurface->GetTexels());
	u8* last = first + aDstSurface->GetWidth()*aDstSurface->GetHeight();
	u8* dst = first;
	const argb8* src = reinterpret_cast<const argb8*>(aSrcSurface->GetTexels());
	for ( dst = first; dst != last; ++dst, ++src )
	{
		*dst = src->a;
	}
}

void A8::CopyToA8R8G8B8( Surface* aDstSurface, const Surface* aSrcSurface ) const
{
	argb8* first = reinterpret_cast<argb8*>(aDstSurface->GetTexels());
	argb8* last = first + aDstSurface->GetWidth()*aDstSurface->GetHeight();
	argb8* dst = first;
	const u8* src = reinterpret_cast<const u8*>(aSrcSurface->GetTexels());
	for ( dst = first; dst != last; ++dst, ++src )
	{
		dst->a = *src;
	}
}
void A8L8::CopyFromA8R8G8B8( Surface* aDstSurface, const Surface* aSrcSurface ) const
{
	u16* first = reinterpret_cast<u16*>(aDstSurface->GetTexels());
	u16* last = first + aDstSurface->GetWidth()*aDstSurface->GetHeight();
	u16* dst = first;
	const argb8* src = reinterpret_cast<const argb8*>(aSrcSurface->GetTexels());
	for ( dst = first; dst != last; ++dst, ++src )
	{
		int sum = (int)src->r + (int)src->g + (int)src->b;
		*dst = static_cast<u16>((sum / 3) + (src->a << 8));
	}
}

void A8L8::CopyToA8R8G8B8( Surface* aDstSurface, const Surface* aSrcSurface ) const
{
	argb8* first = reinterpret_cast<argb8*>(aDstSurface->GetTexels());
	argb8* last = first + aDstSurface->GetWidth()*aDstSurface->GetHeight();
	argb8* dst = first;
	const u16* src = reinterpret_cast<const u16*>(aSrcSurface->GetTexels());
	for ( dst = first; dst != last; ++dst, ++src )
	{
		dst->a = static_cast<u8>(((*src) >> 8) & 0xff);
		dst->r =
		dst->g =
		dst->b = static_cast<u8>((*src) & 0xff);
	}
}

void PalettedFormat::CopyToA32R32G32B32F( Surface* ACTT_UNUSED(aDstSurface), const Surface* ACTT_UNUSED(aSrcSurface) ) const
{
	assert(false);
}

void PalettedFormat::CopyFromA32R32G32B32F( Surface* ACTT_UNUSED(aDstSurface), const Surface* ACTT_UNUSED(aSrcSurface) ) const
{
	assert(false);
}

void P4::CopyToA8R8G8B8( ActTools::Surface* ACTT_UNUSED(aDstSurface), const ActTools::Surface* ACTT_UNUSED(aSrcSurface) ) const
{
}

ACTRESULT P4::CopyFromP8( Surface* aDstSurface, const Surface* aSrcSurface ) const
{
	if( aDstSurface->GetFormat()->GetClass() != aSrcSurface->GetFormat()->GetClass())
		return ACT_ERROR;

	u8* first = reinterpret_cast<u8*>(aDstSurface->GetTexels());
	u8* last = first + ( aDstSurface->GetWidth()*aDstSurface->GetHeight() )/2;
	const u8* src = reinterpret_cast<const u8*>(aSrcSurface->GetTexels());
	for ( u8* dst = first; dst != last; ++dst, ++src )
	{
		*dst = *(src++) << 4;
		*dst = *dst | (*src & 0x0f);
	}
	return ACT_OK;
}

ACTRESULT P4::CopyToP8( Surface* aDstSurface, const Surface* aSrcSurface ) const
{
	if( aDstSurface->GetFormat()->GetClass() != aSrcSurface->GetFormat()->GetClass())
		return ACT_ERROR;
	u8* first = reinterpret_cast<u8*>(aDstSurface->GetTexels());
	u8* last = first + aDstSurface->GetWidth()*aDstSurface->GetHeight();
	const u8* src = reinterpret_cast<const u8*>(aSrcSurface->GetTexels());
	for ( u8* dst = first; dst != last; ++dst, ++src )
	{
		*dst = ((*src)& 0xf0 ) >> 4;
		*(++dst) = (*src)& 0x0f;
	}
	return ACT_OK;
}

void P8::CopyToA8R8G8B8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const
{
	assert(aDstSurface->GetWidth() == aSrcSurface->GetWidth() && aDstSurface->GetHeight() == aSrcSurface->GetHeight());
	assert ( aSrcSurface->GetPalette() );

	int r, g, b, a;
	size_t count = aDstSurface->GetWidth() * aDstSurface->GetHeight();
	const u8* src = aSrcSurface->GetTexels();
	u8* dst = aDstSurface->GetTexels();
	while ( count-- )
	{
		aSrcSurface->GetPalette()->Get( *src++, &r, &g, &b, &a );
		*dst++ = static_cast<u8>(b);
		*dst++ = static_cast<u8>(g);
		*dst++ = static_cast<u8>(r);
		*dst++ = static_cast<u8>(a);
	}
}

ACTRESULT P8::CopyFromP8( Surface* aDstSurface, const Surface* aSrcSurface ) const
{
	if( aDstSurface->GetFormat()->GetClass() != aSrcSurface->GetFormat()->GetClass())
		return ACT_ERROR;
	// Nothing to be done with the format, just copy the texels
	memcpy( aDstSurface->GetTexels(), aSrcSurface->GetTexels(), aDstSurface->GetSize() ); 
	return ACT_OK;
}

ACTRESULT P8::CopyToP8( Surface* aDstSurface, const Surface* aSrcSurface ) const
{
	if( aDstSurface->GetFormat()->GetClass() != aSrcSurface->GetFormat()->GetClass())
		return ACT_ERROR;
	// Nothing to be done with the format, just copy the texels
	memcpy( aDstSurface->GetTexels(), aSrcSurface->GetTexels(), aDstSurface->GetSize() ); 
	return ACT_OK;
}

void A32R32G32B32F::Get( const void* ACTT_UNUSED(pixel), int* ACTT_UNUSED(r), int* ACTT_UNUSED(g), int* ACTT_UNUSED(b), int* ACTT_UNUSED(a) ) const
{
	assert(false && "Not implemented yet\n");
}

void A32R32G32B32F::Set( void* ACTT_UNUSED(pixel), int ACTT_UNUSED(r), int ACTT_UNUSED(g), int ACTT_UNUSED(b), int ACTT_UNUSED(a) ) const
{
	assert(false && "Not implemented yet\n");
}

void A32R32G32B32F::CopyToA32R32G32B32F( Surface* aDstSurface, const Surface* aSrcSurface ) const
{
	assert( aDstSurface->GetWidth() == aSrcSurface->GetWidth() && aDstSurface->GetHeight() == aSrcSurface->GetHeight() );
	assert( aDstSurface->GetFormat()->GetClass() == aSrcSurface->GetFormat()->GetClass() );
	// Nothing to be done with the format, just copy the texels
	memcpy( aDstSurface->GetTexels(), aSrcSurface->GetTexels(), aDstSurface->GetSize() ); 
}

void A32R32G32B32F::CopyFromA32R32G32B32F( Surface* aDstSurface, const Surface* aSrcSurface ) const
{
	assert( aDstSurface->GetWidth() == aSrcSurface->GetWidth() && aDstSurface->GetHeight() == aSrcSurface->GetHeight() );
	assert( aDstSurface->GetFormat()->GetClass() == aSrcSurface->GetFormat()->GetClass() );
	// Nothing to be done with the format, just copy the texels
	memcpy( aDstSurface->GetTexels(), aSrcSurface->GetTexels(), aDstSurface->GetSize() ); 
}

void A32R32G32B32F::CopyFromA8R8G8B8( Surface* aDstSurface, const Surface* aSrcSurface ) const
{
	argb32f* first = reinterpret_cast<argb32f*>(aDstSurface->GetTexels());
	argb32f* last = first + aDstSurface->GetWidth()*aDstSurface->GetHeight();
	argb32f* dst = first;
	const argb8* src = reinterpret_cast<const argb8*>(aSrcSurface->GetTexels());
	for ( dst = first; dst != last; dst++, src++ )
	{
		dst->a = ((float)(int)src->a) / 255.0f;
		dst->r = ((float)(int)src->r) / 255.0f;
		dst->g = ((float)(int)src->g) / 255.0f;
		dst->b = ((float)(int)src->b) / 255.0f;
	}
}

void A32B32G32R32F::CopyFromA8R8G8B8( Surface* aDstSurface, const Surface* aSrcSurface ) const
{
	abgr32f* first = reinterpret_cast<abgr32f*>(aDstSurface->GetTexels());
	abgr32f* last = first + aDstSurface->GetWidth()*aDstSurface->GetHeight();
	abgr32f* dst = first;
	const argb8* src = reinterpret_cast<const argb8*>(aSrcSurface->GetTexels());
	for ( dst = first; dst != last; dst++, src++ )
	{
		dst->a = ((float)(int)src->a) / 255.0f;
		dst->r = ((float)(int)src->r) / 255.0f;
		dst->g = ((float)(int)src->g) / 255.0f;
		dst->b = ((float)(int)src->b) / 255.0f;
	}
}


}
