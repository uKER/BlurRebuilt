#ifndef EDITOR_RENDER_VIEWPORT_H
#define EDITOR_RENDER_VIEWPORT_H

#ifdef _MSC_VER
#pragma once
#endif


//DECLARE_EVENT_TYPE(wxEVT_CAMERACHANGE, wxID_ANY);
extern const wxEventType wxEVT_CAMERACHANGE;

class CameraChangeEvent : public wxEvent
{
public:
	enum E_CameraChangeEventType
	{
		E_CameraChange_Position,
		E_CameraChange_Focus,
		E_CameraChange_UpVector,
		E_CameraChange_NearClip,
		E_CameraChange_FarClip,
		E_CameraChange_FieldOfView,
		E_CameraChange_ViewExtent,
		E_CameraChange_Type,
		E_CameraChange_Type_Unknown
	};

	CameraChangeEvent();
	CameraChangeEvent(const CameraChangeEvent &e);

	virtual CameraChangeEvent* Clone() const;

	void SetEventType(E_CameraChangeEventType type);
	E_CameraChangeEventType GetEventType() const;

	void SetValue(const Maths::HVec3& v);
	void SetValue(F32 v);
	void SetValue(AftkRender::C_Camera::E_CameraType v);

	const Maths::HVec3& GetVec3Value();
	F32 GetFloatValue();
	AftkRender::C_Camera::E_CameraType GetEnumValue();

private:
	E_CameraChangeEventType				m_Type;

	Maths::HVec3						m_Vec3Value;
	F32									m_FloatValue;
	AftkRender::C_Camera::E_CameraType	m_EnumValue;

	DECLARE_DYNAMIC_CLASS_NO_ASSIGN(CameraChangeEvent);
};


typedef void (wxEvtHandler::*CameraChangeEventFunction)(CameraChangeEvent&);

#define EVT_CAMERACHANGE(id, fn) \
	DECLARE_EVENT_TABLE_ENTRY( wxEVT_CAMERACHANGE, id, -1, \
	(wxObjectEventFunction) (wxEventFunction) \
	wxStaticCastEvent( CameraChangeEventFunction, &fn ), (wxObject*) NULL ),



namespace Editor
{
	class C_CameraProperties;

	class C_RenderViewport : public wxControl
	{
	public:
		C_RenderViewport(	wxWindow *parent, wxWindowID id = wxID_ANY,
							const wxPoint& pos = wxDefaultPosition,
							const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE,
							const wxValidator& validator = wxDefaultValidator,
							const wxString& name = wxControlNameStr);

		~C_RenderViewport();

		bool Create();

		void OnMouseEvent(wxMouseEvent &e);
		void OnMouseCaptureLostEvent(wxMouseCaptureLostEvent &e);
		void OnKeyDown(wxKeyEvent &e);
		void OnKeyUp(wxKeyEvent &e);
		void OnSize(wxSizeEvent &e);
		void OnEraseBackground(wxEraseEvent &e);
		void OnIdle(wxIdleEvent& e);
		void OnFocus(wxFocusEvent &e);
		void OnShow(wxShowEvent &e);
		
		void OnCameraChange(CameraChangeEvent &e);

		void OnTimer(wxTimerEvent &e);
		void SetRefreshRate(S32 rate);

		void SetDirty(bool dirty);

		AftkRender::C_Camera& GetCamera();
		const AftkRender::C_Camera& GetCamera() const;
		
		void SetCameraDefault(const AftkRender::C_Camera& cam);
		void RevertCamera();

		void SetCameraOrientation(const Maths::C_Quaternion& q);
		void SetCameraPosition(const Maths::HVec3& p);

		AftkRender::C_ViewContext& GetViewContext();
		const AftkRender::C_ViewContext& GetViewContext() const;

		bool IsCameraActive() const;

	protected:
		bool Render() const;

	private:
		wxTimer						m_UpdateTimer;

		struct Flags
		{
			U32 resizePending : 1;
			U32 cameraActive : 1;
			U32 tempRefreshRate : 1;
			U32 viewportDirty : 1;
		};
		Flags						m_Flags;

		wxPoint						m_ClickPosition;
		S32							m_ClickedButton;

		//camera state on mouse click
		Maths::HVec3				m_CameraClickPosition;
		Maths::HVec3				m_CameraClickUpVector;
		Maths::HVec3				m_CameraClickFocus;
		F32							m_CameraClickOrthoHeight;

		AftkRender::C_ViewContext	*m_pViewContext;
		AftkRender::C_Camera		m_DefaultCamera;
		wxString					m_DefaultLabel;

		friend class C_RenderCore;

		DECLARE_CLASS(C_RenderViewport);
		DECLARE_EVENT_TABLE();
	};
}

#endif