#ifndef SCENE_GRAPH_VISITOR_H
#define SCENE_GRAPH_VISITOR_H

#ifdef _MSC_VER
#pragma once
#endif

namespace SceneGraph
{

class I_GatherNode;
class C_LevelInstance;
class C_CachedStaticScene;
class C_CachedRenderListNode;
class C_StaticRenderListNode;
class C_ModelInstanceFlatList;
class C_ModelInstance;
class C_LightManager;
class C_SceneGraph;

class I_Visitor 
{
public:
	I_Visitor() : m_Type(-1) {}
	I_Visitor(S32 type) : m_Type(type) {}
	~I_Visitor() {}

	const S32 GetType() const { return m_Type; }

	virtual void Visit(const I_GatherNode& node) = 0;
	virtual void Visit(const C_LevelInstance& node) = 0;
	virtual void Visit(const C_CachedStaticScene& node) = 0;
	virtual void Visit(const C_CachedRenderListNode& node) = 0;
	virtual void Visit(const C_StaticRenderListNode& node) = 0;
	virtual void Visit(const C_ModelInstanceFlatList& node) = 0;
	virtual void Visit(const C_ModelInstance& node) = 0;
	virtual void Visit(const C_LightManager& node) = 0;
	virtual void Visit(const C_SceneGraph& sceneGraph) = 0;
protected:
	S32		m_Type;
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_H
