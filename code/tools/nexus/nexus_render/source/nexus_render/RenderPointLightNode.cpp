#include <nexus_render/stdafx.h>

using namespace NexusRender;

RenderPointLightNode::RenderPointLightNode(
	SceneRenderer* pSceneRenderer, 
	Aftk::C_SceneNode* pAftkSceneNode)
: RenderNonAmbientLightNode(
	pSceneRenderer,
	pAftkSceneNode)
{
}

RenderPointLightNode::~RenderPointLightNode()
{
}

void RenderPointLightNode::Clear()
{
	__super::Clear();
}

Tu::Status RenderPointLightNode::Rebuild()
{
	m_nodeFlags.isBuilt = true;
	return Tu::Status::SUCCESS;
}

Tu::Status RenderPointLightNode::Update(
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
	m_nodeFlags.isHidden = !m_pSceneRenderer->GetSettingsPtr()->renderPointLights || !m_pSceneRenderer->GetSettingsPtr()->renderLights;
	return Tu::Status::SUCCESS;
}

Tu::Status RenderPointLightNode::Evaluate()
{
	return Tu::Status::SUCCESS;
}

Tu::Status RenderPointLightNode::Draw()
{
/*
	Maths::HMat44 widgetMatrix = GizmoMatrix(worldMatrix);
	m_pSceneRenderer->D3DWindowPtr()->D3DDevicePtr()->SetTransform(D3DTS_WORLD, (const D3DMATRIX*)&widgetMatrix);

	Tu::Status status = m_widget.Draw();
	if (!status)
	{
		return status;
	}
*/
	return Tu::Status::SUCCESS;
}

RenderNode::ERenderNodeType RenderPointLightNode::GetType() const
{
	return RenderNode::RENDER_NODE_TYPE_POINT_LIGHT;
}

bool RenderPointLightNode::HasType(RenderNode::ERenderNodeType type) const
{
	if (type == RenderPointLightNode::GetType()) 
		return true;
	return false;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

RenderPointLightNode::RenderPointLightNode()
{
}

#endif
