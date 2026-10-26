#ifndef MODEL_RENDERER_H
#define MODEL_RENDERER_H
#ifdef _MSC_VER
#pragma once
#endif

#include "gamepad.h"
#include "btl/utility/scopedpointer.h"
#include "display/render/basicrender.h"
#include "renderingdata/renderingdata.h"
#include "renderingdata/scene.h"
#include "renderingdata/target.h"
#include "framework/view/view.h"
#include "world_render/pvs.h"

namespace SceneGraph
{
	class C_SceneGraph;
	class I_VisibleNodeListSorter;
	class C_View;
	class C_StaticScene;
}

class ModelRendererApp
{
public:
	ModelRendererApp() {}
	~ModelRendererApp() {}

	bool Create();
	void Destroy();
	void Update(const Framework::C_View& view);
	void Render();

	//Display::C_Device* Device() {return Pointer(m_pDevice);}

private:
	//void InitialiseCamera();
	//void UpdateCamera(float t);
	void DisplayDebugTriangle();
	void ImportModel();

	//Btl::ScopedPointer<Display::C_Manager>	m_pDisplayManager;
	//Btl::ScopedPointer<Display::C_Device> m_pDevice;
	//Btl::ScopedPointer<Display::C_BasicRender> m_pBasicRender;
	//Display::C_View m_gameView;

	//Btl::ScopedPointer<Cam2::C_DebugFeed> m_pDebugFeed;

	RenderingData::Target::Scene m_scene;
	Pvs::Target::PvsData m_pvsData;

	// scene graph
	SceneGraph::C_SceneGraph* m_pSceneGraph;
	SceneGraph::C_View*	m_pView;
	SceneGraph::I_VisibleNodeListSorter* m_pSorter;
	SceneGraph::I_VisibleNodeListSorter* m_pSorter2;
	SceneGraph::C_StaticScene*	m_pStaticScene;
};

#endif
