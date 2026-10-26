#include "stdafx.h"
#include "OptionsDialogs.h"
#include "FloatValidator.h"
#include <animation/tools/Scene.h>
#include <animation/tools/CycleBuilder.h>


void AddFloatOption( wxWindow* parent, wxSizer* parentSizer, const wxString& label, F32* value )
{
	const S32 kBorder = 4;
	wxSizer* s = new wxBoxSizer( wxHORIZONTAL );
	s->Add( new wxStaticText( parent, wxID_ANY, label ), wxSizerFlags().Right().Border(wxALL, kBorder) );
	s->Add( new wxTextCtrl( parent, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, 0, C_FloatValidator(value) ),
		wxSizerFlags().Left().Border(wxALL, kBorder) );
	parentSizer->Add(s, wxSizerFlags().Centre());
}

void AddBoolOption( wxWindow* parent, wxSizer* parentSizer, const wxString& label, bool* value )
{
	const S32 kBorder = 4;
	parentSizer->Add( new wxCheckBox( parent, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(value) ),
		wxSizerFlags().Right().Border( wxALL, kBorder )
	);

	//const S32 kBorder = 4;
	//wxSizer* s = new wxBoxSizer( wxHORIZONTAL );
	//s->Add( new wxStaticText( parent, wxID_ANY, label ), wxSizerFlags().Right().Border(wxALL, kBorder) );
	//s->Add( new wxCheckBox( parent, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(value) ),
	//	wxSizerFlags().Left().Border(wxALL, kBorder) );
	//parentSizer->Add(s);
}

void AddSceneOptions( wxWindow* parent, wxSizer* parentSizer, AnimationTools::C_SceneConfig& sceneConfig )
{
	AddFloatOption( parent, parentSizer, _T("Scene Scale Factor:"), &sceneConfig.scaleFactor );
};


void AddSkeletonOptions( wxWindow* parent, wxSizer* parentSizer )
{
};



//
// C_SkeletonOptionsDialog
//

C_SkeletonOptionsDialog::C_SkeletonOptionsDialog(
	AnimationTools::C_SceneConfig& sceneConfig,
	wxWindow *parent,
	wxWindowID id,
	const wxString& title,
	const wxPoint& pos,
	const wxSize& size,
	long style
):
	wxDialog( parent, id, title, pos, size, style )
{
	wxSizer* topSizer = new wxBoxSizer( wxVERTICAL );
	
	AddSceneOptions( this, topSizer, sceneConfig );
	AddSkeletonOptions( this, topSizer );
		
	wxSizer* buttonSizer = CreateButtonSizer( wxOK | wxCANCEL );
	topSizer->Add( buttonSizer, wxSizerFlags().Right().Border(wxALL, 4) );
	
	SetSizerAndFit( topSizer );
	Layout();
}

//
// C_CycleOptionsDialog
//



void AddCycleOptions( wxWindow* parent, wxSizer* parentSizer, AnimationTools::C_CycleParameters& cycleParams )
{
	const S32 kBorder = 4;
	
	AddFloatOption( parent, parentSizer, _T("Max Rotation Error:"), &cycleParams.maxGlobalRotationError );
	AddFloatOption( parent, parentSizer, _T("Max Translation Error:"), &cycleParams.maxGlobalTranslationError );
	AddBoolOption( parent, parentSizer, _T("Remove Translation Matching Base Pose"), &cycleParams.removeTranslationChannelsMatchingBasePose );
	AddBoolOption( parent, parentSizer, _T("Remove Channels Without Keys"), &cycleParams.removeChannelsWithNoKeys );
	//AddBoolOption( parent, parentSizer, _T("Remove Translation Channels That Match Base Pose:"), &sceneConfig.removeTranslationChannelsMatchingBasePose );
}

C_CycleOptionsDialog::C_CycleOptionsDialog(
	AnimationTools::C_SceneConfig& sceneConfig,
	AnimationTools::C_CycleParameters& cycleParams,
	wxWindow *parent,
	wxWindowID id,
	const wxString& title,
	const wxPoint& pos,
	const wxSize& size,
	long style
):
	wxDialog( parent, id, title, pos, size, style )
{
	wxSizer* topSizer = new wxBoxSizer( wxVERTICAL );
	
	AddSceneOptions( this, topSizer, sceneConfig );
	AddCycleOptions( this, topSizer, cycleParams );
		
	wxSizer* buttonSizer = CreateButtonSizer( wxOK | wxCANCEL );
	topSizer->Add( buttonSizer, wxSizerFlags().Right().Border(wxALL, 4) );
	
	SetSizerAndFit( topSizer );
	Layout();
}
