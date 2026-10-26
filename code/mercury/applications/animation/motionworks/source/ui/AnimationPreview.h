#ifndef ANIMATION_PREVIEW_H
#define ANIMATION_PREVIEW_H

#ifdef _MSC_VER
#pragma once
#endif

#include "PreviewDisplayWindow.h"
#include "wx/dirctrl.h"
#include "wx/notebook.h"
#include "wx/splitter.h"

class C_PreviewSection;


//
// Animation 3D preview window.
// 

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_REFRESH_DISPLAY_WINDOW,	wxID_ANY)
END_DECLARE_EVENT_TYPES()

class AnimationPreviewWindow : public wxPanel
{
public:
	AnimationPreviewWindow( wxWindow *parent, wxWindowID, const wxPoint &pos, const wxSize &size );
	~AnimationPreviewWindow();

	bool	IsOk() const
	{
		return m_pDisplayWindow != NULL;
	}

	void AcquireDisplayWindow(C_DisplayWindow* w)
	{
		if (m_pDisplayWindow == NULL)
		{
			m_pDisplayWindow = w;
			if (w)
			{
				w->Reparent(this);
				GetSizer()->Add(w, wxSizerFlags(1).Centre().Expand());
			}
			Layout();
		}
	}
	
	C_DisplayWindow* ReleaseDisplayWindow()
	{
		C_DisplayWindow* w = m_pDisplayWindow;
		if (w)
		{
			GetSizer()->Detach(w);
			m_pDisplayWindow = NULL;
			Layout();
		}
		return w;
	}
	
	void OnRefreshDisplayWindow( wxCommandEvent& e );
	
protected:
	C_DisplayWindow*		m_pDisplayWindow;		///< 3D display window.
	C_PreviewSection*		m_section;

	DECLARE_CLASS(AnimationPreviewWindow)
    //DECLARE_EVENT_TABLE()
};


#endif
