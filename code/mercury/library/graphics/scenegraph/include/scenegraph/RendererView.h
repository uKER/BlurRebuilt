#ifndef SCENE_GRAPH_RENDERER_VIEW_H
#define SCENE_GRAPH_RENDERER_VIEW_H

#ifdef _MSC_VER
#pragma once
#endif

namespace SceneGraph
{

class I_Renderer;
class C_View;

// renderer specific views can derive off this then store additional unique data per view into the scene graph
// eg dynamic effect renderer needs to store vertex build constants, scheduler groups etc per view which particles can be rendered in

class I_RendererView
{
public:
	I_RendererView() : m_pRenderer(NULL) {}
	I_RendererView(const I_Renderer* pRenderer, const C_View* pView) : m_pRenderer(pRenderer), m_pView(pView) {}
	~I_RendererView() {}

	virtual void Create() {};
	virtual void Destroy() {};

	virtual void BeginGather(const C_View& view) const {}
	virtual void EndGather(const C_View& view) const {}
	virtual void BeginRender(const C_View& view) const {}
	virtual void EndRender(const C_View& view) const {}
	virtual void EndFrame(const C_View& view) const {}

	// called just before the pass is actually rendered, either normally on main thread or as a command buffer being run
	virtual void RenderPass(S32 renderPass) const {}

	const C_View*			GetView() const { return m_pView; }
	const I_Renderer*		GetRenderer() const { return m_pRenderer; }

protected:
	const I_Renderer*		m_pRenderer;
	const C_View*			m_pView;
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_RENDERER_VIEW_H
