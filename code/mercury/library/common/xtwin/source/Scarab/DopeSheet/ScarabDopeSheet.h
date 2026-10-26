#ifndef SCARABDOPESHEET_H
#define SCARABDOPESHEET_H

class SnDsRow;
class SnDsKey;

class ScarabDopeSheet : public ChildWindow
{
public:
	ScarabDopeSheet();
	virtual ~ScarabDopeSheet();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	void SetForm(ScarabForm* form);
	void SetDom(Xt::DomDb* db, LtStruct* rootType, Xt::DomBlock* rootDom);

public:
	SnDsRow* FindRow(XtId id, int index);

private:
	void SyncControls();
	void SyncSelection();

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, ScarabDopeSheet* tl);

private:
	ScarabForm*		_form;
	Xt::DomDb*		_db;
	LtStruct*		_domType;
	Xt::DomMonitor	_domRoot;

private:
	DwHost			_host;
	DwSplit*		_splitter;
	DopeSheetWnd*	_wndDopeSheet;
	ScarabForm*		_wndForm;

private:
	XtUt::LinkList<SnDsRow> _rows;

private:
	bool _resyncControls;
	bool _resyncSelection;

friend class SnDsRow;
friend class SnDsKey;
friend class SnDSContext;
};

#endif // SCARABDOPESHEET_H
