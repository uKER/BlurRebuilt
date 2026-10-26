#ifndef EDITOR_AFTK_SCENE_RENDERER_H
#define EDITOR_AFTK_SCENE_RENDERER_H

#ifdef _MSV_VER
#pragma once
#endif

namespace Editor
{
	class C_AftkSceneRenderer : public AftkRender::C_SceneRenderer
	{
	public:
		C_AftkSceneRenderer();
		~C_AftkSceneRenderer();

		void Update(const Maths::C_TimeS& timeStep, AftkRender::E_RendererPass pass=AftkRender::E_RendererPass_Normal);
		void AddRenderContext(AftkRender::C_RenderContext& rc, const AftkRender::C_SceneGraphNode* pNode);

		void Render();
		void OnSelectionChanged();

		void ShowWireframeOverlay(bool state);

	private:
		Ut::UtSet<const AftkRender::C_SceneGraphNode*>	m_Selection;

		//supplementary struct for wireframe overlays
		Ut::UtVector<Ut::DynamicVector<
			Ut::Pair<const AftkRender::C_SceneGraphNode*, AftkRender::C_RenderContext> 
		> >	m_OverlayRenderContextArray;

		bool	m_showWireframeOverlay;

	};
}

#endif