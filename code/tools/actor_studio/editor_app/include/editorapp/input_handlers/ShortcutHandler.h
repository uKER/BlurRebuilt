#ifndef EDITOR_SHORTCUT_HANDLER_H
#define EDITOR_SHORTCUT_HANDLER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_RenderViewport;

	class C_ShortcutHandler : public wxEvtHandler
	{
	public:
		C_ShortcutHandler();
		~C_ShortcutHandler();

		bool ProcessKey(WXMSG *msg);

		void OnViewportFocus(wxFocusEvent& e);
		void OnIdle(wxIdleEvent& e);

		const C_RenderViewport* GetLastFocusedViewport() const;
		C_RenderViewport* GetLastFocusedViewport();

	private:
		C_RenderViewport	*m_pLastFocusedViewport;
		wxAcceleratorTable	m_acceleratorTable;
		bool				m_acceleratorDisable;
		bool				m_snapToggle[3];
	};
}

#endif
