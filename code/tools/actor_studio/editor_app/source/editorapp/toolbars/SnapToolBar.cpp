#include "stdafx.h"

#include <EditorApp/Misc/EventProxy.h>
#include <EditorApp/ToolBars/SnapToolBar.h>

#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/Manipulators/Manipulator.h>

#include <wx/clntdata.h>

//why must I derive from wxObject and not wxClientData? cf wxTreeCtrl, wxComboBox, wxListBox, wxChoice etc...
class wxIntClientData : public wxObject
{
public:
	wxIntClientData() : m_data(0) { }
    wxIntClientData(S32 data ) : m_data(data) { }
    void SetData(S32 data ) { m_data = data; }
    S32 GetData() const { return m_data; }

private:
	S32 m_data;

	DECLARE_DYNAMIC_CLASS(wxIntClientData)
};
IMPLEMENT_DYNAMIC_CLASS(wxIntClientData,wxObject)

static wxIntClientData snapGridClientData(Editor::E_ManipulatorMode_GridSnap);
static wxIntClientData snapVertexClientData(Editor::E_ManipulatorMode_PointSnap);
static wxIntClientData snapEdgeClientData(Editor::E_ManipulatorMode_EdgeSnap);

wxToolBar* Editor::ToolBars::CreateSnapToolBar(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
	wxToolBar *pSnapToolBar = new wxToolBar(parent, id, pos, size, style);
	pSnapToolBar->SetToolBitmapSize(wxSize(16,16));

	wxToolBarToolBase* pTool = NULL;
	wxImage toolIcon;

	toolIcon.LoadFile(_T("./icons/snap_grid.png"));
	pTool = pSnapToolBar->AddTool(wxID_ANY, "Grid Snap", toolIcon, wxNullBitmap, wxITEM_CHECK, _T("Snap To Grid"));
	pTool->SetClientData(&snapGridClientData);
	wxConnect<wxEVENT_COMMAND_TOOL_CLICKED>(pSnapToolBar, &Editor::ToolBars::OnToggleSnappingMode, pTool->GetId());
	wxConnect<wxEVENT_UPDATE_UI>(pSnapToolBar, &Editor::ToolBars::OnUpdateSnapToolBar, pTool->GetId());

	toolIcon.LoadFile(_T("./icons/snap_vertex.png"));
	pTool = pSnapToolBar->AddTool(wxID_ANY, "Vertex Snap", toolIcon, wxNullBitmap, wxITEM_CHECK, _T("Snap to Vertex"));
	pTool->SetClientData(&snapVertexClientData);
	wxConnect<wxEVENT_COMMAND_TOOL_CLICKED>(pSnapToolBar, &Editor::ToolBars::OnToggleSnappingMode, pTool->GetId());
	wxConnect<wxEVENT_UPDATE_UI>(pSnapToolBar, &Editor::ToolBars::OnUpdateSnapToolBar, pTool->GetId());

	toolIcon.LoadFile(_T("./icons/snap_edge.png"));
	pTool = pSnapToolBar->AddTool(wxID_ANY, "Edge Snap", toolIcon, wxNullBitmap, wxITEM_CHECK, _T("Snap to Edge"));
	pTool->SetClientData(&snapEdgeClientData);
	wxConnect<wxEVENT_COMMAND_TOOL_CLICKED>(pSnapToolBar, &Editor::ToolBars::OnToggleSnappingMode, pTool->GetId());
	wxConnect<wxEVENT_UPDATE_UI>(pSnapToolBar, &Editor::ToolBars::OnUpdateSnapToolBar, pTool->GetId());

	pSnapToolBar->Realize();

	return pSnapToolBar;
}

void Editor::ToolBars::OnToggleSnappingMode(wxCommandEvent& e)
{
	if (e.GetEventObject()->IsKindOf(CLASSINFO(wxToolBar)))
	{
		wxToolBar *pToolBar = (wxToolBar*)e.GetEventObject();
		wxToolBarToolBase *pTool = pToolBar->FindById(e.GetId());
		if (pTool)
		{
			const wxIntClientData *pData = static_cast<wxIntClientData*>(pTool->GetClientData());
			if (pData)
				C_RenderCore::Instance().GetManipulatorManager().ToggleMode((Editor::E_ManipulatorMode)pData->GetData());
		}
	}
}

void Editor::ToolBars::OnUpdateSnapToolBar(wxUpdateUIEvent& e)
{
	if (e.GetEventObject()->IsKindOf(CLASSINFO(wxToolBar)))
	{
		wxToolBar *pToolBar = (wxToolBar*)e.GetEventObject();
		wxToolBarToolBase *pTool = pToolBar->FindById(e.GetId());
		if (pTool)
		{
			const wxIntClientData *pData = static_cast<wxIntClientData*>(pTool->GetClientData());
			if (pData)
				e.Check(C_RenderCore::Instance().GetManipulatorManager().IsModeEnabled((Editor::E_ManipulatorMode)pData->GetData()));
		}
	}
	else
		e.Skip();
}
