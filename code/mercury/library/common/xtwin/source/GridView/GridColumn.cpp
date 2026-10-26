#include "Core/XtWinPch.h"
#include "GridViewImpl.h"

GridColumn::GridColumn() :
	_view(0),
	_width(80.0f),
	_x(0.0f),
	_cx(0.0f)
{
}

GridColumn::~GridColumn()
{
}

void GridColumn::SetWidth(float width)
{
	_width = width;
}

void GridColumn::SetLabel(const Sz16& label)
{
	_label = label;
}