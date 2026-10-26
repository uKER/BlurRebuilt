#ifndef SCARABTIMELINE_H
#define SCARABTIMELINE_H

class SnTlRow;
class SnTlKey;

class ScarabTimeline : public ChildWindow
{
public:
	ScarabTimeline();
	virtual ~ScarabTimeline();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	void SetForm(ScarabForm* form);
	void SetDom(Xt::DomDb* db, LtStruct* rootType, Xt::DomBlock* rootDom);

public:
	SnTlRow* FindRow(XtId id, int index);

private:
	void SyncControls();
	void SyncSelection();

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, ScarabTimeline* tl);

private:
	ScarabForm*		_form;
	Xt::DomDb*		_db;
	LtStruct*		_domType;
	Xt::DomMonitor	_domRoot;

private:
	DwHost			_host;
	DwSplit*		_splitter;
	TimelineWnd*	_wndTimeline;
	ScarabForm*		_wndForm;

private:
	XtUt::LinkList<SnTlRow> _rows;

private:
	bool _resyncControls;
	bool _resyncSelection;

friend class SnTlRow;
friend class SnTlKey;
friend class SnTlContext;
};

#endif // SCARABTIMELINE_H