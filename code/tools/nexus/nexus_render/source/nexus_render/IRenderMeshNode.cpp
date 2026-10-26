#include <nexus_render/stdafx.h>
#include <tools_common/Utility.h>

namespace NexusRender
{
	char* g_effectModeAsString[IRenderMeshNode::EFFECT_MODE_COUNT] = 
	{
		"Static",
		"Skinned",
		"BonedInstance"
	};
}

using namespace NexusRender;

//Initialise statics.
char** IRenderMeshNode::m_ppEffectModeAsString = g_effectModeAsString;

IRenderMeshNode::IRenderMeshNode(
	SceneRenderer* pSceneRenderer, 
	Aftk::C_SceneNode* pAftkNode)
	: RenderNode(pSceneRenderer, pAftkNode)
{
}

IRenderMeshNode::~IRenderMeshNode()
{
}

void IRenderMeshNode::Clear()
{
}

RenderNode::ERenderNodeType IRenderMeshNode::GetType() const
{
	return RenderNode::RENDER_NODE_TYPE_IMESH;
}

bool IRenderMeshNode::HasType(RenderNode::ERenderNodeType type) const
{
	if (__super::HasType(type))
		return true;
	if (type == IRenderMeshNode::GetType()) 
		return true;

	return false;
}

