#include "Core/XtWinPch.h"
#include "NgImpl.h"

/*

	NgConnection* c;

	c->SetFrom(pin);
	c->SetTo(pin);

	Need to keep track of the connection in the SnNgNode, so if it the pointer gets changed, or deleted I can update the connection appropriately.

	Use bezier splines as connections, allow editing of the control point attached to the box, so you can angle it.

	Iterate dom element node is attached to, find all the weakptr's it contains and make Pins for each, don't worry about being able to add/remove
	pins for now, I'll come up with ui for that later.

*/

Gdiplus::Color Brighter(Gdiplus::Color colour, float val, float sat, float gamma)
{
	float r = colour.GetR() / 255.0f;
	float g = colour.GetG() / 255.0f;
	float b = colour.GetB() / 255.0f;

	r = powf(r, 1.0f / gamma);
	g = powf(g, 1.0f / gamma);
	b = powf(b, 1.0f / gamma);

	r *= val;
	g *= val;
	b *= val;

	float grey = (r + g + b) / 3.0f;

	r = grey + ((r - grey) * sat);
	g = grey + ((g - grey) * sat);
	b = grey + ((b - grey) * sat);

	r = powf(r, gamma);
	g = powf(g, gamma);
	b = powf(b, gamma);

	int ir = Clamp((int)(r * 255.0f), 0, 255);
	int ig = Clamp((int)(g * 255.0f), 0, 255);
	int ib = Clamp((int)(b * 255.0f), 0, 255);

	return Gdiplus::Color(colour.GetA(), ir, ig, ib);
}

Gdiplus::Color Lerp(Gdiplus::Color colour0, Gdiplus::Color colour1, float a)
{
	return Gdiplus::Color(
			Clamp((int)Lerp(colour0.GetA(), colour1.GetA(), a), 0, 255),
			Clamp((int)(255.0f * Square(Lerp(sqrtf(colour0.GetR() / 255.0f), sqrtf(colour1.GetR() / 255.0f), a))), 0, 255),
			Clamp((int)(255.0f * Square(Lerp(sqrtf(colour0.GetG() / 255.0f), sqrtf(colour1.GetG() / 255.0f), a))), 0, 255),
			Clamp((int)(255.0f * Square(Lerp(sqrtf(colour0.GetB() / 255.0f), sqrtf(colour1.GetB() / 255.0f), a))), 0, 255)
		);
}

NgRenderContext::NgRenderContext(Gdiplus::Graphics* g_) :
	g(g_),
	nodeFont(Gdiplus::FontFamily::GenericSansSerif(), 9.0f, Gdiplus::FontStyleBold),
	pinFont(Gdiplus::FontFamily::GenericSansSerif(), 9.0f, Gdiplus::FontStyleRegular),
	waterFont(Gdiplus::FontFamily::GenericSansSerif(), 32.0f, Gdiplus::FontStyleRegular),
	cornerSize(8.0f),
	topMargin(18.0f),
	horzMargin(3.0f),
	columnGap(8.0f),
	pinSize(11.0f),
	pinLineHeight(16.0f),
	arrowWidth(8.0f),
	arrowHeight(4.0f)
{
}

NgVector NgRenderContext::Measure(const Sz16& text, Gdiplus::Font* font)
{
	Gdiplus::RectF bb;

	g->MeasureString(text, text.Length(), font, NgVector(), &bb);

	return NgVector(bb.Width, bb.Height);
}

NgDiagram::NgDiagram(NodeGraph* graph) :
	_graph(graph),
	_transformScale(1.0f, 1.0f),
	_rectSelectionActive(false),
	_connectionDragActive(false),
	_connectionDragFrom(0),
	_connectionDragTo(0),
	_hot(0),
	_isDirty(true)
{
}

NgDiagram::~NgDiagram()
{
	XtUt::Abandon(&_selection);

	XtUt::Free(&_connections);
	XtUt::Free(&_nodes);
}

NodeGraph* NgDiagram::Graph()
{
	return _graph;
}

NgNode* NgDiagram::AddNode(NgNode* node)
{
	if (!node)
		return 0;

	if (node->_diagram)
		node->_diagram->SetDirty(true);

	node->_diagram = this;

	XtUt::PushFront(&_nodes, &node->_linkDiagram);

	SetDirty(true);

	return node;
}

NgConnection* NgDiagram::AddConnection(NgConnection* connection)
{
	if (!connection || (connection->_diagram != 0))
		return 0;

	if (connection->_diagram)
		connection->_diagram->SetDirty(true);

	connection->_diagram = this;

	XtUt::PushFront(&_connections, &connection->_linkDiagram);

	SetDirty(true);

	return connection;
}

void NgDiagram::Clear()
{
	XtUt::Free(&_nodes);
	XtUt::Free(&_connections);

	SetTransformOffset(NgVector(0.0f));
	SetTransformScale(NgVector(1.0f));

	SetDirty(true);
}

void NgDiagram::Pan(NgVector delta)
{
	SetTransformOffset(_transformOffset + delta);
}

void NgDiagram::Zoom(NgVector pt, float factor)
{
	NgVector ptClient = ToClient(pt);

	SetTransformScale(_transformScale * factor);

	NgVector ptPost = ToCanvas(ptClient);

	SetTransformOffset(_transformOffset + (ptPost - pt));
}

void NgDiagram::Frame()
{
	Layout();

	NgVector min;
	NgVector max;

	int count = 0;

	for(XtUt::LinkList<NgNode>::Iterator itNode(&_nodes); itNode; ++itNode)
	{
		const NgRect& l = itNode->Location();

		if (count > 0)
		{
			min.vx = Min(min.vx, l.TopLeft().vx);
			min.vy = Min(min.vy, l.TopLeft().vy);

			max.vx = Max(max.vx, l.BottomRight().vx);
			max.vy = Max(max.vy, l.BottomRight().vy);
		}
		else
		{
			min = l.TopLeft();
			max = l.BottomRight();
		}

		count++;
	}

	SetTransformOffset(-(min + max) * 0.5f);
}

void NgDiagram::SetClientRect(const NgRect& rect)
{
	_clientRect		= rect;
	_clientCentre	= rect.pt + (rect.sz * 0.5f);

	SetDirty(false);
}

void NgDiagram::SetTransformOffset(const NgVector& offset)
{
	_transformOffset = offset;

	SetDirty(false);
}

void NgDiagram::SetTransformScale(const NgVector& scale)
{
	_transformScale.vx = Clamp(scale.vx, 0.01f, 100.0f);
	_transformScale.vy = Clamp(scale.vy, 0.01f, 100.0f);

	SetDirty(false);
}

void NgDiagram::SetHotPin(NgPin* hot)
{
	if (_hot != hot)
	{
		_hot = hot ? hot->Anchor() : 0;

		SetDirty(false);
	}
}

void NgDiagram::SetWatermark(const Sz16& watermark)
{
	_watermark = watermark;
}

void NgDiagram::SetDirty(bool redoLayout)
{
	_isDirty = _isDirty || redoLayout;

	if (_graph)
		InvalidateWindow(*_graph);
}

void NgDiagram::Layout()
{
	if (!_isDirty)
		return;

	_isDirty = false;

	Gdiplus::Graphics g(*Graph());

	NgRenderContext context(&g);

	for(XtUt::LinkList<NgNode>::ReverseIterator itNode(&_nodes); itNode; ++itNode)
		itNode->Layout(&context);
}

namespace
{
	void DrawArrow(Gdiplus::Graphics* g, NgRenderContext* ctx, const NgVector& pt, float dir, XtU32 colour)
	{
		Gdiplus::PointF arrow[3];

		arrow[0] = pt + NgVector(dir * (ctx->pinSize * 0.5f), 0.0f);
		arrow[1] = pt + NgVector(dir * ((ctx->pinSize * 0.5f) + ctx->arrowWidth), ctx->arrowHeight);
		arrow[2] = pt + NgVector(dir * ((ctx->pinSize * 0.5f) + ctx->arrowWidth), -ctx->arrowHeight);

		Gdiplus::SolidBrush brush(colour);

		g->FillPolygon(&brush, arrow, 3);
	}
}

void NgDiagram::Render(Gdiplus::Graphics* g)
{
	Layout();

	NgRenderContext ctx(g);

	Gdiplus::Pen outline(Gdiplus::Color::Black, 1.0f);
	Gdiplus::Pen selected(0xB0FFA500, 2.0f);
	Gdiplus::Pen selectedAlt(0xB0FF0000, 2.0f);
	Gdiplus::Pen rectSelection(Gdiplus::Color::DodgerBlue, 1.75f);
	Gdiplus::Pen connectionDragPt(Gdiplus::Color::DeepSkyBlue, 2.5f);
	Gdiplus::Pen connectionDragGood(Gdiplus::Color::Chartreuse, 2.5f);
	Gdiplus::Pen connectionDragBad(Gdiplus::Color::OrangeRed, 2.5f);

	float pattern[] = { 2.0f, 2.0f };

	selected.SetDashPattern(pattern, ArrayCount(pattern));
	selectedAlt.SetDashPattern(pattern, ArrayCount(pattern));
	rectSelection.SetDashPattern(pattern, ArrayCount(pattern));

	selectedAlt.SetDashOffset(2.0f);

	Gdiplus::SolidBrush shadow(Gdiplus::Color(0x40, 0x00, 0x00, 0x00));
	Gdiplus::SolidBrush nodeText(Gdiplus::Color::Black);
	Gdiplus::SolidBrush pinText(Gdiplus::Color::Black);
	Gdiplus::SolidBrush whiteFill(Gdiplus::Color::White);
	Gdiplus::SolidBrush waterFill(Gdiplus::Color(230, 230, 230));

	Gdiplus::StringFormat nodeFormat;

	nodeFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
	nodeFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);

	Gdiplus::StringFormat pinFormat;

	pinFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);

	Gdiplus::StringFormat pinFormatRight;

	pinFormatRight.SetAlignment(Gdiplus::StringAlignmentFar);
	pinFormatRight.SetLineAlignment(Gdiplus::StringAlignmentCenter);

	Gdiplus::StringFormat waterFormat;

	waterFormat.SetAlignment(Gdiplus::StringAlignmentFar);
	waterFormat.SetLineAlignment(Gdiplus::StringAlignmentFar);

	NgVector kShadowOffset(4.0f);
	NgVector kSelectionInflation(3.0f);

	// Watermark

	if (*_watermark)
		g->DrawString(_watermark, _watermark.Length(), &ctx.waterFont, _clientRect.BottomRight() - NgVector(4.0f, 4.0f), &waterFormat, &waterFill);

	// Transform canvas

	g->TranslateTransform(_clientCentre.vx, _clientCentre.vy);
	g->ScaleTransform(_transformScale.vx, _transformScale.vy);
	g->TranslateTransform(_transformOffset.vx, _transformOffset.vy);

	NgVector	canvasTl	= ToCanvas(_clientRect.TopLeft());
	NgVector	canvasBr	= ToCanvas(_clientRect.BottomRight());
	NgRect		canvas		= Inflate(NgRect(canvasTl, canvasBr - canvasTl), ctx.pinSize + kShadowOffset + kSelectionInflation);

	// Draw connections

	for(XtUt::LinkList<NgConnection>::ReverseIterator itCon(&_connections); itCon; ++itCon)
	{
		NgConnection*	con		= itCon;
		NgPin*			from	= con->From();
		NgPin*			to		= con->To();

		if (!from || !to)
			continue;

		NgVector start	= from->Node()->_location.pt + from->_centre;
		NgVector end	= to->Node()->_location.pt + to->_centre;

		if (!Intersects(Normalise(NgRect(start, end - start)), canvas))
			continue;

		bool startArrow	= from->FromArrow();
		bool endArrow	= to->ToArrow();

		float startDir	= (from->NodeAlign() == kNgAlignLeft) ? -1.0f : 1.0f;
		float endDir	= (to->NodeAlign() == kNgAlignLeft) ? -1.0f : 1.0f;

		float strength = 5.0f + Clamp((fabsf(end.vx - start.vx) - 25.0f) / 125.0f, 0.0f, 1.0f) * 75.0f;

		Gdiplus::PointF points[4];

		points[0] = start + NgVector(startDir * ctx.pinSize * 0.5f, 0.0f);
		points[3] = end + NgVector(endDir * ctx.pinSize * 0.5f, 0.0f);

		if (startArrow)
			points[0].X += startDir * (ctx.arrowWidth * 0.9f);

		if (endArrow)
			points[3].X += endDir * (ctx.arrowWidth * 0.9f);

		points[1] = points[0] + NgVector(startDir * strength, 0.0f);
		points[2] = points[3] + NgVector(endDir * strength, 0.0f);

		Gdiplus::Pen connection(itCon->Colour(), 2.5f);

		if (itCon->IsConflicted())
			connection.SetDashPattern(pattern, ArrayCount(pattern));

		bool isHot = itCon->IsHot();

		if (isHot)
			connection.SetWidth(4.5f);

		if (!isHot && (itCon->Virtual() != kNgVirtualNone))
		{
			switch(itCon->Virtual())
			{
				case kNgVirtualFrom:
				case kNgVirtualBoth:
				{
					const Sz16& label = to->Node()->Label();

					g->DrawString(label, label.Length(), &ctx.pinFont, NgVector(points[0].X + startDir * 22.0f, points[0].Y), (from->NodeAlign() == kNgAlignLeft) ? &pinFormatRight : &pinFormat, &pinText);
				}
				break;
			}

			switch(itCon->Virtual())
			{
				case kNgVirtualTo:
				case kNgVirtualBoth:
				{
					const Sz16& label = from->Node()->Label();

					g->DrawString(label, label.Length(), &ctx.pinFont, NgVector(points[3].X + endDir * 22.0f, points[3].Y), (to->NodeAlign() == kNgAlignLeft) ? &pinFormatRight : &pinFormat, &pinText);
				}
				break;
			}

			g->DrawLine(&connection, points[0], NgVector(points[0].X + startDir * 20.0f, points[0].Y));
			g->DrawLine(&connection, points[3], NgVector(points[3].X + endDir * 20.0f, points[3].Y));

			if (startArrow)
				DrawArrow(g, &ctx, start, startDir, itCon->Colour());

			if (endArrow)
				DrawArrow(g, &ctx, end, endDir, itCon->Colour());
		}
		else
		{
			g->DrawBezier(&connection, points[0], points[1], points[2], points[3]);

			if (startArrow)
				DrawArrow(g, &ctx, start, startDir, itCon->Colour());

			if (endArrow)
				DrawArrow(g, &ctx, end, endDir, itCon->Colour());
		}
	}

	// Draw connection drag

	if (_connectionDragActive)
	{
		NgPin* from	= _connectionDragFrom;
		NgPin* to	= _connectionDragTo;

		if (from)
		{
			NgVector start	= from->Node()->_location.pt + from->_centre;
			NgVector end	=_connectionDragPt;

			float startDir	= (from->NodeAlign() == kNgAlignLeft) ? -1.0f : 1.0f;
			float endDir	= (end.vx > start.vx) ? -1.0f : 1.0f;
			float endPin	= 0.0f;

			bool startArrow	= from->FromArrow();
			bool endArrow	= false;

			if (to)
			{
				endDir		= (to->NodeAlign() == kNgAlignLeft) ? -1.0f : 1.0f;
				endPin		= 1.0f;
				endArrow	= to->ToArrow();
			}

			float strength = 5.0f + Clamp((fabsf(end.vx - start.vx) - 25.0f) / 125.0f, 0.0f, 1.0f) * 75.0f;

			Gdiplus::PointF points[4];

			points[0] = start + NgVector(startDir * ctx.pinSize * 0.5f, 0.0f);
			points[3] = end + NgVector(endPin * endDir * ctx.pinSize * 0.5f, 0.0f);

			if (startArrow)
				points[0].X += startDir * (ctx.arrowWidth * 0.9f);

			if (endArrow)
				points[3].X += endDir * (ctx.arrowWidth * 0.9f);

			points[1] = points[0] + NgVector(startDir * strength, 0.0f);
			points[2] = points[3] + NgVector(endDir * strength, 0.0f);

			XtU32 colour = Gdiplus::Color::DeepSkyBlue;

			if (to)
			{
				if (from->OnTryConnect(to))
				{
					colour = Gdiplus::Color::Chartreuse;

					g->DrawBezier(&connectionDragGood, points[0], points[1], points[2], points[3]);
				}
				else
				{
					colour = Gdiplus::Color::OrangeRed;

					g->DrawBezier(&connectionDragBad, points[0], points[1], points[2], points[3]);
				}
			}
			else
				g->DrawBezier(&connectionDragPt, points[0], points[1], points[2], points[3]);

			if (startArrow)
				DrawArrow(g, &ctx, start, startDir, colour);

			if (endArrow)
				DrawArrow(g, &ctx, end, endDir, colour);
		}
	}

	// Draw nodes	

	for(XtUt::LinkList<NgNode>::ReverseIterator itNode(&_nodes); itNode; ++itNode)
	{
		NgNode*			node	= itNode;
		const NgRect&	l		= node->Location();

		if (!Intersects(l, canvas))
			continue;

		// Node box

		Gdiplus::GraphicsPath p;

		p.AddArc(NgRect(l.TopLeft(), ctx.cornerSize), 180.0f, 90.0f);
		p.AddArc(NgRect(l.TopRight() - NgVector(ctx.cornerSize, 0.0f), ctx.cornerSize), 270.0f, 90.0f);
		p.AddArc(NgRect(l.BottomRight() - ctx.cornerSize, ctx.cornerSize), 0.0f, 90.0f);
		p.AddArc(NgRect(l.BottomLeft() - NgVector(0.0f, ctx.cornerSize), ctx.cornerSize), 90.0f, 90.0f);

		p.CloseFigure();

		Gdiplus::GraphicsContainer gc = g->BeginContainer();

		g->TranslateTransform(kShadowOffset.vx, kShadowOffset.vy, Gdiplus::MatrixOrderAppend);

		g->FillPath(&shadow, &p);

		g->EndContainer(gc);

		Gdiplus::LinearGradientBrush fill(l.TopLeft(), l.TopRight(), itNode->Colour(), Lerp((Gdiplus::Color)itNode->Colour(), Gdiplus::Color::White, 0.8f));

		g->FillPath(&fill, &p);

		g->FillRectangle(&whiteFill, NgRect(l.pt + NgVector(0.0f, ctx.topMargin), l.sz - NgVector(0.0f, ctx.topMargin + ctx.cornerSize * 0.5f)));

		g->DrawPath(&outline, &p);

		// Selection

		if (node->IsSelected())
		{
			g->DrawRectangle(&selected, Inflate(l, kSelectionInflation));
			g->DrawRectangle(&selectedAlt, Inflate(l, kSelectionInflation));
		}

		// Pins

		for(XtUt::LinkList<NgPin>::Iterator itPin(&node->_pins); itPin; ++itPin)
		{
			NgRect pinRect;

			pinRect.pt = l.pt + itPin->_centre - NgVector(ctx.pinSize * 0.5f);
			pinRect.sz = NgVector(ctx.pinSize);

			float v = (itPin ==_hot) ? 1.75f : 1.0f;

			Gdiplus::GraphicsPath fillPath;
			fillPath.AddEllipse(pinRect);

			Gdiplus::PathGradientBrush pinFill(&fillPath);

			pinFill.SetCenterPoint(pinRect.pt + NgVector(ctx.pinSize * 0.3f));
			pinFill.SetCenterColor(Brighter(itPin->_colour, 2.0f * v, 1.0f, 2.2f));

			Gdiplus::Color colours(Brighter(itPin->_colour, 0.9f * v, 1.0f, 2.2f));
			int countColours = 1;
			pinFill.SetSurroundColors(&colours, &countColours);

			g->FillEllipse(&pinFill, pinRect);
			g->DrawEllipse(&outline, pinRect);
		}

		// Node interior

		Gdiplus::GraphicsContainer gcInterior = g->BeginContainer();

		g->SetClip(l);

		// Label

		const Sz16& label = node->Label();

		float extraLabelY = node->Compact() ? (ctx.cornerSize * 0.333f) : 0.0f;

		g->DrawString(label, label.Length(), &ctx.nodeFont, l.pt + NgVector(l.sz.vx * 0.5f, 9.0f + extraLabelY), &nodeFormat, &nodeText);

		// Pin Labels

		for(XtUt::LinkList<NgPin>::Iterator itPin(&node->_pins); itPin; ++itPin)
		{
			const Sz16& pinLabel = itPin->Label();

			if (itPin->NodeAlign() == kNgAlignLeft)
			{
				NgVector pt = l.pt + itPin->_location.pt + NgVector(ctx.horzMargin + ctx.pinSize, ctx.pinLineHeight * 0.5f);

				g->DrawString(pinLabel, pinLabel.Length(), &ctx.pinFont, pt, &pinFormat, &pinText);
			}
			else
			{
				NgVector pt = l.pt + itPin->_location.TopRight() + NgVector(-(ctx.horzMargin + ctx.pinSize), ctx.pinLineHeight * 0.5f);

				g->DrawString(pinLabel, pinLabel.Length(), &ctx.pinFont, pt, &pinFormatRight, &pinText);
			}
		}

		g->EndContainer(gcInterior);
	}

	// Draw selection rectangle

	if (_rectSelectionActive)
		g->DrawRectangle(&rectSelection, RectSelection());

	g->ResetTransform();
}

NgVector NgDiagram::ToCanvas(const NgVector& clientPt)
{
	return ((clientPt - _clientCentre) / _transformScale) - _transformOffset;
}

NgVector NgDiagram::ToClient(const NgVector& canvasPt)
{
	return (canvasPt + _transformOffset) * _transformScale + _clientCentre;
}

NgNode* NgDiagram::HitTestNode(const NgVector& canvasPt)
{
	Layout();

	for(XtUt::LinkList<NgNode>::Iterator it(&_nodes); it; ++it)
	{
		if (PtInRect(it->Location(), canvasPt))
			return it;
	}

	return 0;
}

NgPin* NgDiagram::HitTestPin(const NgVector& canvasPt)
{
	Layout();

	NgRenderContext ctx(0);

	for(XtUt::LinkList<NgNode>::Iterator itNode(&_nodes); itNode; ++itNode)
	{
		if (!PtInRect(Inflate(itNode->Location(), ctx.pinSize * 0.5f), canvasPt))
			continue;

		NgVector newPt = canvasPt - itNode->Location().pt;

		for(XtUt::LinkList<NgPin>::Iterator itPin(itNode->Pins()); itPin; ++itPin)
		{
			if (PtInRect(itPin->_selectRect, newPt))
				return itPin;
			
		}
	}

	return 0;
}

void NgDiagram::ClearSelection()
{
	while(NgNode* node = _selection.elements.next->object)
		node->Deselect();
}

void NgDiagram::Select(NgNode* node, NgSelectMode mode)
{
	switch(mode)
	{
		case kNgSelectSingle:
			if (_selection.elements.next->object && (_selection.elements.next->object != node))
				ClearSelection();

			if (node)
				node->Select();
		break;

		case kNgSelectAdd:
			if (node)
				node->Select();
		break;

		case kNgSelectRemove:
			if (node)
				node->Deselect();
		break;

		case kNgSelectToggle:
			if (node)
			{
				if (node->IsSelected())
					node->Deselect();
				else
					node->Select();
			}
		break;
	}
}

void NgDiagram::SelectRect(const NgRect& rect, NgSelectMode mode)
{
	Layout();

	XtUt::Array<NgNode*> toSelect;

	for(XtUt::LinkList<NgNode>::ReverseIterator it(&_nodes); it; ++it)
	{
		const NgRect& l = it->Location();

		if (Intersects(rect, l))
			XtUt::AppendEx(&toSelect, (NgNode*)it);
	}

	for(XtUt::Array<NgNode*>::Iterator it(&toSelect); it; ++it)
		Select(it, mode);
}

void NgDiagram::MoveSelectedBy(const NgVector& offset)
{
	for(XtUt::LinkList<NgNode>::Iterator it(&_selection); it; ++it)
		it->MoveBy(offset);
}

void NgDiagram::RectSelectionBegin(const NgVector& pt)
{
	_rectSelectionActive	= true;
	_rectSelectionStart		= pt;
	_rectSelectionEnd		= pt;

	SetDirty(false);
}

void NgDiagram::RectSelectionUpdate(const NgVector& pt)
{
	if (_rectSelectionActive)
	{
		_rectSelectionEnd = pt;

		SetDirty(false);
	}
}

void NgDiagram::RectSelectionEnd()
{
	if (_rectSelectionActive)
	{
		_rectSelectionActive = false;

		SetDirty(false);
	}
}

void NgDiagram::ConnectionDragBegin(NgPin* from, NgPin* to)
{
	_connectionDragActive	= true;
	_connectionDragFrom		= from->Anchor();
	_connectionDragTo		= 0;
	_connectionDragPt		= NgVector();

	ConnectionDragUpdate(to);
}

void NgDiagram::ConnectionDragUpdate(NgVector& pt)
{
	if (_connectionDragActive)
	{
		_connectionDragTo = 0;
		_connectionDragPt = pt;

		SetDirty(false);
	}
}

void NgDiagram::ConnectionDragUpdate(NgPin* to)
{
	if (_connectionDragActive)
	{
		_connectionDragTo = to->Anchor();
		_connectionDragPt = to ? (to->Node()->_location.pt + to->_centre) : NgVector();

		SetDirty(false);
	}
}

void NgDiagram::ConnectionDragEnd()
{
	if (_connectionDragActive)
	{
		_connectionDragActive = false;

		SetDirty(false);
	}
}

int NgDiagram::SelectionCount() const
{
	int count = 0;

	for(XtUt::LinkList<NgNode>::ConstIterator it(&_selection); it; ++it)
		count++;

	return count;
}