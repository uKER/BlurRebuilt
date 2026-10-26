#ifndef TIMELINEPANEL_H
#define TIMELINEPANEL_H

#include <wx/treectrl.h>
#include <btl/containers/vector.h>
#include <btl/utility/string.h>
#include <sigslot/sigslot.h>

namespace Animation
{
	class C_Skeleton;
	class C_SkeletonInstance;
	class C_Pose;
}

class C_TimelinePanel: public wxPanel, public sigslot::has_slots<>
{
public:
	C_TimelinePanel(
		wxWindow* parent,
		wxWindowID winId,
		const wxPoint& pos,
		const wxSize& size
	);
	
	void OnScrollEvent(wxCommandEvent& e);
	void OnValueTextChanged(wxCommandEvent& e);
private:

	wxSlider*	m_slider;
	wxTextCtrl*	m_valueText;

};

#endif
