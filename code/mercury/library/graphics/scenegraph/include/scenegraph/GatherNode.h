#ifndef SCENE_GRAPH_NODE_H
#define SCENE_GRAPH_NODE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace SceneGraph
{

class C_SceneGraph;
class C_View;
class C_MultiView;
class I_Visitor;

enum E_GatherNodeType
{
	E_GatherNodeType_Unknown = -1,
	
	E_GatherNodeType_ModelInstanceNode,

	// static renderlist nodes
	E_GatherNodeType_StaticRenderListNode,
	
	// cached static renderlist node
	E_GatherNodeType_CachedRenderListNode,

	E_GatherNodeType_Count
};

// represents a class which is gathered for rendering. one or more render nodes is the result
class I_GatherNode
{
public:
	I_GatherNode() : m_Type(-1), m_pSceneGraph(NULL) {}
	I_GatherNode(S32 type, const C_SceneGraph* pSceneGraph) : m_Type(type), m_pSceneGraph(pSceneGraph) {}
	virtual ~I_GatherNode() {}

	void						BeginFrame() const;
	void						Gather(const C_View& view) const;
	void						Gather(const C_MultiView& multiView) const;
	void						GatherDeferred(const void* pData) const;
	const C_SceneGraph*			GetSceneGraph() const;

	// virtual add and remove for group nodes
	virtual const S32			GetNumChildren() const { return 0; }
	virtual const I_GatherNode*	GetChild(S32 i) const { return 0; }
	virtual I_GatherNode*		GetChild(S32 i) { return 0; }
	virtual void				AddChild(I_GatherNode* pChild) {}
	const S32					GetType() const;

	// visitor methods
	virtual void				Accept(I_Visitor& visitor) const = 0;

	virtual void				OnInsert(C_SceneGraph& sceneGraph) {};
	virtual void				OnRemove(C_SceneGraph& sceneGraph) {};

protected:
	const C_SceneGraph*			m_pSceneGraph;
private:
	virtual void				OnBeginFrame() const {}
	virtual void				OnGather(const C_View& view) const = 0;
	virtual void				OnGatherDeferred(const void* pData) const {}

	virtual void				OnGather(const C_MultiView& view) const;
		
	S32							m_Type;
};


// ! C_GroupNode - gather node with children

class C_GroupNode : public I_GatherNode
{
public:
	C_GroupNode(const C_SceneGraph* pSceneGraph);
	C_GroupNode(S32 type, const C_SceneGraph* pSceneGraph);
	virtual ~C_GroupNode();

	const S32				GetNumChildren() const { return m_Children.Size(); }
	const I_GatherNode*		GetChild(S32 i) const { return m_Children[i]; }
	I_GatherNode*			GetChild(S32 i) { return m_Children[i]; }

	void					AddChild(I_GatherNode* pChild);
	void					RemoveChild(I_GatherNode* pChild);

	void					BeginFrame() const;

	void					Gather(const C_View& view) const;
	void					Gather(const C_MultiView& multiView) const;
	
	void					Accept(I_Visitor& visitor) const;
	void					Accept(I_Visitor& visitor);

protected:
	virtual void			OnBeginFrame() const { BeginFrame(); }
	virtual void			OnGather(const C_View& view) const { Gather(view); }
	virtual void			OnGather(const C_MultiView& multiView) const { Gather(multiView); }
	
	Btl::Vector<I_GatherNode*>	m_Children;

};

} // namespace SceneGraph

#include "GatherNode.inl"

#endif // SCENE_GRAPH_NODE_H
