#ifndef HYPERVIEW_H
#define HYPERVIEW_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Core/List.h"
#include "XtWin/WndCore.h"
#include "Application/SorceryApp.h"
#include "XtWin/NgMaths.h"

//
// HyperViewMdiWindow
//

class HyperViewMdiWindow : public MdiWindow
{
public:
	HyperViewMdiWindow(class SorceryView& view);
	virtual ~HyperViewMdiWindow();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	void SetRecord(Xt::DomRecord* record);
	void GoForward(XtId recordId);
	void GoBack();
	void AdjustSizeToForm();
	void UpdateWindowTitle();

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
	SorceryView&		_view;
	class NodeGraph*	_graph;
	Xt::DomMonitor		_record;
	XtUt::Array<Item>	_history;
};

#endif // HYPERVIEW_H