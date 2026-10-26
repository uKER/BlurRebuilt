#ifndef REFLECTION_VIEW_H
#define REFLECTION_VIEW_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/Render/TextureRender.h>

namespace GameView
{

class C_ReflectionView
{
public:

	C_ReflectionView();
	~C_ReflectionView();

	Render::C_TextureRender*	GetTextureRender();

	void						SetFormat(const U32 width, const U32 height, const Display::E_Multisample multiSample, const U32 numMips = 1);

	void						Create();
	void						Destroy();

	void						Bind();
	void						Unbind();

	void						DebugRender();

	void							SetupView(const Display::C_ViewSetup& viewSetup, const Maths::C_Plane reflectionPlane, const F32 fFovExpand = 0.f);
	const Display::C_Matrix&		GetReflectedViewMatrix();
	const Display::C_View&			GetView() const { return m_View; }
	const Maths::C_Plane&			GetReflectionPlane() const { return m_ReflectionPlane; }
	const Display::E_Multisample	GetMultisample() const { return m_Multisample; }

private:
	
	Render::C_TextureRender*		m_pRenderTexture;
	
	U32								m_Width;
	U32								m_Height;
	Display::E_Multisample			m_Multisample;
	U32								m_NumMips;
	Display::C_View					m_View;
	Display::C_ViewSetup			m_ViewSetup;
	Maths::C_Plane					m_ReflectionPlane;
};

}

#endif // OFFSCREEN_VIEW_H
