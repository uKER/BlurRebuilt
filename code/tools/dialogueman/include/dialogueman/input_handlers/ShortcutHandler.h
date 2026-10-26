#ifndef DMAN_SHORTCUT_HANDLER_H
#define DMAN_SHORTCUT_HANDLER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

	class C_ShortcutHandler : public wxEvtHandler
	{
	public:
		C_ShortcutHandler();
		~C_ShortcutHandler();

		void OnKeyDown(wxKeyEvent& e);
		void OnKeyUp(wxKeyEvent& e);
		void OnFocus(wxFocusEvent& e);

	private:
		bool m_acceleratorDisable;
	};
}

#endif //DMAN_SHORTCUT_HANDLER_H
