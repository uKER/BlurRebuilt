#ifndef SCENE_GRAPH_DEBUG_RENDER_VISITOR_H
#define SCENE_GRAPH_DEBUG_RENDER_VISITOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include "SceneGraph/Visitor.h"

namespace SceneGraph
{

class I_GatherNode;
class C_CachedStaticScene;
class C_LevelInstance;
class C_CachedRenderListNode;
class C_StaticRenderListNode;
class C_ModelInstanceFlatList;
class C_ModelInstance;
class C_LightManager;

class C_DebugRenderVisitor : public I_Visitor
{
public:
	C_DebugRenderVisitor();
	~C_DebugRenderVisitor();

	void	Visit(const I_GatherNode& node);
	void	Visit(const C_LevelInstance& node);
	void	Visit(const C_CachedStaticScene& node);
	void	Visit(const C_CachedRenderListNode& node);
	void	Visit(const C_StaticRenderListNode& node);
	void	Visit(const C_ModelInstanceFlatList& node) {}
	void	Visit(const C_ModelInstance& node) {}
	void	Visit(const C_LightManager& node);
	void	Visit(const C_SceneGraph& node) {}
private:

	S32		m_RenderListIndex;
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_H
