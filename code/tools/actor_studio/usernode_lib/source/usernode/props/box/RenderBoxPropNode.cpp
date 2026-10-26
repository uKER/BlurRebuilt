#include "stdafx.h"

#include <Aftk/AftkEnum.h>

// aftk rendering
#include <AftkRender.h>

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/SceneXtActorNode.h>
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
#include <EditorApp/Editor/DgNodeSelection.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderPipeline.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/PropertyVisibilityManager.h>
#include <EditorApp/Editor/RenderHelpers.h>
#include <EditorApp/Misc/MathsHelpers.h>

#include <UserNode/Props/Box/DgBoxPropNode.h>
#include <UserNode/Props/Box/RenderBoxPropNode.h>

void RegisterBoxPropRenderNodes()
{
	REGISTER_RENDER_NODE(Editor::C_RenderBoxPropNode)
}

namespace Editor
{

C_RenderBoxPropNode::C_RenderBoxPropNode(
	C_ActorScene* pScene, 
	const EditorLib::C_DgXtPropNodeBase* pDgNode)
: I_PropertyNode(pScene, pDgNode)
{
	m_CustomTypeData = E_CustomNodeType_Property;
}

C_RenderBoxPropNode::~C_RenderBoxPropNode()
{
}

const char*	C_RenderBoxPropNode::GetStaticTypeName()
{
	return EditorLib::C_DgBoxPropNode::GetStaticTypeName();
}

const char* C_RenderBoxPropNode::GetStaticDisplayName()
{
	static char* p = "Box Property";
	return p;
}

const char*	C_RenderBoxPropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

EditorLib::E_ObjType C_RenderBoxPropNode::GetStaticType()
{
	return EditorLib::C_DgBoxPropNode::GetStaticType();
}

void* C_RenderBoxPropNode::Create()
{
	return new C_RenderBoxPropNode(0,0);
}

void C_RenderBoxPropNode::SimpleRendererRegister()
{
	Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().SetObjTypePrimitive(
		EditorLib::C_DgBoxPropNode::GetStaticTypeName(),
		Editor::Primitive::BoxOutline(1.f, 1.f, 1.f), 
		Maths::RGBA(255,0,0,255));
}

void C_RenderBoxPropNode::AttachToRenderer(
	AftkRender::E_RendererPass pass)
{
	if (pass == AftkRender::E_RendererPass_2D)
		return;

	assert(m_pParent && m_pParent->GetType()==AftkRender::E_SceneGraphNode_Custom 
			&& m_pParent->GetCustomTypeData()==E_CustomNodeType_Actor);

	if (!Editor::C_PropertyVisibilityManager::Instance().IsVisible((EditorLib::C_DgBoxPropNode*)m_pDgNode))
		return;

	Maths::RGBA highlight(0,0,0,0);
	highlight = RenderHelpers::GetHighlightColour(m_pDgNode, highlight);

	Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().AddPrimitive(
		EditorLib::C_DgBoxPropNode::GetStaticTypeName(),
		&m_WorldMatrix, 
		m_WorldAABBox, 
		m_pDgNode->GetId(), 
		-1, 
		highlight.Colour() ? &highlight : 0);
}

void C_RenderBoxPropNode::OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode)
{
	if (pDgNode && pDgNode!=m_pDgNode)
		return;

	__super::OnNodeDeserialised(pDgNode);

	UpdateScaleAndOffset();
}

void C_RenderBoxPropNode::OnNodeResolved(const EditorLib::I_DgNode* pDgNode)
{
	if (pDgNode && pDgNode!=m_pDgNode)
		return;

	__super::OnNodeResolved(pDgNode);

	UpdateScaleAndOffset();
}

void C_RenderBoxPropNode::UpdateCache() const
{
	AftkRender::C_SceneGraphNode::UpdateCache();

	m_WorldAABBox.SetInvalidExtents();
	{
		const Maths::C_AxisBox *pBox = 
			Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().GetPrimitiveBounds(
				EditorLib::C_DgBoxPropNode::GetStaticTypeName());

		Maths::HVec3 p1 = (const Maths::HVec3&)(Maths::HVec4(pBox->m_Min[0],pBox->m_Min[1],pBox->m_Min[2],1.f) * m_WorldMatrix);
		Maths::HVec3 p2 = (const Maths::HVec3&)(Maths::HVec4(pBox->m_Min[0],pBox->m_Min[1],pBox->m_Max[2],1.f) * m_WorldMatrix);
		Maths::HVec3 p3 = (const Maths::HVec3&)(Maths::HVec4(pBox->m_Min[0],pBox->m_Max[1],pBox->m_Min[2],1.f) * m_WorldMatrix);
		Maths::HVec3 p4 = (const Maths::HVec3&)(Maths::HVec4(pBox->m_Min[0],pBox->m_Max[1],pBox->m_Max[2],1.f) * m_WorldMatrix);
		Maths::HVec3 p5 = (const Maths::HVec3&)(Maths::HVec4(pBox->m_Max[0],pBox->m_Min[1],pBox->m_Min[2],1.f) * m_WorldMatrix);
		Maths::HVec3 p6 = (const Maths::HVec3&)(Maths::HVec4(pBox->m_Max[0],pBox->m_Min[1],pBox->m_Max[2],1.f) * m_WorldMatrix);
		Maths::HVec3 p7 = (const Maths::HVec3&)(Maths::HVec4(pBox->m_Max[0],pBox->m_Max[1],pBox->m_Min[2],1.f) * m_WorldMatrix);
		Maths::HVec3 p8 = (const Maths::HVec3&)(Maths::HVec4(pBox->m_Max[0],pBox->m_Max[1],pBox->m_Max[2],1.f) * m_WorldMatrix);
		
		m_WorldAABBox.UpdateExtents(p1);
		m_WorldAABBox.UpdateExtents(p2);
		m_WorldAABBox.UpdateExtents(p3);
		m_WorldAABBox.UpdateExtents(p4);
		m_WorldAABBox.UpdateExtents(p5);
		m_WorldAABBox.UpdateExtents(p6);
		m_WorldAABBox.UpdateExtents(p7);
		m_WorldAABBox.UpdateExtents(p8);
	}
}

void C_RenderBoxPropNode::UpdateScaleAndOffset()
{
	EditorLib::C_DgBoxPropNode *pBoxProp = (EditorLib::C_DgBoxPropNode*) m_pDgNode;
	m_LocalMatrix = ConvertToHMat44(pBoxProp->GetLocalMatrix());

	EditorLib::Vector3 size;
	EditorLib::PlugHelpers::GetCompoundValue(pBoxProp->GetSizePlug(), size);

	//scale & offset unit cube (centred at origin)
	m_LocalMatrix[0][0] *= size[0];
	m_LocalMatrix[0][1] *= size[0];
	m_LocalMatrix[0][2] *= size[0];
	m_LocalMatrix[1][0] *= size[1];
	m_LocalMatrix[1][1] *= size[1];
	m_LocalMatrix[1][2] *= size[1];		
	m_LocalMatrix[2][0] *= size[2];
	m_LocalMatrix[2][1] *= size[2];
	m_LocalMatrix[2][2] *= size[2];
	m_LocalMatrix[3][0] = m_LocalMatrix[0][0]*0.5f + m_LocalMatrix[1][0]*0.5f + m_LocalMatrix[2][0]*0.5f;
	m_LocalMatrix[3][1] = m_LocalMatrix[0][1]*0.5f + m_LocalMatrix[1][1]*0.5f + m_LocalMatrix[2][1]*0.5f;
	m_LocalMatrix[3][2] = m_LocalMatrix[0][2]*0.5f + m_LocalMatrix[1][2]*0.5f + m_LocalMatrix[2][2]*0.5f;

	UpdateCache();
}

}
