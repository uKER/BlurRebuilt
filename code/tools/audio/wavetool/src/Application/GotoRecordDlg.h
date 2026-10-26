#ifndef GOTORECORDDLG_H
#define GOTORECORDDLG_H

#ifdef _MSC_VER
#pragma once
#endif

class GotoRecordDlg
{
public:
	GotoRecordDlg(HWND parentWindow);
	~GotoRecordDlg();

public:
	bool Modal();

public:
	inline XtId RecordId()
	{
		return _recordId;
	}

	inline XtId AltRecordId()
	{
		return _altRecordId;
	}

private:
	static INT_PTR CALLBACK WndProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	HWND	_parentWindow;
	XtId	_recordId;
	XtId	_altRecordId;
};

#endif // CONNECTTO_H