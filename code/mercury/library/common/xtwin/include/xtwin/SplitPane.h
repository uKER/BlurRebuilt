#ifndef SPLITPANE_H
#define SPLITPANE_H

#ifdef _MSC_VER
#pragma once
#endif

// Includes
//

#include "XtWin/WndCore.h"

// SplitPane
//

class SplitPane : public ChildWindow
{
public:
	SplitPane(DwSplit& split);
	virtual ~SplitPane();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

private:
	DwSplit&	_split;
	bool		_capture;
	int			_position;
};

#endif // SPLITPANE_H