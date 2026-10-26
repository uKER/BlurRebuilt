#include "stdafx.h"
#include <SceneGraph/View.h>
#include <SceneGraph/MultiView.h>
#include <SceneGraph/Util.h>
#include <SceneGraph/Visitor.h>
#include <SceneGraph/renderingdata/PortalCullNode.h>

using namespace SceneGraph;


/// Accept a visitor to the cull node.
void C_PortalCullNode::Accept(I_Visitor& rkVisitor)
{
	rkVisitor.Visit(*this);
	return;
}

// -----------------------------------------------------------------

/// Remove all spatial probes from the cull node.
void C_PortalCullNode::ClearSpatialProbes()
{
	m_vProbes.Clear();
	return;
}

// -----------------------------------------------------------------

/// Gather the cull node.
void C_PortalCullNode::Gather(const C_View& rkView) const
{
	// Don't think we need to do anything! =)
	return;
}

// -----------------------------------------------------------------

/// Gather the cull node for a multiple view.
void C_PortalCullNode::Gather(const C_MultiView& rkMultiView) const
{
	for(S32 i=0; i<rkMultiView.Size(); ++i)
		Gather(*rkMultiView.GetView(i));
	
	return;
}

// -----------------------------------------------------------------

/// Gather the node for a single view.
void C_PortalCullNode::OnGather(const C_View& rkView) const
{
	Gather(rkView);
	return;
}

// -----------------------------------------------------------------

/// Gather the node for multiple views.
void C_PortalCullNode::OnGather(const C_MultiView& rkMultiView) const
{
	Gather(rkMultiView);
	return;
}
