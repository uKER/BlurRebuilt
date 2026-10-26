#ifndef RENDER_DYNAMICEFFECT_TEXTURECOORDS_H
#define RENDER_DYNAMICEFFECT_TEXTURECOORDS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core\utility\Singleton.h>
#include "Dynamic/Enums.h"

#ifndef PLATFORM_SPU
#include <system\misc\random.h>
#endif

namespace DynamicEffect
{

template<class T>
struct TextureCoords
{
	T	u[4];
	T	v[4];
};

struct TextureCoordsMapping
{
	U32	m0, m1, m2, m3;
};

// on SPU get rid of the singleton etc
#ifdef PLATFORM_SPU
class STOREALIGN16 C_TextureCoords
{
public:
#else
class STOREALIGN16 C_TextureCoords : public Ut::Singleton<C_TextureCoords, Ut::CreateStatic>
{
public:
	C_TextureCoords();
	~C_TextureCoords() {}
#endif

	template<class T>
	const TextureCoords<T>&		GetTextureCoordinates(U32 iIndex) const;
	template<class T>
	const TextureCoords<T>&		GetTextureCoordinates(E_Atlas atlas, U32 iIndex) const;
	const TextureCoordsMapping&	GetTextureCoordinatesMapping(U32 index) const;
	U32							GetTextureCoordinateIndex(E_Atlas atlas) const;
	U32							GetAtlasSize(E_Atlas atlas) const;
	U32							GetAtlasOffset(E_Atlas atlas) const;


private:

	TextureCoords<F32>			m_TextureCoordsF32[148];
	TextureCoords<S16>			m_TextureCoordsS16[148];
	TextureCoords<U16>			m_TextureCoordsU16[148];
	U32							m_Offset[E_Atlas_Count];
	U32							m_Size[E_Atlas_Count];
	TextureCoordsMapping		m_Mapping[4];

#ifndef PLATFORM_SPU
	mutable Util::C_Random		m_Random;
#endif
};

template<>
INLINE const TextureCoords<F32>&	C_TextureCoords::GetTextureCoordinates(U32 iIndex) const
{
	return m_TextureCoordsF32[iIndex];
}

template<>
INLINE const TextureCoords<F32>&	C_TextureCoords::GetTextureCoordinates(E_Atlas atlas, U32 iIndex) const
{
	assert(iIndex < m_Size[atlas]);
	return m_TextureCoordsF32[m_Offset[atlas] + iIndex];
}

template<>
INLINE const TextureCoords<U16>&	C_TextureCoords::GetTextureCoordinates(U32 iIndex) const
{
	return m_TextureCoordsU16[iIndex];
}

template<>
INLINE const TextureCoords<U16>&	C_TextureCoords::GetTextureCoordinates(E_Atlas atlas, U32 iIndex) const
{
	assert(iIndex < m_Size[atlas]);
	return m_TextureCoordsU16[m_Offset[atlas] + iIndex];
}

template<>
INLINE const TextureCoords<S16>&	C_TextureCoords::GetTextureCoordinates(U32 iIndex) const
{
	return m_TextureCoordsS16[iIndex];
}

template<>
INLINE const TextureCoords<S16>&	C_TextureCoords::GetTextureCoordinates(E_Atlas atlas, U32 iIndex) const
{
	assert(iIndex < m_Size[atlas]);
	return m_TextureCoordsS16[m_Offset[atlas] + iIndex];
}

INLINE const TextureCoordsMapping&	C_TextureCoords::GetTextureCoordinatesMapping(U32 iIndex) const
{
	assert(iIndex < 4);
	return m_Mapping[iIndex];
}

}

#endif
