#ifndef TEST_SECTION_SHADOW_MAP_H
#define TEST_SECTION_SHADOW_MAP_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "TestSections/TestSectionHDR.h"

#include <Render/Sky/SkyDome.h>
#include <SceneGraph/RenderNode.h>
#include <SceneGraph/DynamicCullNode.h>

#include <Render/Texture/RenderUtils.h>
#include <Effect/Shadow/ShadowMap.h>
#include <Effect/Shadow/ExponentialShadowMap.h>
#include <Effect/Shadow/VarianceShadowMap.h>

namespace Render
{ 
namespace Debug
{
	class C_DebugRenderSys;
}
}

namespace SceneGraph
{
	class C_SceneGraph;
	class I_VisibleNodeListSorter;
	class C_View;
}

namespace RenderingData
{ 
namespace Target
{
	class Scene;
}
}

class C_TestSectionShadowMap : public Framework::C_TestSectionHDR
{
public:

	C_TestSectionShadowMap(const Util::C_String& name);
	~C_TestSectionShadowMap();

	void OnUpdate();
	void OnPreRender();
	void OnRenderHdr3DViewWithAA(const S32 viewIndex);

protected:

	void LoadScene();

	void RenderShadowPass();
	void RenderWorldPass();

protected:

	SceneGraph::C_SceneGraph*				m_pSceneGraph;
	SceneGraph::I_VisibleNodeListSorter*	m_pSorter;
	SceneGraph::C_View*						m_pEyeView;
	SceneGraph::C_View*						m_pShadowView;
	RenderingData::Target::Scene*			m_pScene;

	Render::Debug::C_DebugRenderSys*		m_pDebugRenderSys;
	Render::Texture::C_RenderUtils			m_TextureRenderUtils;
	Render::Sky::C_SkyDome					m_SkyDome;

	Effect::C_ShadowMap						m_ShadowMap;
	Effect::C_ExponentialShadowMap			m_ExponentialShadowMap;
	Effect::C_VarianceShadowMap				m_VarianceShadowMap;

	Display::C_View							m_ShadowEyeView;

	Display::C_TextureCube					m_TextureCube;

};

#endif // TEST_SECTION_SHADOW_MAP_H
