#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

//
// ScarabTabWnd
//

ScarabTabWnd::ScarabTabWnd(ScarabForm* form) :
	_form(form)
{
}

ScarabTabWnd::~ScarabTabWnd()
{
}

bool ScarabTabWnd::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!ChildWindow::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_CLIPCHILDREN;
	cs->dwExStyle	|= WS_EX_CONTROLPARENT;

	return true;
}

LRESULT ScarabTabWnd::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(_hwnd, &ps);

			SetBkColor(ps.hdc, GetSysColor(COLOR_3DSHADOW));
			ExtTextOut(ps.hdc, 0, 0, ETO_OPAQUE, &ps.rcPaint, 0, 0, 0);

			PaintButtons(ps.hdc);

			EndPaint(_hwnd, &ps);
		}
		return 0;

		case WM_LBUTTONDOWN:
		{
			WinPoint pt(LOWORD(lParam), HIWORD(lParam));
			WinRect rcClient(*this);

			int tabWidth = 100;

			for(XtUt::List<ScarabForm::TabBase>::Iterator it(&_form->_tabs); it; ++it)
			{
				WinRect rc;
				
				rc.left		= rcClient.left + (it.index * tabWidth);
				rc.top		= rcClient.top + (_form->_activeTab == it.index) ? 0 : 1;
				rc.right	= rcClient.left + ((it.index + 1) * tabWidth) - 1;
				rc.bottom	= rcClient.bottom;

				if (PtInRect(&rc, pt))
				{
					if (_form->_activeTab != it.index)
					{
						_form->_activeTab = it.index;
						_form->UpdateTabs();

						InvalidateWindow(*this);
					}

					break;
				}
			}
		}
		return 0;
	}

	return ChildWindow::WndProc(msg, wParam, lParam);
}

void ScarabTabWnd::PaintButtons(HDC hdc)
{
	WinRect rcClient(*this);

	int tabWidth = 100;

	SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));

	for(XtUt::List<ScarabForm::TabBase>::Iterator it(&_form->_tabs); it; ++it)
	{
		WinRect rc;

		rc.left		= rcClient.left + (it.index * tabWidth);
		rc.top		= rcClient.top + (_form->_activeTab == it.index) ? 0 : 1;
		rc.right	= rcClient.left + ((it.index + 1) * tabWidth) - 1;
		rc.bottom	= rcClient.bottom;

		if (_form->_activeTab == it.index)
			SetBkColor(hdc, GetSysColor(COLOR_3DHIGHLIGHT));
		else
			SetBkColor(hdc, GetSysColor(COLOR_3DFACE));

		ExtTextOut(hdc, rc.left + 4, rcClient.top + 4, ETO_OPAQUE | ETO_CLIPPED, &rc, it->label, it->label.Length(), 0);
	}
}

//
// ScarabForm::TabBase
//

ScarabForm::TabBase::~TabBase()
{
}

void ScarabForm::TabBase::SetDom(Xt::DomElement* element, LarvaType* type)
{
}

void ScarabForm::TabBase::SetDom(Xt::DomBlock* parentBlock, LarvaField* field)
{
}

WinSize ScarabForm::TabBase::IdealSize()
{
	return WinSize();
}

void ScarabForm::TabBase::Layout()
{
}

struct ScarabForm::TabForm : public TabBase
{
	ScarabScrollPanel	scroll;
	ScarabPanel			panel;

public:
	class SpcTabForm : public ScarabPanelContext
	{
		ScarabForm::TabForm* _form;

	public:
		SpcTabForm(ScarabForm::TabForm* form) :
			_form(form)
		{
		}

	public:
		virtual void OnSizeChanged()
		{
			_form->scroll.Refresh();
		}
	};

	SpcTabForm spc;

public:
	TabForm(ScarabForm* form) :
		scroll(form),
		spc(this),
		panel(form, &spc)
	{
	}

public:
	virtual void SetDom(Xt::DomElement* element, LarvaType* type)
	{
		panel.SetDom(element, type, filter);
		scroll.Refresh();
	}

	virtual void SetDom(Xt::DomBlock* parentBlock, LarvaField* field)
	{
		panel.SetDom(parentBlock, field);
		scroll.Refresh();
	}

	virtual void Layout()
	{
		panel.LayoutNow();
		scroll.Refresh();
	}

	virtual WinSize IdealSize()
	{
		WinRect rc(panel);

		return WinSize(rc.Width(), rc.Height());
	}

	virtual HWND Window()
	{
		return scroll;
	}

	virtual HWND MsgTarget()
	{
		return panel;
	}
};

struct ScarabForm::TabGraph : public TabBase
{
	ScarabDomNodeGraph graph;

public:
	TabGraph(ScarabForm* form)
	{
	}

public:
	virtual void SetDom(Xt::DomElement* element, LarvaType* type)
	{
		graph.SetDom(AsStruct(type), AsBlock(element));
	}

	virtual void SetDom(Xt::DomBlock* parentBlock, LarvaField* field)
	{
	}

	virtual HWND Window()
	{
		return graph;
	}

	virtual HWND MsgTarget()
	{
		return graph;
	}

	virtual void Layout()
	{
		graph.Layout();
	}
};

struct ScarabForm::TabDopeSheet : public TabBase
{
	ScarabDopeSheet dopesheet;

public:
	TabDopeSheet(ScarabForm* form)
	{
	}

public:
	virtual void SetDom(Xt::DomElement* element, LarvaType* type)
	{
		dopesheet.SetDom(DbOf(element), AsStruct(type), AsBlock(element));
	}

	virtual void SetDom(Xt::DomBlock* parentBlock, LarvaField* field)
	{
	}

	virtual HWND Window()
	{
		return dopesheet;
	}

	virtual HWND MsgTarget()
	{
		return dopesheet;
	}

	virtual void Layout()
	{
		//dopesheet.Layout();
	}
};

struct ScarabForm::TabTimeline : public TabBase
{
	ScarabTimeline timeline;

public:
	TabTimeline(ScarabForm* form)
	{
	}

public:
	virtual void SetDom(Xt::DomElement* element, LarvaType* type)
	{
		timeline.SetDom(DbOf(element), AsStruct(type), AsBlock(element));
	}

	virtual void SetDom(Xt::DomBlock* parentBlock, LarvaField* field)
	{
	}

	virtual HWND Window()
	{
		return timeline;
	}

	virtual HWND MsgTarget()
	{
		return timeline;
	}

	virtual void Layout()
	{
		//timeline.Layout();
	}
};

//
// ScarabForm
//

ScarabForm::ScarabForm(ScarabFormContext* formContext) :
	_formContext(formContext),
	_db(0),
	_tabWnd(this),
	_activeTab(0)
{
}

ScarabForm::~ScarabForm()
{
	XtUt::Free(&_tabs);

	delete _formContext;
}

void ScarabForm::SetDom(Xt::DomElement* element, LarvaType* type)
{
	Xt::Monitor(&_dom, element);

	_db = DbOf(element);

	XtUt::Free(&_tabs);

	_activeTab = 0;

	AddTabsForType(type);

	for(XtUt::List<TabBase>::Iterator it(&_tabs); it; ++it)
		it->SetDom(element, type);

	UpdateTabs();
}

void ScarabForm::SetDom(Xt::DomBlock* parentBlock, LarvaField* field)
{
	Xt::Monitor(&_dom, parentBlock);

	_db = DbOf(parentBlock);

	XtUt::Free(&_tabs);

	_activeTab = 0;

	AddTab("");

	for(XtUt::List<TabBase>::Iterator it(&_tabs); it; ++it)
		it->SetDom(parentBlock, field);

	UpdateTabs();
}

void ScarabForm::SetDom(Xt::DomComponent* component, LarvaField* field)
{
	Xt::DomDb* db = DbOf(component);

	LtStruct* type = 0;

	if (component && db)
		type = AsStruct(TypeOf(db->schema, field, component));

	SetDom(&component->data, type);
}

void ScarabForm::SetRecord(Xt::DomRecord* record)
{
	LarvaType* type = 0;

	Xt::DomDb* db = DbOf(record);

	if (record && db)
		type = TypeOf(db->schema, record);

	SetDom(&record->root.data, type);
}

WinSize ScarabForm::IdealSize()
{
	WinSize largest;

	for(XtUt::List<TabBase>::Iterator it(&_tabs); it; ++it)
	{
		it->Layout();

		WinSize sz = it->IdealSize();

		largest.cx = Max(largest.cx, sz.cx);
		largest.cy = Max(largest.cy, sz.cy);
	}

	return largest;
}

bool ScarabForm::AddTab(const XtString& filter)
{
	for(XtUt::List<TabBase>::Iterator it(&_tabs); it; ++it)
	{
		if (_stricmp(it->filter, filter) == 0)
			return true;
	}

	TabForm* tab = new TabForm(this);

	if (!tab)
		return false;

	if (!tab->scroll.Create(*this) || !tab->panel.Create(tab->scroll))
	{
		delete tab;
		return false;
	}

	tab->scroll.SetPanel(&tab->panel);

	tab->filter	= filter;
	tab->label	= CleanUpLabel(filter);

	if (!*tab->label)
		tab->label = "General";

	return XtUt::AppendEx(&_tabs, (TabBase*)tab) != 0;
}

bool ScarabForm::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!ChildWindow::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_CLIPCHILDREN;
	cs->dwExStyle	|= WS_EX_CONTROLPARENT;

	return true;
}

LRESULT ScarabForm::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
			if (ChildWindow::WndProc(msg, wParam, lParam) < 0)
				return -1;

			if (!_tabWnd.Create(*this))
				return -1;
		return 0;

		case WM_DESTROY:
			XtUt::Free(&_tabs);
		break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(_hwnd, &ps);

			ExtTextOut(ps.hdc, 0, 0, ETO_OPAQUE, &ps.rcPaint, 0, 0, 0);

			EndPaint(_hwnd, &ps);
		}
		return 0;

		case WM_SIZE:
			UpdateTabs();
		break;

		/*case WM_NCHITTEST:
		{
			LRESULT result = ChildWindow::WndProc(msg, wParam, lParam);

			if (result == HTCLIENT)
				return HTTRANSPARENT;

			return result;
		}
		break;*/

		case WM_MOUSEWHEEL:
		{
			static bool recursionHack = false;

			if (!recursionHack)
			{
				recursionHack = true;

				if (TabBase* tab = At(&_tabs, _activeTab))
					SendMessage(tab->MsgTarget(), msg, wParam, lParam);

				recursionHack = false;
			}
		}
		return 0;

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			SetFocus(*this);
		return 0;
	}

	return ChildWindow::WndProc(msg, wParam, lParam);
}

void ScarabForm::AddTabsForType(LarvaType* type)
{
	if (!type || (type->kind != kLtStruct)) 
		return;

	LtStruct* ls = (LtStruct*)type;

	for(XtUt::List<LarvaField>::Iterator it(&ls->fields); it; ++it)
	{
		if (it->kind == kLfControl)
		{
			LfControl* control = (LfControl*)(LarvaField*)it;

			if (control->type == "NodeGraph")
			{
				if (control->attrs.GetBool("NodeGraph", "ownTab", false))
				{
					TabGraph* tab = new TabGraph(this);

					XtString filter = it->tab;

					if (!*filter)
						filter = "NodeGraph";

					tab->filter	= "__ng_" + filter;
					tab->label	= CleanUpLabel(filter);

					tab->graph.SetForm(this);
					tab->graph.Create(*this);

					tab->graph.SetWatermark(control->attrs.GetString("NodeGraph", "watermark"));

					XtUt::AppendEx(&_tabs, (TabBase*)tab);
				}
			}
			else if (control->type == "DopeSheet")
			{
				if (control->attrs.GetBool("DopeSheet", "ownTab", false))
				{
					TabDopeSheet* tab = new TabDopeSheet(this);

					XtString filter = it->tab;

					if (!*filter)
						filter = "DopeSheet";

					tab->filter	= "__tl_" + filter;
					tab->label	= CleanUpLabel(filter);

					tab->dopesheet.SetForm(this);
					tab->dopesheet.Create(*this);

					XtUt::AppendEx(&_tabs, (TabBase*)tab);
				}
			}
			else if (control->type == "Timeline")
			{
				if (control->attrs.GetBool("Timeline", "ownTab", false))
				{
					TabTimeline* tab = new TabTimeline(this);

					XtString filter = it->tab;

					if (!*filter)
						filter = "Timeline";

					tab->filter	= "__tl_" + filter;
					tab->label	= CleanUpLabel(filter);

					tab->timeline.SetForm(this);
					tab->timeline.Create(*this);

					XtUt::AppendEx(&_tabs, (TabBase*)tab);
				}
			}
		}

		if (!it->hidden)
			AddTab(it->tab);
	}

	if (ls->base)
		AddTabsForType(ls->base->type);
}

void ScarabForm::UpdateTabs()
{
	WinRect rcClient(*this);

	int tabHeight = 20;

	if (_tabs.count <= 1)
		tabHeight = 0;

	SetWindowPos(_tabWnd, 0, rcClient.left, 0, rcClient.Width(), tabHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	HWND focus = GetFocus();

	bool killFocus = false;

	for(XtUt::List<TabBase>::Iterator it(&_tabs); it; ++it)
	{
		int flags = (it.index == _activeTab) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;

		SetWindowPos(it->Window(), 0, rcClient.left, rcClient.top + tabHeight, rcClient.Width(), rcClient.Height() - tabHeight, SWP_NOACTIVATE | SWP_NOZORDER | flags);

		EnableWindow(it->Window(), it.index == _activeTab);

		it->Layout();

		if (it.index != _activeTab)
		{
			if (IsChild(it->Window(), focus))
				killFocus = true;
		}
	}

	if (killFocus)
	{
		if (TabBase* tab = At(&_tabs, _activeTab))
			SetFocus(tab->MsgTarget());
		else
			SetFocus(*this);
	}
}

bool ScarabForm::IsGroupCollapsed(const XtString& key, bool defValue)
{
	if (bool* v = XtUt::At(&_collapsedGroups, key))
		return *v;

	return defValue;
}

void ScarabForm::SetGroupCollapsed(const XtString& key, bool collapsed)
{
	if (bool* v = XtUt::At(&_collapsedGroups, key))
	{
		*v = collapsed;
	}
	else
		XtUt::Insert(&_collapsedGroups, key, new bool(collapsed));
}