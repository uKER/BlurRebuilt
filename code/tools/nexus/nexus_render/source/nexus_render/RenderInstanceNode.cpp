#include <nexus_render/stdafx.h>
#include <nexus_render/Utility.h>

using namespace NexusRender;

RenderInstanceNode::RenderInstanceNode(
	SceneRenderer* pSceneRenderer, 
	Aftk::C_SceneNode* pAftkSceneNode)
: RenderNode(
	pSceneRenderer,
	pAftkSceneNode)
{
	m_pSource = 0;
}

RenderInstanceNode::~RenderInstanceNode()
{
}

Tu::Status RenderInstanceNode::Rebuild()
{
	m_nodeFlags.isBuilt = true;
	return Tu::Status::SUCCESS;
}

Tu::Status RenderInstanceNode::Evaluate()
{
	return Tu::Status::SUCCESS;
}

Tu::Status RenderInstanceNode::Update(
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
	RenderNode* pSource = Utility::GetIndirectSourcePtr(m_pSource);
	if (!pSource)
		m_nodeFlags.isHidden = (!m_pSceneRenderer->GetSettingsPtr()->renderInstances);
	else
		//Also look at our base instance to determine visibility. Remmember that instance nodes are processed
		//after their base instances.
		m_nodeFlags.isHidden = 
		(!m_pSceneRenderer->GetSettingsPtr()->renderInstances) ||
		pSource->GetNodeFlags().isHidden;
		
	return Tu::Status::SUCCESS;
}

void RenderInstanceNode::SetSourcePtr(RenderNode* pSource)
{
	m_pSource = pSource;
}

RenderNode* RenderInstanceNode::GetSourcePtr()
{
	return m_pSource;
}

const RenderNode* RenderInstanceNode::GetSourcePtr() const
{
	return m_pSource;
}

RenderNode::ERenderNodeType RenderInstanceNode::GetType() const
{
	return RenderNode::RENDER_NODE_TYPE_INSTANCE;
}

bool RenderInstanceNode::HasType(RenderNode::ERenderNodeType type) const
{
	if (__super::HasType(type))
		return true;
	if (type == RenderInstanceNode::GetType()) 
		return true;

	return false;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

RenderInstanceNode::RenderInstanceNode()
{
}

void RenderInstanceNode::SetActiveLodLevel()
{
	//This node type will never instance a mesh, which is currently the only node type to have
	//internal levels of detail. Therefore we don't need to do anything here.
}


#endif
