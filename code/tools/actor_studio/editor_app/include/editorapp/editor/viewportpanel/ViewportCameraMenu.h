#ifndef EDITOR_VIEWPORT_CAMERA_MENU_H
#define EDITOR_VIEWPORT_CAMERA_MENU_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_ViewportMenuBar;

	class C_ViewportCameraMenu
	{
	public:
		C_ViewportCameraMenu(C_ViewportMenuBar *pMenuBar);
		~C_ViewportCameraMenu();
		bool InitialiseUI();

	private:
		void OnSetCameraMode(wxCommandEvent &e);
		void OnUpdateCameraMode(wxUpdateUIEvent &e);
		void OnUpdateCameraSync(wxUpdateUIEvent &e);

		C_ViewportMenuBar *m_pMenuBar;

		S32 m_perspectiveId;
		S32 m_frontId;
		S32 m_sideId;
		S32 m_topId;
		S32 m_attachId;
		S32 m_syncId;
	};

}

#endif