#ifndef RENDER_DEPTHRENDER_H
#define RENDER_DEPTHRENDER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{

struct C_DepthRenderShaders
{
	Display::C_VertexShader	m_VertexShader;
	Display::C_PixelShader	m_PixelShader;
};

class C_DepthRender : public Display::I_Renderer
{
public:

	C_DepthRender();
	~C_DepthRender();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

	void Bind();
	void Unbind();

	void LookAt(const Maths::HVec3& pos, const Maths::HVec3& dir);

public:

	U32								m_Width;
	U32								m_Height;
	Display::C_RenderTexture2D		m_RenderTexture;
	Display::C_Surface				m_DepthStencilSurface;
	Display::C_View					m_View;
	Display::C_ViewSetup			m_ViewSetup;
	Display::C_Viewport				m_OldViewport;
	Display::C_Viewport				m_NewViewport;

private:

	static C_DepthRenderShaders		m_Shaders;
};

} // namespace Render

#endif // RENDER_DEPTHRENDER_H
