#ifndef DYNAMICFORM_H
#define DYNAMICFORM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Core/List.h"
#include "XtWin/WndCore.h"
#include "Application/SorceryApp.h"
#include "XtWin/Scarab.h"

//
// DynamicFormMdiWindow
//

class DynamicFormMdiWindow : public MdiWindow
{
public:
	DynamicFormMdiWindow(class SorceryView& view);
	virtual ~DynamicFormMdiWindow();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	void AdjustSizeToForm();
	void UpdateWindowTitle();

private:
	SorceryView&	_view;
	ScarabForm*		_form;
	HWND			_lastFocus;
};

#endif // DYNAMICFORM_H