#include "Core/XtWinPch.h"
#include "SequencerImpl.h"
#include "XtWin/Resource.h"

SequencerItem::SequencerItem() :
	_linkItem(this),
	_anchor(this),
	_sequencer(0),
	_parent(0),
	_isSelected(false),
	_linkSelection(this),
	_background(0xFFD0D0D0),
	_icon(0),
	_indent(17.0f),
	_height(34.0f),
	_gap(2.0f)
{
}

SequencerItem::~SequencerItem()
{
	if (_isSelected)
		Deselect();

	SetDirty(true);
}

void SequencerItem::OnClick()
{
}

void SequencerItem::SetTitle(const Sz16& title)
{
	if (_title != title)
	{
		_title = title;

		SetDirty(false);
	}
}

void SequencerItem::SetInfo(const Sz16& info)
{
	if (_info != info)
	{
		_info = info;

		SetDirty(false);
	}
}

void SequencerItem::SetBackground(XtU32 colour)
{
	colour |= 0xFF000000;

	if (_background != colour)
	{
		_background = colour;

		SetDirty(false);
	}
}

void SequencerItem::SetIcon(Gdiplus::Bitmap* icon)
{
	if (_icon != icon)
	{
		_icon = icon;

		SetDirty(false);
	}
}

void SequencerItem::SetIndent(float indent)
{
	if (_indent != indent)
	{
		_indent = indent;

		SetDirty(true);
	}
}

void SequencerItem::SetHeight(float height)
{
	if (_height != height)
	{
		_height = height;

		SetDirty(true);
	}
}

void SequencerItem::SetGap(float gap)
{
	if (_gap != gap)
	{
		_gap = gap;

		SetDirty(true);
	}
}

SequencerItem* SequencerItem::Add(SequencerItem* item)
{
	if (!item || (item->_parent != 0))
		return 0;

	item->_sequencer	= _sequencer;
	item->_parent		= this;

	XtUt::PushBack(&_items, &item->_linkItem);

	SetDirty(true);

	return item;
}

SequencerItem* SequencerItem::Insert(SequencerItem* item, SequencerItem* after)
{
	if (!item || ((item->_parent != 0) && (item->_parent != this)))
		return 0;

	item->_sequencer	= _sequencer;
	item->_parent		= this;

	if (after && (after->_parent == this))
		XtUt::AttachAfter(&item->_linkItem, &after->_linkItem);
	else
		XtUt::PushFront(&_items, &item->_linkItem);

	SetDirty(true);

	return item;
}

void SequencerItem::Clear()
{
	XtUt::Free(&_items);
}

void SequencerItem::Select()
{
	if (_isSelected || !_sequencer)
		return;

	_isSelected = true;

	XtUt::PushFront(&_sequencer->_selection, &_linkSelection);

	SetDirty(false);

	//_diagram->Graph()->Context()->SelectionChanged();
}

void SequencerItem::Deselect()
{
	if (!_isSelected || !_sequencer)
		return;

	_isSelected = false;

	XtUt::Detach(&_linkSelection);

	SetDirty(false);

	//_diagram->Graph()->Context()->SelectionChanged();
}

void SequencerItem::SetDirty(bool redoLayout)
{
	if (_sequencer)
		_sequencer->SetDirty(redoLayout);
}

void SequencerItem::DoLayout(SequencerLayoutContext* ctx)
{
	float indent = ctx->x;

	_location.pt = NgVector(indent, ctx->y);
	_location.sz = NgVector(ctx->width - indent, _height);

	ctx->x += _indent;
	ctx->y += _height + _gap;

	for(XtUt::LinkList<SequencerItem>::Iterator it(&_items); it; ++it)
		it->DoLayout(ctx);

	ctx->x = indent;
}

void SequencerItem::DoRender(Gdiplus::Graphics* g)
{
	if (_height > 0.0f)
	{
		Gdiplus::Pen pen(Gdiplus::Color::Black, 1.0f);
		Gdiplus::Pen penSelected(Gdiplus::Color::Red, 1.0f);
		Gdiplus::SolidBrush brush(Gdiplus::Color::Black);
		Gdiplus::Font font(Gdiplus::FontFamily::GenericSansSerif(), 9.0f, Gdiplus::FontStyleRegular);
		Gdiplus::Font boldFont(Gdiplus::FontFamily::GenericSansSerif(), 9.0f, Gdiplus::FontStyleBold);

		Gdiplus::StringFormat fmt;

		fmt.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
		fmt.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);

		Gdiplus::SolidBrush bg(Lighten(_background, 0.8f));

		g->FillRectangle(&bg, _location);
		g->DrawRectangle(_isSelected ? &penSelected : &pen, _location);

		if (_icon)
			g->DrawImage(_icon, _location.pt.vx + 2.0f, _location.pt.vy + ((_location.sz.vy - 32.0f) * 0.5f), 32.0f, 32.0f);

		NgRect clip = _location;

		clip.pt.vy += 2.0f;

		clip.pt.vx += 36.0f;
		clip.sz.vx -= 36.0f;

		g->DrawString(_title, _title.Length(), &boldFont, clip, &fmt, &brush);

		clip.pt.vy += 14.0f;

		g->DrawString(_info, _info.Length(), &font, clip, &fmt, &brush);
	}

	for(XtUt::LinkList<SequencerItem>::Iterator it(&_items); it; ++it)
		it->DoRender(g);
}

SequencerItem* SequencerItem::DoHitTest(const NgVector& pt)
{
	if (PtInRect(_location, pt))
		return this;

	for(XtUt::LinkList<SequencerItem>::Iterator it(&_items); it; ++it)
	{
		SequencerItem* item = it->DoHitTest(pt);

		if (item)
			return item;
	}

	return 0;
}