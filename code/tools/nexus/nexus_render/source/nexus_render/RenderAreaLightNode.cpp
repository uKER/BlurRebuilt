#include <nexus_render/stdafx.h>

using namespace NexusRender;

RenderAreaLightNode::RenderAreaLightNode(
	SceneRenderer* pSceneRenderer, 
	Aftk::C_SceneNode* pAftkSceneNode)
: RenderNonAmbientLightNode(
	pSceneRenderer,
	pAftkSceneNode)
{
}

RenderAreaLightNode::~RenderAreaLightNode()
{
}

void RenderAreaLightNode::Clear()
{
	__super::Clear();
}

Tu::Status RenderAreaLightNode::Rebuild()
{
	m_nodeFlags.isBuilt = true;
	return Tu::Status::SUCCESS;
}

Tu::Status RenderAreaLightNode::Update(
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
	m_nodeFlags.isHidden = !m_pSceneRenderer->GetSettingsPtr()->renderAreaLights || !m_pSceneRenderer->GetSettingsPtr()->renderLights;
	return Tu::Status::SUCCESS;
}

Tu::Status RenderAreaLightNode::Evaluate()
{
	return Tu::Status::SUCCESS;
}

Tu::Status RenderAreaLightNode::Draw()
{
	return Tu::Status::SUCCESS;
}

RenderNode::ERenderNodeType RenderAreaLightNode::GetType() const
{
	return RenderNode::RENDER_NODE_TYPE_POINT_LIGHT;
}

bool RenderAreaLightNode::HasType(RenderNode::ERenderNodeType type) const
{
	if (type == RenderAreaLightNode::GetType()) 
		return true;
	return false;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#endif
