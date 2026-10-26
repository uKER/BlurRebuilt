#ifndef RENDER_POSTPROCESS_HDR_H
#define RENDER_POSTPROCESS_HDR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/PostProcess/HDR/HDRInterface.h>
#include <Render/PostProcess/Bloom/Bloom.h>
#include <Render/PostProcess/Blur/Gaussian.h>
#include <Render/PostProcess/Misc/SunRays.h>
#include <Render/PostProcess/Scale/GaussianScale.h>

namespace PostProcess
{

struct HighDynamicRangeParameters : public LuminanceParameters, public BloomParameters
{
	F32	fBloomThreshold;
};

enum E_BrightPassShader
{
	E_BrightPassShader_Default,
	E_BrightPassShader_Gamma,
	E_BrightPassShader_Lookup,
	E_BrightPassShader_LookupGamma,
	E_BrightPassShader_ReducedAlphaLookupGamma,

	E_BrightPassShader_Count
};

class C_HighDynamicRange : public I_HighDynamicRange
{
public:

	C_HighDynamicRange( C_Manager& manager );
	virtual ~C_HighDynamicRange();

	bool Create();
	void Destroy();
	bool CreateVolatile();
	void DestroyVolatile();
	
	void Display() const;
	void Update() {};

	const Display::C_Texture2D* CreateBloom() const;

	void SetBloom(I_Bloom* pBloom);
	void SetBloomGamma(const bool bBloomGamma) const;
	void SetBloomCurve(const Display::C_Texture2D* pTexture = 0) const;
	void SetReducedAlphaTexture(const Display::C_Texture2D* pReducedAlphaTexture) { m_pReducedAlphaTexture = pReducedAlphaTexture; }

	I_Bloom*	GetBloom() { return m_pBloom; }

	virtual void SetUserParameters(const MathsStorage::Float4& settings1, const MathsStorage::Float4& settings2) const;
	virtual void UnsetUserParameters() const;

	HighDynamicRangeParameters* GetParameters(const S32 iViewIndex);

protected:

	void UpdateParameters() const;

	Display::C_PixelShader					m_ToneMap;

	Display::C_PixelShader					m_BrightPassShader[E_BrightPassShader_Count];
	Display::C_PixelShaderParameter			m_BrightPassSettings1[E_BrightPassShader_Count];
	Display::C_PixelShaderParameter			m_BrightPassSettings2[E_BrightPassShader_Count];

	Display::C_PixelShader					m_BloomDisplay;
	Display::C_PixelShader					m_BloomDisplayGamma;
	Display::C_PixelShaderParameter			m_BloomDisplayParam;
	Display::C_PixelShaderParameter			m_BloomDisplayGammaParam;

	// bloom lookup
	mutable const Display::C_Texture2D*		m_pBloomCurveTexture;
	const Display::C_Texture2D*				m_pReducedAlphaTexture;

	C_GaussianScale							m_BrightPass;
	
	I_Bloom*								m_pBloom;

	mutable bool							m_bCalculateLuminanceAuto;
	
};

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_HDR_H
