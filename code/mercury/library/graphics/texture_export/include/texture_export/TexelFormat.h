/*! \file Surface.h
	\brief ActToolsLib Surface module.
*/

/**
	\defgroup ACTTOOLSLIB_TEXELFORMAT TexelFormat Class
	@{
*/
#ifndef SURFACEFORMAT_H__INCLUDED
#define SURFACEFORMAT_H__INCLUDED

#include "ngl/defs.h"
#include "ngl/assert.h"

namespace ActTools 
{

class Surface;

/**
 * A TexelFormat class
 * A base class for all texel formats
 */
class TexelFormat
{
public:
	enum Class
	{
		CLASS_PALETTE,
		CLASS_INTEGER,
		CLASS_FLOAT,
		CLASS_INVALID
	};

	/** Returns the number of bits used for a single texel
	 */
	virtual size_t Bits() const { assert(false); return 0; }

	/** Returns the number of bytes neded for a surface with speified width and height having this texel format
	 */
	virtual size_t Bytes( size_t aWidth, size_t aHeight ) const
	{
		return (aWidth*aHeight*Bits() + 7) / 8;
	}

	virtual size_t BlockSize() const {return 1;}

	/** Returns a string id for this texel format
	*/
	virtual const char* GetId() const = 0;

	int Compare( const TexelFormat* aSF ) const;
	/** Returns TRUE if a specfied texel format is equivalent to this one
	*/
	int operator == ( const TexelFormat& aSF ) const
	{
		return 0 == Compare( &aSF );
	}
	/** Gets the red green and blue component for a texel
	 */
	virtual void Get( const void* pixel, int* r, int* g, int* b, int* a ) const;
	/** Set the value for a texel specified by red, green and blue components
	 */
	virtual void Set( void* pixel, int r, int g, int b, int a ) const;
	/** Copy texels from 32bit ARGB surfate to a surface specified by this texel format
	 */
	virtual void CopyFromA8R8G8B8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;
	/** Copy texels to a 32bit ARGB surfate from a surface specified by this texel format
	 */
	virtual void CopyToA8R8G8B8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;
	/** Copy texels to 8bit paletted surface
	 */
	virtual ACTRESULT CopyToP8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;
	/** Copy texels from 8bit paletted surface
	 */
	virtual ACTRESULT CopyFromP8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;
	/** Copy texels to 32bit ARGB surfate from a 128bit floating poit surface
	 */
	virtual void CopyToA32R32G32B32F( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;
	/** Copy texels from a 32bit ARGB surfate to a 128bit floating poit surface
	 */
	virtual void CopyFromA32R32G32B32F( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;
	/** Returns a new instance of a TexelFormat equivalent to this one
	 */
	virtual TexelFormat* Clone() const = 0;
	/** retrieves the class for the texels: CLASS_PALETTE, CLASS_INTEGER, CLASS_FLOAT and CLASS_INVALID
	 */
	virtual Class GetClass() const = 0;
};

/* Returns an instance of a TexelFormat object specified by a string ID
 */
TexelFormat* GetTexelFormat( const char* format );

/**
 * A NullTexelFormat class
 */
class NullTexelFormat : public ActTools::TexelFormat
{
public:
	const char* GetId() const { return "INVALID"; }
	size_t Bits() const { return 0; }

	void CopyFromA8R8G8B8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;
	void CopyToA8R8G8B8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;

	TexelFormat* Clone() const { return new NullTexelFormat; }

	Class GetClass() const { return CLASS_INVALID; }
};

/**
 * A UnknownTexelFormat class
 */
class UnknownTexelFormat : public ActTools::TexelFormat
{
public:
	UnknownTexelFormat() { strcpy(m_IdName, "INVALID"); }
	UnknownTexelFormat(const char *psz) { strcpy(m_IdName, psz); }

	const char* GetId() const { return m_IdName; }
	size_t Bits() const { assert(0); return 0; }

	TexelFormat* Clone() const { return new UnknownTexelFormat(m_IdName); }

	Class GetClass() const { return CLASS_INVALID; }

	char m_IdName[512];
};

/**
 * A TexelFormat_StandardInt class
 * Instances of this template class can represent any 16/24/32 bit texel format class
 */
template <int bits, int apos,int abits, int rpos,int rbits, int gpos,int gbits, int bpos,int bbits>
class TexelFormat_StandardInt : public ActTools::TexelFormat
{
public:
	size_t Bits() const { return bits; }

	void CopyFromA8R8G8B8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const
	{
		assert(bits <= 32 && (bits&7) == 0);
		assert(aDstSurface->GetWidth() == aSrcSurface->GetWidth() && aDstSurface->GetHeight() == aSrcSurface->GetHeight());
		size_t count = aDstSurface->GetWidth()*aDstSurface->GetHeight();
		const u8* src = (const u8*)aSrcSurface->GetTexels();
		u8* dst = aDstSurface->GetTexels();
		while (count--)
		{
			u32 tmp;
			tmp  = bbits ? (u32(src[0]) >> (8-bbits)) << bpos : 0;
			tmp |= gbits ? (u32(src[1]) >> (8-gbits)) << gpos : 0;
			tmp |= rbits ? (u32(src[2]) >> (8-rbits)) << rpos : 0;
			tmp |= abits ? (u32(src[3]) >> (8-abits)) << apos : 0;
			memcpy( dst, &tmp, bits/8 ); // KGT: compiler should optimize this memcpy because it have constant size blocks
			src += 4;
			dst += bits/8;
		}
	}

	void CopyToA8R8G8B8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const
	{
		assert(bits <= 32 && (bits&7) == 0);
		assert(aDstSurface->GetWidth() == aSrcSurface->GetWidth() && aDstSurface->GetHeight() == aSrcSurface->GetHeight());
		size_t count = aDstSurface->GetWidth()*aDstSurface->GetHeight();
		const u32* src = (const u32*)aSrcSurface->GetTexels();
		u8* dst = (u8*)aDstSurface->GetTexels();
		while (count--)
		{
			dst[0] = static_cast<u8>(bbits ? (((*src) >> bpos) & ((1<<bbits)-1)) << (8-bbits) : 0xff);
			dst[1] = static_cast<u8>(gbits ? (((*src) >> gpos) & ((1<<gbits)-1)) << (8-gbits) : 0xff);
			dst[2] = static_cast<u8>(rbits ? (((*src) >> rpos) & ((1<<rbits)-1)) << (8-rbits) : 0xff);
			dst[3] = static_cast<u8>(abits ? (((*src) >> apos) & ((1<<abits)-1)) << (8-abits) : 0xff);
			src = (u32*)( (size_t)src + bits/8 );
			dst += 4;
		}
	}

	// for compabibility with some existing AC modules
	void Get( const void* pixel, int* r, int* g, int* b, int* a ) const
	{
		const u32* src = (const u32*)pixel;
		*r = rbits ? (((*src) >> rpos) & ((1<<rbits)-1)) << (8-rbits) : 0xff;
		*g = gbits ? (((*src) >> gpos) & ((1<<gbits)-1)) << (8-gbits) : 0xff;
		*b = bbits ? (((*src) >> bpos) & ((1<<bbits)-1)) << (8-bbits) : 0xff;
		*a = abits ? (((*src) >> apos) & ((1<<abits)-1)) << (8-abits) : 0xff;
	}
	void Set( void* pixel, int r, int g, int b, int a ) const
	{
		u32* dst = (u32*)pixel;
		*dst |= rbits ? (u32(r) >> (8-rbits)) << rpos : 0;
		*dst |= gbits ? (u32(g) >> (8-gbits)) << gpos : 0;
		*dst  = bbits ? (u32(b) >> (8-bbits)) << bpos : 0;
		*dst |= abits ? (u32(a) >> (8-abits)) << apos : 0;
	}

	TexelFormat* Clone() const { return new TexelFormat_StandardInt<bits,apos,abits,rpos,rbits,gpos,gbits,bpos,bbits>; }

	const char* GetId() const;

	Class GetClass() const { return CLASS_INTEGER; }
};

typedef TexelFormat_StandardInt<16, 0,0, 10,5, 5,5, 0,5> R5G5B5;
typedef TexelFormat_StandardInt<32, 24,8, 16,8, 8,8, 0,8> A8R8G8B8;
typedef TexelFormat_StandardInt<32, 24,8, 0,8, 8,8, 16,8> A8B8G8R8;
typedef TexelFormat_StandardInt<32, 0,8, 8,8, 16,8, 24,8> B8G8R8A8;
typedef TexelFormat_StandardInt<32, 0,8, 24,8, 16,8, 8,8> R8G8B8A8;
typedef TexelFormat_StandardInt<24, 0,0, 16,8, 8,8, 0,8> R8G8B8;
typedef TexelFormat_StandardInt<24, 0,0, 0,8, 8,8, 16,8> B8G8R8;

typedef TexelFormat_StandardInt<16, 0,0, 8,8, 0,8, 0,0> G8R8;
typedef TexelFormat_StandardInt<16, 0,0, 0,0, 8,8, 0,8> G8B8;

// todo- figure out a better solution
inline const char* R5G5B5::GetId() const { return "R5G5B5"; }
inline const char* A8R8G8B8::GetId() const { return "A8R8G8B8"; }
inline const char* A8B8G8R8::GetId() const { return "A8B8G8R8"; }
inline const char* B8G8R8A8::GetId() const { return "B8G8R8A8"; }
inline const char* R8G8B8A8::GetId() const { return "R8G8B8A8"; }
inline const char* R8G8B8::GetId() const { return "R8G8B8"; }
inline const char* B8G8R8::GetId() const { return "B8G8R8"; }

inline const char* G8R8::GetId() const { return "G8R8"; }
inline const char* G8B8::GetId() const { return "G8B8"; }

/**
 * A A32R32G32B32F class
 * A 128bit ARGB floating point texel format class
 */
class A32R32G32B32F : public ActTools::TexelFormat
{
public:
	const char* GetId() const { return "A32R32G32B32F"; }
	size_t Bits() const { return 128; }
	TexelFormat* Clone() const { return new A32R32G32B32F; }

	void Get( const void* pixel, int* r, int* g, int* b, int* a ) const;
	void Set( void* pixel, int r, int g, int b, int a ) const;

	Class GetClass() const { return CLASS_FLOAT; }
	void CopyToA32R32G32B32F( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;
	void CopyFromA32R32G32B32F( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;
	void CopyFromA8R8G8B8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;
};

/**
 * A A32R32G32B32F class
 * A 128bit ABGR floating point texel format class
 */
class A32B32G32R32F : public ActTools::TexelFormat
{
public:
	const char* GetId() const { return "A32B32G32R32F"; }
	size_t Bits() const { return 128; }
	TexelFormat* Clone() const { return new A32B32G32R32F; }

	Class GetClass() const { return CLASS_FLOAT; }
	void CopyFromA8R8G8B8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;
};

/**
 * A R32F class
 * A 32bit R floating point texel format class
 */
class R32F : public ActTools::TexelFormat
{
public:
	const char* GetId() const { return "R32F"; }
	size_t Bits() const { return 32; }
	TexelFormat* Clone() const { return new R32F; }
	Class GetClass() const { return CLASS_FLOAT; }
};

/**
 * A L16 class
 * A 16bit luminance texel format
 */
class L16 : public ActTools::TexelFormat
{
public:
	const char* GetId() const { return "L16"; }
	size_t Bits() const { return 16; }
	TexelFormat* Clone() const { return new L16; }
	Class GetClass() const { return CLASS_INTEGER; }
};


/**
 * A L8 class
 * A 8bit luminance texel format
 */
class L8 : public ActTools::TexelFormat
{
public:
	const char* GetId() const { return "L8"; }
	void CopyFromA8R8G8B8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;
	void CopyToA8R8G8B8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;
	size_t Bits() const { return 8; }
	TexelFormat* Clone() const { return new L8; }
	Class GetClass() const { return CLASS_INTEGER; }
};

typedef TexelFormat_StandardInt<8, 0,8, 0,0, 0,0, 0,0> A8;

inline const char* A8::GetId() const { return "A8"; }

/**
 * A A8L8 class
 * A 16bit alpha+luminance texel format
 */
class A8L8 : public ActTools::TexelFormat
{
public:
	const char* GetId() const { return "A8L8"; }
	void CopyFromA8R8G8B8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;
	void CopyToA8R8G8B8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;
	size_t Bits() const { return 16; }
	TexelFormat* Clone() const { return new A8L8; }
	Class GetClass() const { return CLASS_INTEGER; }
};

/**
 * A PalettedFormat class
 * A base class for paletted texel formats
 */
class PalettedFormat : public ActTools::TexelFormat
{
public:
	Class GetClass() const { return CLASS_PALETTE; }
	void CopyToA32R32G32B32F( Surface* aDstSurface, const Surface* aSrcSurface ) const;
	void CopyFromA32R32G32B32F( Surface* aDstSurface, const Surface* aSrcSurface ) const;
};

/**
 * A P4 class
 * A 4bit paletted texel formats
 */
class P4 : public PalettedFormat
{
public:
	size_t Bits() const { return 4; }
	const char* GetId() const { return "P4"; }
	TexelFormat* Clone() const { return new P4; }

	void CopyToA8R8G8B8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;

	ACTRESULT CopyToP8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;
	ACTRESULT CopyFromP8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;
};

/**
 * A P8 class
 * A 8bit paletted texel formats
 */
class P8 : public PalettedFormat
{
public:
	size_t Bits() const { return 8; }
	const char* GetId() const { return "P8"; }
	TexelFormat* Clone() const { return new P8; }

	void CopyToA8R8G8B8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;

	ACTRESULT CopyToP8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;
	ACTRESULT CopyFromP8( ActTools::Surface* aDstSurface, const ActTools::Surface* aSrcSurface ) const;
};

}

#endif //SURFACEFORMT_H__INCLUDED

// ACTTOOLSLIB_TEXELFORMAT
//@}
