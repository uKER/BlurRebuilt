#ifndef SCENE_GRAPH_MEMORY_USAGE_VISITOR_H
#define SCENE_GRAPH_MEMORY_USAGE_VISITOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include "SceneGraph/Visitor.h"

namespace SceneGraph
{

class C_MemoryUsageVisitor : public I_Visitor
{
public:
	C_MemoryUsageVisitor();
	~C_MemoryUsageVisitor();

	void	Visit(const I_GatherNode& node);
	void	Visit(const C_LevelInstance& node);
	void	Visit(const C_CachedStaticScene& node);
	void	Visit(const C_CachedRenderListNode& node);
	void	Visit(const C_StaticRenderListNode& node);
	void	Visit(const C_ModelInstanceFlatList& node) {}
	void	Visit(const C_ModelInstance& node);
	void	Visit(const C_LightManager& node);
	void	Visit(const C_SceneGraph& node);
private:

	U32		m_MemoryUsed;
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_H
