#ifndef RENDER_BILLBOARD_TEXTURE_COORDINATES_H
#define RENDER_BILLBOARD_TEXTURE_COORDINATES_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace Render
{
namespace Billboard
{



struct TextureCoord
{
	F32 u, v;
};

struct TextureCoords
{
	const TextureCoord & GetTextureCoord(const U32 iIndex) const;

	union
	{
		TextureCoord m_TextureCoord[4];
		struct
		{
			F32 u0, v0;
			F32 u1, v1;
			F32 u2, v2;
			F32 u3, v3;
		};
	};
};



template<int Width, int Height>
class C_TextureCoords
{

public:

	C_TextureCoords();
	~C_TextureCoords();

	const TextureCoords & GetTextureCoords(const U32 iIndex) const;

private:
	
	TextureCoords m_TextureCoords[Width * Height];

};

FORCE_INLINE const TextureCoord & TextureCoords::GetTextureCoord(const U32 iIndex) const
{
	return m_TextureCoord[iIndex];
}



template<int Width, int Height>
C_TextureCoords<Width, Height>::C_TextureCoords()
{
	const F32 fWidth = 1.0f / (F32) Width;
	const F32 fHeight = 1.0f / (F32) Height;

	U32 iIndex = 0;
	for (U32 y = 0; y < Height; ++y)
	{
		for (U32 x = 0; x < Width; ++x)
		{
			const F32 fu = x * fWidth;
			const F32 fv = y * fHeight;

			m_TextureCoords[iIndex].u0 = fu;
			m_TextureCoords[iIndex].v0 = fHeight + fv;
			m_TextureCoords[iIndex].u1 = fWidth + fu;
			m_TextureCoords[iIndex].v1 = fHeight + fv;
			m_TextureCoords[iIndex].u2 = fWidth + fu;
			m_TextureCoords[iIndex].v2 = fv;
			m_TextureCoords[iIndex].u3 = fu;
			m_TextureCoords[iIndex].v3 = fv;

			++iIndex;
		}
	}
}

template<int Width, int Height>
C_TextureCoords<Width, Height>::~C_TextureCoords()
{
}

template<int Width, int Height>
const TextureCoords & C_TextureCoords<Width, Height>::GetTextureCoords(const U32 iIndex) const
{
	assert(iIndex < Width * Height);
	return m_TextureCoords[iIndex];
}



}; // namespace Billboard
}; // namespace Render



#endif // RENDER_BILLBOARD_TEXTURE_COORDINATES_H
