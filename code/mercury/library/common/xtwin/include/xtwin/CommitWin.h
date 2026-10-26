#ifndef COMMITWIN_H
#define COMMITWIN_H

#ifdef _MSC_VER
#pragma once
#endif

struct CommitParams
{
	XtString	path;
	XtString	comment;
	HWND		parentWindow;

	XtUt::Array<XtString> files;

public:
	inline CommitParams() :
		parentWindow(0)
	{
	}
};

enum CommitResult
{
	kCrProceed,
	kCrCancelled,
	kCrFailed
};

CommitResult SorceryCommit(CommitParams* params);

#endif // COMMITWIN_H