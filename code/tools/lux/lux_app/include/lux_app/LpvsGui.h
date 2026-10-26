#ifndef LPVS_GUI_H
#define LPVS_GUI_H

//This class is specifically to allow the program to perform operations which alter the GUI in some way.
//For example outputting text to the window, or updating the position of the viewport camera.

#include "definitions.h"

namespace LPVS
{
	#define LPVSGUI LpvsGui::GetSingleton()
/*
	enum LpvsGuiMouseButtonEnum
	{
		LPVS_GUI_MOUSE_BUTTON_LEFT,
		LPVS_GUI_MOUSE_BUTTON_RIGHT,
		LPVS_GUI_MOUSE_BUTTON_MIDDLE,

		LPVS_GUI_MOUSE_BUTTON_COUNT
	};
*/
	class LpvsGui
	{
	public:
		LpvsGui();
		virtual ~LpvsGui();

		static LpvsGui &GetSingleton();

		bool Create(CWnd* pRenderWindowCWnd);
		void Destroy();

		void KeyboardEvent(UINT nChar, UINT nRepCnt, UINT nFlags);
		void MouseMoveEvent(UINT nFlags, CPoint point);
		void MouseButtonEvent(int buttonId, bool doubleClick, UINT nFlags, CPoint point);

		///Outputs a string to the log window.
		void PrintToLogWindow(const std::string& string);

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void Reset();

		static void Init();

		static LpvsGui*			m_pSingleton;

		CWnd*					m_pRenderWindowCWnd;


	#endif
	};
}

#endif
