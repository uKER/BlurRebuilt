#ifndef XTWIN_FINDER_H
#define XTWIN_FINDER_H

#ifdef _MSC_VER
#pragma once
#endif

// FinderDlgContext

struct FinderDlgContext
{
public:
	FinderDlgContext();
	virtual ~FinderDlgContext();

public:
	virtual Xt::DomDb* Db();

public:
	virtual void OpenRecord(Xt::DomRecord* record);
};

//
// FinderResult
//

struct FinderResult
{
	Xt::DomMonitor	record;
	XtString		path;
	LarvaField*		field;
	LtStruct*		type;
	Xt::DomMonitor	dom;

public:
	FinderResult() :
		field(0),
		type(0)
	{
	}
};

//
// FinderColumn
//

class FinderColumn
{
public:
	FinderColumn(const XtString& title);
	virtual ~FinderColumn();

public:
	virtual XtString Evaluate(FinderResult* result);
	virtual Xt::FieldDom Get(FinderResult* result);
	virtual void Set(FinderResult* result, Xt::FieldDom value);

public:
	const XtString& Title()
	{
		return _title;
	}

private:
	XtString _title;
};

//
// FinderDlg
//

class FinderRow;

class FinderDlg : public DlgWindow
{
public:
	FinderDlg(FinderDlgContext* ctx);
	virtual ~FinderDlg();

public:
	FinderRow* AddRow(FinderRow* row);

public:
	void SetupDefaultLayout();
	void SetupFindDependencies(Xt::DomRecord* record);

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);;

public:
	void DeferLayout();

private:
	void DoLayout();

public:
	Xt::DomDb* Db();
	LarvaSchema* Schema();

public:
	void AddResult(const Xt::QueryResult* result);

private:
	void ExecuteQuery();
	void RefreshQueryColumns();

private:
	static int CALLBACK ListCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

private:
	FinderDlgContext* _ctx;

private:
	HWND _listWnd;
	HWND _statusWnd;
	HWND _findNowWnd;
	HWND _newSearchWnd;

private:
	bool _deferLayout;

private:
	XtUt::List<FinderRow> _rows;
	XtUt::List<FinderResult> _results;
	XtUt::List<FinderColumn> _columns;
};

#endif // XTWIN_FINDER_H