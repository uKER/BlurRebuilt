#include "stdafx.h"
#include "AnimationPreview.h"
#include "../AppSettings.h"
//#include "Samples/Section/SectionSkyDome.h"
#include "../PreviewSection.h"
#include "../PreviewState.h"
#include "wx/progdlg.h"
#include "wx/dnd.h"


//
// AnimationPreviewWindow
//

IMPLEMENT_CLASS(AnimationPreviewWindow, wxPanel)

DEFINE_EVENT_TYPE(wxEVT_REFRESH_DISPLAY_WINDOW)

//BEGIN_EVENT_TABLE(AnimationPreviewWindow, wxWindow)
//END_EVENT_TABLE()




AnimationPreviewWindow::AnimationPreviewWindow(
					wxWindow *parent,
					wxWindowID id,
                    const wxPoint &pos,
					const wxSize &size )
        : wxPanel( parent, id, pos, size )
{

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	m_pDisplayWindow = new C_PreviewDisplayWindow(gClipPreviewState->camera, this, wxID_ANY, pos, size);

	sizer->Add(m_pDisplayWindow, wxSizerFlags(1).Expand());

	if(m_pDisplayWindow->Create())
	{
		wxLogMessage("Animation Preview Display Device created successfully!");
		//m_pAuiManager->AddPane(m_pDisplayWindow, wxCENTER, wxT("Display Window"));

		m_section = new C_PreviewSection;
		m_pDisplayWindow->SetSection(m_section);
	}
	else
	{
		m_section = NULL;
		wxLogMessage("ERROR: Animation Preview Display Device failed to create");
		delete m_pDisplayWindow;
		m_pDisplayWindow = NULL;
	}

	SetSizer(sizer);
	
	Connect( wxEVT_REFRESH_DISPLAY_WINDOW, wxCommandEventHandler(AnimationPreviewWindow::OnRefreshDisplayWindow) );
}

AnimationPreviewWindow::~AnimationPreviewWindow()
{
	if (m_pDisplayWindow)
	{
		m_pDisplayWindow->DestroyRenderers();
		//delete m_pDisplayWindow;
	}
	
	gClipPreviewState->DestroyGraphicsResources();

	delete m_section;
	
}

void AnimationPreviewWindow::OnRefreshDisplayWindow( wxCommandEvent& e )
{
	if ( m_pDisplayWindow )
	{
		m_pDisplayWindow->OnIdle( wxIdleEvent() );
		//m_pDisplayWindow->Refresh();
	}
}
