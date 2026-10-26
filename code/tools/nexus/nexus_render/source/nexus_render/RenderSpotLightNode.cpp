#include <nexus_render/stdafx.h>

using namespace NexusRender;

RenderSpotLightNode::RenderSpotLightNode(
	SceneRenderer* pSceneRenderer, 
	Aftk::C_SceneNode* pAftkSceneNode)
: RenderNonAmbientLightNode(
	pSceneRenderer,
	pAftkSceneNode)
{
}

RenderSpotLightNode::~RenderSpotLightNode()
{
}

Tu::Status RenderSpotLightNode::Rebuild()
{
	m_nodeFlags.isBuilt = true;
	return Tu::Status::SUCCESS;
}

Tu::Status RenderSpotLightNode::Update(
	bool timeChanged,
	bool scaleChanged,
	bool skinningModeChanged) 
{
	Tu::Status status;
	status = RenderNode::Update(
		timeChanged,
		scaleChanged,
		skinningModeChanged);
	if (!status) return status;

	//Determine if we're hidden or not by node type.
	m_nodeFlags.isHidden = !m_pSceneRenderer->GetSettingsPtr()->renderSpotLights || !m_pSceneRenderer->GetSettingsPtr()->renderLights;
	return Tu::Status::SUCCESS;
}

Tu::Status RenderSpotLightNode::Evaluate()
{
	return Tu::Status::SUCCESS;
}

Tu::Status RenderSpotLightNode::Draw()
{
	return Tu::Status::SUCCESS;
}

RenderNode::ERenderNodeType RenderSpotLightNode::GetType() const
{
	return RenderNode::RENDER_NODE_TYPE_SPOT_LIGHT;
}

bool RenderSpotLightNode::HasType(RenderNode::ERenderNodeType type) const
{
	if (__super::HasType(type))
		return true;
	if (type == RenderSpotLightNode::GetType()) 
		return true;

	return false;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

RenderSpotLightNode::RenderSpotLightNode()
{
}

#endif
