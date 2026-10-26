#ifndef EFFECT_POSTPROCESS_DOFREDUCED_H
#define EFFECT_POSTPROCESS_DOFREDUCED_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Render/PostProcess/Interface.h>
#include <Render/PostProcess/Manager.h>
#include <Effect/PostProcess/DoFBase.h>

namespace PostProcess
{

class C_DoFReduced : public I_PostProcess, public C_DoFBase
{
public:

	C_DoFReduced(C_Manager& manager);
	~C_DoFReduced();

	bool Create();
	void Destroy();
	bool CreateVolatile();
	void DestroyVolatile();
	void Update() {}

	void SetComposite(const bool bComposite) { m_bComposite = bComposite; }

	void SetTextureBuffer(U8* pTextureBuffer, const U32 iTextureBufferSize);
	U32 GetTextureBufferRequiredSize() const;

	void Enable() { m_bEnabled = true; }
	void Disable() { m_bEnabled = false; }

	void Display() const;
	
	Display::C_Texture2D* GetTexture();

private:

	mutable bool							m_bEnabled;
	mutable bool							m_bComposite;

	Display::C_Texture2D					m_BlackTexture;

	RenderTextures							m_DownsampleRenderTextures;

	U8*										m_pTextureBuffer;
	U32										m_iTextureBufferSize;

	Display::C_PixelShader					m_BlurShader;
	Display::C_PixelShader					m_CompositeShader;
	Display::C_PixelShader					m_DownsampleShader;

	Display::C_PixelShaderParameterArray	m_BlurDistancesParameter;
	Display::C_PixelShaderParameterArray	m_BlurWeightsParameter;
	Display::C_PixelShaderParameterArray	m_BlurTapsParameter;
	Display::C_PixelShaderParameter			m_BlurDepthParameter;
	Display::C_PixelShaderParameter			m_DownsampleCameraParameter;
	Display::C_PixelShaderParameter			m_DownsamplePixelSizeParameter;
	Display::C_PixelShaderParameter			m_DownsampleDepthParameter;

};

}; // namespace PostProcess

#endif // EFFECT_POSTPROCESS_DOFREDUCED_H
