#ifndef OPTIONSDIALOGS_H
#define OPTIONSDIALOGS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace AnimationTools
{
	class C_SceneConfig;
	class C_CycleParameters;
}

// FIXME: more like skeleton options...
class C_SkeletonOptionsDialog: public wxDialog
{
public:
	C_SkeletonOptionsDialog(
		AnimationTools::C_SceneConfig& sceneConfig,
		wxWindow *parent,
		wxWindowID id=wxID_ANY,
		const wxString& title = wxString(_T("Scene Options")),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxSize(600,400),
		long style = wxDEFAULT_DIALOG_STYLE | wxBORDER_SIMPLE
	);
	
};

class C_CycleOptionsDialog: public wxDialog
{
public:
	C_CycleOptionsDialog(
		AnimationTools::C_SceneConfig& sceneConfig,
		AnimationTools::C_CycleParameters& cycleParameters,
		wxWindow *parent,
		wxWindowID id=wxID_ANY,
		const wxString& title = wxString(_T("Cycle Options")),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxSize(600,400),
		long style = wxDEFAULT_DIALOG_STYLE | wxBORDER_SIMPLE
	);
	
};



#endif
