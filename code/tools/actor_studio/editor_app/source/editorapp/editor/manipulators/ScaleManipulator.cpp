#include "stdafx.h"

#include "Display/Buffer/VertexDeclarations.h"
#include "Display/Shaders/Constants.h"

#include "shaders/vsConstants.h"
#include "shaders/psConstants.h"

#include <EditorApp/Editor/manipulators/contexts/IManipulatorContext.h>
#include <EditorApp/Editor/manipulators/contexts/IVectorContext.h>
#include <EditorApp/Editor/manipulators/Manipulator.h>
#include <EditorApp/Editor/manipulators/SelectManipulator.h>
#include <EditorApp/Editor/manipulators/ScaleManipulator.h>
#include <EditorApp/Editor/RenderViewport.h>
#include <EditorApp/Editor/RenderCore.h>

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

BEGIN_EVENT_TABLE(C_ScaleManipulator, C_SelectManipulator)
  EVT_MOUSE_EVENTS(C_ScaleManipulator::OnMouseEvent)
  EVT_MOUSE_CAPTURE_LOST(C_ScaleManipulator::OnMouseCaptureLostEvent)
  EVT_KEY_DOWN(C_ScaleManipulator::OnKeyDown)
  EVT_KEY_UP(C_ScaleManipulator::OnKeyUp)
END_EVENT_TABLE()
C_ScaleManipulator::C_ScaleManipulator()
: C_SelectManipulator()
, m_Flags(0)
, m_HoverFlags(0)
, m_Scale(1.f)
, m_ScaleSnap(1.f)
, m_SnapToggle(false)
{
}

C_ScaleManipulator::~C_ScaleManipulator()
{
}

E_ManipulatorType C_ScaleManipulator::GetType() const
{
	return E_ManipulatorType_Scale;
}

bool C_ScaleManipulator::IsA(E_ManipulatorType type) const
{
	if (__super::IsA(type))
		return true;
	if (type == C_ScaleManipulator::GetType()) 
		return true;

	return false;
}

void C_ScaleManipulator::OnMouseEvent(wxMouseEvent &e)
{
	if (C_SelectManipulator::IsActive())
		return C_SelectManipulator::OnMouseEvent(e);

	U8 previousState = m_Flags;
	U8 previousHoverState = m_HoverFlags;

	if (m_Flags & Active)
	{
		if (e.LeftUp() || e.MiddleUp())
		{
			if (wxWindow::GetCapture() == m_pViewport)
				m_pViewport->ReleaseMouse();

			m_Flags = m_Flags & ~Active;

			if (m_Flags & Centre)
				ApplyContext(Maths::HVec3(m_Scale, m_Scale, m_Scale));
			else if (m_Flags & XAxis)
				ApplyContext(Maths::HVec3(m_Scale, 1.f, 1.f));
			else if (m_Flags & YAxis)
				ApplyContext(Maths::HVec3(1.f, m_Scale, 1.f));
			else if (m_Flags & ZAxis)
				ApplyContext(Maths::HVec3(1.f, 1.f, m_Scale));

			m_Flags = previousState & ~Active;
			m_StartTransform = m_Transform;
		}
		else
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

			Maths::HVec3 axisX(m_StartTransform.AX()[0], m_StartTransform.AX()[1], m_StartTransform.AX()[2]);
			Maths::HVec3 axisY(m_StartTransform.AY()[0], m_StartTransform.AY()[1], m_StartTransform.AY()[2]);
			Maths::HVec3 axisZ(m_StartTransform.AZ()[0], m_StartTransform.AZ()[1], m_StartTransform.AZ()[2]);

			if (m_Flags & Centre)
				intersectionPlane = Maths::C_Plane(Maths::C_Normal(toZ), Maths::C_Vertex(m_StartTransform.AW()[0], m_StartTransform.AW()[1], m_StartTransform.AW()[2]));
			else
			{
				if (m_Flags & XAxis)
					axis = axisX;
				else if (m_Flags & YAxis)
					axis = axisY;
				else if (m_Flags & ZAxis)
					axis = axisZ;
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
			F32 t0 = intersectionPlane.Intersect(r0);

			Maths::C_Line r1 = CalculateRay(pos.x, pos.y);
			F32 t1 = intersectionPlane.Intersect(r1);

			Maths::C_Vertex p0 = r0.Point(t0);
			Maths::C_Vertex p1 = r1.Point(t1);
			Maths::HVec3 d = p1-p0;

			const Maths::HVec3 position = Maths::HVec3(m_StartTransform[3][0], m_StartTransform[3][1], m_StartTransform[3][2]);
			F32 rcpSize = 1.f/GetRenderScale(*m_pViewport, position);

			if (m_Flags & Centre)
				m_Scale = 1.f + rcpSize*(d|toX);
			else
				m_Scale = 1.f + rcpSize*(d|axis);

			if (m_Flags & Snap)
				m_Scale = Maths::Round(m_Scale/m_ScaleSnap)*m_ScaleSnap;

			if (m_Flags & Centre)
			{
				axisX *= m_Scale;
				axisY *= m_Scale;
				axisZ *= m_Scale;
				UpdateContext(Maths::HVec3(m_Scale, m_Scale, m_Scale));
			}
			else
			{
				F32 xProjection(Maths::Abs(axisX|axis)), yProjection(Maths::Abs(axisY|axis)), zProjection(Maths::Abs(axisZ|axis));
				xProjection = (xProjection<Maths::Constants<F32>::Epsilon()*axisX.Length()) ? 0.f : xProjection;
				yProjection = (yProjection<Maths::Constants<F32>::Epsilon()*axisY.Length()) ? 0.f : yProjection;
				zProjection = (zProjection<Maths::Constants<F32>::Epsilon()*axisZ.Length()) ? 0.f : zProjection;

				axisX = m_Scale*xProjection*axis + (1.f-xProjection/axisX.Length())*axisX;
				axisY = m_Scale*yProjection*axis + (1.f-yProjection/axisY.Length())*axisY;
				axisZ = m_Scale*zProjection*axis + (1.f-zProjection/axisZ.Length())*axisZ;
				
				if (m_Flags & XAxis)
					UpdateContext(Maths::HVec3(m_Scale, 1.f, 1.f));
				else if (m_Flags & YAxis)
					UpdateContext(Maths::HVec3(1.f, m_Scale, 1.f));
				else if (m_Flags & ZAxis)
					UpdateContext(Maths::HVec3(1.f, 1.f, m_Scale));
			}

			m_Transform[0][0] = axisX.X(); m_Transform[0][1] = axisX.Y(); m_Transform[0][2] = axisX.Z();
			m_Transform[1][0] = axisY.X(); m_Transform[1][1] = axisY.Y(); m_Transform[1][2] = axisY.Z();
			m_Transform[2][0] = axisZ.X(); m_Transform[2][1] = axisZ.Y(); m_Transform[2][2] = axisZ.Z();

			C_RenderCore::Instance().RefreshAllViewports();
			return;
		}
	}
	else if (m_Flags & Visible)
	{
		if (e.LeftDown())
			//reset axis in case the selection subsequently changes
			m_Flags &= Visible|Snap;

		m_HoverFlags = 0;

		//we perform the rendering using the start transform instead of the current transform, so it needs to be kept up to date..
		m_StartTransform = m_Transform;

		wxPoint pos = e.GetPosition();
		Maths::C_Line ray = CalculateRay(pos.x, pos.y);

		const Maths::HVec3 position = Maths::HVec3(m_StartTransform[3][0], m_StartTransform[3][1], m_StartTransform[3][2]);
		F32 scale = GetRenderScale(*m_pViewport, position);
		F32 rayDistanceToCentre = ray.Distance(Maths::C_Vertex(m_Transform.AW()[0], m_Transform.AW()[1], m_Transform.AW()[2]));

		if (rayDistanceToCentre < scale*1.1f || e.MiddleDown())
		{
			F32 relativeScale = (80.f/m_screenSize);	//scaling relative to a screen size of 80 pixels
			//mouse cursor within range...work out active component
			if (rayDistanceToCentre < scale*relativeScale*0.075f)
				m_HoverFlags |= Centre;
			else
			{
				Maths::HVec3 xAxisCubeCentre(Maths::HVec3(m_Transform.AX()[0], m_Transform.AX()[1], m_Transform.AX()[2]));
				Maths::HVec3 yAxisCubeCentre(Maths::HVec3(m_Transform.AY()[0], m_Transform.AY()[1], m_Transform.AY()[2]));
				Maths::HVec3 zAxisCubeCentre(Maths::HVec3(m_Transform.AZ()[0], m_Transform.AZ()[1], m_Transform.AZ()[2]));

				xAxisCubeCentre.Normalise();
				xAxisCubeCentre *= scale;
				xAxisCubeCentre += Maths::HVec3(m_Transform.AW()[0], m_Transform.AW()[1], m_Transform.AW()[2]);

				yAxisCubeCentre.Normalise();
				yAxisCubeCentre *= scale;
				yAxisCubeCentre += Maths::HVec3(m_Transform.AW()[0], m_Transform.AW()[1], m_Transform.AW()[2]);

				zAxisCubeCentre.Normalise();
				zAxisCubeCentre *= scale;
				zAxisCubeCentre += Maths::HVec3(m_Transform.AW()[0], m_Transform.AW()[1], m_Transform.AW()[2]);

				F32 distanceX = ray.Distance(Maths::C_Vertex(xAxisCubeCentre));
				F32 distanceY = ray.Distance(Maths::C_Vertex(yAxisCubeCentre));
				F32 distanceZ = ray.Distance(Maths::C_Vertex(zAxisCubeCentre));

				if (distanceX < scale*relativeScale*0.075f)
					m_HoverFlags |= XAxis;
				else if (distanceY < scale*relativeScale*0.075f)
					m_HoverFlags |= YAxis;
				else if (distanceZ < scale*relativeScale*0.075f)
					m_HoverFlags |= ZAxis;
				else
					m_HoverFlags = 0;
			}

			if (((e.LeftDown() && m_HoverFlags) || e.MiddleDown()) && !e.Dragging() && !wxWindow::GetCapture())
			{
				m_Flags &= ~(XAxis|YAxis|ZAxis|Centre);

				if (m_HoverFlags)
					m_Flags |= Active|m_HoverFlags;
				else
					m_Flags |= Active|(previousState & (XAxis|YAxis|ZAxis|Centre));

				m_ClickPosition = pos;
				m_Scale = 1.f;
				m_pViewport->CaptureMouse();
			}
		}
	}

	if (!(m_Flags & Active))
		C_SelectManipulator::OnMouseEvent(e);
	else
		e.Skip();

	if (previousState != m_Flags || previousHoverState != m_HoverFlags)
		C_RenderCore::Instance().RefreshAllViewports();
}

void C_ScaleManipulator::OnMouseCaptureLostEvent(wxMouseCaptureLostEvent &e)
{
	if (m_Flags & Active && wxWindow::GetCapture() == m_pViewport)
		m_pViewport->ReleaseMouse();

	e.Skip();
}

void C_ScaleManipulator::OnKeyDown(wxKeyEvent& e)
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

void C_ScaleManipulator::OnKeyUp(wxKeyEvent& e)
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

void C_ScaleManipulator::OnSelectionChanged()
{
	__super::OnSelectionChanged();

	if (m_Flags & Active && wxWindow::GetCapture() == m_pViewport)
		m_pViewport->ReleaseMouse();

	m_Flags &= ~Active;
	if (GetInitialMatrix(m_Transform, E_ManipulatorSpace_Local))
	{
		m_StartTransform = m_Transform;
		if (m_pContext)
			m_Flags |= Visible;
		else
			m_Flags &= ~Visible;
	}
	else
		m_Flags &= ~Visible;

	if ((m_Flags & (XAxis|YAxis|ZAxis)) == 0)
		m_Flags |= Centre;
}

void C_ScaleManipulator::EnableSnapScale(bool snap, F32 value)
{
	m_ScaleSnap = Maths::Abs(value);
	if (snap)
		m_Flags |= Snap;
	else
		m_Flags &= ~Snap;
}

bool C_ScaleManipulator::SnapEnabled() const
{
	return ((m_Flags & Snap) > 0);
}

F32 C_ScaleManipulator::SnapValue() const
{
	return m_ScaleSnap;
}

void C_ScaleManipulator::Render(const C_RenderViewport *pViewport) const
{
	C_SelectManipulator::Render(pViewport);
	if (!pViewport || !(m_Flags & Visible))
		return;

	const Maths::HVec3 position = Maths::HVec3(m_StartTransform[3][0], m_StartTransform[3][1], m_StartTransform[3][2]);
	F32 scale = GetRenderScale(*pViewport, position);

	//manipulator vertices will be defined in local space with scale & rotations stripped out
	Display::C_Matrix worldMatrix;
	Display::MatrixSetIdentity(&worldMatrix);
	worldMatrix._11 *= scale;
	worldMatrix._22 *= scale;
	worldMatrix._33 *= scale;
	worldMatrix._41 = m_Transform.AW()[0];
	worldMatrix._42 = m_Transform.AW()[1];
	worldMatrix._43 = m_Transform.AW()[2];

	Display::Device()->SetCullMode(Display::E_CullMode_Default);

	Maths::HVec3 axisX(m_StartTransform.AX()[0], m_StartTransform.AX()[1], m_StartTransform.AX()[2]);
	Maths::HVec3 axisY(m_StartTransform.AY()[0], m_StartTransform.AY()[1], m_StartTransform.AY()[2]);
	Maths::HVec3 axisZ(m_StartTransform.AZ()[0], m_StartTransform.AZ()[1], m_StartTransform.AZ()[2]);

	axisX.Normalise();
	axisY.Normalise();
	axisZ.Normalise();


	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&worldMatrix, 4);

	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
	//flip z-test to render occluded regions
	if (Display::Device()->GetDepthFlip())
		Display::Device()->SetDepthFunc(Display::E_CmpFunc_Less);
	else
		Display::Device()->SetDepthFunc(Display::E_CmpFunc_Greater);

	//the stencil pattern and reference values should already be initialised
	Display::Device()->SetStencilEnable(Display::E_Bool_True);
	//draw stippled occluded regions
	DrawManipulator(axisX, axisY, axisZ);

	Display::Device()->SetStencilEnable(Display::E_Bool_False);
	Display::Device()->SetDepthFunc(Display::E_CmpFunc_Default);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);

	//draw normally
	DrawManipulator(axisX, axisY, axisZ);
}

void C_ScaleManipulator::DrawManipulator(const Maths::HVec3& axisX, const Maths::HVec3& axisY, const Maths::HVec3& axisZ) const
{
	if (!m_pViewport)
		return;

	bool coordFlip = ((axisX^axisY)|axisZ) > 0.f ? false : true;
	if (coordFlip)
		Display::Device()->SetCullMode(Display::E_CullMode_Cw);

	{
		Display::Device()->SetVertexShader(&Display::BasicRender()->m_VertexShader[Display::E_BasicRenderVertexShader_Coloured3D]);
		Display::Device()->SetPixelShader(&Display::BasicRender()->m_PixelShader[Display::E_BasicRenderPixelShader_Coloured]);	

		Display::Vertex_XYZ_C *pVertex = (Display::Vertex_XYZ_C*) Display::Device()->BeginVertices(
			Display::E_PrimitiveType_LineList, 6, sizeof(Display::Vertex_XYZ_C), Display::Vertex_XYZ_C::m_pDeclaration, true);

		if (pVertex)
		{
			F32 sX = ((m_Flags & Active) && (m_Flags & (XAxis|Centre))) ? m_Scale : 1.f;
			F32 sY = ((m_Flags & Active) && (m_Flags & (YAxis|Centre))) ? m_Scale : 1.f;
			F32 sZ = ((m_Flags & Active) && (m_Flags & (ZAxis|Centre))) ? m_Scale : 1.f;

			SetVertex(pVertex++, 0.f, 0.f, 0.f, 0xff646464);
			SetVertex(pVertex++, sX*axisX.X(), sX*axisX.Y(), sX*axisX.Z(), 0xff646464);

			SetVertex(pVertex++, 0.f, 0.f, 0.f, 0xff646464);
			SetVertex(pVertex++, sY*axisY.X(), sY*axisY.Y(), sY*axisY.Z(), 0xff646464);

			SetVertex(pVertex++, 0.f, 0.f, 0.f, 0xff646464);
			SetVertex(pVertex++, sZ*axisZ.X(), sZ*axisZ.Y(), sZ*axisZ.Z(), 0xff646464);

			Display::Device()->EndVertices(pVertex);
		}
	}

	{
		BindRender3D();
		F32 relativeScale = (80.f/m_screenSize);	//scaling relative to a screen size of 80 pixels

		U32 nVertices = nVertices = 4*36;
		AftkRender::Vertex_XYZ2_C *pVertex = (AftkRender::Vertex_XYZ2_C*) Display::Device()->BeginVertices(
			Display::E_PrimitiveType_TriangleList, nVertices, sizeof(AftkRender::Vertex_XYZ2_C), 
			AftkRender::Vertex_XYZ2_C::m_pDeclaration, true);

		if (pVertex)
		{
			U32 colour = (m_Flags & Active) ? 0xffffffff : (m_HoverFlags & Centre) ? 0xff43ffa3 : (m_Flags & Centre) ? 0xffffff00 : 0xff00ffff;
			F32 r = 0.075f*relativeScale;
			Maths::HVec3 p0 = r*( axisX+axisY+axisZ);
			Maths::HVec3 p1 = r*(-axisX+axisY+axisZ);
			Maths::HVec3 p2 = r*(-axisX-axisY+axisZ);
			Maths::HVec3 p3 = r*( axisX-axisY+axisZ);
			Maths::HVec3 p4 = r*(-axisX+axisY-axisZ);
			Maths::HVec3 p5 = r*( axisX+axisY-axisZ);
			Maths::HVec3 p6 = r*( axisX-axisY-axisZ);
			Maths::HVec3 p7 = r*(-axisX-axisY-axisZ);

			//set all 144 vertices manually... :)

			SetVertex(pVertex++, p0.X(), p0.Y(), p0.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, p1.X(), p1.Y(), p1.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, p2.X(), p2.Y(), p2.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, p0.X(), p0.Y(), p0.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, p2.X(), p2.Y(), p2.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, p3.X(), p3.Y(), p3.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);

			SetVertex(pVertex++, p1.X(), p1.Y(), p1.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, p4.X(), p4.Y(), p4.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, p7.X(), p7.Y(), p7.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, p1.X(), p1.Y(), p1.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, p7.X(), p7.Y(), p7.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, p2.X(), p2.Y(), p2.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);

			SetVertex(pVertex++, p4.X(), p4.Y(), p4.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, p5.X(), p5.Y(), p5.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, p6.X(), p6.Y(), p6.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, p4.X(), p4.Y(), p4.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, p6.X(), p6.Y(), p6.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, p7.X(), p7.Y(), p7.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);

			SetVertex(pVertex++, p5.X(), p5.Y(), p5.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, p0.X(), p0.Y(), p0.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, p3.X(), p3.Y(), p3.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, p5.X(), p5.Y(), p5.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, p3.X(), p3.Y(), p3.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, p6.X(), p6.Y(), p6.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);

			SetVertex(pVertex++, p5.X(), p5.Y(), p5.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, p4.X(), p4.Y(), p4.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, p1.X(), p1.Y(), p1.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, p5.X(), p5.Y(), p5.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, p1.X(), p1.Y(), p1.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, p0.X(), p0.Y(), p0.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);

			SetVertex(pVertex++, p3.X(), p3.Y(), p3.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, p2.X(), p2.Y(), p2.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, p7.X(), p7.Y(), p7.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, p3.X(), p3.Y(), p3.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, p7.X(), p7.Y(), p7.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, p6.X(), p6.Y(), p6.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);

			colour = (m_Flags & Active && m_Flags & (XAxis|Centre)) ? 0xffffffff : (m_HoverFlags & XAxis) ? 0xff43ffa3 : (m_Flags & XAxis) ? 0xffffff00 : 0xffff0000;
			F32 sX = ((m_Flags & Active) && (m_Flags & (XAxis|Centre))) ? m_Scale : 1.f;

			SetVertex(pVertex++, sX*axisX.X()+p0.X(), sX*axisX.Y()+p0.Y(), sX*axisX.Z()+p0.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p1.X(), sX*axisX.Y()+p1.Y(), sX*axisX.Z()+p1.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p2.X(), sX*axisX.Y()+p2.Y(), sX*axisX.Z()+p2.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p0.X(), sX*axisX.Y()+p0.Y(), sX*axisX.Z()+p0.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p2.X(), sX*axisX.Y()+p2.Y(), sX*axisX.Z()+p2.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p3.X(), sX*axisX.Y()+p3.Y(), sX*axisX.Z()+p3.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);

			SetVertex(pVertex++, sX*axisX.X()+p1.X(), sX*axisX.Y()+p1.Y(), sX*axisX.Z()+p1.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p4.X(), sX*axisX.Y()+p4.Y(), sX*axisX.Z()+p4.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p7.X(), sX*axisX.Y()+p7.Y(), sX*axisX.Z()+p7.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p1.X(), sX*axisX.Y()+p1.Y(), sX*axisX.Z()+p1.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p7.X(), sX*axisX.Y()+p7.Y(), sX*axisX.Z()+p7.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p2.X(), sX*axisX.Y()+p2.Y(), sX*axisX.Z()+p2.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);

			SetVertex(pVertex++, sX*axisX.X()+p4.X(), sX*axisX.Y()+p4.Y(), sX*axisX.Z()+p4.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p5.X(), sX*axisX.Y()+p5.Y(), sX*axisX.Z()+p5.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p6.X(), sX*axisX.Y()+p6.Y(), sX*axisX.Z()+p6.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p4.X(), sX*axisX.Y()+p4.Y(), sX*axisX.Z()+p4.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p6.X(), sX*axisX.Y()+p6.Y(), sX*axisX.Z()+p6.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p7.X(), sX*axisX.Y()+p7.Y(), sX*axisX.Z()+p7.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);

			SetVertex(pVertex++, sX*axisX.X()+p5.X(), sX*axisX.Y()+p5.Y(), sX*axisX.Z()+p5.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p0.X(), sX*axisX.Y()+p0.Y(), sX*axisX.Z()+p0.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p3.X(), sX*axisX.Y()+p3.Y(), sX*axisX.Z()+p3.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p5.X(), sX*axisX.Y()+p5.Y(), sX*axisX.Z()+p5.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p3.X(), sX*axisX.Y()+p3.Y(), sX*axisX.Z()+p3.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p6.X(), sX*axisX.Y()+p6.Y(), sX*axisX.Z()+p6.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);

			SetVertex(pVertex++, sX*axisX.X()+p5.X(), sX*axisX.Y()+p5.Y(), sX*axisX.Z()+p5.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p4.X(), sX*axisX.Y()+p4.Y(), sX*axisX.Z()+p4.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p1.X(), sX*axisX.Y()+p1.Y(), sX*axisX.Z()+p1.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p5.X(), sX*axisX.Y()+p5.Y(), sX*axisX.Z()+p5.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p1.X(), sX*axisX.Y()+p1.Y(), sX*axisX.Z()+p1.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p0.X(), sX*axisX.Y()+p0.Y(), sX*axisX.Z()+p0.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);

			SetVertex(pVertex++, sX*axisX.X()+p3.X(), sX*axisX.Y()+p3.Y(), sX*axisX.Z()+p3.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p2.X(), sX*axisX.Y()+p2.Y(), sX*axisX.Z()+p2.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p7.X(), sX*axisX.Y()+p7.Y(), sX*axisX.Z()+p7.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p3.X(), sX*axisX.Y()+p3.Y(), sX*axisX.Z()+p3.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p7.X(), sX*axisX.Y()+p7.Y(), sX*axisX.Z()+p7.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, sX*axisX.X()+p6.X(), sX*axisX.Y()+p6.Y(), sX*axisX.Z()+p6.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);

			colour = (m_Flags & Active && m_Flags & (YAxis|Centre)) ? 0xffffffff : (m_HoverFlags & YAxis) ? 0xff43ffa3 : (m_Flags & YAxis) ? 0xffffff00 : 0xff00ff00;
			F32 sY = ((m_Flags & Active) && (m_Flags & (YAxis|Centre))) ? m_Scale : 1.f;

			SetVertex(pVertex++, sY*axisY.X()+p0.X(), sY*axisY.Y()+p0.Y(), sY*axisY.Z()+p0.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p1.X(), sY*axisY.Y()+p1.Y(), sY*axisY.Z()+p1.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p2.X(), sY*axisY.Y()+p2.Y(), sY*axisY.Z()+p2.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p0.X(), sY*axisY.Y()+p0.Y(), sY*axisY.Z()+p0.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p2.X(), sY*axisY.Y()+p2.Y(), sY*axisY.Z()+p2.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p3.X(), sY*axisY.Y()+p3.Y(), sY*axisY.Z()+p3.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);

			SetVertex(pVertex++, sY*axisY.X()+p1.X(), sY*axisY.Y()+p1.Y(), sY*axisY.Z()+p1.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p4.X(), sY*axisY.Y()+p4.Y(), sY*axisY.Z()+p4.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p7.X(), sY*axisY.Y()+p7.Y(), sY*axisY.Z()+p7.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p1.X(), sY*axisY.Y()+p1.Y(), sY*axisY.Z()+p1.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p7.X(), sY*axisY.Y()+p7.Y(), sY*axisY.Z()+p7.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p2.X(), sY*axisY.Y()+p2.Y(), sY*axisY.Z()+p2.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);

			SetVertex(pVertex++, sY*axisY.X()+p4.X(), sY*axisY.Y()+p4.Y(), sY*axisY.Z()+p4.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p5.X(), sY*axisY.Y()+p5.Y(), sY*axisY.Z()+p5.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p6.X(), sY*axisY.Y()+p6.Y(), sY*axisY.Z()+p6.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p4.X(), sY*axisY.Y()+p4.Y(), sY*axisY.Z()+p4.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p6.X(), sY*axisY.Y()+p6.Y(), sY*axisY.Z()+p6.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p7.X(), sY*axisY.Y()+p7.Y(), sY*axisY.Z()+p7.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);

			SetVertex(pVertex++, sY*axisY.X()+p5.X(), sY*axisY.Y()+p5.Y(), sY*axisY.Z()+p5.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p0.X(), sY*axisY.Y()+p0.Y(), sY*axisY.Z()+p0.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p3.X(), sY*axisY.Y()+p3.Y(), sY*axisY.Z()+p3.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p5.X(), sY*axisY.Y()+p5.Y(), sY*axisY.Z()+p5.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p3.X(), sY*axisY.Y()+p3.Y(), sY*axisY.Z()+p3.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p6.X(), sY*axisY.Y()+p6.Y(), sY*axisY.Z()+p6.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);

			SetVertex(pVertex++, sY*axisY.X()+p5.X(), sY*axisY.Y()+p5.Y(), sY*axisY.Z()+p5.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p4.X(), sY*axisY.Y()+p4.Y(), sY*axisY.Z()+p4.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p1.X(), sY*axisY.Y()+p1.Y(), sY*axisY.Z()+p1.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p5.X(), sY*axisY.Y()+p5.Y(), sY*axisY.Z()+p5.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p1.X(), sY*axisY.Y()+p1.Y(), sY*axisY.Z()+p1.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p0.X(), sY*axisY.Y()+p0.Y(), sY*axisY.Z()+p0.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);

			SetVertex(pVertex++, sY*axisY.X()+p3.X(), sY*axisY.Y()+p3.Y(), sY*axisY.Z()+p3.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p2.X(), sY*axisY.Y()+p2.Y(), sY*axisY.Z()+p2.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p7.X(), sY*axisY.Y()+p7.Y(), sY*axisY.Z()+p7.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p3.X(), sY*axisY.Y()+p3.Y(), sY*axisY.Z()+p3.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p7.X(), sY*axisY.Y()+p7.Y(), sY*axisY.Z()+p7.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, sY*axisY.X()+p6.X(), sY*axisY.Y()+p6.Y(), sY*axisY.Z()+p6.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);

			colour = (m_Flags & Active && m_Flags & (ZAxis|Centre)) ? 0xffffffff : (m_HoverFlags & ZAxis) ? 0xff43ffa3 : (m_Flags & ZAxis) ? 0xffffff00 : 0xff0000ff;
			F32 sZ = ((m_Flags & Active) && (m_Flags & (ZAxis|Centre))) ? m_Scale : 1.f;

			SetVertex(pVertex++, sZ*axisZ.X()+p0.X(), sZ*axisZ.Y()+p0.Y(), sZ*axisZ.Z()+p0.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p1.X(), sZ*axisZ.Y()+p1.Y(), sZ*axisZ.Z()+p1.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p2.X(), sZ*axisZ.Y()+p2.Y(), sZ*axisZ.Z()+p2.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p0.X(), sZ*axisZ.Y()+p0.Y(), sZ*axisZ.Z()+p0.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p2.X(), sZ*axisZ.Y()+p2.Y(), sZ*axisZ.Z()+p2.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p3.X(), sZ*axisZ.Y()+p3.Y(), sZ*axisZ.Z()+p3.Z(), axisZ.X(), axisZ.Y(), axisZ.Z(), colour);

			SetVertex(pVertex++, sZ*axisZ.X()+p1.X(), sZ*axisZ.Y()+p1.Y(), sZ*axisZ.Z()+p1.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p4.X(), sZ*axisZ.Y()+p4.Y(), sZ*axisZ.Z()+p4.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p7.X(), sZ*axisZ.Y()+p7.Y(), sZ*axisZ.Z()+p7.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p1.X(), sZ*axisZ.Y()+p1.Y(), sZ*axisZ.Z()+p1.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p7.X(), sZ*axisZ.Y()+p7.Y(), sZ*axisZ.Z()+p7.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p2.X(), sZ*axisZ.Y()+p2.Y(), sZ*axisZ.Z()+p2.Z(), -axisX.X(), -axisX.Y(), -axisX.Z(), colour);

			SetVertex(pVertex++, sZ*axisZ.X()+p4.X(), sZ*axisZ.Y()+p4.Y(), sZ*axisZ.Z()+p4.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p5.X(), sZ*axisZ.Y()+p5.Y(), sZ*axisZ.Z()+p5.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p6.X(), sZ*axisZ.Y()+p6.Y(), sZ*axisZ.Z()+p6.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p4.X(), sZ*axisZ.Y()+p4.Y(), sZ*axisZ.Z()+p4.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p6.X(), sZ*axisZ.Y()+p6.Y(), sZ*axisZ.Z()+p6.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p7.X(), sZ*axisZ.Y()+p7.Y(), sZ*axisZ.Z()+p7.Z(), -axisZ.X(), -axisZ.Y(), -axisZ.Z(), colour);

			SetVertex(pVertex++, sZ*axisZ.X()+p5.X(), sZ*axisZ.Y()+p5.Y(), sZ*axisZ.Z()+p5.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p0.X(), sZ*axisZ.Y()+p0.Y(), sZ*axisZ.Z()+p0.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p3.X(), sZ*axisZ.Y()+p3.Y(), sZ*axisZ.Z()+p3.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p5.X(), sZ*axisZ.Y()+p5.Y(), sZ*axisZ.Z()+p5.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p3.X(), sZ*axisZ.Y()+p3.Y(), sZ*axisZ.Z()+p3.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p6.X(), sZ*axisZ.Y()+p6.Y(), sZ*axisZ.Z()+p6.Z(), axisX.X(), axisX.Y(), axisX.Z(), colour);

			SetVertex(pVertex++, sZ*axisZ.X()+p5.X(), sZ*axisZ.Y()+p5.Y(), sZ*axisZ.Z()+p5.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p4.X(), sZ*axisZ.Y()+p4.Y(), sZ*axisZ.Z()+p4.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p1.X(), sZ*axisZ.Y()+p1.Y(), sZ*axisZ.Z()+p1.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p5.X(), sZ*axisZ.Y()+p5.Y(), sZ*axisZ.Z()+p5.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p1.X(), sZ*axisZ.Y()+p1.Y(), sZ*axisZ.Z()+p1.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p0.X(), sZ*axisZ.Y()+p0.Y(), sZ*axisZ.Z()+p0.Z(), axisY.X(), axisY.Y(), axisY.Z(), colour);

			SetVertex(pVertex++, sZ*axisZ.X()+p3.X(), sZ*axisZ.Y()+p3.Y(), sZ*axisZ.Z()+p3.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p2.X(), sZ*axisZ.Y()+p2.Y(), sZ*axisZ.Z()+p2.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p7.X(), sZ*axisZ.Y()+p7.Y(), sZ*axisZ.Z()+p7.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p3.X(), sZ*axisZ.Y()+p3.Y(), sZ*axisZ.Z()+p3.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p7.X(), sZ*axisZ.Y()+p7.Y(), sZ*axisZ.Z()+p7.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);
			SetVertex(pVertex++, sZ*axisZ.X()+p6.X(), sZ*axisZ.Y()+p6.Y(), sZ*axisZ.Z()+p6.Z(), -axisY.X(), -axisY.Y(), -axisY.Z(), colour);

			Display::Device()->EndVertices(pVertex);
		}
	}
}

void C_ScaleManipulator::UpdateContext(const Maths::HVec3& scale)
{
	if (!m_pContext)
		return;
	((I_VectorContext*)m_pContext)->Update(scale);
}

void C_ScaleManipulator::ApplyContext(const Maths::HVec3 &scale)
{
	if (!m_pContext)
		return;

	if (((I_VectorContext*)m_pContext)->Apply(scale))
	{
		OnSelectionChanged();
		C_RenderCore::Instance().RefreshAllViewports();
	}
}