#include "stdafx.h"
#include <sigslot/sigslot.h>

#include <DialogueMan/Frame/Frame.h>
#include <DialogueMan/Frame/Menus/FrameMenuBar.h>
#include <wx_shared/EventProxy.h>
#include <wx_shared/MenuBuilder.h>

#include <xtwin/ASyncOpWin.h>
#include "Xt/XtTool.h"

#include <DialogueMan/Actions/GenerateWavs.h>
#include <DialogueMan/Actions/GenerateNames.h>
#include <DialogueMan/Actions/GenerateHashes.h>
#include <DialogueMan/Actions/GenerateSplits.h>
#include <DialogueMan/Actions/TTSFilesInFolder.h>
#include <DialogueMan/Actions/NonTTSFilesInFolder.h>
#include <DialogueMan/Actions/BatchGenerate.h>
#include <DialogueMan/Actions/ExecuteDML.h>
#include <DialogueMan/Actions/AudioXML.h>
#include <DialogueMan/Frame/Menus/FrameToolsMenu.h>

#include <DialogueMan/Document/DbManager.h>

#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>

struct SorceryExportParams
{
	HWND		app;
	bool		dataOnly;
};


namespace DMan
{


C_FrameMenuBar* C_FrameToolsMenu::m_pMenuBar = 0;

C_FrameToolsMenu::C_FrameToolsMenu(C_FrameMenuBar* pMenuBar)
{
	m_pMenuBar = pMenuBar;
}

bool C_FrameToolsMenu::InitialiseUI()
{
	C_Frame* pFrame = m_pMenuBar->GetFrame();
	wxMenuBar* pWxMenuBar = pFrame->GetMenuBar();

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Tools|Build\tF7", pFrame, &C_FrameToolsMenu::OnBuild);
   
	m_pMenuBar->GetMenuBuilder()->AddMenuEntry("Tools|Separator0", Tools::E_MenuItem_Separator);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Tools|Generate Names", pFrame, &C_FrameToolsMenu::OnGenerateNames);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Tools|Generate Hashes", pFrame, &C_FrameToolsMenu::OnGenerateHashes);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Tools|Generate Split Groups", pFrame, &C_FrameToolsMenu::OnGenerateSplitGroups);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Tools|Generate Text-to-Speech", pFrame, &C_FrameToolsMenu::OnGenerateTTS);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Tools|Generate Audio XML", pFrame, &C_FrameToolsMenu::OnAudioXML);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Tools|Display TTS Files in Folder", pFrame, &C_FrameToolsMenu::OnDisplayTTSFilesInFolder);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Tools|Display Non-TTS Files in Folder", pFrame, &C_FrameToolsMenu::OnDisplayNonTTSFilesInFolder);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Tools|Batch Generate...", pFrame, &C_FrameToolsMenu::OnBatchGenerate);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry("Tools|Separator1", Tools::E_MenuItem_Separator);

	m_pMenuBar->GetMenuBuilder()->AddMenuEntry(
		"Tools|Execute DML...", pFrame, &C_FrameToolsMenu::OnExecuteDML);

	return true;
}

void C_FrameToolsMenu::OnBuild(wxCommandEvent& e)
{
	if(wxGetApp().InMemorySavingMode())
	{
		wxMessageDialog msg(m_pMenuBar->GetFrame(), "This is not available in Memory Saving Mode. Please restart in Normal Mode and try again", "Dialogue Manager", wxOK | wxICON_INFORMATION);
		msg.ShowModal();
		return;
	}

	AsyncInvoke((HWND)wxGetApp().GetFrame()->GetHandle(), C_FrameToolsMenu::OnBuildSub, &wxGetApp());
}

void C_FrameToolsMenu::OnGenerateTTS(wxCommandEvent& e)
{
	C_Selection selection;
	wxGetApp().GetSelection(selection);

	if (selection.GetMode() == C_Selection::E_Mode_Pack ||
		selection.GetMode() == C_Selection::E_Mode_Unknown)
	{
 		wxMessageDialog msg(NULL, 
			wxT("Selection is invalid.\n") 
			wxT("Please select a Record or Row and try again\n"), 
			wxT("Generate TTS"),
			wxOK | wxICON_WARNING);
		msg.ShowModal();
		return;
	}

	const C_Selection::ItemArray& arr = selection.GetItems();
	const C_Selection::C_Item& item = arr.Back();
	C_Record* pRecord = wxGetApp().GetDbManager().GetRecord(item.id.GetRecordId());

	GenerateWavs::C_Options options;
	options.SetRecord(pRecord);
	
	GenerateWavs::C_OptionsDlg optionsDlg(NULL, options);
    if (optionsDlg.ShowModal() == wxID_OK )
	{
		GenerateWavs::Do(options);
	}
}

void C_FrameToolsMenu::OnAudioXML(wxCommandEvent& e)
{
	C_Selection selection;
	wxGetApp().GetSelection(selection);

	if (selection.GetMode() == C_Selection::E_Mode_Pack ||
		selection.GetMode() == C_Selection::E_Mode_Unknown)
	{
 		wxMessageDialog msg(NULL, 
			wxT("Selection is invalid.\n") 
			wxT("Please select a Record and try again\n"), 
			wxT("Audio XML"),
			wxOK | wxICON_WARNING);
		msg.ShowModal();
		return;
	}

	const C_Selection::ItemArray& arr = selection.GetItems();
	const C_Selection::C_Item& item = arr.Back();
	C_Record* pRecord = wxGetApp().GetDbManager().GetRecord(item.id.GetRecordId());

	AudioXML::C_Options options;
	options.SetRecord(pRecord);
	
	AudioXML::C_OptionsDlg optionsDlg(NULL, options);
    if (optionsDlg.ShowModal() == wxID_OK )
	{
		AudioXML::Do(options);
	}
}

void C_FrameToolsMenu::OnGenerateNames(wxCommandEvent& e)
{
	C_Selection selection;
	wxGetApp().GetSelection(selection);

	if (selection.GetMode() == C_Selection::E_Mode_Pack ||
		selection.GetMode() == C_Selection::E_Mode_Unknown)
	{
 		wxMessageDialog msg(NULL, 
			wxT("Selection is invalid.\n") 
			wxT("Please select a Record or Row and try again\n"), 
			wxT("Generate Names"),
			wxOK | wxICON_WARNING);
		msg.ShowModal();
		return;
	}

	const C_Selection::ItemArray& arr = selection.GetItems();
	const C_Selection::C_Item& item = arr.Back();
	C_Record* pRecord = wxGetApp().GetDbManager().GetRecord(item.id.GetRecordId());

	GenerateNames::C_Options options;
	options.SetRecord(pRecord);

	GenerateNames::C_OptionsDlg optionsDlg(NULL, options);
    if (optionsDlg.ShowModal() == wxID_OK)
	{
		GenerateNames::Do(options);
	}
}

void C_FrameToolsMenu::OnGenerateHashes(wxCommandEvent& e)
{
	C_Selection selection;
	wxGetApp().GetSelection(selection);

	if (selection.GetMode() == C_Selection::E_Mode_Pack ||
		selection.GetMode() == C_Selection::E_Mode_Unknown)
	{
 		wxMessageDialog msg(NULL, 
			wxT("Selection is invalid.\n") 
			wxT("Please select a Record or Row and try again\n"), 
			wxT("Generate Hashes"),
			wxOK | wxICON_WARNING);
		msg.ShowModal();
		return;
	}

	const C_Selection::ItemArray& arr = selection.GetItems();
	const C_Selection::C_Item& item = arr.Back();
	C_Record* pRecord = wxGetApp().GetDbManager().GetRecord(item.id.GetRecordId());

	GenerateHashes::C_Options options;
	options.SetRecord(pRecord);

	GenerateHashes::Do(options);
}

void C_FrameToolsMenu::OnGenerateSplitGroups(wxCommandEvent& e)
{
	C_Selection selection;
	wxGetApp().GetSelection(selection);

	if (selection.GetMode() == C_Selection::E_Mode_Pack ||
		selection.GetMode() == C_Selection::E_Mode_Unknown)
	{
 		wxMessageDialog msg(NULL, 
			wxT("Selection is invalid.\n") 
			wxT("Please select a Record or Row and try again\n"), 
			wxT("Generate Split Groups"),
			wxOK | wxICON_WARNING);
		msg.ShowModal();
		return;
	}

	const C_Selection::ItemArray& arr = selection.GetItems();
	const C_Selection::C_Item& item = arr.Back();
	C_Record* pRecord = wxGetApp().GetDbManager().GetRecord(item.id.GetRecordId());

	GenerateSplits::C_Options options;
	options.SetRecord(pRecord);

	GenerateSplits::Do(options);
}

void C_FrameToolsMenu::OnExecuteDML(wxCommandEvent& e)
{
	C_Selection selection;
	wxGetApp().GetSelection(selection);

	if ((selection.GetMode() != C_Selection::E_Mode_Row) &&
		(selection.GetMode() != C_Selection::E_Mode_Record))
	{
 		wxMessageDialog msg(NULL, 
			wxT("Selection is invalid.\n") 
			wxT("Please select a Record or Row and try again\n"), 
			wxT("Compose Amax Records"),
			wxOK | wxICON_WARNING);
		msg.ShowModal();
		return;
	}

	const C_Selection::ItemArray& arr = selection.GetItems();
	const C_Selection::C_Item& item = arr.Back();
	C_Record* pRecord = wxGetApp().GetDbManager().GetRecord(item.id.GetRecordId());

	ExecuteDML::C_Options options;
	options.SetRecord(pRecord);

	wxString defaultDir = wxGetApp().GetConfig().configPath;
	wxFileDialog fileDlg( (wxWindow*)(wxGetApp().GetFrame()), "Select DML File", defaultDir, wxEmptyString, wxT("DML files (*.dml)|*.dml"));
	if (fileDlg.ShowModal() == wxID_OK )
	{
		options.SetDMLFile( fileDlg.GetPath() );
		ExecuteDML::Do(options);
	}
}

void C_FrameToolsMenu::OnDisplayTTSFilesInFolder(wxCommandEvent& e)
{
	const wxString& dir = wxDirSelector("Choose a folder");
	if ( !dir.empty() )
	{
		TTSFilesInFolder::C_Options options;
		options.SetFolder(dir);

		TTSFilesInFolder::Do(options);
	}
}

void C_FrameToolsMenu::OnDisplayNonTTSFilesInFolder(wxCommandEvent& e)
{
	const wxString& dir = wxDirSelector("Choose a folder");
	if ( !dir.empty() )
	{
		NonTTSFilesInFolder::C_Options options;
		options.SetFolder(dir);

		NonTTSFilesInFolder::Do(options);
	}
}

void C_FrameToolsMenu::OnBatchGenerate(wxCommandEvent& e)
{
	Btl::Vector<Xt::DomRecord*>& records = wxGetApp().GetRecordsInCurrentFolder();

	if (records.Size() == 0)
	{
 		wxMessageDialog msg(NULL, 
			wxT("No records found in current folder.\n") 
			wxT("Select another folder and try again\n"), 
			wxT("Batch Generate..."),
			wxOK | wxICON_WARNING);
		msg.ShowModal();
		return;
	}

	BatchGenerate::C_Options options;
	options.SetRecords(records);

	BatchGenerate::C_OptionsDlg optionsDlg(NULL, options);
	if (optionsDlg.ShowModal() == wxID_OK)
	{
		BatchGenerate::Do(options);
	}
}

struct SorceryBuildParams : public XtBuildParams
{
	AsyncOp*	op;
	int			warnings;
	int			errors;

public:
	SorceryBuildParams() :
		op(0),
		warnings(0),
		errors(0)
	{
	}

public:
	virtual void Output(XtSeverity severity, const char* text)
	{
		switch(severity)
		{
			case kXtSevInfo:	op->NewMessage(kAoiInfo, text);		break;
			case kXtSevWarning:	op->NewMessage(kAoiWarning, text);	break;
			case kXtSevError:	op->NewMessage(kAoiError, text);	break;
			case kXtSevTrace:	op->AppendMessage(text);			break;
		}

		if (severity == kXtSevWarning)
			warnings++;

		if (severity == kXtSevError)
			errors++;
	}

	virtual void Progress(float fraction)
	{
		op->SetProgress(fraction);
	}

	virtual bool IsCancelled()
	{
		return op->IsCancelled();
	}
};

bool C_FrameToolsMenu::OnBuildSub(AsyncOp* op, C_App* app)
{
	SorceryBuildParams params;

	params.db = app->GetDb();
	params.op = op;

	/*if (sep->dataOnly)
	{
		params.doData			= true;
		params.doLocalisation	= false;
		params.doDeploy			= true;
	}*/

	op->SetTitle("Exporting...");
	op->NewMessage(kAoiInfo, "Exporting:");
	op->AppendMessage("Database: " + app->GetDb()->path);

	XtBuild(&params);

	if ((params.warnings > 0) || (params.errors > 0))
	{
		op->SetTitle("Export failed");

		XtString msg = Format("Export failed, %i warning(s) and %i error(s)", params.warnings, params.errors);

		op->NewMessage(kAoiWarning, msg);
	}
	else
	{
		op->SetTitle("Export complete");
		op->NewMessage(kAoiInfo, "Export complete");
	}

	return true;
}

}
