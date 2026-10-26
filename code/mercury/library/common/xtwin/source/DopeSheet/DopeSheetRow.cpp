#include "Core/XtWinPch.h"
#include "DopeSheetImpl.h"

DsRow::DsRow() :
	_anchor(this),
	_diagram(0),
	_parent(0),
	_linkParent(this),
	_isSelected(false),
	_linkSelection(this),
	_indent(0.0f),
	_subRowCount(1),
	_colour(0xFFAABBDD),
	_height(24.0f),
	_isExpanded(false),
	_isBlocks(false)
{
}

DsRow::~DsRow()
{
	if (_isSelected)
		Deselect();

	SetDirty(true);
}

void DsRow::SetLabel(const Sz16& label)
{
	if (_label != label)
	{
		_label = label;

		SetDirty(true);
	}
}

void DsRow::SetColour(XtU32 colour)
{
	if ((colour & 0xFFFFFF) != (_colour & 0xFFFFFF))
	{
		_colour = (colour & 0xFFFFFF) | 0xFF000000;

		SetDirty(false);
	}
}

void DsRow::SetHeight(float height)
{
	if (_height != height)
	{
		_height = height;

		SetDirty(true);
	}
}

void DsRow::SetExpanded(bool expanded)
{
	if (expanded != _isExpanded)
	{
		_isExpanded = expanded;

		SetDirty(true);
	}
}

void DsRow::SetBlocks(bool blocks)
{
	if (blocks != _isBlocks)
	{
		_isBlocks = blocks;

		SetDirty(true);
	}
}

DsRow* DsRow::AddRow(DsRow* row)
{
	if (!row || (row->_parent != 0))
		return 0;

	// TODO: Check parent to prevent infinite recursion

	row->_diagram	= _diagram;
	row->_parent	= this;

	XtUt::PushBack(&_rows, &row->_linkParent);

	SetDirty(true);

	return row;
}

DsRow* DsRow::InsertRow(DsRow* row, DsRow* after)
{
	if (!row || (row->_parent != 0))
		return 0;

	// TODO: Check parent to prevent infinite recursion

	row->_diagram	= _diagram;
	row->_parent	= this;

	if (after && (after->_parent == this))
		XtUt::AttachAfter(&row->_linkParent, &after->_linkParent);
	else
		XtUt::PushFront(&_rows, &row->_linkParent);

	SetDirty(true);

	return row;
}

void DsRow::Detach()
{
	if (DsRow* parent = _parent)
	{
		XtUt::Detach(&_linkParent);

		_diagram	= 0;
		_parent		= 0;

		parent->SetDirty(true);
	}
}

void DsRow::Clear()
{
	Free(&_keys);
	Free(&_rows);
}

DsKey* DsRow::AddKey(DsKey* key)
{
	if (!key || (key->_row != 0))
		return 0;

	key->_row = this;

	XtUt::PushBack(&_keys, &key->_linkRow);

	SetDirty(true);

	return key;
}

DsKey* DsRow::InsertKey(DsKey* key, DsKey* after)
{
	if (!key || (key->_row != 0))
		return 0;

	key->_row = this;

	if (after && (after->_row == this))
		XtUt::AttachAfter(&key->_linkRow, &after->_linkRow);
	else
		XtUt::PushFront(&_keys, &key->_linkRow);

	SetDirty(true);

	return key;
}

bool DsRow::CanExpand() const
{
	return !IsEmpty(&_rows);
}

void DsRow::Select()
{
	if (_isSelected || !_diagram)
		return;

	_isSelected = true;

	XtUt::PushFront(&_diagram->_selectionRows, &_linkSelection);

	SetDirty(false);

	_diagram->DopeSheet()->Context()->SelectionChanged();
}

void DsRow::Deselect()
{
	if (!_isSelected || !_diagram)
		return;

	_isSelected = false;

	XtUt::Detach(&_linkSelection);

	SetDirty(false);

	_diagram->DopeSheet()->Context()->SelectionChanged();
}

DsHitResult DsRow::HitTest(const NgVector& pt)
{
	if (PtInRect(_hitRect, pt))
	{
		if (PtInRect(_expandRect, pt))
			return kDsHrRowExpand;

		if (pt.vx < _diagram->_splitterX)
			return kDsHrRowLabel;

		return kDsHrRowContent;
	}

	return kDsHrNone;
}

DsRow* DsRow::HitTestRow(const NgVector& pt)
{
	if (PtInRect(_hitRect, pt))
		return this;

	if (CanExpand() && IsExpanded())
	{
		if (PtInRect(_childRect, pt))
		{
			for(XtUt::LinkList<DsRow>::Iterator itRow(&_rows); itRow; ++itRow)
			{
				if (DsRow* row = itRow->HitTestRow(pt))
					return row;
			}
		}
	}

	return 0;
}

DsKey* DsRow::HitTestKey(const NgVector& pt)
{
	for(XtUt::LinkList<DsKey>::Iterator itKey(&_keys); itKey; ++itKey)
	{
		if (PtInRect(itKey->_hitRect, pt))
			return itKey;
	}

	return 0;
}

namespace
{
	bool CompareTime(const DsKey* lhs, const DsKey* rhs)
	{
		return lhs->Time() < rhs->Time();
	}
}

void DsRow::LayoutKeys()
{
	if (!_isBlocks)
	{
		for(XtUt::LinkList<DsKey>::Iterator itKey(&_keys); itKey; ++itKey)
			itKey->_subRow = 0;

		_subRowCount	= 1;
		_height			= 24.0f;
	}
	else
	{
		XtUt::Array<DsKey*> sorted;

		for(XtUt::LinkList<DsKey>::Iterator itKey(&_keys); itKey; ++itKey)
			AppendEx(&sorted, (DsKey*)itKey);

		XtUt::HeapSort(&sorted, CompareTime);

		XtUt::Array<DsKey*> subRows;

		if (!XtUt::Reserve(&subRows, 16))
			return;

		for(XtUt::Array<DsKey*>::Iterator itKey(&sorted); itKey; ++itKey)
			AssignSubRow(&subRows, itKey);

		_subRowCount	= Max(Count(&subRows), 1u);
		_height			= 4.0f + (_subRowCount * 20.0f) + ((_subRowCount - 1) * 4.0f);
	}
}

void DsRow::AssignSubRow(XtUt::Array<DsKey*>* subRows, DsKey* key)
{
	const float epsilon = 0.01f;

	float start = key->Time() - epsilon;

	for(XtUt::Array<DsKey*>::Iterator it(subRows); it; ++it)
	{
		float end = (*it)->Time() + Max((*it)->Duration(), epsilon);

		if (start < end)
			continue;

		*it = key;

		key->_subRow = it.index;

		return;
	}

	key->_subRow = (int)Count(subRows);

	AppendEx(subRows, key);
}

void DsRow::SetDirty(bool redoLayout)
{
	if (_parent)
		_parent->SetDirty(redoLayout);
	else if (_diagram)
		_diagram->SetDirty(redoLayout);
}