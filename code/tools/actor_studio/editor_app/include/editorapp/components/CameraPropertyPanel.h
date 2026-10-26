#ifndef EDITOR_CAMERA_PROPERTY_PANEL
#define EDITOR_CAMERA_PROPERTY_PANEL

#ifdef _MSC_VER
#pragma once
#endif

class wxPropertyGridManager;
class wxPropertyGridEvent;

namespace Editor
{
	class C_CameraProperties;

	class C_CameraPropertyPanel : public wxPanel
	{
	public:
		C_CameraPropertyPanel(wxWindow *parent, wxWindowID id=wxID_ANY,
								const wxPoint& pos = wxDefaultPosition,
								const wxSize& size = wxDefaultSize,
								long style = wxTAB_TRAVERSAL | wxNO_BORDER);

		~C_CameraPropertyPanel();

		void OnPropertyChange(wxPropertyGridEvent& e);
		void OnCameraReset(wxCommandEvent& e);

		C_CameraProperties* GetCameraPropertiesPtr();
		const C_CameraProperties* GetCameraPropertiesPtr() const;

	private:
		wxPropertyGridManager	*m_pPropertyGridManager;
		wxBitmapButton			*m_pResetButton;
		wxStaticText			*m_pLabel;
		C_CameraProperties		*m_pProperties;

		DECLARE_EVENT_TABLE()
	};
}


#endif