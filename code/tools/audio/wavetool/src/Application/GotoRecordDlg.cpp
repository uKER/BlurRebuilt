#include "Core/StdAfx.h"
#include "SorceryApp.h"
#include "XtWin/Core.h"
#include "Application/GotoRecordDlg.h"
#include "Resource/Resource.h"

GotoRecordDlg::GotoRecordDlg(HWND parentWindow) :
	_parentWindow(parentWindow),
	_recordId(0),
	_altRecordId(0)
{
}

GotoRecordDlg::~GotoRecordDlg()
{
}

bool GotoRecordDlg::Modal()
{
	if (DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_GOTO_RECORD), _parentWindow, GotoRecordDlg::WndProc, (LPARAM)(INT_PTR)this) != 1)
		return false;

	return true;
}

INT_PTR CALLBACK GotoRecordDlg::WndProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GotoRecordDlg* grd = (GotoRecordDlg*)(INT_PTR)GetWindowLong(dlg, DWL_USER);

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			SetWindowLong(dlg, DWL_USER, (LONG)(INT_PTR)lParam);
			grd = (GotoRecordDlg*)lParam;

			//SetDlgItemInt(dlg, IDC_RECORD_ID, grd->_recordId, TRUE);
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

				case IDC_RECORD_ID:
					if (HIWORD(wParam) == EN_CHANGE)
					{
						Sz8 sz = SzGetWindowText(GetDlgItem(dlg, IDC_RECORD_ID));

						if ((sz.Length() >= 2) && (sz[0] == '0') && (sz[1] == 'x'))
							grd->_recordId = (XtId)_strtoi64(sz.Text() + 2, 0, 16);
						else
							grd->_recordId = (XtId)_strtoi64(sz, 0, 10);

						grd->_altRecordId = (XtId)_strtoi64(sz.Text(), 0, 16);

						return TRUE;
					}
				break;
			}
		break;
	}

	return FALSE;
}