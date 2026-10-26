#include "stdafx.h"

#include <Display/Buffer/VertexDeclarations.h>

#include <Misc_Shared/NewCurve/Definitions.h>
#include <Misc_Shared/NewCurve/CurveHelpers.h>
#include <Misc_Shared/NewCurve/Reparameterise.h>
#include <Misc_Shared/NewCurve/ReparamHelpers.h>
#include <Misc_Shared/NewCurve/CurveData.h>
#include <Misc_Shared/NewCurve/Curve.h>
#include <Misc_Shared/NewCurve/HermitePolicies.h>
#include <Misc_Shared/NewCurve/HermiteSpline.h>
#include <Misc_Shared/NewCurve/CurveBounds.h>
#include <Misc_Shared/NewCurve/Reparameterise.h>
#include <Misc_Shared/NewCurve/Actor/ActorHelpers.h>

// aftk rendering
#include <Aftk/AftkEnum.h>
#include <AftkRender.h>

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtFieldHelper.h>
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
#include <EditorApp/Editor/ComplexRenderer.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderPipeline.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/RenderDefinitions.h>
#include <EditorApp/Editor/RenderHelpers.h>
#include <EditorApp/Editor/PropertyVisibilityManager.h>
#include <EditorApp/Editor/VertexRenderer.h>
#include <EditorApp/Misc/MathsHelpers.h>
#include <EditorApp/Editor/DgNodeSelection.h>
#include <EditorApp/Editor/manipulators/contexts/IManipulatorContext.h>
#include <EditorApp/Editor/manipulators/Manipulator.h>
#include <EditorApp/Editor/manipulators/SelectManipulator.h>

#include <UserNode/Props/Poly/DgPolyPropNode.h>
#include <UserNode/Props/Poly/RenderPolyPropNode.h>

void RegisterPolyPropRenderNodes()
{
	REGISTER_RENDER_NODE(Editor::C_RenderPolyPropNode)
}

namespace Editor
{

C_RenderPolyPropNode::C_RenderPolyPropNode(
	C_ActorScene* pScene, 
	const EditorLib::C_DgXtPropNodeBase* pDgNode)
: I_PropertyNode(pScene, pDgNode)
{
	m_CustomTypeData = E_CustomNodeType_Property;
	m_nVertices = 0;
	Editor::C_RenderCore::Instance().GetRenderPipeline().GetComplexRenderer().CreateComplexObject(100, &m_pVertexBuffer, this);
	m_pVertexBuffer->SetPrimitiveType(Display::E_PrimitiveType_LineList);
}

C_RenderPolyPropNode::~C_RenderPolyPropNode()
{
	Editor::C_RenderCore::Instance().GetRenderPipeline().GetComplexRenderer().DeleteComplexObject(&m_pVertexBuffer);
}

void C_RenderPolyPropNode::ComplexRendererCallback(ComplexObject* object)
{
	FillVertexBuffer();
}

const char*	C_RenderPolyPropNode::GetStaticTypeName()
{
	return EditorLib::C_DgPolyPropNode::GetStaticTypeName();
}

const char* C_RenderPolyPropNode::GetStaticDisplayName()
{
	static char* p = "Polygon Property";
	return p;
}

const char*	C_RenderPolyPropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

EditorLib::E_ObjType C_RenderPolyPropNode::GetStaticType()
{
	return EditorLib::C_DgPolyPropNode::GetStaticType();
}

void* C_RenderPolyPropNode::Create()
{
	return new C_RenderPolyPropNode(0,0);
}

void C_RenderPolyPropNode::SimpleRendererRegister()
{
}

bool C_RenderPolyPropNode::GetVisible(const RECT& region, Editor::C_DgNodeSelection *pResult)
{
	if (C_SelectManipulator::GetSelectionMode() == E_SelectionMode_Component)
	{
		const EditorLib::C_DgPolyPropNode* pPolyProp = (EditorLib::C_DgPolyPropNode*)m_pDgNode;

		U32 nCompVertices = pPolyProp->GetNumComponents();

		Ut::Vector<U32> components;
		const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();

		for (U32 iComponent=0; iComponent<nCompVertices; ++iComponent)
		{
			Maths::HVec3 pos = pPolyProp->GetComponent(iComponent);
			Maths::HMat43 translateMatrix;
			translateMatrix.SetIdentity();
			translateMatrix.AW(pos);
			Maths::HMat43 componentMatrix = translateMatrix * (Maths::HMat43)m_WorldMatrix;

			Maths::HVec3 screenPos = viewSettings3D.TransformWorldToWindow((const Maths::HVec3&) componentMatrix[3]);
			if (screenPos.X() >= (F32)region.left-4 && screenPos.X() <= (F32)region.right+4 &&
				screenPos.Y() >= (F32)region.top-4 && screenPos.Y() <= (F32)region.bottom+4)
				components.PushBack(iComponent);
		}

		if (components.Size())
			pResult->Add(const_cast<EditorLib::I_DgNode*>(m_pDgNode), components);

		return components.Size() > 0;
	}
	
	return false;
}

void C_RenderPolyPropNode::AttachToRenderer(AftkRender::E_RendererPass pass)
{
	EditorLib::C_DgPolyPropNode* pPolyProp = (EditorLib::C_DgPolyPropNode*)m_pDgNode;
	bool componentDisplay = false;
	F32 probeArcLength = 0;
	{
		Xt::DomBlock* pData = Xt::AsBlock(pPolyProp->GetXtData());
		if (pData)
		{
			Xt::DomField* pField = Xt::AsField(Xt::At(pData, "componentDisplay"));
			EditorLib::XtFieldHelper::GetBool(componentDisplay, pField, false);
		}
		if (pData)
		{
			Xt::DomField* pField = Xt::AsField(Xt::At(pData, "arcLengthProbe"));
			EditorLib::XtFieldHelper::GetScalar(probeArcLength, pField);
		}
	}

	if (pass != AftkRender::E_RendererPass_2D)
	{
		assert(m_pParent && m_pParent->GetType()==AftkRender::E_SceneGraphNode_Custom 
				&& m_pParent->GetCustomTypeData()==E_CustomNodeType_Actor);

		if (!Editor::C_PropertyVisibilityManager::Instance().IsVisible((EditorLib::C_DgPolyPropNode*)m_pDgNode))
			return;

		const C_DgNodeBase* pParent = (const C_DgNodeBase*) m_pParent;
		const C_DgNodeSelection& selection = C_RenderCore::Instance().GetRenderScene().GetSelection();

		Maths::RGBA highlight(0,4,96,255);

		const C_DgNodeSelection::C_Item *pItem = selection.Find(pParent->GetDgNode());
		if (pItem)
		{
			// Parent is selected
			highlight = Maths::RGBA(67,255,163,255);
		}
		else
		{
			//Highlight this node.
			pItem = selection.Find(GetDgNode());
			if (pItem)
			{
				highlight = Maths::RGBA(200,0,200,255);
			}
		}

		if (C_SelectManipulator::GetSelectionMode() == E_SelectionMode_Component) 
		{
			const EditorLib::C_DgPolyPropNode* pPolyProp = (EditorLib::C_DgPolyPropNode*)m_pDgNode;

			const Ut::UtVector< Ut::DynamicVector<U32> >* pComponents = 0;

			pItem = selection.Find(GetDgNode());
			if (pItem)
				pComponents = &pItem->components;

			U32 nCompVertices = pPolyProp->GetNumComponents();

			if (nCompVertices)
			{
				//render points
				{
					F32 pointSize = 4.0f;
					pointSize = (F32)Display::Device()->GetD3DDevice()->SetRenderState(
							D3DRS_POINTSIZE, *((DWORD*)&pointSize));

					Display::Vertex_XYZ_C* pVertex = 
						(Display::Vertex_XYZ_C*)Editor::C_RenderCore::Instance().GetRenderPipeline().GetVertexRenderer().BeginVertices(
							Display::E_PrimitiveType_PointList, 
							nCompVertices, 
							sizeof(Display::Vertex_XYZ_C), 
							Display::Vertex_XYZ_C::m_pDeclaration, 
							true);

					Display::BasicRender()->BindColoured3D();

					if (pVertex) 
					{
						for (U32 iComponent=0; iComponent<nCompVertices; ++iComponent)
						{
							Maths::HVec3 pos = pPolyProp->GetComponent(iComponent);
							pos.Y(0);
							
							Maths::HMat43 translateMatrix;
							translateMatrix.SetIdentity();
							translateMatrix.AW(pos);

							Maths::HMat43 componentMatrix = translateMatrix * (Maths::HMat43)m_WorldMatrix;
							
							Maths::RGBA propColour = Maths::RGBA(200,0,200);

							if (pComponents) 
							{
								for (S32 j=0; j < pComponents->Size(); ++j)
									if ((*pComponents)[j] == iComponent)
										propColour = Maths::RGBA(255, 255, 128);
							}

							pVertex->x = componentMatrix[3][0];
							pVertex->y = componentMatrix[3][1];
							pVertex->z = componentMatrix[3][2];
							pVertex->c = propColour;
							pVertex++;
						}

						Editor::C_RenderCore::Instance().GetRenderPipeline().GetVertexRenderer().EndVertices(pVertex);
					} 
					else 
					{
						Util::C_String debugMessage;
						debugMessage.Printf("Couldn't create point list for component rendering: %d components", nCompVertices);
						LOGERROR(EditorLib::C_Core::Instance().GetLogger(), debugMessage.Chars());
					}
					Display::Device()->GetD3DDevice()->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&pointSize));
				}
			}	//nCompVertices
		}	//C_SelectManipulator::GetSelectionMode() == E_SelectionMode_Component

		if (m_nVertices > 2)
		{
			Editor::C_RenderCore::Instance().GetRenderPipeline().GetComplexRenderer().DrawInstance(
				m_pVertexBuffer,
				0,
				m_nVertices,
				&m_WorldMatrix,
				m_WorldAABBox,
				m_pDgNode->GetId(),
				&highlight);
		}
	}
}

void C_RenderPolyPropNode::FillVertexBuffer()
{
	const EditorLib::C_DgPolyPropNode* pPolyProp = (EditorLib::C_DgPolyPropNode*)m_pDgNode;

	m_nVertices = 0;

	if (pPolyProp->GetNumComponents() < 2)
		return;

	U32 vReqBufferSize = m_nVertices;

	if (vReqBufferSize > (U32)m_pVertexBuffer->GetVertexBufferSize())
	{
		if (!Editor::C_RenderCore::Instance().GetRenderPipeline().GetComplexRenderer().ResizeComplexObject(
			vReqBufferSize+100, m_pVertexBuffer))
				return;
	}

	if(!m_pVertexBuffer->GetVertexBuffer()->IsCreated())
		return;

	m_nVertices = pPolyProp->GetNumComponents() * 6;

	{
		//Lock vertex buffer.
		Display::Vertex_XYZ* pVertex = (Display::Vertex_XYZ*)m_pVertexBuffer->GetVertexBuffer()->Lock(
			0, m_nVertices, 0);
		if (pVertex)
		{
			//Fill buffer with Poly curve segments.
			for(U32 i = 0; i < pPolyProp->GetNumComponents(); ++i)
			{
				assert(i < m_nVertices);
				
				Maths::HVec3 position;
					
				position = pPolyProp->GetComponent(i);
				pVertex->x = position.X();
				pVertex->y = pPolyProp->GetHeight();
				pVertex->z = position.Z();
				++pVertex;

				position = pPolyProp->GetComponent(i);
				pVertex->x = position.X();
				pVertex->y = 0;
				pVertex->z = position.Z();
				++pVertex;

				position = pPolyProp->GetComponent(i);
				pVertex->x = position.X();
				pVertex->y = 0;
				pVertex->z = position.Z();
				++pVertex;

				position = pPolyProp->GetComponent((i+1) % pPolyProp->GetNumComponents());
				pVertex->x = position.X();
				pVertex->y = 0;
				pVertex->z = position.Z();
				++pVertex;

				position = pPolyProp->GetComponent(i);
				pVertex->x = position.X();
				pVertex->y = pPolyProp->GetHeight();
				pVertex->z = position.Z();
				++pVertex;

				position = pPolyProp->GetComponent((i+1) % pPolyProp->GetNumComponents());
				pVertex->x = position.X();
				pVertex->y = pPolyProp->GetHeight();
				pVertex->z = position.Z();
				++pVertex;
			}
		}
		//Unlock vertex buffer.
		m_pVertexBuffer->GetVertexBuffer()->Unlock();
	}
}

void C_RenderPolyPropNode::UpdatePoly(const Ut::UtVector< Ut::DynamicVector<U32> >* pComponentIds, const Maths::HVec3* pOffset)
{
	EditorLib::C_DgPolyPropNode* pPolyProp = (EditorLib::C_DgPolyPropNode*)m_pDgNode;

	if (pPolyProp && pComponentIds && pOffset)
	{
		for (int i=0; i<pComponentIds->Size(); i++)
		{
			Maths::HVec3 newPosition = pPolyProp->GetComponent((*pComponentIds)[i]) + *pOffset;
			newPosition.Y(0);
			pPolyProp->SetComponent((*pComponentIds)[i], newPosition);
		}
	}

	FillVertexBuffer();
	UpdateCache();
}

void C_RenderPolyPropNode::UpdateOffset(const Ut::UtVector< Ut::DynamicVector<U32> >& componentIds, const Maths::HVec3& offset)
{
	UpdatePoly(&componentIds, &offset);
}

void C_RenderPolyPropNode::OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode)
{
	if (pDgNode && pDgNode != m_pDgNode)
		return;

	__super::OnNodeDeserialised(pDgNode);

	UpdatePoly();
}

void C_RenderPolyPropNode::UpdateCache() const
{
	AftkRender::C_SceneGraphNode::UpdateCache();

	const EditorLib::C_DgPolyPropNode* pPolyProp = (EditorLib::C_DgPolyPropNode*)m_pDgNode;

	m_WorldAABBox.SetInvalidExtents();

	for (U32 i=0; i<pPolyProp->GetNumComponents(); i++)
	{
		m_WorldAABBox.UpdateExtents(pPolyProp->GetComponent(i) * m_WorldMatrix);
	}
}

Maths::C_AxisBox C_RenderPolyPropNode::GetSelectionAABBox() const
{
	if (C_SelectManipulator::GetSelectionMode() == E_SelectionMode_Component)
	{
		const EditorLib::C_DgPolyPropNode* pPolyProp = (EditorLib::C_DgPolyPropNode*)m_pDgNode;
		const Ut::UtVector< Ut::DynamicVector<U32> >* pComponents = 0;

		const C_DgNodeSelection& selection = C_RenderCore::Instance().GetRenderScene().GetSelection();

		const C_DgNodeSelection::C_Item *pItem = selection.Find(GetDgNode());
		if (pItem)
			pComponents = &pItem->components;

		if (pComponents && pComponents->Size())
		{
			Maths::C_AxisBox selectionBounds;
			selectionBounds.SetInvalidExtents();

			Maths::C_Vertex v;

			for(U32 i = 0; i < pPolyProp->GetNumComponents(); ++i)
			{
				for (U32 j = 0; j < (U32)pComponents->Size(); ++j)
				{
					if ((*pComponents)[j] == i)
					{
						Maths::HVec3& pos = pPolyProp->GetComponent(i);

						Maths::HMat44 translateMatrix;
						translateMatrix[3][0] = pos.X();
						translateMatrix[3][1] = pos.Y();
						translateMatrix[3][2] = pos.Z();
						translateMatrix[3][3] = 1;

						Maths::HMat44 componentMatrix = translateMatrix * m_WorldMatrix;

						v.X(componentMatrix[3][0]);
						v.Y(componentMatrix[3][1]);
						v.Z(componentMatrix[3][2]);

						selectionBounds.UpdateExtents(v);
					}
				}
			}

			return selectionBounds;
		}
	}
	
	return m_WorldAABBox;
}

}
