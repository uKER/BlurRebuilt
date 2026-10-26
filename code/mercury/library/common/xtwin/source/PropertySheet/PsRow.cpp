#include "Core/XtWinPch.h"
#include "PropertySheetImpl.h"

const int kPsRow_MinRowHeight = 20;

PsRow::PsRow() :
	_anchor(this),
	_diagram(0),
	_parent(0),
	_linkParent(this),
	_expanded(false),
	_y(0),
	_height(kPsRow_MinRowHeight),
	_indent(0),
	_bottom(0)
{
}

PsRow::~PsRow()
{
}

void PsRow::SetLabel(const Sz16& label)
{
	if (label != _label)
	{
		_label = label;

		SetDirty(false);
	}
}

void PsRow::SetHeight(int height)
{
	height = Max(height, kPsRow_MinRowHeight);

	if (height != _height)
	{
		_height = height;

		SetDirty(true);
	}
}

void PsRow::SetExpanded(bool expanded)
{
	if (expanded != _expanded)
	{
		_expanded = expanded;

		SetDirty(true);
	}
}

void PsRow::Detach()
{
	if (PsRow* parent = _parent)
	{
		XtUt::Detach(&_linkParent);

		_diagram	= 0;
		_parent		= 0;

		parent->SetDirty(true);
	}
}

PsRow* PsRow::AddRow(PsRow* row)
{
	if (!row || (row->_diagram != 0))
		return 0;

	row->_parent	= this;
	row->_diagram	= _diagram;

	XtUt::PushBack(&_rows, &row->_linkParent);

	row->SetDirty(true);

	return row;
}

PsRow* PsRow::InsertRow(PsRow* row, PsRow* after)
{
	if (!row || (row->_diagram != 0))
		return 0;

	row->_parent	= this;
	row->_diagram	= _diagram;

	if (after && (after->_parent == this))
		XtUt::AttachAfter(&row->_linkParent, &after->_linkParent);
	else
		XtUt::PushFront(&_rows, &row->_linkParent);

	row->SetDirty(true);

	return row;
}

void PsRow::ClearRows()
{
	Free(&_rows);
}

PsControl* PsRow::AddControl(PsControl* control)
{
	if (!control || (control->_row != 0))
		return 0;

	control->_row = this;

	XtUt::PushBack(&_controls, &control->_linkRow);

	SetDirty(true);

	return control;
}

PsControl* PsRow::InsertControl(PsControl* control, PsControl* after)
{
	if (!control || (control->_row != 0))
		return 0;

	control->_row = this;

	if (after && (after->_row == this))
		XtUt::AttachAfter(&control->_linkRow, &after->_linkRow);
	else
		XtUt::PushFront(&_controls, &control->_linkRow);

	SetDirty(true);

	return control;
}

void PsRow::ClearControls()
{
	Free(&_controls);
}

void PsRow::DrawLabel(Gdiplus::Graphics* g, PropertySheetDrawContext* dc, const XwRect& rect)
{
	Gdiplus::StringFormat sf;

	sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
	sf.SetTrimming(Gdiplus::StringTrimmingNone);
	sf.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);

	g->DrawString(_label.Text(), _label.Length(), &dc->labelFont, rect, &sf, &dc->labelBrush);
}

void PsRow::DrawContent(Gdiplus::Graphics* g, PropertySheetDrawContext* dc, const XwRect& rect)
{
	for(ControlIterator it(&_controls); it; ++it)
	{
		if (it->_width <= 0)
			continue;

		XwRect controlRect;

		controlRect.pt.x = rect.pt.x + it->_x;
		controlRect.pt.y = rect.pt.y + 1;
		controlRect.sz.x = it->_width;
		controlRect.sz.y = rect.sz.y - 3;

		it->Draw(g, dc, controlRect);
	}
}

bool PsRow::CanExpand()
{
	return !IsEmpty(&_rows);
}

void PsRow::SetDirty(bool performLayout)
{
	if (_parent)
		_parent->SetDirty(performLayout);
	else if (_diagram)
		_diagram->SetDirty(performLayout);
}