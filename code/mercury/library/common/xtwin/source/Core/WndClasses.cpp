#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/WndCore.h"
#include "XtWin/WndClasses.h"

WNDPROC gXtWinComboBoxDefWndProc;
WNDPROC gXtWinTreeViewDefWndProc;
WNDPROC gXtWinTrackBarDefWndProc;

LRESULT WINAPI XtWinComboBoxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_MOUSEWHEEL:
		{
			if (SendMessage(hwnd, CB_GETDROPPEDSTATE, 0, 0) == FALSE)
			{
				HWND parentHwnd = GetParent(hwnd);

				if (parentHwnd)
					SendMessage(parentHwnd, msg, wParam, lParam);

				return 0;
			}
		}
		break;
	}

	return CallWindowProc(gXtWinComboBoxDefWndProc, hwnd, msg, wParam, lParam);
}

LRESULT WINAPI XtWinTreeViewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_KEYDOWN:
		{
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				switch(wParam)
				{
					case VK_UP:
					{
						HTREEITEM hdl = TreeView_GetSelection(hwnd);

						if (hdl == 0)
							hdl = TreeView_GetRoot(hwnd);
						else
							hdl = TreeView_GetPrevVisible(hwnd, hdl);

						if (hdl != 0)
							TreeView_SelectItem(hwnd, hdl);
					}
					return 1;

					case VK_DOWN:
					{
						HTREEITEM hdl = TreeView_GetSelection(hwnd);

						if (hdl == 0)
							hdl = TreeView_GetRoot(hwnd);
						else
							hdl = TreeView_GetNextVisible(hwnd, hdl);
		
						if (hdl != 0)
							TreeView_SelectItem(hwnd, hdl);
					}
					return 1;
				}
			}
		}
		break;
	}

	return CallWindowProc(gXtWinTreeViewDefWndProc, hwnd, msg, wParam, lParam);
}

LRESULT WINAPI XtWinTrackBarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_LBUTTONDOWN:
		{
			WinRect rc(hwnd);

			int cx = GET_X_LPARAM(lParam);

			float ratio = Clamp((float)(cx - 8) / (float)(rc.Width() - 16), 0.0f, 1.0f);

			int min = (int)SendMessage(hwnd, TBM_GETRANGEMIN, 0, 0);
			int max = (int)SendMessage(hwnd, TBM_GETRANGEMAX, 0, 0);

			SendMessage(hwnd, TBM_SETPOS, TRUE, Lerp(min, max, ratio));

			WinRect rcThumb;
			SendMessage(hwnd, TBM_GETTHUMBRECT, 0, (LPARAM)&rcThumb);

			int x = (rcThumb.left + rcThumb.right) / 2;
			int y = (rcThumb.top + rcThumb.bottom) / 2;

			CallWindowProc(gXtWinTrackBarDefWndProc, hwnd, msg, wParam, MAKELONG(x, y));
		}
		return 0;

		case WM_MOUSEWHEEL:
		{
			HWND parentHwnd = GetParent(hwnd);

			if (parentHwnd)
				SendMessage(parentHwnd, msg, wParam, lParam);
		}
		return 0;
	}

	return CallWindowProc(gXtWinTrackBarDefWndProc, hwnd, msg, wParam, lParam);
}

void RegisterXtWinClasses()
{
	{
		WNDCLASSEX wcx = { 0 };

		wcx.cbSize = sizeof(wcx);

		GetClassInfoEx(0, WC_COMBOBOX, &wcx);

		gXtWinComboBoxDefWndProc = wcx.lpfnWndProc;

		wcx.lpszClassName	= _T("XTWINCOMBOBOX");
		wcx.lpfnWndProc		= XtWinComboBoxWndProc;

		RegisterClassEx(&wcx);
	}
	{
		WNDCLASSEX wcx = { 0 };

		wcx.cbSize = sizeof(wcx);

		GetClassInfoEx(0, WC_TREEVIEW, &wcx);

		gXtWinTreeViewDefWndProc = wcx.lpfnWndProc;

		wcx.lpszClassName	= _T("XTWINTREEVIEW");
		wcx.lpfnWndProc		= XtWinTreeViewWndProc;

		RegisterClassEx(&wcx);
	}
	{
		WNDCLASSEX wcx = { 0 };

		wcx.cbSize = sizeof(wcx);

		GetClassInfoEx(0, TRACKBAR_CLASS, &wcx);

		gXtWinTrackBarDefWndProc = wcx.lpfnWndProc;

		wcx.lpszClassName	= _T("XTWINTRACKBAR");
		wcx.lpfnWndProc		= XtWinTrackBarWndProc;
		wcx.hbrBackground	= 0;

		RegisterClassEx(&wcx);
	}
}