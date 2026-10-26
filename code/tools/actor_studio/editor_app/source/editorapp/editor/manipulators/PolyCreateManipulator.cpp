#include "stdafx.h"
#include <Misc_Shared/NewCurve/Definitions.h>
#include <Misc_Shared/NewCurve/CurveHelpers.h>
#include <Misc_Shared/NewCurve/Reparameterise.h>
#include <Misc_Shared/NewCurve/CurveData.h>
#include <Misc_Shared/NewCurve/Curve.h>
#include <Misc_Shared/NewCurve/HermitePolicies.h>
#include <Misc_Shared/NewCurve/HermiteSpline.h>

#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/SceneXtActorNode.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Context/EditorContext.h>
#include <EditorLib/Collision/Collision.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Core.h>

#include <EditorApp/Editor/EditorRenderer.h>
#include <EditorApp/Editor/ComplexRenderer.h>
#include <EditorApp/Editor/ActorSceneNode.h>
#include <EditorApp/Editor/IActorPropertyNode.h>

#include <UserNode/Props/Misc/DgPropContextHelpers.h>
#include <UserNode/Props/Poly/DgPolyPropNode.h>
#include <UserNode/Props/Poly/RenderPolyPropNode.h>

#include <EditorApp/Editor/manipulators/Manipulator.h>
#include <EditorApp/Editor/manipulators/PolyCreateManipulator.h>

#include <EditorApp/Editor/RenderViewport.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderPipeline.h>
#include <EditorApp/Editor/RenderScene.h>

using namespace Editor;

BEGIN_EVENT_TABLE(C_PolyCreateManipulator, I_Manipulator)
  EVT_MOUSE_EVENTS(C_PolyCreateManipulator::OnMouseEvent)
END_EVENT_TABLE()

C_PolyCreateManipulator::C_PolyCreateManipulator()
: I_Manipulator()
{
}

C_PolyCreateManipulator::~C_PolyCreateManipulator()
{
}

E_ManipulatorType C_PolyCreateManipulator::GetType() const
{
	return E_ManipulatorType_CreatePoly;
}

bool C_PolyCreateManipulator::IsA(E_ManipulatorType type) const
{
	if (__super::IsA(type))
		return true;
	if (type == C_PolyCreateManipulator::GetType()) 
		return true;

	return false;
}

void C_PolyCreateManipulator::SetData(const void* userData)
{
	if (userData == 0)
		return;

	EditorLib::I_DgNode* pDgNode = (EditorLib::I_DgNode*)userData;
	EditorLib::C_DgPolyPropNode* pPolyNode = 0;

	EditorLib::C_SceneXtNodeBase* pActor = pDgNode->IsA(EditorLib::E_ObjType_SceneXtNodeBase) ? 
		(EditorLib::C_SceneXtNodeBase*)pDgNode : 0;
	if (!pActor) 
		return;
	
	const EditorLib::DgActorComponentArray& components = pActor->GetComponents();

	for (EditorLib::DgActorComponentArray::ConstIterator i=components.Begin(); i!=components.End(); ++i) {
		const EditorLib::DgPropArray& properties = (*i)->GetProperties();

		for (EditorLib::DgPropArray::ConstIterator j=properties.Begin(); j!=properties.End(); ++j) {
			Btl::String typeName = (*j)->GetTypeName();
			if ((*j)->GetType() == EditorLib::E_ObjType_DgPolyPropNode) {
				pPolyNode = (EditorLib::C_DgPolyPropNode*)(*j)->RawPtr();
			} 
		}
	}

	if (pPolyNode == 0) {
		EditorLib::C_Core::Instance().SetContext(EditorLib::E_EditorContext_Select);
		return;
	}

	EditorLib::C_Core::Instance().GetCommandProcessor().Execute("select", pPolyNode->GetId().AsString());
}

void C_PolyCreateManipulator::OnMouseEvent(wxMouseEvent& e)
{
	// Check that there's only one item selected, and that it's a Poly

	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	const EditorLib::C_Selection::ItemArray& items = selection.GetItems();

	if (items.Size() != 1) {
		EditorLib::C_Core::Instance().SetContext(EditorLib::E_EditorContext_Select);
		LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "The Poly manipulator cannot operate on multiple objects");
		return;
	}

	EditorLib::C_DgPolyPropNode* pPolyNode;

	for (EditorLib::C_Selection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
	{
		pPolyNode = (EditorLib::C_DgPolyPropNode*)EditorLib::C_Core::Instance().GetDgManager().FindNodeById(it->id);

		if (!(pPolyNode != 0 && pPolyNode->IsA(EditorLib::E_ObjType_DgPolyPropNode))) {
			EditorLib::C_Core::Instance().SetContext(EditorLib::E_EditorContext_Select);
			LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "The selected item is not a Poly");
			return;
		}
	}
	
	// Modify Poly

	wxSetCursor(wxCursor("\"IDC_CURSOR_ACTORCREATE\""));
	if (e.LeftDown())
	{
		wxPoint pos = e.GetPosition();
		Maths::C_Line line = CalculateRay(pos.x, pos.y);

		EditorLib::C_Ray ray;
		{
			Maths::C_Vertex a = line.Start();
			Maths::C_Vertex b = line.End();
			ray.origin = EditorLib::Vector3(a.X(), a.Y(), a.Z());
			ray.end = EditorLib::Vector3(b.X(), b.Y(), b.Z());
		}

		const EditorLib::C_SceneNode* pTopLevelParent = 
			PropContextHelpers::GetTopLevelParent(*pPolyNode);
		if (!pTopLevelParent) {
			LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "Cannot obtain the parent of the Poly");
			return;
		}

		const EditorLib::C_SceneNode* pSceneNode = (EditorLib::C_SceneNode*)pTopLevelParent;
		const EditorLib::Matrix44& nodeMatrix = pSceneNode->GetWorldMatrix();

		Maths::HVec3 point(0.f,0.f,0.f);
		bool positionOK = false;

		//intersect camera ray with scene...
		{
			U32 collisionMask = EditorLib::kCollisionSourceType_MaskAll;
			const C_RenderPipeline::ViewportRenderState& renderState = C_RenderCore::Instance().GetRenderPipeline().GetViewportRenderState(m_pViewport);
			if (renderState.fillmode == Display::E_FillMode_Solid)
			{
				if (renderState.cullmode == Display::E_CullMode_Ccw)
					collisionMask &= ~EditorLib::E_CollisionSourceFace_Back;
				else if (renderState.cullmode == Display::E_CullMode_Cw)
					collisionMask &= ~EditorLib::E_CollisionSourceFace_Front;	
			}

			EditorLib::C_RayResult rayResult;
			U32 nResults = 
				EditorLib::C_Core::Instance().GetCollisionManager().Intersect(
				rayResult,
				ray,
				EditorLib::E_CollisionQueryType_Complex,
				collisionMask);

			if (nResults>0)
			{			
				point.X(rayResult.intersectionPoint[0]-nodeMatrix[3][0]);
				point.Y(rayResult.intersectionPoint[1]-nodeMatrix[3][1]);
				point.Z(rayResult.intersectionPoint[2]-nodeMatrix[3][2]);
				positionOK = true;
			}
		}

		if (!positionOK)
		{
			Maths::C_Plane intersectionPlane;

			if (m_pViewport->GetCamera().GetType() == AftkRender::C_Camera::E_CameraType_Perspective)
				intersectionPlane = Maths::C_Plane(Maths::C_Normal(0.f, 1.f, 0.f), Maths::C_Vertex(pTopLevelParent->GetWorldMatrix()[3].X(), pTopLevelParent->GetWorldMatrix()[3].Y(), pTopLevelParent->GetWorldMatrix()[3].Z()));
			else
			{
				const AftkRender::C_Camera& camera = m_pViewport->GetCamera();
				Maths::HVec3 planeVector = camera.GetFocusPosition() - camera.GetPosition();
				planeVector.Normalise();
				intersectionPlane = Maths::C_Plane(Maths::C_Normal(planeVector), Maths::C_Vertex(pTopLevelParent->GetWorldMatrix()[3].X(), pTopLevelParent->GetWorldMatrix()[3].Y(), pTopLevelParent->GetWorldMatrix()[3].Z()));
			}

			F32 t = intersectionPlane.Intersect(line);
			if (t > 0.f)
			{
				Maths::C_Vertex p = line.Point(t);

				point.X(p.X()-nodeMatrix[3][0]);
				point.Y(p.Y()-nodeMatrix[3][1]);
				point.Z(p.Z()-nodeMatrix[3][2]);
				positionOK = true;
			}
		}


		if (positionOK)
		{
			pPolyNode->AddPoint(point);

			AftkRender::C_SceneGraphNode* pRenderSgNode = 
				C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pPolyNode);
			if (!pRenderSgNode)
			{
				LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "Cannot obtain the Polys render node");
				return;
			}

			((C_RenderPolyPropNode*)pRenderSgNode)->UpdatePoly();
			
			EditorLib::C_Core::Instance().DirtyViews();
		}
		else
		{
			LOGWARN(g_Log, "Error calculating new Poly point");
		}
	}
	else if (e.RightDown())
	{
		EditorLib::C_Core::Instance().SetContext(EditorLib::E_EditorContext_Select);
	}
}
