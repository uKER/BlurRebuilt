#ifndef EFFECT_OCEAN_FFTGPU_H
#define EFFECT_OCEAN_FFTGPU_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Render/Texture/TextureUtils.h"

namespace Effect
{
namespace Ocean
{
namespace Waves
{

class C_FFTGPU : public Display::I_Renderer
{
public:

	C_FFTGPU();
	~C_FFTGPU();

	virtual bool	OnCreate();
	virtual bool	OnCreateVolatile();
	virtual void	OnDestroyVolatile();
	virtual void	OnDestroy();

	void			Render(const F32 fTime);

	const Display::C_Texture2D *	GetHeightMap()		{ return &m_HeightMapRenderTexture2D.GetTexture(); }
	const Display::C_Texture2D *	GetGradientMap()	{ return &m_GradientMapRenderTexture2D.GetTexture(); }
	const Display::C_Texture2D *	GetNormalMap()		{ return &m_NormalMapRenderTexture2D.GetTexture(); }

	F32				GetWidth();
	F32				GetHeight();

private:

	void			BuildFrequencies();

private:

	Display::C_PixelShader		m_FFTHorizontalPixelShader;
	Display::C_PixelShader		m_FFTVerticalPixelShader;
	Display::C_PixelShader		m_FrequencyPixelShader;
	Display::C_PixelShader		m_FrequencyDifferentialPixelShader;

	Display::C_RenderTexture2D	m_NormalMapRenderTexture2D;
	Display::C_RenderTexture2D	m_GradientMapRenderTexture2D;
	Display::C_RenderTexture2D	m_HeightMapRenderTexture2D;
	Display::C_Texture2D		m_FrequencyTexture2D;

	S32							m_Width;
	S32							m_Height;

	Render::Texture::C_TextureUtils	m_TextureUtils;

};

}; // namespace Waves
}; // namespace Ocean
}; // namespace Effect

#endif
