#ifndef TEST_SECTION_FOG_H
#define TEST_SECTION_FOG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>
#include "Effect/Fog/Fog.h"
#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/HDR/HDR.h"
#include "Render/Mesh/Mesh.h"
#include "Effect/Sky/PreethamSkyDome.h"

namespace Render { namespace Debug { class C_DebugRenderSys; } }
namespace DynamicEffect { class C_Renderer; }
namespace DynamicEffect { namespace Fog { class C_BasicManager; } }
namespace DynamicEffect { namespace Fog { class Volume; } }

class C_TestSectionFog : public Framework::C_TestSection
{
public:

	C_TestSectionFog(const Util::C_String& name);
	~C_TestSectionFog();

	bool OnCreate();
	void OnDestroy();
	void OnUpdate();
	void OnPreRender();
	void OnRender3DViewWithAA(const S32 viewIndex);
	void OnRender3DViewWithoutAA(const S32 viewIndex);

	void OnBeginPostEffects();
	void OnEndPostEffects();
	void OnResize();

private:

	void SetupPostProcess();

private:
	
	PostProcess::C_Manager*				m_pPostProcessMgr;
	PostProcess::C_HighDynamicRange*	m_pPostProcessHdr;
	Effect::Sky::C_PreethamSkyDome		m_SkyDome;
	Effect::Fog::C_Fog					m_Fog;
	Render::Mesh::C_Mesh				m_Mesh;

	DynamicEffect::C_Renderer*			m_pDynamicEffectRenderer;
	DynamicEffect::Fog::C_BasicManager* m_pFogManager;
	DynamicEffect::Fog::Volume*			m_pFogVolume0;
	DynamicEffect::Fog::Volume*			m_pFogVolume1;

	Render::Debug::C_DebugRenderSys*	m_pDebugRenderSys;

	Display::C_Texture2D				m_DepthTexture;

};

#endif // TEST_SECTION_FOG_H
