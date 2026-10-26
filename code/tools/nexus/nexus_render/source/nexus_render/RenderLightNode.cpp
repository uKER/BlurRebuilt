#include <nexus_render/stdafx.h>

using namespace NexusRender;

RenderLightNode::RenderLightNode(
	SceneRenderer* pSceneRenderer, 
	Aftk::C_SceneNode* pAftkSceneNode)
: RenderNode(
	pSceneRenderer,
	pAftkSceneNode)
{
}

void RenderLightNode::Clear()
{
	__super::Clear();
}

RenderLightNode::~RenderLightNode()
{
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

RenderLightNode::RenderLightNode()
{
}

#endif
