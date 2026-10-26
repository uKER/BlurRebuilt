#include "stdafx.h"

#include <Aftk/AftkEnum.h>

// aftk rendering
#include <AftkRender.h>

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtFieldHelper.h>
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
#include <EditorApp/Editor/EditorRenderer.h>
#include <EditorApp/Editor/VertexRenderer.h>
#include <EditorApp/Editor/SimpleRenderer.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderPipeline.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/PropertyVisibilityManager.h>
#include <EditorApp/Editor/RenderHelpers.h>
#include <EditorApp/Editor/manipulators/Manipulator.h>
#include <EditorApp/Editor/manipulators/SelectManipulator.h>
#include <EditorApp/Editor/DgNodeSelection.h>
#include <EditorApp/Misc/MathsHelpers.h>

#include <UserNode/Props/Radius/DgRadiusPropNode.h>
#include <UserNode/Props/Radius/RenderRadiusPropNode.h>

#include <Display/Shaders/Constants.h>
#include <Display/Buffer/VertexDeclarations.h>

void RegisterRadiusPropRenderNodes()
{
	REGISTER_RENDER_NODE(Editor::C_RenderRadiusPropNode)
}

namespace Editor
{

struct Radius
{
	F32 radius;
	Maths::C_RGBA_U8 colour;

	bool operator <(const Radius& r)
	{
		return radius < r.radius;
	}

	bool operator >(const Radius& r)
	{
		return radius > r.radius;
	}
};


C_RenderRadiusPropNode::C_RenderRadiusPropNode(
	C_ActorScene* pScene, 
	const EditorLib::C_DgXtPropNodeBase* pDgNode)
: I_PropertyNode(pScene, pDgNode)
{
	m_CustomTypeData = E_CustomNodeType_Property;
}

C_RenderRadiusPropNode::~C_RenderRadiusPropNode()
{
}

const char*	C_RenderRadiusPropNode::GetStaticTypeName()
{
	return EditorLib::C_DgRadiusPropNode::GetStaticTypeName();
}

const char* C_RenderRadiusPropNode::GetStaticDisplayName()
{
	static char* p = "Radius Property";
	return p;
}

const char*	C_RenderRadiusPropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

EditorLib::E_ObjType C_RenderRadiusPropNode::GetStaticType()
{
	return EditorLib::C_DgRadiusPropNode::GetStaticType();
}

void* C_RenderRadiusPropNode::Create()
{
	return new C_RenderRadiusPropNode(0,0);
}

void C_RenderRadiusPropNode::SimpleRendererRegister()
{
	struct Vertex
	{
		F32 x, y, z;
	};

	//wireframe
	{
		PrimitiveResourcePtr pWirePrimitive = new AftkRender::C_MeshClusterResource();
		const U32 nVertices = 39;

		Vertex *pVertices = new Vertex[nVertices];

		pWirePrimitive->SetNonIndexedPrimitiveType(AftkRender::E_PrimitiveType_LineStrip);
		pWirePrimitive->SetNumVertices(nVertices);
		pWirePrimitive->AttachVertexDataElement(pVertices, AftkRender::E_ElementType_Position);

		Vertex *pCurrent = pVertices;
		for (S32 i=-1; i<36; ++i)
		{
			pCurrent->x = sinf(Maths::DegToRad(i*10.f));
			pCurrent->y = 0.f;
			pCurrent->z = cosf(Maths::DegToRad(i*10.f));
			pCurrent++;
		}

		pCurrent->x = 0.f;
		pCurrent->y = 0.f;
		pCurrent->z = 1.1f;
		pCurrent++;

		pCurrent->x = sinf(Maths::DegToRad(10.f));
		pCurrent->y = 0.f;
		pCurrent->z = cosf(Maths::DegToRad(10.f));
		pCurrent++;

		Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().SetObjTypePrimitive(
			EditorLib::C_DgRadiusPropNode::GetStaticTypeName(), 
			pWirePrimitive, 
			Maths::RGBA(255,0,0,255));
	}
}

bool C_RenderRadiusPropNode::GetVisible(const RECT& region, C_DgNodeSelection *pResult)
{
	return false;
}

void C_RenderRadiusPropNode::AttachToRenderer(
	AftkRender::E_RendererPass pass)
{
	const EditorLib::C_DgRadiusPropNode *pProp = static_cast<const EditorLib::C_DgRadiusPropNode*>(m_pDgNode);
	U32 nRadii= pProp->GetNumRadii();

	bool solid = false;
	Xt::DomBlock* pData = Xt::AsBlock(const_cast<EditorLib::C_DgRadiusPropNode*>(pProp)->GetXtData());
	if (pData)
	{
		Xt::DomField* pField = Xt::AsField(Xt::At(pData, "solid"));
		EditorLib::XtFieldHelper::GetBool(solid, pField, false);
	}

	if (!solid || pass == AftkRender::E_RendererPass_VisibilityTest)
	{
		for (U32 i=0; i<nRadii; ++i)
		{
			const EditorLib::C_StaticPlug<Maths::C_RGBA_U8>& colourPlug = pProp->GetColourPlug(i);
			Maths::C_RGBA_U8 colour = colourPlug.GetValue();
			
			if (C_SelectManipulator::GetSelectionMode() == E_SelectionMode_Component)
			{
				const C_DgNodeSelection& selection = C_RenderCore::Instance().GetRenderScene().GetSelection();
				const C_DgNodeSelection::C_Item *pItem = selection.Find(GetDgNode());

				bool found = false;
				if (pItem && pItem->components.Size())
				{
					for (S32 j=0; j<pItem->components.Size(); ++j)
					{
						if (pItem->components[j] == i)
						{
							found = true;
							break;
						}
					}
				}

			if (!found)
				colour = Maths::C_RGBA_U8((U8)(colour.R()>>1), (U8)(colour.G()>>1), (U8)(colour.B()>>1), colour.A()); 
		}

			Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().AddPrimitive(
				EditorLib::C_DgRadiusPropNode::GetStaticTypeName(),
				&m_matrixCache[i], 
				m_WorldAABBox, 
				m_pDgNode->GetId(), 
				i, 
				&colour);
		}
	}
	else if (pass == AftkRender::E_RendererPass_Alpha)
	{
		Btl::Vector<Radius> radii;
		for (U32 i=0; i<nRadii; ++i)
		{
			const EditorLib::C_StaticPlug<F32>& radiusPlug = pProp->GetRadiusPlug(i);
			if (radiusPlug.GetValue() < Maths::Constants<F32>::Epsilon())
				continue;

			const EditorLib::C_StaticPlug<Maths::C_RGBA_U8>& colourPlug = pProp->GetColourPlug(i);
			Maths::C_RGBA_U8 colour = colourPlug.GetValue();

			if (C_SelectManipulator::GetSelectionMode() == E_SelectionMode_Component)
			{
				const C_DgNodeSelection& selection = C_RenderCore::Instance().GetRenderScene().GetSelection();
				const C_DgNodeSelection::C_Item *pItem = selection.Find(GetDgNode());

				bool found = false;
				if (pItem && pItem->components.Size())
				{
					for (S32 j=0; j<pItem->components.Size(); ++j)
					{
						if (pItem->components[j] == i)
						{
							found = true;
							break;
						}
					}
				}

				if (!found)
					colour = Maths::C_RGBA_U8((U8)(colour.R()>>1), (U8)(colour.G()>>1), (U8)(colour.B()>>1), colour.A()); 
			}

			radii.PushBack();
			radii.Back().radius = m_scaleCache[i]*radiusPlug.GetValue();
			radii.Back().colour = colour;
		}

		std::sort(radii.Begin(), radii.End());
		
		Display::E_CullMode cull = Display::Device()->GetCullMode();

		Display::Device()->SetCullMode(Display::E_CullMode_None);
		Display::Device()->SetStencilEnable(Display::E_Bool_True);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);

		F32 inner = 0.f;
		for (S32 i=0; i<radii.Size(); ++i)
		{
			if (radii[i].radius-inner > Maths::Constants<F32>::Epsilon())
				RenderRing(inner, radii[i].radius, radii[i].colour);

			inner = radii[i].radius;
		}

		Display::Device()->SetCullMode(cull);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
		Display::Device()->SetStencilEnable(Display::E_Bool_False);
	}
}


void C_RenderRadiusPropNode::OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode)
{
	const EditorLib::C_DgRadiusPropNode *pProp = static_cast<const EditorLib::C_DgRadiusPropNode*>(m_pDgNode);
	Editor::C_ActorScene *pScene = static_cast<Editor::C_ActorScene*>(m_pScene);

	if (pDgNode && pDgNode!=m_pDgNode)
		return;

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

void C_RenderRadiusPropNode::UpdateCache() const
{
	AftkRender::C_SceneGraphNode::UpdateCache();

	const EditorLib::C_DgRadiusPropNode *pProp = static_cast<const EditorLib::C_DgRadiusPropNode*>(m_pDgNode);
	Editor::C_ActorScene *pScene = static_cast<Editor::C_ActorScene*>(m_pScene);
	
	m_matrixCache.Clear();
	m_scaleCache.Clear();
	{
		U32 nRadii= pProp->GetNumRadii();
		for (U32 i=0; i<nRadii; ++i)
		{
			const EditorLib::C_StaticPlug<F32>& rPlug = pProp->GetRadiusPlug(i);
			Maths::HMat44 m = m_LocalMatrix;
			m[0][0] *= rPlug.GetValue(); m[0][2] *= rPlug.GetValue();
			m[1][0] *= rPlug.GetValue(); m[1][2] *= rPlug.GetValue();
			m[2][0] *= rPlug.GetValue(); m[2][2] *= rPlug.GetValue();

			m_matrixCache.PushBack(m * m_pParent->GetWorldMatrix());		
			m_scaleCache.PushBack(1.f);
		}
	}
	
	{
		F32 r = 0.f;
		U32 nRadii= pProp->GetNumRadii();
		for (U32 i=0; i<nRadii; ++i)
		{
			const EditorLib::C_StaticPlug<F32>& rPlug = pProp->GetRadiusPlug(i);
			r = Maths::Max<F32>(r, rPlug.GetValue());
		}

		m_WorldAABBox.SetInvalidExtents();
		if (r > 0.f)
			m_WorldAABBox.UpdateExtents(Maths::C_Sphere(m_WorldMatrix[3][0], m_WorldMatrix[3][1], m_WorldMatrix[3][2], r));
	}
}


void C_RenderRadiusPropNode::ScaleRadii(F32 scale, const Ut::UtVector< Ut::DynamicVector<U32> >& componentIds)
{
	const EditorLib::C_DgRadiusPropNode *pProp = static_cast<const EditorLib::C_DgRadiusPropNode*>(m_pDgNode);
	Editor::C_ActorScene *pScene = static_cast<Editor::C_ActorScene*>(m_pScene);
			
	for (S32 i=0; i<componentIds.Size(); ++i)
	{
		const U32 componentId = componentIds[i];
		if (componentId < pProp->GetNumRadii())
		{
			const EditorLib::C_StaticPlug<F32>& rPlug = pProp->GetRadiusPlug(componentId);
			Maths::HMat44 m = m_LocalMatrix;
			m[0][0] *= scale*rPlug.GetValue(); m[0][2] *= scale*rPlug.GetValue();
			m[1][0] *= scale*rPlug.GetValue(); m[1][2] *= scale*rPlug.GetValue();
			m[2][0] *= scale*rPlug.GetValue(); m[2][2] *= scale*rPlug.GetValue();

			m_matrixCache[componentId] = m * m_pParent->GetWorldMatrix();
			m_scaleCache[componentId] = scale;
		}
	}

	//we can probably get away with not updating the bounding box
}

void C_RenderRadiusPropNode::RenderRing(F32 innerRadius, F32 outerRadius, const Maths::C_RGBA_U8& colour)
{
	Display::Vertex_XYZ_C *pVertex = 
		(Display::Vertex_XYZ_C*) Editor::C_RenderCore::Instance().GetRenderPipeline().GetVertexRenderer().BeginVertices(
			Display::E_PrimitiveType_TriangleStrip, 74, sizeof(Display::Vertex_XYZ_C), Display::Vertex_XYZ_C::m_pDeclaration, true);
			
	for (S32 i=0; i<37; ++i)
	{
		pVertex->x = innerRadius*sinf(Maths::DegToRad(i*10.f));
		pVertex->y = 0;
		pVertex->z = innerRadius*cosf(Maths::DegToRad(i*10.f));
		pVertex->c = colour;
		pVertex++;

		pVertex->x = outerRadius*sinf(Maths::DegToRad(i*10.f));
		pVertex->y = 0;
		pVertex->z = outerRadius*cosf(Maths::DegToRad(i*10.f));
		pVertex->c = colour;
		pVertex++;
	}

	Display::C_Matrix matrix;
	Maths::Store(m_WorldMatrix, &matrix);
	Display::BasicRender()->BindColoured3D();
	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&matrix, 4);	

	Editor::C_RenderCore::Instance().GetRenderPipeline().GetVertexRenderer().EndVertices(pVertex);
}

}
