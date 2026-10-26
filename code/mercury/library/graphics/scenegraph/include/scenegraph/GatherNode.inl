#ifndef SCENE_GRAPH_NODE_INL
#define SCENE_GRAPH_NODE_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace SceneGraph
{

/// Return the scene graph pointer.
INLINE const C_SceneGraph* I_GatherNode::GetSceneGraph() const
{
 	return m_pSceneGraph;
}

/// Return the type of the gather node.
INLINE const S32 I_GatherNode::GetType() const
{
	return m_Type;
}

} // namespace SceneGraph

#endif // SCENE_GRAPH_NODE_INL
