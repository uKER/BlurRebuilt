#ifndef XTWIN_LOCTEXTDLG_H
#define XTWIN_LOCTEXTDLG_H

#ifdef _MSC_VER
#pragma once
#endif

struct LocTextItem
{
	XtString language;
	XtString text;
};

struct LocTextData
{
	XtString					name;
	XtString					comment;
	int							currentItem;
	XtUt::Array<LocTextItem>	items;

public:
	LocTextData() :
		currentItem(0)
	{
	}
};

bool LocTextDlg(HWND parent, LocTextData* ldd);

#endif // XTWIN_LOCTEXTDLG_H