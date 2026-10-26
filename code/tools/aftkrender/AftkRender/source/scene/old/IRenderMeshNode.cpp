#include "stdafx.h"
#include "AftkRender.h"

using namespace AftkRender;

I_RenderMeshNode::I_RenderMeshNode(
	Aftk::C_SceneNode *pAftkSceneNode,
	C_SceneRenderer *pRenderer)
:	C_RenderNode(pAftkSceneNode, pRenderer)
{
}

I_RenderMeshNode::~I_RenderMeshNode()
{
}

void I_RenderMeshNode::Clear()
{
}

E_RenderNodeType I_RenderMeshNode::GetType() const
{
	return E_RenderNodeType_Mesh_Interface;
}

bool I_RenderMeshNode::HasType(E_RenderNodeType type) const
{
	return (C_RenderNode::HasType(type) || (type == I_RenderMeshNode::GetType()));
}