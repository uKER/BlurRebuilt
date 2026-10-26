#include "Core/XtWinPch.h"
#include "NgImpl.h"

NgNode::NgNode() :
	_anchor(this),
	_diagram(0),
	_linkDiagram(this),
	_isSelected(false),
	_linkSelection(this),
	_isDirty(true),
	_colour(Gdiplus::Color::LightSteelBlue),
	_mirrored(false),
	_compact(false),
	_drawSplit(false),
	_splitAt(0.0f)
{
}

NgNode::~NgNode()
{
	if (_isSelected)
		Deselect();

	SetDirty(false);
}

void NgNode::OnPositionChanged()
{
}

void NgNode::OnDoubleClicked()
{
}

void NgNode::SetPosition(const NgVector& pt)
{
	_location.pt = pt;

	OnPositionChanged();

	SetDirty(false);
}

void NgNode::SetLabel(const Sz16& label)
{
	_label = label;

	SetDirty(true);
}

void NgNode::SetColour(XtU32 colour)
{
	_colour = (colour & 0xFFFFFF) | 0xFF000000;

	SetDirty(false);
}

void NgNode::SetMirrored(bool mirrored)
{
	if (_mirrored != mirrored)
	{
		_mirrored = mirrored;

		SetDirty(true);
	}
}

void NgNode::SetCompact(bool compact)
{
	if (_compact != compact)
	{
		_compact = compact;

		SetDirty(true);
	}
}

NgPin* NgNode::AddPin(NgPin* pin)
{
	if (!pin || (pin->_node != 0))
		return 0;

	pin->_node = this;

	XtUt::PushBack(&_pins, &pin->_link);

	SetDirty(true);

	return pin;
}

NgPin* NgNode::InsertPin(NgPin* pin, NgPin* after)
{
	if (!pin || (pin->_node != 0))
		return 0;

	pin->_node = this;

	if (after && (after->_node == this))
		XtUt::AttachAfter(&pin->_link, &after->_link);
	else
		XtUt::PushFront(&_pins, &pin->_link);

	SetDirty(true);

	return pin;
}

void NgNode::Select()
{
	if (_isSelected || !_diagram)
		return;

	_isSelected = true;

	XtUt::PushFront(&_diagram->_selection, &_linkSelection);

	BringToTop();

	_diagram->SetDirty(false);

	_diagram->Graph()->Context()->SelectionChanged();
}

void NgNode::Deselect()
{
	if (!_isSelected || !_diagram)
		return;

	_isSelected = false;

	XtUt::Detach(&_linkSelection);

	_diagram->SetDirty(false);

	_diagram->Graph()->Context()->SelectionChanged();
}

void NgNode::BringToTop()
{
	if (_diagram)
		XtUt::PushFront(&_diagram->_nodes, &_linkDiagram);

	_diagram->SetDirty(false);
}

void NgNode::SendToBottom()
{
	if (_diagram)
		XtUt::PushBack(&_diagram->_nodes, &_linkDiagram);

	_diagram->SetDirty(false);
}

void NgNode::MoveBy(const NgVector& offset)
{
	SetPosition(_location.pt + offset);
}

bool NgNode::IsBehind(NgNode* node)
{
	if (node && (node != this))
	{
		while((node = node->_linkDiagram.next->object) != 0)
		{
			if (node == this)
				return true;
		}
	}

	return false;
}

int NgNode::PinCount() const
{
	int count = 0;	

	for(XtUt::LinkList<NgPin>::ConstIterator itPin(&_pins); itPin; ++itPin)
		count++;

	return count;
}

void NgNode::SetDirty(bool redoLayout)
{
	_isDirty = _isDirty || redoLayout;

	if (_diagram)
		_diagram->SetDirty(redoLayout);
}

struct NgNodeLayoutColumn
{
	float width;
	int count;
	NgVector pt;

public:
	NgNodeLayoutColumn() :
		width(8.0f),
		count(0)
	{
	}
};

void NgNode::Layout(NgRenderContext* context)
{
	if (!_isDirty)
		return;

	_isDirty = false;

	// Measure

	for(XtUt::LinkList<NgPin>::Iterator itPin(&_pins); itPin; ++itPin)
		itPin->Measure(context);

	NgVector nodeLabelSize = context->Measure(_label, &context->nodeFont) + NgVector(12.0f, 0.0f);

	if (_compact)
	{
		_location.sz.vx = nodeLabelSize.vx + 16.0f;
		_location.sz.vy = context->topMargin + (context->cornerSize * 0.5f);

		for(XtUt::LinkList<NgPin>::Iterator itPin(&_pins); itPin; ++itPin)
		{
			itPin->_location.pt		= NgVector(-context->pinSize * 0.5f, (_location.sz.vy - context->pinSize) * 0.5f);
			itPin->_location.sz.vx	= (context->pinSize * 0.5f);
			itPin->_location.sz.vy	= context->pinSize;

			itPin->_centre = itPin->_location.pt + NgVector(context->pinSize * 0.5f);

			itPin->_selectRect			= itPin->_location;
			itPin->_selectRect.sz.vx	= context->pinSize;

			if (itPin->NodeAlign() == kNgAlignRight)
			{
				itPin->_centre.vx += itPin->_location.sz.vx - context->pinSize;

				itPin->_selectRect.pt.vx = itPin->_centre.vx - (itPin->_selectRect.sz.vx - context->pinSize * 0.5f);
			}
		}
	}
	else
	{
		NgNodeLayoutColumn columns[2];

		// Find column widths

		for(XtUt::LinkList<NgPin>::Iterator itPin(&_pins); itPin; ++itPin)
		{
			NgNodeLayoutColumn* column = &columns[(itPin->NodeAlign() == kNgAlignLeft) ? 0 : 1];

			column->width = Max(column->width, itPin->_labelSize.vx + context->horzMargin);
			column->count++;
		}

		// Adjust columns for margins

		if ((columns[0].count > 0) && (columns[1].count > 0))
		{
			columns[0].width += context->columnGap;
			columns[1].width += context->columnGap;
		}

		float adjustForLabel = Max((nodeLabelSize.vx - (columns[0].width + columns[1].width)) * 0.5f, 0.0f);

		columns[0].width += adjustForLabel;
		columns[1].width += adjustForLabel;

		// Remove zero count columns

		if (columns[0].count <= 0)
		{
			columns[1].width += columns[0].width;
			columns[0].width = 0;
		}
		else if (columns[1].count <= 0)
		{
			columns[0].width += columns[1].width;
			columns[1].width = 0;
		}

		// Calculate total size of node

		int maxCount = Max(columns[0].count, columns[1].count);

		_location.sz.vx = columns[0].width + columns[1].width;
		_location.sz.vy = context->topMargin + (16.0f * maxCount) + (context->cornerSize * 0.5f);

		// Position pins

		_splitAt	= columns[0].width;
		_drawSplit	= ((columns[0].count > 0) && (columns[1].count > 0));

		columns[0].pt = NgVector(-context->pinSize * 0.5f, context->topMargin);
		columns[1].pt = NgVector(_splitAt, context->topMargin);

		for(XtUt::LinkList<NgPin>::Iterator itPin(&_pins); itPin; ++itPin)
		{
			NgNodeLayoutColumn* column = &columns[(itPin->NodeAlign() == kNgAlignLeft) ? 0 : 1];

			itPin->_location.pt		= column->pt;
			itPin->_location.sz.vx	= column->width + (context->pinSize * 0.5f);
			itPin->_location.sz.vy	= context->pinLineHeight;

			itPin->_centre = itPin->_location.pt + NgVector(0.0f, (context->pinLineHeight - context->pinSize) * 0.5f) + NgVector(context->pinSize * 0.5f);

			itPin->_selectRect			= itPin->_location;
			itPin->_selectRect.sz.vx	= context->pinSize + context->horzMargin + itPin->_labelSize.vx;

			if (itPin->NodeAlign() == kNgAlignRight)
			{
				itPin->_centre.vx += itPin->_location.sz.vx - context->pinSize;

				itPin->_selectRect.pt.vx = itPin->_centre.vx - (itPin->_selectRect.sz.vx - context->pinSize * 0.5f);
			}

			column->pt.vy += context->pinLineHeight;
		}
	}
}