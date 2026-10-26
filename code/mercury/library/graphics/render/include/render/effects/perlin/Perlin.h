#ifndef RENDER_PERLIN_H
#define RENDER_PERLIN_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/Texture/TextureUtils.h>

namespace Perlin
{



struct C_SettingsPS
{
	MathsStorage::Float4		m_Persistence;
	MathsStorage::Float4		m_Time;
	MathsStorage::Float4		m_ChannelScale[4];
};

#define PERLIN_SETTINGS_REGISTER_COUNT	6

enum E_PixelShader
{
	E_PixelShader_Default,
	E_PixelShader_2Samples,
	E_PixelShader_4Samples,

	E_PixelShader_Count
};

class C_Perlin : public Display::I_Renderer
{
public:

	C_Perlin();
	~C_Perlin();

	bool	OnCreate();
	void	OnDestroy();

	void	Update(const F32 time);
	void	Render();
	void	Show() const;

	void	SetPixelShader(const E_PixelShader pixelShaderType) { m_PixelShaderType = pixelShaderType; }
	void	SetBuildMipMaps(const bool buildMipMaps) { m_BuildMipMaps = buildMipMaps; }
#ifndef GRAPHICS_GL
	const Display::C_Texture2D *	GetTexture()	{ return &m_RenderTexture.GetTexture(); }
#else
	const Display::C_Texture2D *	GetTexture()	{ return NULL; }
#endif

private:

	void	FillTexture3D(const Display::C_Texture3D &texture3d);

#ifndef GRAPHICS_GL
	Display::C_RenderTexture2D		m_RenderTexture;
#endif
	Display::C_VertexDeclaration *	m_pVertexDeclaration;
	Display::C_VertexShader			m_VertexShader;
	Display::C_PixelShader			m_PixelShader[E_PixelShader_Count];
	E_PixelShader					m_PixelShaderType;

	Display::C_Texture3D			m_Texture3D;

	Display::C_ViewSetup			m_ViewSetup;
	Display::C_View					m_View;

	Render::Texture::C_TextureUtils	m_TextureUtils;
	bool							m_BuildMipMaps;

	C_SettingsPS					m_Settings;

};



}; // namespace Perlin

#endif // RENDER_PERLIN_H
