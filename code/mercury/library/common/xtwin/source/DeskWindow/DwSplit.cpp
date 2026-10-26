#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/DeskWindow.h"
#include "XtWin/Resource.h"

/*
	Hold the "min size" for each DwItem? Or just calc each time?

	"min size" is not something (at least at the moment) that panes can set, it's just there as a prevention measure so that
	the splits don't all get crunched up together

	Find out the min size of the left/right split panes and then bubble that pressure up to the parent
	the parent should then shift, and allow its other child to correct itself, then it should also bubble the pressure up

	Change from using "GenericPane" to an HWND?  An extra title bar drawing can always be done by the DwPane...

	DwPanes should be created, then have windows added to them...  They'll then need to add tabs to themselves if they deal with
	more than one window

	When the parent (host is consider a parent) is sizing in a direction which is not "locked", it should push a please scale yourself
	message down into its children.
*/

const int kDwSplitSize	= 4;
const int kDwSplitLeft	= kDwSplitSize / 2;
const int kDwSplitRight	= kDwSplitSize - kDwSplitLeft;

DwSplit::DwSplit(DwHost* host) :
	DwItem(kDwtSplit, host),
	_split(*this),
	_position(0),
	_ratio(0.5f),
	_vertical(true)
{
}

DwSplit::~DwSplit()
{
}

void DwSplit::SetPosition(int position)
{
	SetPositionImpl(position);

	DwItem*	lt				= GetChild(0);
	DwItem*	rb				= GetChild(1);
	WinSize	leftPaneSize	= lt ? lt->RequiredSize() : WinSize();
	WinSize	rightPaneSize	= rb ? rb->RequiredSize() : WinSize();
	int		leftMinSize		= _vertical ? leftPaneSize.cx : leftPaneSize.cy;
	int		rightMinSize	= _vertical ? rightPaneSize.cx : rightPaneSize.cy;

	int low		= (_vertical ? Location().left : Location().top) + kDwSplitLeft + leftMinSize;
	int high	= (_vertical ? Location().right : Location().bottom) - kDwSplitRight - rightMinSize;

	if (_position < low)
	{
		if (DwSplit* shift = FindShiftableAncestor(true))
			shift->SetPosition(shift->_position - (low - _position));
	}
	else if (_position > high)
	{
		if (DwSplit* shift = FindShiftableAncestor(false))
			shift->SetPosition(shift->_position + (_position - high));
	}
}

void DwSplit::SetVertical(bool vertical)
{
	_vertical = vertical;
}

bool DwSplit::Create()
{
	return _split.Create(WinRect(), Host()->Window());
}

int DwSplit::CountWindows()
{
	return DwItem::CountWindows() + 1;
}

WinSize DwSplit::RequiredSize()
{
	WinSize sz;

	if (_vertical)
		sz.cx = kDwSplitSize;
	else
		sz.cy = kDwSplitSize;

	for(int i = 0; i < kMaxChildren; i++)
	{
		if (DwItem* child = GetChild(i))
		{
			WinSize childSz = child->RequiredSize();

			if (_vertical)
			{
				sz.cx += childSz.cx;
				sz.cy = Max(sz.cy, childSz.cy);
			}
			else
			{
				sz.cx = Max(sz.cx, childSz.cx);
				sz.cy += childSz.cy;
			}
		}
	}

	return sz;
}

void DwSplit::OnLayout(const WinRect& location, HDWP dwp, bool scaling)
{
	int size	= _vertical ? location.Width() : location.Height();
	int middle	= scaling ? (int)(size * _ratio) : (_position - (_vertical ? location.left : location.top));

	DwItem* lt = GetChild(0);
	DwItem* rb = GetChild(1);

	WinSize leftPaneSize	= lt ? lt->RequiredSize() : WinSize();
	WinSize rightPaneSize	= rb ? rb->RequiredSize() : WinSize();

	int leftMinSize		= _vertical ? leftPaneSize.cx : leftPaneSize.cy;
	int rightMinSize	= _vertical ? rightPaneSize.cx : rightPaneSize.cy;
	int totalMinSize	= leftMinSize + rightMinSize;

	if (size < totalMinSize)
		middle = size / 2;
	else if (middle < leftMinSize)
		middle = leftMinSize;
	else if (middle > (size - rightMinSize))
		middle = size - rightMinSize;

	middle += _vertical ? location.left : location.top;

	if (scaling)
		_position = middle;
	else
		SetPositionImpl(middle);

	if (_vertical)
	{
		if (lt)
			lt->PerformLayout(WinRect(location.left, location.top, _position - kDwSplitLeft, location.bottom), dwp, scaling);

		DeferWindowPos(dwp, _split, 0, _position - kDwSplitLeft, location.top, kDwSplitSize, location.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

		if (rb)
			rb->PerformLayout(WinRect(_position + kDwSplitRight, location.top, location.right, location.bottom), dwp, scaling);
	}
	else
	{
		if (lt)
			lt->PerformLayout(WinRect(location.left, location.top, location.right, _position - kDwSplitLeft), dwp, scaling);

		DeferWindowPos(dwp, _split, 0, location.left, _position - kDwSplitLeft, location.Width(), kDwSplitSize, SWP_NOACTIVATE | SWP_NOZORDER);

		if (rb)
			rb->PerformLayout(WinRect(location.left, _position + kDwSplitRight, location.right, location.bottom), dwp, scaling);
	}
}

DwSplit* DwSplit::FindShiftableAncestor(bool lookLeft)
{
	DwSplit* split	= this;
	DwSplit* parent	= (DwSplit*)Parent();

	for(;;)
	{
		if ((parent == 0) || (parent->Type() != kDwtSplit))
			return 0;

		bool splitLeft = split->ChildIndex() == 0;

		if ((_vertical == parent->Vertical()) && (splitLeft != lookLeft))
			break;

		split	= parent;
		parent	= (DwSplit*)parent->Parent();
	}

	 return parent;
}

void DwSplit::SetPositionImpl(int position)
{
	_position = position;

	if (_vertical)
		_ratio = Clamp((float)(position - Location().left) / (float)Location().Width(), 0.0f, 1.0f);
	else
		_ratio = Clamp((float)(position - Location().top) / (float)Location().Height(), 0.0f, 1.0f);
}