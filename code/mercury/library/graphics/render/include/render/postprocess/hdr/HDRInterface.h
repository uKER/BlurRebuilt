#ifndef RENDER_POSTPROCESS_LUMINANCE_H
#define RENDER_POSTPROCESS_LUMINANCE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/PostProcess/Interface.h>

namespace PostProcess
{

static const U32 MAX_LUMINANCE_BUFFERS = 3;

enum E_HighDynamicRangeType
{
	E_HighDynamicRangeType_PostProcess,
	E_HighDynamicRangeType_ShaderU32,
	E_HighDynamicRangeType_Shader
};

struct LuminanceParameters
{
	F32	fMiddleGrey;
	F32	fExposureMin;
	F32	fExposureMax;
	F32	fGamma;
};

class LuminanceResolve;

class I_HighDynamicRange : public I_PostProcess
{
public:

	I_HighDynamicRange(C_Manager& manager);
	virtual ~I_HighDynamicRange();

	bool Create();
	void Destroy();
	bool CreateVolatile();
	void DestroyVolatile();
	
	void Display() const;
	void Update() {};
	
	void ClearLuminance() const;

	const Display::C_Texture2D*		GetAdaptedLuminanceTexture(S32 index) const;
	const Display::C_Texture2D*		GetAdaptedLuminanceTexture() const;
	const Display::C_Texture2D*		GetDepthBufferCentreTexture() const;
		
	F32								GetAdaptedExposure() const;
	void							SetAdaptedExposure(const F32 fAdaptedExposure);
	void							SetAdaptedExposure(const S32 iViewIndex, const F32 fAdaptedExposure);
	void							SetGamma(const F32 fGamma);
	void							SetGamma(const S32 iViewIndex, const F32 fGamma);

	void							SetFastLuminance(const bool bFastLuminance) { m_bFastLuminance = bFastLuminance; }

	E_HighDynamicRangeType			GetType() const;
	MathsStorage::Float4			GetShaderParameters() const;
	MathsStorage::Float4			GetShaderParameters(const S32 iViewIndex) const;

	void							SetHighDynamicRangeType(const E_HighDynamicRangeType eType);
	void							SetCalculateLuminance(const bool bCalculateLuminance);
	void							CalculateLuminance() const;

protected:

	void							ResolveLuminance() const;

	// shader parameters
	Display::C_PixelShaderParameterArray	m_LuminanceIterativeTaps;
	Display::C_PixelShaderParameterArray	m_LuminanceFinalTaps;
	Display::C_PixelShaderParameter			m_LuminanceFinalParam;
	Display::C_PixelShaderParameter			m_LuminanceAdaptedParam;

	Display::C_PixelShaderParameter			m_LuminanceClearParam;
	Display::C_PixelShaderParameter			m_LuminanceWeightDisplayParam;

	Display::C_PixelShaderParameter			m_LuminanceInitialParam;
	Display::C_PixelShaderParameter			m_LuminanceInitialWeightParam;
	
	RenderTextures							m_AdaptedLuminances;

	E_HighDynamicRangeType					m_eType;
	bool									m_bCalculateLuminance;
	bool									m_bFastLuminance;

	mutable Btl::Vector<LuminanceParameters*, Btl::VectorParameters_Static<16> >	m_Parameters;

	Display::C_Texture2D					m_Noise;
	Display::C_Texture2D					m_BlackTexture;
	F32										m_NoiseTextureRcp;
	
	Display::C_RenderTexture2D				m_Luminance64;
	Display::C_RenderTexture2D				m_Luminance16;
	Display::C_RenderTexture2D				m_Luminance4;
	Display::C_RenderTexture2D				m_CurrentLuminance;
	Display::C_RenderTexture2D				m_DepthBufferCentre;
	
	Display::C_PixelShader					m_LuminanceInitial;
	Display::C_PixelShader					m_LuminanceIterative;
	Display::C_PixelShader					m_LuminanceFinal;
	Display::C_PixelShader					m_LuminanceClear;
	Display::C_PixelShader					m_LuminanceDisplay;
	Display::C_PixelShader					m_LuminanceDisplayExp;
	Display::C_PixelShader					m_LuminanceWeightDisplay;
	Display::C_PixelShader					m_LuminanceAdapted;
	Display::C_PixelShader					m_LuminanceResolve;
	Display::C_PixelShader					m_DepthCentreTap;
	
	mutable Btl::Vector<LuminanceResolve*, Btl::VectorParameters_Static<16> >	m_LuminanceResolves;
	mutable Btl::Vector<bool, Btl::VectorParameters_Static<16> >				m_AdaptedInitialised;
	mutable Btl::Vector<F32, Btl::VectorParameters_Static<16> >					m_AdaptedExposures;

};

INLINE E_HighDynamicRangeType I_HighDynamicRange::GetType() const
{
	return m_eType;
}

class LuminanceResolve
{
public:
	LuminanceResolve();

	bool Create();
	void Destroy();

	void Next() const;
	
	bool IsInitialised() const;
	void Reset() const;

	#if !defined(PLATFORM_PC)
	Display::C_RenderTexture2D*	GetRenderTexture();
	#else
	Display::C_Texture2D*		GetTexture();
	#endif

public:
	#if !defined(PLATFORM_PC)
	Display::C_RenderTexture2D	m_RenderTextures[MAX_LUMINANCE_BUFFERS];
	#else
	Display::C_Texture2D		m_Textures[MAX_LUMINANCE_BUFFERS];
	#endif
	mutable U32					m_Index;
	mutable U32					m_Count;
};

INLINE void LuminanceResolve::Next() const
{
	m_Index = (m_Index + 1) % MAX_LUMINANCE_BUFFERS;
	if (m_Count < MAX_LUMINANCE_BUFFERS * 2)
		m_Count++;
}

INLINE void LuminanceResolve::Reset() const
{
	m_Index = 0;
	m_Count = 0;
}

#if !defined(PLATFORM_PC)
INLINE Display::C_RenderTexture2D* LuminanceResolve::GetRenderTexture()
{
	return &m_RenderTextures[m_Index];
}
#else
INLINE Display::C_Texture2D* LuminanceResolve::GetTexture()
{
	return &m_Textures[m_Index];
}
#endif

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_LUMINANCE_H
