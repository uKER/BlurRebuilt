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
#include <EditorLib/Dg/DgXtCompNode.h>
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

#include <UserNode/Props/CoverNode/DgCoverNodePropNode.h>
#include <UserNode/Props/CoverNode/RenderCoverNodePropNode.h>

#include <EditorLib/Xt/XtVisualisation.h>

#include <UserNode/Props/CoverNode/RenderCoverNodeMeshes.h>

void RegisterCoverNodePropRenderNodes()
{
	REGISTER_RENDER_NODE(Editor::C_RenderCoverNodePropNode)
}

namespace Editor
{

C_RenderCoverNodePropNode::C_RenderCoverNodePropNode(
	C_ActorScene* pScene, 
	const EditorLib::C_DgXtPropNodeBase* pDgNode)
: I_PropertyNode(pScene, pDgNode)
{
	m_CustomTypeData = E_CustomNodeType_Property;
}

C_RenderCoverNodePropNode::~C_RenderCoverNodePropNode()
{
}

const char*	C_RenderCoverNodePropNode::GetStaticTypeName()
{
	return EditorLib::C_DgCoverNodePropNode::GetStaticTypeName();
}

const char* C_RenderCoverNodePropNode::GetStaticDisplayName()
{
	static char* p = "Cover Node Property";
	return p;
}

const char*	C_RenderCoverNodePropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

EditorLib::E_ObjType C_RenderCoverNodePropNode::GetStaticType()
{
	return EditorLib::C_DgCoverNodePropNode::GetStaticType();
}

void* C_RenderCoverNodePropNode::Create()
{
	return new C_RenderCoverNodePropNode(0,0);
}

void C_RenderCoverNodePropNode::SimpleRendererRegister()
{
	AftkRender::C_MeshClusterResource *pMeshCluster = new AftkRender::C_MeshClusterResource();
	
	pMeshCluster->SetNumVertices(MeshHighWallVertex_Count);
	pMeshCluster->AddVertexDataElement(MeshHighWallVertexPosition, AftkRender::E_ElementType_Position);
	pMeshCluster->AddVertexDataElement(MeshHighWallVertexNormal, AftkRender::E_ElementType_Normal);
	pMeshCluster->AddIndexData(MeshHighWallIndex, MeshHighWallIndex_Count, AftkRender::E_PrimitiveType_TriList, 0);

	Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().SetObjTypePrimitive(
		"ASCN_HW",
		pMeshCluster, 
		Maths::RGBA(0,0,128,255));

	pMeshCluster = new AftkRender::C_MeshClusterResource();
	
	pMeshCluster->SetNumVertices(MeshLowWallVertex_Count);
	pMeshCluster->AddVertexDataElement(MeshLowWallVertexPosition, AftkRender::E_ElementType_Position);
	pMeshCluster->AddVertexDataElement(MeshLowWallVertexNormal, AftkRender::E_ElementType_Normal);
	pMeshCluster->AddIndexData(MeshLowWallIndex, MeshLowWallIndex_Count, AftkRender::E_PrimitiveType_TriList, 0);

	Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().SetObjTypePrimitive(
		"ASCN_LW",
		pMeshCluster, 
		Maths::RGBA(0,0,128,255));

	pMeshCluster = new AftkRender::C_MeshClusterResource();

	pMeshCluster->SetNumVertices(MeshLowStepOutLeftVertex_Count);
	pMeshCluster->AddVertexDataElement(MeshLowStepOutLeftVertexPosition, AftkRender::E_ElementType_Position);
	pMeshCluster->AddVertexDataElement(MeshLowStepOutLeftVertexNormal, AftkRender::E_ElementType_Normal);
	pMeshCluster->AddIndexData(MeshLowStepOutLeftIndex, MeshLowStepOutLeftIndex_Count, AftkRender::E_PrimitiveType_TriList, 0);

	Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().SetObjTypePrimitive(
		"ASCN_LSOL",
		pMeshCluster, 
		Maths::RGBA(0,0,128,255));

	pMeshCluster = new AftkRender::C_MeshClusterResource();

	pMeshCluster->SetNumVertices(MeshLowStepOutRightVertex_Count);
	pMeshCluster->AddVertexDataElement(MeshLowStepOutRightVertexPosition, AftkRender::E_ElementType_Position);
	pMeshCluster->AddVertexDataElement(MeshLowStepOutRightVertexNormal, AftkRender::E_ElementType_Normal);
	pMeshCluster->AddIndexData(MeshLowStepOutRightIndex, MeshLowStepOutRightIndex_Count, AftkRender::E_PrimitiveType_TriList, 0);

	Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().SetObjTypePrimitive(
		"ASCN_LSOR",
		pMeshCluster, 
		Maths::RGBA(0,0,128,255));

	pMeshCluster = new AftkRender::C_MeshClusterResource();

	pMeshCluster->SetNumVertices(MeshHighStepOutLeftVertex_Count);
	pMeshCluster->AddVertexDataElement(MeshHighStepOutLeftVertexPosition, AftkRender::E_ElementType_Position);
	pMeshCluster->AddVertexDataElement(MeshHighStepOutLeftVertexNormal, AftkRender::E_ElementType_Normal);
	pMeshCluster->AddIndexData(MeshHighStepOutLeftIndex, MeshHighStepOutLeftIndex_Count, AftkRender::E_PrimitiveType_TriList, 0);

	Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().SetObjTypePrimitive(
		"ASCN_HSOL",
		pMeshCluster, 
		Maths::RGBA(0,0,128,255));

	pMeshCluster = new AftkRender::C_MeshClusterResource();

	pMeshCluster->SetNumVertices(MeshHighStepOutRightVertex_Count);
	pMeshCluster->AddVertexDataElement(MeshHighStepOutRightVertexPosition, AftkRender::E_ElementType_Position);
	pMeshCluster->AddVertexDataElement(MeshHighStepOutRightVertexNormal, AftkRender::E_ElementType_Normal);
	pMeshCluster->AddIndexData(MeshHighStepOutRightIndex, MeshHighStepOutRightIndex_Count, AftkRender::E_PrimitiveType_TriList, 0);

	Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().SetObjTypePrimitive(
		"ASCN_HSOR",
		pMeshCluster, 
		Maths::RGBA(0,0,128,255));

	pMeshCluster = new AftkRender::C_MeshClusterResource();

	pMeshCluster->SetNumVertices(MeshFireOverVertex_Count);
	pMeshCluster->AddVertexDataElement(MeshFireOverVertexPosition, AftkRender::E_ElementType_Position);
	pMeshCluster->AddVertexDataElement(MeshFireOverVertexNormal, AftkRender::E_ElementType_Normal);
	pMeshCluster->AddIndexData(MeshFireOverIndex, MeshFireOverIndex_Count, AftkRender::E_PrimitiveType_TriList, 0);

	Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().SetObjTypePrimitive(
		"ASCN_FO",
		pMeshCluster, 
		Maths::RGBA(0,0,128,255));
}

void C_RenderCoverNodePropNode::AttachToRenderer(
	AftkRender::E_RendererPass pass)
{
	if (pass == AftkRender::E_RendererPass_2D)
		return;

	assert(m_pParent && m_pParent->GetType()==AftkRender::E_SceneGraphNode_Custom 
			&& m_pParent->GetCustomTypeData()==E_CustomNodeType_Actor);

	if (!Editor::C_PropertyVisibilityManager::Instance().IsVisible((EditorLib::C_DgCoverNodePropNode*)m_pDgNode))
		return;

	const C_DgNodeSelection& selection = C_RenderCore::Instance().GetRenderScene().GetSelection();

	Maths::RGBA highlight(0,0,0,0);
	highlight = RenderHelpers::GetHighlightColour(m_pDgNode, highlight);

	EditorLib::C_DgCoverNodePropNode *pCoverNodeProp = (EditorLib::C_DgCoverNodePropNode*)m_pDgNode;

	Util::C_String primitiveName;
	switch (pCoverNodeProp->GetCoverNodeType())
	{
	case(EditorLib::C_DgCoverNodePropNode::E_CoverNodeType_HighWall):
		primitiveName = "ASCN_HW";
		break;
	case(EditorLib::C_DgCoverNodePropNode::E_CoverNodeType_LowWall):
		primitiveName = "ASCN_LW";
		break;
	case(EditorLib::C_DgCoverNodePropNode::E_CoverNodeType_LowStepOutLeft):
		primitiveName = "ASCN_LSOL";
		break;
	case(EditorLib::C_DgCoverNodePropNode::E_CoverNodeType_HighStepOutLeft):
		primitiveName = "ASCN_HSOL";
		break;
	case(EditorLib::C_DgCoverNodePropNode::E_CoverNodeType_LowStepOutRight):
		primitiveName = "ASCN_LSOR";
		break;
	case(EditorLib::C_DgCoverNodePropNode::E_CoverNodeType_HighStepOutRight):
		primitiveName = "ASCN_HSOR";
		break;
	default:
		primitiveName = "";
		break;
	}

	UpdateScaleAndOffset();
	Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().AddPrimitive(
		primitiveName.Chars(),
		&m_DrawPosition,
		m_WorldAABBox,
		m_pDgNode->GetId(), -1,
		highlight.Colour() ? &highlight : 0);

	if (pCoverNodeProp->GetCoverNodeType() == EditorLib::C_DgCoverNodePropNode::E_CoverNodeType_LowStepOutLeft || pCoverNodeProp->GetCoverNodeType() == EditorLib::C_DgCoverNodePropNode::E_CoverNodeType_LowStepOutRight)
		Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().AddPrimitive(
		"ASCN_FO",
		&m_DrawPosition,
		m_WorldAABBox,
		m_pDgNode->GetId(), -1,
		highlight.Colour() ? &highlight : 0);
}

void C_RenderCoverNodePropNode::OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode)
{
	const EditorLib::C_DgCoverNodePropNode *pProp = static_cast<const EditorLib::C_DgCoverNodePropNode*>(m_pDgNode);
	const EditorLib::I_DgNode *pParentActor = pProp->GetParent()->GetParent();
	Editor::C_ActorScene *pScene = static_cast<Editor::C_ActorScene*>(m_pScene);

	if (pDgNode && pDgNode!=m_pDgNode && pDgNode!=pProp->GetLinkedNode() && pDgNode != pParentActor)
		return;

	__super::OnNodeDeserialised(pDgNode);

	m_pLinkedNode = pScene->Find(pProp->GetLinkedNode());
	UpdateScaleAndOffset();
	UpdateCache();
}

void C_RenderCoverNodePropNode::OnNodeResolved(const EditorLib::I_DgNode* pDgNode)
{
	const EditorLib::C_DgCoverNodePropNode *pProp = static_cast<const EditorLib::C_DgCoverNodePropNode*>(m_pDgNode);
	const EditorLib::I_DgNode *pParentActor = pProp->GetParent()->GetParent();
	Editor::C_ActorScene *pScene = static_cast<Editor::C_ActorScene*>(m_pScene);

	if (pDgNode && pDgNode!=m_pDgNode && pDgNode!=pProp->GetLinkedNode() && pDgNode != pParentActor)
		return;

	__super::OnNodeResolved(pDgNode);

	m_pLinkedNode = pScene->Find(pProp->GetLinkedNode());
	UpdateScaleAndOffset();
	UpdateCache();
}

void C_RenderCoverNodePropNode::UpdateCache() const
{
	AftkRender::C_SceneGraphNode::UpdateCache();

	m_WorldAABBox.SetInvalidExtents();
	{
		const Maths::HVec3 x(3.f, 0.f, 0.f), y(0.f, 3.f, 0.f), z(0.f, 0.f, 3.f);


		Maths::HMat33 worldMatrix(m_DrawPosition);
		{
			m_WorldAABBox.UpdateExtents(Maths::C_Vertex(worldMatrix*-x));
			m_WorldAABBox.UpdateExtents(Maths::C_Vertex(worldMatrix* x));
			m_WorldAABBox.UpdateExtents(Maths::C_Vertex(worldMatrix*-y));
			m_WorldAABBox.UpdateExtents(Maths::C_Vertex(worldMatrix* y));
			m_WorldAABBox.UpdateExtents(Maths::C_Vertex(worldMatrix*-z));
			m_WorldAABBox.UpdateExtents(Maths::C_Vertex(worldMatrix* z));
		}

		m_WorldAABBox.m_Min[0] += m_DrawPosition[3][0];
		m_WorldAABBox.m_Min[1] += m_DrawPosition[3][1];
		m_WorldAABBox.m_Min[2] += m_DrawPosition[3][2];

		m_WorldAABBox.m_Max[0] += m_DrawPosition[3][0];
		m_WorldAABBox.m_Max[1] += m_DrawPosition[3][1];
		m_WorldAABBox.m_Max[2] += m_DrawPosition[3][2];
	}
}

void C_RenderCoverNodePropNode::UpdateScaleAndOffset()
{
	const EditorLib::C_DgCoverNodePropNode *pProp = static_cast<const EditorLib::C_DgCoverNodePropNode*>(m_pDgNode);
	assert(pProp);

	if (pProp->GetCoverNodeType() == EditorLib::C_DgCoverNodePropNode::E_CoverNodeType_LowWall || pProp->GetCoverNodeType() == EditorLib::C_DgCoverNodePropNode::E_CoverNodeType_HighWall)
	{
		if (m_pLinkedNode)
		{
			Maths::HVec4 row;
			const Maths::HMat44& m = m_pLinkedNode->GetWorldMatrix();

			Maths::RotY rotation = atan2(m_WorldMatrix.AW().X()-m.AW().X(), m_WorldMatrix.AW().Z()-m.AW().Z()) - Maths::Constants<F32>::Pi()/2;
			Maths::HMat44 rot( rotation );

			m_DrawPosition = rot;

			row.X(m_WorldMatrix.AW().X() - (m_WorldMatrix.AW().X()-m.AW().X())/2);
			row.Y(m_WorldMatrix.AW().Y() - (m_WorldMatrix.AW().Y()-m.AW().Y())/2);
			row.Z(m_WorldMatrix.AW().Z() - (m_WorldMatrix.AW().Z()-m.AW().Z())/2);
			row.W(1);
			m_DrawPosition.AW(row);
		} else {
			m_DrawPosition = m_WorldMatrix;
		}
	} else {
		m_DrawPosition = m_WorldMatrix;
	}
}

}
