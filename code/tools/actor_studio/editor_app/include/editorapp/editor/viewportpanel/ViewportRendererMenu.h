#ifndef EDITOR_VIEWPORT_RENDERER_MENU_H
#define EDITOR_VIEWPORT_RENDERER_MENU_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_ViewportMenuBar;

	class C_ViewportRendererMenu
	{
	public:
		C_ViewportRendererMenu(C_ViewportMenuBar *pMenuBar);
		~C_ViewportRendererMenu();
		bool InitialiseUI();

	private:
		void OnSetGridState(wxCommandEvent &e);
		void OnUpdateGridState(wxUpdateUIEvent &e);

		void OnSetLineMode(wxCommandEvent &e);
		void OnUpdateLineMode(wxUpdateUIEvent &e);

		void OnSetLabelMode(wxCommandEvent &e);
		void OnUpdateLabelMode(wxUpdateUIEvent &e);

		Ut::Pair<S32, U8>	m_LabelModeId[4];

		C_ViewportMenuBar *m_pMenuBar;
	};

}

#endif