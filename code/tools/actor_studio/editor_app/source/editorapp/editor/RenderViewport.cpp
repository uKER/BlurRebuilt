#include "stdafx.h"

#include <EditorApp/Editor/RenderViewport.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderPipeline.h>
#include <EditorApp/Editor/Manipulators/contexts/IManipulatorContext.h>
#include <EditorApp/Editor/Manipulators/Manipulator.h>
#include <EditorApp/Editor/Primitives.h>
#include <EditorApp/Editor/EditorRenderer.h>
#include <EditorApp/Editor/SimpleRenderer.h>
#include <EditorApp/Properties/CameraProperties.h>
#include <EditorApp/Components/CameraPropertyPanel.h>
#include <EditorApp/App.h>

using namespace Editor;

IMPLEMENT_DYNAMIC_CLASS(CameraChangeEvent, wxEvent)

DEFINE_EVENT_TYPE(wxEVT_CAMERACHANGE)

CameraChangeEvent::CameraChangeEvent()
: wxEvent(wxID_ANY, wxEVT_CAMERACHANGE)
, m_Type(CameraChangeEvent::E_CameraChange_Type_Unknown)
{
}

CameraChangeEvent::CameraChangeEvent(const CameraChangeEvent& e)
: wxEvent(wxID_ANY, wxEVT_CAMERACHANGE)
{
	m_Type = e.m_Type;
	m_Vec3Value = e.m_Vec3Value;
	m_FloatValue = e.m_FloatValue;
	m_EnumValue = e.m_EnumValue;
}

CameraChangeEvent* CameraChangeEvent::Clone() const
{
	return new CameraChangeEvent(*this);
}

void CameraChangeEvent::SetEventType(E_CameraChangeEventType type)
{
	m_Type = type;
}

CameraChangeEvent::E_CameraChangeEventType CameraChangeEvent::GetEventType() const
{
	return m_Type;
}

void CameraChangeEvent::SetValue(const Maths::HVec3& v)
{
	assert(m_Type==E_CameraChange_Position || m_Type==E_CameraChange_Focus || m_Type==E_CameraChange_UpVector);
	m_Vec3Value = v;
}

void CameraChangeEvent::SetValue(F32 v)
{
	assert(m_Type==E_CameraChange_NearClip || m_Type==E_CameraChange_FarClip || m_Type==E_CameraChange_FieldOfView || m_Type==E_CameraChange_ViewExtent);
	m_FloatValue = v;
}

void CameraChangeEvent::SetValue(AftkRender::C_Camera::E_CameraType v)
{
	assert(m_Type==E_CameraChange_Type);
	m_EnumValue = v;
}

const Maths::HVec3& CameraChangeEvent::GetVec3Value()
{
	return m_Vec3Value;
}

F32 CameraChangeEvent::GetFloatValue()
{
	return m_FloatValue;
}

AftkRender::C_Camera::E_CameraType CameraChangeEvent::GetEnumValue()
{
	return m_EnumValue;
}







IMPLEMENT_CLASS(C_RenderViewport, wxControl)
BEGIN_EVENT_TABLE(C_RenderViewport, wxControl)
  EVT_MOUSE_EVENTS(C_RenderViewport::OnMouseEvent)
  EVT_MOUSE_CAPTURE_LOST(C_RenderViewport::OnMouseCaptureLostEvent)
  EVT_KEY_DOWN(C_RenderViewport::OnKeyDown)
  EVT_KEY_UP(C_RenderViewport::OnKeyUp)
  EVT_SIZE(C_RenderViewport::OnSize)
  EVT_TIMER(wxID_ANY, C_RenderViewport::OnTimer)
  EVT_ERASE_BACKGROUND(C_RenderViewport::OnEraseBackground)
  EVT_IDLE(C_RenderViewport::OnIdle)
  EVT_SET_FOCUS(C_RenderViewport::OnFocus)
  EVT_KILL_FOCUS(C_RenderViewport::OnFocus)
  EVT_SHOW(C_RenderViewport::OnShow)
  EVT_CAMERACHANGE(wxID_ANY, C_RenderViewport::OnCameraChange)
END_EVENT_TABLE()

C_RenderViewport::C_RenderViewport(	wxWindow *parent, wxWindowID id, 
									const wxPoint& pos, 
									const wxSize& size, 
									long style, 
									const wxValidator& validator, 
									const wxString& name)
: wxControl(parent, id, pos, size, style, validator, name) 
, m_pViewContext(0)
, m_ClickedButton(wxMOUSE_BTN_NONE)
{
	m_UpdateTimer.SetOwner(this, wxID_ANY);
	memset(&m_Flags, 0, sizeof(C_RenderViewport::Flags));
}

C_RenderViewport::~C_RenderViewport()
{
	if (m_pViewContext)
	{
		I_Manipulator *pManip = C_RenderCore::Instance().GetManipulatorManager().GetActiveManipulator();
		if (pManip && pManip->GetAttachedViewport() == this)
			pManip->AttachToViewport(0);

		C_RenderCore::Instance().UnregisterViewport(this);
		m_pViewContext = 0;
	}
}

bool C_RenderViewport::Create()
{
	m_pViewContext = C_RenderCore::Instance().RegisterViewport(this);
	if (m_pViewContext)
		wxSize size = GetSize();

	return (m_pViewContext != 0);
}

void C_RenderViewport::OnMouseEvent(wxMouseEvent &e)
{
	if ((e.LeftDown() || e.MiddleDown() || e.RightDown()) && wxWindow::FindFocus() != this)
	{
		//manipulators are only attached to the viewport with focus...
		//set focus before processing event
		SetFocus();
		GetEventHandler()->AddPendingEvent(e);
		return;
	}

	bool handled = m_Flags.cameraActive;
	AftkRender::C_Camera& camera = m_pViewContext->GetView().camera;

	if(!m_Flags.cameraActive && (e.LeftDown() || e.MiddleDown() || e.RightDown()) && e.AltDown())
	{
		if (!wxWindow::GetCapture())
		{
			m_ClickedButton = e.GetButton();
			CaptureMouse();
			m_ClickPosition = e.GetPosition();
			m_CameraClickPosition = camera.GetPosition();
			m_CameraClickUpVector = camera.GetUpVector();
			m_CameraClickFocus = camera.GetFocusPosition();
			m_CameraClickOrthoHeight = camera.GetHeightExtent();

			if (!m_UpdateTimer.IsRunning())
			{
				//temporarily force a refresh at 30hz
				SetRefreshRate(30);
				m_Flags.tempRefreshRate = true;
			}
			m_Flags.cameraActive = true;
		}
		handled = true;
	}
	else if (!m_Flags.cameraActive)
	{
		S32 rotation = e.GetWheelRotation();
		if (rotation)
		{
			handled = true;

			if (camera.GetType() == AftkRender::C_Camera::E_CameraType_Orthographic)
			{
				F32 delta = (F32) -rotation/(F32)e.GetWheelDelta() * 0.1f*camera.GetHeightExtent();
				if (delta > 0.f)
					delta = Maths::Max(delta, 1.f);
				else
					delta = Maths::Min(delta, -1.f);

				camera.SetHeightExtent(Maths::Clamp(camera.GetHeightExtent() + delta, 0.25f, 10000.f));

				C_CameraProperties *pProperties = wxGetApp().GetCameraPropertyPanel() 
					? wxGetApp().GetCameraPropertyPanel()->GetCameraPropertiesPtr() : 0;
				if (pProperties)
				{
					CameraChangeEvent camExtentEvt;
					camExtentEvt.SetEventType(CameraChangeEvent::E_CameraChange_ViewExtent);
					camExtentEvt.SetValue(camera.GetHeightExtent());
					pProperties->OnCameraChange(camExtentEvt);
				}
			}
			else
			{
				F32 delta = (F32) rotation/(F32)e.GetWheelDelta();
				
				Maths::HVec3 direction = camera.GetFocusPosition() - camera.GetPosition();
				delta =  0.2f * delta * direction.Length();
				if (delta > 0.f)
					delta = Maths::Max(0.5f, delta);
				else
					delta = Maths::Min(-0.5f, delta);

				direction.Normalise();

				camera.GetPosition() = camera.GetPosition() + delta*direction;

				bool focusMoved = false;
				//move pivot only when necessary
				{
					Maths::HVec3 newDirection = camera.GetFocusPosition() - camera.GetPosition();
					if ((newDirection|direction) < 0.f)
					{
						//we gone through our focal point... move the focal point to compensate
						camera.GetFocusPosition() = camera.GetFocusPosition() + 2.5f*delta*direction;
						focusMoved = true;
					}
				}

				C_CameraProperties *pProperties = wxGetApp().GetCameraPropertyPanel() 
					? wxGetApp().GetCameraPropertyPanel()->GetCameraPropertiesPtr() : 0;
				if (pProperties)
				{
					CameraChangeEvent camPosEvt;
					camPosEvt.SetEventType(CameraChangeEvent::E_CameraChange_Position);
					camPosEvt.SetValue(camera.GetPosition());
					pProperties->OnCameraChange(camPosEvt);

					if (focusMoved)
					{
						CameraChangeEvent camFocusEvt;
						camFocusEvt.SetEventType(CameraChangeEvent::E_CameraChange_Focus);
						camFocusEvt.SetValue(camera.GetFocusPosition());
						pProperties->OnCameraChange(camFocusEvt);
					}
				}


			}
			Refresh();
		}
	}

	if(m_Flags.cameraActive && (e.LeftUp() || e.MiddleUp() || e.RightUp()) && m_ClickedButton == e.GetButton())
	{
		if (wxWindow::GetCapture() == this)
			ReleaseMouse();

		if (m_Flags.tempRefreshRate)
		{
			SetRefreshRate(0);
			m_Flags.tempRefreshRate = false;
		}
		m_ClickedButton = wxMOUSE_BTN_NONE;
		m_Flags.cameraActive = false;

		//render one more frame to stop the camera jumping next time around...
		Refresh();
		handled = true;
	}

	if (m_Flags.cameraActive && e.Dragging())
	{
		handled = true;
		wxPoint pos = e.GetPosition();

		//camera tilt/pan
		if (m_ClickedButton == wxMOUSE_BTN_LEFT)
		{
			if (camera.GetType() == AftkRender::C_Camera::E_CameraType_Orthographic)
				wxSetCursor(wxCursor("\"IDC_CURSOR_INVALID\""));
			else
			{
				wxSetCursor(wxCursor("\"IDC_CURSOR_TILTPAN\""));

				Maths::HVec3 direction = m_CameraClickPosition - m_CameraClickFocus;
				F32 distance = direction.Length();

				F32 deltaX = (F32)(pos.x-m_ClickPosition.x)/(F32)GetSize().x;
				F32 deltaY = (F32)(pos.y-m_ClickPosition.y)/(F32)GetSize().y;

				F32 thetaX = 2.f*Maths::Constants<F32>::Pi() * deltaY;
				F32 thetaY = -2.f*Maths::Constants<F32>::Pi() * deltaX;

				//Maths::HVec3& camPos = camera.GetPosition();
				//camPos.X(Maths::Cos(thetaX) * Maths::Cos(thetaY));
				//camPos.Y(Maths::Sin(thetaX));
				//camPos.Z(Maths::Cos(thetaX) * Maths::Sin(thetaY));

				Maths::HVec3 right = m_CameraClickUpVector^direction;
				right.Normalise();
				Maths::C_Quaternion r0(right, thetaX);
				Maths::C_Quaternion r1(Maths::HVec3(0.f, 1.f, 0.f), thetaY);

				Maths::C_Quaternion p0(direction.X(), direction.Y(), direction.Z(), 0.f);

				Maths::HVec4 p1 = r1 * r0 * p0 * !r0 * !r1;
				Maths::HVec3& camPos = camera.GetPosition();
				camPos.X(p1.X()); camPos.Y(p1.Y()); camPos.Z(p1.Z());
				camPos += m_CameraClickFocus;

				Maths::C_Quaternion v0(m_CameraClickUpVector.X(), m_CameraClickUpVector.Y(), m_CameraClickUpVector.Z(), 0.f);
				Maths::HVec4 v1 = r1 * r0 * v0 * !r0 * !r1;
				Maths::HVec3& camUp = camera.GetUpVector();
				camUp.X(v1.X()); camUp.Y(v1.Y()); camUp.Z(v1.Z());

				C_CameraProperties *pProperties = wxGetApp().GetCameraPropertyPanel() 
					? wxGetApp().GetCameraPropertyPanel()->GetCameraPropertiesPtr() : 0;
				if (pProperties)
				{
					CameraChangeEvent camPosEvt;
					camPosEvt.SetEventType(CameraChangeEvent::E_CameraChange_Position);
					camPosEvt.SetValue(camPos);
					pProperties->OnCameraChange(camPosEvt);

					CameraChangeEvent camUpEvt;
					camUpEvt.SetEventType(CameraChangeEvent::E_CameraChange_UpVector);
					camUpEvt.SetValue(camUp);
					pProperties->OnCameraChange(camUpEvt);
				}
			}
		}
		//camera tracking
		else if (m_ClickedButton == wxMOUSE_BTN_MIDDLE)
		{
			wxSetCursor(wxCursor("\"IDC_CURSOR_TRACK\""));
			Maths::HVec3 direction = m_CameraClickFocus - m_CameraClickPosition;

			Maths::HVec3 toX, toY, toZ;
			toZ = direction;
			toZ.Normalise();

			toX = m_CameraClickUpVector^toZ;
			toX.Normalise();

			toY = toZ^toX;
			toY.Normalise();

			F32 aspect = camera.GetAspectRatio();
			F32 focalDist = direction.Length();
			F32 farClip = camera.GetFarClipDistance();
			F32 fov = camera.GetFieldOfView();

			F32 h1 = (camera.GetType() == AftkRender::C_Camera::E_CameraType_Perspective)
						? 1.0f * focalDist * Maths::Sin(Maths::DegToRad(fov*0.5f))
						: 0.5f * camera.GetHeightExtent();

			F32 w1 = aspect*h1;

			F32 h2 = (F32)(pos.y-m_ClickPosition.y)/(F32)GetSize().y;
			F32 w2 = (F32)(pos.x-m_ClickPosition.x)/(F32)GetSize().x;

			Maths::HVec3 delta = toY*h1*h2 - toX*w1*w2;
			camera.GetPosition() = m_CameraClickPosition + 2*delta;
			camera.GetFocusPosition() = m_CameraClickFocus + 2*delta;

			C_CameraProperties *pProperties = wxGetApp().GetCameraPropertyPanel() 
				? wxGetApp().GetCameraPropertyPanel()->GetCameraPropertiesPtr() : 0;
			if (pProperties)
			{
				CameraChangeEvent camPosEvt;
				camPosEvt.SetEventType(CameraChangeEvent::E_CameraChange_Position);
				camPosEvt.SetValue(camera.GetPosition());
				pProperties->OnCameraChange(camPosEvt);

				CameraChangeEvent camFocusEvt;
				camFocusEvt.SetEventType(CameraChangeEvent::E_CameraChange_Focus);
				camFocusEvt.SetValue(camera.GetFocusPosition());
				pProperties->OnCameraChange(camFocusEvt);
			}
		}
		//camera dolly(perspective) or extents(orthographic)
		else if (m_ClickedButton == wxMOUSE_BTN_RIGHT)
		{
			if (camera.GetType() == AftkRender::C_Camera::E_CameraType_Orthographic)
			{
				wxSetCursor(wxCursor("\"IDC_CURSOR_EXTENT\""));
				F32 delta = m_CameraClickOrthoHeight*((F32)(pos.x-m_ClickPosition.x)/(F32)GetSize().x + (F32)(pos.y-m_ClickPosition.y)/(F32)GetSize().y);

				camera.SetHeightExtent(Maths::Clamp(m_CameraClickOrthoHeight - delta, 0.25f, 10000.f));

				C_CameraProperties *pProperties = wxGetApp().GetCameraPropertyPanel() 
					? wxGetApp().GetCameraPropertyPanel()->GetCameraPropertiesPtr() : 0;
				if (pProperties)
				{
					CameraChangeEvent camExtentEvt;
					camExtentEvt.SetEventType(CameraChangeEvent::E_CameraChange_ViewExtent);
					camExtentEvt.SetValue(camera.GetHeightExtent());
					pProperties->OnCameraChange(camExtentEvt);
				}
			}
			else
			{
				wxSetCursor(wxCursor("\"IDC_CURSOR_DOLLY\""));
				F32 delta = (F32)(pos.x-m_ClickPosition.x)/(F32)GetSize().x + (F32)(pos.y-m_ClickPosition.y)/(F32)GetSize().y;
				
				Maths::HVec3 direction = m_CameraClickFocus - m_CameraClickPosition;
				delta = delta * direction.Length();
				direction.Normalise();

				camera.GetPosition() = m_CameraClickPosition + delta*direction;

				bool focusMoved = false;
				//move pivot only when necessary
				{
					Maths::HVec3 newDirection = m_CameraClickFocus - camera.GetPosition();
					if ((newDirection|direction) < 0.f)
					{
						//we gone through our focal point... move the focal point to compensate
						camera.GetFocusPosition() = m_CameraClickFocus + 2.5f*delta*direction;
						focusMoved = true;
					}
				}

				C_CameraProperties *pProperties = wxGetApp().GetCameraPropertyPanel() 
					? wxGetApp().GetCameraPropertyPanel()->GetCameraPropertiesPtr() : 0;
				if (pProperties)
				{
					CameraChangeEvent camPosEvt;
					camPosEvt.SetEventType(CameraChangeEvent::E_CameraChange_Position);
					camPosEvt.SetValue(camera.GetPosition());
					pProperties->OnCameraChange(camPosEvt);

					if (focusMoved)
					{
						CameraChangeEvent camFocusEvt;
						camFocusEvt.SetEventType(CameraChangeEvent::E_CameraChange_Focus);
						camFocusEvt.SetValue(camera.GetFocusPosition());
						pProperties->OnCameraChange(camFocusEvt);
					}
				}
			}
		}
	}

	if (!handled)
		e.Skip();

}

void C_RenderViewport::OnMouseCaptureLostEvent(wxMouseCaptureLostEvent &e)
{
	if (m_Flags.cameraActive)
	{
		m_ClickedButton = wxMOUSE_BTN_NONE;
		if (m_Flags.tempRefreshRate)
		{
			SetRefreshRate(0);
			m_Flags.tempRefreshRate = false;
		}
		m_Flags.cameraActive = false;
	}

	if (wxWindow::GetCapture() == this)
		ReleaseMouse();
}

void C_RenderViewport::OnKeyDown(wxKeyEvent &e)
{
	if (!m_Flags.cameraActive)
		e.Skip();
}

void C_RenderViewport::OnKeyUp(wxKeyEvent &e)
{
	if (!m_Flags.cameraActive)
		e.Skip();
}

void C_RenderViewport::OnSize(wxSizeEvent& e)
{
	//Prevent continuous resizing ... only resize d3d surfaces after events have taken place
	m_Flags.resizePending = true;
}

void C_RenderViewport::OnEraseBackground(wxEraseEvent &e)
{
	if (m_pViewContext!=NULL)
		Render();
}

void C_RenderViewport::OnIdle(wxIdleEvent& e)
{
	if (m_Flags.resizePending && m_pViewContext!=NULL)
	{
		wxSize size = GetSize();
		m_Flags.resizePending = !C_RenderCore::Instance().ResizeViewport(size.x, size.y, this);
		if (!m_Flags.resizePending)
			Render();
	}
	else if (m_Flags.viewportDirty)
	{
		if (Render())
			m_Flags.viewportDirty = false;
	}
}

void C_RenderViewport::OnFocus(wxFocusEvent &e)
{
	if (e.GetEventType() == wxEVT_KILL_FOCUS)
	{
	}
	else if (e.GetEventType() == wxEVT_SET_FOCUS)
	{
		C_CameraProperties *pProperties = wxGetApp().GetCameraPropertyPanel() 
			? wxGetApp().GetCameraPropertyPanel()->GetCameraPropertiesPtr() : 0;
		if (pProperties)
			pProperties->AttachViewport(this);

		if (m_pViewContext)
		{
			I_Manipulator *pHandler = C_RenderCore::Instance().GetManipulatorManager().GetActiveManipulator();
			if (pHandler)
				pHandler->AttachToViewport(this);
		}
	}

	//slightly guerilla method of forcing the C_ViewportPanel to which this is attached to repaint
	//connecting to this event from the C_ViewportPanel doesn't seem to have the same effect
	GetParent()->Refresh();
}

void C_RenderViewport::OnShow(wxShowEvent &e)
{
	if (e.GetShow())
	{
		wxSize size = GetSize();
		C_RenderCore::Instance().ResizeViewport(size.x, size.y, this);
		Refresh();
	}
	else
	{
		C_RenderCore::Instance().ResizeViewport(1, 1, this);
	}
}

void C_RenderViewport::OnCameraChange(CameraChangeEvent& e)
{
	if (m_pViewContext)
	{
		AftkRender::C_Camera& camera = m_pViewContext->GetView().camera;
		switch(e.GetEventType())
		{
		case CameraChangeEvent::E_CameraChange_Position:
			camera.SetPosition(e.GetVec3Value());
			break;
		case CameraChangeEvent::E_CameraChange_Focus:
			camera.SetFocusPosition(e.GetVec3Value());
			break;
		case CameraChangeEvent::E_CameraChange_UpVector:
			camera.SetUpVector(e.GetVec3Value());
			break;
		case CameraChangeEvent::E_CameraChange_NearClip:
			camera.SetNearClipDistance(e.GetFloatValue());
			break;
		case CameraChangeEvent::E_CameraChange_FarClip:
			camera.SetFarClipDistance(e.GetFloatValue());
			break;
		case CameraChangeEvent::E_CameraChange_FieldOfView:
			camera.SetFieldOfView(e.GetFloatValue());
			break;
		case CameraChangeEvent::E_CameraChange_ViewExtent:
			camera.SetHeightExtent(e.GetFloatValue());
			break;
		case CameraChangeEvent::E_CameraChange_Type:
			camera.SetType(e.GetEnumValue());
			break;
		}

		Refresh();
	}
}

void C_RenderViewport::OnTimer(wxTimerEvent &e)
{
	if (!m_Flags.resizePending)
		Refresh();
}

void C_RenderViewport::SetRefreshRate(S32 rate)
{
	if (rate > 0)
	{
		S32 interval = (S32) (1000.f/rate);

		if (m_UpdateTimer.IsRunning())
			m_UpdateTimer.Stop();

		m_UpdateTimer.Start(interval);
	}
	else
	{
		m_UpdateTimer.Stop();
	}
}

void C_RenderViewport::SetDirty(bool dirty)
{
	m_Flags.viewportDirty = dirty;
}

AftkRender::C_Camera& C_RenderViewport::GetCamera()
{
	assert(m_pViewContext && "View context not created");
	return m_pViewContext->GetView().camera;
}

const AftkRender::C_Camera& C_RenderViewport::GetCamera() const
{
	assert(m_pViewContext && "View context not created");
	return m_pViewContext->GetView().camera;
}

void C_RenderViewport::SetCameraDefault(const AftkRender::C_Camera& cam)
{
	m_DefaultCamera = m_pViewContext->GetView().camera;
	m_DefaultLabel = GetLabel();
}

void C_RenderViewport::RevertCamera()
{
	m_pViewContext->GetView().camera.SetPosition(m_DefaultCamera.GetPosition());
	m_pViewContext->GetView().camera.SetFocusPosition(m_DefaultCamera.GetFocusPosition());
	m_pViewContext->GetView().camera.SetUpVector(m_DefaultCamera.GetUpVector());
	m_pViewContext->GetView().camera.SetFieldOfView(m_DefaultCamera.GetFieldOfView());
	m_pViewContext->GetView().camera.SetHeightExtent(m_DefaultCamera.GetHeightExtent());
	m_pViewContext->GetView().camera.SetType(m_DefaultCamera.GetType());
}

void C_RenderViewport::SetCameraOrientation(const Maths::C_Quaternion& q)
{
	AftkRender::C_Camera& camera = m_pViewContext->GetView().camera;
	F32 focalDistance = (camera.GetFocusPosition()-camera.GetPosition()).Length();

	Maths::HMat33 m(q);
	camera.SetUpVector(m.AY());
	Maths::HVec3 focalPosition = camera.GetPosition() + focalDistance*m.AZ();
	camera.SetFocusPosition(focalPosition);

	C_CameraProperties *pProperties = wxGetApp().GetCameraPropertyPanel() 
		? wxGetApp().GetCameraPropertyPanel()->GetCameraPropertiesPtr() : 0;
	if (pProperties)
	{
		CameraChangeEvent camFocusEvt;
		camFocusEvt.SetEventType(CameraChangeEvent::E_CameraChange_Focus);
		camFocusEvt.SetValue(camera.GetFocusPosition());
		pProperties->OnCameraChange(camFocusEvt);

		CameraChangeEvent camUpEvt;
		camUpEvt.SetEventType(CameraChangeEvent::E_CameraChange_UpVector);
		camUpEvt.SetValue(camera.GetUpVector());
		pProperties->OnCameraChange(camUpEvt);
	}
}

void C_RenderViewport::SetCameraPosition(const Maths::HVec3& p)
{
	AftkRender::C_Camera& camera = m_pViewContext->GetView().camera;
	Maths::HVec3 delta = p - camera.GetPosition();

	camera.GetPosition() = p;
	camera.GetFocusPosition() += delta;

	C_CameraProperties *pProperties = wxGetApp().GetCameraPropertyPanel() 
		? wxGetApp().GetCameraPropertyPanel()->GetCameraPropertiesPtr() : 0;
	if (pProperties)
	{
		CameraChangeEvent camPosEvt;
		camPosEvt.SetEventType(CameraChangeEvent::E_CameraChange_Position);
		camPosEvt.SetValue(camera.GetPosition());
		pProperties->OnCameraChange(camPosEvt);

		CameraChangeEvent camFocusEvt;
		camFocusEvt.SetEventType(CameraChangeEvent::E_CameraChange_Focus);
		camFocusEvt.SetValue(camera.GetFocusPosition());
		pProperties->OnCameraChange(camFocusEvt);
	}
}

AftkRender::C_ViewContext& C_RenderViewport::GetViewContext()
{
	assert(m_pViewContext && "View context not created");
	return *m_pViewContext;
}

const AftkRender::C_ViewContext& C_RenderViewport::GetViewContext() const
{
	assert(m_pViewContext && "View context not created");
	return *m_pViewContext;
}

bool C_RenderViewport::IsCameraActive() const
{
	return m_Flags.cameraActive;
}

bool C_RenderViewport::Render() const
{
	if (!m_Flags.resizePending && this->IsShownOnScreen() && m_pViewContext != 0)
	{
		if (!C_RenderCore::Instance().BeginRendering(this))
			return false;

		Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth | Display::E_Clear_Stencil, Maths::RGBA(160, 160, 160, 255));
	
		bool rendered = C_RenderCore::Instance().GetRenderPipeline().Render(this);

		C_RenderCore::Instance().EndRendering(this);

		return rendered;
	}

	return false;
}
