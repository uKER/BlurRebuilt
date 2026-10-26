#include "stdafx.h"

#include "wx/propgrid/propgrid.h"
#include "wx/propgrid/propdev.h"
#include "wx/propgrid/advprops.h"
#include "wx/propgrid/manager.h"

#include <EditorApp/properties/CameraProperties.h>
#include <EditorApp/properties/PropertyTypes.h>
#include <EditorApp/editor/RenderViewport.h>

using namespace Editor;

C_CameraProperties::C_CameraProperties(wxPropertyGridManager *pgMgr, C_RenderViewport *pViewport)
: m_pPropertyGridMgr(pgMgr)
, m_pViewport(pViewport)
{
	m_pPropertyGridMgr->Freeze();

	const TCHAR* CameraTypes[] =
	{
		_T("Perspective"),
		_T("Orthographic"),
		NULL
	};

	const long CameraTypeIndices[] =
	{
		0,
		1
	};

	m_pPropertyGridMgr->AddPage(_T("Camera"));
	m_pPropertyGridMgr->SelectPage(_T("Camera"));

	if (m_pViewport)
	{
		const AftkRender::C_Camera& cam = m_pViewport->GetCamera();

		m_pPropertyGridMgr->Append(new wxPropertyCategory(_T("Transform"), wxPG_LABEL));
		m_pPositionProp = m_pPropertyGridMgr->Append(new PGHVec3Property(_T("Position"), wxPG_LABEL, cam.GetPosition()));
		m_pFocusProp = m_pPropertyGridMgr->Append(new PGHVec3Property(_T("Focus"), wxPG_LABEL, cam.GetFocusPosition()));
		m_pUpProp = m_pPropertyGridMgr->Append(new PGHVec3Property(_T("Up"), wxPG_LABEL, cam.GetUpVector()));

		m_pPropertyGridMgr->Append(new wxPropertyCategory(_T("Attributes"), wxPG_LABEL));
		m_pNearClipProp = m_pPropertyGridMgr->Append(new wxFloatProperty(_T("Near Clipping Distance"), wxPG_LABEL, cam.GetNearClipDistance()));
		m_pFarClipProp = m_pPropertyGridMgr->Append(new wxFloatProperty(_T("Far Clipping Distance"), wxPG_LABEL, cam.GetFarClipDistance()));

		m_pFieldOfViewProp = m_pPropertyGridMgr->Append(new wxFloatProperty(_T("Field of View"), wxPG_LABEL, cam.GetFieldOfView()));
		m_pViewExtentProp = m_pPropertyGridMgr->Append(new wxFloatProperty(_T("View Extent"), wxPG_LABEL, cam.GetHeightExtent()));

		if (cam.GetType() == AftkRender::C_Camera::E_CameraType_Perspective)
		{
			m_pFieldOfViewProp->Hide(false);
			m_pViewExtentProp->Hide(true);
		}
		else
		{
			m_pFieldOfViewProp->Hide(true);
			m_pViewExtentProp->Hide(false);
		}

		m_pCameraTypeProp = m_pPropertyGridMgr->Append(new wxEnumProperty(_T("Type"), wxPG_LABEL, CameraTypes, CameraTypeIndices, cam.GetType()));
		m_pPropertyGridMgr->EnableProperty(m_pCameraTypeProp, false);
	}
	else
	{
		m_pPropertyGridMgr->Append(new wxPropertyCategory(_T("Transform"), wxPG_LABEL));
		m_pPositionProp = m_pPropertyGridMgr->Append(new PGHVec3Property(_T("Position"), wxPG_LABEL));
		m_pFocusProp = m_pPropertyGridMgr->Append(new PGHVec3Property(_T("Focus"), wxPG_LABEL));
		m_pUpProp = m_pPropertyGridMgr->Append(new PGHVec3Property(_T("Up"), wxPG_LABEL));

		m_pPropertyGridMgr->Append(new wxPropertyCategory(_T("Attributes"), wxPG_LABEL));
		m_pNearClipProp = m_pPropertyGridMgr->Append(new wxFloatProperty(_T("Near Clipping Distance"), wxPG_LABEL, 0.f));
		m_pFarClipProp = m_pPropertyGridMgr->Append(new wxFloatProperty(_T("Far Clipping Distance"), wxPG_LABEL, 0.f));

		m_pFieldOfViewProp = m_pPropertyGridMgr->Append(new wxFloatProperty(_T("Field of View"), wxPG_LABEL, 0.f));

		m_pViewExtentProp = m_pPropertyGridMgr->Append(new wxFloatProperty(_T("View Extent"), wxPG_LABEL, 0.f));
		m_pViewExtentProp->Hide(true);

		m_pCameraTypeProp = m_pPropertyGridMgr->Append(new wxEnumProperty(_T("Type"), wxPG_LABEL, CameraTypes, CameraTypeIndices, AftkRender::C_Camera::E_CameraType_Perspective));
		m_pPropertyGridMgr->EnableProperty(m_pCameraTypeProp, false);
	}

	m_pPropertyGridMgr->Thaw();
}

C_CameraProperties::~C_CameraProperties()
{
	m_pPropertyGridMgr->RemovePage(m_pPropertyGridMgr->GetPageByName(_T("Camera")));
}

void C_CameraProperties::OnPropertyChange(wxPropertyGridEvent& e)
{
	if (m_pViewport)
	{
		wxVariant value = e.GetPropertyValue();
		if (value.IsNull())
			return;

		const wxPGProperty *pProperty = e.GetProperty();
		CameraChangeEvent evt;
		if (pProperty == m_pPositionProp)
		{
			evt.SetEventType(CameraChangeEvent::E_CameraChange_Position);
			evt.SetValue(PGHVec3FromVariant(value));
			wxPostEvent(m_pViewport, evt);
		}
		else if (pProperty == m_pFocusProp)
		{
			evt.SetEventType(CameraChangeEvent::E_CameraChange_Focus);
			evt.SetValue(PGHVec3FromVariant(value));
			wxPostEvent(m_pViewport, evt);
		}
		else if (pProperty == m_pUpProp)
		{
			evt.SetEventType(CameraChangeEvent::E_CameraChange_UpVector);
			evt.SetValue(PGHVec3FromVariant(value));
			wxPostEvent(m_pViewport, evt);
		}
		else if (pProperty == m_pFieldOfViewProp)
		{
			evt.SetEventType(CameraChangeEvent::E_CameraChange_FieldOfView);
			evt.SetValue((F32)e.GetPropertyValueAsDouble());
			wxPostEvent(m_pViewport, evt);
		}
		else if (pProperty == m_pViewExtentProp)
		{
			evt.SetEventType(CameraChangeEvent::E_CameraChange_ViewExtent);
			evt.SetValue((F32)e.GetPropertyValueAsDouble());
			wxPostEvent(m_pViewport, evt);
		}
		else if (pProperty == m_pNearClipProp)
		{
			evt.SetEventType(CameraChangeEvent::E_CameraChange_NearClip);
			evt.SetValue((F32)e.GetPropertyValueAsDouble());
			wxPostEvent(m_pViewport, evt);
		}
		else if (pProperty == m_pFarClipProp)
		{
			evt.SetEventType(CameraChangeEvent::E_CameraChange_FarClip);
			evt.SetValue((F32)e.GetPropertyValueAsDouble());
			wxPostEvent(m_pViewport, evt);
		}
		else if (pProperty == m_pCameraTypeProp)
		{
			m_pPropertyGridMgr->Freeze();

			if (((AftkRender::C_Camera::E_CameraType) e.GetPropertyValueAsInt()) == AftkRender::C_Camera::E_CameraType_Perspective)
			{
				m_pFieldOfViewProp->Hide(false);
				m_pViewExtentProp->Hide(true);
			}
			else
			{
				m_pFieldOfViewProp->Hide(true);
				m_pViewExtentProp->Hide(false);
			}
			
			m_pPropertyGridMgr->Thaw();

			evt.SetEventType(CameraChangeEvent::E_CameraChange_Type);
			evt.SetValue((AftkRender::C_Camera::E_CameraType) e.GetPropertyValueAsInt());
			
			//process this event immediately
			m_pViewport->ProcessEvent(evt);
		}
	}
}

void C_CameraProperties::OnCameraChange(CameraChangeEvent &e)
{
	m_pPropertyGridMgr->ClearSelection();

	switch (e.GetEventType())
	{
	case CameraChangeEvent::E_CameraChange_Position:
		m_pPositionProp->SetValue(PGHVec3ToVariant(e.GetVec3Value()));
		break;
	case CameraChangeEvent::E_CameraChange_Focus:
		m_pFocusProp->SetValue(PGHVec3ToVariant(e.GetVec3Value()));
		break;
	case CameraChangeEvent::E_CameraChange_UpVector:
		m_pUpProp->SetValue(PGHVec3ToVariant(e.GetVec3Value()));
		break;
	case CameraChangeEvent::E_CameraChange_NearClip:
		m_pNearClipProp->SetValue(e.GetFloatValue());
		break;
	case CameraChangeEvent::E_CameraChange_FarClip:
		m_pFarClipProp->SetValue(e.GetFloatValue());
		break;
	case CameraChangeEvent::E_CameraChange_FieldOfView:
		m_pFieldOfViewProp->SetValue(e.GetFloatValue());
		break;
	case CameraChangeEvent::E_CameraChange_ViewExtent:
		m_pViewExtentProp->SetValue(e.GetFloatValue());
		break;
	case CameraChangeEvent::E_CameraChange_Type:
	{
		m_pCameraTypeProp->SetValue(e.GetEnumValue());

		if (e.GetEnumValue() == AftkRender::C_Camera::E_CameraType_Perspective)
		{
			m_pFieldOfViewProp->Hide(false);
			m_pViewExtentProp->Hide(true);
		}
		else
		{
			m_pFieldOfViewProp->Hide(true);
			m_pViewExtentProp->Hide(false);
		}
	}
		break;
	};

	m_pPropertyGridMgr->Refresh();
}

void C_CameraProperties::AttachViewport(C_RenderViewport *pViewport)
{
	m_pViewport = pViewport;
	m_pPropertyGridMgr->ClearSelection();

	if (m_pViewport)
	{
		const AftkRender::C_Camera& cam = m_pViewport->GetCamera();

		m_pPositionProp->SetValue(PGHVec3ToVariant(cam.GetPosition()));
		m_pFocusProp->SetValue(PGHVec3ToVariant(cam.GetFocusPosition()));
		m_pUpProp->SetValue(PGHVec3ToVariant(cam.GetUpVector()));
		m_pNearClipProp->SetValue(cam.GetNearClipDistance());
		m_pFarClipProp->SetValue(cam.GetFarClipDistance());
		m_pFieldOfViewProp->SetValue(cam.GetFieldOfView());
		m_pViewExtentProp->SetValue(cam.GetHeightExtent());
		m_pCameraTypeProp->SetValue(cam.GetType());

		if (cam.GetType()==AftkRender::C_Camera::E_CameraType_Perspective)
		{
			m_pFieldOfViewProp->Hide(false);
			m_pViewExtentProp->Hide(true);
		}
		else
		{
			m_pFieldOfViewProp->Hide(true);
			m_pViewExtentProp->Hide(false);
		}
	}
}

C_RenderViewport* C_CameraProperties::GetAttachedViewport()
{
	return m_pViewport;
}