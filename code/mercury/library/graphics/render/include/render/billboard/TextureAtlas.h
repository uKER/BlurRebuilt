#ifndef RENDER_BILLBOARD_TEXTURE_ATLAS_H
#define RENDER_BILLBOARD_TEXTURE_ATLAS_H

#if _MSC_VER > 1000
#pragma once
#endif



#include "Render/Billboard/TextureCoordinates.h"



namespace Render
{
namespace Billboard
{



class C_TextureAtlas
{

public:

	C_TextureAtlas();
	~C_TextureAtlas();

	void Resize(const U32 iWidth, const U32 iHeight);

	const TextureCoords & GetTextureCoords(const U32 iIndex) const;

private:
	
	TextureCoords*	m_pTextureCoords;
	U32				m_iWidth;
	U32				m_iHeight;

};



FORCE_INLINE C_TextureAtlas::C_TextureAtlas()
{
	m_iWidth = 0;
	m_iHeight = 0;
	m_pTextureCoords = 0;
}

FORCE_INLINE C_TextureAtlas::~C_TextureAtlas()
{
	if (m_pTextureCoords)
		delete[] m_pTextureCoords;
}

FORCE_INLINE void C_TextureAtlas::Resize(const U32 iWidth, const U32 iHeight)
{
	m_iWidth = iWidth;
	m_iHeight = iHeight;

	if (m_pTextureCoords)
		delete[] m_pTextureCoords;

	m_pTextureCoords = new TextureCoords[iWidth * iHeight];

	const F32 fWidth = 1.0f / (F32) iWidth;
	const F32 fHeight = 1.0f / (F32) iHeight;

	U32 iIndex = 0;
	for (U32 y = 0; y < iHeight; ++y)
	{
		for (U32 x = 0; x < iWidth; ++x)
		{
			const F32 fu = x * fWidth;
			const F32 fv = y * fHeight;

			m_pTextureCoords[iIndex].u0 = fu;
			m_pTextureCoords[iIndex].v0 = fHeight + fv;
			m_pTextureCoords[iIndex].u1 = fWidth + fu;
			m_pTextureCoords[iIndex].v1 = fHeight + fv;
			m_pTextureCoords[iIndex].u2 = fWidth + fu;
			m_pTextureCoords[iIndex].v2 = fv;
			m_pTextureCoords[iIndex].u3 = fu;
			m_pTextureCoords[iIndex].v3 = fv;

			++iIndex;
		}
	}
}

FORCE_INLINE const TextureCoords & C_TextureAtlas::GetTextureCoords(const U32 iIndex) const
{
	assert(iIndex < m_iWidth * m_iHeight);
	return m_pTextureCoords[iIndex];
}



}; // namespace Billboard
}; // namespace Render



#endif // RENDER_BILLBOARD_TEXTURE_ATLAS_H
