#ifndef SCENE_GRAPH_MODELINSTANCECULLNODE_H
#define SCENE_GRAPH_MODELINSTANCECULLNODE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "SceneGraph/CullNode.h"
#include "SceneGraph/RenderNode.h"
#include "SceneGraph/GatherNode.h"

namespace SceneGraph
{

class C_ModelInstance;

/// Interface class for model instance cull nodes.
class ALIGN16 I_ModelInstanceCullNode : public C_BoundsCull,
										public C_PassCull,
										public C_GatherCull
{
public:
	/// C'tor.
	I_ModelInstanceCullNode(const C_ModelInstance* pModelInstance) : 
		m_pModelInstance(pModelInstance), 
		m_VolatileInfoFlags(0), 
		m_PassIndex(0xFFFFFFFF) 
	{ return; }
	/// D'tor.
	virtual ~I_ModelInstanceCullNode() 
	{ return; }

	/// Type of model instance cull node.
	enum ModelInstanceCullNodeType
	{
		MODELINSTANCECULLNODE_TYPE_BASIC,			// Basic type.
		MODELINSTANCECULLNODE_TYPE_PORTAL,			// Portal type.

		MODELINSTANCECULLNODE_TYPE_COUNT
	};
	
	/// Return the model instance pointer that parents this cull node.
	const C_ModelInstance*					GetModelInstance() const { return m_pModelInstance; }
	/// Return the pass index for the cull node.
	NOALIAS const U32						GetPassIndex() const __restrict { return m_PassIndex; }
	/// Return the volatile flags for the cull node.
	NOALIAS const U32						GetVolatileInfoFlags() const __restrict { return m_VolatileInfoFlags; }
	/// Return the type of the model instance cull node.
	virtual const ModelInstanceCullNodeType	GetType() const __restrict = 0;
	
	friend class C_ModelInstance;

protected:
	const C_ModelInstance*			m_pModelInstance;
	U32								m_VolatileInfoFlags;
	U32								m_PassIndex;
};

/// Basic cull node for model instances.
class ALIGN16 C_ModelInstanceBasicCullNode :	public I_ModelInstanceCullNode,
												public I_GatherNode
{
public:
	/// C'tor.
	C_ModelInstanceBasicCullNode(const C_ModelInstance* pModelInstance);
	/// D'tor.
	virtual ~C_ModelInstanceBasicCullNode();

	// I_GatherNode methods
	virtual void								Gather(const C_View& view) const;
	virtual void								Gather(const C_MultiView& multiView) const;
	virtual void								Accept(I_Visitor& visitor) const;
	
	friend class C_ModelInstanceFlatList;

	/// Set the position index.
	void										SetPositionIndex(S32 iIndex)
	{ m_PositionIndex = iIndex; }
	/// Return the position index.
	NOALIAS const S32							GetPositionIndex() const __restrict
	{ return m_PositionIndex; }
	/// Return the type of the model instance cull node.
	virtual const ModelInstanceCullNodeType		GetType() const __restrict
	{
		return I_ModelInstanceCullNode::MODELINSTANCECULLNODE_TYPE_BASIC;
	}

private:
	virtual void						OnGather(const C_View& view) const { Gather(view); }
	virtual void						OnGather(const C_MultiView& multiView) const { Gather(multiView); }

	S32									m_PositionIndex;	// Position in cull node array.
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_MODELINSTANCECULLNODE_H
