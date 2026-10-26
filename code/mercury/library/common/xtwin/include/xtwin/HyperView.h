#ifndef HYPERVIEW_H
#define HYPERVIEW_H

#ifdef _MSC_VER
#pragma once
#endif

#include "XtWin/Core.h"
#include "XtWin/WndCore.h"
#include "XtWin/NgMaths.h"

//
// HyperViewMdiWindow
//

class HyperViewForm : public ChildWindow
{
public:
	HyperViewForm(Xt::DomDb* db);
	virtual ~HyperViewForm();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	void SetDom(Xt::DomElement* record);
	void GoForward(XtId recordId);
	void GoBack();
	void AdjustSizeToForm();
	void UpdateWindowTitle();

public:
	void SetDb(Xt::DomDb* db);

public:
	void Populate();

private:
	struct Item
	{
		Item(XtId id_, const NgVector& offset_, const NgVector& scale_) :
			id(id_),
			offset(offset_),
			scale(scale_)
		{
		}

	public:
		XtId id;
		NgVector offset;
		NgVector scale;
	};

private:
	class NodeGraph*	_graph;
	Xt::DomMonitor		_component;
	XtUt::Array<Item>	_history;
	Xt::DomDb*			_db;
};

#endif // HYPERVIEW_H