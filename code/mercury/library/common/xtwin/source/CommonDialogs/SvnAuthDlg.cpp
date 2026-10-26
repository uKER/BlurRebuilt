#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/WndCore.h"
#include "XtWin/SvnAuthDlg.h"
#include "XtWin/Resource.h"

SvnAuthDlg::SvnAuthDlg(HWND parentWindow) :
	_parentWindow(parentWindow),
	_remember(false),
	_canRemember(false)
{
	_authNotice = "Please enter your login details.";
}

SvnAuthDlg::~SvnAuthDlg()
{
}

bool SvnAuthDlg::Modal()
{
	if (DialogBoxParam(GetXtWinIconDll(), MAKEINTRESOURCE(IDD_XTWIN_SVN_LOGIN), _parentWindow, SvnAuthDlg::WndProc, (LPARAM)(INT_PTR)this) != 1)
		return false;

	return true;
}

INT_PTR CALLBACK SvnAuthDlg::WndProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SvnAuthDlg* sad = (SvnAuthDlg*)(INT_PTR)GetWindowLong(dlg, DWL_USER);

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			SetWindowLong(dlg, DWL_USER, (LONG)(INT_PTR)lParam);
			sad = (SvnAuthDlg*)lParam;

			SetDlgItemText(dlg, IDC_XTWIN_AUTH_NOTICE, sad->_authNotice); 
			SetDlgItemText(dlg, IDC_XTWIN_USERNAME, sad->_username); 
			SetDlgItemText(dlg, IDC_XTWIN_PASSWORD, sad->_password); 

			if (sad->_canRemember)
			{
				if (sad->_remember)
					CheckDlgButton(dlg, IDC_XTWIN_REMEMBER, BST_CHECKED);
			}
			else
			{
				EnableWindow(GetDlgItem(dlg, IDC_XTWIN_REMEMBER), FALSE);
			}
		}
		return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						EndDialog(dlg, 1);
						return TRUE;
					}
				break;

				case IDCANCEL:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						EndDialog(dlg, 0);
						return TRUE;
					}
				break;

				case IDC_XTWIN_USERNAME:
					if (HIWORD(wParam) == EN_CHANGE)
					{
						sad->_username = SzGetWindowText(GetDlgItem(dlg, IDC_XTWIN_USERNAME));

						return TRUE;
					}
				break;

				case IDC_XTWIN_PASSWORD:
					if (HIWORD(wParam) == EN_CHANGE)
					{
						sad->_password = SzGetWindowText(GetDlgItem(dlg, IDC_XTWIN_PASSWORD));

						return TRUE;
					}
				break;

				case IDC_XTWIN_REMEMBER:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						sad->_remember = IsDlgButtonChecked(dlg, IDC_XTWIN_REMEMBER) != FALSE;

						return TRUE;
					}
				break;
			}
		break;
	}

	return FALSE;
}