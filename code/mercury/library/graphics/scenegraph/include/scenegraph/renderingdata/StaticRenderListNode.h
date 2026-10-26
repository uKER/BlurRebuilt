#ifndef SCENE_GRAPH_STATICRENDERLISTNODE_H
#define SCENE_GRAPH_STATICRENDERLISTNODE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/scheduler/jobqueue.h>

#include "SceneGraph/RenderingData/RenderListNode.h"

#include "SceneGraph/GatherNode.h"
#include "SceneGraph/CullNode.h"
#include "SceneGraph/Context.h"

#include <RenderingData/ResourceIndex.h>
#include <renderingdata/MaterialIndex.h>

namespace SceneGraph { class C_LevelInstance; }	// [Blur2PC] defined in StaticScene.h

namespace RenderingData
{
	class MaterialType;

	namespace Target
	{
		class Bounds;
		class Scene;
		class RenderListNode;
		class Context;
		class MaterialList;
		class MaterialListData;
	}
}

namespace Animation
{
	class C_PoseFormat;
}

namespace SceneGraph
{

class C_Context;
class C_StaticRenderListNode;

class C_StaticRenderListItem : public T_RenderListItem<C_StaticRenderListNode>
{
public:
	C_StaticRenderListItem() :
		T_RenderListItem<C_StaticRenderListNode>() {}
	C_StaticRenderListItem(S32 type, const C_StaticRenderListNode* pRenderListNode) :
		T_RenderListItem<C_StaticRenderListNode>(type, pRenderListNode) {}
	~C_StaticRenderListItem() {}

protected:
	void		DebugRenderBounds(const C_VisibleNode& visibleNode, const RenderingData::Bounds& bounds, const Maths::RGBA& colour) const;
	void		DebugRenderPaletteId(const RenderingData::Bounds& bounds, const S16 paletteId) const;
};

class C_StaticPrimitive : public C_StaticRenderListItem
{
public:
	C_StaticPrimitive() :
		C_StaticRenderListItem() {}
	C_StaticPrimitive(const C_StaticRenderListNode* pRenderListNode, U32 primitiveIndex) :
		C_StaticRenderListItem(E_RenderNodeType_StaticPrimitive, pRenderListNode),
		m_PrimitiveIndex(primitiveIndex) {}
	~C_StaticPrimitive() {}

	void		DebugRender(const C_VisibleNode& node) const;

	const U32	GetPrimitiveIndex() const { return m_PrimitiveIndex; }

	friend class C_StaticRenderListNode;

private:
	U32			m_PrimitiveIndex;
};

// C_StaticInstance

class C_StaticInstance : public C_StaticRenderListItem
{
public:
	C_StaticInstance() :
		C_StaticRenderListItem(), m_PaletteId(-1) {}
	C_StaticInstance(const C_StaticRenderListNode* pRenderListNode, U32 instanceIndex) :
		C_StaticRenderListItem(E_RenderNodeType_StaticInstance, pRenderListNode),
		m_InstanceIndex(instanceIndex), m_PaletteId(-1) {}
	~C_StaticInstance() {}

	void		DebugRender(const C_VisibleNode& node) const;

	const U32	GetInstanceIndex() const { return m_InstanceIndex; }

	const S16	GetPaletteId() const { return m_PaletteId; }

	friend class C_StaticRenderListNode;

private:
	U32			m_InstanceIndex;

	S16			m_PaletteId;
};

// C_StaticRenderListInstanceBatch

class C_StaticInstanceBatch : public C_StaticRenderListItem
{
public:
	C_StaticInstanceBatch() :
		C_StaticRenderListItem() {}
	C_StaticInstanceBatch(const C_StaticRenderListNode* pRenderListNode, U32 instanceBatchIndex) :
		C_StaticRenderListItem(E_RenderNodeType_StaticInstanceBatch, pRenderListNode),
		m_InstanceBatchIndex(instanceBatchIndex) {}
	~C_StaticInstanceBatch() {}

	void		Render(SceneGraph::C_Context& context) const {}
	void		DebugRender(const C_VisibleNode& node) const {}

	friend class C_StaticRenderListNode;

protected:
	void		OnRender(SceneGraph::C_Context& context) const { Render(context); }

private:
	U32			m_InstanceBatchIndex;
};

// C_StaticRenderListNode

struct FrustumCullInput;
struct FrustumCullOutput;

// static render list nodes are fixed spatial cull nodes and also render list nodes
class C_StaticRenderListNode :	public T_RenderListNode<C_StaticPrimitive, C_StaticInstance, C_StaticInstanceBatch>,
								public I_GatherNode,
								public C_BoundsCull,
								public C_PassCull
													
{
	friend class C_Context;
public:
	C_StaticRenderListNode(const C_SceneGraph* pSceneGraph);
	~C_StaticRenderListNode();

	void		Build(const C_LevelInstance *pScene, const RenderingData::Target::RenderListNode* pRenderListNode);
	void		BeginFrame() const;
	void		Gather(const C_View& view) const;
	void		Accept(I_Visitor& visitor) const;

#ifndef PLATFORM_SPU
	RenderingData::ResourceIndex	GetLocation() const { return m_Location; }
#endif // PLATFORM_SPU

private:
	virtual void	OnBeginFrame() const { BeginFrame(); }
	void			OnGather(const C_View& view) const { Gather(view); }
	void			OnRender(SceneGraph::C_Context& context) const { assert(0); }

#ifndef PLATFORM_SPU
	void			BuildDynamicMaterialList(const U32 numMatrixPalettes);
#endif // PLATFORM_SPU

#ifndef PLATFORM_SPU
	RenderingData::ResourceIndex			m_Location;
#endif // PLATFORM_SPU

protected:
	// dynamic material list data for global dynamic materials on the level
	RenderingData::Target::MaterialListData*	m_pLevelMaterialListData;
	const RenderingData::Target::MaterialList*	m_pLevelMaterialListTypes;
	const void*									m_pLevelMaterialListDmaData;
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_STATICRENDERLISTNODE_H
