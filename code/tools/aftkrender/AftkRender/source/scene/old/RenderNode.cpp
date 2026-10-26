#include "stdafx.h"
#include "AftkRender.h"

using namespace AftkRender;

__forceinline Maths::HMat44 ConvertToHMat44(
	const Aftk::Matrix44& rhs)
{
	return Maths::HMat44(
		Maths::HVec4(rhs[0][0], rhs[0][1], rhs[0][2], rhs[0][3]),
		Maths::HVec4(rhs[1][0], rhs[1][1], rhs[1][2], rhs[1][3]),
		Maths::HVec4(rhs[2][0], rhs[2][1], rhs[2][2], rhs[2][3]),
		Maths::HVec4(rhs[3][0], rhs[3][1], rhs[3][2], rhs[3][3]));
}

	__forceinline Maths::HVec3 ConvertToHVec3( 
		const UtMaths::MPoint< UtMaths::VectorXYZ<F32> >& rhs)
	{
		return Maths::HVec3(rhs[0], rhs[1], rhs[2]);
	}


C_RenderNode::C_RenderNode(
	Aftk::C_SceneNode* pAftkSceneNode,
	C_SceneRenderer *pRenderer)
:
	m_pAftkSceneNode(pAftkSceneNode),
	m_pRenderer(pRenderer)
{
	m_LocalMatrix = ConvertToHMat44(m_pAftkSceneNode->GetLocalMatrix());
	m_WorldMatrix = ConvertToHMat44(m_pAftkSceneNode->GetWorldMatrix());
	m_WorldInverseTransposeMatrix = ConvertToHMat44(m_pAftkSceneNode->GetWorldMatrixInverseTranspose());

	const Aftk::C_Bounds& sceneNodeBounds = m_pAftkSceneNode->GetWorldBounds();

	m_WorldAABBox.UpdateExtents(ConvertToHVec3(sceneNodeBounds.GetMin()));
	m_WorldAABBox.UpdateExtents(ConvertToHVec3(sceneNodeBounds.GetMax()));

	memset(&m_RenderNodeFlags, 0, sizeof(C_RenderNode::C_Flags));
}

C_RenderNode::~C_RenderNode()
{
}

E_RenderNodeType C_RenderNode::GetType() const
{
	return E_RenderNodeType_Node;
}

bool C_RenderNode::HasType(
	E_RenderNodeType type) const
{
	return (type == GetType());
}

const Aftk::C_SceneNode* C_RenderNode::GetAftkSceneNodePtr() const
{
	return m_pAftkSceneNode;
}

Aftk::C_SceneNode* C_RenderNode::GetAftkSceneNodePtr()
{
	return m_pAftkSceneNode;
}

U32 C_RenderNode::GetNumChildren() const
{
	return m_Children.Size();
}

const C_RenderNode* C_RenderNode::GetChildPtr(
	U32 child) const
{
	if (child > (U32) m_Children.Size())
		return 0;
	else
		return m_Children[child];
}

void C_RenderNode::AddChild(
	C_RenderNode* pNode)
{
	pNode->m_pParent = this;
	m_Children.PushBack(pNode);
}

C_SceneRenderer* C_RenderNode::GetRenderer() const
{
	return m_pRenderer;
}

bool C_RenderNode::IsSkinned() const
{
	return m_RenderNodeFlags.isSkinned;
}

bool C_RenderNode::IsSkinAnimated() const
{
	return m_RenderNodeFlags.isSkinAnimated;
}