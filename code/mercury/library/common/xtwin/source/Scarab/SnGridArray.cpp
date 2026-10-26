#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

//
// SnGridColumn
//

class SnGridColumn : public GridColumn
{
public:
	SnGridColumn(SnGridView* wnd, const XtString& path, const XtString& labelPrefix, LarvaField* field) :
		_wnd(wnd),
		_path(path),
		_field(field)
	{
		SetLabel(MbToWc(labelPrefix + field->label));
		SetWidth(100.0f);
	}

	virtual ~SnGridColumn()
	{
	}

private:
	SnGridView*		_wnd;
	XtString		_path;
	LarvaField*		_field;

friend class SnGridRow;
friend class SnGridContext;
};

//
// SnGridRow
//

class SnGridRow : public GridRow
{
public:
	SnGridRow(SnGridView* wnd, LarvaField* field, Xt::DomElement* element, int index) :
		_wnd(wnd),
		_field(field)
	{
		_dom.SetCallback(DomCallback, this);

		Monitor(&_dom, element);

		//SetLabel(MbToWc(EvaluateSummaryText(wnd->Db(), wnd->Schema(), _field, element)));
		SetLabel(MbToWc(Format("%i", index)));
	}

	virtual ~SnGridRow()
	{
	}

public:
	virtual Sz16 Content(GridColumn* column)
	{
		SnGridColumn* col = (SnGridColumn*)column;

		Xt::DomDb* db = _wnd->Db();

		if (Xt::DomElement* element = Xt::FindElementByPath(&*_dom, col->_path))
			return MbToWc(EvaluateSummaryText(db, db->schema, col->_field, element));

		return Sz16();
	}

	void KeyPressed(GridColumn* column, int key)
	{
		DebugLn("yay! %i", key);
	}

	void DoubleClicked(GridColumn* column)
	{
		DebugLn("yay!");
	}

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnGridRow* node)
	{
		if (event->code != Xt::kDomRemoved)
			InvalidateWindow(*node->View());
	}

private:
	SnGridView*		_wnd;
	LarvaField*		_field;
	Xt::DomMonitor	_dom;

friend class SnGridColumn;
friend class SnGridContext;
};

//
// SnGridContext
//

class SnGridContext : public GridViewContext
{
public:
	SnGridContext(SnGridView* wnd) :
		_wnd(wnd)
	{
	}


	virtual ~SnGridContext()
	{
	}

public:
	virtual void ContextMenu(const WinPoint& pt)
		{
		HMENU menu	= CreatePopupMenu();

		AppendMenu(menu, MF_STRING, 1, _T("Set selected to focus value"));

		int result = TrackPopupMenu(menu, TPM_NONOTIFY | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, *_grid, 0);

		DestroyMenu(menu);

		if (result == 1)
		{
			int x0 = _grid->SelectX0();
			int y0 = _grid->SelectY0();
			int x1 = _grid->SelectX1();
			int y1 = _grid->SelectY1();

			if (x0 > x1)
				Swap(x0, x1);

			if (y0 > y1)
				Swap(y0, y1);

			SnGridColumn*	focusCol = (SnGridColumn*)_grid->ColumnAt(_grid->FocusX());
			SnGridRow*		focusRow = (SnGridRow*)_grid->RowAt(_grid->FocusY());

			if (focusRow && focusCol)
			{
				if (Xt::DomElement* srcElement = Xt::FindElementByPath(&*focusRow->_dom, focusCol->_path))
				{
					for(int y = y0; y <= y1; y++)
					{
						SnGridRow* row = (SnGridRow*)_grid->RowAt(y);

						for(int x = x0; x <= x1; x++)
						{
							SnGridColumn* col = (SnGridColumn*)_grid->ColumnAt(x);

							if (Xt::DomElement* dstElement = Xt::FindElementByPath(&*row->_dom, col->_path))
							{
								if ((srcElement != dstElement) && (srcElement->kind == dstElement->kind))
								{
									if (Xt::DomElement* newElement = Duplicate(_wnd->Schema(), focusCol->_field, srcElement))
									{
										switch(srcElement->kind)
										{
											case Xt::kDomField:		Clone((Xt::DomField*)dstElement, (Xt::DomField*)newElement);			break;
											case Xt::kDomRef:		Clone((Xt::DomRef*)dstElement, (Xt::DomRef*)newElement);				break;
											case Xt::kDomBlock:		Clone((Xt::DomBlock*)dstElement, (Xt::DomBlock*)newElement);			break;
											case Xt::kDomArray:		Clone((Xt::DomArray*)dstElement, (Xt::DomArray*)newElement);			break;
											case Xt::kDomComponent:	Clone((Xt::DomComponent*)dstElement, (Xt::DomComponent*)newElement);	break;
										}

										delete newElement;
									}
								}
							}
						}
					}
				}

				InvalidateWindow(*_grid);
			}
		}
	}

private:
	SnGridView* _wnd;
};

SnGridView::SnGridView(LfControl* field) :
	SnControl(field),
	_idGrid(this),
	_panelSize(1000, 500)
{
	_dom.SetCallback(DomCallback, this);

	_panelSize.cx = (int)GetInt("GridView", "width", _panelSize.cx);
	_panelSize.cy = (int)GetInt("GridView", "height", _panelSize.cy);
}

SnGridView::~SnGridView()
{
}

void SnGridView::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(parentDom);
}

void SnGridView::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireBlock(parentDom, index));
}

bool SnGridView::OnBuild()
{
	if (!_idGrid.Allocate())
		return false;

	if ((HWND)_wndGrid == 0)
	{
		if (!_wndGrid.Create(*Panel(), _idGrid.IdAsHMenu()))
			return false;

		_wndGrid.SetContext(new SnGridContext(this));
		_wndGrid.SetColumnHeaderWidth(32.0f);
	}

	return true;
}

void SnGridView::OnMeasure(HDC ic)
{
	_size = _panelSize;
}

void SnGridView::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_wndGrid, _client.left, _client.top, _client.Width(), _client.Height(), SWP_SHOWWINDOW);
}

ScarabClipTarget SnGridView::ClipTarget()
{
	return ScarabClipTarget(AsArray(_dom), _field);
}

void SnGridView::SetDom(Xt::DomBlock* block)
{
	SetEnabled(Monitor(&_dom, block) != 0);

	if (block)
	{
		if (Xt::FieldDom fd = Xt::GetByDottedPath(_field->container, block, GetString("GridView", "path")))
		{
			if (fd.field->kind == kLfArray)
			{
				if (Xt::DomArray* arr = AsArray(fd.dom))
				{
					_wndGrid.Clear();

					for(Xt::DomArray::IndexIterator it(arr); it; ++it)
						_wndGrid.RowAppend(new SnGridRow(this, ((LfArray*)fd.field)->field, it, it.index));

					AddColumnsForType("", "", AsStruct(fd.field->Type()));
				}
			}
		}
	}
}

void SnGridView::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnGridView* node)
{
	if ((event->from == monitor->element) && (event->code == Xt::kDomRemoved))
		node->SetDom(0);
	else if (*monitor == event->from)
	{
	}
}

void SnGridView::AddColumnsForType(const XtString& path, const XtString& labelPrefix, LtStruct* type)
{
	if (type->base && !type->base->hidden)
		AddColumnsForType(path, labelPrefix, type->base->type);

	for(XtUt::List<LarvaField>::Iterator it(&type->fields); it; ++it)
	{
		if (it->hidden)
			continue;

		switch(it->kind)
		{
			case kLfStruct:
			{
				LfStruct* str = (LfStruct*)(LarvaField*)it;

				AddColumnsForType(path + str->name + "/", path + str->label + " / ", str->type);
			}
			break;

			case kLfFlags:
			case kLfStrictPtr:
			case kLfPtr:
			case kLfWeakPtr:
			case kLfArray:
			case kLfControl:
			break;

			default:
				SnGridColumn* col = new SnGridColumn(this, path + it->name, labelPrefix, it);

				_wndGrid.ColumnAppend(col);
			break;
		}
	}
}