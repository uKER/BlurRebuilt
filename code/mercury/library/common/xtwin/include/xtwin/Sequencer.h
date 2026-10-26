#ifndef XTWIN_SEQUENCER_H
#define XTWIN_SEQUENCER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "XtWin\WndCore.h"
#include "XtWin\NgMaths.h"

struct SequencerLayoutContext
{
	float	width;
	float	x;
	float	y;

public:
	SequencerLayoutContext();
};

class Sequencer;

class SequencerItem
{
public:
	SequencerItem();
	virtual ~SequencerItem();

public:
	virtual void OnClick();

public:
	XtUt::WeakAnchor<SequencerItem>& Anchor() { return _anchor; }

public:
	void SetTitle(const Sz16& title);
	void SetInfo(const Sz16& info);
	void SetBackground(XtU32 colour);
	void SetIcon(Gdiplus::Bitmap* icon);
	void SetIndent(float indent);
	void SetHeight(float height);
	void SetGap(float gap);

public:
	SequencerItem* Add(SequencerItem* item);
	SequencerItem* Insert(SequencerItem* item, SequencerItem* after);
	void Clear();

public:
	void Select();
	void Deselect();

public:
	const NgRect& Location() const	{ return _location; }
	const Sz16& Title() const		{ return _title; }
	const Sz16& Info() const		{ return _info; }
	XtU32 Background() const		{ return _background; }
	HANDLE Icon() const				{ return _icon; }
	float Indent() const			{ return _indent; }
	float Height() const			{ return _height; }
	float Gap() const				{ return _gap; }

private:
	void SetDirty(bool redoLayout);
	void DoLayout(SequencerLayoutContext* ctx);
	void DoRender(Gdiplus::Graphics* g);
	SequencerItem* DoHitTest(const NgVector& pt);

private:
	XtUt::Link<SequencerItem> _linkItem;
	XtUt::WeakAnchor<SequencerItem> _anchor;

private:
	Sequencer*						_sequencer;
	SequencerItem*					_parent;
	XtUt::LinkList<SequencerItem>	_items;

private:
	bool						_isSelected;
	XtUt::Link<SequencerItem>	_linkSelection;

private:
	NgRect _location;

private:
	Sz16				_title;
	Sz16				_info;
	XtU32				_background;
	Gdiplus::Bitmap*	_icon;
	float				_indent;
	float				_height;
	float				_gap;

friend class Sequencer;
};

class Sequencer : public ChildWindow
{
public:
	Sequencer();
	virtual ~Sequencer();

public:
	SequencerItem* Root()
	{
		return &_root;
	}

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	SequencerItem* HitTest(const NgVector& pt);

private:
	void Layout();
	void DoScrollBars();
	void Render(Gdiplus::Graphics* g);

private:
	void SetDirty(bool redoLayout);

private:
	SequencerItem _root;

private:
	XtUt::LinkList<SequencerItem> _selection;

private:
	bool	_isDirty;
	float	_scrollHeight;
	float	_scrollOffset;

friend class SequencerItem;
};

#endif // XTWIN_SEQUENCER_H