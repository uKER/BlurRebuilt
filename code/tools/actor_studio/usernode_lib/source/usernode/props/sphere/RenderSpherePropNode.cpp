#include "stdafx.h"

#include <Aftk/AftkEnum.h>

// aftk rendering
#include <AftkRender.h>

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>
#include <EditorLib/Core.h>

#include <EditorApp/Editor/ActorSceneNode.h>
#include <EditorApp/Editor/IActorPropertyNode.h>
#include <EditorApp/Editor/ActorScene.h>
#include <EditorApp/Editor/Primitives.h>
#include <EditorApp/Editor/EditorRenderer.h>
#include <EditorApp/Editor/SimpleRenderer.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderPipeline.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/PropertyVisibilityManager.h>
#include <EditorApp/Editor/RenderHelpers.h>
#include <EditorApp/Misc/MathsHelpers.h>

#include <UserNode/Props/Sphere/DgSpherePropNode.h>
#include <UserNode/Props/Sphere/RenderSpherePropNode.h>

void RegisterSpherePropRenderNodes()
{
	REGISTER_RENDER_NODE(Editor::C_RenderSpherePropNode)
}

namespace Editor
{

C_RenderSpherePropNode::C_RenderSpherePropNode(
	C_ActorScene* pScene, 
	const EditorLib::C_DgXtPropNodeBase* pDgNode)
: I_PropertyNode(pScene, pDgNode)
{
	m_CustomTypeData = E_CustomNodeType_Property;
}

C_RenderSpherePropNode::~C_RenderSpherePropNode()
{
}

const char*	C_RenderSpherePropNode::GetStaticTypeName()
{
	return EditorLib::C_DgSpherePropNode::GetStaticTypeName();
}

const char* C_RenderSpherePropNode::GetStaticDisplayName()
{
	static char* p = "Sphere Property";
	return p;
}

const char*	C_RenderSpherePropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

EditorLib::E_ObjType C_RenderSpherePropNode::GetStaticType()
{
	return EditorLib::C_DgSpherePropNode::GetStaticType();
}

void* C_RenderSpherePropNode::Create()
{
	return new C_RenderSpherePropNode(0,0);
}

void C_RenderSpherePropNode::SimpleRendererRegister()
{
	Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().SetObjTypePrimitive(
		EditorLib::C_DgSpherePropNode::GetStaticTypeName(), 
		Primitive::SphereWireframe(1.f,12,6), 
		Maths::RGBA(255,0,0,255));
}

void C_RenderSpherePropNode::AttachToRenderer(
	AftkRender::E_RendererPass pass)
{
	if (pass == AftkRender::E_RendererPass_2D)
		return;

	assert(m_pParent && m_pParent->GetType()==AftkRender::E_SceneGraphNode_Custom 
			&& m_pParent->GetCustomTypeData()==E_CustomNodeType_Actor);

	if (!Editor::C_PropertyVisibilityManager::Instance().IsVisible((EditorLib::C_DgSpherePropNode*)m_pDgNode))
		return;

	Maths::RGBA highlight(0,0,0,0);
	highlight = RenderHelpers::GetHighlightColour(m_pDgNode, highlight);

	Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().AddPrimitive(
		EditorLib::C_DgSpherePropNode::GetStaticTypeName(),
		&m_WorldMatrix, 
		m_WorldAABBox,
		m_pDgNode->GetId(), 
		-1, 
		highlight.Colour() ? &highlight : 0);
}

void C_RenderSpherePropNode::OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode)
{
	if (pDgNode && pDgNode != m_pDgNode)
		return;

	__super::OnNodeDeserialised(pDgNode);

	EditorLib::C_DgSpherePropNode *pSphereProp = (EditorLib::C_DgSpherePropNode*) m_pDgNode;
	F32 radius;
	pSphereProp->GetRadiusPlug().GetValue(radius);
	m_LocalMatrix[0][0] = m_LocalMatrix[1][1] = m_LocalMatrix[2][2] = radius;

	EditorLib::Vector3 pos;
	EditorLib::PlugHelpers::GetCompoundValue(pSphereProp->GetPositionPlug(), pos);
	m_LocalMatrix[3][0] = pos[0];
	m_LocalMatrix[3][1] = pos[1];
	m_LocalMatrix[3][2] = pos[2];
	UpdateCache();
}

void C_RenderSpherePropNode::UpdateCache() const
{
	AftkRender::C_SceneGraphNode::UpdateCache();

	m_WorldAABBox.SetInvalidExtents();
	{
		const Maths::HVec3 x(1.f, 0.f, 0.f), y(0.f, 1.f, 0.f), z(0.f, 0.f, 1.f);

		Maths::HMat33 worldMatrix(m_WorldMatrix);
		{
			m_WorldAABBox.UpdateExtents(Maths::C_Vertex(worldMatrix *-x));
			m_WorldAABBox.UpdateExtents(Maths::C_Vertex(worldMatrix * x));
			m_WorldAABBox.UpdateExtents(Maths::C_Vertex(worldMatrix *-y));
			m_WorldAABBox.UpdateExtents(Maths::C_Vertex(worldMatrix * y));
			m_WorldAABBox.UpdateExtents(Maths::C_Vertex(worldMatrix *-z));
			m_WorldAABBox.UpdateExtents(Maths::C_Vertex(worldMatrix * z));
		}

		m_WorldAABBox.m_Min[0] += m_WorldMatrix[3][0];
		m_WorldAABBox.m_Min[1] += m_WorldMatrix[3][1];
		m_WorldAABBox.m_Min[2] += m_WorldMatrix[3][2];

		m_WorldAABBox.m_Max[0] += m_WorldMatrix[3][0];
		m_WorldAABBox.m_Max[1] += m_WorldMatrix[3][1];
		m_WorldAABBox.m_Max[2] += m_WorldMatrix[3][2];
	}
}


}
