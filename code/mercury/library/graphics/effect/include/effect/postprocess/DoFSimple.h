#ifndef EFFECT_POSTPROCESS_DEPTHOFFIELD_SIMPLE_H
#define EFFECT_POSTPROCESS_DEPTHOFFIELD_SIMPLE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Render/PostProcess/Interface.h>
#include <Render/PostProcess/Manager.h>
#include <Render/Texture/Downsample.h>
#include <Render/Texture/GaussianBlur.h>

namespace PostProcess
{

class C_DoFSimple : public I_PostProcess
{
public:

	C_DoFSimple(C_Manager& manager);
	~C_DoFSimple();

	bool Create();
	bool CreateVolatile();
	void Destroy();
	void DestroyVolatile();
	void Update() {}

	void Display() const;

private:

	Display::C_RenderTexture2D		m_BlurRenderTexture;
	Display::C_RenderTexture2D		m_TempRenderTexture;

	Display::C_PixelShader			m_PixelShader;
	Display::C_PixelShaderParameter	m_SettingsParameter;
	Display::C_PixelShaderParameter	m_DepthParameter;

	Render::Texture::C_Downsample	m_DownsampleUtil;
	Render::Texture::C_GaussianBlur	m_GaussianBlurUtil;

	RenderTextures					m_RenderTextures;

};

}; // namespace PostProcess

#endif
