#include "stdafx.h"

#include <EditorApp/Misc/EventProxy.h>
#include <EditorApp/ToolBars/DisplayToolBar.h>
#include <EditorApp/editor/RenderCore.h>
#include <EditorApp/editor/RenderPipeline.h>

#include <EditorLib/context/EditorContext.h>

wxToolBar* Editor::ToolBars::CreateDisplayToolBar(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
	wxToolBar *pDisplayToolBar = new wxToolBar(parent, id, pos, size, style);
	pDisplayToolBar->SetToolBitmapSize(wxSize(16,16));

	wxToolBarToolBase* pTool = NULL;
	wxImage toolIcon;

	toolIcon.LoadFile(_T("./icons/locator.png"));
	pTool = pDisplayToolBar->AddTool(wxID_ANY, "locator", toolIcon, wxNullBitmap, wxITEM_CHECK, _T("Locator"));
	wxConnect<wxEVENT_COMMAND_TOOL_CLICKED>(pDisplayToolBar, &Editor::ToolBars::OnToggleDisplayMode, pTool->GetId());
	wxConnect<wxEVENT_UPDATE_UI>(pDisplayToolBar, &Editor::ToolBars::OnUpdateDisplayToolBar, pTool->GetId());
	
	toolIcon.LoadFile(_T("./icons/sprite.png"));
	pTool = pDisplayToolBar->AddTool(wxID_ANY, "sprite", toolIcon, wxNullBitmap, wxITEM_CHECK, _T("Sprite"));
	wxConnect<wxEVENT_COMMAND_TOOL_CLICKED>(pDisplayToolBar, &Editor::ToolBars::OnToggleDisplayMode, pTool->GetId());
	wxConnect<wxEVENT_UPDATE_UI>(pDisplayToolBar, &Editor::ToolBars::OnUpdateDisplayToolBar, pTool->GetId());

	toolIcon.LoadFile(_T("./icons/mesh.png"));
	pTool = pDisplayToolBar->AddTool(wxID_ANY, "mesh", toolIcon, wxNullBitmap, wxITEM_CHECK, _T("Mesh"));
	wxConnect<wxEVENT_COMMAND_TOOL_CLICKED>(pDisplayToolBar, &Editor::ToolBars::OnToggleDisplayMode, pTool->GetId());
	wxConnect<wxEVENT_UPDATE_UI>(pDisplayToolBar, &Editor::ToolBars::OnUpdateDisplayToolBar, pTool->GetId());

	pDisplayToolBar->Realize();

	return pDisplayToolBar;
}

void Editor::ToolBars::OnToggleDisplayMode(wxCommandEvent& e)
{
	if (e.GetEventObject()->IsKindOf(CLASSINFO(wxToolBar)))
	{
		wxToolBar *pToolBar = (wxToolBar*)e.GetEventObject();
		wxToolBarToolBase *pTool = pToolBar->FindById(e.GetId());
		if (pTool)
		{
			C_RenderPipeline::GlobalRenderState& state = 
				C_RenderCore::Instance().GetRenderPipeline().GetGlobalRenderState();
			if(pTool->GetLabel() == "locator")
				state.locatorVisible = !state.locatorVisible;
			else if(pTool->GetLabel() == "sprite")
				state.spriteVisible = !state.spriteVisible;
			else if(pTool->GetLabel() == "mesh")
				state.meshVisible = !state.meshVisible;

			C_RenderCore::Instance().RefreshAllViewports();
		}
	}
}

void Editor::ToolBars::OnUpdateDisplayToolBar(wxUpdateUIEvent& e)
{
	if (e.GetEventObject()->IsKindOf(CLASSINFO(wxToolBar)))
	{
		wxToolBar *pToolBar = (wxToolBar*)e.GetEventObject();
		wxToolBarToolBase *pTool = pToolBar->FindById(e.GetId());
		if (pTool)
		{
			const C_RenderPipeline::GlobalRenderState& state = 
				C_RenderCore::Instance().GetRenderPipeline().GetGlobalRenderState();
			if(pTool->GetLabel() == "locator")
				e.Check(state.locatorVisible);
			else if(pTool->GetLabel() == "sprite")
				e.Check(state.spriteVisible);
			else if(pTool->GetLabel() == "mesh")
				e.Check(state.meshVisible);
		}
	}
	else
		e.Skip();
}