#include "stdafx.h"
#include <algorithm>

#include <Gravity/NarrowPhase/Primitive.h>
#include <Gravity/NarrowPhase/CollisionResult.h>
#include <Gravity/NarrowPhase/PrimitiveTest.h>
#include <Gravity/NarrowPhase/Primitive.inl>

#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Collision/Collision.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Core.h>

#include "Display/Buffer/VertexDeclarations.h"
#include "Display/Shaders/Constants.h"

#include "shaders/vsConstants.h"
#include "shaders/psConstants.h"

#include <EditorApp/Editor/manipulators/contexts/IManipulatorContext.h>
#include <EditorApp/Editor/manipulators/contexts/IVectorContext.h>
#include <EditorApp/Editor/manipulators/Manipulator.h>
#include <EditorApp/Editor/manipulators/SelectManipulator.h>
#include <EditorApp/Editor/manipulators/MoveManipulator.h>
#include <EditorApp/Editor/RenderViewport.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderPipeline.h>

#include <EditorApp/components/ManipulatorPropertyPanel.h>

using namespace Editor;

inline void SetVertex(AftkRender::Vertex_XYZ2_C *pVertex, F32 x0, F32 y0, F32 z0, F32 x1, F32 y1, F32 z1, U32 c)
{
	pVertex->x0=x0; pVertex->y0=y0, pVertex->z0=z0;
	pVertex->x1=x1; pVertex->y1=y1, pVertex->z1=z1;

	pVertex->a = (c & 0xff000000) >> 24;
	pVertex->r = (c & 0x00ff0000) >> 16;
	pVertex->g = (c & 0x0000ff00) >> 8;
	pVertex->b = (c & 0x000000ff) >> 0;
}

inline void SetVertex(Display::Vertex_XYZ_C *pVertex, F32 x0, F32 y0, F32 z0, U32 c)
{
	pVertex->x=x0; pVertex->y=y0, pVertex->z=z0;
	pVertex->c = c;
}
  
BEGIN_EVENT_TABLE(C_MoveManipulator, C_SelectManipulator)
  EVT_MOUSE_EVENTS(C_MoveManipulator::OnMouseEvent)
  EVT_MOUSE_CAPTURE_LOST(C_MoveManipulator::OnMouseCaptureLostEvent)
  EVT_KEY_DOWN(C_MoveManipulator::OnKeyDown)
  EVT_KEY_UP(C_MoveManipulator::OnKeyUp)
END_EVENT_TABLE()

C_MoveManipulator::C_MoveManipulator()
: C_SelectManipulator()
, m_Flags(Centre)
, m_HoverFlags(0)
, m_SnapMove(1.f)
, m_SnapToggle(false)
, m_Constraint(E_Constraint_None)
, m_pEdge(NULL)
{
}

C_MoveManipulator::~C_MoveManipulator()
{
	SAFE_DELETE_PTR(m_pEdge);
}

E_ManipulatorType C_MoveManipulator::GetType() const
{
	return E_ManipulatorType_Move;
}

bool C_MoveManipulator::IsA(E_ManipulatorType type) const
{
	if (__super::IsA(type))
		return true;
	if (type == C_MoveManipulator::GetType()) 
		return true;

	return false;
}

void C_MoveManipulator::OnMouseEvent(wxMouseEvent &e)
{
	U8 previousHoverState = m_HoverFlags;
	U8 previousState = m_Flags;
			
	if (!(m_Flags & Active) && e.ButtonDown())
	{
		m_SnapFlags.GridSnap = m_pToolManager->IsModeEnabled(E_ManipulatorMode_GridSnap);
		m_SnapFlags.EdgeSnap = m_pToolManager->IsModeEnabled(E_ManipulatorMode_EdgeSnap);
		m_SnapFlags.PointSnap = m_pToolManager->IsModeEnabled(E_ManipulatorMode_PointSnap);
	}

	if (C_SelectManipulator::IsActive())
		return C_SelectManipulator::OnMouseEvent(e);

	if (m_Flags & Active)
	{
		if (e.LeftUp() || e.MiddleUp())
		{
			if (wxWindow::GetCapture() == m_pViewport)
				m_pViewport->ReleaseMouse();

			m_Flags &= ~Active;
			
			Maths::HVec3 delta(
				m_Transform[3][0]-m_StartTransform[3][0],
				m_Transform[3][1]-m_StartTransform[3][1],
				m_Transform[3][2]-m_StartTransform[3][2]);

			if (m_pContext)
				((I_VectorContext*)m_pContext)->Apply(delta);

			m_StartTransform = m_Transform;

			// Destroy collision edge cache so we force a search for closest edge next time around
			SAFE_DELETE_PTR(m_pEdge)
		}
		else
		{
			if (!(m_SnapFlags.PointSnap||m_SnapFlags.EdgeSnap||m_SnapFlags.GridSnap))
			{
				wxPoint pos = e.GetPosition();

				//orthonormalise camera axii
				const AftkRender::C_Camera& camera = m_pViewport->GetCamera();
				Maths::HVec3 toZ= camera.GetFocusPosition()-camera.GetPosition();
				toZ.Normalise();

				Maths::HVec3 toX = camera.GetUpVector()^toZ;
				toX.Normalise();

				Maths::HVec3 toY = toZ^toX;
				toY.Normalise();

				//calculate plane for performing ray-intersection tests against
				Maths::C_Plane intersectionPlane;
				Maths::HVec3 axis;

				if (m_Flags & Centre || (m_Constraint && e.ControlDown()))
				{
					if (m_pViewport->GetCamera().GetType() == AftkRender::C_Camera::E_CameraType_Perspective)
					{
						Maths::HVec3 axis = (m_Constraint == E_Constraint_XY) ? Maths::HVec3(m_Transform[2][0], m_Transform[2][1], m_Transform[2][2]) :
											(m_Constraint == E_Constraint_XZ) ? Maths::HVec3(m_Transform[1][0], m_Transform[1][1], m_Transform[1][2]) :
											(m_Constraint == E_Constraint_YZ) ? Maths::HVec3(m_Transform[0][0], m_Transform[0][1], m_Transform[0][2]) :
											toZ;

						intersectionPlane = Maths::C_Plane(Maths::C_Normal(axis), Maths::C_Vertex(m_StartTransform.AW()[0], m_StartTransform.AW()[1], m_StartTransform.AW()[2]));
					}
					else
						intersectionPlane = Maths::C_Plane(Maths::C_Normal(toZ), Maths::C_Vertex(m_StartTransform.AW()[0], m_StartTransform.AW()[1], m_StartTransform.AW()[2]));
				}
				else
				{
					if (m_Flags & XAxis)
						axis = Maths::HVec3(m_Transform[0][0], m_Transform[0][1], m_Transform[0][2]);
					else if (m_Flags & YAxis)
						axis = Maths::HVec3(m_Transform[1][0], m_Transform[1][1], m_Transform[1][2]);
					else if (m_Flags & ZAxis)
						axis = Maths::HVec3(m_Transform[2][0], m_Transform[2][1], m_Transform[2][2]);
					else
					{
						LOGERROR(g_Log, "Move manipulator is without any active components");
						return;
					}
					axis.Normalise();

					Maths::HVec3 planeUp = axis^toZ;
					Maths::HVec3 planeNormal = axis^planeUp;
					planeNormal.Normalise();

					intersectionPlane = Maths::C_Plane(Maths::C_Normal(planeNormal), Maths::C_Vertex(m_StartTransform.AW()[0], m_StartTransform.AW()[1], m_StartTransform.AW()[2]));
				}

				Maths::C_Line r0 = CalculateRay(m_ClickPosition.x, m_ClickPosition.y);
				F32 t0 = Maths::Max(0.f, intersectionPlane.Intersect(r0));

				Maths::C_Line r1 = CalculateRay(pos.x, pos.y);
				F32 t1 = Maths::Max(0.f, intersectionPlane.Intersect(r1));

				Maths::C_Vertex p0 = r0.Point(t0);
				Maths::C_Vertex p1 = r1.Point(t1);

				Maths::HVec3 delta = p1-p0;

				Maths::HVec3 newPos = (Maths::HVec3&)m_StartTransform.AW();

				if (m_Flags & Centre || (m_Constraint && e.ControlDown()))
				{
					newPos.X(m_StartTransform.AW().X()+delta.X());
					newPos.Y(m_StartTransform.AW().Y()+delta.Y());
					newPos.Z(m_StartTransform.AW().Z()+delta.Z());
				}
				else
				{
					F32 s = delta|axis;
					if (m_Flags & Snap)
						s = Maths::Round(s/m_SnapMove)*m_SnapMove;
					newPos.X(m_StartTransform.AW().X()+s*axis.X());
					newPos.Y(m_StartTransform.AW().Y()+s*axis.Y());
					newPos.Z(m_StartTransform.AW().Z()+s*axis.Z());
				}
				m_Transform.AW((Maths::HVec4&)newPos);
			}
			else
			{
				Maths::HVec3 newPos = (Maths::HVec3&)m_StartTransform.AW();

				if (m_SnapFlags.PointSnap)
				{
					newPos = PointSnap(e.GetPosition(), newPos);
				}

				else if (m_SnapFlags.EdgeSnap)
				{
					if(FindClosestEdge(e.GetPosition()))
					{
						Maths::HVec3 edgeDir = m_pEdge->end - m_pEdge->begin;
						F32 edgeLen = edgeDir.Length();
						edgeDir.Normalise();

						Maths::C_Plane intersectionPlane;
						{
							const AftkRender::C_Camera& camera = m_pViewport->GetCamera();
							Maths::HVec3 planeNormal = camera.GetFocusPosition()-camera.GetPosition();

							Maths::HVec3 planeAxis = edgeDir^planeNormal;
							planeAxis.Normalise();

							planeNormal = edgeDir^planeAxis;
							planeNormal.Normalise();

							intersectionPlane = Maths::C_Plane(Maths::C_Normal(planeNormal), Maths::C_Vertex(m_pEdge->begin));
						}

						Maths::C_Line r0 = CalculateRay(m_ClickPosition.x, m_ClickPosition.y);
						F32 t0 = Maths::Max(0.f, intersectionPlane.Intersect(r0));

						wxPoint pos = e.GetPosition();
						Maths::C_Line r1 = CalculateRay(pos.x, pos.y);
						F32 t1 = Maths::Max(0.f, intersectionPlane.Intersect(r1));

						Maths::C_Vertex p0 = r0.Point(t0);
						Maths::C_Vertex p1 = r1.Point(t1);

						Maths::HVec3 offset = p0.m_V - m_pEdge->begin;
						Maths::HVec3 delta = p1-p0;

						F32 s = Maths::Clamp((F32)(offset|edgeDir) + (F32)(delta|edgeDir), 0.f, edgeLen);
						newPos = m_pEdge->begin + s*edgeDir;
					}
				}
				else if (m_SnapFlags.GridSnap)
				{
					newPos = GridSnap(e.GetPosition(), newPos);
				}

				if (!(m_Flags & Centre || (m_Constraint && e.ControlDown())))
				{
					Maths::HVec3 axis;
					if (m_Flags & XAxis)
						axis = Maths::HVec3(m_Transform[0][0], m_Transform[0][1], m_Transform[0][2]);
					else if (m_Flags & YAxis)
						axis = Maths::HVec3(m_Transform[1][0], m_Transform[1][1], m_Transform[1][2]);
					else if (m_Flags & ZAxis)
						axis = Maths::HVec3(m_Transform[2][0], m_Transform[2][1], m_Transform[2][2]);
											

					if (axis.LengthSquared())
					{
						//only move along constraint axis
						Maths::HVec3 newVec = (axis | ((Maths::HVec3)m_StartTransform.AW()-newPos))*axis;
						newPos.X(m_StartTransform.AW().X()-newVec.X());
						newPos.Y(m_StartTransform.AW().Y()-newVec.Y());
						newPos.Z(m_StartTransform.AW().Z()-newVec.Z());
					}
				}
			
				m_Transform.AW((Maths::HVec4&)newPos);
			}

			if (m_pContext)
			{
				Maths::HVec3 delta = Maths::HVec3(
					m_Transform[3][0]-m_StartTransform[3][0],
					m_Transform[3][1]-m_StartTransform[3][1],
					m_Transform[3][2]-m_StartTransform[3][2]);

				((I_VectorContext*)m_pContext)->Update(delta);
			}

			C_RenderCore::Instance().RefreshAllViewports();
			return;
		}
	}
	else if (m_Flags & Visible)
	{
		//we're not active...
		wxPoint pos = e.GetPosition();
	 		
		m_HoverFlags = PickAxis(pos);

		if (e.LeftDown())
			m_Flags &= Visible|Snap;

		if (((e.LeftDown() && m_HoverFlags) || e.MiddleDown()) && !e.Dragging())
		{
			m_Flags &= ~(XAxis|YAxis|ZAxis|Centre);
			if (m_HoverFlags)
				m_Flags |= m_HoverFlags;
			else
				m_Flags |= Active|(previousState & (XAxis|YAxis|ZAxis|Centre));

			if (e.ControlDown())
			{
				m_Constraint =	(m_Flags & XAxis) ? E_Constraint_YZ :
										(m_Flags & YAxis) ? E_Constraint_XZ :
										(m_Flags & ZAxis) ? E_Constraint_XY :
										E_Constraint_None;
			}
			
			if (!wxWindow::GetCapture())
			{
				m_Flags |= Active;
				m_StartTransform = m_Transform;
				m_ClickPosition = pos;
				m_pViewport->CaptureMouse();
			}
		}
	}

	if (!(m_Flags & Active))
		C_SelectManipulator::OnMouseEvent(e);

	if (previousState != m_Flags || previousHoverState != m_HoverFlags || e.ControlDown())
		C_RenderCore::Instance().RefreshAllViewports();
}

U8 C_MoveManipulator::PickAxis(const wxPoint& pos)
{
	Maths::C_Line ray = CalculateRay(pos.x, pos.y);
	const Maths::HVec3 position = Maths::HVec3(m_Transform[3][0], m_Transform[3][1], m_Transform[3][2]);;
	F32 scale = GetRenderScale(*m_pViewport, position);
	F32 rayDistanceToCentre = ray.Distance(Maths::C_Vertex(position));

	if (rayDistanceToCentre < scale)
	{
		F32 relativeScale = (80.f/m_screenSize);	//scaling relative to a screen size of 80 pixels
		//mouse cursor within range...work out active component
		if (rayDistanceToCentre < scale*relativeScale*0.15f)
			return Centre;
		else
		{
			Maths::HVec3 axisX(m_Transform[0][0], m_Transform[0][1], m_Transform[0][2]);
			Maths::HVec3 axisY(m_Transform[1][0], m_Transform[1][1], m_Transform[1][2]);
			Maths::HVec3 axisZ(m_Transform[2][0], m_Transform[2][1], m_Transform[2][2]);

			Maths::C_Line axisXLine = Maths::C_Line(Maths::C_Vertex(position), Maths::C_Vector(axisX));
			Maths::C_Line axisYLine = Maths::C_Line(Maths::C_Vertex(position), Maths::C_Vector(axisY));
			Maths::C_Line axisZLine = Maths::C_Line(Maths::C_Vertex(position), Maths::C_Vector(axisZ));


			//calculate ray-axis distances. ignore if colinear
			Maths::HVec3 rayNorm = ray.Normal().m_V;
			const F32 d = Maths::Cos(Maths::DegToRad(5.f));

			F32 rayDistanceX = ((rayNorm|axisX) < d) ? ray.Distance(axisXLine) : Maths::Constants<F32>::Max();
			F32 rayDistanceY = ((rayNorm|axisY) < d) ? ray.Distance(axisYLine) : Maths::Constants<F32>::Max();
			F32 rayDistanceZ = ((rayNorm|axisZ) < d) ? ray.Distance(axisZLine) : Maths::Constants<F32>::Max();

			F32 intersectX   = axisXLine.Intersect(ray);
			F32 intersectY   = axisYLine.Intersect(ray);
			F32 intersectZ   = axisZLine.Intersect(ray);

			//work out nearest axis
			U8 axis = 0;
			F32 minDist = Maths::Constants<F32>::Max();
			if (rayDistanceX < scale*0.1f && rayDistanceX < minDist && intersectX > 0.f)
			{
				axis = XAxis;
				minDist = rayDistanceX;
			}
			if (rayDistanceY < scale*0.1f && rayDistanceY < minDist  && intersectY > 0.f)
			{
				axis = YAxis;
				minDist = rayDistanceY;
			}
			if (rayDistanceZ < scale*0.1f && rayDistanceZ < minDist && intersectZ > 0.f)
			{
				axis = ZAxis;
				minDist = rayDistanceZ;
			}
			
			return axis;
		}
	}

	return 0;
}

void C_MoveManipulator::OnMouseCaptureLostEvent(wxMouseCaptureLostEvent &e)
{
	if (m_Flags & Active && wxWindow::GetCapture() == m_pViewport)
		m_pViewport->ReleaseMouse();

	e.Skip();
}

void C_MoveManipulator::OnKeyDown(wxKeyEvent& e)
{
	//may need to move this into C_ShortcutHandler, or connect to C_App for event to work globally
	if (e.GetKeyCode() == 'J' && !m_SnapToggle)
	{
		(m_Flags & Snap) ? m_Flags &= ~Snap : m_Flags |= Snap;

		C_ManipulatorPropertyPanel *pPropertyPanel = m_pToolManager->GetAttachedPropertyPanel();
		if (pPropertyPanel)
			pPropertyPanel->RefreshProperties();
		m_SnapToggle = true;
	}
	else
		e.Skip();
}

void C_MoveManipulator::OnKeyUp(wxKeyEvent& e)
{
	if (e.GetKeyCode() == 'J' && m_SnapToggle)
	{
		(m_Flags & Snap) ? m_Flags &= ~Snap : m_Flags |= Snap;

		C_ManipulatorPropertyPanel *pPropertyPanel = m_pToolManager->GetAttachedPropertyPanel();
		if (pPropertyPanel)
			pPropertyPanel->RefreshProperties();
		m_SnapToggle = false;
	}
	else
		e.Skip();
}

void C_MoveManipulator::OnSelectionChanged()
{
	__super::OnSelectionChanged();

	if (m_Flags & Active && wxWindow::GetCapture() == m_pViewport)
		m_pViewport->ReleaseMouse();

	m_Flags &= ~Active;
	if (GetInitialMatrix(m_Transform, m_space))
	{
		m_StartTransform = m_Transform;
		if (m_pContext)
			m_Flags |= Visible;
		else
			m_Flags &= ~Visible;
	}
	else
		m_Flags &= ~Visible;

	if ((m_Flags & (XAxis | YAxis | ZAxis)) == 0) {
		m_Flags |= Centre;
	}

	//Clear constraint upon selection change.
	if (m_pToolManager->IsModeEnabled(E_ManipulatorMode_EdgeSnap))
		m_Constraint = E_Constraint_None;
}

void C_MoveManipulator::EnableSnapMove(bool snap, F32 value)
{
	m_SnapMove = Maths::Abs(value);
	if (snap)
		m_Flags |= Snap;
	else
		m_Flags &= ~Snap;
}

bool C_MoveManipulator::SnapEnabled() const
{
	return ((m_Flags & Snap) > 0);
}

F32 C_MoveManipulator::SnapValue() const
{
	return m_SnapMove;
}

void C_MoveManipulator::Render(const C_RenderViewport *pViewport) const
{
	C_SelectManipulator::Render(pViewport);
	if (!pViewport || !(m_Flags & Visible))
		return;

	const Maths::HVec3 position = Maths::HVec3(m_Transform[3][0], m_Transform[3][1], m_Transform[3][2]);;
	F32 scale = GetRenderScale(*pViewport, position);

	Display::C_Matrix worldMatrix;
	Display::MatrixSetIdentity(&worldMatrix);
	worldMatrix._11 *= scale;
	worldMatrix._22 *= scale;
	worldMatrix._33 *= scale;
	worldMatrix._41 = position[0];
	worldMatrix._42 = position[1];
	worldMatrix._43 = position[2];


	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&worldMatrix, 4);	

	//orthonormalise camera axii
	const AftkRender::C_Camera& camera = pViewport->GetCamera();
	Maths::HVec3 toZ= camera.GetFocusPosition()-camera.GetPosition();
	toZ.Normalise();

	Maths::HVec3 toX = camera.GetUpVector()^toZ;
	toX.Normalise();

	Maths::HVec3 toY = toZ^toX;
	toY.Normalise();

	Maths::HVec3 axisX(m_Transform[0][0], m_Transform[0][1], m_Transform[0][2]);
	Maths::HVec3 axisY(m_Transform[1][0], m_Transform[1][1], m_Transform[1][2]);
	Maths::HVec3 axisZ(m_Transform[2][0], m_Transform[2][1], m_Transform[2][2]);

	axisX.Normalise();
	axisY.Normalise();
	axisZ.Normalise();
	
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
	//flip z-test to render occluded regions
	if (Display::Device()->GetDepthFlip())
		Display::Device()->SetDepthFunc(Display::E_CmpFunc_Less);
	else
		Display::Device()->SetDepthFunc(Display::E_CmpFunc_Greater);

	//the stencil pattern and reference values should already be initialised
	Display::Device()->SetStencilEnable(Display::E_Bool_True);

	//draw stippled occluded regions
	DrawManipulator(axisX, axisY, axisZ, toX, toY, toZ);

	Display::Device()->SetStencilEnable(Display::E_Bool_False);
	Display::Device()->SetDepthFunc(Display::E_CmpFunc_Default);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);

	//draw normally
	DrawManipulator(axisX, axisY, axisZ, toX, toY, toZ);
}

void C_MoveManipulator::DrawManipulator(
	const Maths::HVec3& axisX, const Maths::HVec3& axisY, const Maths::HVec3& axisZ,
	const Maths::HVec3& toX, const Maths::HVec3& toY, const Maths::HVec3& toZ) const
{
	if (!m_pViewport)
		return;

	bool coordFlip = ((axisX^axisY)|axisZ) > 0.f ? false : true;
	if (coordFlip)
		Display::Device()->SetCullMode(Display::E_CullMode_Cw);


	//calculate vertices for centre section
	Maths::HVec3 p0, p1, p2, p3;

	F32 handleScale;
	F32 relativeScale = (80.f/m_screenSize);	//scaling relative to a screen size of 80 pixels
	if (m_pToolManager->IsNormalMode())
		handleScale = 0.125f*relativeScale;
	else
		handleScale = 0.125*0.5f*relativeScale;

	if (m_pViewport->GetCamera().GetType() == AftkRender::C_Camera::E_CameraType_Perspective)
	{
		switch (m_Constraint)
		{
		case E_Constraint_XY:
			p0 =  handleScale*axisX + handleScale*axisY;
			p1 = (coordFlip) ? -handleScale*axisX + handleScale*axisY :  handleScale*axisX - handleScale*axisY;
			p2 = (coordFlip) ? -handleScale*axisX - handleScale*axisY : -handleScale*axisX - handleScale*axisY;
			p3 = (coordFlip) ?  handleScale*axisX - handleScale*axisY : -handleScale*axisX + handleScale*axisY;
			break;
		case E_Constraint_XZ:
			p0 =  handleScale*axisX + handleScale*axisZ;
			p1 = (coordFlip) ? -handleScale*axisX + handleScale*axisZ :  handleScale*axisX - handleScale*axisZ;
			p2 = (coordFlip) ? -handleScale*axisX - handleScale*axisZ : -handleScale*axisX - handleScale*axisZ;
			p3 = (coordFlip) ?  handleScale*axisX - handleScale*axisZ : -handleScale*axisX + handleScale*axisZ;
			break;
		case E_Constraint_YZ:
			p0 =  handleScale*axisY + handleScale*axisZ;
			p1 = (coordFlip) ? -handleScale*axisY + handleScale*axisZ :  handleScale*axisY - handleScale*axisZ;
			p2 = (coordFlip) ? -handleScale*axisY - handleScale*axisZ : -handleScale*axisY - handleScale*axisZ;
			p3 = (coordFlip) ?  handleScale*axisY - handleScale*axisZ : -handleScale*axisY + handleScale*axisZ;
			break;
		default:
			p0 =  handleScale*toX + handleScale*toY;
			p1 = (coordFlip) ? -handleScale*toX + handleScale*toY :  handleScale*toX - handleScale*toY;
			p2 = (coordFlip) ? -handleScale*toX - handleScale*toY : -handleScale*toX - handleScale*toY;
			p3 = (coordFlip) ?  handleScale*toX - handleScale*toY : -handleScale*toX + handleScale*toY;
			break;
		};
	}
	else
	{
		p0 =  handleScale*toX + handleScale*toY;
		p1 = (coordFlip) ? -handleScale*toX + handleScale*toY :  handleScale*toX - handleScale*toY;
		p2 = (coordFlip) ? -handleScale*toX - handleScale*toY : -handleScale*toX - handleScale*toY;
		p3 = (coordFlip) ?  handleScale*toX - handleScale*toY : -handleScale*toX + handleScale*toY;
	}

	{
		BindRender3D();
		U32 nVertices = (m_Flags & Active) ? (m_Flags & Centre) ? 6 : 6*Tessellation+6 : 6*3*Tessellation+6;
		AftkRender::Vertex_XYZ2_C *pVertex = (AftkRender::Vertex_XYZ2_C*) Display::Device()->BeginVertices(
			Display::E_PrimitiveType_TriangleList, nVertices, sizeof(AftkRender::Vertex_XYZ2_C), 
			AftkRender::Vertex_XYZ2_C::m_pDeclaration, true);

		if (pVertex)
		{
			F32 r=0.075f*relativeScale;	//cone radius
			U32 colour = 0x00000000f;
			F32 cAPrev=1.f, sAPrev=0.f;

			//x axis arrow head
			if (!(m_Flags & Active) || ((m_Flags & Active) && (m_Flags & XAxis)))
			{
				cAPrev=1.f, sAPrev=0.f;
				colour = (m_Flags & Active) ? 0xffffffff : (m_HoverFlags & XAxis) ? 0xff43ffa3 : (m_Flags & XAxis) ? 0xffffff00 : 0xffff0000;

				for(U32 i=0; i<Tessellation; ++i)
				{
					F32 theta=(i+1)*Maths::Constants<F32>::Pi()*2.f/Tessellation;

					F32 cA = cosf(-theta);
					F32 sA = sinf(-theta);

					Maths::HVec3 vec0 = sAPrev*axisY + cAPrev*axisZ;
					Maths::HVec3 vec1 = sA*axisY + cA*axisZ;
					Maths::HVec3 vec2 = 0.5*(vec0+vec1);

					Maths::HVec3 v0 = (1.f-0.25*relativeScale)*axisX + r*vec0;
					Maths::HVec3 v1 = (1.f-0.25*relativeScale)*axisX + r*vec1;

					SetVertex(pVertex++, axisX.X(), axisX.Y(), axisX.Z(), vec2.X(), vec2.Y(), vec2.Z(), colour);
					SetVertex(pVertex++, v0.X(), v0.Y(), v0.Z(), vec0.X(), vec0.Y(), vec0.Z(), colour);
					SetVertex(pVertex++, v1.X(), v1.Y(), v1.Z(), vec1.X(), vec1.Y(), vec1.Z(), colour);

					SetVertex(pVertex++, v1.X(), v1.Y(), v1.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
					SetVertex(pVertex++, v0.X(), v0.Y(), v0.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
					SetVertex(pVertex++, (1.f-0.25f*relativeScale)*axisX.X(), (1.f-0.25f*relativeScale)*axisX.Y(), (1.f-0.25f*relativeScale)*axisX.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);

					cAPrev = cA;
					sAPrev = sA;
				}

			}

			if (!(m_Flags & Active) || ((m_Flags & Active) && (m_Flags & YAxis)))
			{
				//y axis arrow head
				cAPrev=1.f, sAPrev=0.f;
				colour = (m_Flags & Active) ? 0xffffffff : (m_HoverFlags & YAxis) ? 0xff43ffa3 : (m_Flags & YAxis) ? 0xffffff00 : 0xff00ff00;
				for(U32 i=0; i<Tessellation; ++i)
				{
					F32 theta=(i+1)*Maths::Constants<F32>::Pi()*2.f/Tessellation;

					F32 cA = cosf(-theta);
					F32 sA = sinf(-theta);

					Maths::HVec3 vec0 = sAPrev*axisZ + cAPrev*axisX;
					Maths::HVec3 vec1 = sA*axisZ + cA*axisX;
					Maths::HVec3 vec2 = 0.5*(vec0+vec1);

					Maths::HVec3 v0 = (1.f-0.25*relativeScale)*axisY + r*vec0;
					Maths::HVec3 v1 = (1.f-0.25*relativeScale)*axisY + r*vec1;
				
					SetVertex(pVertex++, axisY.X(), axisY.Y(), axisY.Z(), vec2.X(), vec2.Y(), vec2.Z(), colour);
					SetVertex(pVertex++, v0.X(), v0.Y(), v0.Z(), vec0.X(), vec0.Y(), vec0.Z(), colour);
					SetVertex(pVertex++, v1.X(), v1.Y(), v1.Z(), vec1.X(), vec1.Y(), vec1.Z(), colour);

					SetVertex(pVertex++, v1.X(), v1.Y(), v1.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
					SetVertex(pVertex++, v0.X(), v0.Y(), v0.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
					SetVertex(pVertex++, (1.f-0.25f*relativeScale)*axisY.X(), (1.f-0.25f*relativeScale)*axisY.Y(), (1.f-0.25f*relativeScale)*axisY.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);

					cAPrev = cA;
					sAPrev = sA;
				}
			}

			if (!(m_Flags & Active) || ((m_Flags & Active) && (m_Flags & ZAxis)))
			{
				//z axis arrow head
				cAPrev=1.f, sAPrev=0.f;
				colour = (m_Flags & Active) ? 0xffffffff : (m_HoverFlags & ZAxis) ? 0xff43ffa3 : (m_Flags & ZAxis) ? 0xffffff00 : 0xff0000ff;
				for(U32 i=0; i<Tessellation; ++i)
				{
					F32 theta=(i+1)*Maths::Constants<F32>::Pi()*2.f/Tessellation;

					F32 cA = cosf(-theta);
					F32 sA = sinf(-theta);
					
					Maths::HVec3 vec0 = sAPrev*axisX + cAPrev*axisY;
					Maths::HVec3 vec1 = sA*axisX + cA*axisY;
					Maths::HVec3 vec2 = 0.5*(vec0+vec1);

					Maths::HVec3 v0 = (1.f-0.25*relativeScale)*axisZ + r*vec0;
					Maths::HVec3 v1 = (1.f-0.25*relativeScale)*axisZ + r*vec1;

					SetVertex(pVertex++, axisZ.X(), axisZ.Y(), axisZ.Z(), vec2.X(), vec2.Y(), vec2.Z(), colour);
					SetVertex(pVertex++, v0.X(), v0.Y(), v0.Z(), vec0.X(), vec0.Y(), vec0.Z(), colour);
					SetVertex(pVertex++, v1.X(), v1.Y(), v1.Z(), vec1.X(), vec1.Y(), vec1.Z(), colour);

					SetVertex(pVertex++, v1.X(), v1.Y(), v1.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
					SetVertex(pVertex++, v0.X(), v0.Y(), v0.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
					SetVertex(pVertex++, (1.f-0.25f*relativeScale)*axisZ.X(), (F32)axisZ.Y(), (1.f-0.25f*relativeScale)*axisZ.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);

					cAPrev = cA;
					sAPrev = sA;
				}
			}

			colour = (m_Flags & Active) ? 0x2200ffff : (m_HoverFlags & Centre) ? 0x2243ffa3 : (m_Flags & Centre) ? 0x22ffff00 : 0x2200ffff;
			SetVertex(pVertex++, p0.X(), p0.Y(), p0.Z(), -toZ.X(), -toZ.Y(), -toZ.Z(), colour);
			SetVertex(pVertex++, p1.X(), p1.Y(), p1.Z(), -toZ.X(), -toZ.Y(), -toZ.Z(), colour);
			SetVertex(pVertex++, p2.X(), p2.Y(), p2.Z(), -toZ.X(), -toZ.Y(), -toZ.Z(), colour);

			SetVertex(pVertex++, p0.X(), p0.Y(), p0.Z(), -toZ.X(), -toZ.Y(), -toZ.Z(), colour);
			SetVertex(pVertex++, p2.X(), p2.Y(), p2.Z(), -toZ.X(), -toZ.Y(), -toZ.Z(), colour);
			SetVertex(pVertex++, p3.X(), p3.Y(), p3.Z(), -toZ.X(), -toZ.Y(), -toZ.Z(), colour);

			Display::Device()->EndVertices(pVertex);
		}
	}

	//arrow shafts
	{
		Display::Device()->SetVertexShader(&Display::BasicRender()->m_VertexShader[Display::E_BasicRenderVertexShader_Coloured3D]);
		Display::Device()->SetPixelShader(&Display::BasicRender()->m_PixelShader[Display::E_BasicRenderPixelShader_Coloured]);	

		U32 nVertices = (m_Flags & Active) ? ((m_Flags & Centre) ? 14 : 10) : 14;
		Display::Vertex_XYZ_C *pVertex = (Display::Vertex_XYZ_C*) Display::Device()->BeginVertices(
			Display::E_PrimitiveType_LineList, nVertices, sizeof(Display::Vertex_XYZ_C), 
			Display::Vertex_XYZ_C::m_pDeclaration, true);

		if (pVertex)
		{
			U32 colour = 0x00000000f;
			
			if (!(m_Flags & Active) || ((m_Flags & Active) && (m_Flags & (XAxis|Centre))))
			{
				colour = (m_Flags & Active) ? 0xffffffff : (m_HoverFlags & XAxis) ? 0xff43ffa3 : (m_Flags & XAxis) ? 0xffffff00 : 0xffff0000;
				SetVertex(pVertex++, 0.f, 0.f, 0.f, colour);
				SetVertex(pVertex++, (1.f-0.25f*relativeScale)*axisX.X(), (1.f-0.25f*relativeScale)*axisX.Y(), (1.f-0.25f*relativeScale)*axisX.Z(), colour);
			}

			if (!(m_Flags & Active) || ((m_Flags & Active) && (m_Flags & (YAxis|Centre))))
			{
				colour = (m_Flags & Active) ? 0xffffffff : (m_HoverFlags & YAxis) ? 0xff43ffa3 : (m_Flags & YAxis) ? 0xffffff00 : 0xff00ff00;
				SetVertex(pVertex++, 0.f, 0.f, 0.f, colour);
				SetVertex(pVertex++, (1.f-0.25f*relativeScale)*axisY.X(), (1.f-0.25f*relativeScale)*axisY.Y(), (1.f-0.25f*relativeScale)*axisY.Z(), colour);
			}

			if (!(m_Flags & Active) || ((m_Flags & Active) && (m_Flags & (ZAxis|Centre))))
			{
				colour = (m_Flags & Active) ? 0xffffffff : (m_HoverFlags & ZAxis) ? 0xff43ffa3 : (m_Flags & ZAxis) ? 0xffffff00 : 0xff0000ff;
				SetVertex(pVertex++, 0.f, 0.f, 0.f, colour);
				SetVertex(pVertex++, (1.f-0.25f*relativeScale)*axisZ.X(), (1.f-0.25f*relativeScale)*axisZ.Y(), (1.f-0.25f*relativeScale)*axisZ.Z(), colour);
			}

			colour = (m_Flags & Active) ? 0xffffffff : (m_HoverFlags & Centre) ? 0xff43ffa3 : (m_Flags & Centre) ? 0xffffff00 : 0xff00ffff;
			SetVertex(pVertex++, p0.X(), p0.Y(), p0.Z(), colour);
			SetVertex(pVertex++, p1.X(), p1.Y(), p1.Z(), colour);

			SetVertex(pVertex++, p1.X(), p1.Y(), p1.Z(), colour);
			SetVertex(pVertex++, p2.X(), p2.Y(), p2.Z(), colour);

			SetVertex(pVertex++, p2.X(), p2.Y(), p2.Z(), colour);
			SetVertex(pVertex++, p3.X(), p3.Y(), p3.Z(), colour);

			SetVertex(pVertex++, p3.X(), p3.Y(), p3.Z(), colour);
			SetVertex(pVertex++, p0.X(), p0.Y(), p0.Z(), colour);

			Display::Device()->EndVertices(pVertex);
		}
	}
}

void C_MoveManipulator::UpdateContext(const Maths::HVec3& translate)
{
	if (!m_pContext)
		return;
	((I_VectorContext*)m_pContext)->Update(translate);
}

void C_MoveManipulator::ApplyContext(const Maths::HVec3 &translate)
{
	if (!m_pContext)
		return;

	if (((I_VectorContext*)m_pContext)->Apply(translate))
	{
		OnSelectionChanged();
		C_RenderCore::Instance().RefreshAllViewports();
	}
}

Maths::HVec3 C_MoveManipulator::PointSnap(
	const wxPoint& mousePos,
	const Maths::HVec3& manipulatorPos)
{
	Maths::HVec3 result(manipulatorPos);

	Maths::C_Line line = CalculateRay(mousePos.x, mousePos.y);
	Maths::C_Vertex a = line.Start();
	Maths::C_Vertex b = line.End();
	EditorLib::C_Ray ray;
	ray.origin = EditorLib::Vector3(a.X(), a.Y(), a.Z());
	ray.end = EditorLib::Vector3(b.X(), b.Y(), b.Z());
/*
	DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dLine(
		a,
		b, Maths::RGBA(255, 255, 0));
*/
	//Intersect camera ray with scene.
	bool found = false;
	static const U32 kMaxResults = 100;
	static EditorLib::C_RayResult rayResults[kMaxResults];
	EditorLib::C_RayResult rayBoundsResult;
	//Find closest intersection for null/locators.
	U32 nResults = 
		EditorLib::C_Core::Instance().GetCollisionManager().Intersect(
		rayResults,
		kMaxResults,
		ray,
		EditorLib::E_CollisionQueryType_Bounds,
		EditorLib::E_CollisionSourceType_Proxy);
	if (nResults > 0)
	{
		//Sort low to high
		std::sort(rayResults, rayResults + nResults);
		EditorLib::C_Selection selection;
		EditorLib::C_Core::Instance().GetSelection(selection);

		//Find closest intersection.
		bool foundNonSelectedSnapTarget = false;
		for (U32 i = 0; i < nResults; i++)
		{
			EditorLib::C_RayResult& tmpResult = rayResults[i];
			//If we're nearest and we're not a member of the active selection...
			if (tmpResult.distanceSquared < rayBoundsResult.distanceSquared &&
				!selection.Find(tmpResult.pDgNode->GetId()))
			{
				foundNonSelectedSnapTarget = true;
				rayBoundsResult = tmpResult;
				break;
			}
		}
		if (!foundNonSelectedSnapTarget)
			found = false;
		else
			found = true;
	}

	//Find intersection with geometry.

#ifdef RAY_FIND_TARGET
	EditorLib::C_RayResult geometryResult;
	nResults = EditorLib::C_Core::Instance().GetCollisionManager().Intersect(
		geometryResult,
		ray,
		EditorLib::E_CollisionQueryType_Complex);
	if (nResults > 0)
	{
		Maths::HVec3 inter = Maths::Load<Maths::HVec3>(
			(const MathsStorage::Float3*)&geometryResult.intersectionPoint);

		//DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dSphere(
//			(Maths::HVec3&)inter, 0.5, Maths::RGBA(255, 255, 0));
		found = true;
	}
#else
//	ray.origin = EditorLib::Vector3(154.58f, 20.94f, -378.84f);
//	ray.end = EditorLib::Vector3(-772.39, -2253.2468, 9647.09);
//	ray.origin = EditorLib::Vector3(66.686096, 146.08101, -498.03397);
//	ray.end = EditorLib::Vector3(1566.4163, -6581.0127, 7520.8594);

//	DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dLine(
//		Maths::HVec3(ray.origin.x, ray.origin.y, ray.origin.z),
//		Maths::HVec3(ray.end.x, ray.end.y, ray.end.z), Maths::RGBA(255, 255, 0));

	//	static EditorLib::Vector3 v0;
//	static EditorLib::Vector3 v1;
//	v0 = ray.origin;
//	v1 = ray.end;

	EditorLib::C_NearestPointResult geometryResult;
	if (EditorLib::C_Core::Instance().GetCollisionManager().NearestPoint(
		geometryResult,
		ray))
	{
		found = true;
	}
#endif

	if (found)
	{
		//Use null/locator
//		Cannot compare directly as are comparing against different origins
//		if (rayBoundsResult.distanceSquared < geometryResult.distanceSquared)
		if (rayBoundsResult.distanceSquared != FLT_MAX)
		{
			assert(rayBoundsResult.pCollisionInstance);
			const EditorLib::Vector3& worldPos = 
				(const EditorLib::Vector3&)rayBoundsResult.pCollisionInstance->toWorldSpace[3];
			result = Maths::HVec3(worldPos.x, worldPos.y, worldPos.z);
		}
		//Use geometry
		else
		{
#ifdef RAY_FIND_TARGET
			//Find closest vertex to intersection point.
			EditorLib::C_NearestPointResult pointResult;
			EditorLib::CollisionHelpers::NearestPoint(
				pointResult,
				rayGeometryResult);
			
			result = Maths::HVec3(pointResult.point.x, pointResult.point.y, pointResult.point.z);
#else
//		DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dSphere(
//			Maths::HVec3(geometryResult.point.x, geometryResult.point.y, geometryResult.point.z), 0.5, Maths::RGBA(255, 255, 0));
			result = Maths::HVec3(geometryResult.point.x, geometryResult.point.y, geometryResult.point.z);
#endif
		}
	}

	return result;
}

			 
Maths::HVec3 C_MoveManipulator::GridSnap(
	const wxPoint& mousePos,
	const Maths::HVec3& manipulatorPos)
{
	Maths::HVec3 result(manipulatorPos);

	Maths::C_Line line = CalculateRay(mousePos.x, mousePos.y);
	Maths::C_Vertex a = line.Start();
	Maths::C_Vertex b = line.End();

	if (m_pViewport->GetCamera().GetType() == AftkRender::C_Camera::E_CameraType_Perspective)
	{
		//snap to xz grid
		Maths::C_Plane intersectionPlane = Maths::C_Plane(Maths::C_Normal(0.f, 1.f, 0.f), Maths::C_Vertex(0.f, 0.f, 0.f));
		F32 t = intersectionPlane.Intersect(line);
		if (t < 0.f)
			return manipulatorPos;
		
		result = line.Point(t).m_V;
		F32 gridspacing = C_RenderCore::Instance().GetRenderPipeline().GetGlobalRenderState().gridSpacing;
		result[0] = Maths::Round(result[0]/gridspacing)*gridspacing;
		result[1] = 0.f;
		result[2] = Maths::Round(result[2]/gridspacing)*gridspacing;
	}
	else
	{
		Maths::HVec3 planeNormal = m_pViewport->GetCamera().GetFocusPosition()-m_pViewport->GetCamera().GetPosition();
		planeNormal.Normalise();
		Maths::C_Plane intersectionPlane = Maths::C_Plane(Maths::C_Normal(planeNormal), manipulatorPos);
		F32 t = intersectionPlane.Intersect(line);
		if (t < 0.f)
			return manipulatorPos;

		result = line.Point(t).m_V;
		F32 gridspacing = C_RenderCore::Instance().GetRenderPipeline().GetGlobalRenderState().gridSpacing;
		if (Maths::Abs(planeNormal|Maths::HVec3(1.f, 0.f, 0.f)) < Maths::Constants<F32>::Epsilon())
			result[0] = Maths::Round(result[0]/gridspacing)*gridspacing;

		if (Maths::Abs(planeNormal|Maths::HVec3(0.f, 1.f, 0.f)) < Maths::Constants<F32>::Epsilon())
			result[1] = Maths::Round(result[1]/gridspacing)*gridspacing;


		if (Maths::Abs(planeNormal|Maths::HVec3(0.f, 0.f, 1.f)) < Maths::Constants<F32>::Epsilon())
			result[2] = Maths::Round(result[2]/gridspacing)*gridspacing;
	}

	return result;
}

bool C_MoveManipulator::FindClosestEdge(const wxPoint& mousePos)
{
	//If we've already got a edge cached, use it
	if(m_pEdge)
		return true;
			   
	Maths::C_Line line = CalculateRay(mousePos.x, mousePos.y);
	Maths::C_Vertex a = line.Start();
	Maths::C_Vertex b = line.End();
	EditorLib::C_Ray ray;
	ray.origin = EditorLib::Vector3(a.X(), a.Y(), a.Z());
	ray.end = EditorLib::Vector3(b.X(), b.Y(), b.Z());

	EditorLib::C_RayResult rayResult;
	if (EditorLib::C_Core::Instance().GetCollisionManager().Intersect(
		rayResult,
		ray, 
		EditorLib::E_CollisionQueryType_Complex))
	{
		if (rayResult.pCollisionInstance->pSource->GetType() == 
			EditorLib::E_CollisionSourceType_Mesh)
		{
			EditorLib::C_NearestPointResult nearestPointResult;

			EditorLib::C_CollisionMesh* pMesh = 
				(EditorLib::C_CollisionMesh*)rayResult.pCollisionInstance->pSource;
			
			EditorLib::CollisionHelpers::NearestPointOnEdge(
				nearestPointResult, 
				rayResult.intersectionPoint,
				pMesh,
				rayResult.triIndex,
				rayResult.pCollisionInstance->toObjectSpace,
				rayResult.pCollisionInstance->toWorldSpace);
			
			if (nearestPointResult.distanceSquared < 1.0f)
			{
				//Store the edge we'll be snapping to.
				Maths::C_Vertex a = Maths::Load<Maths::C_Vertex>(
					(const MathsStorage::Float3*)&pMesh->pVertices[nearestPointResult.adjacentIdx1]);
				Maths::C_Vertex b = Maths::Load<Maths::C_Vertex>(
					(const MathsStorage::Float3*)&pMesh->pVertices[nearestPointResult.adjacentIdx2]);
				//Transform edge into world space.
				Maths::HMat44 wM = Maths::Load<Maths::HMat44>((const MathsStorage::Float44*)&rayResult.pCollisionInstance->toWorldSpace);
				a = a * wM;
				b = b * wM;
				
				//Create line
				m_pEdge = new EditorLib::C_CollisionEdge();
				m_pEdge->begin = a;
				m_pEdge->end = b;
				
				//Debug rendering.
				/*
				DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dLine(
					m_pEdge->begin,
					m_pEdge->end, Maths::RGBA(255, 255, 0));*/


				return true;
			}
		}
	}
	return false;
}