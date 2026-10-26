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

#include <UserNode/Props/Link/DgLinkPropNode.h>
#include <UserNode/Props/Link/RenderLinkPropNode.h>

#include <Display/Buffer/VertexDeclarations.h>

void RegisterLinkPropRenderNodes()
{
	REGISTER_RENDER_NODE(Editor::C_RenderLinkPropNode)
}

namespace Editor
{

#define I	0xffffffff,
#define O	0x00ffffff,

const U32 dashPatterns[][16] =
{
	{ I I I I I I I I I I I I I I I I },
	{ I I I I I I I I O O O O O O O O },
	{ I I I I O O O O I I I I O O O O },
	{ I I O O I I O O I I O O I I O O },
	{ I O I O I O I O I O I O I O I O },
};
#undef I
#undef O

const char*const dashNames[] = 
{
	"proplink_pattern0",
	"proplink_pattern1",
	"proplink_pattern2",
	"proplink_pattern3",
	"proplink_pattern4",
};

AftkRender::C_ResourcePtr<Display::C_RefTexture2D> dashTextures[sizeof(dashPatterns)/sizeof(dashPatterns[0])];

C_RenderLinkPropNode::C_RenderLinkPropNode(
	C_ActorScene* pScene, 
	const EditorLib::C_DgXtPropNodeBase* pDgNode)
: I_PropertyNode(pScene, pDgNode)
, m_pLinkedNode(0)
, m_style(0)
, m_colourStart(0xff0000ff)
, m_colourEnd(0xff9999ff)
{
	m_CustomTypeData = E_CustomNodeType_Property;
}

C_RenderLinkPropNode::~C_RenderLinkPropNode()
{
}

const char*	C_RenderLinkPropNode::GetStaticTypeName()
{
	return EditorLib::C_DgLinkPropNode::GetStaticTypeName();
}

const char* C_RenderLinkPropNode::GetStaticDisplayName()
{
	static char* p = "Link Property";
	return p;
}

const char*	C_RenderLinkPropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

EditorLib::E_ObjType C_RenderLinkPropNode::GetStaticType()
{
	return EditorLib::C_DgLinkPropNode::GetStaticType();
}

void* C_RenderLinkPropNode::Create()
{
	return new C_RenderLinkPropNode(0,0);
}

void C_RenderLinkPropNode::SimpleRendererRegister()
{
	for (S32 i=0; i<sizeof(dashPatterns)/sizeof(dashPatterns[0]); ++i)
	{
		Display::C_RefTexture2D *pTexture = new Display::C_RefTexture2D();
		if (Display::Device()->CreateTexture2D(sizeof(dashPatterns[0])/sizeof(U32),1,1,Display::E_Usage_Managed,Display::E_Format_A8R8G8B8,pTexture))
		{
			Display::C_LockedRect rect = pTexture->Lock(0);
			if (rect.pBits)
				memcpy(rect.pBits, dashPatterns[i], sizeof(dashPatterns[i]));
			pTexture->Unlock(0);

			dashTextures[i] = Editor::I_EditorRenderer::GetTextureManager().AddTexture2D(dashNames[i], pTexture);
		}
		else
		{
			delete pTexture;
		}
	}
}

bool C_RenderLinkPropNode::GetVisible(const RECT& region, C_DgNodeSelection *pResult)
{
	Editor::C_RenderCore::Instance().GetRenderPipeline().GetVertexRenderer().BeginVisibilityTest(region);
	Render();
	if (Editor::C_RenderCore::Instance().GetRenderPipeline().GetVertexRenderer().EndVisibilityTest())
	{
		if (pResult) pResult->Add(const_cast<EditorLib::I_DgNode*>(m_pDgNode));
		return true;
	}
	return false;
}

void C_RenderLinkPropNode::AttachToRenderer(
	AftkRender::E_RendererPass pass)
{
	if (pass == AftkRender::E_RendererPass_2D)
		return;

	Render();
}

void C_RenderLinkPropNode::Render() const
{
	assert(m_pParent && m_pParent->GetType()==AftkRender::E_SceneGraphNode_Custom 
			&& m_pParent->GetCustomTypeData()==E_CustomNodeType_Actor);

	if (!Editor::C_PropertyVisibilityManager::Instance().IsVisible((EditorLib::C_DgLinkPropNode*)m_pDgNode))
		return;

	const Display::C_ViewFrustum& frustum = Display::Device()->GetViewFrustum();
	S32 cullResult = frustum.CullAxisBox(m_WorldAABBox);

	if (!cullResult)
		return;

	//TODO: optimise this: batch line primitives
	//TODO: selection
	if (m_pLinkedNode)
	{
		Maths::RGBA highlight(0,0,0,0);
		highlight = RenderHelpers::GetHighlightColour(m_pDgNode, highlight);

		U32 c1 = m_colourStart;
		U32 c2 = m_colourEnd;

		if (highlight.Colour())
			c1 = c2 = highlight.Colour();

		//link
		Display::Vertex_XYZ_C_UV *pVertex = (Display::Vertex_XYZ_C_UV*)
			Editor::C_RenderCore::Instance().GetRenderPipeline().GetVertexRenderer().BeginVertices(
				Display::E_PrimitiveType_LineList, 
				10, 
				sizeof(Display::Vertex_XYZ_C_UV), 
				Display::Vertex_XYZ_C_UV::m_pDeclaration, true);

		pVertex->x = m_WorldMatrix[3][0]; pVertex->y = m_WorldMatrix[3][1]; pVertex->z = m_WorldMatrix[3][2];
		pVertex->c = c1;
		pVertex->u = 0.f; pVertex->v = 0.f;
		pVertex++;

		const Maths::HMat44& m = m_pLinkedNode->GetWorldMatrix();
		pVertex->x = m[3][0]; pVertex->y = m[3][1]; pVertex->z = m[3][2];
		pVertex->c = c2;
		pVertex->u = Maths::HVec3(m[3][0]-m_WorldMatrix[3][0], m[3][1]-m_WorldMatrix[3][1], m[3][2]-m_WorldMatrix[3][2]).Length();
		pVertex++;

		//arrow
		Maths::HVec3 vz(m[3][0]-m_WorldMatrix[3][0], m[3][1]-m_WorldMatrix[3][1], m[3][2]-m_WorldMatrix[3][2]);
		vz.Normalise();
		Maths::HVec3 vy(0.f, 1.f, 0.f);
		Maths::HVec3 vx = vy^vz;
		vx.Normalise();
		vy = vz^vx;
		vy.Normalise();


		Maths::HVec3 midPoint(0.5f*(m[3][0]+m_WorldMatrix[3][0]), 0.5f*(m[3][1]+m_WorldMatrix[3][1]), 0.5f*(m[3][2]+m_WorldMatrix[3][2]));
		Maths::HVec3 vertices[5] = {midPoint + 0.0354f*(vx+vy),
									midPoint + 0.0354f*(vx-vy), 
									midPoint + 0.0354f*(-vx-vy),
									midPoint + 0.0354f*(-vx+vy), 
									midPoint + 0.1f*vz };

		for (S32 i=0; i<4; ++i)
		{
			pVertex->x = vertices[i][0]; pVertex->y = vertices[i][1]; pVertex->z = vertices[i][2];
			pVertex->c = c1;
			pVertex->u = 0.f; pVertex->v = 0.f; 
			pVertex++;

			pVertex->x = vertices[4][0]; pVertex->y = vertices[4][1]; pVertex->z = vertices[4][2];
			pVertex->c = c1;
			pVertex->u = 0.f; pVertex->v = 0.f; 
			pVertex++;
		}

		Display::BasicRender()->BindTextured3D(dashTextures[m_style], Display::E_TextureAddress_Wrap);
		Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
		Editor::C_RenderCore::Instance().GetRenderPipeline().GetVertexRenderer().EndVertices(pVertex);
		Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	}
}

void C_RenderLinkPropNode::OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode)
{
	const EditorLib::C_DgLinkPropNode *pProp = static_cast<const EditorLib::C_DgLinkPropNode*>(m_pDgNode);
	Editor::C_ActorScene *pScene = static_cast<Editor::C_ActorScene*>(m_pScene);

	if (pDgNode && pDgNode!=m_pDgNode && pDgNode!=pProp->GetLinkedNode())
		return;

	m_pLinkedNode = pScene->Find(pProp->GetLinkedNode());

	EditorLib::C_XtObject *pXtPropObject = const_cast<EditorLib::C_XtObject*>(pProp->AsXtObject());
	const EditorLib::C_DgXtCompNode *pComp = pProp->GetParent();
	EditorLib::C_XtObject *pXtCompObject = const_cast<EditorLib::C_XtObject*>(pComp->AsXtObject());
	assert(pXtPropObject);
	assert(pXtCompObject);

	//our corresponding backend prop node may not be bound to any xt data	//eg domref deleted when actor destroyed
	if (!pXtPropObject->GetXtData())
		return;

	if (pDgNode && pDgNode!=m_pDgNode && pDgNode!=pProp->GetLinkedNode())
		return;

	LtStruct *pType = static_cast<LtStruct*>(pXtCompObject->GetXtType());
	LarvaField *pField = FindFieldByName(pType, pXtPropObject->GetXtData()->parentKey);

	m_style = pField->attrs.GetInt("SceneView", "style", 0);
	if (m_style > sizeof(dashPatterns)/sizeof(dashPatterns[0]))
		m_style = 0;
	m_colourStart = ((U32)pField->attrs.GetColour("SceneView", "start", m_colourStart)) | 0xff000000;
	m_colourEnd = ((U32)pField->attrs.GetColour("SceneView", "end", m_colourEnd)) | 0xff000000;

	__super::OnNodeResolved(pDgNode);
	m_pLinkedNode = pScene->Find(pProp->GetLinkedNode());
	UpdateCache();
}

void C_RenderLinkPropNode::OnNodeResolved(const EditorLib::I_DgNode* pDgNode)
{
}

void C_RenderLinkPropNode::UpdateCache() const
{
	AftkRender::C_SceneGraphNode::UpdateCache();

	m_WorldAABBox.SetInvalidExtents();
	m_WorldAABBox.UpdateExtents(
		Maths::C_Vertex(Maths::HVec3(m_WorldMatrix[3][0], m_WorldMatrix[3][1], m_WorldMatrix[3][2])));

	if (m_pLinkedNode)
	{
		const Maths::HMat44& m = m_pLinkedNode->GetWorldMatrix();
		m_WorldAABBox.UpdateExtents(Maths::C_Vertex(Maths::HVec3(m[3][0], m[3][1], m[3][2])));
	}
}

}
