#include <nexus_render/stdafx.h>

using namespace NexusRender;

RenderNonAmbientLightNode::RenderNonAmbientLightNode(
	SceneRenderer* pSceneRenderer, 
	Aftk::C_SceneNode* pAftkSceneNode) 
: RenderLightNode(
	pSceneRenderer,
	pAftkSceneNode)
{
}

RenderNonAmbientLightNode::~RenderNonAmbientLightNode()
{
}

void RenderNonAmbientLightNode::Clear()
{
	__super::Clear();
}

RenderNode::ERenderNodeType RenderNonAmbientLightNode::GetType() const
{
	return RenderNode::RENDER_NODE_TYPE_NON_AMBIENT_LIGHT;
}

bool RenderNonAmbientLightNode::HasType(RenderNode::ERenderNodeType type) const
{
	if (__super::HasType(type))
		return true;
	if (type == RenderNonAmbientLightNode::GetType()) 
		return true;

	return false;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

RenderNonAmbientLightNode::RenderNonAmbientLightNode()
{
}

#endif
