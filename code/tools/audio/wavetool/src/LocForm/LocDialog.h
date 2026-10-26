#ifndef LOCDIALOG_H
#define LOCDIALOG_H

#ifdef _MSC_VER
#pragma once
#endif

struct LocDialogItem
{
	XtString language;
	XtString text;
};

struct LocDialogData
{
	XtString					name;
	XtString					comment;
	int							currentItem;
	XtUt::Array<LocDialogItem>	items;

public:
	LocDialogData() :
		currentItem(0)
	{
	}
};

bool DoLocDialog(HWND parent, LocDialogData* ldd);

#endif // LOCDIALOG_H