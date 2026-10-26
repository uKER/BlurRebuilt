#include "Core/XtWinPch.h"
#include "PropertySheetImpl.h"

const int kPsRow_Indent = 13;

PsDiagram::PsDiagram(PropertySheet* sheet) :
	_sheet(sheet),
	_root(0),
	_splitterX(100),
	_layoutDirty(true),
	_scrollY(0)
{
}

PsDiagram::~PsDiagram()
{
}

PsRow* PsDiagram::SetRootEx(PsRow* root)
{
	if (_root != root)
	{
		delete _root;

		if (root && (root->_diagram == 0))
		{
			_root = root;

			_root->_diagram		= this;
			_root->_height		= 0;
			_root->_expanded	= true;
		}
		else
			_root = 0;
	}

	return _root;
}

void PsDiagram::SetScrollY(int offset)
{
	offset = Clamp(offset, 0, Max((_root ? _root->_bottom : 0) - _clientSize.y, 0));

	if (_scrollY != offset)
	{
		_scrollY = offset;

		SetDirty(false);
	}
}

PsHitResult PsDiagram::HitTestSub(PsRow* row, const XwVector& pt)
{
	if (!row)
		return PsHitResult();

	if (pt.y < row->_y)
		return PsHitResult();

	if (pt.y >= row->_bottom)
		return PsHitResult();

	if (row->_parent)
	{
		if (pt.y < (row->_y + row->_height))
		{
			if (pt.x < (_splitterX - 2))
			{
				int margin = (row->_indent - 1) * kPsRow_Indent;

				if (pt.x < margin)
					return PsHitResult(PsHitResult::kMargin, row);

				if (pt.x < (margin + kPsRow_Indent))
					return PsHitResult(PsHitResult::kExpander, row);

				return PsHitResult(PsHitResult::kLabel, row);
			}

			int contentX = (_splitterX + 2);

			if (pt.x > contentX)
			{
				for(PsRow::ControlIterator it(row->Controls()); it; ++it)
				{
					if ((pt.x >= contentX + it->_x) && (pt.x < contentX + it->_x + it->_width))
						return PsHitResult(PsHitResult::kContent, row, it);
				}

				return PsHitResult(PsHitResult::kContent, row);
			}

			return PsHitResult(PsHitResult::kSplitter, row);
		}
	}

	if (row->CanExpand() && row->IsExpanded())
	{
		for(PsRow::RowIterator itRow(row->Rows()); itRow; ++itRow)
		{
			if (PsHitResult hit = HitTestSub(itRow, pt))
				return hit;
		}
	}

	return PsHitResult();
}

PsHitResult PsDiagram::HitTest(const XwVector& pt)
{
	Layout();

	return HitTestSub(_root, pt);
}

XwVector PsDiagram::ToCanvas(const XwVector& pt)
{
	return pt + XwVector(0, _scrollY);
}

XwVector PsDiagram::ToClient(const XwVector& pt)
{
	return pt - XwVector(0, _scrollY);
}

void PsDiagram::LayoutSub(PsRow* row, int& y, int indent)
{
	if (!row)
		return;

	row->_y			= y;
	row->_indent	= indent;

	y += row->_height;

	// Controls

	int flexCount	= 0;
	int normalCount	= 0;
	int normalWidth	= 0;

	for(PsRow::ControlIterator itControl(row->Controls()); itControl; ++itControl)
	{
		if (itControl->_desiredWidth >= 0)
		{
			normalCount++;
			normalWidth += itControl->_desiredWidth;
		}
		else
			flexCount++;
	}

	int totalCount = flexCount + normalCount;

	if (totalCount > 0)
	{
		int contentWidth	= Max(_clientSize.x - (_splitterX + 2), 0);
		int freeWidth		= Max(contentWidth - (normalWidth + totalCount - 1), 0);
		int flexWidth		= (flexCount > 0) ? (freeWidth / flexCount) : 0;

		int x = 0;

		for(PsRow::ControlIterator itControl(row->Controls()); itControl; ++itControl)
		{
			int width = itControl->_desiredWidth;

			if (width < 0)
				width = flexWidth;

			itControl->_x		= x;
			itControl->_width	= width;

			x += width + 1;
		}
	}
	
	// Child rows

	if (row->CanExpand() && row->IsExpanded())
	{
		for(PsRow::RowIterator itRow(row->Rows()); itRow; ++itRow)
			LayoutSub(itRow, y, indent + 1);
	}

	row->_bottom = y;
}

void PsDiagram::Layout()
{
	if (!_layoutDirty)
		return;

	_layoutDirty = false;

	int y = 0;

	LayoutSub(_root, y, 0);

	_sheet->DoScrollBars();

	_scrollY = Clamp(_scrollY, 0, Max((_root ? _root->_bottom : 0) - _clientSize.y, 0));

	_sheet->DoScrollPos();
}

void PsDiagram::RenderSub(Gdiplus::Graphics* g, PropertySheetDrawContext* dc, PsRow* row)
{
	if (!row)
		return;

	if (!row->_parent)
	{
		for(PsRow::RowIterator itRow(row->Rows()); itRow; ++itRow)
			RenderSub(g, dc, itRow);
	}
	else
	{
		int x0 = 0;
		int x1 = row->_indent * kPsRow_Indent;
		int x2 = _splitterX;
		int x3 = _clientSize.x;
		int y0 = row->_y;
		int y1 = row->_y + row->_height;

		// Rects

		XwRect labelRect(x1, row->_y, Max(x2 - x1, 0), row->_height);
		XwRect contentRect(x2 + 1, row->_y, Max(x3 - x2 - 1, 0), row->_height);

		// Clip

		Gdiplus::GraphicsContainer gc = g->BeginContainer();

		g->SetClip(Gdiplus::Rect(0, y0 - 1, _splitterX, y1 - y0 + 2));

		// Label

		row->DrawLabel(g, dc, labelRect);

		// Margins

		for(int i = 0; i < row->_indent; i++)
		{
			int mx = i * kPsRow_Indent;

			g->FillRectangle(&dc->gridBrush, mx, y0 - 1, kPsRow_Indent - 2, y1 - y0 + 1);
		}

		// Expander box

		if (row->CanExpand())
		{
			int bx = x1 - 12;
			int by = y0 + 5;

			Gdiplus::Pen boxPen(Gdiplus::Color::DarkGray);
			Gdiplus::Pen crossPen(Gdiplus::Color::Black);

			g->FillRectangle(&dc->whiteBrush, bx, by, 9, 9);
			g->DrawRectangle(&boxPen, bx, by, 8, 8);

			g->DrawLine(&crossPen, bx + 2, by + 4, bx + 6, by + 4);

			if (!row->IsExpanded())
				g->DrawLine(&crossPen, bx + 4, by + 2, bx + 4, by + 6);
		}

		// Content

		g->EndContainer(gc);

		row->DrawContent(g, dc, contentRect);

		// Splitter

		g->DrawLine(&dc->gridPen, x2, y0, x2, y1 - 1);

		// Children

		if (row->CanExpand())
		{
			if (row->IsExpanded())
			{
				g->DrawLine(&dc->gridPen, x1 + kPsRow_Indent, y1 - 1, x2, y1 - 1);
				g->DrawLine(&dc->gridPen, x2, y1 - 1, x3, y1 - 1);

				for(PsRow::RowIterator itRow(row->Rows()); itRow; ++itRow)
					RenderSub(g, dc, itRow);
			}
			else
			{
				g->DrawLine(&dc->gridPen, x1, y1 - 1, x2, y1 - 1);
				g->DrawLine(&dc->gridPen, x2, y1 - 1, x3, y1 - 1);
			}
		}
		else
		{
			g->DrawLine(&dc->gridPen, x1, y1 - 1, x2, y1 - 1);
			g->DrawLine(&dc->gridPen, x2, y1 - 1, x3, y1 - 1);
		}
	}
}

void PsDiagram::Render(Gdiplus::Graphics* g, PropertySheetDrawContext* dc)
{
	Layout();

	g->TranslateTransform(0.0f, (float)-_scrollY);

	RenderSub(g, dc, _root);
}

void PsDiagram::SetDirty(bool performLayout)
{
	if (_sheet)
		InvalidateWindow(*_sheet);

	_layoutDirty = _layoutDirty || performLayout;
}