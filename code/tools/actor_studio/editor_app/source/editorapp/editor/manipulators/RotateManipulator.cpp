#include "stdafx.h"

#include <EditorApp/Editor/manipulators/contexts/IManipulatorContext.h>
#include <EditorApp/Editor/manipulators/contexts/IOrientationContext.h>
#include <EditorApp/Editor/manipulators/Manipulator.h>
#include <EditorApp/Editor/manipulators/SelectManipulator.h>
#include <EditorApp/Editor/manipulators/RotateManipulator.h>
#include <EditorApp/Editor/RenderViewport.h>

#include <EditorApp/Editor/RenderCore.h>

#include <EditorApp/components/ManipulatorPropertyPanel.h>

#include <aftk/AftkEuler.h>

#include "Display/Buffer/VertexDeclarations.h"
#include "Display/Shaders/Constants.h"

#include "shaders/vsConstants.h"
#include "shaders/psConstants.h"

using namespace Editor;

inline void SetVertex(Display::Vertex_XYZ_C *pVertex, F32 x0, F32 y0, F32 z0, U32 c)
{
	pVertex->x=x0; pVertex->y=y0, pVertex->z=z0;
	pVertex->c = c;
}


BEGIN_EVENT_TABLE(C_RotateManipulator, C_SelectManipulator)
  EVT_MOUSE_EVENTS(C_RotateManipulator::OnMouseEvent)
  EVT_MOUSE_CAPTURE_LOST(C_RotateManipulator::OnMouseCaptureLostEvent)
  EVT_KEY_DOWN(C_RotateManipulator::OnKeyDown)
  EVT_KEY_UP(C_RotateManipulator::OnKeyUp)
END_EVENT_TABLE()
C_RotateManipulator::C_RotateManipulator()
: C_SelectManipulator()
, m_Flags(0)
, m_HoverFlags(0)
, m_Angle(0)
, m_StartAngle(0)
, m_RotateSnap(Maths::DegToRad(45.f))
, m_SnapToggle(false)
{
}

C_RotateManipulator::~C_RotateManipulator()
{
}

E_ManipulatorType C_RotateManipulator::GetType() const
{
	return E_ManipulatorType_Rotate;
}

bool C_RotateManipulator::IsA(E_ManipulatorType type) const
{
	if (__super::IsA(type))
		return true;
	if (type == C_RotateManipulator::GetType()) 
		return true;

	return false;
}

void C_RotateManipulator::OnMouseEvent(wxMouseEvent &e)
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

			ApplyContext(m_RotationAxis, m_Angle-m_StartAngle);
			m_Flags = previousState & ~(Active|Inner);
		}
		else
		{
			wxPoint pos = e.GetPosition();
			Maths::C_Line ray = CalculateRay(pos.x, pos.y);

			const Maths::HVec3 position = Maths::HVec3(m_StartTransform[3][0], m_StartTransform[3][1], m_StartTransform[3][2]);
			F32 scale = GetRenderScale(*m_pViewport, position);
			F32 rayDistanceToCentre = ray.Distance(Maths::C_Vertex(position));

			const AftkRender::C_Camera& camera = m_pViewport->GetCamera();
			Maths::HVec3 toZ= camera.GetFocusPosition()-camera.GetPosition();
			toZ.Normalise();

			Maths::HVec3 toX = camera.GetUpVector()^toZ;
			toX.Normalise();

			Maths::HVec3 toY = toZ^toX;
			toY.Normalise();


			Maths::HVec3 axisX(m_StartTransform.AX()[0], m_StartTransform.AX()[1], m_StartTransform.AX()[2]);
			Maths::HVec3 axisY(m_StartTransform.AY()[0], m_StartTransform.AY()[1], m_StartTransform.AY()[2]);
			Maths::HVec3 axisZ(m_StartTransform.AZ()[0], m_StartTransform.AZ()[1], m_StartTransform.AZ()[2]);

			axisX.Normalise();
			axisY.Normalise();
			axisZ.Normalise();


			Maths::C_Vertex intersection;

			if (m_Flags & Outer || rayDistanceToCentre > scale)
			{
				//ray intersection against plane
				Maths::HVec3 planeNormal = 
					(m_Flags & XAxis) ? axisX :
					(m_Flags & YAxis) ? axisY :
					(m_Flags & ZAxis) ? axisZ : toZ;

				Maths::C_Plane plane(Maths::C_Normal(planeNormal), Maths::C_Vertex(m_StartTransform.AW()[0], m_StartTransform.AW()[1], m_StartTransform.AW()[2]));
				intersection = ray.Point(plane.Intersect(ray));
			}
			else
			{
				//ray intersection against sphere
				Maths::C_Sphere sphere(Maths::C_Vertex(m_StartTransform.AW()[0], m_StartTransform.AW()[1], m_StartTransform.AW()[2]), scale);
				Maths::C_QuadraticResult intersect = sphere.Intersect(ray);
				Maths::C_Vertex vertex = ray.Point(intersect.T0());
				intersection = vertex.m_V;
			}

			Maths::HVec3 v(intersection.X()-m_StartTransform.AW()[0], intersection.Y()-m_StartTransform.AW()[1], intersection.Z()-m_StartTransform.AW()[2]);
			v.Normalise();

			//work out angle/axis of rotation
			m_Angle =
				(m_Flags & XAxis) ? acosf(v|axisZ) :
				(m_Flags & YAxis) ? acosf(v|axisX) :
				(m_Flags & ZAxis) ? acosf(v|axisY) :
				(m_Flags & Outer) ? acosf(v|toX) : acosf(v|m_IntersectionVector);

			bool coordFlip = ((axisX^axisY)|axisZ) > 0.f ? false : true;
			if (coordFlip && (m_Flags & (XAxis|YAxis|ZAxis)))
				m_Angle *= -1;

			F32 direction = 
				(m_Flags & XAxis) ? ((v|axisY) < 0.f)	? -1.f : 1.f :
				(m_Flags & YAxis) ? ((v|axisZ) < 0.f)	? -1.f : 1.f :
				(m_Flags & ZAxis) ? ((v|axisX) < 0.f)	? -1.f : 1.f :
				(m_Flags & Outer) ? ((v|toY) < 0.f)		? -1.f : 1.f : 1.f;

			m_Angle *= direction;

			if ((m_Flags & Snap) && (m_Flags & (XAxis|YAxis|ZAxis|Outer)))
				m_Angle = Maths::Round((m_Angle-m_StartAngle)/m_RotateSnap)*m_RotateSnap + m_StartAngle;

			{
				m_RotationAxis =
					(m_Flags & XAxis) ? axisX :
					(m_Flags & YAxis) ? axisY :
					(m_Flags & ZAxis) ? axisZ :
					(m_Flags & Outer) ? -toZ : 
					(m_Flags & Inner) ? (v^m_IntersectionVector) : toX;

				m_RotationAxis.Normalise();
				Maths::C_Quaternion q0(m_RotationAxis, m_Angle-m_StartAngle);

				q0.Normalise();
				Maths::HMat44 rot(q0);
				
				m_Transform = m_StartTransform;
				m_Transform.AW(Maths::HVec4(0.f, 0.f, 0.f, 1.f));
				m_Transform = m_Transform*rot;
				m_Transform.AW(m_StartTransform.AW());

				UpdateContext(m_RotationAxis, m_Angle-m_StartAngle);

				C_RenderCore::Instance().RefreshAllViewports();
				return;
			}
		}
	}
	else if (m_Flags & Visible)
	{
		wxPoint pos = e.GetPosition();
		Maths::C_Line ray = CalculateRay(pos.x, pos.y);
		
		const Maths::HVec3 position = Maths::HVec3(m_StartTransform[3][0], m_StartTransform[3][1], m_StartTransform[3][2]);
		F32 scale = GetRenderScale(*m_pViewport, position);
		F32 rayDistanceToCentre = ray.Distance(Maths::C_Vertex(position));

		if (e.LeftDown())
			//reset axis in case the selection subsequently changes
			m_Flags &= Visible|Snap;

		m_HoverFlags = 0;

		if (rayDistanceToCentre < scale*1.25 || e.MiddleDown())
		{
			//mouse cursor within range...work out active component
			if (rayDistanceToCentre > scale && rayDistanceToCentre < scale*1.25)
			{
				const AftkRender::C_Camera& camera = m_pViewport->GetCamera();
				Maths::HVec3 toZ= camera.GetFocusPosition()-camera.GetPosition();
				toZ.Normalise();

				Maths::HVec3 toX = camera.GetUpVector()^toZ;
				toX.Normalise();

				Maths::HVec3 toY = toZ^toX;
				toY.Normalise();

				Maths::C_Plane plane(Maths::C_Normal(toZ), Maths::C_Vertex(m_Transform.AW()[0], m_Transform.AW()[1], m_Transform.AW()[2]));
				Maths::C_Vertex vertex = ray.Point(plane.Intersect(ray));
				Maths::HVec3 v0(vertex.m_V.X()-m_Transform.AW()[0], vertex.m_V.Y()-m_Transform.AW()[1], vertex.m_V.Z()-m_Transform.AW()[2]);
				v0.Normalise();

				m_HoverFlags |= Outer;
				m_StartAngle = acosf(v0|toX);
				if ((v0|toY) < 0.f)
					m_StartAngle *= -1;

				m_Angle = m_StartAngle;
				m_IntersectionVector = v0;
			}
			else
			{
				Maths::HVec3 axisX(m_Transform.AX()[0], m_Transform.AX()[1], m_Transform.AX()[2]);
				Maths::HVec3 axisY(m_Transform.AY()[0], m_Transform.AY()[1], m_Transform.AY()[2]);
				Maths::HVec3 axisZ(m_Transform.AZ()[0], m_Transform.AZ()[1], m_Transform.AZ()[2]);

				axisX.Normalise();
				axisY.Normalise();
				axisZ.Normalise();


				Maths::C_Sphere sphere(Maths::C_Vertex(m_Transform.AW()[0], m_Transform.AW()[1], m_Transform.AW()[2]), scale);
				Maths::C_QuadraticResult intersect = sphere.Intersect(ray);
				Maths::C_Vertex vertex = ray.Point(intersect.T0());
				Maths::HVec3 v0(vertex.m_V.X()-m_Transform.AW()[0], vertex.m_V.Y()-m_Transform.AW()[1], vertex.m_V.Z()-m_Transform.AW()[2]);


				if (Maths::Abs(v0|axisX)<scale*0.125f)
				{
					v0.Normalise();
					m_StartAngle = acosf(v0|axisZ);
					if ((v0|axisY) < 0.f)
						m_StartAngle *= -1;
					m_HoverFlags |= XAxis;
				}
				else if (Maths::Abs(v0|axisY)<scale*0.125f)
				{
					v0.Normalise();
					m_StartAngle = acosf(v0|axisX);
					if ((v0|axisZ) < 0.f)
						m_StartAngle *= -1;
					m_HoverFlags |= YAxis;
				}
				else if (Maths::Abs(v0|axisZ)<scale*0.125f)
				{
					v0.Normalise();
					m_StartAngle = acosf(v0|axisY);
					if ((v0|axisX) < 0.f)
						m_StartAngle *= -1;
					m_HoverFlags |= ZAxis;
				}
				else
				{
					v0.Normalise();
					m_IntersectionVector = v0;
					m_StartAngle = 0.f;
					m_HoverFlags |= Inner;
				}
				bool coordFlip = ((axisX^axisY)|axisZ) > 0.f ? false : true;
				if (coordFlip)
					m_StartAngle *= -1;

				m_Angle = m_StartAngle;
			}


			if (((e.LeftDown() && m_HoverFlags) || e.MiddleDown()) && !e.Dragging() && !wxWindow::GetCapture())
			{
				if (e.MiddleDown() && (m_HoverFlags & ~Inner) == 0)
				{
					//use currently active rotation axis
					m_HoverFlags = previousState & (XAxis|YAxis|ZAxis|Outer);
					if (!m_HoverFlags)
						m_HoverFlags = Outer;

					//planar intersection
					Maths::HVec3 axisX(m_Transform.AX()[0], m_Transform.AX()[1], m_Transform.AX()[2]);
					Maths::HVec3 axisY(m_Transform.AY()[0], m_Transform.AY()[1], m_Transform.AY()[2]);
					Maths::HVec3 axisZ(m_Transform.AZ()[0], m_Transform.AZ()[1], m_Transform.AZ()[2]);

					axisX.Normalise();
					axisY.Normalise();
					axisZ.Normalise();

					const AftkRender::C_Camera& camera = m_pViewport->GetCamera();
					Maths::HVec3 toZ= camera.GetFocusPosition()-camera.GetPosition();
					toZ.Normalise();

					Maths::HVec3 toX = camera.GetUpVector()^toZ;
					toX.Normalise();

					Maths::HVec3 toY = toZ^toX;
					toY.Normalise();

					Maths::C_Vertex intersection;
					{
						Maths::HVec3 planeNormal = 
							(m_HoverFlags & XAxis) ? axisX :
							(m_HoverFlags & YAxis) ? axisY :
							(m_HoverFlags & ZAxis) ? axisZ : toZ;

						Maths::C_Plane plane(Maths::C_Normal(planeNormal), Maths::C_Vertex(m_Transform.AW()[0], m_Transform.AW()[1], m_Transform.AW()[2]));
						intersection = ray.Point(plane.Intersect(ray));
					}

					Maths::HVec3 v(intersection.X()-m_Transform.AW()[0], intersection.Y()-m_Transform.AW()[1], intersection.Z()-m_Transform.AW()[2]);
					v.Normalise();

					//work out initial angle
					m_StartAngle =
						(m_HoverFlags & XAxis) ? acosf(v|axisZ) :
						(m_HoverFlags & YAxis) ? acosf(v|axisX) :
						(m_HoverFlags & ZAxis) ? acosf(v|axisY) : acosf(v|toX);

					bool coordFlip = ((axisX^axisY)|axisZ) > 0.f ? false : true;
					if (coordFlip && (m_Flags & (XAxis|YAxis|ZAxis)))
						m_StartAngle *= -1;

					F32 direction = 
						(m_HoverFlags & XAxis) ? ((v|axisY) < 0.f)	? -1.f : 1.f :
						(m_HoverFlags & YAxis) ? ((v|axisZ) < 0.f)	? -1.f : 1.f :
						(m_HoverFlags & ZAxis) ? ((v|axisX) < 0.f)	? -1.f : 1.f :
						(m_HoverFlags & Outer) ? ((v|toY) < 0.f)	? -1.f : 1.f : 1.f;

					m_StartAngle *= direction;
					m_Angle = m_StartAngle;
					m_IntersectionVector = v;

				}
				else
					assert(m_HoverFlags);

				m_Flags &= Active|Visible|Snap;
				m_Flags |= Active|m_HoverFlags;

				m_ClickPosition = pos;
				m_StartTransform = m_Transform;

				m_pViewport->CaptureMouse();
			}
		}
	}

	if (!(m_Flags & Active))
		C_SelectManipulator::OnMouseEvent(e);

	if (previousState != m_Flags || previousHoverState != m_HoverFlags)
		C_RenderCore::Instance().RefreshAllViewports();

}

void C_RotateManipulator::OnMouseCaptureLostEvent(wxMouseCaptureLostEvent &e)
{
	if (m_Flags & Active && wxWindow::GetCapture() == m_pViewport)
		m_pViewport->ReleaseMouse();

	e.Skip();
}

void C_RotateManipulator::OnKeyDown(wxKeyEvent& e)
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

void C_RotateManipulator::OnKeyUp(wxKeyEvent& e)
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

void C_RotateManipulator::OnSelectionChanged()
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

	if ((m_Flags & (XAxis|YAxis|ZAxis)) == 0)
		m_Flags |= Outer;
}

void C_RotateManipulator::EnableSnapRotate(bool snap, F32 value)
{
	m_RotateSnap = Maths::Abs(Maths::DegToRad(value));
	if (snap)
		m_Flags |= Snap;
	else
		m_Flags &= ~Snap;
}

bool C_RotateManipulator::SnapEnabled() const
{
	return ((m_Flags & Snap) > 0);
}

F32 C_RotateManipulator::SnapValue() const
{
	return Maths::RadToDeg(m_RotateSnap);
}

void C_RotateManipulator::Render(const C_RenderViewport *pViewport) const
{
	C_SelectManipulator::Render(pViewport);
	if (!pViewport || !(m_Flags & Visible))
		return;

	const Maths::HVec3 position = Maths::HVec3(m_Transform[3][0], m_Transform[3][1], m_Transform[3][2]);
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


	//orthonormalise camera axii
	const AftkRender::C_Camera& camera = pViewport->GetCamera();
	Maths::HVec3 toZ= camera.GetFocusPosition()-camera.GetPosition();
	toZ.Normalise();

	Maths::HVec3 toX = camera.GetUpVector()^toZ;
	toX.Normalise();

	Maths::HVec3 toY = toZ^toX;
	toY.Normalise();

	Maths::HVec3 viewX, viewY;
	if (m_pViewport && pViewport != m_pViewport)
	{
		const AftkRender::C_Camera& camera = m_pViewport->GetCamera();
		Maths::HVec3 viewZ = camera.GetFocusPosition()-camera.GetPosition();
		viewZ.Normalise();

		viewX= camera.GetUpVector()^viewZ;
		viewX.Normalise();

		viewY = viewZ^viewX;
		viewY.Normalise();
	}
	else
	{
		viewX = toX;
		viewY = toY;
	}

	Maths::HVec3 axisX(m_Transform.AX()[0], m_Transform.AX()[1], m_Transform.AX()[2]);
	Maths::HVec3 axisY(m_Transform.AY()[0], m_Transform.AY()[1], m_Transform.AY()[2]);
	Maths::HVec3 axisZ(m_Transform.AZ()[0], m_Transform.AZ()[1], m_Transform.AZ()[2]);

	axisX.Normalise();
	axisY.Normalise();
	axisZ.Normalise();


	Display::BasicRender()->BindColoured3D();
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
	DrawManipulator(axisX, axisY, axisZ, toX, toY, toZ, viewX, viewY);

	Display::Device()->SetStencilEnable(Display::E_Bool_False);
	Display::Device()->SetDepthFunc(Display::E_CmpFunc_Default);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);

	//draw normally
	DrawManipulator(axisX, axisY, axisZ, toX, toY, toZ, viewX, viewY);
}

void C_RotateManipulator::DrawManipulator(const Maths::HVec3& axisX, const Maths::HVec3& axisY, const Maths::HVec3& axisZ,
									const Maths::HVec3& toX, const Maths::HVec3& toY, const Maths::HVec3& toZ,
									const Maths::HVec3& viewX, const Maths::HVec3& viewY) const
{
	if (!m_pViewport)
		return;

	U32 nVertices = nVertices = ((m_Flags & Active) && (m_Flags & (Outer|XAxis|YAxis|ZAxis))) ? 7*Tessellation + 4 : 7*Tessellation;
	Display::Vertex_XYZ_C *pVertex = (Display::Vertex_XYZ_C*) Display::Device()->BeginVertices(
		Display::E_PrimitiveType_LineList, nVertices, sizeof(Display::Vertex_XYZ_C), 
		Display::Vertex_XYZ_C::m_pDeclaration, true);

	U32 colour = 0xff000000;
	
	bool coordFlip = ((axisX^axisY)|axisZ) > 0.f ? false : true;
	if (pVertex)
	{
		colour = ((m_Flags & Active) && (m_Flags & Outer)) ? 0xffffffff : (m_HoverFlags & Outer) ? 0xff43ffa3 : (m_Flags & Outer) ? 0xffffff00 : 0xff00ffff ;
		for (U32 i=0; i<Tessellation; ++i)
		{
			F32 theta0 = i*2.f*Maths::Constants<F32>::Pi()/Tessellation;
			F32 theta1 = (i+1)*2.f*Maths::Constants<F32>::Pi()/Tessellation;

			Maths::HVec3 v0 = 1.125f*toX*Maths::Sin(theta0) + 1.125f*toY*Maths::Cos(theta0);
			Maths::HVec3 v1 = 1.125f*toX*Maths::Sin(theta1) + 1.125f*toY*Maths::Cos(theta1);

			SetVertex(pVertex++, v0.X(), v0.Y(), v0.Z(), colour);
			SetVertex(pVertex++, v1.X(), v1.Y(), v1.Z(), colour);
		}

		colour = 0xff646464;
		for (U32 i=0; i<Tessellation; ++i)
		{
			F32 theta0 = i*2.f*Maths::Constants<F32>::Pi()/Tessellation;
			F32 theta1 = (i+1)*2.f*Maths::Constants<F32>::Pi()/Tessellation;

			Maths::HVec3 v0 = toX*Maths::Sin(theta0) + toY*Maths::Cos(theta0);
			Maths::HVec3 v1 = toX*Maths::Sin(theta1) + toY*Maths::Cos(theta1);

			SetVertex(pVertex++, v0.X(), v0.Y(), v0.Z(), colour);
			SetVertex(pVertex++, v1.X(), v1.Y(), v1.Z(), colour);
		}

		//x axis - only need the nearest 180 degree segment
		F32 angleOffset = 0.f;

		{
			Maths::HVec3 cp = toZ^axisX;
			cp.Normalise();
			F32 dp = toZ|axisZ;
			angleOffset = (dp < 0.f) ? -acosf(cp|axisZ) : acosf(cp|axisZ);
		}

		colour = ((m_Flags & Active) && (m_Flags & XAxis)) ? 0xffffffff : (m_HoverFlags & XAxis) ? 0xff43ffa3 : (m_Flags & XAxis) ? 0xffffff00  : 0xffff0000;
		for (U32 i=0; i<Tessellation/2; ++i)
		{
			F32 theta0 = angleOffset + i*2.f*Maths::Constants<F32>::Pi()/Tessellation;
			F32 theta1 = angleOffset + (i+1)*2.f*Maths::Constants<F32>::Pi()/Tessellation;

			if (coordFlip)
			{
				theta0 *= -1.f;
				theta1 *= -1.f;
			}

			Maths::HVec3 v0 = axisY*Maths::Sin(theta0) + axisZ*Maths::Cos(theta0);
			Maths::HVec3 v1 = axisY*Maths::Sin(theta1) + axisZ*Maths::Cos(theta1);

			SetVertex(pVertex++, v0.X(), v0.Y(), v0.Z(), colour);
			SetVertex(pVertex++, v1.X(), v1.Y(), v1.Z(), colour);
		}

		//y axis - only need the nearest 180 degree segment
		{
			Maths::HVec3 cp = toZ^axisY;
			cp.Normalise();
			F32 dp = toZ|axisX;
			angleOffset = (dp < 0.f) ? -acosf(cp|axisX) : acosf(cp|axisX);
		}

		colour = ((m_Flags & Active) && (m_Flags & YAxis)) ? 0xffffffff : (m_HoverFlags & YAxis) ? 0xff43ffa3 : (m_Flags & YAxis) ? 0xffffff00 :0xff00ff00;
		for (U32 i=0; i<Tessellation/2; ++i)
		{
			F32 theta0 = angleOffset + i*2.f*Maths::Constants<F32>::Pi()/Tessellation;
			F32 theta1 = angleOffset + (i+1)*2.f*Maths::Constants<F32>::Pi()/Tessellation;

			if (coordFlip)
			{
				theta0 *= -1.f;
				theta1 *= -1.f;
			}

			Maths::HVec3 v0 = axisZ*Maths::Sin(theta0) + axisX*Maths::Cos(theta0);
			Maths::HVec3 v1 = axisZ*Maths::Sin(theta1) + axisX*Maths::Cos(theta1);

			SetVertex(pVertex++, v0.X(), v0.Y(), v0.Z(), colour);
			SetVertex(pVertex++, v1.X(), v1.Y(), v1.Z(), colour);
		}

		//z axis - only need the nearest 180 degree segment
		{
			Maths::HVec3 cp = toZ^axisZ;
			cp.Normalise();
			F32 dp = toZ|axisY;
			angleOffset = (dp < 0.f) ? -acosf(cp|axisY) : acosf(cp|axisY);
		}

		colour = ((m_Flags & Active) && (m_Flags & ZAxis)) ? 0xffffffff : (m_HoverFlags & ZAxis) ? 0xff43ffa3 : (m_Flags & ZAxis) ? 0xffffff00 :0xff0000ff;
		for (U32 i=0; i<Tessellation/2; ++i)
		{
			F32 theta0 = angleOffset + i*2.f*Maths::Constants<F32>::Pi()/Tessellation;
			F32 theta1 = angleOffset + (i+1)*2.f*Maths::Constants<F32>::Pi()/Tessellation;

			if (coordFlip)
			{
				theta0 *= -1.f;
				theta1 *= -1.f;
			}

			Maths::HVec3 v0 = axisX*Maths::Sin(theta0) + axisY*Maths::Cos(theta0);
			Maths::HVec3 v1 = axisX*Maths::Sin(theta1) + axisY*Maths::Cos(theta1);

			SetVertex(pVertex++, v0.X(), v0.Y(), v0.Z(), colour);
			SetVertex(pVertex++, v1.X(), v1.Y(), v1.Z(), colour);
		}

		if ((m_Flags & Active) && (m_Flags & (Outer|XAxis|YAxis|ZAxis)))
		{
			//draw angle indicator
			if (m_Flags & XAxis)
			{
				Maths::HVec3 v0 = (coordFlip)	? axisY*Maths::Sin(-m_StartAngle) + axisZ*Maths::Cos(-m_StartAngle)
												: axisY*Maths::Sin(m_StartAngle) + axisZ*Maths::Cos(m_StartAngle);

				Maths::HVec3 v1 = (coordFlip)	? axisY*Maths::Sin(-2.f*m_StartAngle+m_Angle) + axisZ*Maths::Cos(-2.f*m_StartAngle+m_Angle)
												: axisY*Maths::Sin(2.f*m_StartAngle-m_Angle) + axisZ*Maths::Cos(2.f*m_StartAngle-m_Angle);

				SetVertex(pVertex++, 0.f, 0.f, 0.f, 0xff646464);
				SetVertex(pVertex++, v0.X(), v0.Y(), v0.Z(), 0xffffffff);
				SetVertex(pVertex++, 0.f, 0.f, 0.f, 0xff646464);
				SetVertex(pVertex++, v1.X(), v1.Y(), v1.Z(), 0xffffffff);
			}
			else if (m_Flags & YAxis)
			{
				Maths::HVec3 v0 = (coordFlip)	? axisZ*Maths::Sin(-m_StartAngle) + axisX*Maths::Cos(-m_StartAngle)
												: axisZ*Maths::Sin(m_StartAngle) + axisX*Maths::Cos(m_StartAngle);

				Maths::HVec3 v1 = (coordFlip)	? axisZ*Maths::Sin(-2.f*m_StartAngle+m_Angle) + axisX*Maths::Cos(-2.f*m_StartAngle+m_Angle)
												: axisZ*Maths::Sin(2.f*m_StartAngle-m_Angle) + axisX*Maths::Cos(2.f*m_StartAngle-m_Angle);

				SetVertex(pVertex++, 0.f, 0.f, 0.f, 0xff646464);
				SetVertex(pVertex++, v0.X(), v0.Y(), v0.Z(), 0xffffffff);
				SetVertex(pVertex++, 0.f, 0.f, 0.f, 0xff646464);
				SetVertex(pVertex++, v1.X(), v1.Y(), v1.Z(), 0xffffffff);
			}
			else if (m_Flags & ZAxis)
			{
				Maths::HVec3 v0 = (coordFlip)	? axisX*Maths::Sin(-m_StartAngle) + axisY*Maths::Cos(-m_StartAngle)
												: axisX*Maths::Sin(m_StartAngle) + axisY*Maths::Cos(m_StartAngle);

				Maths::HVec3 v1 = (coordFlip)	? axisX*Maths::Sin(-2.f*m_StartAngle+m_Angle) + axisY*Maths::Cos(-2.f*m_StartAngle+m_Angle)
												: axisX*Maths::Sin(2.f*m_StartAngle-m_Angle) + axisY*Maths::Cos(2.f*m_StartAngle-m_Angle);

				SetVertex(pVertex++, 0.f, 0.f, 0.f, 0xff646464);
				SetVertex(pVertex++, v0.X(), v0.Y(), v0.Z(), 0xffffffff);
				SetVertex(pVertex++, 0.f, 0.f, 0.f, 0xff646464);
				SetVertex(pVertex++, v1.X(), v1.Y(), v1.Z(), 0xffffffff);

			}
			else if (m_Flags & Outer)
			{
				Maths::HVec3 v0 = 1.125f*viewX*Maths::Cos(m_StartAngle) + 1.125f*viewY*Maths::Sin(m_StartAngle);
				Maths::HVec3 v1 = 1.125f*viewX*Maths::Cos(m_Angle) + 1.125f*viewY*Maths::Sin(m_Angle);

				SetVertex(pVertex++, 0.f, 0.f, 0.f, 0xff646464);		
				SetVertex(pVertex++, v0.X(), v0.Y(), v0.Z(), 0xffffffff);

				SetVertex(pVertex++, 0.f, 0.f, 0.f, 0xff646464);
				SetVertex(pVertex++, v1.X(), v1.Y(), v1.Z(), 0xffffffff);
			}
		}
		Display::Device()->EndVertices(pVertex);
	}
	if ((m_Flags & Active) && (m_Flags & (Outer|XAxis|YAxis|ZAxis)))
	{
		F32 segmentAngle = 2.f*Maths::Constants<F32>::Pi()/Tessellation;
		if (Maths::Abs(m_Angle-m_StartAngle)<Maths::Constants<F32>::Pi())
			nVertices = (U32)(Maths::Abs(m_Angle-m_StartAngle)/segmentAngle) + 3;
		else
			nVertices = (U32)((2.f*Maths::Constants<F32>::Pi()-(Maths::Abs(m_Angle-m_StartAngle)))/segmentAngle) + 3;

		pVertex = (Display::Vertex_XYZ_C*) Display::Device()->BeginVertices(
			Display::E_PrimitiveType_TriangleFan, nVertices, sizeof(Display::Vertex_XYZ_C), 
			Display::Vertex_XYZ_C::m_pDeclaration, true);

		Display::Device()->SetCullMode(Display::E_CullMode_None);

		if (pVertex)
		{
			if (m_Flags & XAxis)
			{
				SetVertex(pVertex++, 0.f, 0.f, 0.f, 0xff646464);

				Maths::HVec3 v0 =  (coordFlip)	? axisY*Maths::Sin(-2.f*m_StartAngle+m_Angle) + axisZ*Maths::Cos(-2.f*m_StartAngle+m_Angle)
												: axisY*Maths::Sin(2.f*m_StartAngle-m_Angle) + axisZ*Maths::Cos(2.f*m_StartAngle-m_Angle);
				SetVertex(pVertex++, v0.X(), v0.Y(), v0.Z(), 0x22646464);

				if (m_Angle < m_StartAngle)
					segmentAngle *= -1;

				if (Maths::Abs(m_Angle-m_StartAngle)>Maths::Constants<F32>::Pi())
					segmentAngle *= -1;

				for (U32 i=1; i<nVertices-2; ++i)
				{
					F32 angle = (coordFlip)	? -segmentAngle*i - 2.f*m_StartAngle+m_Angle
												: segmentAngle*i + 2.f*m_StartAngle-m_Angle;
					Maths::HVec3 v1 = axisY*Maths::Sin(angle) + axisZ*Maths::Cos(angle);

					SetVertex(pVertex++, v1.X(), v1.Y(), v1.Z(), 0x22646464);
				}

				Maths::HVec3 v2 = (coordFlip)	? axisY*Maths::Sin(-m_StartAngle) + axisZ*Maths::Cos(-m_StartAngle)
												: axisY*Maths::Sin(m_StartAngle) + axisZ*Maths::Cos(m_StartAngle);
				SetVertex(pVertex++, v2.X(), v2.Y(), v2.Z(), 0x22646464);

			}
			else if (m_Flags & YAxis)
			{
				SetVertex(pVertex++, 0.f, 0.f, 0.f, 0xff646464);

				Maths::HVec3 v0 =  (coordFlip)	? axisZ*Maths::Sin(-2.f*m_StartAngle+m_Angle) + axisX*Maths::Cos(-2.f*m_StartAngle+m_Angle)
												: axisZ*Maths::Sin(2.f*m_StartAngle-m_Angle) + axisX*Maths::Cos(2.f*m_StartAngle-m_Angle);
				SetVertex(pVertex++, v0.X(), v0.Y(), v0.Z(), 0x22646464);

				if (m_Angle < m_StartAngle)
					segmentAngle *= -1;

				if (Maths::Abs(m_Angle-m_StartAngle)>Maths::Constants<F32>::Pi())
					segmentAngle *= -1;

				for (U32 i=1; i<nVertices-2; ++i)
				{
					F32 angle = (coordFlip)	? -segmentAngle*i - 2.f*m_StartAngle+m_Angle
												: segmentAngle*i + 2.f*m_StartAngle-m_Angle;
					Maths::HVec3 v1 = axisZ*Maths::Sin(angle) + axisX*Maths::Cos(angle);

					SetVertex(pVertex++, v1.X(), v1.Y(), v1.Z(), 0x22646464);
				}

				Maths::HVec3 v2 = (coordFlip)	? axisZ*Maths::Sin(-m_StartAngle) + axisX*Maths::Cos(-m_StartAngle)
												: axisZ*Maths::Sin(m_StartAngle) + axisX*Maths::Cos(m_StartAngle);
				SetVertex(pVertex++, v2.X(), v2.Y(), v2.Z(), 0x22646464);

			}
			else if (m_Flags & ZAxis)
			{
				SetVertex(pVertex++, 0.f, 0.f, 0.f, 0xff646464);

				Maths::HVec3 v0 = (coordFlip)	? axisX*Maths::Sin(-2.f*m_StartAngle+m_Angle) + axisY*Maths::Cos(-2.f*m_StartAngle+m_Angle)
												: axisX*Maths::Sin(2.f*m_StartAngle-m_Angle) + axisY*Maths::Cos(2.f*m_StartAngle-m_Angle);
				SetVertex(pVertex++, v0.X(), v0.Y(), v0.Z(), 0x22646464);

				if (m_Angle < m_StartAngle)
					segmentAngle *= -1;

				if (Maths::Abs(m_Angle-m_StartAngle)>Maths::Constants<F32>::Pi())
					segmentAngle *= -1;

				for (U32 i=1; i<nVertices-2; ++i)
				{
					F32 angle = (coordFlip)	? -segmentAngle*i - 2.f*m_StartAngle+m_Angle
												: segmentAngle*i + 2.f*m_StartAngle-m_Angle;
					Maths::HVec3 v1 = axisX*Maths::Sin(angle) + axisY*Maths::Cos(angle);

					SetVertex(pVertex++, v1.X(), v1.Y(), v1.Z(), 0x22646464);
				}

				Maths::HVec3 v2 = (coordFlip)	? axisX*Maths::Sin(-m_StartAngle) + axisY*Maths::Cos(-m_StartAngle)
												: axisX*Maths::Sin(m_StartAngle) + axisY*Maths::Cos(m_StartAngle);
				SetVertex(pVertex++, v2.X(), v2.Y(), v2.Z(), 0x22646464);

			}
			if (m_Flags & Outer)
			{
				SetVertex(pVertex++, 0.f, 0.f, 0.f, 0xff646464);

				Maths::HVec3 v0 = 1.125f*viewX*Maths::Cos(m_StartAngle) + 1.125f*viewY*Maths::Sin(m_StartAngle);
				SetVertex(pVertex++, v0.X(), v0.Y(), v0.Z(), 0x22646464);

				if (m_Angle < m_StartAngle)
					segmentAngle *= -1;

				if (Maths::Abs(m_Angle-m_StartAngle)>Maths::Constants<F32>::Pi())
					segmentAngle *= -1;

				for (U32 i=1; i<nVertices-2; ++i)
				{
					F32 angle = segmentAngle*i + m_StartAngle;
					Maths::HVec3 v1 = 1.125f*viewX*Maths::Cos(angle) + 1.125f*viewY*Maths::Sin(angle);

					SetVertex(pVertex++, v1.X(), v1.Y(), v1.Z(), 0x22646464);
				}

				Maths::HVec3 v2 = 1.125f*viewX*Maths::Cos(m_Angle) + 1.125f*viewY*Maths::Sin(m_Angle);
				SetVertex(pVertex++, v2.X(), v2.Y(), v2.Z(), 0x22646464);

			}
			Display::Device()->EndVertices(pVertex);
		}
	}
}

void C_RotateManipulator::UpdateContext(const Maths::HVec3& axis, F32 angle)
{
	if (!m_pContext)
		return;
	((I_OrientationContext*)m_pContext)->Update(axis, angle);
	m_RotationAxis.Normalise();
}

void C_RotateManipulator::ApplyContext(const Maths::HVec3& axis, F32 angle)
{
	if (!m_pContext)
		return;
	if (((I_OrientationContext*)m_pContext)->Apply(axis, angle))
	{
		OnSelectionChanged();
		C_RenderCore::Instance().RefreshAllViewports();
	}
	m_RotationAxis.Normalise();
}
