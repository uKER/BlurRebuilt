#include <stdafx.h>
#include <SceneGraph/ListNode.h>

using namespace SceneGraph;


/// Default c'tor.
I_ListNode::I_ListNode() : 
	I_GatherNode(-1, NULL)
{
	return;
}

// -----------------------------------------------------------------

/// Specified c'tor.
I_ListNode::I_ListNode(S32 iType, const C_SceneGraph* pSceneGraph) : 
	I_GatherNode(iType, pSceneGraph)
{
	return;
}

// -----------------------------------------------------------------

/// D'tor.
I_ListNode::~I_ListNode()
{
	return;
}

// -----------------------------------------------------------------

/// Accept a visitor.
void I_ListNode::Accept(I_Visitor& rkVisitor)
{
	return;
}

// -----------------------------------------------------------------

/// Gather the node.
void I_ListNode::Gather(const C_View& rkView) const
{
	return;
}

// -----------------------------------------------------------------

/// Begin frame method.
void I_ListNode::OnBeginFrame() const
{
	// Does nothing by default, overload in the derived classes.
	return;
}


// -----------------------------------------------------------------

/// Handle movement of a model instance node.
void I_ListNode::OnMove(C_ModelInstance* pModelInstance)
{
	// Default implementation does nothing.
	return;
}