#ifndef RENDER_SKY_SKY_DOME_H
#define RENDER_SKY_SKY_DOME_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{
namespace Sky
{

//! E_SkyDomeShader

enum E_SkyDomeShader
{
	E_SkyDomeShader_Default = 0,
	E_SkyDomeShader_HDR,
	E_SkyDomeShader_HDRU32,
	E_SkyDomeShader_Count
};

//! C_SkyDome

class C_SkyDome : public Display::I_Renderer
{
public:

	C_SkyDome();
	virtual ~C_SkyDome();

	const F32 GetRotation() const;
	void SetRotation(const F32 rotation);

	const F32 GetIntensity() const;
	void SetIntensity(const F32 intensity);

	const Maths::C_RGBA_F32& GetFogColour() const;
	void SetFogColour(const Maths::C_RGBA_F32& fogColour);

	bool OnCreate();
	void OnDestroy();

	void Render() const;

protected:

	bool CreateTexture();
	void DestroyTexture();
	void Build();

protected:

	Maths::C_RGBA_F32						m_FogColour;
	F32										m_Rotation;
	F32										m_Intensity;
	U32										m_VertexCount;
	U32										m_IndexCount;
	Util::C_String							m_TextureFileName;
	Display::C_VertexDeclaration*			m_pVertexDeclaration;
	Display::C_VertexBuffer					m_VertexBuffer;
	Display::C_IndexBuffer					m_IndexBuffer;
	Display::C_Texture2D					m_Texture;
	Display::C_VertexShader					m_VertexShader;
	Display::C_VertexShaderParameter		m_vsSkySettings;
	Display::C_VertexShaderParameter		m_vsSkySettings2;
	Display::C_VertexShaderParameter		m_vsFogSettings;
	Display::C_VertexShaderParameter		m_vsWorldMatrix;
	Display::C_VertexShaderParameter		m_vsViewProjMatrix;
	Display::C_PixelShader					m_PixelShader[E_SkyDomeShader_Count];
	Display::C_PixelShaderParameter			m_psSkySettings[E_SkyDomeShader_Count];
	Display::C_PixelShaderParameter			m_psFogColour[E_SkyDomeShader_Count];
	Display::C_PixelShaderParameter			m_psLuminance[E_SkyDomeShader_Count];
};

extern C_SkyDome* g_pSkyDome;

void SkyDome(C_SkyDome* pSkyDome);

INLINE C_SkyDome* SkyDome()
{
	assert(g_pSkyDome);
	return g_pSkyDome;
}

INLINE const bool SkyDomeExists()
{
	return g_pSkyDome ? true : false;
}

} // namespace Sky
} // namespace Render

#include "Render/Sky/SkyDome.inl"

#endif // RENDER_SKY_SKY_DOME_H
