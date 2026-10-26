#ifndef SCENE_STATICSCENE_H
#define SCENE_STATICSCENE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "SceneGraph/GatherNode.h"
#include "RenderingData/ResourceIndex.h"
#include "SceneGraph/RenderingData/CachedRenderListNodeGather.h"
#include "SceneGraph/Util/RingBuffer.h"
#include "Btl/Allocators/Pool.h"
#include "Btl/Containers/ChainedHashMap.h"

namespace RenderingData
{
	class Node;

	namespace Target
	{
		class Scene;
		class RenderListNode;
		class Context;
		class MaterialList;
		class MaterialListData;
	}
}

namespace SceneGraph
{

class I_GatherNode;
class C_SceneGraph;
class I_View;
class C_View;
class C_MultiView;
class C_GroupNode;

class C_LevelInstance;
class C_StaticRenderListNode;
class C_CachedStaticScene;
class C_CachedRenderListNode;

// like a group node expect it calls a method on an owner object rather than deleting children
template<class T>
class T_StaticSceneRootNode : public C_GroupNode
{
public:
	T_StaticSceneRootNode(const C_SceneGraph* pSceneGraph, T* pStaticScene);
	~T_StaticSceneRootNode();

private:
	T*		m_pStaticScene;
};

template<class T>
T_StaticSceneRootNode<T>::T_StaticSceneRootNode(const C_SceneGraph* pSceneGraph, T* pStaticScene) : C_GroupNode(pSceneGraph), m_pStaticScene(pStaticScene)
{

}

template<class T>
T_StaticSceneRootNode<T>::~T_StaticSceneRootNode()
{
	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		I_GatherNode* pGatherNode = m_Children[i];
		m_pStaticScene->DeleteNode(pGatherNode);
	}
	
	m_Children.Clear();
}

typedef T_StaticSceneRootNode<C_CachedStaticScene>	C_CachedStaticSceneRootNode;

typedef Btl::ChainedHashMap<const I_GatherNode*, I_GatherNode*, Btl::HashKeyOpDefault<const I_GatherNode*>, Btl::DynamicPool<> >	CachedNodeCache;

class C_CachedStaticScene : public I_GatherNode
{
public:
	C_CachedStaticScene(const C_SceneGraph* pSceneGraph) : I_GatherNode(-1, pSceneGraph), m_pRootNode(NULL) {}
	~C_CachedStaticScene();

	void					Build(const C_LevelInstance* pStaticScene);
	void					Destroy();
	void					Gather(const C_View& view) const;
	void					Gather(const C_MultiView& multiView) const;
	void					Accept(I_Visitor& visitor) const;

	const CachedNodeCache&	GetNodeCache() const { return m_CachedNodeCache; }

	friend class T_StaticSceneRootNode<C_CachedStaticScene>;

private:
	void	OnGather(const C_View& view) const { Gather(view); }
	void	OnGather(const C_MultiView& multiView) const { Gather(multiView); }
	void	DeleteNode(I_GatherNode* pGatherNode);
	// builds a cached version of a scenegraph hierarchy based on addition information (eg pvs)
	void	Build(I_GatherNode* pParent, const I_GatherNode* pNode);

	CachedNodeCache								m_CachedNodeCache;

	C_CachedStaticSceneRootNode*				m_pRootNode;
	const C_LevelInstance*						m_pStaticScene;
};

// static scene

typedef T_StaticSceneRootNode<C_LevelInstance>	C_StaticSceneRootNode;

typedef Btl::ChainedHashMap<RenderingData::ResourceIndex, I_GatherNode*, Btl::HashKeyOpDefault<RenderingData::ResourceIndex>, Btl::DynamicPool<> >	NodeCache;

class C_LevelInstance : public I_GatherNode
{
public:
	C_LevelInstance(const C_SceneGraph* pSceneGraph, const RenderingData::Target::Scene* pScene);
	~C_LevelInstance();
	
	void	Build();
	void	Destruct();	// renamed from Destroy to stop conflict with Btl::List

	void	BeginFrame() const;
	void	Gather(const C_View& view) const;
	void	Gather(const C_MultiView& multiView) const;
	
	void	Accept(I_Visitor& visitor) const;
	
	const RenderingData::Target::Scene*			GetScene() const { return m_pScene; }
	const I_GatherNode*							GetRootNode() const { return m_pRootNode; }
	const NodeCache&							GetNodeCache() const { return m_NodeCache; }

	void SetGroupMask(U32 mask);
	U32 GetGroupMask() {return m_groupMask;}
	void GetDynamicMaterialData(	
		RenderingData::Target::MaterialListData **ppMaterialListData,
		const RenderingData::Target::MaterialList **ppMaterialListTypes,
		const void*									*ppMaterialListDmaData) const;


	static void	BeginGather1(const I_View& view);
	static void BeginGather2(const I_View& view);
	
	friend class T_StaticSceneRootNode<C_LevelInstance>;
	friend class C_CachedRenderListNode;
	friend class C_MemoryUsageVisitor;
	friend class C_CachedRenderListNodeGatherBatcher;

private:
	void	OnGather(const C_View& view) const { Gather(view); }
	void	OnGather(const C_MultiView& multiView) const { Gather(multiView); }
	
	// builds a scenegraph version of the rendering data hierarchy
	void	Build(I_GatherNode* pParent, const RenderingData::Node* pNode);
	
	const C_SceneGraph*							m_pSceneGraph;

	// pointer to scene data
	const RenderingData::Target::Scene*			m_pScene;
	S32											m_RenderListNodes;

	// map of render list node cache
	void										DeleteNode(I_GatherNode* pGatherNode);
	NodeCache									m_NodeCache;

	// hierarchy of scene. just a root node containing a bunch of renderlist nodes for now
	// could contain arbitrary other nodes, eg portal nodes
	C_StaticSceneRootNode*						m_pRootNode;

	// cached subset of above hierarchy based on pvs and portals
	//C_GroupNode*								m_pCachedRootNode;
	C_CachedStaticScene							m_CachedScene;
	U32											m_groupMask;

protected:
	// dynamic material list data for global dynamic materials on the level
	RenderingData::Target::MaterialListData*	m_pMaterialListData;
	const RenderingData::Target::MaterialList*	m_pMaterialListTypes;
	const void*									m_pMaterialListDmaData;

	// memory for spu program input/output
#ifdef USE_DYNAMIC_RINGBUFFER
	static SceneGraph::DynamicRingBuffer<CachedRenderListNodeDataBatch>					m_CachedRenderListNodeDataBatch;
	static SceneGraph::DynamicRingBuffer<CachedRenderListNodeData>						m_CachedRenderListNodeData;
	static SceneGraph::DynamicRingBuffer<CachedRenderListNodeData::PassData>			m_CachedRenderListNodeDataPass;
	static SceneGraph::DynamicRingBuffer<CachedRenderListNodeData::PassData::Output>	m_CachedRenderListNodeDataPassOutput;
#else
	static Btl::DynamicPool<CachedRenderListNodeDataBatch>					m_CachedRenderListNodeDataBatch;
	static Btl::DynamicPool<CachedRenderListNodeData>						m_CachedRenderListNodeData;
	static Btl::DynamicPool<CachedRenderListNodeData::PassData>				m_CachedRenderListNodeDataPass;
	static Btl::DynamicPool<CachedRenderListNodeData::PassData::Output>		m_CachedRenderListNodeDataPassOutput;
#endif
};


} // namespace SceneGraph

#endif // SCENE_GRAPH_H
