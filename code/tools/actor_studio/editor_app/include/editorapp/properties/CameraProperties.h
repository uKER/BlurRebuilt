#ifndef EDITOR_CAMERA_PROPERTIES_H
#define EDITOR_CAMERA_PROPERTIES_H

#ifdef _MSC_VER
#pragma once
#endif

class wxPropertyGridManager;
class wxPropertyGridEvent;
class wxPGProperty;

class CameraChangeEvent;

namespace Editor
{
	class C_RenderViewport;

	class C_CameraProperties
	{
	public:
		C_CameraProperties(wxPropertyGridManager *pgMgr, C_RenderViewport *pViewport=0);
		~C_CameraProperties();

		void OnPropertyChange(wxPropertyGridEvent& e);
		void OnCameraChange(CameraChangeEvent& e);

		void AttachViewport(C_RenderViewport *pViewport);
		C_RenderViewport* GetAttachedViewport();

	private:

		C_RenderViewport	*m_pViewport;

		wxPropertyGridManager *m_pPropertyGridMgr;
		
		wxPGProperty *m_pPositionProp;
		wxPGProperty *m_pFocusProp;
		wxPGProperty *m_pUpProp;

		wxPGProperty *m_pNearClipProp;
		wxPGProperty *m_pFarClipProp;

		wxPGProperty *m_pFieldOfViewProp;
		wxPGProperty *m_pViewExtentProp;

		wxPGProperty *m_pCameraTypeProp;
	};
}

#endif