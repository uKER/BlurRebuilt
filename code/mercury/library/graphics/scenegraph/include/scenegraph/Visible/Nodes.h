#ifndef SCENE_GRAPH_VISBLE_NODES_H
#define SCENE_GRAPH_VISBLE_NODES_H

#ifdef _MSC_VER
#pragma once
#endif

#include "SceneGraph/Visible/VolatileNodeInfo.h"

namespace SceneGraph
{

class I_RenderNode;

//! C_VisibleNode

// C_Views contain arrays of visible nodes which are obtained somehow from the hierarchy
// sort key contains something we want to sort with for this

// about 10% quicker to make this 4 bytes where possible
class C_VisibleNode
{
public:
	C_VisibleNode() {}
	U32			GetPredication() const { return 0; }

	~C_VisibleNode() {}

	const I_RenderNode*		GetNode() const { return m_pNode; }

	template<class T>
	const T*				GetNode() const { return static_cast<const T*>(m_pNode); }
	void					SetNode(const I_RenderNode* pNode) { m_pNode = pNode; }

	FORCE_INLINE const bool operator<(const C_VisibleNode& other) const
	{
		return m_SortKey > other.m_SortKey;
	}

	U32						m_SortKey;

	// actual node that gets rendered
	const I_RenderNode*		m_pNode;
	VolatileNodeInfo		m_VolatileNodeInfo;
};

// abstract visible nodes can be put into node batches of the same type to help rendering
// these then get submitted to a specific renderer - eg particle renderer, crowd renderer, etc
class C_VisibleNodeBatch
{
public:
	C_VisibleNodeBatch() {}
	~C_VisibleNodeBatch() {}

	S32			m_NodeType;
	S32			m_NodeCount;
	S32			m_StartIndex;
	S32			m_EndIndex;
};

// generic stuff
typedef Btl::Vector<C_VisibleNode>						C_VisibleNodes;
typedef Btl::Vector<C_VisibleNodeBatch>					C_VisibleNodeBatches;

class I_VisibleNodeListSorter;

inline U32 RadixKey(const C_VisibleNode& info)
{
	return info.m_SortKey;
}

struct ALIGN16 C_VisibleNodeList
{
	U32						m_PassIndex;
	U32						m_PassFlags;
	const I_VisibleNodeListSorter*	m_pSorter;
	U32						m_ScreenSize;
	bool					m_Processed;
	bool					m_Empty;
	bool					m_Abstract;

	// generic nodes - these have possibly been sorted
	C_VisibleNodes			m_Nodes;
	// batches of nodes of the same type - these can get submitted to a renderer with a single virtual
	C_VisibleNodeBatches	m_NodeBatches;
	
	// render list specific nodes for when we don't need to sort inbetween node types
	C_VisibleNodes			m_StaticPrimitives;
	C_VisibleNodes			m_StaticInstances;
	C_VisibleNodes			m_ModelInstances;
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_VIEW_H
