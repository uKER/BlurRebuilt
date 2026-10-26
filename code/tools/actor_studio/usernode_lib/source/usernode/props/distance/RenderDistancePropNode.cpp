#include "stdafx.h"

#include <Aftk/AftkEnum.h>

// aftk rendering
#include <AftkRender.h>

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Core.h>

#include <EditorApp/Editor/ActorSceneNode.h>
#include <EditorApp/Editor/IActorPropertyNode.h>
#include <EditorApp/Editor/ActorScene.h>
#include <EditorApp/Editor/Primitives.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderPipeline.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/PropertyVisibilityManager.h>
#include <EditorApp/Editor/RenderHelpers.h>
#include <EditorApp/Editor/EditorRenderer.h>
#include <EditorApp/Editor/VertexRenderer.h>
#include <EditorApp/Misc/MathsHelpers.h>
#include <EditorApp/Editor/DgNodeSelection.h>
#include <EditorApp/Editor/RenderDefinitions.h>

#include <UserNode/Props/Distance/DgDistancePropNode.h>
#include <UserNode/Props/Distance/RenderDistancePropNode.h>

#include <Display/Buffer/VertexDeclarations.h>

void RegisterDistancePropRenderNodes()
{
	REGISTER_RENDER_NODE(Editor::C_RenderDistancePropNode)
}

namespace Editor
{
const F32 kSideHeight = .8f;

C_RenderDistancePropNode::C_RenderDistancePropNode(
	C_ActorScene* pScene, 
	const EditorLib::C_DgXtPropNodeBase* pDgNode)
: I_PropertyNode(pScene, pDgNode)
{
	m_linkedNodes[0] = 0;
	m_linkedNodes[1] = 0;
	m_CustomTypeData = E_CustomNodeType_Property;
}

C_RenderDistancePropNode::~C_RenderDistancePropNode()
{
}

const char*	C_RenderDistancePropNode::GetStaticTypeName()
{
	return EditorLib::C_DgDistancePropNode::GetStaticTypeName();
}

const char* C_RenderDistancePropNode::GetStaticDisplayName()
{
	static char* p = "Distance Property";
	return p;
}

const char*	C_RenderDistancePropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

EditorLib::E_ObjType C_RenderDistancePropNode::GetStaticType()
{
	return EditorLib::C_DgDistancePropNode::GetStaticType();
}

void* C_RenderDistancePropNode::Create()
{
	return new C_RenderDistancePropNode(0,0);
}

void C_RenderDistancePropNode::SimpleRendererRegister()
{
}

bool C_RenderDistancePropNode::GetVisible(const RECT& region, C_DgNodeSelection *pResult)
{
	Editor::C_RenderCore::Instance().GetRenderPipeline().GetVertexRenderer().BeginVisibilityTest(region);
	RenderText();
	Render();
	if (Editor::C_RenderCore::Instance().GetRenderPipeline().GetVertexRenderer().EndVisibilityTest())
	{
		if (pResult) pResult->Add(const_cast<EditorLib::I_DgNode*>(m_pDgNode));
		return true;
	}
	return false;
}

void C_RenderDistancePropNode::AttachToRenderer(
	AftkRender::E_RendererPass pass)
{
	if (pass == AftkRender::E_RendererPass_2D || pass == AftkRender::E_RendererPass_VisibilityTest)
	{
		RenderText();
	}

	Render();
}

void DrawArrow(
   Display::Vertex_XYZ_C*& pVertex,
   const Maths::HVec3& pos, 
   const Maths::HVec3& a, 
   const Maths::HVec3& b,
   U32 colour)
{
	//arrow
	Maths::HVec3 vz(b[0]-a[0], b[1]-a[1], b[2]-a[2]);
	vz.Normalise();
	Maths::HVec3 vy(0.f, 1.f, 0.f);
	Maths::HVec3 vx = vy^vz;
	vx.Normalise();
	vy = vz^vx;
	vy.Normalise();

	Maths::HVec3 vertices[5] = {(pos-vz*0.2f) - 0.1f*(vx+vy),
								(pos-vz*0.2f) - 0.1f*(vx-vy), 
								(pos-vz*0.2f) - 0.1f*(-vx-vy),
								(pos-vz*0.2f) - 0.1f*(-vx+vy), 
								pos};

	for (S32 i=0; i<4; ++i)
	{
		pVertex->x = vertices[i][0]; pVertex->y = vertices[i][1]; pVertex->z = vertices[i][2];
		pVertex->c = colour;
		pVertex++;

		pVertex->x = vertices[4][0]; pVertex->y = vertices[4][1]; pVertex->z = vertices[4][2];
		pVertex->c = colour;
		pVertex++;
	}
}

void C_RenderDistancePropNode::Render() const
{
	assert(m_pParent && m_pParent->GetType()==AftkRender::E_SceneGraphNode_Custom 
			&& m_pParent->GetCustomTypeData()==E_CustomNodeType_Actor);

	if (!Editor::C_PropertyVisibilityManager::Instance().IsVisible((EditorLib::C_DgDistancePropNode*)m_pDgNode))
		return;

	const Display::C_ViewFrustum& frustum = Display::Device()->GetViewFrustum();
	S32 cullResult = frustum.CullAxisBox(m_WorldAABBox);

	if (!cullResult)
		return;

	if (m_linkedNodes[0] && m_linkedNodes[1])
	{
		Maths::RGBA highlight(0,0,0,0);
		highlight = RenderHelpers::GetHighlightColour(m_pDgNode, highlight);

		U32 colour = 0xff3f3f3f;

		if (highlight.Colour())
			colour = highlight.Colour();

		//link
		Display::Vertex_XYZ_C *pVertex = (Display::Vertex_XYZ_C*)
			Editor::C_RenderCore::Instance().GetRenderPipeline().GetVertexRenderer().BeginVertices(
				Display::E_PrimitiveType_LineList, 
				6+8*2, 
				sizeof(Display::Vertex_XYZ_C), 
				Display::Vertex_XYZ_C::m_pDeclaration, true);

		//Bar
		const Maths::HMat44& mA = m_linkedNodes[0]->GetWorldMatrix();
		pVertex->x = mA[3][0]; pVertex->y = mA[3][1]+kSideHeight; pVertex->z = mA[3][2];
		pVertex->c = colour;
		pVertex++;
		const Maths::HMat44& mB = m_linkedNodes[1]->GetWorldMatrix();
		pVertex->x = mB[3][0]; pVertex->y = mB[3][1]+kSideHeight; pVertex->z = mB[3][2];
		pVertex->c = colour;
		pVertex++;

		//Side A
		pVertex->x = mA[3][0]; pVertex->y = mA[3][1]; pVertex->z = mA[3][2];
		pVertex->c = colour;
		pVertex++;
		pVertex->x = mA[3][0]; pVertex->y = mA[3][1]+kSideHeight; pVertex->z = mA[3][2];
		pVertex->c = colour;
		pVertex++;

		//Side B
		pVertex->x = mB[3][0]; pVertex->y = mB[3][1]; pVertex->z = mB[3][2];
		pVertex->c = colour;
		pVertex++;
		pVertex->x = mB[3][0]; pVertex->y = mB[3][1]+kSideHeight; pVertex->z = mB[3][2];
		pVertex->c = colour;
		pVertex++;

		Maths::HVec3 a = (Maths::HVec3)mA[3];
		Maths::HVec3 b = (Maths::HVec3)mB[3];
		{
			Maths::HVec3 p(a.X(), a.Y()+kSideHeight, a.Z());
			DrawArrow(pVertex, p, b, a, colour);
		}
		{
			Maths::HVec3 p(b.X(), b.Y()+kSideHeight, b.Z());
			DrawArrow(pVertex, p, a, b, colour);
		}

		Editor::C_RenderCore::Instance().GetRenderPipeline().GetVertexRenderer().EndVertices(pVertex);
	}
}

void C_RenderDistancePropNode::RenderText() const
{
	if (!m_linkedNodes[0] || !m_linkedNodes[1])
		return;

	AftkRender::C_FontRenderer& fontRenderer = Editor::C_RenderCore::Instance().GetRenderPipeline().GetFontRenderer();
	fontRenderer.Begin();

	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();

	const Maths::HMat44& mA = m_linkedNodes[0]->GetWorldMatrix();
	const Maths::HMat44& mB = m_linkedNodes[1]->GetWorldMatrix();
	const Maths::HVec3& a = (Maths::HVec3&)mA.AW();
	const Maths::HVec3& b = (Maths::HVec3&)mB.AW();
	Maths::HVec3 midPoint(0.5f*(b[0]+a[0]), 0.5f*(b[1]+a[1])+kSideHeight, 0.5f*(b[2]+a[2]));
	Maths::HVec3 tmpPos = viewSettings3D.TransformWorldToWindow(midPoint);

	F32 d = Maths::Sqrt(Maths::DistanceSquared(a, b));

	Maths::RGBA highlight(0,0,0,0);
	highlight = RenderHelpers::GetHighlightColour(m_pDgNode, highlight);

	Maths::RGBA colour(Maths::RGBA(63,63,63,255));
	if (highlight.Colour())
		colour = highlight;

	char buf[1024];
	sprintf(buf, "%.2fm", d);
	fontRenderer.Draw(
		"Arial",
		10,
		buf,
		tmpPos[0], 
		tmpPos[1]-kComponentTextOffset,
		colour);

	
	fontRenderer.End();
}

void C_RenderDistancePropNode::OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode)
{
	const EditorLib::C_DgDistancePropNode *pProp = static_cast<const EditorLib::C_DgDistancePropNode*>(m_pDgNode);
	Editor::C_ActorScene *pScene = static_cast<Editor::C_ActorScene*>(m_pScene);

	if (pDgNode && pDgNode!=m_pDgNode && pDgNode!=pProp->GetLinkedNodeA() && pDgNode!=pProp->GetLinkedNodeB())
		return;

	m_linkedNodes[0] = pScene->Find(pProp->GetLinkedNodeA());
	m_linkedNodes[1] = pScene->Find(pProp->GetLinkedNodeB());

	EditorLib::C_XtObject *pXtPropObject = const_cast<EditorLib::C_XtObject*>(pProp->AsXtObject());
	const EditorLib::C_DgXtCompNode *pComp = pProp->GetParent();
	EditorLib::C_XtObject *pXtCompObject = const_cast<EditorLib::C_XtObject*>(pComp->AsXtObject());
	BIZ_ASSERT(pXtPropObject);
	BIZ_ASSERT(pXtCompObject);

	//our corresponding backend prop node may not be bound to any xt data	//eg domref deleted when actor destroyed
	if (!pXtPropObject->GetXtData())
		return;

	__super::OnNodeResolved(pDgNode);
	UpdateCache();
}

void C_RenderDistancePropNode::UpdateCache() const
{
	AftkRender::C_SceneGraphNode::UpdateCache();

	m_WorldAABBox.SetInvalidExtents();
	m_WorldAABBox.UpdateExtents(
		Maths::C_Vertex(Maths::HVec3(m_WorldMatrix[3][0], m_WorldMatrix[3][1], m_WorldMatrix[3][2])));

	if (m_linkedNodes[0] && m_linkedNodes[1])
	{
		const Maths::HMat44& mA = m_linkedNodes[0]->GetWorldMatrix();
		const Maths::HMat44& mB = m_linkedNodes[1]->GetWorldMatrix();
		m_WorldAABBox.UpdateExtents(Maths::C_Vertex(Maths::HVec3(mA[3][0], mA[3][1], mA[3][2])));
		m_WorldAABBox.UpdateExtents(Maths::C_Vertex(Maths::HVec3(mB[3][0], mB[3][1], mB[3][2])));
	}
}

}
