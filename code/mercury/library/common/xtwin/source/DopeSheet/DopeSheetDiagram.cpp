#include "Core/XtWinPch.h"
#include "DopeSheetImpl.h"

Gdiplus::Color LerpNoGamma(Gdiplus::Color colour0, Gdiplus::Color colour1, float a)
{
	return Gdiplus::Color(
			Clamp(Lerp((int)colour0.GetA(), (int)colour1.GetA(), a), 0, 255),
			Clamp(Lerp((int)colour0.GetR(), (int)colour1.GetR(), a), 0, 255),
			Clamp(Lerp((int)colour0.GetG(), (int)colour1.GetG(), a), 0, 255),
			Clamp(Lerp((int)colour0.GetB(), (int)colour1.GetB(), a), 0, 255)
		);
}

struct DsRenderContext
{
	Gdiplus::Graphics*	g;
	NgVector			clientSize;

public:
	DsRenderContext(Gdiplus::Graphics* g_);
};

DsRenderContext::DsRenderContext(Gdiplus::Graphics* g_) :
	g(g_)
{
}

DsDiagram::DsDiagram(DopeSheetWnd* dopesheet) :
	_dopesheet(dopesheet),
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
	_root._diagram		= this;
	_root._height		= 0.0f;
	_root._isExpanded	= true;
}

DsDiagram::~DsDiagram()
{
}

void DsDiagram::SetClientRect(const NgRect& client)
{
	_client = client;
}

void DsDiagram::Layout()
{
	if (_redoLayout)
	{
		_redoLayout = false;

		float rowY = 0.0f;

		DoRowLayout(&_root, 0.0f, rowY, _client.sz.vx);

		_totalHeight = rowY;
	}

	_scrollY = Clamp(_scrollY, 0.0f, Max(_totalHeight - (_client.sz.vy - _headerY), 0.0f));

	_dopesheet->DoScrollBars();
	_dopesheet->DoScrollBarPos();
}

float DsRoundMarkGap(float v)
{
	if (v < 0.05f)			return DsRoundMarkGap(v * 100.0f) / 100.0f;
	else if (v < 0.25f)		return 0.1f;
	else if (v < 0.75f)		return 0.5f;
	else if (v < 2.5f)		return 1.0f;
	else if (v < 7.5f)		return 5.0f;
	else if (v < 25.0f)		return 10.0f;
	else if (v < 75.0f)		return 50.0f;
	else if (v < 250.0f)	return 100.0f;
	else if (v < 750.0f)	return 500.0f;
	else					return DsRoundMarkGap(v / 100.0f) * 100.0f;
}

void DsDiagram::Render(Gdiplus::Graphics* g)
{
	DsRenderContext ctx(g);

	ctx.clientSize = _client.sz;

	Layout();

	// Draw background

	{
		Gdiplus::GraphicsContainer gc = g->BeginContainer();

		g->SetClip(Gdiplus::RectF(0.0f, _headerY, _client.sz.vx, _client.sz.vy - _headerY));

		g->TranslateTransform(0.0f, floor(_headerY - _scrollY));

		DoRenderBackground(&ctx, &_root);

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

		float markGap = DsRoundMarkGap(width / (_zoomX * 10.0f));

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

			DoRenderRow(&ctx, &_root);

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

			DoRenderLabel(&ctx, &_root, 0.0f);
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

void DsDiagram::SetScrollY(float y)
{
	_scrollY = y;

	InvalidateWindow(*DopeSheet());
}

void DsDiagram::Pan(const NgVector& delta)
{
	_scrollX += delta.vx;
	_scrollY = Clamp(_scrollY - delta.vy, 0.0f, Max(_totalHeight - (_client.sz.vy - _headerY), 0.0f));

	SetDirty(true);

	_dopesheet->DoScrollBarPos();
}

void DsDiagram::Zoom(const NgVector& pt, float factor)
{
	NgVector ptClient = ToContentArea(pt) / NgVector(_zoomX, 1.0f);

	_zoomX = Clamp(_zoomX * factor, 5.0f, 2000.0f);

	NgVector ptPost = NgVector((ptClient.vx * _zoomX) + _splitterX + _scrollX, ptClient.vy + _headerY - _scrollY);

	Pan(pt - ptPost);
}

void DsDiagram::SetRange(float low, float high)
{
	_rangeLow	= low;
	_rangeHigh	= high;

	SetDirty(false);
}

void DsDiagram::SetQuantisation(bool quantise, float q)
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

void DsDiagram::BeginMoveKeys()
{
	_moveKeysOffset = 0.0f;
}

void DsDiagram::MoveSelectedKeysBy(const NgVector& offset)
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

		for(XtUt::LinkList<DsKey>::Iterator it(&_selectionKeys); it; ++it)
			it->MoveBy(newOffset);
	}
	else
	{
		for(XtUt::LinkList<DsKey>::Iterator it(&_selectionKeys); it; ++it)
			it->MoveBy(offset);
	}
}

void DsDiagram::EndMoveKeys()
{
	_moveKeysOffset = 0.0f;
}

DsHitResult DsDiagram::HitTest(const NgVector& pt)
{
	if (pt.vy < _headerY)
		return kDsHrHeader;

	if (pt.vx < _splitterX)
		return kDsHrRowLabel;

	return kDsHrRowContent;
}

DsRow* DsDiagram::HitTestRow(const NgVector& pt)
{
	return _root.HitTestRow(pt);
}

void DsDiagram::ClearSelection()
{
	while(DsRow* row = _selectionRows.elements.next->object)
		row->Deselect();

	while(DsKey* key = _selectionKeys.elements.next->object)
		key->Deselect();
}

void DsDiagram::Select(DsRow* row, DsSelectMode mode)
{
	if (KeySelectionCount() > 0)
		ClearSelection();

	switch(mode)
	{
		case kDsSelectSingle:
			if (_selectionRows.elements.next->object && (_selectionRows.elements.next->object != row))
				ClearSelection();

			if (row)
				row->Select();
		break;

		case kDsSelectAdd:
			if (row)
				row->Select();
		break;

		case kDsSelectRemove:
			if (row)
				row->Deselect();
		break;

		case kDsSelectToggle:
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

void DsDiagram::Select(DsKey* key, DsSelectMode mode)
{
	if (RowSelectionCount() > 0)
		ClearSelection();

	switch(mode)
	{
		case kDsSelectSingle:
			if (_selectionKeys.elements.next->object && (_selectionKeys.elements.next->object != key))
				ClearSelection();

			if (key)
				key->Select();
		break;

		case kDsSelectAdd:
			if (key)
				key->Select();
		break;

		case kDsSelectRemove:
			if (key)
				key->Deselect();
		break;

		case kDsSelectToggle:
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

void DsSelectRectShadow(DsRow* parent, DsRow* row, const NgRect& rect, XtUt::Array<DsKey*>* toSelect)
{
	for(XtUt::LinkList<DsKey>::ReverseIterator itKey = row->ReverseKeys(); itKey; ++itKey)
	{
		NgRect keyRect		= itKey->HitRect();
		NgRect parentRect	= parent->HitRect();

		if (row->IsBlocks())
		{
			keyRect.pt.vy = parentRect.pt.vy + ((itKey->SubRow() + 0.5f) / (float)row->SubRowCount()) * parentRect.sz.vy;
			keyRect.sz.vy = 2.0f;
		}
		else
		{
			keyRect.pt.vx += 2.0f;
			keyRect.sz.vx -= 4.0f;

			keyRect.pt.vy = parentRect.pt.vy + 7.0f;
			keyRect.sz.vy = parentRect.sz.vy - 14.0f;
		}

		if (Intersects(rect, keyRect))
			XtUt::AppendEx(toSelect, (DsKey*)itKey);
	}

	for(XtUt::LinkList<DsRow>::Iterator itRow = row->Rows(); itRow; ++itRow)
		DsSelectRectShadow(parent, itRow, rect, toSelect);
}

void DsSelectRectSub(DsRow* row, const NgRect& rect, XtUt::Array<DsKey*>* toSelect)
{
	for(XtUt::LinkList<DsKey>::ReverseIterator itKey = row->ReverseKeys(); itKey; ++itKey)
	{
		const NgRect& l = itKey->HitRect();

		if (Intersects(rect, l))
			XtUt::AppendEx(toSelect, (DsKey*)itKey);
	}

	if (row->CanExpand() && row->IsExpanded())
	{
		for(XtUt::LinkList<DsRow>::Iterator itRow = row->Rows(); itRow; ++itRow)
			DsSelectRectSub(itRow, rect, toSelect);
	}

	for(XtUt::LinkList<DsRow>::Iterator itRow = row->Rows(); itRow; ++itRow)
		DsSelectRectShadow(row, itRow, rect, toSelect);
}

void DsDiagram::SelectRect(const NgRect& rect, DsSelectMode mode)
{
	if (RowSelectionCount() > 0)
		ClearSelection();

	Layout();

	XtUt::Array<DsKey*> toSelect;

	DsSelectRectSub(&_root, rect, &toSelect);

	for(XtUt::Array<DsKey*>::Iterator it(&toSelect); it; ++it)
		Select(it, mode);
}

int DsDiagram::RowSelectionCount() const
{
	int count = 0;

	for(XtUt::LinkList<DsRow>::ConstIterator it(&_selectionRows); it; ++it)
		count++;

	return count;
}

int DsDiagram::KeySelectionCount() const
{
	int count = 0;

	for(XtUt::LinkList<DsKey>::ConstIterator it(&_selectionKeys); it; ++it)
		count++;

	return count;
}

void DsDiagram::RectSelectionBegin(const NgVector& pt)
{
	_rectSelectionActive	= true;
	_rectSelectionStart		= pt;
	_rectSelectionEnd		= pt;

	SetDirty(false);
}

void DsDiagram::RectSelectionUpdate(const NgVector& pt)
{
	if (_rectSelectionActive)
	{
		_rectSelectionEnd = pt;

		SetDirty(false);
	}
}

void DsDiagram::RectSelectionEnd()
{
	if (_rectSelectionActive)
	{
		_rectSelectionActive = false;

		SetDirty(false);
	}
}

NgVector DsDiagram::ToRowArea(const NgVector& pt)
{
	return NgVector(pt.vx, pt.vy - _headerY + _scrollY);
}

NgVector DsDiagram::ToContentArea(const NgVector& pt)
{
	return NgVector(pt.vx - _splitterX - _scrollX, pt.vy - _headerY + _scrollY);
}

void DsDiagram::DoRowLayout(DsRow* row, float indent, float& rowY, float width)
{
	const float rowIndent = 12.0f;

	if (row != &_root)
		row->LayoutKeys();

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
		for(XtUt::LinkList<DsRow>::Iterator itRow = row->Rows(); itRow; ++itRow)
			DoRowLayout(itRow, indent + rowIndent, rowY, width);
	}
	else
	{
		float shadowY = rowY;

		for(XtUt::LinkList<DsRow>::Iterator itRow = row->Rows(); itRow; ++itRow)
			DoRowLayout(itRow, indent + rowIndent, shadowY, width);
	}

	row->_childRect.sz.vx = width;
	row->_childRect.sz.vy = rowY - row->_childRect.pt.vy;

	if (row->_isBlocks)
	{
		float y0 = row->_hitRect.pt.vy;

		for(XtUt::LinkList<DsKey>::Iterator itKey = row->Keys(); itKey; ++itKey)
		{
			float x0 = floor(itKey->Time() * _zoomX);
			float x1 = floor((itKey->Time() + itKey->Duration()) * _zoomX);
			float centreY = y0 + 2.0f + itKey->_subRow * 24.0f;

			itKey->_hitRect.pt.vx = x0 - 3.0f;
			itKey->_hitRect.pt.vy = centreY;
			itKey->_hitRect.sz.vx = x1 - x0 + 6.0f;
			itKey->_hitRect.sz.vy = 20.0f;
		}
	}
	else
	{
		for(XtUt::LinkList<DsKey>::Iterator itKey = row->Keys(); itKey; ++itKey)
		{
			float x = floor(itKey->Time() * _zoomX);

			itKey->_hitRect.pt.vx = x - 4.0f;
			itKey->_hitRect.pt.vy = row->_hitRect.pt.vy + 5.0f;
			itKey->_hitRect.sz.vx = 8;
			itKey->_hitRect.sz.vy = row->_hitRect.sz.vy - 10.0f;
		}
	}
}

void DsDiagram::DoRenderBackground(DsRenderContext* ctx, DsRow* row)
{
	Gdiplus::Pen penBlack(Gdiplus::Color::Black);

	for(XtUt::LinkList<DsRow>::Iterator itRow = row->Rows(); itRow; ++itRow)
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
	}
}

void DsDiagram::DoRenderLabel(DsRenderContext* ctx, DsRow* row, float oldBoxY)
{
	Gdiplus::SolidBrush brushWhite(Gdiplus::Color::White);
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

	for(XtUt::LinkList<DsRow>::Iterator itRow = row->Rows(); itRow; ++itRow)
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
	}
}

void DsDiagram::DoRenderRowShadow(DsRenderContext* ctx, DsRow* parent, DsRow* row, bool selected)
{
	if (parent != row)
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
		float outerSizeX	= 2.0f;
		float outerSizeY	= 4.0f;

		for(XtUt::LinkList<DsKey>::ReverseIterator itKey(&row->_keys); itKey; ++itKey)
		{
			if (itKey->IsSelected() != selected)
				continue;

			if (row->_isBlocks)
			{
				float x0		= floor(_splitterX + itKey->Time() * _zoomX) + 0.5f;
				float x1		= floor(_splitterX + (itKey->Time() + itKey->Duration()) * _zoomX) + 0.5f;
				float centreY	= y0 + ((itKey->_subRow + 0.5f) / (float)row->_subRowCount) * h - 0.5f;

				ctx->g->FillRectangle(&brush, x0 - 1.0f, centreY, x1 - x0 + 2.0f, 2.0f);
				ctx->g->DrawRectangle(&pen, x0 - 1.0f, centreY, x1 - x0 + 2.0f, 2.0f);
			}
			else
			{
				float x			= floor(_splitterX + itKey->Time() * _zoomX);
				float centreY	= floor((y0 + y1) * 0.5f);

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
	}

	for(XtUt::LinkList<DsRow>::Iterator itRow(&row->_rows); itRow; ++itRow)
		DoRenderRowShadow(ctx, parent, itRow, selected);
}

void DsDiagram::DoRenderRow(DsRenderContext* ctx, DsRow* row)
{
	Gdiplus::SolidBrush brushBlack(Gdiplus::Color::Black);
	Gdiplus::SolidBrush brushWhite(Gdiplus::Color::White);
	Gdiplus::Pen penBlack(Gdiplus::Color::Black);
	Gdiplus::Pen penOrange(Gdiplus::Color::Orange);
	Gdiplus::Pen penRed(Gdiplus::Color::Red);

	for(XtUt::LinkList<DsRow>::Iterator itRow(&row->_rows); itRow; ++itRow)
	{
		float h			= itRow->_hitRect.sz.vy;
		float y0		= itRow->_hitRect.pt.vy;
		float y1		= y0 + h;

		DoRenderRowShadow(ctx, itRow, itRow, false);
		DoRenderRowShadow(ctx, itRow, itRow, true);

		if (itRow->_isBlocks)
		{
			for(XtUt::LinkList<DsKey>::ReverseIterator itKey(&itRow->_keys); itKey; ++itKey)
			{
				float x0 = floor(_splitterX + itKey->Time() * _zoomX) + 0.5f;
				float x1 = floor(_splitterX + (itKey->Time() + itKey->Duration()) * _zoomX) + 0.5f;
				float centreY = y0 + 2.0f + itKey->_subRow * 24.0f;

				ctx->g->FillRectangle(&brushWhite, x0 - 1.0f, centreY, x1 - x0 + 2.0f, 20.0f);

				if (itKey->IsSelected())
				{
					ctx->g->DrawRectangle(&penRed, x0 + 1.0f, centreY + 2.0f, x1 - x0 - 2.0f, 16.0f);
					ctx->g->DrawRectangle(&penOrange, x0 - 1.0f, centreY, x1 - x0 + 2.0f, 20.0f);
				}
				else
				{
					ctx->g->DrawRectangle(&penBlack, x0 - 1.0f, centreY, x1 - x0 + 2.0f, 20.0f);
				}
			}
		}
		else
		{
			float centreY = (y0 + y1) * 0.5f;

			for(XtUt::LinkList<DsKey>::ReverseIterator itKey(&itRow->_keys); itKey; ++itKey)
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
		}

		if (itRow->CanExpand() && itRow->IsExpanded())
			DoRenderRow(ctx, itRow);
	}
}

void DsDiagram::SetDirty(bool redoLayout)
{
	_redoLayout = _redoLayout || redoLayout;

	InvalidateWindow(*DopeSheet());
}