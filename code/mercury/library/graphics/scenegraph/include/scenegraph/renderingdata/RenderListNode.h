#ifndef SCENE_GRAPH_RENDERLISTNODE_H
#define SCENE_GRAPH_RENDERLISTNODE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <SceneGraph/RenderNode.h>

namespace RenderingData
{
namespace Target
{
	class Scene;
	class RenderListNode;
}
}

namespace SceneGraph
{

template<class Node>
class T_RenderListItem : public I_RenderNode
{
public:
	T_RenderListItem() : I_RenderNode() {}
	T_RenderListItem(S32 type, const Node* pNode) :
		I_RenderNode(type),
		m_pNode(pNode) {}
	~T_RenderListItem() {}

	const Node*		GetNode() const { return m_pNode; }
	
protected:
	
	const Node*		m_pNode;
};

class C_RenderListNodeBase
{
public:
	C_RenderListNodeBase(const RenderingData::Target::Scene* pScene, const RenderingData::Target::RenderListNode* pRenderListNode);
	~C_RenderListNodeBase() {};

	const RenderingData::Target::RenderListNode*	GetRenderListNode() const { return m_pRenderListNode; }

#if !PLATFORM_SPU
	const RenderingData::Target::Scene*				GetScene() const { return m_pScene; }
#else
	const void*										GetNodeDmaData() const { return m_pNodeDmaData; }
#endif

protected:
	const RenderingData::Target::Scene*				m_pScene;
	const RenderingData::Target::RenderListNode*	m_pRenderListNode;
	const void*	m_pNodeDmaData;
};

// T_RenderListNode

template<class Primitive, class Instance, class InstanceBatch>
class T_RenderListNode : public C_RenderListNodeBase
{
public:
	T_RenderListNode(const RenderingData::Target::Scene* pScene, const RenderingData::Target::RenderListNode* pRenderListNode) :
		C_RenderListNodeBase(pScene, pRenderListNode) {};
	~T_RenderListNode() {};

	
	U32												GetNumPrimitives() const { return m_PrimitiveNodes.Size(); }
	U32												GetNumInstances() const { return m_InstanceNodes.Size(); }
	U32												GetNumInstanceBatches() const { return m_InstanceBatchNodes.Size(); }
	
	const Primitive&								GetPrimitiveNode(U32 index) const { return m_PrimitiveNodes[index]; }
	const Btl::Vector<Primitive>&					GetPrimitiveNodes() const { return m_PrimitiveNodes; }
	const Instance&									GetInstanceNode(U32 index) const { return m_InstanceNodes[index]; }
	const Btl::Vector<Instance>&					GetInstanceNodes() const { return m_InstanceNodes; }
	const InstanceBatch&							GetInstanceBatchNode(U32 index) const { return m_InstanceBatchNodes[index]; }
	const Btl::Vector<InstanceBatch>&				GetInstanceBatchNodes() const { return m_InstanceBatchNodes; }

protected:

	// scene graph objects which represent renderables in the render list
	Btl::Vector<Primitive>							m_PrimitiveNodes;
	Btl::Vector<Instance>							m_InstanceNodes;
	Btl::Vector<InstanceBatch>						m_InstanceBatchNodes;
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_RENDERLISTNODE_H
