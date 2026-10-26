#ifndef SCARAB_H
#define SCARAB_H

#ifdef _MSC_VER
#pragma once
#endif

class ScarabNode;
class ScarabForm;

//
// ScarabDrawContext
//

struct ScarabDrawContext
{
	COLORREF	backgroundColour;
	HBRUSH		backgroundBrush;

	COLORREF	borderColour;
	HPEN		borderPen;
	HBRUSH		borderBrush;

	COLORREF	headerColour;
	HPEN		headerPen;
	HBRUSH		headerBrush;

	HFONT		labelFont;
	HFONT		boldFont;
	HFONT		iconFont;
	HFONT		iconFont2;

	int			labelSizeY;

public:
	ScarabDrawContext();
	~ScarabDrawContext();
};

//
// ScarabFormContext
//

class ScarabFormContext
{
public:
	ScarabFormContext();
	virtual ~ScarabFormContext();

public:
	virtual void OpenRecord(Xt::DomRecord* record);
	virtual void BeginDropper(LarvaField* field, Xt::DomElement* element);
};

//
// ScarabPanel
//

class ScarabPanelContext
{
public:
	ScarabPanelContext();
	virtual ~ScarabPanelContext();

public:
	virtual void OnSizeChanged();
};

class ScarabPanel : public ChildWindow
{
public:
	ScarabPanel(ScarabForm* form, ScarabPanelContext* panelContext);
	virtual ~ScarabPanel();

public:
	int AllocateControlId(ScarabNode* node);
	void FreeControlId(int id);

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	void DirtyLayout();
	void LayoutNow();

public:
	void SetDom(Xt::DomElement* element, LarvaType* type, const XtString& filter);
	void SetDom(Xt::DomBlock* parentBlock, LarvaField* field);

public:
	ScarabNode* Root();
	ScarabFormContext* FormContext();
	ScarabDrawContext* DrawContext();
	ScarabForm* Form();
	Xt::DomDb* Db();

private:
	struct NodeId
	{
		int			id;
		ScarabNode*	node;

	public:
		inline NodeId(int id_ = 0, ScarabNode* node_ = 0) :
			id(id_),
			node(node_)
		{
		}

	public:
		inline bool operator<(const NodeId& ni) const
		{
			return id < ni.id;
		}
	};

private:
	ScarabForm*			_form;
	ScarabPanelContext*	_panelContext;

private:
	class ScarabNode*	_root;
	XtUt::Set<NodeId>	_ids;

private:
	bool				_dirtyLayout;

private:
	Xt::DomMonitor		_dom;
};

//
// ScarabScrollPanel
//

class ScarabScrollPanel : public ChildWindow
{
public:
	ScarabScrollPanel(ScarabForm* form);
	virtual ~ScarabScrollPanel();

public:
	void SetPanel(ScarabPanel* panel);
	void Refresh();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void CalculateScrollBar();
	void DoMouseWheel(WPARAM wParam);

private:
	ScarabForm*		_form;
	ScarabPanel*	_panel;
};

//
// ScarabTabWnd
//

class ScarabTabWnd : public ChildWindow
{
public:
	ScarabTabWnd(ScarabForm* form);
	virtual ~ScarabTabWnd();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void PaintButtons(HDC hdc);

private:
	ScarabForm* _form;
};

//
// ScarabForm
//

class ScarabForm : public ChildWindow
{
public:
	ScarabForm(ScarabFormContext* context);
	virtual ~ScarabForm();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	void SetDom(Xt::DomElement* element, LarvaType* type);
	void SetDom(Xt::DomBlock* parentBlock, LarvaField* field);
	void SetDom(Xt::DomComponent* component, LarvaField* field);
	void SetRecord(Xt::DomRecord* record);

public:
	WinSize IdealSize();

public:
	bool AddTab(const XtString& filter);

private:
	void AddTabsForType(LarvaType* type);
	void UpdateTabs();

public:
	inline ScarabFormContext* FormContext()
	{
		return _formContext;
	}

	inline ScarabDrawContext* DrawContext()
	{
		return &_drawContext;
	}

	inline Xt::DomDb* Db()
	{
		return _db;
	}

public:
	bool IsGroupCollapsed(const XtString& key, bool defValue);
	void SetGroupCollapsed(const XtString& key, bool collapsed);

private:
	struct TabBase
	{
		XtString filter;
		XtString label;

	public:
		virtual ~TabBase();

	public:
		virtual void SetDom(Xt::DomElement* element, LarvaType* type);
		virtual void SetDom(Xt::DomBlock* parentBlock, LarvaField* field);
		virtual WinSize IdealSize();
		virtual HWND Window() = 0;
		virtual HWND MsgTarget() = 0;
		virtual void Layout();
	};

	struct TabForm;
	struct TabGraph;
	struct TabDopeSheet;
	struct TabTimeline;

private:
	ScarabFormContext*	_formContext;
	ScarabDrawContext	_drawContext;
	Xt::DomDb*			_db;
	Xt::DomMonitor		_dom;
	ScarabTabWnd		_tabWnd;
	XtUt::List<TabBase>	_tabs;
	int					_activeTab;

private:
	XtUt::Dictionary<XtString, bool> _collapsedGroups;

friend class ScarabTabWnd;
};

#endif // SCARAB_H