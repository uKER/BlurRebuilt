#include "Core/StdAfx.h"
#include "XtWin/Core.h"
#include "Application/SorceryApp.h"
#include "Application/SorceryView.h"
#include "DynamicForm/GridForm.h"
#include "Xt/XtTool.h"
#include "Xt/Impl/Grimoire.h"
#include "Resource/Resource.h"
#include "XtWin/GridView.h"

extern UINT gWmRebindControls;

//
// DomGridColumn
//

class DomGridColumn : public GridColumn
{
public:
	DomGridColumn(GridMdiWindow* grid, const XtString& path, const XtString& labelPrefix, LarvaField* field) :
		_grid(grid),
		_path(path),
		_field(field)
	{
		SetLabel(MbToWc(labelPrefix + field->label));
		SetWidth(100.0f);
	}

	virtual ~DomGridColumn()
	{
	}

private:
	GridMdiWindow*	_grid;
	XtString		_path;
	LarvaField*		_field;

friend class DomGridRow;
};

//
// DomGridRow
//

class DomGridRow : public GridRow
{
public:
	DomGridRow(GridMdiWindow* grid, Xt::DomElement* element) :
		_grid(grid)
	{
		Monitor(&_dom, element);
	}

	virtual ~DomGridRow()
	{
	}

public:
	virtual Sz16 Content(GridColumn* column)
	{
		DomGridColumn* col = (DomGridColumn*)column;

		if (Xt::DomBlock* block = AsBlock(Xt::FindElementByPath(AsBlock(_dom), col->_path)))
		{
			switch(col->_field->kind)
			{
				case kLfStruct:
				return L"(struct)";

				case kLfInt:
				case kLfFloat:
				case kLfString:
				case kLfEnum:
				return MbToWc(AsText(At(block, col->_field->name)));

				case kLfWeakPtr:
				return MbToWc(AsText(At(block, col->_field->name)));

				case kLfHandle:
				{
					XtId id = Xt::ParseId(AsText(At(block, col->_field->name)));

					if (Xt::DomRecord* record = FindRecordById(_grid->View()._app._db, id))
						return MbToWc(record->name);
				}
				return L"(null)";

				case kLfBool:
				return atoi(AsText(At(block, col->_field->name))) ? L"\x25CF" : L"";

				case kLfWideString:
				return L"(widestring)";

				case kLfFlags:
				return L"(flags)";

				case kLfPtr:
				return L"(ptr)";

				case kLfArray:
				return L"(array)";

				case kLfLocString:
				return L"(locstring)";

				case kLfControl:
				return L"(control)";
			}
		}

		return Sz16();
	}

private:
	GridMdiWindow*	_grid;
	Xt::DomMonitor	_dom;

friend class DomGridColumn;
};

//
// GridMdiWindow
//

GridMdiWindow::GridMdiWindow(SorceryView& view) :
	_view(view),
	_gridView(0)
{
}

GridMdiWindow::~GridMdiWindow()
{
	delete _gridView;
}

bool GridMdiWindow::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!MdiWindow::PreCreateWindow(cs))
		return false;

	cs->dwExStyle |= WS_EX_CLIENTEDGE;

	return true;
}

LRESULT GridMdiWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
			SendMessage(*this, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_SORCERY)));

			WinRect rcClient(*this);

			if ((_gridView = new GridView) == 0)
				return -1;

			if (!_gridView->Create(rcClient, *this))
				return -1;

			Xt::DomDb* db = _view._app._db;

			if (LarvaType* type = db->schema->types[_typeName])
			{
				if (type->kind == kLtStruct)
					AddColumnsForType("", "", (LtStruct*)type);
			}

			for(Xt::DomDb::Iterator itPack(db); itPack; ++itPack)
			{
				for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
				{
					if (itRecord->root.type != _typeName)
						continue;

					DomGridRow* row = new DomGridRow(this, &itRecord->root);

					XtString path;
					XtString name;
					XtString ext;

					SplitPath((const char*)itRecord->name, path, name, ext);

					if (*ext)
						row->SetLabel(MbToWc(Substring(ext, 1)));
					else
						row->SetLabel(MbToWc(name));

					_gridView->AddRow(row);
				}
			}

			UpdateWindowTitle();
		}
		break;

		case WM_DESTROY:
		break;

		case WM_CLOSE:
			_view.Destroy(true);
		return 0;

		case WM_SIZE:
		{
			WinRect rcClient(*this);

			SetWindowPos(*_gridView, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
		}
		break;

	    case DM_GETDEFID: return 0;
	    case DM_SETDEFID: return 0;
	}

	return MdiWindow::WndProc(msg, wParam, lParam);
}

void GridMdiWindow::AdjustSizeToForm()
{
}

void GridMdiWindow::UpdateWindowTitle()
{
	SetWindowText(*this, "Grid View Test");
}

void GridMdiWindow::AddColumnsForType(const XtString& path, const XtString& labelPrefix, LtStruct* type)
{
	if (type->base)
		AddColumnsForType(path, labelPrefix, type->base->type);

	for(XtUt::List<LarvaField>::Iterator it(&type->fields); it; ++it)
	{
		switch(it->kind)
		{
			case kLfStruct:
			{
				LfStruct* str = (LfStruct*)(LarvaField*)it;

				AddColumnsForType(path + str->name + "/", path + str->label + " / ", str->type);
			}
			break;

			//case kLfPtr:
			//case kLfArray:
			case kLfControl:
			break;

			default:
				DomGridColumn* col = new DomGridColumn(this, path, labelPrefix, it);

				_gridView->AddColumn(col);
			break;
		}
	}
}