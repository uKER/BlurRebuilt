#ifndef OFFSCREEN_VIEW_H
#define OFFSCREEN_VIEW_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/Render/TextureRender.h>

namespace GameView
{

class C_OffscreenView
{
public:

	C_OffscreenView();
	~C_OffscreenView();

	const Render::C_TextureRender*	GetTextureRender() const;

	void SetFormat(const U32 width, const U32 height, const Display::E_Multisample multiSample);
	void SetupView(const F32 aspectRatio, const F32 fov, const F32 farPlane);

	void Create();
	void Destroy();

	void SetupView();
	void Bind();
	void Unbind();

	void DebugRender();

	void SetCamera(const Maths::HVec3 position, const Maths::HVec3 view, const Maths::HVec3 up);

	const Display::C_View&			GetView() const { return m_View; }
	const Maths::HVec3&				GetPosition() const { return m_Position; }
	const Display::E_Multisample	GetMultisample() const { return m_Multisample; }
private:
	void SetCameraToWorld(const Maths::HVec3& pos, const Maths::HVec3& dir, const Maths::HVec3& up);

private:
	
	Render::C_TextureRender*		m_pRenderTexture;

	Maths::HVec3					m_Position;
	Maths::HVec3					m_CameraView;
	Maths::HVec3					m_CameraUp;
	U32								m_Width;
	U32								m_Height;
	Display::E_Multisample			m_Multisample;
	Display::C_View					m_View;
	Display::C_ViewSetup			m_ViewSetup;
};

}

#endif // OFFSCREEN_VIEW_H
