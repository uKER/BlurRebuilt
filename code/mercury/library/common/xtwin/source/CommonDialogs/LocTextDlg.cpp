#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/WndCore.h"
#include "XtWin/LocTextDlg.h"
#include "XtWin/Resource.h"

XtString EncodeUnicodeWindowText(HWND hwnd)
{
	return WcToMb(SzGetWindowTextW(hwnd));
}

void DecodeUnicodeWindowText(HWND hwnd, XtString string)
{
	SetWindowTextW(hwnd, MbToWc(string));
}

BOOL CALLBACK LocDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LocTextData* ldd = (LocTextData*)(INT_PTR)GetWindowLong(hwnd, DWL_USER);

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			ldd = (LocTextData*)lParam;

			EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);

			SetWindowLong(hwnd, DWL_USER, (LONG)lParam);

			SetDlgItemText(hwnd, IDC_XTWIN_NAME, ldd->name);
			DecodeUnicodeWindowText(GetDlgItem(hwnd, IDC_XTWIN_COMMENT), ldd->comment);

			if (ldd->currentItem < (int)ldd->items.count)
				DecodeUnicodeWindowText(GetDlgItem(hwnd, IDC_XTWIN_TEXT), ldd->items[ldd->currentItem].text);

			HWND combo = GetDlgItem(hwnd, IDC_XTWIN_LANGUAGE);

			for(XtUt::Array<LocTextItem>::Iterator it(&ldd->items); it; ++it)
				SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)(const char*)it->language);

			SendMessage(combo, CB_SETCURSEL, ldd->currentItem, 0);
		}
		break;

		case WM_COMMAND:
		{
			int id		= LOWORD(wParam);
			int code	= HIWORD(wParam);

			if ((id == IDC_XTWIN_NAME) && (code == EN_CHANGE))
			{
				XtString name = SzGetWindowText(GetDlgItem(hwnd, IDC_XTWIN_NAME));

				EnableWindow(GetDlgItem(hwnd, IDOK), XtsIsIdentifier(name));
			}
			else if ((id == IDCANCEL) && (code == BN_CLICKED))
			{
				EndDialog(hwnd, IDCANCEL);
				return TRUE;
			}
			else if ((id == IDC_XTWIN_LANGUAGE) && (code == CBN_SELCHANGE))
			{
				HWND combo = GetDlgItem(hwnd, IDC_XTWIN_LANGUAGE);

				int curr = (int)SendMessage(combo, CB_GETCURSEL, 0, 0);

				if ((ldd->currentItem >= 0) && (ldd->currentItem < (int)ldd->items.count))
					ldd->items[ldd->currentItem].text = EncodeUnicodeWindowText(GetDlgItem(hwnd, IDC_XTWIN_TEXT));

				ldd->currentItem = curr;

				if ((ldd->currentItem >= 0) && (ldd->currentItem < (int)ldd->items.count))
					DecodeUnicodeWindowText(GetDlgItem(hwnd, IDC_XTWIN_TEXT), ldd->items[ldd->currentItem].text);
			}
			else if ((id == IDOK) && (code == BN_CLICKED))
			{
				ldd->name		= SzGetWindowText(GetDlgItem(hwnd, IDC_XTWIN_NAME));
				ldd->comment	= EncodeUnicodeWindowText(GetDlgItem(hwnd, IDC_XTWIN_COMMENT));

				if ((ldd->currentItem >= 0) && (ldd->currentItem < (int)ldd->items.count))
					ldd->items[ldd->currentItem].text = EncodeUnicodeWindowText(GetDlgItem(hwnd, IDC_XTWIN_TEXT));

				EndDialog(hwnd, IDOK);

				return TRUE;
			}
		}
		break;
	}

	return FALSE;
}

bool LocTextDlg(HWND parent, LocTextData* ldd)
{
	return DialogBoxParam(GetXtWinIconDll(), MAKEINTRESOURCE(IDD_XTWIN_LOC_STRING), parent, LocDialogProc, (LPARAM)ldd) == IDOK;
}