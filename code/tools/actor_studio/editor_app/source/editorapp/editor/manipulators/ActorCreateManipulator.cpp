#include "stdafx.h"

#include <algorithm>

#include <EditorLib/Collision/Collision.h>
#include <EditorLib/Context/EditorContext.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Core.h>

#include <EditorApp/Components/ManipulatorPropertyPanel.h>
#include <EditorApp/Editor/Manipulators/Manipulator.h>
#include <EditorApp/Editor/Manipulators/ActorCreateManipulator.h>
#include <EditorApp/Editor/Manipulators/SplineCreateManipulator.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderPipeline.h>
#include <EditorApp/Editor/RenderViewport.h>
#include <EditorApp/App.h>

#include <core/maths/hvec/new/IncludeGeneral.inl>

using namespace Editor;

BEGIN_EVENT_TABLE(C_ActorCreateManipulator, I_Manipulator)
  EVT_MOUSE_EVENTS(C_ActorCreateManipulator::OnMouseEvent)
END_EVENT_TABLE()

E_CreationMode C_ActorCreateManipulator::m_CreationMode = E_CreationMode_Single;
E_OrientationMode C_ActorCreateManipulator::m_OrientationMode = E_OrientationMode_Vertical;

C_ActorCreateManipulator::C_ActorCreateManipulator()
: I_Manipulator()
{
}

C_ActorCreateManipulator::~C_ActorCreateManipulator()
{
}

E_ManipulatorType C_ActorCreateManipulator::GetType() const
{
	return E_ManipulatorType_Create;
}

bool C_ActorCreateManipulator::IsA(E_ManipulatorType type) const
{
	if (__super::IsA(type))
		return true;
	if (type == C_ActorCreateManipulator::GetType()) 
		return true;

	return false;
}

E_CreationMode C_ActorCreateManipulator::GetCreationMode() const
{
	return m_CreationMode;
}

void C_ActorCreateManipulator::SetOrientationMode(E_OrientationMode mode)
{
	m_OrientationMode = mode;
	C_ManipulatorPropertyPanel *pPropertyPanel = wxGetApp().GetManipulatorPropertyPanel();
	if (pPropertyPanel)
		pPropertyPanel->RefreshProperties();
}

E_OrientationMode C_ActorCreateManipulator::GetOrientationMode() const
{
	return m_OrientationMode;
}

void C_ActorCreateManipulator::SetCreationMode(E_CreationMode mode)
{
	m_CreationMode = mode;
	C_ManipulatorPropertyPanel *pPropertyPanel = wxGetApp().GetManipulatorPropertyPanel();
	if (pPropertyPanel)
		pPropertyPanel->RefreshProperties();
}

void C_ActorCreateManipulator::SetData(const void* userData)
{
	C_ActorCreateManipulatorContext* contextData = (C_ActorCreateManipulatorContext*)userData;
	m_ActorType = contextData->typeName;
	m_PostCreate = contextData->postCreate;
}

void C_ActorCreateManipulator::OnMouseEvent(wxMouseEvent& e)
{
	assert(m_ActorType.Length() && "actor type to be created has not been set");

	wxSetCursor(wxCursor("\"IDC_CURSOR_ACTORCREATE\""));
	if (e.LeftDown())
	{
		wxPoint pos = e.GetPosition();
		Maths::C_Line line = CalculateRay(pos.x, pos.y);

		Maths::Vector3 n = Maths::MakeVector3(0,0,0);
		Maths::HVec3 p(0.f,0.f,0.f);
		bool positionOK = false;

		//intersect camera ray with scene...
		//if (!positionOK)
		{
			Maths::C_Vertex a = line.Start();
			Maths::C_Vertex b = line.End();
			EditorLib::C_Ray ray;
			ray.origin = EditorLib::Vector3(a.X(), a.Y(), a.Z());
			ray.end = EditorLib::Vector3(b.X(), b.Y(), b.Z());

			static const U32 kMaxResults = 100;
			static EditorLib::C_RayResult rayResults[kMaxResults];
			U32 collisionMask = EditorLib::kCollisionSourceType_MaskAll;
			const C_RenderPipeline::ViewportRenderState& renderState = C_RenderCore::Instance().GetRenderPipeline().GetViewportRenderState(m_pViewport);
			if (renderState.fillmode == Display::E_FillMode_Solid)
			{
				if (renderState.cullmode == Display::E_CullMode_Ccw)
					collisionMask &= ~EditorLib::E_CollisionSourceFace_Back;
				else if (renderState.cullmode == Display::E_CullMode_Cw)
					collisionMask &= ~EditorLib::E_CollisionSourceFace_Front;	
			}

			EditorLib::C_RayResult rayBoundsResult;
			U32 nResults = 
				EditorLib::C_Core::Instance().GetCollisionManager().Intersect(
				rayResults,
				kMaxResults,
				ray,
				EditorLib::E_CollisionQueryType_Complex,
				collisionMask);

			if (nResults>0)
			{
				//Sort low to high
				std::sort(rayResults, rayResults + nResults);

				
				for (U32 i=0; i<nResults; i++)
				{
					if (rayResults[i].pCollisionInstance->pSource->GetType() == EditorLib::E_CollisionSourceType_Mesh)
					{
						if (GetOrientationMode() == E_OrientationMode_Mesh)
						{
							S32 triIndex = rayResults[i].triIndex;
							EditorLib::C_CollisionMesh* pCollisionMesh = (EditorLib::C_CollisionMesh*)rayResults[i].pCollisionInstance->pSource;
							EditorLib::Vector3 tri1 = pCollisionMesh->pVertices[pCollisionMesh->pTriIndices[triIndex*3]];
							EditorLib::Vector3 tri2 = pCollisionMesh->pVertices[pCollisionMesh->pTriIndices[(triIndex*3)+1]];
							EditorLib::Vector3 tri3 = pCollisionMesh->pVertices[pCollisionMesh->pTriIndices[(triIndex*3)+2]];
							tri1 = EditorLib::Vector4(tri1.x, tri1.y, tri1.z, 1.f) * rayResults[i].pCollisionInstance->toWorldSpace;
							tri2 = EditorLib::Vector4(tri2.x, tri2.y, tri2.z, 1.f) * rayResults[i].pCollisionInstance->toWorldSpace;
							tri3 = EditorLib::Vector4(tri3.x, tri3.y, tri3.z, 1.f) * rayResults[i].pCollisionInstance->toWorldSpace;

							EditorLib::Vector3 cross = (tri1-tri2) ^ (tri1-tri3);
							UtMaths::Normalise(&cross);
							n = Maths::MakeVector3(cross.X(), cross.Y(), cross.Z());
						}

						p[0] = rayResults[i].intersectionPoint[0];
						p[1] = rayResults[i].intersectionPoint[1];
						p[2] = rayResults[i].intersectionPoint[2];
						positionOK = true;

						break;
					}
				}
			}
		}

		//no intersection with scene mesh...use axii planes
		if (!positionOK)
		{
			Maths::C_Plane intersectionPlane;
				
			if (m_pViewport->GetCamera().GetType() == AftkRender::C_Camera::E_CameraType_Perspective)
				intersectionPlane = Maths::C_Plane(Maths::C_Normal(0.f, 1.f, 0.f), Maths::C_Vertex(0.f, 0.f, 0.f));
			else
			{
				const AftkRender::C_Camera& camera = m_pViewport->GetCamera();
				Maths::HVec3 planeVector = camera.GetFocusPosition() - camera.GetPosition();
				planeVector.Normalise();
				intersectionPlane = Maths::C_Plane(Maths::C_Normal(planeVector), Maths::C_Vertex(0.f, 0.f, 0.f));
			}

			F32 t = intersectionPlane.Intersect(line);
			if (t > 0.f)
			{
				Maths::C_Vertex v = line.Point(t);
				p = v.m_V;
				positionOK = true;
			}
		}


		{
			EditorLib::C_Status status;
			EditorLib::ResultsArray results;

			Btl::String args;
			if (Maths::IsNonZero(Maths::Length(n)))
			{
				double dotProd = (Maths::MakeVector3(0, 1, 0) | n).m_V;
				Maths::Mat33 temp = Maths::MakeMat33_AxisAngle(Maths::Cross(Maths::MakeVector3(0, 1, 0), n), Maths::ArcCos(dotProd));
				Maths::EulerXYZ rotateXYZ = Maths::MakeEulerXYZ(temp);

				args.Printf("%s -t %f %f %f -r %f %f %f", m_ActorType.Chars(), p.m_V[0], p.m_V[1], p.m_V[2], Maths::X(rotateXYZ).m_V, Maths::Y(rotateXYZ).m_V, Maths::Z(rotateXYZ).m_V);
			}
			else
				args.Printf("%s -t %f %f %f", m_ActorType.Chars(), p.m_V[0], p.m_V[1], p.m_V[2]);

			if (m_PostCreate != "")
				args += Btl::String(" -p ") + m_PostCreate;

			status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute(
				"createXtNode", args, results);

			if (!status)
			{
				LOGERROR(g_Log, "Failed to create actor of type %s", m_ActorType.Chars());
				return;
			}

			if (!positionOK)
				LOGWARN(g_Log, "New %s created at (0,0,0)", m_ActorType.Chars());

			EditorLib::C_NodeId nodeId;
			Btl::String tmp;
			results[0].GetValue(tmp);
			nodeId.FromString(tmp);

			bool hasCustomManipulator = false;

			EditorLib::I_DgNode* pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(nodeId);

			if (pNode->IsA(EditorLib::E_ObjType_SceneNode))
			{
				EditorLib::I_XtType* pXtType = EditorLib::C_Core::Instance().GetXtTypeDatabase().FindType(m_ActorType.Chars());
				if (pXtType)
				{
					XtString context = pXtType->GetXtType()->GetString("SceneView", "creationcontext");
					if (context.Length())
					{
						EditorLib::C_Core::Instance().SetContext(Btl::String(context.Text()), pNode);
						hasCustomManipulator = true;
					}
				}
			}

			if (!hasCustomManipulator && m_CreationMode == E_CreationMode_Single) {
				EditorLib::C_Core::Instance().SetContext(EditorLib::E_EditorContext_Select);
				m_ActorType = "";
			}
		}
	} else if (e.RightDown())
	{
		EditorLib::C_Core::Instance().SetContext(EditorLib::E_EditorContext_Select);
		m_ActorType = "";
	}


}
