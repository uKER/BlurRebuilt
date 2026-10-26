#ifndef SCENE_GRAPH_MODELINSTANCENODE_H
#define SCENE_GRAPH_MODELINSTANCENODE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/bitvector.h>
#include "SceneGraph/RenderingData/RenderListNode.h"
#include "SceneGraph/RenderingData/ModelInstanceCullNode.h"
#include "RenderingData/MaterialIndex.h"
#include "RenderingData/RenderEnums.h"
#include "RenderingData/primitivestream.h"


namespace RenderingData
{
	class Node;

	namespace Target
	{
		class Context;
		class MaterialList;
		class MaterialListData;
	}
}

namespace SceneGraph
{

class C_Context;
class C_ModelInstance;
class C_VisibleDetect;
class C_SceneGraph;

class C_ModelInstancePrimitive : public T_RenderListItem<C_ModelInstance>
{
public:
	C_ModelInstancePrimitive(const C_ModelInstance* pModelInstance, U16 primitiveIndex, U16 elementIndex, U32 visibilityFlags, F32 distanceMin, F32 distanceMax) :
		T_RenderListItem<C_ModelInstance>(E_RenderNodeType_ModelInstancePrimitive, pModelInstance),
		m_PrimitiveIndex(primitiveIndex),
		m_ElementIndex(elementIndex),
		m_VisibilityFlags(visibilityFlags),
		m_DistanceMin(distanceMin),
		m_DistanceMax(distanceMax)
		{}
	~C_ModelInstancePrimitive() {}

	void		DebugRender(const C_VisibleNode& visibleNode) const {};
	const U16	GetPrimitiveIndex() const { return m_PrimitiveIndex; }
	const U16	GetElementIndex() const { return m_ElementIndex; }
	const U32	GetVisibilityFlags() const { return m_VisibilityFlags; }

	const U32	IsVisibleType(const U32 typeFlag) const;
	const U32	IsVisibleDistance(const F32 distance, const F32 radius) const;

private:
	U16			m_PrimitiveIndex;
	U16			m_ElementIndex;
	U32			m_VisibilityFlags;
	F32			m_DistanceMin;
	F32			m_DistanceMax;
};

INLINE const U32 C_ModelInstancePrimitive::IsVisibleType(const U32 typeFlag) const
{
	return typeFlag == (typeFlag & m_VisibilityFlags);
}

INLINE const U32 C_ModelInstancePrimitive::IsVisibleDistance(const F32 distanceMin, const F32 distanceMax) const
{
	return distanceMin > m_DistanceMin && distanceMax < m_DistanceMax;
}

class C_ModelInstanceDummyObject
{
public:
	C_ModelInstanceDummyObject() {};
	~C_ModelInstanceDummyObject() {};
};

// renderable model instance node as a dynamic culled object. contains a render list which is gathered either in whole or in part
class C_ModelInstance : public T_RenderListNode<C_ModelInstancePrimitive, C_ModelInstanceDummyObject, C_ModelInstanceDummyObject>,
						public I_RenderNode
							
{
	friend class C_Context;
	friend class C_ModelInstanceFlatList;
public:
	C_ModelInstance(const C_SceneGraph* pSceneGraph, const RenderingData::Target::Scene* pScene, const RenderingData::Target::RenderListNode* pRenderListNode);
	virtual ~C_ModelInstance();

	void									SetRenderingDataRenderPassMapping(RenderingData::RenderPass renderPass, const S32 passType);
	void									Build();

	void									Gather(const C_View& view, const U32 predication, const U32 volatileNodeInfoIndex) const;

	// I_RenderNode methods
	void									Render(SceneGraph::C_Context& context) const;
	void									DebugRender(const C_VisibleNode& node) const;

	void									Bind(SceneGraph::C_Context& context) const;

	// Model visibility
	void									SetVisibilityFlags(U32 visibilityFlags);
	const U32								GetVisibilityFlags() const  { return m_VisibilityFlags;}
	void									EnableVisibilityFlags(U32 visibilityFlags);
	void									DisableVisibilityFlags(U32 visibilityFlags);

	// Element visibility
	void									SetElementVisibility(const S32 index, const bool visibility);
	bool									GetElementVisibility(const S32 index) const;
	U32										GetNumElements() const { return m_Elements; }
	U32										GetNumVisibleElements() const { return m_VisibleElements; }

	// Cull node
	void									SetCullNode(I_ModelInstanceCullNode* pCullNode);
	I_ModelInstanceCullNode*				GetCullNode() { return m_pCullNode; }
	const I_ModelInstanceCullNode*			GetCullNode() const { return m_pCullNode; }
	U32										GetVolatileInfoFlags() const { return m_VolatileInfoFlags; }
	const SceneGraph::C_SceneGraph*			GetSceneGraph() const { return m_pSceneGraph; }
	void									FlagMovement();


	// set visibility detection flag and add this model instance to the scenegraph's 'newly visible' list of applicable
	void									SetVisible(U32 iViewBitfield) const;

	// set custom data which is added into the 'newly visible' list on the scenegraph when this modelinstance becomes visible
	void									SetVisDetect(C_VisibleDetect *p) { m_pVisDetect = p; }

	// override sub technique handle
	void									SetSubtechniqueHandle(U32 subtechniqueHandle) { m_SubtechniqueHandle = subtechniqueHandle; }
	U32										GetSubtechniqueHandle() const { return m_SubtechniqueHandle; }

	// set the parent matrix pointer. this is used during rendering to calculate the PVS viewing direction
	void									SetParentMatrixPtr(const Maths::HMat43 *pMtx);
	const Maths::HMat43	*					GetParentMatrixPtr() const { return m_pParentMatrix; }

	// accessors for dynamic primitive streams and vertex buffer pointers.
	void									AddDynamicVertexBuffer(const RenderingData::Target::VertexBufferData& rkVertexBuffer);
	void									AddDynamicPrimitiveStreams(const RenderingData::Target::PrimitiveStream& rkPrimitiveStream);
	const Btl::Vector<RenderingData::Target::PrimitiveStream>&
											GetDynamicPrimitiveStreams() const; 
	const Btl::Vector<RenderingData::Target::VertexBufferData>&	
											GetDynamicVertexBuffers() const; 
private:
	// move these up to avoid cache misses during gathering
	C_VisibleDetect*																	m_pVisDetect;
	Btl::Vector<S32, Btl::VectorParameters_Static<RenderingData::RENDER_PASS_MAX> >		m_Passes;

protected:
	RenderingData::Target::MaterialListData*	m_pMaterialListData;
	const RenderingData::Target::MaterialList*	m_pMaterialListTypes;
	const void*									m_pMaterialListDmaData;
	RenderingData::MaterialIndex				m_MaterialIndex;
	I_ModelInstanceCullNode*					m_pCullNode;

private:
	void									BuildRenderPasses();
	
	const C_SceneGraph*						m_pSceneGraph;
	
	U32																					m_Elements;
	Btl::BitVector																		m_VisibleElementFlags;
	Btl::BitVector																		m_RenderableElementFlags;
	U32																					m_VisibleElements;
	U32																					m_VolatileInfoFlags;
	Btl::Vector<S32>																	m_SgToRdPassMapping;
	Btl::Vector<S32, Btl::VectorParameters_Array<RenderingData::RENDER_PASS_MAX> >		m_RdToSgPassMapping;
	U32																					m_SubtechniqueHandle;
	U32																					m_VisibilityFlags;
	bool																				m_PerPrimitiveGather;
	const Maths::HMat43 *																m_pParentMatrix;
	Btl::Vector<RenderingData::Target::PrimitiveStream>									m_DynamicVertexBufferPrimitiveStreams;			//!< Dynamic primitive streams.
	Btl::Vector<RenderingData::Target::VertexBufferData>								m_DynamicVertexBuffers;							//!< Dynamic vertex buffers.
		
	mutable U32																			m_Frame;
public:
	const char *							m_pszFriendlyName; // for debugging
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_MODELINSTANCENODE_H
