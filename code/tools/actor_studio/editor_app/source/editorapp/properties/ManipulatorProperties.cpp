#include "stdafx.h"

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/propdev.h>
#include <wx/propgrid/advprops.h>
#include <wx/propgrid/manager.h>

#include <EditorApp/properties/ManipulatorProperties.h>
#include <EditorApp/editor/manipulators/contexts/IManipulatorContext.h>
#include <EditorApp/editor/manipulators/Manipulator.h>
#include <EditorApp/editor/manipulators/ActorCreateManipulator.h>
#include <EditorApp/editor/manipulators/SelectManipulator.h>
#include <EditorApp/editor/manipulators/MoveManipulator.h>
#include <EditorApp/editor/manipulators/RotateManipulator.h>
#include <EditorApp/editor/manipulators/ScaleManipulator.h>

namespace Editor
{
	extern Debug::Logger& g_Log;
}

using namespace Editor;


I_ManipulatorProperties::I_ManipulatorProperties(wxPropertyGridManager *pPgMgr)
: m_pPropertyGridMgr(pPgMgr)
{
}

I_ManipulatorProperties::~I_ManipulatorProperties()
{
}

void I_ManipulatorProperties::Refresh()
{
}

void I_ManipulatorProperties::OnPropertyChange(wxPropertyGridEvent& e)
{
	LOGERROR(g_Log, "Unhandled property change event");
}

C_CreateManipulatorProperties::C_CreateManipulatorProperties(wxPropertyGridManager *pPgMgr, C_ActorCreateManipulator *pTool)
: I_ManipulatorProperties(pPgMgr)
, m_pTool(pTool)
{
	m_pPropertyGridMgr->Append(new wxPropertyCategory(_T("Creation Settings")));

	const TCHAR* CreationMode[] =
	{
		_T("Single"),
		_T("Multiple"),
		NULL
	};

	const long CreationModeIndices[] =
	{
		0,
		1,
	};

	const TCHAR* OrientationMode[] =
	{
		_T("Vertical"),
		_T("Mesh"),
		NULL
	};

	const long OrientationModeIndices[] =
	{
		0,
		1,
	};

	m_pCreationModeProp = m_pPropertyGridMgr->Append(new wxEnumProperty(_T("Creation Mode"), wxPG_LABEL, CreationMode, CreationModeIndices, pTool->GetCreationMode()));
	m_pOrientationModeProp = m_pPropertyGridMgr->Append(new wxEnumProperty(_T("Orientation Mode"), wxPG_LABEL, OrientationMode, OrientationModeIndices, pTool->GetOrientationMode()));
}

C_CreateManipulatorProperties::~C_CreateManipulatorProperties()
{
}

void C_CreateManipulatorProperties::Refresh()
{
}

void C_CreateManipulatorProperties::OnPropertyChange(wxPropertyGridEvent &e)
{
	const wxPGProperty *pProperty = e.GetProperty();
	if (pProperty == m_pCreationModeProp)
		m_pTool->SetCreationMode((E_CreationMode)e.GetPropertyValueAsInt());

	if (pProperty == m_pOrientationModeProp)
		m_pTool->SetOrientationMode((E_OrientationMode)e.GetPropertyValueAsInt());
}

C_SelectManipulatorProperties::C_SelectManipulatorProperties(wxPropertyGridManager *pPgMgr, C_SelectManipulator *pTool)
: I_ManipulatorProperties(pPgMgr)
, m_pTool(pTool)
{
	m_pPropertyGridMgr->Append(new wxPropertyCategory(_T("Selection Settings")));

	const TCHAR* SelectionMode[] =
	{
		_T("Object"),
		_T("Component"),
		NULL
	};

	const long SelectionModeIndices[] =
	{
		0,
		1,
	};

	m_pSelectionModeProp = m_pPropertyGridMgr->Append(new wxEnumProperty(_T("Mode"), wxPG_LABEL, SelectionMode, SelectionModeIndices, pTool->GetSelectionMode()));
}

C_SelectManipulatorProperties::~C_SelectManipulatorProperties()
{
}

void C_SelectManipulatorProperties::Refresh()
{
	m_pSelectionModeProp->SetValue(m_pTool->GetSelectionMode());
}

void C_SelectManipulatorProperties::OnPropertyChange(wxPropertyGridEvent &e)
{
	const wxPGProperty *pProperty = e.GetProperty();
	if (pProperty == m_pSelectionModeProp)
		m_pTool->SetSelectionMode((E_SelectionMode)e.GetPropertyValueAsInt());
}


C_MoveManipulatorProperties::C_MoveManipulatorProperties(wxPropertyGridManager *pPgMgr, C_MoveManipulator *pTool)
: C_SelectManipulatorProperties(pPgMgr, pTool)
, m_pTool(pTool)
{
	m_pPropertyGridMgr->Append(new wxPropertyCategory(_T("Move Settings")));

	const TCHAR* CoordSpace[] =
	{
		_T("Local"),
		_T("World"),
		NULL
	};

	const long CoordSpaceIndices[] =
	{
		0,
		1,
	};

	m_pCoordinateSpaceProp = m_pPropertyGridMgr->Append(new wxEnumProperty(_T("Alignment"), wxPG_LABEL, CoordSpace, CoordSpaceIndices, pTool->GetCoordinateSpace()));
	m_pSnapMoveProp = m_pPropertyGridMgr->Append(new wxBoolProperty(_T("Snap Move"), wxPG_LABEL, false));
	m_pPropertyGridMgr->SetPropertyAttribute(m_pSnapMoveProp, wxPG_BOOL_USE_CHECKBOX, 1);


	m_pSnapMoveValueProp = m_pPropertyGridMgr->Append(new wxFloatProperty(_T("Snap Value"), wxPG_LABEL, 1.f));
	m_pPropertyGridMgr->DisableProperty(m_pSnapMoveValueProp);
}

C_MoveManipulatorProperties::~C_MoveManipulatorProperties()
{
}

void C_MoveManipulatorProperties::Refresh()
{
	C_SelectManipulatorProperties::Refresh();

	m_pCoordinateSpaceProp->SetValue(m_pTool->GetCoordinateSpace());
	m_pSnapMoveProp->SetValue(m_pTool->SnapEnabled());
	m_pSnapMoveValueProp->SetValue(m_pTool->SnapValue());

	if (m_pPropertyGridMgr->GetPropertyValueAsBool(m_pSnapMoveProp))
		m_pPropertyGridMgr->EnableProperty(m_pSnapMoveValueProp);
	else
		m_pPropertyGridMgr->DisableProperty(m_pSnapMoveValueProp);
}

void C_MoveManipulatorProperties::OnPropertyChange(wxPropertyGridEvent& e)
{
	C_SelectManipulatorProperties::OnPropertyChange(e);
	const wxPGProperty *pProperty = e.GetProperty();

	if (pProperty == m_pCoordinateSpaceProp)
		m_pTool->SetCoordinateSpace((E_ManipulatorSpace)e.GetPropertyValueAsInt());
	else if (pProperty == m_pSnapMoveProp)
		m_pTool->EnableSnapMove(e.GetPropertyValueAsBool()>0, m_pPropertyGridMgr->GetPropertyValueAsDouble(m_pSnapMoveValueProp));	
	else if (pProperty == m_pSnapMoveValueProp)
		m_pTool->EnableSnapMove(m_pPropertyGridMgr->GetPropertyValueAsBool(m_pSnapMoveProp), e.GetPropertyValueAsDouble());

	if (m_pPropertyGridMgr->GetPropertyValueAsBool(m_pSnapMoveProp))
		m_pPropertyGridMgr->EnableProperty(m_pSnapMoveValueProp);
	else
		m_pPropertyGridMgr->DisableProperty(m_pSnapMoveValueProp);
}



C_RotateManipulatorProperties::C_RotateManipulatorProperties(wxPropertyGridManager *pPgMgr, C_RotateManipulator *pTool)
: C_SelectManipulatorProperties(pPgMgr, pTool)
, m_pTool(pTool)
{
	m_pPropertyGridMgr->Append(new wxPropertyCategory(_T("Rotate Settings")));

	const TCHAR* CoordSpace[] =
	{
		_T("Local"),
		_T("World"),
		NULL
	};

	const long CoordSpaceIndices[] =
	{
		0,
		1,
	};

	m_pCoordinateSpaceProp = m_pPropertyGridMgr->Append(new wxEnumProperty(_T("Alignment"), wxPG_LABEL, CoordSpace, CoordSpaceIndices, pTool->GetCoordinateSpace()));
	m_pSnapRotateProp = m_pPropertyGridMgr->Append(new wxBoolProperty(_T("Snap Rotate"), wxPG_LABEL, false));
	m_pPropertyGridMgr->SetPropertyAttribute(m_pSnapRotateProp, wxPG_BOOL_USE_CHECKBOX, 1);


	m_pSnapRotateValueProp = m_pPropertyGridMgr->Append(new wxFloatProperty(_T("Snap Value"), wxPG_LABEL, 45.f));
	m_pPropertyGridMgr->DisableProperty(m_pSnapRotateValueProp);
}

C_RotateManipulatorProperties::~C_RotateManipulatorProperties()
{
}

void C_RotateManipulatorProperties::Refresh()
{
	C_SelectManipulatorProperties::Refresh();

	m_pCoordinateSpaceProp->SetValue(m_pTool->GetCoordinateSpace());
	m_pSnapRotateProp->SetValue(m_pTool->SnapEnabled());
	m_pSnapRotateValueProp->SetValue(m_pTool->SnapValue());

	if (m_pPropertyGridMgr->GetPropertyValueAsBool(m_pSnapRotateProp))
		m_pPropertyGridMgr->EnableProperty(m_pSnapRotateValueProp);
	else
		m_pPropertyGridMgr->DisableProperty(m_pSnapRotateValueProp);
}

void C_RotateManipulatorProperties::OnPropertyChange(wxPropertyGridEvent& e)
{
	C_SelectManipulatorProperties::OnPropertyChange(e);
	const wxPGProperty *pProperty = e.GetProperty();

	if (pProperty == m_pCoordinateSpaceProp)
		m_pTool->SetCoordinateSpace((E_ManipulatorSpace)e.GetPropertyValueAsInt());
	else if (pProperty == m_pSnapRotateProp)
		m_pTool->EnableSnapRotate(e.GetPropertyValueAsBool()>0, m_pPropertyGridMgr->GetPropertyValueAsDouble(m_pSnapRotateValueProp));	
	else if (pProperty == m_pSnapRotateValueProp)
		m_pTool->EnableSnapRotate(m_pPropertyGridMgr->GetPropertyValueAsBool(m_pSnapRotateProp), e.GetPropertyValueAsDouble());

	if (m_pPropertyGridMgr->GetPropertyValueAsBool(m_pSnapRotateProp))
		m_pPropertyGridMgr->EnableProperty(m_pSnapRotateValueProp);
	else
		m_pPropertyGridMgr->DisableProperty(m_pSnapRotateValueProp);
}



C_ScaleManipulatorProperties::C_ScaleManipulatorProperties(wxPropertyGridManager *pPgMgr, C_ScaleManipulator *pTool)
: C_SelectManipulatorProperties(pPgMgr, pTool)
, m_pTool(pTool)
{
	m_pPropertyGridMgr->Append(new wxPropertyCategory(_T("Scale Settings")));

	const TCHAR* CoordSpace[] =
	{
		_T("Local"),
		_T("World"),
		NULL
	};

	const long CoordSpaceIndices[] =
	{
		0,
		1,
	};

	m_pCoordinateSpaceProp = m_pPropertyGridMgr->Append(new wxEnumProperty(_T("Alignment"), wxPG_LABEL, CoordSpace, CoordSpaceIndices, pTool->GetCoordinateSpace()));
	
	//disable this for now...
	m_pPropertyGridMgr->DisableProperty(m_pCoordinateSpaceProp);
	
	m_pSnapScaleProp = m_pPropertyGridMgr->Append(new wxBoolProperty(_T("Snap Scale"), wxPG_LABEL, false));
	m_pPropertyGridMgr->SetPropertyAttribute(m_pSnapScaleProp, wxPG_BOOL_USE_CHECKBOX, 1);

	m_pSnapScaleValueProp = m_pPropertyGridMgr->Append(new wxFloatProperty(_T("Snap Value"), wxPG_LABEL, 1.f));
	m_pPropertyGridMgr->DisableProperty(m_pSnapScaleValueProp);
}

C_ScaleManipulatorProperties::~C_ScaleManipulatorProperties()
{
}

void C_ScaleManipulatorProperties::Refresh()
{
	C_SelectManipulatorProperties::Refresh();

	m_pCoordinateSpaceProp->SetValue(m_pTool->GetCoordinateSpace());
	m_pSnapScaleProp->SetValue(m_pTool->SnapEnabled());
	m_pSnapScaleValueProp->SetValue(m_pTool->SnapValue());

	if (m_pPropertyGridMgr->GetPropertyValueAsBool(m_pSnapScaleProp))
		m_pPropertyGridMgr->EnableProperty(m_pSnapScaleValueProp);
	else
		m_pPropertyGridMgr->DisableProperty(m_pSnapScaleValueProp);
}

void C_ScaleManipulatorProperties::OnPropertyChange(wxPropertyGridEvent& e)
{
	C_SelectManipulatorProperties::OnPropertyChange(e);
	const wxPGProperty *pProperty = e.GetProperty();

	if (pProperty == m_pCoordinateSpaceProp)
		m_pTool->SetCoordinateSpace((E_ManipulatorSpace)e.GetPropertyValueAsInt());
	else if (pProperty == m_pSnapScaleProp)
		m_pTool->EnableSnapScale(e.GetPropertyValueAsBool()>0, m_pPropertyGridMgr->GetPropertyValueAsDouble(m_pSnapScaleValueProp));
	else if (pProperty == m_pSnapScaleValueProp)
		m_pTool->EnableSnapScale(m_pPropertyGridMgr->GetPropertyValueAsBool(m_pSnapScaleProp), e.GetPropertyValueAsDouble());

	if (m_pPropertyGridMgr->GetPropertyValueAsBool(m_pSnapScaleProp))
		m_pPropertyGridMgr->EnableProperty(m_pSnapScaleValueProp);
	else
		m_pPropertyGridMgr->DisableProperty(m_pSnapScaleValueProp);
}


