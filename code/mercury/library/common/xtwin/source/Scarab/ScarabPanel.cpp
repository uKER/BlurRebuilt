#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

//
// ScarabPanelContext
//

ScarabPanelContext::ScarabPanelContext()
{
}

ScarabPanelContext::~ScarabPanelContext()
{
}

void ScarabPanelContext::OnSizeChanged()
{
}

//
// ScarabPanel
//

ScarabPanel::ScarabPanel(ScarabForm* form, ScarabPanelContext* panelContext) :
	_form(form),
	_panelContext(panelContext),
	_root(0),
	_dirtyLayout(false)
{
	if ((_root = new ScarabNode(this)) != 0)
	{
		_root->_padding = WinRect(8, 8, 8, 8);
	}
}

ScarabPanel::~ScarabPanel()
{
	delete _root;
}

int ScarabPanel::AllocateControlId(ScarabNode* node)
{
	for(int i = 0; i < 128; i++)
	{
		int id = 1 + (rand() % 25000);

		if (!Exists(&_ids, NodeId(id)))
			return Insert(&_ids, NodeId(id, node)) ? id : 0;
	}

	return 0;
}

void ScarabPanel::FreeControlId(int id)
{
	Remove(&_ids, FirstOf(&_ids, NodeId(id)), LastOf(&_ids, NodeId(id)));
}

void ScarabPanel::DirtyLayout()
{
	if (!_dirtyLayout && *this)
	{
		_dirtyLayout = true;

		PostMessage(*this, WM_USER, 0, 0);
	}
}

void ScarabPanel::LayoutNow()
{
	//static int indent = 0; indent++; DWORD w= GetTickCount();

	_dirtyLayout = false;

	_root->Build();

	{
		HDC ic = CreateIC(_T("DISPLAY"), 0, 0, 0);
		SelectObject(ic, DrawContext()->labelFont);
		_root->Measure(ic);
		DeleteDC(ic);
	}

	ScarabLayoutContext ctx;

	ctx.Begin();
	_root->Layout(&ctx, WinPoint(0, 0));
	ctx.End();

	WinRect rcBefore(*this);

	SetWindowPos(*this, 0, 0, 0, _root->_size.cx, _root->_size.cy, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);

	WinRect rcAfter(*this);

	InvalidateWindow(*this);

	if (_panelContext)
	{
		if ((rcBefore.Width() != rcAfter.Width()) || (rcBefore.Height() != rcAfter.Height()))
			_panelContext->OnSizeChanged();
	}

	//--indent; DebugLn("%*sLayoutNow: %i", indent * 2, "", GetTickCount()-w);
}

void ScarabPanel::SetDom(Xt::DomElement* element, LarvaType* type, const XtString& filter)
{
	Xt::Monitor(&_dom, element);

	_root->Clear();

	if (type && (type->kind == kLtStruct))
		_root->Add(new SnRecord((LtStruct*)type, filter));

	_root->Populate();

	if (Xt::DomBlock* block = AsBlock(element))
		_root->Bind(block);

	DirtyLayout();
}

void ScarabPanel::SetDom(Xt::DomBlock* parentBlock, LarvaField* field)
{
	Xt::Monitor(&_dom, parentBlock);

	_root->Clear();

	ScarabBuild(_root, field);

	_root->Populate();
	_root->Bind(parentBlock);

	DirtyLayout();
}

ScarabNode* ScarabPanel::Root()
{
	return _root;
}

ScarabFormContext* ScarabPanel::FormContext()
{
	return _form->FormContext();
}

ScarabDrawContext* ScarabPanel::DrawContext()
{
	return _form->DrawContext();
}

ScarabForm* ScarabPanel::Form()
{
	return _form;
}

Xt::DomDb* ScarabPanel::Db()
{
	return _form->Db();
}

bool ScarabPanel::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!ChildWindow::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	cs->dwExStyle	|= WS_EX_CONTROLPARENT;

	return true;
}

LRESULT ScarabPanel::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
			if (ChildWindow::WndProc(msg, wParam, lParam) < 0)
				return -1;

			DirtyLayout();
		return 0;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(_hwnd, &ps);

			ExtTextOut(ps.hdc, 0, 0, ETO_OPAQUE, &ps.rcPaint, 0, 0, 0);

			_root->OnPaint(ps.hdc, ps.rcPaint);

			EndPaint(_hwnd, &ps);
		}
		return 0;

		case WM_NOTIFY:
		{
			NMHDR* hdr = (NMHDR*)lParam;

			if (NodeId* ni = _ids[hdr->idFrom])
				return ni->node->OnNotify(hdr);
		}
		break;

		case WM_COMMAND:
		{
			int id		= LOWORD(wParam);
			int code	= HIWORD(wParam);

			if (NodeId* ni = _ids[id])
				ni->node->OnCommand(id, code);
		}
		break;

		case WM_VSCROLL:
		case WM_HSCROLL:
			if (lParam != 0)
			{
				int id = GetDlgCtrlID((HWND)lParam);

				if (NodeId* ni = _ids[id])
					ni->node->OnScroll(id, msg, LOWORD(wParam), (HWND)lParam);
			}
		return 0;

		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLOREDIT:
			if (lParam != 0)
			{
				int id = GetDlgCtrlID((HWND)lParam);

				if (NodeId* ni = _ids[id])
				{
					if (HBRUSH brush = ni->node->OnControlColour(id, msg, (HDC)wParam, (HWND)lParam))
						return (LRESULT)(INT_PTR)brush;
				}
			}
		break;

		case WM_NCHITTEST:
		return HTCLIENT;

		case WM_MEASUREITEM:
			if (NodeId* ni = _ids[(int)wParam])
			{
				if (ni->node->MeasureItem((int)wParam, (MEASUREITEMSTRUCT*)lParam))
					return TRUE;
			}
		break;

		case WM_DRAWITEM:
			if (NodeId* ni = _ids[(int)wParam])
			{
				if (ni->node->DrawItem((int)wParam, (DRAWITEMSTRUCT*)lParam))
					return TRUE;
			}
		break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		{
			WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			SetFocus(*this);

			if (ScarabNode* node = _root->HitTestLabel(pt))
			{
				if (node->IsEnabled())
					node->OnLeftButton(pt);
			}
		}
		break;

		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
			SetFocus(*this);
		break;

		case WM_CONTEXTMENU:
		{
			WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			if (ScarabNode* node = _root->HitTestLabel(pt.ScreenToClient(*this)))
			{
				if (node->IsEnabled())
					node->OnContextMenu(pt);
			}
		}
		return 0;

		case WM_USER:
			if (_dirtyLayout)
			{
				LayoutNow();
			}
		return 0;
	}

	return ChildWindow::WndProc(msg, wParam, lParam);
}