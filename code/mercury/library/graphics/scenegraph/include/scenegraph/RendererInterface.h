#ifndef SCENE_GRAPH_RENDERER_INTERFACE_H
#define SCENE_GRAPH_RENDERER_INTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <SceneGraph/Visible/Nodes.h>


namespace SceneGraph
{

class C_Context;
class I_RendererView;

// renderers are external rendering classes, eg DynamicEffect renderer, game specific rendering etc, etc
class I_Renderer
{
public:
	I_Renderer(C_SceneGraph* pSceneGraph);
	virtual ~I_Renderer();

	// renderers can store additional renderer-specific data about views which exist onto the scenegraph
	virtual I_RendererView*	AddView(const C_View* pParent) { return NULL; }
	virtual void			RemoveView(const C_View* pParent, const S32 index) {}
	
	// called by the scenegraph at the start of the frame
	virtual void		BeginFrame() const {}

	// build a batch of visible nodes. this could involve prebuilding data (eg dynamic vertices) for each node
	// or could even involve replacing nodes with composite nodes (combining nodes)
	// this happens straight after gathering, potentially a while before rendering
	virtual void		Build(const SceneGraph::C_View& view, const S32 renderPass, C_VisibleNodes& visibleNodes, const C_VisibleNodeBatch& batch) const {}
	virtual void		EndBuild(const SceneGraph::C_View& view, const S32 renderPass) const {}

	// render a subset of visible nodes
	virtual void		Render(const SceneGraph::C_View& view, C_Context& renderContext, const C_VisibleNodes& visibleNodes, const C_VisibleNodeBatch& batch) const {}
	
	const U32			GetNodeFlag() const { return m_NodeFlag; }
	const U32			GetNodeType() const { return m_NodeType; }

	friend class C_SceneGraph;

protected:
	U32 				ViewEnabled(const C_View* View);

	U32					m_NodeType;
	U32					m_NodeFlag;
	C_SceneGraph*		m_pSceneGraph;
};

template<class View>
class T_Renderer : public I_Renderer
{
public:
	T_Renderer(C_SceneGraph* pSceneGraph) : I_Renderer(pSceneGraph) {}

	virtual I_RendererView*	AddView(const C_View* pParent);
	virtual void			RemoveView(const C_View* pParent, const S32 index);

	const View&				GetView(const S32 index) const;

protected:
	Btl::Vector<View*>		m_Views;
};

template<class View>
I_RendererView* T_Renderer<View>::AddView(const C_View* pParent)
{
	View* pView;

	// only create view if render node is enabled for this view
	if(ViewEnabled(pParent))
	{
		pView = new View(this, pParent);
		pView->Create();

		m_Views.PushBack(pView);
	}
	else
	{
		pView = NULL;
		m_Views.PushBack(pView);
	}

	return pView;
}

template<class View>
void T_Renderer<View>::RemoveView(const C_View* pParent, const S32 index)
{
	if(View* pView = m_Views[index])
	{
		pView->Destroy();
		delete pView;
	}

	m_Views.EraseByIndexNonOrdered(index);
}

template<class View>
const View& T_Renderer<View>::GetView(const S32 index) const
{
	return *m_Views[index];
}

} // namespace SceneGraph

#endif // SCENE_GRAPH_RENDERER_INTERFACE_H
