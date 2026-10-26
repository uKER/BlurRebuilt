#include "stdafx.h"

#include <wx/Dir.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>

#include <System/Misc/Progress.h>
#include <System/Misc/StringUtil.h>
#include <System/Shell/ShellExec.h>

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>
#include <Tools_Shared/Shim/ShimFileCopyXmlExport.h>

#include <EditorLib/Aftk/AftkRefHelpers.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/LevelPack.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Xt/Export/XtBuildLevelPack.h>
#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/CoreConfig.h>
#include <EditorLib/Core.h>

#include <EditorApp/App.h>
#include <EditorApp/Components/LevelSelectDialog.h>
#include <EditorApp/Components/NewLevelDialog.h>
#include <EditorApp/Components/Progress.h>

#include <EditorApp/Editor/RenderCore.h>

#include <EditorApp/Frame/Frame.h>
#include <EditorApp/Misc/RecentFiles.h>
#include <EditorApp/Tasks/LevelTasks.h>
#include <EditorApp/Tasks/UITasks.h>

namespace Editor
{
namespace Tasks
{

wxDateTime g_SessionBeginTime = wxDateTime::GetTimeNow();

inline void SetStatusBarColour(const wxColour& c)
{
	C_ProgressStatusBar *pStatus = (C_ProgressStatusBar*) wxGetApp().GetFrame()->GetStatusBar();
	::SendMessage((HWND)pStatus->GetHandle(), SB_SETBKCOLOR, 0, (LPARAM)RGB(c.Red(),c.Green(),c.Blue()));
}

bool IsLevelLoaded()
{
	return (EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel() != 0);
}

bool IsPackLoaded()
{
	return (EditorLib::C_Core::Instance().GetLevelManager()->GetPacks().Size() != 0);
}

bool IsDbOpen()
{
	return (EditorLib::C_Core::Instance().GetXtManager().GetDb() != 0);
}

void NewLevel()
{
	C_NewLevelDialog newLevelDialog(wxGetApp().GetFrame(), wxID_ANY);
	newLevelDialog.ShowModal();
}

bool Open(XtId id, bool closeOthers)
{
	Btl::String args;

	bool showWarning = false;

	Xt::DomPack* pPack = EditorLib::C_Core::Instance().GetXtManager().FindPack(id);
	if (pPack)
		showWarning = true;

	Xt::DomRecord* pRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(id);
	if (closeOthers || pRecord && EditorLib::C_Core::Instance().GetLevelManager()->GetPacks().Size() && !EditorLib::C_Core::Instance().GetLevelManager()->GetPack(Xt::PackOf(pRecord)->id))
		showWarning = true;

	args.Printf("-o 0x%x", id);

	if (showWarning && EditorLib::C_Core::Instance().GetXtManager().GetDb()->dirty)
	{
		wxString msg;
		msg << "Save changes to entire database? \n Selecting no will revert changes to all open actor lists.";
		wxMessageDialog dialog(wxGetApp().GetFrame(), msg, "Save changes?", wxYES|wxNO|wxCANCEL|wxICON_QUESTION);
		switch (dialog.ShowModal())
		{
		case wxID_CANCEL:
			LOGINFO(g_Log, "User cancelled.");
			return false;
		case wxID_NO:
			{
				LOGINFO(g_Log, "User chose NOT to save changes, reverting changes.");

				Btl::Vector<XtId> levelRecordIds;

				// Store record IDs, since the levels will be invalid once the pack is closed
				Btl::Vector<EditorLib::C_LevelPack*> levelPacks = EditorLib::C_Core::Instance().GetLevelManager()->GetPacks();
				for (Btl::Vector<EditorLib::C_LevelPack*>::Iterator levelPack = levelPacks.Begin(); levelPack != levelPacks.End(); ++levelPack)
				{
					Btl::Vector<EditorLib::C_XtLevel*> levels = (*levelPack)->GetLevels();
					for (Btl::Vector<EditorLib::C_XtLevel*>::Iterator it = levels.Begin(); it != levels.End(); ++it)
					{
						levelRecordIds.PushBack((*it)->GetXtRecordId());
					}
				}

				if (!EditorLib::C_Core::Instance().GetLevelManager()->Close())
					return false;

				// Revert records with previously stored IDs
				for (Btl::Vector<XtId>::Iterator it = levelRecordIds.Begin(); it != levelRecordIds.End(); ++it)
				{
					EditorLib::C_Core::Instance().GetXtManager().RevertRecord(*it);
				}
			}
			break;
		case wxID_YES:
			{
				LOGINFO(g_Log, "User chose save changes.");

				if (!EditorLib::C_Core::Instance().GetXtManager().Save()) 
				{
					LOGINFO(g_Log, "Could not save changes.");
					return false;
				}

				EditorLib::C_Core::Instance().GetLevelManager()->Close();
			}
			break;
		}
	}

	if (closeOthers)
		EditorLib::C_Core::Instance().GetLevelManager()->Close();

	EditorLib::ResultsArray results;
	if (!EditorLib::C_Core::Instance().GetCommandProcessor().Execute("file", args, results))
	{
		wxString msg;
		msg << "Error loading actor list '" << id << "'";
		LOGERROR(g_Log, msg.c_str());
		wxMessageDialog dialog(wxGetApp().GetFrame(), msg, "Error", wxOK|wxICON_ERROR);
		dialog.ShowModal();
	}

	return true;
}

void OpenLevel(bool closeOthers)
{
	const EditorLib::C_CoreConfig& config = EditorLib::C_Core::Instance().GetConfig();

	if (config.xtDbPath.Length())
	{
		//safe to create on modal dialogs on stack...
		C_LevelSelectDialog levelSelect(wxGetApp().GetFrame(), wxID_ANY);
		if (levelSelect.ShowModal() == wxID_OK && levelSelect.GetSelectedItem())
		{
			Xt::DomElement* pSelected = levelSelect.GetSelectedItem();

			BIZ_ASSERT(pSelected);

			XtId id;

			if (pSelected->kind == Xt::kDomPack)
				id = Xt::AsPack(pSelected)->id;
			else if (pSelected->kind == Xt::kDomRecord)
				id = Xt::AsRecord(pSelected)->id;
			else
				return;

			Btl::String name;
			if (pSelected->kind == Xt::kDomPack)
			{
				name = Xt::AsPack(pSelected)->name.Text();
				name += " (Pack)";
			}
			else if (pSelected->kind == Xt::kDomRecord)
			{
				name = Xt::AsRecord(pSelected)->name.Text();
				name += " (Actor List)";
			}

			if (Open(id, closeOthers))
			{
				// Add level to recent files list
				C_RecentFiles::AddRecentFile(name, Btl::String::Format("%x", id));

				Tasks::LoadAllReferences(true);
			}
		}
	}
}

void SetActiveLevel(EditorLib::I_Level* level)
{
	wxMessageDialog dlg(
		0,
		"You will not be able to undo any of your previous changes if you continue, are you sure?",
		"Set active level?",
		wxYES|wxNO|wxICON_QUESTION);

	if (Tasks::GetDisableClearUndoWarning() || dlg.ShowModal() == wxID_YES)
	{
		EditorLib::C_Core::Instance().GetLevelManager()->SetActiveLevel((EditorLib::C_XtLevel*)level);
		C_RenderCore::Instance().DirtyAllViewports();
	}
}

void FastQuit()
{
	//FAST QUIT.
	EditorLib::C_Core::Instance().Shutdown(true);
	LOGINFO(g_Log, "TerminateProcess!");
	g_Log.DeleteAllSubscribers();
	//Avoid atexit calls, Ideally I'd rather call ExitProcess(0)
	TerminateProcess(GetCurrentProcess(), 0);
}

int SaveChanges()
{
	if (!IsLevelLoaded()) 
		return wxID_NO;

	//Prompt to store unsaved changes if db dirty.
	Xt::DomDb* pDb = EditorLib::C_Core::Instance().GetXtManager().GetDb();
	int choice = wxID_CANCEL;
	if (pDb->dirty)
	{
		wxMessageDialog dlg(
			0,
			"Save changes to entire database?",
			"Warning: Scene Not Saved",
			wxYES|wxNO|wxCANCEL | wxICON_QUESTION);
		choice = dlg.ShowModal();
		if (choice == wxID_YES)
		{
			//TODO - message output should come from back-end to prevent code duplication.
			EditorLib::C_Status status;

			LOGINFO(g_Log, "User chose to save changes.");
			status = EditorLib::C_Core::Instance().GetXtManager().Save();
			if (!status) 
			{
				LOGINFO(g_Log, "Could not save changes.");
				return wxID_CANCEL;
			}
		}
		else if (choice == wxID_NO)
		{
			LOGINFO(g_Log, "User chose NOT to save changes.");
		}
		else if (choice == wxID_CANCEL)
		{
			LOGINFO(g_Log, "User chose to cancel level closing.");
		}
	}
	else
		choice = wxID_NO;

	Tasks::EraseAutosaveFiles();
	return choice;
}

bool SaveRevert(EditorLib::I_Level* level)
{
	if (level->GetType() != EditorLib::E_LevelType_XtLevel)
		return true;

	Xt::DomDb* pDb = EditorLib::C_Core::Instance().GetXtManager().GetDb();

	if (!pDb->dirty)
		return true;

	wxMessageDialog dlg(
		0,
		"Do you want to save before closing?\nThis will save all modifications to the entire database.\nSelecting no will revert the changes to the actor list you are closing.",
		"Save?",
		wxYES|wxNO|wxCANCEL | wxICON_QUESTION);
	int choice = dlg.ShowModal();
	if (choice == wxID_YES)
	{
		LOGINFO(g_Log, "User chose to save changes.");
		EditorLib::C_Status status = EditorLib::C_Core::Instance().GetXtManager().Save();
		if (!status) 
		{
			LOGINFO(g_Log, "Could not save changes.");
			return false;
		}
		return true;
	}
	else if (choice == wxID_NO)
	{
		LOGINFO(g_Log, "User chose NOT to save changes, reverting changes.");

		EditorLib::C_Core::Instance().GetXtManager().RevertRecord(((EditorLib::C_XtLevel*)level)->GetXtRecordId());
		
		return true;
	}
	else if (choice == wxID_CANCEL)
	{
		LOGINFO(g_Log, "User chose to cancel save changes.");
		return false;
	}

	return false;
}

bool SaveRevert(EditorLib::C_LevelPack* levelPack)
{
	Xt::DomDb* pDb = EditorLib::C_Core::Instance().GetXtManager().GetDb();

	if (!pDb->dirty)
	{
		if (!EditorLib::C_Core::Instance().GetLevelManager()->ClosePack(levelPack->GetId()))
			return false;

		return true;
	}

	wxMessageDialog dlg(
		0,
		"Do you want to save before closing?\nThis will save all modifications to the entire database.\nSelecting no will revert the changes to all actor lists within the pack you are closing.",
		"Save?",
		wxYES|wxNO|wxCANCEL | wxICON_QUESTION);
	int choice = dlg.ShowModal();
	if (choice == wxID_YES)
	{
		LOGINFO(g_Log, "User chose to save changes.");
		EditorLib::C_Status status = EditorLib::C_Core::Instance().GetXtManager().Save();
		if (!status)
		{
			LOGINFO(g_Log, "Could not save changes.");
			return false;
		}
		
		if (!EditorLib::C_Core::Instance().GetLevelManager()->ClosePack(levelPack->GetId()))
			return false;

		return true;
	}
	else if (choice == wxID_NO)
	{
		LOGINFO(g_Log, "User chose NOT to save changes, reverting changes.");

		Btl::Vector<XtId> levelRecordIds;

		// Store record IDs, since the levels will be invalid once the pack is closed
		Btl::Vector<EditorLib::C_XtLevel*> levels = levelPack->GetLevels();
		for (Btl::Vector<EditorLib::C_XtLevel*>::Iterator it = levels.Begin(); it != levels.End(); ++it)
		{
			levelRecordIds.PushBack((*it)->GetXtRecordId());
		}

		if (!EditorLib::C_Core::Instance().GetLevelManager()->ClosePack(levelPack->GetId()))
			return false;

		// Revert records with previously stored IDs
		for (Btl::Vector<XtId>::Iterator it = levelRecordIds.Begin(); it != levelRecordIds.End(); ++it)
		{
			EditorLib::C_Core::Instance().GetXtManager().RevertRecord(*it);
		}
		
		return true;
	}
	else if (choice == wxID_CANCEL)
	{
		LOGINFO(g_Log, "User chose to cancel save changes.");
		return false;
	}

	return false;
}

bool SaveRevert()
{
	Xt::DomDb* pDb = EditorLib::C_Core::Instance().GetXtManager().GetDb();

	if (!pDb->dirty)
	{
		EditorLib::C_Core::Instance().GetLevelManager()->Close();

		return true;
	}

	wxMessageDialog dlg(
		0,
		"Do you want to save before closing?\nThis will save all modifications to the entire database.\nSelecting no will revert the changes to all actor lists in all currently open packs.",
		"Save?",
		wxYES|wxNO|wxCANCEL | wxICON_QUESTION);
	int choice = dlg.ShowModal();
	if (choice == wxID_YES)
	{
		LOGINFO(g_Log, "User chose to save changes.");
		EditorLib::C_Status status = EditorLib::C_Core::Instance().GetXtManager().Save();
		if (!status)
		{
			LOGINFO(g_Log, "Could not save changes.");
			return false;
		}

		EditorLib::C_Core::Instance().GetLevelManager()->Close();

		return true;
	}
	else if (choice == wxID_NO)
	{
		LOGINFO(g_Log, "User chose NOT to save changes, reverting changes.");

		Btl::Vector<XtId> levelRecordIds;

		// Store record IDs, since the levels will be invalid once the pack is closed
		Btl::Vector<EditorLib::C_LevelPack*> levelPacks = EditorLib::C_Core::Instance().GetLevelManager()->GetPacks();
		for (Btl::Vector<EditorLib::C_LevelPack*>::Iterator levelPack = levelPacks.Begin(); levelPack != levelPacks.End(); ++levelPack)
		{
			Btl::Vector<EditorLib::C_XtLevel*> levels = (*levelPack)->GetLevels();
			for (Btl::Vector<EditorLib::C_XtLevel*>::Iterator it = levels.Begin(); it != levels.End(); ++it)
			{
				levelRecordIds.PushBack((*it)->GetXtRecordId());
			}
		}

		if (!EditorLib::C_Core::Instance().GetLevelManager()->Close())
			return false;

		// Revert records with previously stored IDs
		for (Btl::Vector<XtId>::Iterator it = levelRecordIds.Begin(); it != levelRecordIds.End(); ++it)
		{
			EditorLib::C_Core::Instance().GetXtManager().RevertRecord(*it);
		}
				
		return true;
	}
	else if (choice == wxID_CANCEL)
	{
		LOGINFO(g_Log, "User chose to cancel save changes.");
		return false;
	}

	return false;
}

bool CloseLevel(EditorLib::I_Level* level)
{
	EditorLib::C_ViewDisabler disabler;

	if (!SaveRevert(level))
		return false;

	level->Unload();

	return true;
}

bool ClosePack(EditorLib::C_LevelPack* levelPack, bool saveChangesPrompt)
{
	EditorLib::C_ViewDisabler disabler;

	if (!SaveRevert(levelPack))
		return false;

	return true;
}

bool CloseAllPacks(bool saveChangesPrompt)
{
	EditorLib::C_ViewDisabler disabler;

	if (!IsLevelLoaded()) 
		return true;

	if (!SaveRevert())
		return false;

	return true;
}

bool SaveLevel()
{
	Progress::I_ProgressBar* pProgressBar = wxGetApp().GetFrame()->GetProgressBar();
	pProgressBar->ProgressNotify(0, 50.f, "Saving level ...");

	EditorLib::C_Status status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute("file", "-s");
	if (!status)
	{
		pProgressBar->ProgressNotify(0, 101.f, "Failed to save level");
		SetStatusBarColour(wxColour(255, 128, 128));
		return false;
	}

	EraseAutosaveFiles();

	pProgressBar->ProgressNotify(0, 101.f, "Level saved successfully");
	SetStatusBarColour(wxColour(128, 255, 128));
	return true;
}

void SaveLevelNoReturn()
{
	SaveLevel();
}


void BuildLevel()
{
	EditorLib::C_XtLevel* pLevel = EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel();
	if (!pLevel)
		return;

	const EditorLib::C_CoreConfig& config = EditorLib::C_Core::Instance().GetConfig();

	Progress::I_ProgressBar* pProgressBar = wxGetApp().GetFrame()->GetProgressBar();

	pProgressBar->ProgressNotify(0, 0.f, "Exporting ...");
	EditorLib::C_Status status = EditorLib::C_Core::Instance().ExportLevel();
	if (!status)
	{
		pProgressBar->ProgressNotify(0, 101.f, "Export failed");
		SetStatusBarColour(wxColour(255, 128, 128));

//		Not required anymore due to xt dialog.
//		if (Tasks::GetShowUIPromptOption())
//		{
//			wxMessageDialog dialog(wxGetApp().GetFrame(), "Xt data export failed.", "Error", wxICON_ERROR|wxOK);
//			dialog.ShowModal();
//		}
		return;
	}

	pProgressBar->ProgressNotify(0, 101.f, "Export successful");
	SetStatusBarColour(wxColour(128, 255, 128));
	LOGINFO(g_Log, "Export successful.");

	if (Tasks::GetShowUIPromptOption())
	{
		wxMessageDialog dialog(wxGetApp().GetFrame(), "Export successful.", "Information", wxICON_INFORMATION|wxOK);
		dialog.ShowModal();
	}
}

void ExportToLevel()
{
	C_LevelSelectDialog levelSelect(wxGetApp().GetFrame(), wxID_ANY, "Select the target Level");
	if (levelSelect.ShowModal() == wxID_OK)
	{
		Xt::DomRecord* pRecord = Xt::AsRecord(levelSelect.GetSelectedItem());
		
		if (!pRecord)
			return;

		Xt::DomPack* pPack = Xt::PackOf(const_cast<Xt::DomRecord*>(pRecord));
		assert(pPack);

		Btl::String args;
		args = Btl::String().Printf("-l %08x", pRecord->id);

		EditorLib::C_Status status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute("export", args);
	}
}

bool BuildChangedRecordPack(Xt::DomPack& pack)
{
	//important... ensure the pack id is non zero so that it can be loaded back in.
	pack.id = 0xffffffff;

	Xt::DomDb *pXtDb = EditorLib::C_Core::Instance().GetXtManager().GetDb();

	for(Xt::DomDb::Iterator itPack(pXtDb); itPack; ++itPack)
	{
		for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
		{
			if (itRecord->dirty)
			{
				Xt::DomRecord *pNewRecord = (Xt::DomRecord*) Xt::Clone(itRecord);
				if (!pNewRecord)
					continue;

				pNewRecord->name = itPack->name + "." + pNewRecord->name;
				XtUt::Insert(&pack.records, pNewRecord->id, pNewRecord);
			}
		}
	}
	return XtUt::Count<XtId, Xt::DomRecord, XtUt::LessThan<XtId>>(&pack.records) > 0;
}

Btl::String GenerateFileName()
{
	const EditorLib::C_CoreConfig& config = EditorLib::C_Core::Instance().GetConfig();

	Btl::String file = "save000.xtd";

	const Btl::String backupFolder =  Misc::FormatUnixFileString(config.rootTempPath + "/actorstudio/autosave/");
	const wxString searchPattern = "*.xtd";

	wxDir dir;
	if (dir.Exists(backupFolder.Chars()) && dir.Open(backupFolder.Chars()) && dir.HasFiles(searchPattern))
	{
		wxString ret, tmp;
		for (S32 i=0; i<1000; ++i)
		{
			tmp = wxString::Format("save%03d.xtd", i);
			if (!dir.GetFirst(&ret, tmp))
			{
				file = tmp.c_str();
				break;
			}			
		}
	}

	return backupFolder + file;
}

void EraseAutosaveFiles()
{
	const EditorLib::C_CoreConfig& config = EditorLib::C_Core::Instance().GetConfig();
	const Btl::String backupFolder = Misc::FormatUnixFileString(config.rootTempPath + "/actorstudio/autosave/");

	const wxString searchPattern = "*.xtd";
	
	LOGDEBUG(g_Log, "Deleting autosave files generated since %s", g_SessionBeginTime.Format("%d/%m/%Y %H:%M").c_str());
	wxDir dir;
	if (dir.Exists(backupFolder.Chars()) && dir.Open(backupFolder.Chars()) && dir.HasFiles(searchPattern))
	{
		wxString file;
		wxString path;
			
		dir.GetFirst(&file, searchPattern);
		path = backupFolder.Chars() + file;
		{
			wxFileName fileName(path);	
			if (fileName.GetModificationTime().IsLaterThan(g_SessionBeginTime))
				::DeleteFile(path.c_str());
		}

		while (dir.GetNext(&file))
		{
			path = backupFolder.Chars() + file;
			wxFileName fileName(path);
			if (fileName.GetModificationTime().IsLaterThan(g_SessionBeginTime))
				::DeleteFile(path.c_str());
		}
	}

	//reset start time
	g_SessionBeginTime = wxDateTime::GetTimeNow();
}

bool ExportUnsavedChanges(const char* pFileName)
{
	if (Tasks::IsLevelLoaded())
	{
		Xt::DomPack diff;
		const EditorConfig& config = wxGetApp().GetConfig();
		const wxString backupFolder =  Misc::FormatUnixFileString(Misc::ExtractDirectory(Btl::String(pFileName))).Chars();
		if (!wxDirExists(backupFolder.c_str()))
		{
			char tmp[MAX_PATH];
			memset(tmp, 0, sizeof(char)*MAX_PATH);

			const char* pChar = backupFolder.c_str();
			char* pTmpChar = tmp;
			while (pTmpChar!= tmp+MAX_PATH)
			{
				if (*pChar == '/' && !wxDirExists(tmp) && wxMkDir(tmp)!=0)
					return false;

				*pTmpChar++ = *pChar++;
			}
		}

		if (Tasks::BuildChangedRecordPack(diff))
		{
			wxGetApp().GetFrame()->SetStatusText("Generating Autosave File...");
			diff.name = "ActorStudio Autosave File";
			Xt::ExportPack(&diff, pFileName);
			wxGetApp().GetFrame()->SetStatusText("Autosave File Created");
			LOGINFO(g_Log, "Autosave File Created");
		}
	}
	return false;
}

void ReloadReferences()
{
	//Load all references.
	bool convert = EditorLib::AftkRefHelpers::RefConversionRequired();
	if (convert)
	{
		if (EditorLib::C_Core::Instance().GetConfig().appType == EditorLib::E_AppType_Interactive)
		{
			wxMessageDialog dlg(
				0,
				"Some mesh files need converting, this may take a while. Would you like to convert them now?",
				"Convert meshes?",
				wxYES_NO | wxICON_QUESTION);
			convert = dlg.ShowModal() == wxID_YES;
		}
	}
	EditorLib::AftkRefHelpers::ReloadReferences(convert, true, &AppYield);
}

void LoadAllReferences(bool promptConvert)
{
	//Load all references.
	bool convert = EditorLib::AftkRefHelpers::RefConversionRequired();
	if (promptConvert && convert)
	{
		if (EditorLib::C_Core::Instance().GetConfig().appType == EditorLib::E_AppType_Interactive)
		{
			wxMessageDialog dlg(
				0,
				"Some mesh files need converting, this may take a while. Would you like to convert them now?",
				"Convert meshes?",
				wxYES_NO | wxICON_QUESTION);
			convert = dlg.ShowModal() == wxID_YES;
		}
	}
	EditorLib::AftkRefHelpers::LoadAllReferences(promptConvert && convert, false, &AppYield);
}

void GetLatestReferences()
{
	if (EditorLib::AftkRefHelpers::GetLatestReferences(&AppYield))
	{
		if (IsLevelLoaded())
			EditorLib::AftkRefHelpers::ReloadReferences(true, true, &AppYield);
	}
}





class C_RecoverBackupDialog : public wxDialog
{
public:

	C_RecoverBackupDialog(wxWindow *parent, wxWindowID id=wxID_ANY,
							const wxString& title=wxString("Autosave Recovery"),
							const wxPoint& pos=wxDefaultPosition,
							const wxSize& size=wxSize(550,300),
							long style=wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
	: wxDialog(parent, id, title, pos, size, style)
	{
		wxBoxSizer* pSizer0;
		pSizer0 = new wxBoxSizer(wxVERTICAL);

		wxBoxSizer* pSizer1;
		pSizer1 = new wxBoxSizer(wxHORIZONTAL);

		m_pListCtrl = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxSTATIC_BORDER);
		m_pListCtrl->InsertColumn(0, _T("Name"));
		m_pListCtrl->InsertColumn(1, _T("Date Modified"));

		pSizer1->Add(m_pListCtrl, 1, wxALL|wxEXPAND, 1);

		m_pCheckListCtrl = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0|wxSTATIC_BORDER ); 
		pSizer1->Add(m_pCheckListCtrl, 1, wxALL|wxEXPAND, 0);

		wxBoxSizer* pSizer2;
		pSizer2 = new wxBoxSizer(wxHORIZONTAL);
		
		m_pCheckBox = new wxCheckBox( this, wxID_ANY, wxT("Delete autosave files when dialog closes"), wxDefaultPosition, wxDefaultSize, 0 );
		m_pCheckBox->SetValue(true);

		pSizer2->Add(m_pCheckBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );
		pSizer2->Add(0, 0, 1, wxEXPAND, 0);

		m_pOkButton = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
		pSizer2->Add(m_pOkButton, 0, wxALL, 2);
		
		m_pCancelButton = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
		pSizer2->Add(m_pCancelButton, 0, wxALL, 2);

		pSizer0->Add( pSizer1, 1, wxALL|wxEXPAND, 1 );
		pSizer0->Add( pSizer2, 0, wxALIGN_RIGHT|wxALL|wxEXPAND, 1 );

		SetSizer(pSizer0);
		Layout();

		m_pListCtrl->SetColumnWidth(0, 0.6*m_pListCtrl->GetSize().x);
		m_pListCtrl->SetColumnWidth(1, 0.4*m_pListCtrl->GetSize().x - 4);

		Connect(m_pOkButton->GetId(), wxEVT_UPDATE_UI, wxUpdateUIEventHandler(C_RecoverBackupDialog::OnUpdateUI));
		Connect(m_pOkButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_RecoverBackupDialog::OnButtonClick));

		Connect(m_pCancelButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_RecoverBackupDialog::OnButtonClick));
		
		Connect(m_pListCtrl->GetId(), wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler(C_RecoverBackupDialog::OnBackupSelected));
		Connect(m_pListCtrl->GetId(), wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler(C_RecoverBackupDialog::OnBackupSelected));

		PopulateBackupList();
	}

	void PopulateBackupList()
	{
		const EditorLib::C_CoreConfig& config = EditorLib::C_Core::Instance().GetConfig();
		const Btl::String backupFolder = Misc::FormatUnixFileString(config.rootTempPath + "/actorstudio/autosave/");

		const wxString searchPattern = "*.xtd";

		wxDir dir;
		if (dir.Exists(backupFolder.Chars()) && dir.Open(backupFolder.Chars()) && dir.HasFiles(searchPattern))
		{
			m_pListCtrl->Freeze();
			wxString file;
			
			{
				bool r = dir.GetFirst(&file, searchPattern);
				assert(r);
				
				wxListItem listItem;
				listItem.SetText(file);
				long i = m_pListCtrl->InsertItem(listItem);

				wxFileName fileInfo(wxString(backupFolder.Chars() + file));
				wxDateTime dt = fileInfo.GetModificationTime();
				m_pListCtrl->SetItem(i, 1, dt.Format("%d/%m/%Y %H:%M"));
			}

			while (dir.GetNext(&file))
			{
				wxListItem listItem;
				listItem.SetText(file);
				long i = m_pListCtrl->InsertItem(listItem);

				wxFileName fileInfo(wxString(backupFolder.Chars() + file));
				wxDateTime dt = fileInfo.GetModificationTime();
				m_pListCtrl->SetItem(i, 1, dt.Format("%d/%m/%Y %H:%M"));

			}
			m_pListCtrl->Thaw();
		}
	}

	void PopulateRecordList()
	{
		XtUt::Ptr<Xt::DomPack> xtPack;
		{
			if (m_pListCtrl->GetSelectedItemCount())
			{
				long index = m_pListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
				assert(index != -1);

				const EditorLib::C_CoreConfig& config = EditorLib::C_Core::Instance().GetConfig();
				const Btl::String path = Misc::FormatUnixFileString(config.rootTempPath + "/actorstudio/autosave/" + m_pListCtrl->GetItemText(index).c_str());
				
				xtPack = Xt::ImportPack(path.Chars(), 0);
			}
		}

		m_pCheckListCtrl->Freeze();
		m_pCheckListCtrl->Clear();
		m_records.Clear();
		
		if (!xtPack)
			return;

		for(XtUt::Dictionary<XtId, Xt::DomRecord>::Iterator it(&xtPack->records); it; ++it)
		{
			S32 i = m_pCheckListCtrl->Append((it->name+"."+it->root.type).Text());
			m_pCheckListCtrl->Check(i);
			m_records.PushBack(it->id);	//this should be ok...
		}
		m_pCheckListCtrl->Thaw();
	}

	void OnBackupSelected(wxListEvent& e)
	{
		PopulateRecordList();
	}

	void OnUpdateUI(wxUpdateUIEvent& e)
	{
		if (e.GetId() == m_pOkButton->GetId())
		{
			U32 checkCount = 0;
			for (U32 i=0; i<m_pCheckListCtrl->GetCount(); ++i)
				if (m_pCheckListCtrl->IsChecked(i))
					checkCount++;

			e.Enable(m_pListCtrl->GetSelectedItemCount()>0 && checkCount>0);
		}
	}

	void OnButtonClick(wxCommandEvent& e)
	{
		if (e.GetId() == m_pOkButton->GetId())
		{
			long index = m_pListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			assert(index != -1);

			const EditorLib::C_CoreConfig& config = EditorLib::C_Core::Instance().GetConfig();
			m_path = Misc::FormatUnixFileString(config.rootTempPath + "/actorstudio/autosave/" + m_pListCtrl->GetItemText(index).c_str()).Chars();

			m_selectedRecords.Clear();
			for (U32 i=0; i<m_pCheckListCtrl->GetCount(); ++i)
			{
				if (m_pCheckListCtrl->IsChecked(i))
					m_selectedRecords.PushBack(m_records[i]);
			}
			EndModal(wxID_OK);
		}
		else if (e.GetId() == m_pCancelButton->GetId())
		{
			EndModal(wxID_CANCEL);
		}
	}

	const wxString GetPath() const
	{
		return m_path;
	}

	const Ut::Vector<XtId>& GetSelectedRecords() const
	{
		return m_selectedRecords;
	}

	bool ShouldDeleteBackups() const
	{
		return m_pCheckBox->IsChecked();
	}

private:
	wxListCtrl		*m_pListCtrl;
	wxCheckListBox	*m_pCheckListCtrl;
	wxCheckBox		*m_pCheckBox;
	wxButton		*m_pOkButton;
	wxButton		*m_pCancelButton;

	wxString		m_path;
	Ut::Vector<XtId> m_records;
	Ut::Vector<XtId> m_selectedRecords;
};




bool ImportUnsavedChanges()
{
	//need to close level so that any changes we make are not reverted
	if (IsLevelLoaded())
		CloseAllPacks();

	const EditorLib::C_CoreConfig& config = EditorLib::C_Core::Instance().GetConfig();
	const Btl::String backupFolder = Misc::FormatUnixFileString(config.rootTempPath + "/actorstudio/autosave/");

	const wxString searchPattern = "*.xtd";

	wxDir dir;
	if (dir.Exists(backupFolder.Chars()) && dir.Open(backupFolder.Chars()) && dir.HasFiles(searchPattern))
	{
		bool deleteBackups = false;

		wxMessageDialog dialog(wxGetApp().GetFrame(), "Unsaved changes have been found. Do you want to recover them?", "Warning", wxICON_WARNING|wxYES_NO);
		if (dialog.ShowModal() == wxID_YES)
		{
			C_RecoverBackupDialog backupDialog(wxGetApp().GetFrame());
			if (backupDialog.ShowModal() == wxID_OK)
			{
				XtUt::Ptr<Xt::DomPack> backup = Xt::ImportPack(backupDialog.GetPath().c_str(), 0);
				if (backup)
				{
					//load level if backup contains a level actor list
					const Xt::DomRecord *pLevel = 0;

					const Ut::Vector<XtId>& records = backupDialog.GetSelectedRecords();
					for(XtUt::Dictionary<XtId, Xt::DomRecord>::Iterator it(&backup->records); it; ++it)
					{
						//was the record selected?
						bool found = false;
						for (S32 i=0; i<records.Size(); ++i)
						{
							if (records[i] == it->id)
							{
								found = true;
								break;
							}
						}

						if (!found)
							continue;

						LOGINFO(g_Log, "Autosave loading : %s.%s", backup->name.Text(), it->name.Text());

						//find record to replace
						Xt::DomRecord *pNewRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(it->id);
						if (!pNewRecord || pNewRecord->root.type != it->root.type)
						{
							//find pack..create if necessary
							XtString rName, pName;
							XtsSplit(it->name, '.', pName, rName);
							Xt::DomPack *pPack = EditorLib::C_Core::Instance().GetXtManager().FindPack(Btl::String(pName));
							if (!pPack)
							{
								pPack = Xt::NewPack(EditorLib::C_Core::Instance().GetXtManager().GetDb(), pName.Text());
								assert(pPack);
							}

							assert(pPack);						
							
							pNewRecord = Xt::NewRecord(pPack, rName, it->root.type);
						}
						else
						{
							Xt::Clone(&pNewRecord->root, &it->root);
						}

						if (!pLevel && pNewRecord->root.type == EditorLib::kXtLevelRecordType)
							pLevel = pNewRecord;
					}

					if (pLevel)
					{
						Btl::String args;
						args.Printf("-o 0x%x", pLevel->id);

						Xt::DomPack* pPack = Xt::PackOf(const_cast<Xt::DomRecord*>(pLevel));
						assert(pPack);

						EditorLib::C_Status status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute("file", args);
						if (!status)
						{
							wxString msg;
							msg << "Error loading level '" << pLevel->name.Text() << "' (pack " << pPack->id << ", record " << pLevel->id << ")";
							LOGERROR(g_Log, msg.c_str());
							wxMessageDialog dialog(wxGetApp().GetFrame(), msg, "Error", wxOK|wxICON_ERROR);
							dialog.ShowModal();
						}
					}
				}
			}

			if (backupDialog.ShouldDeleteBackups())
				deleteBackups = true;
		}
		else
			deleteBackups = true;

		if (deleteBackups)
		{
			wxString file;
			wxString path;
				
			dir.GetFirst(&file, searchPattern);
			path = backupFolder.Chars() + file;

			::DeleteFile(path.c_str());
				
			while (dir.GetNext(&file))
			{
				path = backupFolder.Chars() + file;
				::DeleteFile(path.c_str());
			}
		}
	}

	return true;
}

}
}
