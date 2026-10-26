#include "Core/XtWinPch.h"
#include "GridViewImpl.h"

GridRow::GridRow() :
	_view(0),
	_height(17.0f),
	_y(0.0f),
	_cy(0.0f)
{
}

GridRow::~GridRow()
{
}

Sz16 GridRow::Content(GridColumn* column)
{
	return Sz16();
}

void GridRow::KeyPressed(GridColumn* column, int key)
{
}

void GridRow::DoubleClicked(GridColumn* column)
{
}

void GridRow::SetHeight(float height)
{
	_height = height;
}

void GridRow::SetLabel(const Sz16& label)
{
	_label = label;
}