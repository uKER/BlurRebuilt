#ifndef EDITOR_VIEWPORT_PANEL_H
#define EDITOR_VIEWPORT_PANEL_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_ViewportMenuBar;
	class C_RenderViewport;


	class C_ViewportPanel : public wxPanel
	{
	public:
		C_ViewportPanel(
			wxWindow *parent, 
			wxWindowID id = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxSize(200,400),
			long style = wxTAB_TRAVERSAL|wxSIMPLE_BORDER);

		~C_ViewportPanel();

		void SetViewport(C_RenderViewport *pViewport);
		C_RenderViewport* GetViewport();

		void OnPaint(wxPaintEvent &e);
		void OnSize(wxSizeEvent& e);

	private:
		C_ViewportMenuBar	*m_pMenuBar;
		C_RenderViewport	*m_pViewport;
		wxColour			m_BorderColour;
	};


}

#endif