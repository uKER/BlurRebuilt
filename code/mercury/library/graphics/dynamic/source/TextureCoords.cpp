#include "stdafx.h"
#include "Dynamic/TextureCoords.h"

namespace DynamicEffect
{

const MathsStorage::Int2 g_AtlasDimensions[E_Atlas_Count] = {	MathsStorage::Int2(1, 1),
																MathsStorage::Int2(2, 2),
																MathsStorage::Int2(4, 4),
																MathsStorage::Int2(8, 8),
																MathsStorage::Int2(2, 1),
																MathsStorage::Int2(4, 1),
																MathsStorage::Int2(4, 2),
																MathsStorage::Int2(8, 2),
																MathsStorage::Int2(8, 4)
															};


C_TextureCoords::C_TextureCoords()
{
	U32 iIndex = 0;

	// mapping for no standard UV
	m_Mapping[0].m0 = 0;
	m_Mapping[0].m1 = 1;
	m_Mapping[0].m2 = 2;
	m_Mapping[0].m3 = 3;

	// mapping for flipped U
	m_Mapping[1].m0 = 1;
	m_Mapping[1].m1 = 0;
	m_Mapping[1].m2 = 3;
	m_Mapping[1].m3 = 2;

	// mapping for flipped V
	m_Mapping[2].m0 = 3;
	m_Mapping[2].m1 = 2;
	m_Mapping[2].m2 = 1;
	m_Mapping[2].m3 = 0;

	// mapping for flipped UV
	m_Mapping[3].m0 = 2;
	m_Mapping[3].m1 = 3;
	m_Mapping[3].m2 = 0;
	m_Mapping[3].m3 = 1;

	for(S32 i = 0; i < E_Atlas_Count; ++i)
	{
		S32 iWidth = g_AtlasDimensions[i].x;
		S32 iHeight = g_AtlasDimensions[i].y;

		m_Offset[i] = iIndex;
		m_Size[i] = iWidth * iHeight;

		const F32 fWidth = 1.0f / (F32) iWidth;
		const F32 fHeight = 1.0f / (F32) iHeight;

		for(S32 y = 0; y < iHeight; ++y)
		{
			for(S32 x = 0; x < iWidth; ++x)
			{
				const F32 fu = x * fWidth;
				const F32 fv = y * fHeight;

				m_TextureCoordsF32[iIndex].u[0] = fu;
				m_TextureCoordsF32[iIndex].v[0] = fv;
				m_TextureCoordsF32[iIndex].u[1] = fWidth + fu;
				m_TextureCoordsF32[iIndex].v[1] = fv;
				m_TextureCoordsF32[iIndex].u[2] = fWidth + fu;
				m_TextureCoordsF32[iIndex].v[2] = fHeight + fv;
				m_TextureCoordsF32[iIndex].u[3] = fu;
				m_TextureCoordsF32[iIndex].v[3] = fHeight + fv;
				
				m_TextureCoordsU16[iIndex].u[0] = (U16)(m_TextureCoordsF32[iIndex].u[0] * 65535.99f);
				m_TextureCoordsU16[iIndex].v[0] = (U16)(m_TextureCoordsF32[iIndex].v[0] * 65535.99f);
				m_TextureCoordsU16[iIndex].u[1] = (U16)(m_TextureCoordsF32[iIndex].u[1] * 65535.99f);
				m_TextureCoordsU16[iIndex].v[1] = (U16)(m_TextureCoordsF32[iIndex].v[1] * 65535.99f);
				m_TextureCoordsU16[iIndex].u[2] = (U16)(m_TextureCoordsF32[iIndex].u[2] * 65535.99f);
				m_TextureCoordsU16[iIndex].v[2] = (U16)(m_TextureCoordsF32[iIndex].v[2] * 65535.99f);
				m_TextureCoordsU16[iIndex].u[3] = (U16)(m_TextureCoordsF32[iIndex].u[3] * 65535.99f);
				m_TextureCoordsU16[iIndex].v[3] = (U16)(m_TextureCoordsF32[iIndex].v[3] * 65535.99f);

				m_TextureCoordsS16[iIndex].u[0] = (S16)(m_TextureCoordsF32[iIndex].u[0] * 32767.99f);
				m_TextureCoordsS16[iIndex].v[0] = (S16)(m_TextureCoordsF32[iIndex].v[0] * 32767.99f);
				m_TextureCoordsS16[iIndex].u[1] = (S16)(m_TextureCoordsF32[iIndex].u[1] * 32767.99f);
				m_TextureCoordsS16[iIndex].v[1] = (S16)(m_TextureCoordsF32[iIndex].v[1] * 32767.99f);
				m_TextureCoordsS16[iIndex].u[2] = (S16)(m_TextureCoordsF32[iIndex].u[2] * 32767.99f);
				m_TextureCoordsS16[iIndex].v[2] = (S16)(m_TextureCoordsF32[iIndex].v[2] * 32767.99f);
				m_TextureCoordsS16[iIndex].u[3] = (S16)(m_TextureCoordsF32[iIndex].u[3] * 32767.99f);
				m_TextureCoordsS16[iIndex].v[3] = (S16)(m_TextureCoordsF32[iIndex].v[3] * 32767.99f);
		
				++iIndex;
			}
		}
	}
}

U32 C_TextureCoords::GetTextureCoordinateIndex(E_Atlas atlas) const
{
	return m_Offset[atlas] + m_Random.Rand(m_Size[atlas]);
}

U32 C_TextureCoords::GetAtlasSize(E_Atlas atlas) const
{
	return m_Size[atlas];
}

U32 C_TextureCoords::GetAtlasOffset(E_Atlas atlas) const
{
	return m_Offset[atlas];
}


}
