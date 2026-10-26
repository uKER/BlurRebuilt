#ifndef TEST_SECTIONS_TEST_SECTION_SCENEGRAPH_H
#define TEST_SECTIONS_TEST_SECTION_SCENEGRAPH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "TestSections/TestSectionHDR.h"

#include <Render/Sky/SkyDome.h>
#include <SceneGraph/RenderNode.h>
#include <SceneGraph/DynamicCullNode.h>

namespace DynamicEffect
{
	class C_Renderer;
}

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
	class C_StaticScene;
}

namespace RenderingData
{ 
namespace Target
{
	class Scene;
}
}

class C_SGSkyDome : public Render::Sky::C_SkyDome, public SceneGraph::I_RenderNode, public SceneGraph::I_GatherNode, public SceneGraph::C_PassCull
{
public:
	C_SGSkyDome();
	~C_SGSkyDome() {}

	void OnRender(SceneGraph::C_RenderState &renderState) const;
	void OnGather(const SceneGraph::C_View &view) const;
	void DebugRender() const {}
};

class C_SGDynamicObject : public SceneGraph::C_DynamicCullRenderNode
{
public:
	C_SGDynamicObject() : C_DynamicCullRenderNode(100) {}
	~C_SGDynamicObject() {}

	void	SetPositionSize(const Maths::HVec3& position, const Maths::HVec3& size);
	void	OnRender(SceneGraph::C_RenderState& renderState) const;
	void	OnGather(const SceneGraph::C_View& view) const;
	void	DebugRender() const {}
private:

	Maths::HVec3		m_Position;
	Maths::HVec3		m_Size;
};


namespace Framework
{

class C_TestSectionSceneGraph : public C_TestSectionHDR
{
public:

	C_TestSectionSceneGraph(const Util::C_String& name);
	~C_TestSectionSceneGraph();

	void OnPreRender();
	void OnUpdate();
	void OnRenderHdr3DViewWithAA(const S32 viewIndex);

protected:

	void LoadScene();

	// objects to go in scene graph
	C_SGSkyDome*							m_pSkyDome;
	Btl::Vector<C_SGDynamicObject*>			m_DynamicObects;


	SceneGraph::C_SceneGraph*				m_pSceneGraph;
	SceneGraph::I_VisibleNodeListSorter*	m_pSorter;
	SceneGraph::C_View*						m_pView;
	SceneGraph::C_StaticScene*				m_pStaticScene;
	RenderingData::Target::Scene*			m_pScene;

	DynamicEffect::C_Renderer*				m_pDeRenderer;

	Render::Debug::C_DebugRenderSys*		m_pDebugRenderSys;

};

}; // namespace Framework

#endif // TEST_SECTIONS_TEST_SECTION_SCENEGRAPH_H
