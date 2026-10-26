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

#include <UserNode/Props/Spline/DgSplinePropNode.h>
#include <UserNode/Props/Spline/RenderSplinePropNode.h>

void RegisterSplinePropRenderNodes()
{
	REGISTER_RENDER_NODE(Editor::C_RenderSplinePropNode)
}

namespace Editor
{

C_RenderSplinePropNode::C_RenderSplinePropNode(
	C_ActorScene* pScene, 
	const EditorLib::C_DgXtPropNodeBase* pDgNode)
: I_PropertyNode(pScene, pDgNode)
{
	m_CustomTypeData = E_CustomNodeType_Property;
	m_nVertices = 0;
	Editor::C_RenderCore::Instance().GetRenderPipeline().GetComplexRenderer().CreateComplexObject(100, &m_pVertexBuffer, this);
}

C_RenderSplinePropNode::~C_RenderSplinePropNode()
{
	Editor::C_RenderCore::Instance().GetRenderPipeline().GetComplexRenderer().DeleteComplexObject(&m_pVertexBuffer);
}

void C_RenderSplinePropNode::ComplexRendererCallback(ComplexObject* object)
{
	FillVertexBuffer();
}

const char*	C_RenderSplinePropNode::GetStaticTypeName()
{
	return EditorLib::C_DgSplinePropNode::GetStaticTypeName();
}

const char* C_RenderSplinePropNode::GetStaticDisplayName()
{
	static char* p = "Spline Property";
	return p;
}

const char*	C_RenderSplinePropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

EditorLib::E_ObjType C_RenderSplinePropNode::GetStaticType()
{
	return EditorLib::C_DgSplinePropNode::GetStaticType();
}

void* C_RenderSplinePropNode::Create()
{
	return new C_RenderSplinePropNode(0,0);
}

void C_RenderSplinePropNode::SimpleRendererRegister()
{
	Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().SetObjTypePrimitive(
		"arcLengthProbe", 
		Primitive::Locator(0.5f, 0.5f, 0.5f), 
		Maths::RGBA(255,0,0,255));
}

bool C_RenderSplinePropNode::GetVisible(const RECT& region, Editor::C_DgNodeSelection *pResult)
{
	if (C_SelectManipulator::GetSelectionMode() == E_SelectionMode_Component)
	{
		const EditorLib::C_DgSplinePropNode* pSplineProp = (EditorLib::C_DgSplinePropNode*)m_pDgNode;
		const EditorLib::C_DgSplinePropNode::CurveType* pCurve = pSplineProp->GetCurve();
		const EditorLib::C_DgSplinePropNode::CurveDataType* pCurveData = pSplineProp->GetCurveData();
		const Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >& knots = pCurveData->GetKnots();

		U32 nCompVertices = pSplineProp->IsSplineClosed() 
							? pSplineProp->GetNumComponents() 
							: Maths::Max((S32)0, (S32)pSplineProp->GetNumComponents()-2);

		U32 compOffset = pSplineProp->IsSplineClosed() ? 0 : 1;

		Ut::Vector<U32> components;
		const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();

		for (U32 iComponent=0; iComponent<nCompVertices; ++iComponent)
		{
			Maths::HVec3 pos = pSplineProp->GetComponent(iComponent+compOffset);
			Maths::HMat43 translateMatrix;
			translateMatrix.SetIdentity();
			translateMatrix.AW(pos);
			Maths::HMat43 componentMatrix = translateMatrix * (Maths::HMat43)m_WorldMatrix;


			Maths::HVec3 screenPos = viewSettings3D.TransformWorldToWindow((const Maths::HVec3&) componentMatrix[3]);
			if (screenPos.X() >= (F32)region.left-4 && screenPos.X() <= (F32)region.right+4 &&
				screenPos.Y() >= (F32)region.top-4 && screenPos.Y() <= (F32)region.bottom+4)
				components.PushBack(iComponent+compOffset);
		}

		if (components.Size())
			pResult->Add(const_cast<EditorLib::I_DgNode*>(m_pDgNode), components);

		return components.Size() > 0;
	}

	return false;
}

void C_RenderSplinePropNode::AttachToRenderer(AftkRender::E_RendererPass pass)
{
	EditorLib::C_DgSplinePropNode* pSplineProp = (EditorLib::C_DgSplinePropNode*)m_pDgNode;
	bool componentDisplay = false;
	F32 probeArcLength = 0;
	{
		Xt::DomBlock* pData = Xt::AsBlock(pSplineProp->GetXtData());
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

		if (!Editor::C_PropertyVisibilityManager::Instance().IsVisible((EditorLib::C_DgSplinePropNode*)m_pDgNode))
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
			const EditorLib::C_DgSplinePropNode* pSplineProp = (EditorLib::C_DgSplinePropNode*)m_pDgNode;
			const EditorLib::C_DgSplinePropNode::CurveType* pCurve = pSplineProp->GetCurve();
			const EditorLib::C_DgSplinePropNode::CurveDataType* pCurveData = pSplineProp->GetCurveData();
			const Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >& knots = pCurveData->GetKnots();

			const Ut::UtVector< Ut::DynamicVector<U32> >* pComponents = 0;

			pItem = selection.Find(GetDgNode());
			if (pItem)
				pComponents = &pItem->components;

			U32 nCompVertices = pSplineProp->IsSplineClosed() 
								? pSplineProp->GetNumComponents() 
								: Maths::Max((S32)0, (S32)pSplineProp->GetNumComponents()-2);
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
						U32 compOffset = pSplineProp->IsSplineClosed() ? 0 : 1;
						for (U32 iComponent=0; iComponent<nCompVertices; ++iComponent)
						{
							EditorLib::C_DgSplinePropNode::E_SplineCompType componentType;
							Maths::HVec3 pos = pSplineProp->GetComponent(iComponent+compOffset, &componentType);
							Maths::RGBA propColour;
							switch(componentType)
							{
								case EditorLib::C_DgSplinePropNode::E_SplineCompType_InTangent:
								case EditorLib::C_DgSplinePropNode::E_SplineCompType_OutTangent:
									propColour = Maths::RGBA(130,0,130);
									break;
								default:
									propColour = Maths::RGBA(200,0,200);
									break;
							};

							Maths::HMat43 translateMatrix;
							translateMatrix.SetIdentity();
							translateMatrix.AW(pos);
							Maths::HMat43 componentMatrix = translateMatrix * (Maths::HMat43)m_WorldMatrix;
							
							if (pComponents) 
							{
								for (S32 j=0; j < pComponents->Size(); ++j)
									if ((*pComponents)[j] == iComponent+compOffset)
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

				//render tangent lines
				if (knots.Size() > 1)
				{
					U32 nLineVertices = pSplineProp->IsSplineClosed() ? knots.Size()*4 : knots.Size()*4-4;

					Maths::RGBA handleColour = highlight;
					Display::Vertex_XYZ_C* pVertex = 
						(Display::Vertex_XYZ_C*)Editor::C_RenderCore::Instance().GetRenderPipeline().GetVertexRenderer().BeginVertices(
							Display::E_PrimitiveType_LineList, 
							nLineVertices, 
							sizeof(Display::Vertex_XYZ_C), 
							Display::Vertex_XYZ_C::m_pDeclaration, 
							true);
					Display::BasicRender()->BindColoured3D();
					if (pVertex) 
					{
						//Create point data.
						Maths::HMat43 translateMatrix;
						translateMatrix.SetIdentity();

						Display::Vertex_XYZ_C *pStartVertex = pVertex;

						U32 componentIndex = pSplineProp->IsSplineClosed() ? 0 : 1;
						for (S32 iKnot=0; iKnot<knots.Size(); ++iKnot)
						{
							EditorLib::C_DgSplinePropNode::E_SplineCompType componentType;

							Maths::HMat43 componentMatrix, translateMatrix;
							translateMatrix.SetIdentity();
							
							Maths::HVec3 p1 = pSplineProp->GetComponent(componentIndex, &componentType);
							assert(componentType != EditorLib::C_DgSplinePropNode::E_SplineCompType_OutTangent);

							translateMatrix.AW(p1);
							componentMatrix = translateMatrix * (Maths::HMat43)m_WorldMatrix;							
							pVertex->x = componentMatrix[3][0];
							pVertex->y = componentMatrix[3][1];
							pVertex->z = componentMatrix[3][2];
							pVertex->c = handleColour;
							pVertex++;
							componentIndex++;

							Maths::HVec3 p2 = pSplineProp->GetComponent(componentIndex, &componentType);
							translateMatrix.AW(p2);
							componentMatrix = translateMatrix * (Maths::HMat43)m_WorldMatrix;							
							pVertex->x = componentMatrix[3][0];
							pVertex->y = componentMatrix[3][1];
							pVertex->z = componentMatrix[3][2];
							pVertex->c = handleColour;
							pVertex++;
							componentIndex++;

							U32 vtxCount = pVertex - pStartVertex;
							if ((U32)(pVertex-pStartVertex) < nLineVertices &&
								componentType==EditorLib::C_DgSplinePropNode::E_SplineCompType_Value)
							{
								pVertex->x = componentMatrix[3][0];
								pVertex->y = componentMatrix[3][1];
								pVertex->z = componentMatrix[3][2];
								pVertex->c = handleColour;
								pVertex++;

								Maths::HVec3 p3 = pSplineProp->GetComponent(componentIndex, &componentType);
								assert(componentType==EditorLib::C_DgSplinePropNode::E_SplineCompType_OutTangent);

								translateMatrix.AW(p3);
								componentMatrix = translateMatrix * (Maths::HMat43)m_WorldMatrix;							
								pVertex->x = componentMatrix[3][0];
								pVertex->y = componentMatrix[3][1];
								pVertex->z = componentMatrix[3][2];
								pVertex->c = handleColour;
								pVertex++;
								componentIndex++;
							}
						}

						Editor::C_RenderCore::Instance().GetRenderPipeline().GetVertexRenderer().EndVertices(pVertex);
					}
					else 
					{
						Util::C_String debugMessage;
						debugMessage.Printf("Failed to create line list for component tangent rendering");
						LOGERROR(EditorLib::C_Core::Instance().GetLogger(), debugMessage.Chars());
					}
				}

				//render normals
				if (knots.Size() > 1 && pSplineProp->GetReparam()->GetNumPoints())
				{
					U32 nLineVertices = knots.Size() > 1 ? (knots.Size()-1)*2 : 0;

					Maths::RGBA handleColour = highlight;
					Display::Vertex_XYZ_C* pVertex = 
						(Display::Vertex_XYZ_C*)Editor::C_RenderCore::Instance().GetRenderPipeline().GetVertexRenderer().BeginVertices(
							Display::E_PrimitiveType_LineList, 
							nLineVertices, 
							sizeof(Display::Vertex_XYZ_C), 
							Display::Vertex_XYZ_C::m_pDeclaration, 
							true);
					Display::BasicRender()->BindColoured3D();
					if (pVertex) 
					{
						//Create point data.
						Maths::HMat43 translateMatrix;
						translateMatrix.SetIdentity();

						Display::Vertex_XYZ_C *pStartVertex = pVertex;

						U32 componentIndex = pSplineProp->IsSplineClosed() ? 0 : 1;
						for (S32 iKnot=0; iKnot<knots.Size()-1; ++iKnot)
						{
							Maths::HMat43 componentMatrix, translateMatrix;
							translateMatrix.SetIdentity();

							F32 thisU = (F32)iKnot/ (F32)(knots.Size()-1);
							F32 nextU = (F32)(iKnot+1) / (F32)(knots.Size()-1);

							F32 thisLength = pSplineProp->GetReparam()->GetArcLength(thisU);
							F32 nextLength = pSplineProp->GetReparam()->GetArcLength(nextU);
							F32 midLength = (nextLength - thisLength) * 0.5f;
							F32 midU = pSplineProp->GetReparam()->GetU(thisLength + midLength);

							//Find mid-point position and tangent.
							Maths::HVec3 midPos = pSplineProp->GetCurve()->Evaluate(midU);
							Maths::HVec3 tangent = pSplineProp->GetCurve()->EvaluateTangent(midU);
							tangent.Normalise();
							const Maths::HVec3 up(0.f,1.f,0.f);
							Maths::HVec3 normal = tangent ^ up; 

							translateMatrix.AW(midPos);
							componentMatrix = translateMatrix * (Maths::HMat43)m_WorldMatrix;
							pVertex->x = componentMatrix[3][0];
							pVertex->y = componentMatrix[3][1];
							pVertex->z = componentMatrix[3][2];
							pVertex->c = highlight;
							++pVertex;

							translateMatrix.AW(midPos+normal);
							componentMatrix = translateMatrix * (Maths::HMat43)m_WorldMatrix;
							pVertex->x = componentMatrix[3][0];
							pVertex->y = componentMatrix[3][1];
							pVertex->z = componentMatrix[3][2];
							pVertex->c = highlight;
							++pVertex;
						}

						Editor::C_RenderCore::Instance().GetRenderPipeline().GetVertexRenderer().EndVertices(pVertex);
					}
					else 
					{
						Util::C_String debugMessage;
						debugMessage.Printf("Failed to create line list for component normal rendering");
						LOGERROR(EditorLib::C_Core::Instance().GetLogger(), debugMessage.Chars());
					}
				}

			}	//nCompVertices
		}	//C_SelectManipulator::GetSelectionMode() == E_SelectionMode_Component

		if (m_nVertices > 0)
		{
			Editor::C_RenderCore::Instance().GetRenderPipeline().GetComplexRenderer().DrawInstance(
				m_pVertexBuffer,
				0,
				m_nVertices,
				&m_WorldMatrix,
				m_WorldAABBox,
				m_pDgNode->GetId(),
				&highlight);

			if (componentDisplay)
			{
				F32 probeU = pSplineProp->GetReparam()->GetU(probeArcLength);
				Maths::C_QuatTrans quatTrans;
				Curve::Actor::GetForwardFacingSplinePos(
					quatTrans,
					*pSplineProp->GetCurve(),
					probeU,
					probeU+0.01f);
				
				//This is baaad, required due to AddPrim not taking data copy! TODO: Refactor renderer...
				m_probeTransform = (Maths::HMat44)quatTrans.Matrix() * m_WorldMatrix;

				Maths::RGBA highlight(0,0,0,0);
				highlight = RenderHelpers::GetHighlightColour(m_pDgNode, highlight);
				Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().AddPrimitive(
					"arcLengthProbe",
					&m_probeTransform, 
					m_WorldAABBox, 
					m_pDgNode->GetId(), 
					-1,
					highlight.Colour() ? &highlight : 0);
			}
		}
	}
	else
	{
		if (componentDisplay)
		{
			AftkRender::C_FontRenderer& fontRenderer = Editor::C_RenderCore::Instance().GetRenderPipeline().GetFontRenderer();
			const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
			Maths::HVec3 tmpPos = viewSettings3D.TransformWorldToWindow(
				(Maths::HVec3&)m_pParent->GetWorldMatrix()[3]);

			char buf[1024];
			sprintf(buf, "%.2fm", pSplineProp->GetReparam()->GetArcLength());
			fontRenderer.Draw(
				"Arial",
				10,
				buf,
				tmpPos[0], 
				tmpPos[1]-kComponentTextOffset,
				Maths::RGBA(255,255,255,255));

			if (C_SelectManipulator::GetSelectionMode() == E_SelectionMode_Component) 
			{
				S32 nKnots = pSplineProp->GetCurve()->GetNumVertices();
				if (nKnots > 1)
				{
					S32 lastKnot = pSplineProp->IsSplineClosed() ? nKnots-1 : nKnots;
					for (S32 iKnot = 0; iKnot < lastKnot; ++iKnot)
					{
						Maths::HVec3 pos = pSplineProp->GetCurve()->GetVertexValue(iKnot);

						Maths::HMat43 translateMatrix;
						translateMatrix.SetIdentity();
						translateMatrix.AW(pos);
						Maths::HMat43 componentMatrix = translateMatrix * (Maths::HMat43)m_WorldMatrix;

						tmpPos = viewSettings3D.TransformWorldToWindow(
							(Maths::HVec3&)componentMatrix[3]);
			
						F32 knotU = (F32)iKnot / (F32)(nKnots-1);
						F32 knotArcLength = pSplineProp->GetReparam()->GetArcLength(knotU);
						sprintf(buf, "%i\n%.2fm", iKnot, knotArcLength);
						fontRenderer.Draw(
							"Arial", 
							10, 
							buf, 
							tmpPos[0], 
							tmpPos[1]-kComponentTextOffset,
							Maths::RGBA(255,255,255,255));
					}
				}
			}
		}
	}
}

void C_RenderSplinePropNode::FillVertexBuffer()
{
	const EditorLib::C_DgSplinePropNode* pSplineProp = (EditorLib::C_DgSplinePropNode*)m_pDgNode;

	const EditorLib::C_DgSplinePropNode::CurveType* pCurve = pSplineProp->GetCurve();
	const EditorLib::C_DgSplinePropNode::CurveDataType* pCurveData = pSplineProp->GetCurveData();
	const Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >& knots = pCurveData->GetKnots();

	m_nVertices = knots.Size()*200;

	U32 vReqBufferSize = m_nVertices;

	if (vReqBufferSize > (U32)m_pVertexBuffer->GetVertexBufferSize())
	{
		if (!Editor::C_RenderCore::Instance().GetRenderPipeline().GetComplexRenderer().ResizeComplexObject(
			vReqBufferSize+100, m_pVertexBuffer))
				return;
	}

	if(!m_pVertexBuffer->GetVertexBuffer()->IsCreated())
		return;

	if (!m_nVertices)
		return;

	{
		Curve::C_Reparameterise<Maths::HVec3> reparam;
		reparam.BuildTable(pCurve, m_nVertices);
		F32 length = reparam.GetArcLength();
		F32 stepSize = reparam.GetArcLength() / (F32)m_nVertices;

		//Lock vertex buffer.
		Display::Vertex_XYZ* pVertex = (Display::Vertex_XYZ*)m_pVertexBuffer->GetVertexBuffer()->Lock(
			0, m_nVertices, 0);
		if (pVertex)
		{
			//Fill buffer with spline curve segments.
			F32 u1 = stepSize;
			for(U32 i = 0; i < m_nVertices; ++i, u1+=stepSize)
			{
				F32 u2 = reparam.GetU(u1);
				assert(i < m_nVertices);
				Maths::HVec3 position = pCurve->Evaluate(u2);
				pVertex->x = position.X();
				pVertex->y = position.Y();
				pVertex->z = position.Z();
				++pVertex;
			}
		}
		//Unlock vertex buffer.
		m_pVertexBuffer->GetVertexBuffer()->Unlock();
	}
}

void C_RenderSplinePropNode::UpdateSpline(const Ut::UtVector< Ut::DynamicVector<U32> >* pComponentIds, const Maths::HVec3* pOffset, const float* pTension)
{
	EditorLib::C_DgSplinePropNode* pSplineProp = (EditorLib::C_DgSplinePropNode*)m_pDgNode;
	EditorLib::C_DgSplinePropNode::CurveType* pCurve = pSplineProp->GetCurve();
	EditorLib::C_DgSplinePropNode::CurveDataType* pCurveData = pSplineProp->GetCurveData();
	Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >& knots = pCurveData->GetKnots();

	m_LocalMatrix.SetIdentity();

	if (pOffset && pComponentIds) 
	{
		for (Ut::UtVector< Ut::DynamicVector<U32> >::ConstIterator it=pComponentIds->Begin(); it!=pComponentIds->End(); ++it) 
		{
			EditorLib::C_DgSplinePropNode::E_SplineCompType compType;
			Maths::HVec3 pos = pSplineProp->GetComponent(*it, &compType);
			if (compType == EditorLib::C_DgSplinePropNode::E_SplineCompType_Value)
			{
				for (Ut::UtVector< Ut::DynamicVector<U32> >::ConstIterator it2=
					pComponentIds->Begin(); it2!=pComponentIds->End(); ++it2) 
				{
					EditorLib::C_DgSplinePropNode::E_SplineCompType compType2;
					Maths::HVec3 pos2 = pSplineProp->GetComponent(*it2, &compType2);
					if (compType2 != EditorLib::C_DgSplinePropNode::E_SplineCompType_Value &&
						((*it2) == (*it) -1 || (*it2) == (*it) +1))

					{
						pos2 = pos2 - *pOffset;
						pSplineProp->SetComponent(*it2, pos2, true);
					}
				}
			}

			pos = pos + *pOffset;
			pSplineProp->SetComponent(*it, pos, true);
		}

		if (pSplineProp->IsSplineClosed() && knots.Size()>1)
			knots[knots.Size()-1] = knots[0];
	}

	if (pTension && pComponentIds) 
	{
		for (Ut::UtVector< Ut::DynamicVector<U32> >::ConstIterator it=pComponentIds->Begin(); it!=pComponentIds->End(); ++it) 
		{
			U32 knotIndex = pSplineProp->GetComponentKnotIndex(*it);
			F32 t = Maths::Clamped(knots[knotIndex].tangentScale + *pTension, 0.0f, 15.0f);
			knots[knotIndex].tangentScale = t;
		}
	}

	pCurve->Process();
	FillVertexBuffer();
	UpdateCache();
}

void C_RenderSplinePropNode::UpdateOffset(const Ut::UtVector< Ut::DynamicVector<U32> >& componentIds, const Maths::HVec3& offset)
{
	UpdateSpline(&componentIds, &offset);
}

void C_RenderSplinePropNode::UpdateSplineTension(const Ut::UtVector< Ut::DynamicVector<U32> >& componentIds, F32 tension)
{
	EditorLib::C_DgSplinePropNode* pSplineProp = (EditorLib::C_DgSplinePropNode*)m_pDgNode;
	EditorLib::C_DgSplinePropNode::CurveType* pCurve = pSplineProp->GetCurve();
	EditorLib::C_DgSplinePropNode::CurveDataType* pCurveData = pSplineProp->GetCurveData();
	Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >& knots = pCurveData->GetKnots();
	for (S32 i = 0; i < knots.Size(); i++)
	{
		//Handle degenerate tangents.
		bool tangentInInvalid = knots[i].tangentIn.LengthSquared() < 0.01f ? true : false;
		bool tangentOutInvalid = knots[i].tangentOut.LengthSquared() < 0.01f ? true : false;
		if (tangentInInvalid || tangentOutInvalid)
		{
			pCurve->CalculateKnotTangents(i, tangentInInvalid, tangentOutInvalid);
			knots[i].tangentScale = 0.f;
		}
	}

	UpdateSpline(&componentIds, 0, &tension);
}

void C_RenderSplinePropNode::OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode)
{
	if (pDgNode && pDgNode != m_pDgNode)
		return;

	__super::OnNodeDeserialised(pDgNode);

	UpdateSpline();
}

void C_RenderSplinePropNode::UpdateCache() const
{
	AftkRender::C_SceneGraphNode::UpdateCache();

	const EditorLib::C_DgSplinePropNode* pSplineProp = (EditorLib::C_DgSplinePropNode*)m_pDgNode;
	const EditorLib::C_DgSplinePropNode::CurveType* pCurve = pSplineProp->GetCurve();
	const EditorLib::C_DgSplinePropNode::CurveDataType* pCurveData = pSplineProp->GetCurveData();
	const Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >& knots = pCurveData->GetKnots();


	Maths::C_Vertex centre;
	Maths::HF32 radius;

	if (knots.Size() < 2)
	{
		centre = Maths::HVec3(0,0,0);
		radius = 0.f;
	}
	else
	{
		Curve::C_CurveBounds<Maths::HVec3> curveBounds;
		Curve::C_Reparameterise<Maths::HVec3> reparam;
		reparam.BuildTable(pCurve, knots.Size());
		curveBounds.Process(pCurve, &reparam);

		centre = curveBounds.GetAabb().BoundingSphere().Centre();
		radius = curveBounds.GetAabb().BoundingSphere().Radius();
	}

	Maths::HVec3 boxCentre = centre;
	boxCentre = boxCentre*m_WorldMatrix;

	Maths::HVec3 min;
	min.X(boxCentre.X()-radius);
	min.Y(boxCentre.Y()-radius);
	min.Z(boxCentre.Z()-radius);
	Maths::HVec3 max;
	max.X(boxCentre.X()+radius);
	max.Y(boxCentre.Y()+radius);
	max.Z(boxCentre.Z()+radius);

	m_WorldAABBox.Min(min);
	m_WorldAABBox.Max(max);
}

Maths::C_AxisBox C_RenderSplinePropNode::GetSelectionAABBox() const
{
	if (C_SelectManipulator::GetSelectionMode() == E_SelectionMode_Component)
	{
		const EditorLib::C_DgSplinePropNode* pSplineProp = (EditorLib::C_DgSplinePropNode*)m_pDgNode;
		const EditorLib::C_DgSplinePropNode::CurveType* pCurve = pSplineProp->GetCurve();
		const EditorLib::C_DgSplinePropNode::CurveDataType* pCurveData = pSplineProp->GetCurveData();
		const Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >& knots = pCurveData->GetKnots();

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

			for(U32 i = 0; i < pSplineProp->GetNumComponents(); ++i)
			{
				for (U32 j = 0; j < (U32)pComponents->Size(); ++j)
				{
					if ((*pComponents)[j] == i)
					{
						EditorLib::C_DgSplinePropNode::E_SplineCompType componentType;
						pSplineProp->GetComponent(i, &componentType);
						U32 knotIndex = pSplineProp->GetComponentKnotIndex(i);

						Maths::HVec3 pos;
						switch(componentType)
						{
							case EditorLib::C_DgSplinePropNode::E_SplineCompType_InTangent:
							{
								pos = Curve::Helpers::InTangentToControlPoint(
									knots[knotIndex].value, knots[knotIndex].tangentIn, knots[knotIndex].tangentScale);
								break;
							}
							case EditorLib::C_DgSplinePropNode::E_SplineCompType_Value:
							{
								pos = knots[knotIndex].value;
								break;
							}
							case EditorLib::C_DgSplinePropNode::E_SplineCompType_OutTangent:
							{
								pos = Curve::Helpers::OutTangentToControlPoint(
									knots[knotIndex].value, knots[knotIndex].tangentOut, knots[knotIndex].tangentScale);
								break;
							}
							default:
								assert(!"Invalid component type!");
						};

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
		else
		{
			return m_WorldAABBox;
		}
	}
	else
	{
		return m_WorldAABBox;
	}
}

}
