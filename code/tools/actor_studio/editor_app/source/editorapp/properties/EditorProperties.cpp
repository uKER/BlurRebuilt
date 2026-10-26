#include "stdafx.h"

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/manager.h>

#include <EditorApp/Components/PropertyDialog.h>
#include <EditorApp/Properties/EditorProperties.h>
#include <EditorApp/Tasks/UITasks.h>

namespace Editor
{

C_EditorPropertyGroup::C_EditorPropertyGroup()
: m_pPgMgr(0)
, m_pAutosaveTimerProperty(0)
{
}

C_EditorPropertyGroup::~C_EditorPropertyGroup()
{
}

void C_EditorPropertyGroup::Initialise(wxPropertyGridManager* pPgMgr)
{
	m_pPgMgr = pPgMgr;
	assert(m_pPgMgr);
	
	wxPropertyGridPage *pPage = m_pPgMgr->GetPage("General");
	if (!pPage)
	{
		S32 pageIndex = m_pPgMgr->AddPage("General");
		pPage = m_pPgMgr->GetPage(pageIndex);
	}
	assert(pPage);

	pPage->AppendCategory("Autosave", wxPG_LABEL);
	m_pAutosaveTimerProperty = new wxUIntProperty("Interval", wxPG_LABEL, 
		Tasks::GetAutosaveInterval());
	m_pAutosaveTimerProperty->SetHelpString("Autosave interval in minutes");
	pPage->Append(m_pAutosaveTimerProperty);

	pPage->AppendCategory("Notification", wxPG_LABEL);
	m_pUIPromptProperty = new wxBoolProperty("Show Prompts", wxPG_LABEL,
		Tasks::GetShowUIPromptOption());
	m_pUIPromptProperty->SetAttribute(wxPG_BOOL_USE_CHECKBOX, 1);
	m_pUIPromptProperty->SetHelpString("Show dialog prompts when tasks are performed");
	pPage->Append(m_pUIPromptProperty);

	m_pDisableClearUndoProperty = new wxBoolProperty("Disable Clear Undo Warning", wxPG_LABEL,
		Tasks::GetDisableClearUndoWarning());
	m_pDisableClearUndoProperty->SetAttribute(wxPG_BOOL_USE_CHECKBOX, 1);
	m_pDisableClearUndoProperty->SetHelpString("Disable warning when undo stack is cleared");
	pPage->Append(m_pDisableClearUndoProperty);


	pPage = m_pPgMgr->GetPage("Viewport");
	if (!pPage)
	{
		S32 pageIndex = m_pPgMgr->AddPage("Viewport");
		pPage = m_pPgMgr->GetPage(pageIndex);
	}
	assert(pPage);

	pPage->AppendCategory("Grid Rendering", wxPG_LABEL);
	m_pGridSizeProperty = new wxFloatProperty("Size", wxPG_LABEL, Tasks::GetGridSize());
	m_pGridSizeProperty->SetHelpString("Width/Height in the perspective viewport");
	pPage->Append(m_pGridSizeProperty);

	m_pGridSpacingProperty = new wxFloatProperty("Spacing", wxPG_LABEL, Tasks::GetGridSpacing());
	m_pGridSpacingProperty->SetHelpString("Grid Line Spacing");
	pPage->Append(m_pGridSpacingProperty);
}

void C_EditorPropertyGroup::OnPropertyChanged(wxPropertyGridEvent& e)
{
}

void C_EditorPropertyGroup::Apply()
{
	U32 interval = m_pPgMgr->GetPropertyValueAsInt(m_pAutosaveTimerProperty);
	Tasks::SetAutosaveInterval(interval);

	F32 size = m_pPgMgr->GetPropertyValueAsDouble(m_pGridSizeProperty);
	Tasks::SetGridSize(size);

	F32 spacing = m_pPgMgr->GetPropertyValueAsDouble(m_pGridSpacingProperty);
	Tasks::SetGridSpacing(spacing);

	bool showUIPrompts = m_pPgMgr->GetPropertyValueAsBool(m_pUIPromptProperty);
	Tasks::SetShowUIPromptOption(showUIPrompts);
}

void C_EditorPropertyGroup::Revert()
{
}



}