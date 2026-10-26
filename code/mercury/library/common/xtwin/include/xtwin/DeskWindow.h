#ifndef DESKWINDOW_H
#define DESKWINDOW_H

#ifdef _MSC_VER
#pragma once
#endif

// Prototypes
//

class DwItem;
class DwRoot;
class DwSplit;
class DwPane;
class DwTab;
class DwHost;
class GenericPane;

// Includes
//

#include "XtWin/WndCore.h"
#include "Core/List.h"
#include "XtWin/SplitPane.h"

// DwType
//

enum DwType
{
	kDwtRoot,
	kDwtSplit,
	kDwtPane,
	kDwtTab
};

// DwDrawContext

struct DwDrawContext
{
	HDC			hdc;
	WinRect		rcPaint;

	COLORREF	borderColour;
	HPEN		borderPen;
	HBRUSH		borderBrush;

	COLORREF	backgroundColour;
	HPEN		backgroundPen;
	HBRUSH		backgroundBrush;

	COLORREF	faceColour;

	COLORREF	labelColour;
	HFONT		labelFont;

public:
	DwDrawContext();
	~DwDrawContext();

public:
	inline operator HDC() const
	{
		return hdc;
	}
};

// DwItem
//

class DwItem
{
public:
	DwItem(DwType type, DwHost* host);
	virtual ~DwItem();

public:
	DwItem* GetChild(int idx);
	DwItem* SetChild(int idx, DwItem* child);
	DwItem* Detach();

public:
	void PerformLayout(const WinRect& location, HDWP dwp, bool scaling);

public:
	virtual bool Create();
	virtual int CountWindows();
	virtual WinSize RequiredSize();
	virtual void OnLayout(const WinRect& location, HDWP dwp, bool scaling);
	virtual void OnPaint(DwDrawContext& dc);

public:
	inline DwType Type()
	{
		return _type;
	}

	inline DwHost* Host()
	{
		return _host;
	}

	inline DwItem* Parent()
	{
		return _parent;
	}

	inline int ChildIndex()
	{
		return _childIndex;
	}

	inline const WinRect& Location()
	{
		return _location;
	}

public:
	static const int kMaxChildren = 2;

private:
	DwType	_type;
	DwHost*	_host;
	DwItem*	_parent;
	int		_childIndex;
	DwItem*	_child[kMaxChildren];
	WinRect	_location;
};

// DwRoot
//

class DwRoot : public DwItem
{
public:
	DwRoot(DwHost* host);
	virtual ~DwRoot();
};

// DwSplit
//

class DwSplit : public DwItem
{
public:
	DwSplit(DwHost* host);
	virtual ~DwSplit();

public:
	void SetPosition(int position);
	void SetVertical(bool vertical);

public:
	virtual bool Create();
	virtual int CountWindows();
	virtual WinSize RequiredSize();
	virtual void OnLayout(const WinRect& location, HDWP dwp, bool scaling);

public:
	inline int Position() const
	{
		return _position;
	}

	inline bool Vertical() const
	{
		return _vertical;
	}

private:
	DwSplit* FindShiftableAncestor(bool lookLeft);
	void SetPositionImpl(int position);

private:
	SplitPane	_split;
	int			_position;
	float		_ratio;
	bool		_vertical;
};

// DwPane
//

class DwPane : public DwItem
{
public:
	DwPane(DwHost* host);
	virtual ~DwPane();

public:
	DwTab* Add(DwTab* tab);

public:
	virtual int CountWindows();
	virtual WinSize RequiredSize();
	virtual void OnLayout(const WinRect& location, HDWP dwp, bool scaling);
	virtual void OnPaint(DwDrawContext& dc);

private:
	List<DwTab> _tabs;
};

// DwTab
//

class DwTab : public DwItem
{
public:
	DwTab(DwHost* host, HWND window);
	virtual ~DwTab();

public:
	virtual int CountWindows();
	virtual WinSize RequiredSize();
	virtual void OnLayout(const WinRect& location, HDWP dwp, bool scaling);
	virtual void OnPaint(DwDrawContext& dc);

private:
	HWND _window;
};

// DwHost
//

class DwHost
{
public:
	DwHost();
	~DwHost();

public:
	void SetWindow(HWND window);
	void SetLocation(const WinRect& location);

public:
	void PerformLayout(bool scaling);
	void Paint(HDC hdc, const WinRect& rcPaint);

public:
	inline HWND Window()
	{
		return _window;
	}

	inline const WinRect& Location()
	{
		return _location;
	}

	inline DwRoot* Root()
	{
		return &_root;
	}

private:
	HWND	_window;
	WinRect	_location;
	DwRoot	_root;
};

//
// DeskWindow
//

class DeskWindow : public Window
{
public:
	DeskWindow();
	virtual ~DeskWindow();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	inline DwHost* Host()
	{
		return &_host;
	}

private:
	DwHost _host;
};

#endif // DESKWINDOW_H