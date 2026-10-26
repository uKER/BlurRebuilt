#include <nexus_render/stdafx.h>

using namespace NexusRender;

RenderDirectionalLightNode::RenderDirectionalLightNode(
	SceneRenderer* pSceneRenderer, 
	Aftk::C_SceneNode* pAftkSceneNode)
: RenderNonAmbientLightNode(
	pSceneRenderer,
	pAftkSceneNode)
{
}

RenderDirectionalLightNode::~RenderDirectionalLightNode()
{
}

void RenderDirectionalLightNode::Clear()
{
	__super::Clear();
}

Tu::Status RenderDirectionalLightNode::Rebuild()
{
	m_nodeFlags.isBuilt = true;
	return Tu::Status::SUCCESS;
}

Tu::Status RenderDirectionalLightNode::Evaluate()
{
	return Tu::Status::SUCCESS;
}

Tu::Status RenderDirectionalLightNode::Update(
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
	m_nodeFlags.isHidden = !m_pSceneRenderer->GetSettingsPtr()->renderDirectionalLights || !m_pSceneRenderer->GetSettingsPtr()->renderLights;
	return Tu::Status::SUCCESS;
}

Tu::Status RenderDirectionalLightNode::Draw()
{
	return Tu::Status::SUCCESS;
}

RenderNode::ERenderNodeType RenderDirectionalLightNode::GetType() const
{
	return RenderNode::RENDER_NODE_TYPE_DIRECTIONAL_LIGHT;
}

bool RenderDirectionalLightNode::HasType(RenderNode::ERenderNodeType type) const
{
	if (__super::HasType(type))
		return true;
	if (type == RenderDirectionalLightNode::GetType()) 
		return true;

	return false;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

RenderDirectionalLightNode::RenderDirectionalLightNode()
{
}

#endif
