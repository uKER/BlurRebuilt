#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/DeskWindow.h"
#include "XtWin/Resource.h"

DwItem::DwItem(DwType type, DwHost* host) :
	_type(type),
	_host(host),
	_parent(0),
	_childIndex(-1)
{
	for(int i = 0; i < kMaxChildren; i++)
		_child[i] = 0;
}

DwItem::~DwItem()
{
	for(int i = 0; i < kMaxChildren; i++)
	{
		if (_child[i])
			delete _child[i]->Detach();
	}
}

DwItem* DwItem::GetChild(int idx)
{
	return ((idx >= 0) && (idx < kMaxChildren)) ? _child[idx] : 0;
}

DwItem* DwItem::SetChild(int idx, DwItem* child)
{
	if ((idx < 0) || (idx >= kMaxChildren))
	{
		assert(0);

		delete child;

		return 0;
	}

	if (child)
		child->Detach();

	if (_child[idx])
		delete _child[idx]->Detach();

	if ((_child[idx] = child) != 0)
	{
		child->_parent		= this;
		child->_childIndex	= idx;
	}

	return child;
}

DwItem* DwItem::Detach()
{
	if (_parent)
	{
		assert((_childIndex >= 0) && (_childIndex < kMaxChildren));
		assert(_parent->_child[_childIndex] == this);

		_parent->_child[_childIndex] = 0;

		_parent		= 0;
		_host		= 0;
		_childIndex	= -1;
	}

	return this;
}

void DwItem::PerformLayout(const WinRect& location, HDWP dwp, bool scaling)
{
	_location = location;

	OnLayout(location, dwp, scaling);
}

bool DwItem::Create()
{
	return true;
}

int DwItem::CountWindows()
{
	int count = 0;

	for(int i = 0; i < kMaxChildren; i++)
	{
		if (_child[i])
			count += _child[i]->CountWindows();
	}

	return count;
}

WinSize DwItem::RequiredSize()
{
	WinSize sz;

	for(int i = 0; i < kMaxChildren; i++)
	{
		if (_child[i])
		{
			WinSize childSz = _child[i]->RequiredSize();

			sz.cx = Max(sz.cx, childSz.cx);
			sz.cy = Max(sz.cy, childSz.cy);
		}
	}

	return sz;
}

void DwItem::OnLayout(const WinRect& location, HDWP dwp, bool scaling)
{
	for(int i = 0; i < kMaxChildren; i++)
	{
		if (_child[i])
			_child[i]->PerformLayout(location, dwp, scaling);
	}
}

void DwItem::OnPaint(DwDrawContext& dc)
{
	for(int i = 0; i < kMaxChildren; i++)
	{
		if (_child[i])
			_child[i]->OnPaint(dc);
	}
}