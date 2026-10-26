#ifndef GRIDFORM_H
#define GRIDFORM_H

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

class GridMdiWindow : public MdiWindow
{
public:
	GridMdiWindow(class SorceryView& view);
	virtual ~GridMdiWindow();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	void AdjustSizeToForm();
	void UpdateWindowTitle();
	void AddColumnsForType(const XtString& path, const XtString& labelPrefix, LtStruct* type);

public:
	inline SorceryView& View() { return _view; }
	void SetType(const XtString& typeName) { _typeName = typeName; }

private:
	SorceryView&	_view;
	class GridView*	_gridView;
	XtString		_typeName;
};

#endif // GRIDFORM_H