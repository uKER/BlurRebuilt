#ifndef AMAX_GRAPHICS_LIGHTING_SPOTLIGHTS_H
#define AMAX_GRAPHICS_LIGHTING_SPOTLIGHTS_H

#if _MSC_VER > 1000
#pragma once
#endif

enum E_SpotlightShader
{
	E_SpotlightShader_Default,
	E_SpotlightShader_Projected,
	E_SpotlightShader_Volume,
	E_SpotlightShader_Count
};

class C_SpotLights : public Display::I_Renderer, public Ut::Singleton<C_SpotLights, Ut::CreateExternal>
{
public:

	C_SpotLights();
	~C_SpotLights();

	bool OnCreate();
	void OnDestroy();

	void RenderSpotlight(const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfView, const Display::C_Float3& vColour) const;
	void RenderSpotlightSimple(const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfView, const Display::C_Float3& vColour) const;
	void RenderSpotlightProjected(const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfViewX, const F32 fFieldOfViewY, const Display::C_Float3& vColour, const Display::C_Texture2D* pTexture2D) const;
	void RenderSpotlightProjectedSimple(const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfViewX, const F32 fFieldOfViewY, const Display::C_Float3& vColour, const Display::C_Texture2D* pTexture2D) const;
	void RenderSpotlightVolume(const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfViewX, const F32 fFieldOfViewY, const Display::C_Float3& vColour, const Display::C_Texture3D* pTexture3D) const;
	void RenderSpotlightVolumeSimple(const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfViewX, const F32 fFieldOfViewY, const Display::C_Float3& vColour, const Display::C_Texture3D* pTexture3D) const;

	void SetNormalDepthTexture(const Display::C_Texture2D* pNormalDepthTexture)		{ m_pNormalDepthTexture = pNormalDepthTexture; }

	void SetFarClip(const F32 fFarClip)												{ m_FarClip = fFarClip; }

private:

	void BindFrontFaces(const E_SpotlightShader eShader, const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfViewX, const F32 fFieldOfViewY, const Display::C_Float3& vColour) const;
	void BindBackFaces(const E_SpotlightShader eShader, const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfViewX, const F32 fFieldOfViewY, const Display::C_Float3& vColour) const;
	void Bind(const E_SpotlightShader eShader, const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfViewX, const F32 fFieldOfViewY, const Display::C_Float3& vColour, const bool bBackFaces = false) const;
	void Unbind() const;

	void BindStencilTest() const;
	void UnbindStencilTest() const;

	void BindDepthBounds(const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfViewX, const F32 fFieldOfViewY) const;
	void UnbindDepthBounds() const;

	Maths::Mat44 CreateBoxProjectionMatrix(const F32 fFieldOfViewX, const F32 fFieldOfViewY, const F32 fFarClip) const;
	Maths::Mat44 CreateConeProjectionMatrix(const F32 fFieldOfViewX, const F32 fFieldOfViewY, const F32 fFarClip) const;

	bool IsCameraInside(const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfViewX, const F32 fFieldOfViewY) const;

	void RenderCone(const F32 fLength, const F32 fFieldOfViewX, const F32 fFieldOfViewY) const;
	void RenderBox(const F32 fWidth, const F32 fHeight, const F32 fLength) const;

	void RenderStencilCone(const Display::C_Matrix& mLight, const F32 fLength, const F32 fFieldOfViewX, const F32 fFieldOfViewY) const;
	void RenderStencilBox(const Display::C_Matrix& mLight, const F32 fWidth, const F32 fHeight, const F32 fLength) const;

private:

	const Display::C_Texture2D*	m_pNormalDepthTexture;

	Display::C_VertexShader		m_SpotlightVertexShader;
	Display::C_VertexShader		m_StencilVertexShader;
	Display::C_PixelShader		m_SpotlightPixelShader[E_SpotlightShader_Count];
	Display::C_PixelShader		m_StencilPixelShader;

	Display::C_PixelShaderParameterArray	m_SpotlightLightParameter[E_SpotlightShader_Count];
	Display::C_PixelShaderParameter			m_SpotlightPositionParameter[E_SpotlightShader_Count];
	Display::C_PixelShaderParameter			m_SpotlightDirectionParameter[E_SpotlightShader_Count];
	Display::C_PixelShaderParameter			m_SpotlightColourParameter[E_SpotlightShader_Count];
	Display::C_PixelShaderParameter			m_SpotlightFalloffParameter[E_SpotlightShader_Count];

	F32							m_FarClip;

};

#endif // AMAX_GRAPHICS_LIGHTING_SPOTLIGHTS_H
