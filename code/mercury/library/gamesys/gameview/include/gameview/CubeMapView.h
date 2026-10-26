#ifndef CUBE_MAP_VIEW_H
#define CUBE_MAP_VIEW_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/Render/CubeTextureRender.h>
#include <Render/Render/CubeTextureSaver.h>

namespace GameView
{

class I_CubeMapRenderHelper;

class C_CubeMapView
{
public:

	C_CubeMapView();
	~C_CubeMapView();

	void SetFormat(const U32 width, const Display::E_Multisample multiSample);
	void SetFarClip(const F32 farPlane);

	const Display::C_TextureCube* GetRenderTexture() const;
	const Display::E_Multisample GetMultisample() const { return  m_Multisample; }

	void Create();
	void Destroy();

	void Gather();
	virtual void GatherFace(const U32 face) {}

	void Schedule();
	virtual void ScheduleFace(const U32 face) {}

	void Render(bool save, const Util::C_String& fileName);
	virtual void RenderFace(const U32 face) {}

	void Bind();
	void Unbind();

	void SetupView(const Display::E_CubeTextureFace face);
	void BindFace(const Display::E_CubeTextureFace face);
	void UnbindFace(const Display::E_CubeTextureFace face);

	void DebugRender();

	void SetPosition(const Maths::HVec3 position);

	void Save(const Util::C_String& saveTextureFileName);

protected:
	void SetCameraToWorld(const Maths::HVec3& pos, const Maths::HVec3& dir, const Maths::HVec3& up);

	Render::I_CubeTexture*			m_pCurrentTexture;
	Render::C_CubeTextureRender*	m_pRenderTexture;
	Render::C_CubeTextureSaver*		m_pSaverTexture;

	Maths::HVec3					m_Position;
	U32								m_Width;
	Display::E_Multisample			m_Multisample;
	Display::C_View					m_View[6];
	Display::C_ViewSetup			m_ViewSetup;
	bool							m_SaveTexture;
	Util::C_String					m_SaveTextureFileName;
};

}

#endif // CUBE_MAP_VIEW_H
