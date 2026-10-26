#include "stdafx.h"

#include <wx/aui/aui.h>
#include <wx/treectrl.h>
#include <wx/msw/registry.h>

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Core.h>

#include <EditorApp/Frame/Frame.h>
#include <EditorApp/Tasks/UITasks.h>
#include <EditorApp/Config.h>
#include <EditorApp/App.h>

#include <EditorApp/Components/ReferenceDialog.h>
#include <EditorApp/Misc/Registry.h>

#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderPipeline.h>

namespace Editor
{
namespace Tasks
{

void Undo()
{
	EditorLib::C_Status status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute("undo", "");
}

void Redo()
{
	EditorLib::C_Status status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute("redo", "");
}
	
void CopyToClipboard()
{
	EditorLib::C_Status status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute("copy", "");
}

void PasteFromClipboard()
{
	EditorLib::C_Status status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute("paste", "");
}

void DuplicateSelection()
{
	EditorLib::C_Status status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute("duplicate", "");
}

void SelectAll()
{
	EditorLib::C_Status status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute("select", "-l");
}

void ShowNodePropertiesWindow()
{
	C_Frame *pFrame = wxGetApp().GetFrame();
	assert(pFrame);
	wxAuiManager *pManager = pFrame->GetAuiManager();
	assert(pManager);
	wxAuiPaneInfo &paneInfo = pManager->GetPane("Pane_Properties");

	if (paneInfo.HasFlag(wxAuiPaneInfo::optionHidden))
	{
		paneInfo.Show(true);
		if (paneInfo.frame && paneInfo.window)
			//HACK: propagate to child
			paneInfo.window->Show(true);

		pManager->Update();
	}

	if (paneInfo.frame)
		paneInfo.frame->Raise();
}

void ShowLayerEditWindow()
{
	C_Frame *pFrame = wxGetApp().GetFrame();
	assert(pFrame);
	wxAuiManager *pManager = pFrame->GetAuiManager();
	assert(pManager);
	wxAuiPaneInfo &paneInfo = pManager->GetPane("Pane_LayerEdit");
	if (paneInfo.HasFlag(wxAuiPaneInfo::optionHidden))
	{
		paneInfo.window->Show(true);
		paneInfo.SetFlag(wxAuiPaneInfo::optionHidden, false);
		pManager->Update();
	}
}

void ShowDopeSheetDialog()
{
	C_Frame *pFrame = wxGetApp().GetFrame();
	assert(pFrame);
	wxAuiManager *pManager = pFrame->GetAuiManager();
	assert(pManager);
	wxAuiPaneInfo &paneInfo = pManager->GetPane("Pane_AnimationDopeSheet");
	if (paneInfo.HasFlag(wxAuiPaneInfo::optionHidden))
	{
		paneInfo.window->Show(true);
		paneInfo.SetFlag(wxAuiPaneInfo::optionHidden, false);
		pManager->Update();
	}
}

void ShowReferencesDialog()
{
	C_Frame *pFrame = wxGetApp().GetFrame();
	assert(pFrame);
	wxAuiManager *pManager = pFrame->GetAuiManager();
	assert(pManager);
	wxAuiPaneInfo &paneInfo = pManager->GetPane("Pane_Reference");
	if (paneInfo.HasFlag(wxAuiPaneInfo::optionHidden))
	{
		paneInfo.window->Show(true);
		paneInfo.SetFlag(wxAuiPaneInfo::optionHidden, false);
		pManager->Update();
	}
}

void Quit()
{
	EditorLib::C_Status status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute("quit");
	if (!status)
		LOGERROR(g_Log, "Failed to quit!");
}

U32 GetAutosaveInterval()
{
	return Registry::Read(wxString(kRegKeyPath) + "\\config", "autosave_interval", 10);
}

void SetAutosaveInterval(U32 interval)
{
	Registry::Write(wxString(kRegKeyPath) + "\\config", "autosave_interval", interval);
	wxGetApp().SetAutosaveInterval(interval);
}

F32 GetGridSize()
{
	return Registry::Read(wxString(kRegKeyPath) + "\\config", "grid_size", 20.f);
}

F32 GetGridSpacing()
{
	return Registry::Read(wxString(kRegKeyPath) + "\\config", "grid_spacing", 1.f);
}

void SetGridSize(F32 size)
{
	Registry::Write(wxString(kRegKeyPath) + "\\config", "grid_size", size);
	C_RenderPipeline::GlobalRenderState& state = 
		C_RenderCore::Instance().GetRenderPipeline().GetGlobalRenderState();
	state.gridSize = size;
}

void SetGridSpacing(F32 spacing)
{
	Registry::Write(wxString(kRegKeyPath) + "\\config", "grid_spacing", spacing);
	C_RenderPipeline::GlobalRenderState& state = 
		C_RenderCore::Instance().GetRenderPipeline().GetGlobalRenderState();
	state.gridSpacing = spacing;
}

bool GetShowUIPromptOption()
{
	return Registry::Read(wxString(kRegKeyPath) + "\\config", "show_prompts", true);
}

void SetShowUIPromptOption(bool show)
{
	Registry::Write(wxString(kRegKeyPath) + "\\config", "show_prompts", show);
}

bool GetShowHUDSelectedActors()
{
	return Registry::Read(wxString(kRegKeyPath) + "\\config", "hud_selected_actors", false);
}

void SetShowHUDSelectedActors(bool show)
{
	Registry::Write(wxString(kRegKeyPath) + "\\config", "hud_selected_actors", show);
	C_RenderPipeline::GlobalRenderState& state = 
		C_RenderCore::Instance().GetRenderPipeline().GetGlobalRenderState();
	state.hudSelectionCount = show;
}

bool GetDisableClearUndoWarning()
{
	return Registry::Read(wxString(kRegKeyPath), "Disable_Clear_Undo_Warning", false);
}

void SetDisableClearUndoWarning(bool disable)
{
	Registry::Write(kRegKeyPath, "Disable_Clear_Undo_Warning", disable);
}

enum E_CharType
{
	E_CharType_Alpha,
	E_CharType_Numeric,
	E_CharType_Whitespace,
	E_CharType_Symbol,
	E_CharType_Null,
};

E_CharType Classify(WCHAR c)
{
	if ((c >= L'A' && c <= L'Z') || (c >= L'a' && c <= L'z'))
		return E_CharType_Alpha;

	if (c >= L'0' && c <= L'9')
		return E_CharType_Numeric;

	switch(c)
	{
	case L' ':
	case L'\t':
	case L'\n':
	case L'\r':
		return E_CharType_Whitespace;
	default:
		return E_CharType_Symbol;
	}
	return E_CharType_Symbol;
}

int CALLBACK EditWordBreakProc(LPTSTR lpch, int ichCurrent, int cch, int code)
{
	LPWSTR lpwch = (LPWSTR)lpch;
	int ret = 0;

	switch(code)
	{
	case WB_LEFT:
	{
		E_CharType type = E_CharType_Null;
		for (ret=ichCurrent-1; ret>0; --ret)
		{
			E_CharType type2 = Classify(lpwch[ret]);
			if (type2 != E_CharType_Null)
			{
				if (type == E_CharType_Null)
					type = type2;
				else if (type != type2)
				{
					++ret;
					break;
				}
			}
		}
		break;
	}
	case WB_ISDELIMITER:
	{
		if (Classify(lpwch[ichCurrent]) == E_CharType_Whitespace)
			ret = TRUE;
		else
		{
			E_CharType type = Classify(lpwch[ichCurrent]);
			E_CharType typePrev = (ichCurrent > 0) ? Classify(lpwch[ichCurrent-1]) : type;
			E_CharType typeNext = (ichCurrent < cch-1) ? Classify(lpwch[ichCurrent+1]) : type;

			if (type != typePrev || type != typeNext)
				ret = TRUE;
		}
		break;
	}
	case WB_RIGHT:
	{
		{
			E_CharType type0 = Classify(lpwch[ichCurrent]);
			E_CharType type1 = (ichCurrent > 0) ? Classify(lpwch[ichCurrent-1]) : type0;

			if (type0 != type1)
			{
				ret = ichCurrent;
				break;
			}
		}
		E_CharType type = E_CharType_Null;
		for (ret=ichCurrent; ret<cch; ++ret)
		{
			E_CharType type2 = Classify(lpwch[ret]);
			if (type2 != E_CharType_Null)
			{
				if (type == E_CharType_Null)
					type = type2;
				else if (type != type2)
				{
					break;
				}
			}
		}
		break;
	}
	};

	return ret;
}

static HHOOK hCallWndProcHook = NULL;
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0)
		return CallNextHookEx(hCallWndProcHook, nCode, wParam, lParam);

	{
		CWPSTRUCT *pCWP = (CWPSTRUCT*)lParam;
		if (pCWP->message == WM_CREATE)
		{
			char wndClass[256];

			if (::GetClassName(pCWP->hwnd, wndClass, sizeof(wndClass)) && strnicmp(wndClass, "edit", 4)==0)
			{
				//hook in our custom word break procedure
				::SendMessage(pCWP->hwnd, EM_SETWORDBREAKPROC, 0, (LPARAM) EditWordBreakProc);
			}
		}
	}

	return CallNextHookEx(hCallWndProcHook, nCode, wParam, lParam);

}

bool InstallWndProcHook()
{
	hCallWndProcHook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, NULL, GetCurrentThreadId());

	return hCallWndProcHook != NULL;
}

bool UninstallWndProcHook()
{
	return UnhookWindowsHookEx(hCallWndProcHook) > 0;
}

}
}
