#include "Core/XtWinPch.h"
#include "TimelineImpl.h"

extern Gdiplus::Color LerpNoGamma(Gdiplus::Color colour0, Gdiplus::Color colour1, float a);

struct TlRenderContext
{
	Gdiplus::Graphics*	g;
	NgVector			clientSize;

	Gdiplus::SolidBrush	brushBlack;
	Gdiplus::SolidBrush	brushWhite;
	Gdiplus::Pen		penBlack;
	Gdiplus::Pen		penWhite;

public:
	TlRenderContext(Gdiplus::Graphics* g_);
};

TlRenderContext::TlRenderContext(Gdiplus::Graphics* g_) :
	g(g_),
	brushBlack(Gdiplus::Color::Black),
	brushWhite(Gdiplus::Color::White),
	penBlack(Gdiplus::Color::Black),
	penWhite(Gdiplus::Color::White)
{
}

TlDiagram::TlDiagram(TimelineWnd* timeline) :
	_timeline(timeline),
	_headerY(16.0f),
	_splitterX(160.0f),
	_zoomX(100.0f),
	_scrollX(0.0f),
	_scrollY(0.0f),
	_rangeLow(0.0f),
	_rangeHigh(1.0f),
	_quantise(false),
	_quantisation(1.0f),
	_moveKeysOffset(0.0f),
	_redoLayout(true),
	_totalHeight(0.0f),
	_rectSelectionActive(false)
{
}

TlDiagram::~TlDiagram()
{
	ClearAll();
}

TlRow* TlDiagram::AddRow(TlRow* row)
{
	if (!row || (row->_diagram != 0))
		return 0;

	row->_diagram = this;

	XtUt::PushBack(&_rows, &row->_linkDiagram);

	SetDirty(true);

	return row;
}

TlRow* TlDiagram::InsertRow(TlRow* row, TlRow* after)
{
	if (!row || (row->_diagram != 0))
		return 0;

	row->_diagram = this;

	if (after && (after->_diagram == this))
		XtUt::AttachAfter(&row->_linkDiagram, &after->_linkDiagram);
	else
		XtUt::PushFront(&_rows, &row->_linkDiagram);

	SetDirty(true);

	return row;
}

void TlDiagram::ClearRows()
{
	Abandon(&_selectionRows);
	Abandon(&_selectionKeys);

	Free(&_rows);
}

TlGroup* TlDiagram::AddGroup(TlGroup* group)
{
	if (!group || (group->_diagram != 0))
		return 0;

	group->_diagram = this;

	XtUt::PushBack(&_groups, &group->_linkDiagram);

	SetDirty(true);

	return group;
}

TlGroup* TlDiagram::InsertGroup(TlGroup* group, TlGroup* after)
{
	if (!group || (group->_diagram != 0))
		return 0;

	group->_diagram = this;

	if (after && (after->_diagram == this))
		XtUt::AttachAfter(&group->_linkDiagram, &after->_linkDiagram);
	else
		XtUt::PushFront(&_groups, &group->_linkDiagram);

	SetDirty(true);

	return group;
}

void TlDiagram::ClearGroups()
{
	Free(&_groups);
}

void TlDiagram::ClearAll()
{
	ClearRows();
	ClearGroups();
}

void TlDiagram::SetClientRect(const NgRect& client)
{
	_client = client;
}

void TlDiagram::Layout()
{
	if (_redoLayout)
	{
		_redoLayout = false;

		float rowY = 0.0f;

		for(XtUt::LinkList<TlGroup>::Iterator itGroup(&_groups); itGroup; ++itGroup)
		{
			itGroup->_y = rowY;

			rowY += itGroup->_height;

			for(XtUt::LinkList<TlRow>::Iterator itRow(itGroup->Rows()); itRow; ++itRow)
			{
				itRow->_y = rowY;

				rowY += itRow->_height;
			}
		}

		_totalHeight = rowY;
	}

	_scrollY = Clamp(_scrollY, 0.0f, Max(_totalHeight - (_client.sz.vy - _headerY), 0.0f));

	_timeline->DoScrollBars();
	_timeline->DoScrollBarPos();
}

float TlRoundMarkGap(float v)
{
	if (v < 0.05f)			return TlRoundMarkGap(v * 100.0f) / 100.0f;
	else if (v < 0.25f)		return 0.1f;
	else if (v < 0.75f)		return 0.5f;
	else if (v < 2.5f)		return 1.0f;
	else if (v < 7.5f)		return 5.0f;
	else if (v < 25.0f)		return 10.0f;
	else if (v < 75.0f)		return 50.0f;
	else if (v < 250.0f)	return 100.0f;
	else if (v < 750.0f)	return 500.0f;
	else					return TlRoundMarkGap(v / 100.0f) * 100.0f;
}

void TlDiagram::Render(Gdiplus::Graphics* g)
{
	TlRenderContext ctx(g);

	ctx.clientSize = _client.sz;

	Layout();

	// Draw background

	{
		Gdiplus::GraphicsContainer gc = g->BeginContainer();

		g->SetClip(Gdiplus::RectF(0.0f, _headerY, _client.sz.vx, _client.sz.vy - _headerY));

		g->TranslateTransform(0.0f, floor(_headerY - _scrollY));

		for(XtUt::LinkList<TlGroup>::Iterator itGroup(&_groups); itGroup; ++itGroup)
		{
			ctx.g->DrawLine(&ctx.penBlack, 0.0f, itGroup->_y, ctx.clientSize.vx, itGroup->_y);

			for(XtUt::LinkList<TlRow>::Iterator itRow(itGroup->Rows()); itRow; ++itRow)
			{
				ctx.g->DrawLine(&ctx.penBlack, 0.0f, itRow->_y, ctx.clientSize.vx, itRow->_y);
			}
		}

		g->EndContainer(gc);
	}

	// Draw header

	{
		Gdiplus::GraphicsContainer gc = g->BeginContainer();

		g->SetClip(Gdiplus::RectF(_splitterX, 0.0f, _client.sz.vx - _splitterX, _client.sz.vx));

		Gdiplus::SolidBrush brush(0xFFBBBBBB);//0xFF8090C0);

		g->FillRectangle(&brush, Gdiplus::RectF(0.0f, 0.0f, _client.sz.vx, _headerY));

		// Ruler

		Gdiplus::Font fontNormal(Gdiplus::FontFamily::GenericSansSerif(), 9.0f, Gdiplus::FontStyleRegular);
		Gdiplus::SolidBrush brushBlack(Gdiplus::Color::Black);
		Gdiplus::SolidBrush brushOutside(0x20000000);
		Gdiplus::SolidBrush brushBottom(0x80000000);
		Gdiplus::Pen penGrid(0x20000000);
		Gdiplus::Pen penSubGrid(0x08000000);

		float width = _client.sz.vx - _splitterX;

		float lowTime	= (-_scrollX) / _zoomX;
		float highTime	= (width - _scrollX) / _zoomX;

		float markGap = TlRoundMarkGap(width / (_zoomX * 10.0f));

		float	lowMark		= floor(lowTime / markGap) * markGap;
		float	highMark	= floor(highTime / markGap) * markGap;
		int		marks		= 1 + (int)((highMark - lowMark + (markGap * 0.5f)) / markGap);
		float	offset		= (lowMark - lowTime) * _zoomX;

		float startX = _scrollX + (_rangeLow * _zoomX);
		float endX = _scrollX + (_rangeHigh * _zoomX);
		float bottomY = _headerY + _totalHeight - _scrollY;

		g->FillRectangle(&brushOutside, _splitterX, 0.0f, startX, _client.sz.vy);
		g->FillRectangle(&brushOutside, _splitterX + endX, 0.0f, _client.sz.vx - (_splitterX + endX), _client.sz.vy);
		g->FillRectangle(&brushBottom, 0.0f, bottomY, _client.sz.vx, _client.sz.vy - bottomY);

		for(int i = 0; i <= marks; i++)
		{
			float x		= _splitterX + offset + (markGap * _zoomX * i);
			float time	= lowMark + (markGap * i);

			Sz16 label = Format(L"%.1f", time);

			g->DrawString(label, label.Length(), &fontNormal, NgVector(x, 0.0f), &brushBlack);

			if (x >= _splitterX)
				g->DrawLine(&penGrid, x, 0.0f, x, _client.sz.vy);

			for(int j = 1; j < 4; j++)
			{
				float sx = x + (markGap * _zoomX * j) / 4.0f;

				if (sx >= _splitterX)
					g->DrawLine(&penSubGrid, sx, 0.0f, sx, _client.sz.vy);
			}
		}

		g->EndContainer(gc);
	}

	// Draw rows

	{
		Gdiplus::GraphicsContainer gc = g->BeginContainer();
		{
			g->SetClip(Gdiplus::RectF(_splitterX + 1, _headerY, _client.sz.vx - (_splitterX + 1), _client.sz.vy - _headerY));

			g->TranslateTransform(floor(_scrollX), floor(_headerY - _scrollY));

			//DoRenderRow(&ctx, &_root);

			// Selection

			if (_rectSelectionActive)
			{
				Gdiplus::Pen selected(0xB0FFA500, 2.0f);
				Gdiplus::Pen selectedAlt(0xB0FF0000, 2.0f);

				float pattern[] = { 2.0f, 2.0f };

				selected.SetDashPattern(pattern, ArrayCount(pattern));
				selectedAlt.SetDashPattern(pattern, ArrayCount(pattern));

				selectedAlt.SetDashOffset(2.0f);

				NgRect rect = RectSelection();

				rect.pt += NgVector(_splitterX - 1.0f, -1.0f);
				rect.sz += NgVector(2.0f);

				g->DrawRectangle(&selected, rect);
				g->DrawRectangle(&selectedAlt, rect);
			}
		}
		g->EndContainer(gc);
	}

	// Draw labels

	{
		Gdiplus::GraphicsContainer gc = g->BeginContainer();
		{
			g->SetClip(Gdiplus::RectF(0.0f, _headerY, _splitterX, _client.sz.vy - _headerY));

			g->TranslateTransform(0.0f, floor(_headerY - _scrollY));

			//DoRenderLabel(&ctx, &_root, 0.0f);
		}
		g->EndContainer(gc);

		{
			Gdiplus::SolidBrush brushBottom(Gdiplus::Color::LightGray);

			float bottomY = _headerY + _totalHeight - _scrollY;

			g->FillRectangle(&brushBottom, -1.0f, bottomY, _splitterX + 1.0f, _client.sz.vy - bottomY);
		}
	}

	// Draw Corner

	{
		Gdiplus::SolidBrush brush(Gdiplus::Color::Gray);

		g->FillRectangle(&brush, Gdiplus::RectF(-1.0f, -1.0f, _splitterX + 1.0f, _headerY + 1.0f));
	}
}

void TlDiagram::SetScrollY(float y)
{
	_scrollY = y;

	InvalidateWindow(*Timeline());
}

void TlDiagram::Pan(const NgVector& delta)
{
	_scrollX += delta.vx;
	_scrollY = Clamp(_scrollY - delta.vy, 0.0f, Max(_totalHeight - (_client.sz.vy - _headerY), 0.0f));

	SetDirty(true);

	_timeline->DoScrollBarPos();
}

void TlDiagram::Zoom(const NgVector& pt, float factor)
{
	NgVector ptClient = ToContentArea(pt) / NgVector(_zoomX, 1.0f);

	_zoomX = Clamp(_zoomX * factor, 5.0f, 2000.0f);

	NgVector ptPost = NgVector((ptClient.vx * _zoomX) + _splitterX + _scrollX, ptClient.vy + _headerY - _scrollY);

	Pan(pt - ptPost);
}

void TlDiagram::SetRange(float low, float high)
{
	_rangeLow	= low;
	_rangeHigh	= high;

	SetDirty(false);
}

void TlDiagram::SetQuantisation(bool quantise, float q)
{
	if (quantise)
	{
		_quantise		= true;
		_quantisation	= fabsf(q);

		if (_quantisation < 0.0001f)
			_quantisation = 0.0001f;
	}
	else
	{
		_quantise		= false;
		_quantisation	= 1.0f;
	}
}

void TlDiagram::BeginMoveKeys()
{
	_moveKeysOffset = 0.0f;
}

void TlDiagram::MoveSelectedKeysBy(const NgVector& offset)
{
	if (_quantise)
	{
		_moveKeysOffset += offset.vx;

		NgVector newOffset(0.0f, offset.vy);

		if (_moveKeysOffset > 0.0f)
		{
			while(_moveKeysOffset > (_quantisation * 0.5f))
			{
				newOffset.vx	+= _quantisation;
				_moveKeysOffset	-= _quantisation;
			}
		}
		else
		{
			while(_moveKeysOffset < -(_quantisation * 0.5f))
			{
				newOffset.vx	-= _quantisation;
				_moveKeysOffset	+= _quantisation;
			}
		}

		for(XtUt::LinkList<TlKey>::Iterator it(&_selectionKeys); it; ++it)
			it->MoveBy(newOffset);
	}
	else
	{
		for(XtUt::LinkList<TlKey>::Iterator it(&_selectionKeys); it; ++it)
			it->MoveBy(offset);
	}
}

void TlDiagram::EndMoveKeys()
{
	_moveKeysOffset = 0.0f;
}

TlHitResult TlDiagram::HitTest(const NgVector& pt)
{
	if (pt.vy < _headerY)
		return TlHitResult(kTlHitHeader, 0, 0);

	if (pt.vx < _splitterX)
		return TlHitResult(kTlHitRowLabel, 0, 0);

	return TlHitResult(kTlHitRowContent, 0, 0);
}

void TlDiagram::ClearSelection()
{
	while(TlRow* row = _selectionRows.elements.next->object)
		row->Deselect();

	while(TlKey* key = _selectionKeys.elements.next->object)
		key->Deselect();
}

void TlDiagram::Select(TlRow* row, TlSelectMode mode)
{
	if (KeySelectionCount() > 0)
		ClearSelection();

	switch(mode)
	{
		case kTlSelectSingle:
			if (_selectionRows.elements.next->object && (_selectionRows.elements.next->object != row))
				ClearSelection();

			if (row)
				row->Select();
		break;

		case kTlSelectAdd:
			if (row)
				row->Select();
		break;

		case kTlSelectRemove:
			if (row)
				row->Deselect();
		break;

		case kTlSelectToggle:
			if (row)
			{
				if (row->IsSelected())
					row->Deselect();
				else
					row->Select();
			}
		break;
	}
}

void TlDiagram::Select(TlKey* key, TlSelectMode mode)
{
	if (RowSelectionCount() > 0)
		ClearSelection();

	switch(mode)
	{
		case kTlSelectSingle:
			if (_selectionKeys.elements.next->object && (_selectionKeys.elements.next->object != key))
				ClearSelection();

			if (key)
				key->Select();
		break;

		case kTlSelectAdd:
			if (key)
				key->Select();
		break;

		case kTlSelectRemove:
			if (key)
				key->Deselect();
		break;

		case kTlSelectToggle:
			if (key)
			{
				if (key->IsSelected())
					key->Deselect();
				else
					key->Select();
			}
		break;
	}
}

void TlSelectRectShadow(TlRow* parent, TlRow* row, const NgRect& rect, XtUt::Array<TlKey*>* toSelect)
{
	/*for(XtUt::LinkList<TlKey>::ReverseIterator itKey = row->ReverseKeys(); itKey; ++itKey)
	{
		NgRect keyRect		= itKey->HitRect();
		NgRect parentRect	= parent->HitRect();

		keyRect.pt.vx += 2.0f;
		keyRect.sz.vx -= 4.0f;

		keyRect.pt.vy = parentRect.pt.vy + 7.0f;
		keyRect.sz.vy = parentRect.sz.vy - 14.0f;

		if (Intersects(rect, keyRect))
			XtUt::AppendEx(toSelect, (TlKey*)itKey);
	}

	for(XtUt::LinkList<TlRow>::Iterator itRow = row->Rows(); itRow; ++itRow)
		TlSelectRectShadow(parent, itRow, rect, toSelect);*/
}

void TlSelectRectSub(TlRow* row, const NgRect& rect, XtUt::Array<TlKey*>* toSelect)
{
	/*for(XtUt::LinkList<TlKey>::ReverseIterator itKey = row->ReverseKeys(); itKey; ++itKey)
	{
		const NgRect& l = itKey->HitRect();

		if (Intersects(rect, l))
			XtUt::AppendEx(toSelect, (TlKey*)itKey);
	}

	if (row->CanExpand() && row->IsExpanded())
	{
		for(XtUt::LinkList<TlRow>::Iterator itRow = row->Rows(); itRow; ++itRow)
			TlSelectRectSub(itRow, rect, toSelect);
	}

	for(XtUt::LinkList<TlRow>::Iterator itRow = row->Rows(); itRow; ++itRow)
		TlSelectRectShadow(row, itRow, rect, toSelect);*/
}

void TlDiagram::SelectRect(const NgRect& rect, TlSelectMode mode)
{
	/*if (RowSelectionCount() > 0)
		ClearSelection();

	Layout();

	XtUt::Array<TlKey*> toSelect;

	TlSelectRectSub(&_root, rect, &toSelect);

	for(XtUt::Array<TlKey*>::Iterator it(&toSelect); it; ++it)
		Select(it, mode);*/
}

int TlDiagram::RowSelectionCount() const
{
	int count = 0;

	for(XtUt::LinkList<TlRow>::ConstIterator it(&_selectionRows); it; ++it)
		count++;

	return count;
}

int TlDiagram::KeySelectionCount() const
{
	int count = 0;

	for(XtUt::LinkList<TlKey>::ConstIterator it(&_selectionKeys); it; ++it)
		count++;

	return count;
}

void TlDiagram::RectSelectionBegin(const NgVector& pt)
{
	_rectSelectionActive	= true;
	_rectSelectionStart		= pt;
	_rectSelectionEnd		= pt;

	SetDirty(false);
}

void TlDiagram::RectSelectionUpdate(const NgVector& pt)
{
	if (_rectSelectionActive)
	{
		_rectSelectionEnd = pt;

		SetDirty(false);
	}
}

void TlDiagram::RectSelectionEnd()
{
	if (_rectSelectionActive)
	{
		_rectSelectionActive = false;

		SetDirty(false);
	}
}

NgVector TlDiagram::ToRowArea(const NgVector& pt)
{
	return NgVector(pt.vx, pt.vy - _headerY + _scrollY);
}

NgVector TlDiagram::ToContentArea(const NgVector& pt)
{
	return NgVector(pt.vx - _splitterX - _scrollX, pt.vy - _headerY + _scrollY);
}

void TlDiagram::DoRowLayout(TlRow* row, float indent, float& rowY, float width)
{
	/*const float rowIndent = 12.0f;

	row->_indent = indent;

	row->_hitRect.pt.vx = 0.0f;
	row->_hitRect.pt.vy = rowY;
	row->_hitRect.sz.vx = width;
	row->_hitRect.sz.vy = row->Height();

	row->_expandRect.pt.vx = indent;
	row->_expandRect.pt.vy = rowY;
	row->_expandRect.sz.vx = 20.0f;
	row->_expandRect.sz.vy = row->Height();

	rowY += row->Height();

	row->_childRect.pt.vx = 0.0f;
	row->_childRect.pt.vy = rowY;

	if (row->CanExpand() && row->IsExpanded())
	{
		for(XtUt::LinkList<TlRow>::Iterator itRow = row->Rows(); itRow; ++itRow)
			DoRowLayout(itRow, indent + rowIndent, rowY, width);
	}
	else
	{
		float shadowY = rowY;

		for(XtUt::LinkList<TlRow>::Iterator itRow = row->Rows(); itRow; ++itRow)
			DoRowLayout(itRow, indent + rowIndent, shadowY, width);
	}

	row->_childRect.sz.vx = width;
	row->_childRect.sz.vy = rowY - row->_childRect.pt.vy;

	for(XtUt::LinkList<TlKey>::Iterator itKey = row->Keys(); itKey; ++itKey)
	{
		float x = floor(itKey->Time() * _zoomX);

		itKey->_hitRect.pt.vx = x - 4.0f;
		itKey->_hitRect.pt.vy = row->_hitRect.pt.vy + 5.0f;
		itKey->_hitRect.sz.vx = 8;
		itKey->_hitRect.sz.vy = row->_hitRect.sz.vy - 10.0f;
	}*/
}

void TlDiagram::DoRenderBackground(TlRenderContext* ctx, TlRow* row)
{
	/*Gdiplus::Pen penBlack(Gdiplus::Color::Black);

	for(XtUt::LinkList<TlRow>::Iterator itRow = row->Rows(); itRow; ++itRow)
	{
		float h			= itRow->_hitRect.sz.vy;
		float y0		= itRow->_hitRect.pt.vy;
		float y1		= y0 + h;
		float centreY	= (y0 + y1) * 0.5f;

		// Background

		Gdiplus::Color colour = itRow->Colour();

		Gdiplus::SolidBrush brushLabel(LerpNoGamma(colour, Gdiplus::Color::White, 0.5f));
		Gdiplus::SolidBrush brushRow(colour);

		ctx->g->FillRectangle(&brushLabel, 0.0f, y0, _splitterX, itRow->Height());
		ctx->g->FillRectangle(&brushRow, _splitterX, y0, ctx->clientSize.vx - _splitterX, itRow->Height());

		if (itRow->IsSelected())
		{
			Gdiplus::Pen penSelection(Gdiplus::Color::Black);

			penSelection.SetDashStyle(Gdiplus::DashStyleDot);

			ctx->g->DrawRectangle(&penSelection, 2.0f, y0 + 2.0f, _splitterX - 4.0f, itRow->Height() - 4.0f);
		}

		// Horizontal lines

		ctx->g->DrawLine(&penBlack, 0.0f, y0, ctx->clientSize.vx, y0);
		ctx->g->DrawLine(&penBlack, 0.0f, y1, ctx->clientSize.vx, y1);

		// Vertical lines

		ctx->g->DrawLine(&penBlack, 0.0f, y0, 0.0f, y1);
		ctx->g->DrawLine(&penBlack, _splitterX, y0, _splitterX, y1);

		if (itRow->CanExpand() && itRow->IsExpanded())
			DoRenderBackground(ctx, itRow);
	}*/
}

void TlDiagram::DoRenderLabel(TlRenderContext* ctx, TlRow* row, float oldBoxY)
{
	/*Gdiplus::SolidBrush brushWhite(Gdiplus::Color::White);
	Gdiplus::SolidBrush brushBlack(Gdiplus::Color::Black);
	Gdiplus::Pen penBlack(Gdiplus::Color::Black);
	Gdiplus::Pen penGrey(0xFF404040);
	Gdiplus::Pen penDot(Gdiplus::Color::Gray);

	Gdiplus::Font fontNormal(Gdiplus::FontFamily::GenericSansSerif(), 9.0f, Gdiplus::FontStyleRegular);
	Gdiplus::Font fontSelected(Gdiplus::FontFamily::GenericSansSerif(), 9.0f, Gdiplus::FontStyleBold);
	Gdiplus::StringFormat textCentre;

	textCentre.SetLineAlignment(Gdiplus::StringAlignmentCenter);

	float pattern[] = { 2.0f, 1.0f };

	penDot.SetDashPattern(pattern, ArrayCount(pattern));

	float rowIndent		= 12.0f;
	float boxIndent		= 8.0f;

	for(XtUt::LinkList<TlRow>::Iterator itRow = row->Rows(); itRow; ++itRow)
	{
		float x = itRow->_indent;
		float h = itRow->_hitRect.sz.vy;
		float y0 = itRow->_hitRect.pt.vy;
		float y1 = y0 + h;

		float boxSize = itRow->CanExpand() ? 4.0f : 0.0f;

		// Draw dotted line down to top of expander box

		float boxX = x + boxIndent;
		float boxY = (y0 + y1) * 0.5f;

		ctx->g->DrawLine(&penDot, boxX + boxSize, boxY, x + boxIndent + rowIndent, boxY);

		// Draw expander icon

		if (itRow->CanExpand())
		{
			ctx->g->FillRectangle(&brushWhite, boxX - boxSize, boxY - boxSize, boxSize * 2.0f, boxSize * 2.0f);
			ctx->g->DrawRectangle(&penGrey, boxX - boxSize, boxY - boxSize, boxSize * 2.0f, boxSize * 2.0f);

			ctx->g->DrawLine(&penBlack, boxX - boxSize + 2.0f, boxY, boxX + boxSize - 2.0f, boxY);

			if (!itRow->IsExpanded())
				ctx->g->DrawLine(&penBlack, boxX, boxY - boxSize + 2.0f, boxX, boxY + boxSize - 2.0f);
		}

		// Draw label

		const Sz16& label = itRow->Label();

		ctx->g->DrawString(label, label.Length(),
				itRow->IsSelected() ? &fontSelected : &fontNormal,
			NgVector(x + boxIndent + rowIndent + 2.0f, boxY), &textCentre, &brushBlack);

		// Draw children

		if (itRow->CanExpand() && itRow->IsExpanded())
			DoRenderLabel(ctx, itRow, boxY);

		// Draw dotted line down past children

		ctx->g->DrawLine(&penDot, x + boxIndent, oldBoxY, x + boxIndent, boxY - boxSize);

		oldBoxY = boxY + boxSize;
	}*/
}

void TlDiagram::DoRenderRowShadow(TlRenderContext* ctx, TlRow* parent, TlRow* row, bool selected)
{
	/*if (parent != row)
	{
		Gdiplus::Color colour = LerpNoGamma(parent->Colour(), selected ? Gdiplus::Color::White : Gdiplus::Color::Black, 0.4f);
		Gdiplus::Color colour2 = LerpNoGamma(colour, Gdiplus::Color::Black, 0.5f);

		colour.SetValue((colour.GetValue() & 0xFFFFFF) | 0x80000000);
		colour2.SetValue((colour2.GetValue() & 0xFFFFFF) | 0x80000000);

		Gdiplus::SolidBrush brush(colour);
		Gdiplus::Pen pen(colour2);

		float h				= parent->_hitRect.sz.vy;
		float y0			= parent->_hitRect.pt.vy;
		float y1			= y0 + h;
		float centreY		= floor((y0 + y1) * 0.5f);
		float outerSizeX	= 2.0f;
		float outerSizeY	= 4.0f;

		for(XtUt::LinkList<TlKey>::ReverseIterator itKey(&row->_keys); itKey; ++itKey)
		{
			if (itKey->IsSelected() != selected)
				continue;

			float x = floor(_splitterX + itKey->Time() * _zoomX);

			Gdiplus::PointF points[] =
			{
				Gdiplus::PointF(x - outerSizeX, centreY - outerSizeY),
				Gdiplus::PointF(x - outerSizeX, centreY + outerSizeY),
				Gdiplus::PointF(x + outerSizeX, centreY + outerSizeY),
				Gdiplus::PointF(x + outerSizeX, centreY - outerSizeY)
			};

			ctx->g->FillPolygon(&brush, points, 4);
			ctx->g->DrawPolygon(&pen, points, 4);
		}
	}

	for(XtUt::LinkList<TlRow>::Iterator itRow(&row->_rows); itRow; ++itRow)
		DoRenderRowShadow(ctx, parent, itRow, selected);*/
}

void TlDiagram::DoRenderRow(TlRenderContext* ctx, TlRow* row)
{
	/*Gdiplus::SolidBrush brushBlack(Gdiplus::Color::Black);
	Gdiplus::SolidBrush brushWhite(Gdiplus::Color::White);
	Gdiplus::Pen penBlack(Gdiplus::Color::Black);

	for(XtUt::LinkList<TlRow>::Iterator itRow(&row->_rows); itRow; ++itRow)
	{
		float h			= itRow->_hitRect.sz.vy;
		float y0		= itRow->_hitRect.pt.vy;
		float y1		= y0 + h;
		float centreY	= floor((y0 + y1) * 0.5f);

		DoRenderRowShadow(ctx, itRow, itRow, false);
		DoRenderRowShadow(ctx, itRow, itRow, true);

		for(XtUt::LinkList<TlKey>::ReverseIterator itKey(&itRow->_keys); itKey; ++itKey)
		{
			float x = floor(_splitterX + itKey->Time() * _zoomX) + 0.5f;

			float outerSizeX = 3.0f;
			float outerSizeY = 6.0f;

			Gdiplus::PointF points[] =
			{
				Gdiplus::PointF(x - outerSizeX, centreY - outerSizeY),
				Gdiplus::PointF(x - outerSizeX, centreY + outerSizeY),
				Gdiplus::PointF(x + outerSizeX, centreY + outerSizeY),
				Gdiplus::PointF(x + outerSizeX, centreY - outerSizeY)
			};

			ctx->g->FillPolygon(&brushWhite, points, 4);
			ctx->g->DrawPolygon(&penBlack, points, 4);

			if (itKey->IsSelected())
			{
				float selSizeX = 2.0f;
				float selSizeY = 3.0f;

				Gdiplus::PointF selectedPoints[] =
				{
					Gdiplus::PointF(x - selSizeX, centreY - selSizeY),
					Gdiplus::PointF(x - selSizeX, centreY + selSizeY + 1),
					Gdiplus::PointF(x + selSizeX - 1, centreY + selSizeY + 1),
					Gdiplus::PointF(x + selSizeX - 1, centreY - selSizeY)
				};

				ctx->g->FillPolygon(&brushBlack, selectedPoints, 4);
			}
		}

		if (itRow->CanExpand() && itRow->IsExpanded())
			DoRenderRow(ctx, itRow);
	}*/
}

void TlDiagram::SetDirty(bool redoLayout)
{
	_redoLayout = _redoLayout || redoLayout;

	InvalidateWindow(*Timeline());
}