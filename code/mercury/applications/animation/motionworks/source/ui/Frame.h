#ifndef FRAME_H
#define FRAME_H

#include <wx/timer.h>
#include "../Db.h"

namespace AnimationTools
{
	class C_CycleBuilder;
}

class C_SkeletonPanel;
class C_CurveViewerPanel;
class C_XtWindow;
class C_XtTreeView;
class AnimationPreviewWindow;
class ClipBrowser;
class ViewPanel;
class C_AnimationGraphEditor;
class C_ConstraintsPanel;
class C_DynamicsPanel;
class C_JointPropertiesPanel;
class C_TimelinePanel;

class wxAuiManager;
class wxLog;
class wxTextCtrl;
class wxFlatNotebook;

// Access global frame.
extern class C_Frame* gFrame;

class C_Frame : public wxFrame, public C_XtDatabaseListener
{
public:
    C_Frame(const wxString& title, C_XtDatabase& db, C_AnimationGraphEditor& animGraphEditor);
	~C_Frame();

    void OnTimer(wxTimerEvent& e);
    void OnQuit(wxCommandEvent& event);
    void OnHelp(wxCommandEvent& e);
    void OnAbout(wxCommandEvent& event);
    
    
    void OnCharacterMenuCommand(wxCommandEvent& e);
	void OnCameraMenuCommand( wxCommandEvent& e );
	void OnViewMenuCommand(wxCommandEvent& e);
		
	void OnOpenSkeleton( wxCommandEvent& e );
	void OnOpenCycle( wxCommandEvent& e );
	void OnReloadChangedFiles( wxCommandEvent& e );
	void OnExportDynamicJointSettings( wxCommandEvent& e );
	void OnReloadSchema( wxCommandEvent& e );
	void OnDebugMenu( wxCommandEvent& e );
	
	void OnTogglePaused( wxCommandEvent& e );
	void OnIdle(wxIdleEvent& e);
	
	void OnKeyEvent( wxKeyEvent& e );
	void OnSceneScaleChanged( wxCommandEvent& e );
    
    void OnUpdateUI( wxUpdateUIEvent& e );
    void OnPlaybackSpeedSliderScrolled( wxCommandEvent& e );
    void OnPlaybackSpeedTextChanged( wxCommandEvent& e );

	void ShowCurvesForCycle( const AnimationTools::C_CycleBuilder* cycle );

	void SetPlaybackTime( F32 time );
	
	virtual void OnXtDatabaseChanged( Xt::DomEvent* domEvent );
	
	void RequestDisplayUpdate();

private:

	void PopulateToolBar( wxToolBar* toolBar );

	void TogglePane( wxWindow* win );
	bool IsPaneShown( wxWindow* win ) const;
	void ReadConfig();
	void WriteConfig();

	wxAuiManager*			m_pAuiManager;
	//wxDisplayWindow*		m_pDisplayWindow;
	wxLog*					m_pLog;
	wxTextCtrl*				m_pText;
	wxTextCtrl*				m_sceneScaleText;
	wxTimer					m_xtUpdateTimer;
	wxToolBar*				m_toolBar;
	wxSlider*				m_playbackSpeedSlider;
	wxTextCtrl*				m_playbackSpeedText;
	wxImage					m_pauseImage;
	wxImage					m_playImage;
	
	// Blend tree, explorer (could be a dialog), scene hierarchy?
	wxFlatNotebook*			m_notebook;
	
	// Constraints, dynamics properties, joint properties?
	wxFlatNotebook*			m_propertiesNotebook;
	
	// Would be useful to have a joint properties window
	/*
		e.g.
		Name
		Translation
			XYZ
		Rotation
			XYZW
		Scale
			XYZ
		
		Dynamic properties:
			Common:
				Mass
				Air resistance

			Dynamic aim:
				Pitch spring
				Pitch damping
				Heading spring
				Heading damping
				Min pitch limit
				Max pitch limit
				Min heading limit
				Max heading limit

			Dynamic slider:
				Slide spring
				Slide damping
				Min limit
				Max limit
	*/
	
	AnimationPreviewWindow*	m_animationPreviewWindow;

	ClipBrowser*			m_pClipBrowser;
	C_SkeletonPanel*		m_skeletonPanel;
	C_CurveViewerPanel*		m_curveViewerPanel;
	C_XtWindow*				m_xtWindow;
	C_XtTreeView*			m_xtTreeView;
	C_ConstraintsPanel*		m_constraintsPanel;
	C_DynamicsPanel*		m_dynamicsPanel;
	C_JointPropertiesPanel*	m_jointPropertiesPanel;
	C_TimelinePanel*		m_timelinePanel;
	
	C_XtDatabase&			m_db;
	C_AnimationGraphEditor&	m_animGraphEditor;
	
    DECLARE_EVENT_TABLE()
};

#endif
