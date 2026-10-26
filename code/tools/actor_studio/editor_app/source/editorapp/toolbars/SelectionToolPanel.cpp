#include "stdafx.h"

#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Core.h>

#include <EditorApp/toolbars/SelectionToolPanel.h>
#include <EditorApp/Editor/manipulators/Manipulator.h>
#include <EditorApp/editor/manipulators/SelectManipulator.h>
#include <EditorApp/misc/WxEditorIds.h>

#include <wx/msw/registry.h>
#include <EditorApp/Misc/Registry.h>

using namespace Editor;

C_SelectionToolPanel::C_SelectionToolPanel(
	wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: wxPanel(parent, id, pos, size, style)
{
	wxBoxSizer* pSizer;
	pSizer = new wxBoxSizer(wxHORIZONTAL);
	
	wxToolBarToolBase* pTool = NULL;
	wxImage toolIcon;
	S32 objectModeId = -1, componentModeId = -1;

	m_pMainToolBar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_NODIVIDER);
	m_pMainToolBar->SetToolBitmapSize(wxSize(16,16));

	toolIcon.LoadFile(_T("./icons/selection_object.png"));
	pTool = m_pMainToolBar->AddTool(wxID_EDITOR_SELECTION_OBJECT, _T("Select by object type"), toolIcon, wxNullBitmap, wxITEM_RADIO, _T("Object Mode Selection"));
	Connect(pTool->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(C_SelectionToolPanel::OnSelectionMode));
	Connect(pTool->GetId(), wxEVT_UPDATE_UI, wxUpdateUIEventHandler(C_SelectionToolPanel::OnUpdateSelectionMode));
	objectModeId = pTool->GetId();
	m_ToolBarItemIdMap.Insert(Ut::UtMap<S32, S32>::DataType(pTool->GetId(), E_SelectionMode_Object));

	toolIcon.LoadFile(_T("./icons/selection_component.png"));
	pTool = m_pMainToolBar->AddTool(wxID_EDITOR_SELECTION_COMPONENT, _T("Select by component type"), toolIcon, wxNullBitmap, wxITEM_RADIO, _T("Component Mode Selection"));
	Connect(pTool->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(C_SelectionToolPanel::OnSelectionMode));
	Connect(pTool->GetId(), wxEVT_UPDATE_UI, wxUpdateUIEventHandler(C_SelectionToolPanel::OnUpdateSelectionMode));
	componentModeId = pTool->GetId();
	m_ToolBarItemIdMap.Insert(Ut::UtMap<S32, S32>::DataType(pTool->GetId(), E_SelectionMode_Component));

	m_pMainToolBar->AddSeparator();
	m_pMainToolBar->Realize();

	wxToolBar *pObjectSelectFilter = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_NODIVIDER);
	pObjectSelectFilter->SetToolBitmapSize(wxSize(16,16));

	toolIcon.LoadFile(_T("./icons/selection_objectmesh.png"));
	pTool = pObjectSelectFilter->AddTool(wxID_EDITOR_SELECTION_OBJECT_MESH, _T("Select object mesh"), toolIcon, wxNullBitmap, wxITEM_CHECK, _T("Select Object Mesh"));
	Connect(pTool->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(C_SelectionToolPanel::OnSelectionFilter));
	Connect(pTool->GetId(), wxEVT_UPDATE_UI, wxUpdateUIEventHandler(C_SelectionToolPanel::OnUpdateSelectionFilter));
	m_ToolBarItemIdMap.Insert(Ut::UtMap<S32, S32>::DataType(pTool->GetId(), 0x00000001));

	toolIcon.LoadFile(_T("./icons/selection_objectlocator.png"));
	pTool = pObjectSelectFilter->AddTool(wxID_EDITOR_SELECTION_OBJECT_LOCATOR, _T("Select object locator"), toolIcon, wxNullBitmap, wxITEM_CHECK, _T("Select Object Locator"));
	Connect(pTool->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(C_SelectionToolPanel::OnSelectionFilter));
	Connect(pTool->GetId(), wxEVT_UPDATE_UI, wxUpdateUIEventHandler(C_SelectionToolPanel::OnUpdateSelectionFilter));
	m_ToolBarItemIdMap.Insert(Ut::UtMap<S32, S32>::DataType(pTool->GetId(), 0x00000002));

	toolIcon.LoadFile(_T("./icons/selection_objectproperty.png"));
	pTool = pObjectSelectFilter->AddTool(wxID_EDITOR_SELECTION_OBJECT_PROPERTY, _T("Select object property"), toolIcon, wxNullBitmap, wxITEM_CHECK, _T("Select Object Property"));
	Connect(pTool->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(C_SelectionToolPanel::OnSelectionFilter));
	Connect(pTool->GetId(), wxEVT_UPDATE_UI, wxUpdateUIEventHandler(C_SelectionToolPanel::OnUpdateSelectionFilter));
	m_ToolBarItemIdMap.Insert(Ut::UtMap<S32, S32>::DataType(pTool->GetId(), 0x00000004));

	pObjectSelectFilter->Realize();

	wxToolBar *pComponentSelectFilter = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_NODIVIDER);
	pComponentSelectFilter ->SetToolBitmapSize(wxSize(16,16));

	toolIcon.LoadFile(_T("./icons/selection_objectmesh.png"));
	pTool = pComponentSelectFilter->AddTool(wxID_ANY, _T("Select object mesh"), toolIcon, wxNullBitmap, wxITEM_CHECK, _T("Select Object Mesh"));
	Connect(pTool->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(C_SelectionToolPanel::OnSelectionFilter));
	Connect(pTool->GetId(), wxEVT_UPDATE_UI, wxUpdateUIEventHandler(C_SelectionToolPanel::OnUpdateSelectionFilter));
	m_ToolBarItemIdMap.Insert(Ut::UtMap<S32, S32>::DataType(pTool->GetId(), 0x00000001));

	toolIcon.LoadFile(_T("./icons/selection_objectlocator.png"));
	pTool = pComponentSelectFilter->AddTool(wxID_ANY, _T("Select object locator"), toolIcon, wxNullBitmap, wxITEM_CHECK, _T("Select Object Locator"));
	Connect(pTool->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(C_SelectionToolPanel::OnSelectionFilter));
	Connect(pTool->GetId(), wxEVT_UPDATE_UI, wxUpdateUIEventHandler(C_SelectionToolPanel::OnUpdateSelectionFilter));
	m_ToolBarItemIdMap.Insert(Ut::UtMap<S32, S32>::DataType(pTool->GetId(), 0x00000002));

	toolIcon.LoadFile(_T("./icons/selection_objectproperty.png"));
	pTool = pComponentSelectFilter->AddTool(wxID_ANY, _T("Select object property"), toolIcon, wxNullBitmap, wxITEM_CHECK, _T("Select Object Property"));
	Connect(pTool->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(C_SelectionToolPanel::OnSelectionFilter));
	Connect(pTool->GetId(), wxEVT_UPDATE_UI, wxUpdateUIEventHandler(C_SelectionToolPanel::OnUpdateSelectionFilter));
	m_ToolBarItemIdMap.Insert(Ut::UtMap<S32, S32>::DataType(pTool->GetId(), 0x00000004));

	pComponentSelectFilter->Realize();

	pComponentSelectFilter->Show(false);


	pSizer->Add(m_pMainToolBar, 0, wxEXPAND|wxLEFT|wxRIGHT, 1);
	pSizer->Add(pObjectSelectFilter, 0, wxEXPAND|wxLEFT|wxRIGHT, 1);
	pSizer->Add(pComponentSelectFilter, 0, wxEXPAND|wxLEFT|wxRIGHT, 1);
	m_pToolBarSizerMap.Insert(Ut::UtMap<S32, wxToolBar*>::DataType(objectModeId, pObjectSelectFilter));
	m_pToolBarSizerMap.Insert(Ut::UtMap<S32, wxToolBar*>::DataType(componentModeId, pComponentSelectFilter));

	this->SetSizerAndFit(pSizer);
	this->Layout();

	//this probably should be in C_SelectManipulator
	Btl::String registryPath = Util::C_String(kRegKeyPath) + "\\preferences";
	SelectionMask selectionMask;
	selectionMask.mask = Registry::Read(registryPath.Chars(), "selection", 0xffffffff);
	C_SelectManipulator::SetSelectionMask(selectionMask);
}

C_SelectionToolPanel::~C_SelectionToolPanel()
{
	//this probably should be in C_SelectManipulator
	Btl::String registryPath = Util::C_String(kRegKeyPath) + "\\preferences";
	SelectionMask selectionMask = C_SelectManipulator::GetSelectionMask();
	Registry::Write(registryPath.Chars(), "selection", selectionMask.mask);
}

void C_SelectionToolPanel::OnSelectionMode(wxCommandEvent& e)
{
	Ut::UtMap<S32, S32>::Iterator idIt = m_ToolBarItemIdMap.Find(e.GetId());
	if (idIt == m_ToolBarItemIdMap.End())
		return e.Skip();

	C_SelectManipulator::SetSelectionMode((E_SelectionMode)idIt->GetValue());

	Ut::UtMap<S32, wxToolBar*>::Iterator sizerIt = m_pToolBarSizerMap.Begin();
	while (sizerIt != m_pToolBarSizerMap.End())
	{
		S32 key = sizerIt->GetKey();
		wxToolBar* value = sizerIt->GetValue();
		S32 eId = e.GetId();

		GetSizer()->Show(sizerIt->GetValue(), sizerIt->GetKey()==e.GetId());
		sizerIt++;
	}

	this->SetSizerAndFit(GetSizer());
	this->Layout();

	EditorLib::C_Core::Instance().DirtyViews();
}

void C_SelectionToolPanel::OnSelectionFilter(wxCommandEvent& e)
{
	Ut::UtMap<S32, S32>::Iterator it = m_ToolBarItemIdMap.Find(e.GetId());
	if (it == m_ToolBarItemIdMap.End())
		return e.Skip();

	SelectionMask selectionMask = C_SelectManipulator::GetSelectionMask();
	if (e.IsChecked())
		selectionMask.mask |= it->GetValue();
	else
		selectionMask.mask &= ~it->GetValue();

	C_SelectManipulator::SetSelectionMask(selectionMask);
}

void C_SelectionToolPanel::OnUpdateSelectionMode(wxUpdateUIEvent& e)
{
	Ut::UtMap<S32, S32>::Iterator it = m_ToolBarItemIdMap.Find(e.GetId());
	if (it == m_ToolBarItemIdMap.End())
		return e.Skip();

	e.Check(it->GetValue() == (S32) C_SelectManipulator::GetSelectionMode());
}

void C_SelectionToolPanel::OnUpdateSelectionFilter(wxUpdateUIEvent& e)
{
	Ut::UtMap<S32, S32>::Iterator it = m_ToolBarItemIdMap.Find(e.GetId());
	if (it == m_ToolBarItemIdMap.End())
		return e.Skip();

	e.Check((it->GetValue() & C_SelectManipulator::GetSelectionMask().mask) > 0);
}

wxSize C_SelectionToolPanel::DoGetBestSize()
{
	return wxSize(128, 20);
}