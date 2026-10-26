#include "stdafx.h"
#include <wx/treectrl.h>
#include <wx/minifram.h>

#include <xtwin/TreeCtrl.h>

#include <EditorLib/Context/EditorContext.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Core.h>

#include <EditorApp/Input_Handlers/ShortcutHandler.h>

#include <EditorApp/components/SplitterControl.h>
#include <EditorApp/editor/RenderViewport.h>
#include <EditorApp/editor/RenderCore.h>
#include <EditorApp/editor/RenderPipeline.h>
#include <EditorApp/editor/RenderScene.h>
#include <EditorApp/editor/manipulators/contexts/IManipulatorContext.h>
#include <EditorApp/editor/manipulators/Manipulator.h>

#include <EditorLib/Aftk/AftkRefHelpers.h>

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>
#include <EditorApp/Config.h>
#include <EditorApp/App.h>
#include <EditorApp/Frame/Frame.h>
#include <EditorApp/Tasks/LevelTasks.h>
#include <EditorApp/Components/scene_graph/SceneGraphPanel.h>

namespace Editor
{

C_ShortcutHandler::C_ShortcutHandler()
: wxEvtHandler()
, m_pLastFocusedViewport(0)
, m_acceleratorDisable(false)
{
	C_App *pApp = &wxGetApp();

	const RenderViewportArray& viewports = C_RenderCore::Instance().GetRenderViewports();
	for (S32 i=0; i<viewports.Size(); ++i)
		viewports[i]->Connect(viewports[i]->GetId(), wxEVT_SET_FOCUS, wxFocusEventHandler(C_ShortcutHandler::OnViewportFocus), NULL, this);

	m_snapToggle[E_ManipulatorMode_PointSnap] = m_snapToggle[E_ManipulatorMode_EdgeSnap] = m_snapToggle[E_ManipulatorMode_GridSnap] = false;

	pApp->Connect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(C_ShortcutHandler::OnIdle), NULL, this);

	viewports[0]->SetFocus();
}

C_ShortcutHandler::~C_ShortcutHandler()
{
	C_App *pApp = &wxGetApp();

	const RenderViewportArray& viewports = C_RenderCore::Instance().GetRenderViewports();
	for (S32 i=0; i<viewports.Size(); ++i)
		viewports[i]->Disconnect(viewports[i]->GetId(), wxEVT_SET_FOCUS, wxFocusEventHandler(C_ShortcutHandler::OnViewportFocus), NULL, this);

	pApp->Disconnect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(C_ShortcutHandler::OnIdle), NULL, this);
}

bool C_ShortcutHandler::ProcessKey(WXMSG *msg)
{
	wxWindow* eventWindow = wxGetWindowFromHWND((WXHWND)msg->hwnd);

	if (m_acceleratorTable.Translate(eventWindow, msg))
		return true;

	bool isExtendedKey = (msg->lParam & 1<<24) != 0;

	if (msg->message == WM_KEYUP)
	{
		switch(msg->wParam)
		{
		case 'V':
			if (m_snapToggle[E_ManipulatorMode_PointSnap])
				C_RenderCore::Instance().GetManipulatorManager().ToggleMode(E_ManipulatorMode_PointSnap);
			m_snapToggle[E_ManipulatorMode_PointSnap] = false;
			return true;

		case 'C':
			if (m_snapToggle[E_ManipulatorMode_EdgeSnap])
				C_RenderCore::Instance().GetManipulatorManager().ToggleMode(E_ManipulatorMode_EdgeSnap);
			m_snapToggle[E_ManipulatorMode_EdgeSnap] = false;
			return true;

		case 'X':
			if (m_snapToggle[E_ManipulatorMode_GridSnap])
				C_RenderCore::Instance().GetManipulatorManager().ToggleMode(E_ManipulatorMode_GridSnap);
			m_snapToggle[E_ManipulatorMode_GridSnap] = false;
			return true;
		};
	}
	else if (msg->message == WM_KEYDOWN)
	{
		const U32 previousStateFlag = 1 << 30;
		if ((msg->lParam & previousStateFlag)>0)
			return false;	//key state hasn't changed

		if (!m_pLastFocusedViewport)
			return false;

		C_RenderViewport *pViewportAtCursor = 0;
		C_SceneGraphPanel *pSceneGraphAtCursor = 0;
		wxWindow *pWindowAtPtr = wxFindWindowAtPointer(wxGetMousePosition());
		if (pWindowAtPtr && pWindowAtPtr->IsKindOf(CLASSINFO(C_RenderViewport)))
			pViewportAtCursor = (C_RenderViewport*)pWindowAtPtr;
		else if (pWindowAtPtr)
		{
			wxWindow *pParent = pWindowAtPtr;
			while (pParent)
			{
				if (pParent->IsKindOf(CLASSINFO(C_SceneGraphPanel)))
				{
					pSceneGraphAtCursor = (C_SceneGraphPanel*)pParent;
					break;
				}
				pParent = pParent->GetParent();
			}
		}

		if (pWindowAtPtr)
		{
			//raise the top level window.
			wxWindow *pParent = pWindowAtPtr;
			while (pParent)
			{
				if (pParent->IsTopLevel())
				{
					static_cast<wxTopLevelWindow*>(pParent)->Raise();
					break;
				}
				else
					pParent = pParent->GetParent();
			}
		}

		if (pViewportAtCursor && pViewportAtCursor != m_pLastFocusedViewport)
			pViewportAtCursor->SetFocus();

		if (isExtendedKey)
		{
			switch(msg->wParam)
			{
				case VK_DELETE:
				{
					if (Tasks::IsLevelLoaded())
					{
						EditorLib::C_Status status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute("delete", "");
						if (!status)
							LOGERROR(g_Log, "Deletion failed");
					}
					return true;
				}
				#ifdef _DEBUG
				case VK_INSERT:
				{
					if(Tasks::IsLevelLoaded() && GetKeyState(VK_CONTROL) & 0x8000)
						Tasks::ExportUnsavedChanges();
					return true;
				}
				case VK_HOME:
				{
					if(!Tasks::IsLevelLoaded() && GetKeyState(VK_CONTROL) & 0x8000)
						Tasks::ImportUnsavedChanges();
					return true;
				}
				#endif
			}
		}
		else
		{
			switch(msg->wParam)
			{
			case 'V':
				C_RenderCore::Instance().GetManipulatorManager().ToggleMode(E_ManipulatorMode_PointSnap);
				m_snapToggle[E_ManipulatorMode_PointSnap] = true;
				return true;

			case 'C':
				C_RenderCore::Instance().GetManipulatorManager().ToggleMode(E_ManipulatorMode_EdgeSnap);
				m_snapToggle[E_ManipulatorMode_EdgeSnap] = true;
				return true;

			case 'X':
				C_RenderCore::Instance().GetManipulatorManager().ToggleMode(E_ManipulatorMode_GridSnap);
				m_snapToggle[E_ManipulatorMode_GridSnap] = true;
				return true;
			case 'Q':
				if (m_pLastFocusedViewport && m_pLastFocusedViewport != wxWindow::GetCapture())
					EditorLib::C_Core::Instance().SetContext(EditorLib::E_EditorContext_Select);
				return true;
			case 'W':
				if (m_pLastFocusedViewport && m_pLastFocusedViewport != wxWindow::GetCapture())
					EditorLib::C_Core::Instance().SetContext(EditorLib::E_EditorContext_Move);
				return true;
			case 'E':
				if (m_pLastFocusedViewport && m_pLastFocusedViewport != wxWindow::GetCapture())
					EditorLib::C_Core::Instance().SetContext(EditorLib::E_EditorContext_Rotate);
				return true;
			case 'R':
				if (m_pLastFocusedViewport && m_pLastFocusedViewport != wxWindow::GetCapture())
					EditorLib::C_Core::Instance().SetContext(EditorLib::E_EditorContext_Scale);
				return true;
			case 'F':
			{
				if (pSceneGraphAtCursor)
					pSceneGraphAtCursor->FrameSelection();
				else
					C_RenderCore::Instance().FrameSelection(m_pLastFocusedViewport);
				return true;
			}
			case 'A':
				C_RenderCore::Instance().FrameScene(m_pLastFocusedViewport);
				return true;
			case VK_BACK:
			{
				if (Tasks::IsLevelLoaded())
				{
					EditorLib::C_Status status = EditorLib::C_Core::Instance().GetCommandProcessor().Execute("delete", "");
					if (!status)
						LOGERROR(g_Log, "Deletion failed");
				}
				return true;
			}
			case VK_SPACE:
			{
				wxWindow *pChild = m_pLastFocusedViewport;
				wxWindow *pParent = pChild->GetParent();
				if (pParent)
				{
					//check grandparent window
					if (!pParent->IsKindOf(CLASSINFO(wxSplitterCtrl)))
					{
						pChild = pParent;
						pParent = pParent->GetParent();
					}

					if (pParent && pParent->IsKindOf(CLASSINFO(wxSplitterCtrl)))
					{
						wxSplitterCtrl *pSplitter = (wxSplitterCtrl*)pParent;
						if (pSplitter->GetLayout() == E_SplitterLayout_1)
							pSplitter->ChangeLayout(E_SplitterLayout_4);
						else
						{
							if (pSplitter->IsExpanded())
								pSplitter->UnexpandWindow();
							else
								pSplitter->ExpandWindow(pChild);
						}
					}
				}
				return true;
			}
			case '4':
			{
				C_RenderPipeline::ViewportRenderState& state = 
					C_RenderCore::Instance().GetRenderPipeline().GetViewportRenderState(m_pLastFocusedViewport);
				state.fillmode = Display::E_FillMode_Wireframe;
				C_RenderCore::Instance().RefreshAllViewports();
				return true;
			}
			case '5':
			{
				C_RenderPipeline::ViewportRenderState& state = 
					C_RenderCore::Instance().GetRenderPipeline().GetViewportRenderState(m_pLastFocusedViewport);
				state.fillmode = Display::E_FillMode_Solid;
				state.textured = false;
				C_RenderCore::Instance().RefreshAllViewports();
				return true;
			}
			case '6':
			{
				C_RenderPipeline::ViewportRenderState& state = 
					C_RenderCore::Instance().GetRenderPipeline().GetViewportRenderState(m_pLastFocusedViewport);
				state.fillmode = Display::E_FillMode_Solid;
				state.textured = true;
				C_RenderCore::Instance().GetRenderPipeline().LoadTextures();
				C_RenderCore::Instance().RefreshAllViewports();
				return true;
			}
			case '-':
			case 390:
			{
				I_Manipulator::SetScreenSize(I_Manipulator::GetScreenSize()-20);
				C_RenderCore::Instance().RefreshAllViewports();
				return true;
			}
			case '+':
			case 388:
			{
				I_Manipulator::SetScreenSize(I_Manipulator::GetScreenSize()+20);
				C_RenderCore::Instance().RefreshAllViewports();
				return true;
			}
			case '7':
			{
				C_RenderPipeline::ViewportRenderState& state = 
					C_RenderCore::Instance().GetRenderPipeline().GetViewportRenderState(m_pLastFocusedViewport);
				state.fillmode = Display::E_FillMode_Solid;
				state.lit = !state.lit;
				C_RenderCore::Instance().GetRenderPipeline().LoadTextures();
				C_RenderCore::Instance().RefreshAllViewports();
				return true;
			}
			case VK_F5:
				Tasks::ReloadReferences();
				return true;
			};
		}
	}

	return false;
}

void C_ShortcutHandler::OnIdle(wxIdleEvent& e)
{
	// Check if an accelerator table exists, if not then no new menu items have been added
	wxAcceleratorTable* pAcceleratorTable = wxGetApp().GetFrame()->GetMenuBar()->GetAcceleratorTable();
	if (!pAcceleratorTable || !pAcceleratorTable->IsOk())
		return;

	// Build the accelerator table so we can steal the shortcuts
	wxGetApp().GetFrame()->GetMenuBar()->RebuildAccelTable();
	pAcceleratorTable = wxGetApp().GetFrame()->GetMenuBar()->GetAcceleratorTable();
	
	// Populate our local table
	if (pAcceleratorTable && pAcceleratorTable->IsOk())
		m_acceleratorTable = *pAcceleratorTable;
	else
		m_acceleratorTable = wxNullAcceleratorTable;

	// Remove the accelerator table, so only ours is used
	wxGetApp().GetFrame()->GetMenuBar()->SetAcceleratorTable(wxNullAcceleratorTable);
}

const C_RenderViewport* C_ShortcutHandler::GetLastFocusedViewport() const
{
	return m_pLastFocusedViewport;
}

C_RenderViewport* C_ShortcutHandler::GetLastFocusedViewport()
{
	return m_pLastFocusedViewport;
}

void C_ShortcutHandler::OnViewportFocus(wxFocusEvent& e)
{
	wxWindowList::compatibility_iterator it = wxTopLevelWindows.GetFirst();
	while (it)
	{
		wxWindow* wnd = it->GetData();
		if (wnd->IsKindOf(CLASSINFO(wxMiniFrame)))
			static_cast<wxMiniFrame*>(wnd)->SetAcceleratorTable(wxNullAcceleratorTable);

		it = it->GetNext();
	}

	// Set the focused viewport
	if (e.GetEventObject()->IsKindOf(CLASSINFO(C_RenderViewport)))
		m_pLastFocusedViewport = (C_RenderViewport*)e.GetEventObject();

	e.Skip();
}

}
