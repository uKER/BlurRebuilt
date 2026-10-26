#include "stdafx.h"

#include <EditorApp/Misc/EventProxy.h>
#include <EditorApp/ToolBars/StandardToolBar.h>
#include <EditorApp/Tasks/LevelTasks.h>
#include <EditorApp/Tasks/UITasks.h>

using namespace Editor;

void OpenLevel()
{
	Tasks::OpenLevel(true);
}

wxToolBar* ToolBars::CreateStandardToolBar(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
	wxToolBar *pStandardToolBar = new wxToolBar(parent, id, pos, size, style);
	pStandardToolBar->SetToolBitmapSize(wxSize(16,16));
	wxToolBarToolBase* pTool = NULL;
	wxImage toolIcon;

	toolIcon.LoadFile(_T("./icons/document_new_unlined_16.png"));
	pTool = pStandardToolBar->AddTool(wxID_ANY, "New", toolIcon, wxNullBitmap, wxITEM_NORMAL, _T("New"));
	wxConnect<wxEVENT_COMMAND_TOOL_CLICKED>(pStandardToolBar, &Tasks::NewLevel, false, pTool->GetId());

	toolIcon.LoadFile(_T("./icons/document_open_16.png"));
	pTool = pStandardToolBar->AddTool(wxID_ANY, "Open", toolIcon, wxNullBitmap, wxITEM_NORMAL, _T("Open"));
	wxConnect<wxEVENT_COMMAND_TOOL_CLICKED>(pStandardToolBar, &OpenLevel, false, pTool->GetId());

	toolIcon.LoadFile(_T("./icons/save_16.png"));
	pTool = pStandardToolBar->AddTool(wxID_ANY, "Save", toolIcon, wxNullBitmap, wxITEM_NORMAL, _T("Save"));
	wxConnect<wxEVENT_COMMAND_TOOL_CLICKED>(pStandardToolBar, &Tasks::SaveLevelNoReturn, false, pTool->GetId());

	pStandardToolBar->AddSeparator();

	toolIcon.LoadFile(_T("./icons/undo_round_16.png"));
	pTool = pStandardToolBar->AddTool(wxID_ANY, "Undo", toolIcon, wxNullBitmap, wxITEM_NORMAL, _T("Undo"));
	wxConnect<wxEVENT_COMMAND_TOOL_CLICKED>(pStandardToolBar, &Tasks::Undo, false, pTool->GetId());

	toolIcon.LoadFile(_T("./icons/redo_round_16.png"));
	pTool = pStandardToolBar->AddTool(wxID_ANY, "Redo", toolIcon, wxNullBitmap, wxITEM_NORMAL, _T("Redo"));
	wxConnect<wxEVENT_COMMAND_TOOL_CLICKED>(pStandardToolBar, &Tasks::Redo, false, pTool->GetId());

	toolIcon.LoadFile(_T("./icons/clipboard_copy_unlined_16.png"));
	pTool = pStandardToolBar->AddTool(wxID_ANY, "Copy", toolIcon, wxNullBitmap, wxITEM_NORMAL, _T("Copy"));
	wxConnect<wxEVENT_COMMAND_TOOL_CLICKED>(pStandardToolBar, &Tasks::CopyToClipboard, false, pTool->GetId());

	toolIcon.LoadFile(_T("./icons/clipboard_paste_unlined_16.png"));
	pTool = pStandardToolBar->AddTool(wxID_ANY, "Paste", toolIcon, wxNullBitmap, wxITEM_NORMAL, _T("Paste"));
	wxConnect<wxEVENT_COMMAND_TOOL_CLICKED>(pStandardToolBar, &Tasks::PasteFromClipboard, false, pTool->GetId());

	toolIcon.LoadFile(_T("./icons/references_getlatest_16.png"));
	pTool = pStandardToolBar->AddTool(wxID_ANY, "GetLatestRefs", toolIcon, wxNullBitmap, wxITEM_NORMAL, _T("Get Latest References"));
	wxConnect<wxEVENT_COMMAND_TOOL_CLICKED>(pStandardToolBar, &Tasks::GetLatestReferences, false, pTool->GetId());

	pStandardToolBar->Realize();

	return pStandardToolBar;
}