#ifndef SORCERYWINDOW_H
#define SORCERYWINDOW_H

#ifdef _MSC_VER
#pragma once
#endif

#include "XtWin/WndCore.h"
#include "Application/SorceryView.h"
#include "Explorer/Explorer.h"
#include "XtWin/DeskWindow.h"
#include "DynamicForm/DynamicForm.h"
#include "XtWin/Finder.h"

const int kWmNotifyActivate	= WM_APP + 1;
const int kWmNotifySetFocus	= WM_APP + 2;
const int kWmNetworkNotify	= WM_APP + 3;
const int kWmDbNotify		= WM_APP + 4;

const int kNetConnected			= 'CONN';
const int kNetDisconnected		= 'DISC';
const int kNetResolved			= 'RSLV';
const int kNetMessageReceived	= 'RMSG';

class SorceryWindowFdc : public FinderDlgContext
{
	SorceryWindow* window;

public:
	SorceryWindowFdc(SorceryWindow* window_);

public:
	virtual Xt::DomDb* Db();

public:
	virtual void OpenRecord(Xt::DomRecord* record);
};

class SorceryWindow : public FrameWindow
{
public:
	SorceryWindow(class SorceryApp& app);
	~SorceryWindow();

public:
	virtual bool	PreCreateMdiWindow(CLIENTCREATESTRUCT* ccs);
	virtual bool	PreCreateWindow(CREATESTRUCT* cs);
	virtual bool	PreTranslateMessage(MSG& msg);
	virtual LRESULT	WndProc(UINT Msg, WPARAM wParam, LPARAM lParam);

public:
	void SetWindowTitle();

public:
	void OnFileNew();
	void OnFileOpen();
	void OnFileClose();
	void OnBuildProject(bool dataOnly);
	void OpenRecord(Xt::DomRecord* record);
	void OpenPack(Xt::DomPack* pack);
	void OpenDb(Xt::DomDb* db);
	void OpenGridView(const XtString& typeName);
	void OpenHyperView(Xt::DomRecord* record);
	void OpenSearchWindow();
	void FindDependencies(Xt::DomRecord* record);

public:
	class SorceryView* GetActiveView();

public:
	inline SorceryApp& App()
	{
		return _app;
	}

	inline HWND StatusWnd()
	{
		return _statusHwnd;
	}

	inline ExplorerView* Explorer()
	{
		return &_explorer;
	}

private:
	class SorceryApp& _app;

private:
	HACCEL				_accelerator;
	HMENU				_menu;
	HWND				_statusHwnd;
	ExplorerView		_explorer;
	DwHost				_host;
	SorceryWindowFdc	_finderCtx;
	FinderDlg			_finder;
	Xt::DomMonitor		_monitor;

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SorceryWindow* window);

public:
	LinkList<class SorceryView> _views;

public:
	bool _isConnected;

friend class ExplorerView;
friend class SorceryApp;
};

#endif // SORCERYWINDOW_H