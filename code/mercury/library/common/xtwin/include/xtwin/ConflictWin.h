#ifndef CONFLICTWIN_H
#define CONFLICTWIN_H

#ifdef _MSC_VER
#pragma once
#endif

struct ConflictParams
{
	HWND		parentWindow;
	XtString	path;

public:
	inline ConflictParams() :
		parentWindow(0)
	{
	}
};

bool SorceryResolveConflicts(ConflictParams* params);
bool SorceryResolveConflictsAsync(AsyncOp* op, ConflictParams* params);

#endif // CONFLICTWIN_H