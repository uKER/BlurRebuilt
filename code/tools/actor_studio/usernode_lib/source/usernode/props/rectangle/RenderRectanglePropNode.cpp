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

#include <UserNode/Props/Rectangle/DgRectanglePropNode.h>
#include <UserNode/Props/Rectangle/RenderRectanglePropNode.h>

void RegisterRectanglePropRenderNodes()
{
	REGISTER_RENDER_NODE(Editor::C_RenderRectanglePropNode)
}

namespace Editor
{

C_RenderRectanglePropNode::C_RenderRectanglePropNode(
	C_ActorScene* pScene, 
	const EditorLib::C_DgXtPropNodeBase* pDgNode)
: I_PropertyNode(pScene, pDgNode)
{
	m_CustomTypeData = E_CustomNodeType_Property;
}

C_RenderRectanglePropNode::~C_RenderRectanglePropNode()
{
}

const char*	C_RenderRectanglePropNode::GetStaticTypeName()
{
	return EditorLib::C_DgRectanglePropNode::GetStaticTypeName();
}

const char* C_RenderRectanglePropNode::GetStaticDisplayName()
{
	static char* p = "Rectangle Property";
	return p;
}

const char*	C_RenderRectanglePropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

EditorLib::E_ObjType C_RenderRectanglePropNode::GetStaticType()
{
	return EditorLib::C_DgRectanglePropNode::GetStaticType();
}

void* C_RenderRectanglePropNode::Create()
{
	return new C_RenderRectanglePropNode(0,0);
}

void C_RenderRectanglePropNode::SimpleRendererRegister()
{
	Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().SetObjTypePrimitive(
		EditorLib::C_DgRectanglePropNode::GetStaticTypeName(),
		Editor::Primitive::Rectangle(1.0f, 1.0f), 
		Maths::RGBA(255,0,0,255));
}

void C_RenderRectanglePropNode::AttachToRenderer(
	AftkRender::E_RendererPass pass)
{
	if (pass == AftkRender::E_RendererPass_2D)
		return;

	assert(m_pParent && m_pParent->GetType()==AftkRender::E_SceneGraphNode_Custom 
			&& m_pParent->GetCustomTypeData()==E_CustomNodeType_Actor);

	if (!Editor::C_PropertyVisibilityManager::Instance().IsVisible((EditorLib::C_DgRectanglePropNode*)m_pDgNode))
		return;

	Maths::RGBA highlight(0,0,0,0);
	highlight = RenderHelpers::GetHighlightColour(m_pDgNode, highlight);

	Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().AddPrimitive(
		EditorLib::C_DgRectanglePropNode::GetStaticTypeName(),
		&m_WorldMatrix, 
		m_WorldAABBox, 
		m_pDgNode->GetId(), 
		-1, 
		highlight.Colour() ? &highlight : 0);
}

void C_RenderRectanglePropNode::OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode)
{
	if (pDgNode && pDgNode!=m_pDgNode)
		return;

	__super::OnNodeDeserialised(pDgNode);

	UpdateScaleAndOffset();
}

void C_RenderRectanglePropNode::OnNodeResolved(const EditorLib::I_DgNode* pDgNode)
{
	if (pDgNode && pDgNode!=m_pDgNode)
		return;

	__super::OnNodeResolved(pDgNode);

	UpdateScaleAndOffset();
}

void C_RenderRectanglePropNode::UpdateCache() const
{
	AftkRender::C_SceneGraphNode::UpdateCache();

	m_WorldAABBox.SetInvalidExtents();
	{
		const Maths::C_AxisBox *pBox = 
			Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().GetPrimitiveBounds(EditorLib::C_DgRectanglePropNode::GetStaticTypeName());
			   		
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

void C_RenderRectanglePropNode::UpdateScaleAndOffset()
{	
	EditorLib::C_DgRectanglePropNode *pRectangleProp = (EditorLib::C_DgRectanglePropNode*) m_pDgNode;

	F32 width;
	pRectangleProp->GetWidthPlug().GetValue(width);

	F32 height;
	pRectangleProp->GetHeightPlug().GetValue(height);

	m_LocalMatrix[0][0] = width;
	m_LocalMatrix[1][1] = height;
	m_LocalMatrix[2][2] = 0;

	m_LocalMatrix[3][0] = 0;
	m_LocalMatrix[3][1] = 0;
	m_LocalMatrix[3][2] = 0;

	UpdateCache();
}

}
