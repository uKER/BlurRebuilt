#include "Core/XtWinPch.h"
#include "FinderImpl.h"

FinderRow::FinderRow(FinderDlg* finder) :
	_finder(finder),
	_labelWnd(0),
	_labelWidth(0),
	_labelMargin(0),
	_enabled(false)
{
}

FinderRow::~FinderRow()
{
}

void FinderRow::BuildQuery(Xt::Query* query)
{
}

void FinderRow::BuildColumns(XtUt::List<FinderColumn>* columns)
{
}

void FinderRow::SetLabel(const XtString& label)
{
	_labelText = label + ":";

	if (_labelWnd)
		SetWindowText(_labelWnd, _labelText);

	WinSize sz;

	HDC ic = CreateIC(_T("DISPLAY"), 0, 0, 0);
	SelectObject(ic, GetStockObject(DEFAULT_GUI_FONT));
	GetTextExtentPoint32(ic, _labelText.Text(), (int)_labelText.Length(), &sz);
	DeleteDC(ic);

	_labelWidth = sz.cx + 20;

	_finder->DeferLayout();
}

void FinderRow::SetEnabled(bool enabled)
{
	if (_enabled == enabled)
		return;

	_enabled = enabled;

	Button_SetCheck(_labelWnd, _enabled ? BST_CHECKED : BST_UNCHECKED);

	OnEnableChanged();

	_finder->DeferLayout();
}

FinderControl* FinderRow::AddControl(FinderControl* control)
{
	if (!XtUt::AppendEx(&_controls, control))
		return 0;

	if (*this)
	{
		if (!control->Create())
		{
			// TODO: Don't leave a blank space?
		}
		else
		{
			if (_finder)
				_finder->DeferLayout();
		}
	}

	return control;
}

int FinderRow::Height()
{
	return 22;
}

void FinderRow::OnEnableChanged()
{
}

void FinderRow::OnFocus(FinderControl* control)
{
}

void FinderRow::OnChanged(FinderControl* control)
{
}

bool FinderRow::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!ChildWindow::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_CLIPCHILDREN;
	cs->dwExStyle	|= WS_EX_CONTROLPARENT;

	return true;
}

LRESULT FinderRow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
			if (ChildWindow::WndProc(msg, wParam, lParam) < 0)
				return -1;

			_labelWnd = CreateWindowEx(0, WC_BUTTON, _labelText, BS_AUTOCHECKBOX | BS_CHECKBOX | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 0, 0, *this, (HMENU)(intptr_t)kIdLabel, 0, 0);

			SendMessage(_labelWnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), FALSE);

			for(XtUt::List<FinderControl>::Iterator it(&_controls); it; ++it)
			{
				if (!it->Create())
				{
					// TODO: Don't leave blank space?
				}
			}

			if (_enabled)
				SendMessage(_labelWnd, BM_SETCHECK, BST_CHECKED, 0);
		}
		return 0;

		case WM_ERASEBKGND:
		{
			HDC			hdc			= (HDC)wParam;
			COLORREF	oldBkColour	= SetBkColor(hdc, GetSysColor(COLOR_3DFACE));

			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &WinRect(*this), 0, 0, 0);

			SetBkColor(hdc, oldBkColour);
		}
		return 0;

		case WM_COMMAND:
		{
			if ((LOWORD(wParam) == kIdLabel) && (HIWORD(wParam) == BN_CLICKED))
			{
				SetEnabled((int)SendMessage(_labelWnd, BM_GETCHECK, 0, 0) == BST_CHECKED);
				return 0;
			}

			int id		= LOWORD(wParam);
			int code	= HIWORD(wParam);

			if (ControlId* ni = _ids[id])
			{
				ni->control->OnCommand(id, code);
				return 0;
			}
		}
		break;
	}

	return ChildWindow::WndProc(msg, wParam, lParam);
}

void FinderRow::DoLayout()
{
	WinRect rc(*this);

	SetWindowPos(_labelWnd, 0, 0, 0, _labelMargin, 20, SWP_NOZORDER | SWP_NOACTIVATE);

	int totalWidth	= rc.Width() - _labelMargin;
	int minWidth	= 0;

	for(XtUt::List<FinderControl>::Iterator it(&_controls); it; ++it)
	{
		if (it.index > 0)
			minWidth += 4;

		it->_widthActual = it->_widthMin;

		minWidth += it->_widthActual;
	}

	int available = totalWidth - minWidth;

	for(int maxIterations = 32; (available > 0) && (maxIterations > 0); maxIterations--)
	{
		float	totalWeight = 0.0f;
		int		activeCount = 0;

		for(XtUt::List<FinderControl>::Iterator it(&_controls); it; ++it)
		{
			int flex = it->_widthMax - it->_widthActual;

			if (flex <= 0)
				continue;

			totalWeight += it->_widthWeight;

			activeCount++;
		}

		if (activeCount <= 0)
			break;

		float	weight	= 0.0f;
		int		used	= 0;

		for(XtUt::List<FinderControl>::Iterator it(&_controls); it; ++it)
		{
			int flex = it->_widthMax - it->_widthActual;

			if (flex <= 0)
				continue;

			int w0 = (int)(((weight / totalWeight) * available) + 0.5f);

			weight += it->_widthWeight;

			int w1 = (int)(((weight / totalWeight) * available) + 0.5f);

			int toAdd = Clamp(w1 - w0, 0, flex);

			it->_widthActual	+= toAdd;
			used				+= toAdd;

			if ((available - used) <= 0)
				break;
		}

		available -= used;
	}

	int x = 0;

	for(XtUt::List<FinderControl>::Iterator it(&_controls); it; ++it)
	{
		int left	= Clamp(x, 0, totalWidth);
		int right	= Clamp(x + it->_widthActual, 0, totalWidth);

		it->Layout(WinRect(_labelMargin + left, rc.top, _labelMargin + right, rc.bottom));

		x += it->_widthActual + 4;
	}
}

int FinderRow::AllocateControlId(FinderControl* control)
{
	for(int i = 0; i < 128; i++)
	{
		int id = 100 + (rand() % 25000);

		if (!Exists(&_ids, ControlId(id)))
			return Insert(&_ids, ControlId(id, control)) ? id : 0;
	}

	return 0;
}

void FinderRow::FreeControlId(int id)
{
	Remove(&_ids, FirstOf(&_ids, ControlId(id)), LastOf(&_ids, ControlId(id)));
}