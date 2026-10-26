#ifndef EFFECT_RAIN_DROP_H
#define EFFECT_RAIN_DROP_H

#ifdef _MSC_VER
#pragma once
#endif

#include "misc_shared/Types/FP32.h"
#include "System/Misc/Random.h"
#include "Parameters.h"
#include <core\utility\Singleton.h>

namespace Render
{
namespace Chart
{
	class I_BarChart;
	class C_LineChart;
	class C_StackedBarChart;
	class C_StackedBarChartHistory;
};
};

namespace Effect
{
namespace Rain
{

class C_DropRenderer;

class C_OcclusionBox
{
public:
	C_OcclusionBox() : m_pTexture(NULL), m_bFlipV(true) {}
	~C_OcclusionBox() {}

	void							SetBox(const Maths::C_AxisBox& axisBox, const Maths::HMat43& transform);
	void							SetTexture(const Display::C_Texture2D* pTexture);
	void							SetFlipV(const bool bFlipV);
		
	const Maths::C_AxisBox&			GetAxisBox() const;
	const Maths::HMat43&			GetTransform() const;
	const Maths::C_Sphere&			GetBoundingSphere() const;
	const Display::C_Texture2D*		GetTexture() const;
	const Display::C_Matrix*		GetMatrix() const;
	const Maths::HF32&				GetMinDistance() const;

	Maths::HVec3					GetUVs(const Maths::HVec3& position) const;

	friend class					C_DropRenderer;

private:
	void							CalculateMatrix();

	Display::C_Matrix				m_mMatrix;

	Maths::HF32						m_fMinDistance;
	Maths::HMat43					m_mTransform;
	Maths::C_AxisBox				m_AxisBox;
	Maths::C_Sphere					m_BoundingSphere;
	const Display::C_Texture2D*	m_pTexture;
	bool							m_bFlipV;
};

INLINE void C_OcclusionBox::SetFlipV(const bool bFlipV)
{
	m_bFlipV = bFlipV;
}

INLINE void C_OcclusionBox::SetTexture(const Display::C_Texture2D* pTexture)
{
	m_pTexture = pTexture;
}

INLINE const Display::C_Texture2D* C_OcclusionBox::GetTexture() const
{
	return m_pTexture;
}

INLINE const Display::C_Matrix* C_OcclusionBox::GetMatrix() const
{
	return &m_mMatrix;
}

INLINE const Maths::HMat43& C_OcclusionBox::GetTransform() const
{
	return m_mTransform;
}

INLINE const Maths::C_AxisBox& C_OcclusionBox::GetAxisBox() const
{
	return m_AxisBox;
}

INLINE const Maths::C_Sphere& C_OcclusionBox::GetBoundingSphere() const
{
	return m_BoundingSphere;
}

INLINE const Maths::HF32& C_OcclusionBox::GetMinDistance() const
{
	return m_fMinDistance;
}

typedef Ut::Vector<C_OcclusionBox>	OcclusionBoxes;


const S32 g_nPasses = 20;

class C_DropRenderer : public Display::I_Renderer, public Ut::Singleton<C_DropRenderer, Ut::CreateExternal>
{
public:

	C_DropRenderer();
	~C_DropRenderer();

	bool OnCreate();
	void OnDestroy();

	void Update(const F32 fTime, const bool bPaused);

	void					AddOcclusionBox(const Maths::C_AxisBox& axisBox, const Maths::HMat43& transform, const Display::C_Texture2D* pTexture = NULL, const bool bFlipV = true);
	const C_OcclusionBox&	GetOcclusionBox(const U32 iIndex) const;
	C_OcclusionBox&			GetOcclusionBox(const U32 iIndex);
	S32						GetOcclusionBoxCount() const { return m_OcclusionBoxes.Size(); }
	void					ClearOcclusionBoxes();

	void					SetNumSplits(U32 nSplits) { m_nSplits = nSplits; }	
	void					SetLighting(const Maths::C_RGBA_F32& vLighting) { Maths::Store(vLighting, &m_vLighting); }
	void					SetLighting(const MathsStorage::Float4& vLighting) { m_vLighting = vLighting; }

	// Call this before calling initialise
	void					OverrideParticleCount(const float countScale) { m_fParticleCountScale= countScale; }

	void					SetParameters(const DropParameters& parameters);
	const DropParameters&	GetParameters() const { return m_Parameters; }

	void					Display() const;
	const bool				Active() const { return m_nPasses > 0; }

	const Maths::HVec3&		GetNoise() const { return m_vNoise; }

private:
	bool OnInitialise();
	void OnShutdown();

	void SetDefaults();

	bool SetConstants() const;
	void SetPassConstants(U32 i, F32 fAlpha) const;

	Display::C_VertexDeclaration*	m_pVertexDeclaration;
	Display::C_VertexBuffer			m_VertexBuffer;
	Display::C_IndexBuffer			m_IndexBuffer;
	Display::C_Texture2D			m_WhiteTexture;
	Display::C_Texture2D*			m_pTexture;
	Display::C_VertexShader			m_VertexShader;
	Display::C_VertexShader			m_VertexShaderOcclusion;
	Display::C_VertexShader			m_VertexShaderSimple;
	Display::C_VertexShader			m_VertexShaderSimpleOcclusion;
	
	Display::C_PixelShader			m_PixelShader;
	Display::C_PixelShader			m_PixelShaderOcclusion;
	Display::C_PixelShader			m_PixelShaderOcclusion2;

	Display::E_PrimitiveType			m_ePrimitiveType;
	U32									m_iVertexCount;
	U32									m_iIndexCount;

	MathsStorage::Float4				m_vLighting;

	DropParameters						m_Parameters;

	OcclusionBoxes						m_OcclusionBoxes;

	bool								m_bSimple;
	S32									m_nSplits;
	S32									m_nSplitBoxes;
	F32									m_fBoxSize;
	U32									m_nSplitParticles;
	U32									m_nParticles;


	F32									m_fTime;
	
	U32									m_nPasses;
	F32									m_fSpeedScale[g_nPasses];
	F32									m_fSpeedColourScale;
	Maths::HVec3						m_vWind[g_nPasses];
	Maths::HVec3						m_vOffset[g_nPasses];
	Maths::HVec3						m_vRandom[g_nPasses];

	Maths::HVec3						m_vNoise;

	FP32								m_iNoiseX;
	FP32								m_iNoiseY;
	FP32								m_iNoiseZ;

	mutable F32							m_fAlphaFade;

	Util::C_Random						m_RainRandom;
	F32									m_fParticleCountScale;
};

INLINE const C_OcclusionBox& C_DropRenderer::GetOcclusionBox(const U32 iIndex) const
{
	return m_OcclusionBoxes[iIndex];
}

INLINE C_OcclusionBox& C_DropRenderer::GetOcclusionBox(const U32 iIndex)
{
	return m_OcclusionBoxes[iIndex];
}

INLINE void C_DropRenderer::SetParameters(const DropParameters& parameters)
{
#ifndef CFG_SPEED
	bool bRecreateAssets = false;

	if(m_Parameters.texture != parameters.texture || m_Parameters.iAtlasSize != parameters.iAtlasSize)
		bRecreateAssets = true;
#endif

	m_Parameters = parameters;

	// clamp density to a sensible range
	m_Parameters.fDensity = Maths::Clamped(m_Parameters.fDensity, 0.0f, 20.0f);
	m_nPasses = (U32)(m_Parameters.fDensity + 0.999f);

#ifndef CFG_SPEED
	if(bRecreateAssets)
		Reinitialise();
#endif
}

} // namespace Rain
} // namespace Effect

#endif // EFFECT_RAIN_DROP_H
