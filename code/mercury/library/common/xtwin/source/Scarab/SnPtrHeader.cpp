#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

SnPtrHeader::SnPtrHeader(SnPtr* ptr) :
	_ptr(ptr),
	_idCategory(this),
	_idType(this)
{
	_labelText = "Type:";

	_hasCategory = _ptr->_field->attrs.FindAttr("Categorise") != 0;

	_dom.SetCallback(DomCallback, this);
}

SnPtrHeader::~SnPtrHeader()
{
}

void SnPtrHeader::OnBind(Xt::DomComponent* parentDom)
{
	SetDom(parentDom);
}

bool SnPtrHeader::OnBuild()
{
	if (!_idCategory.Allocate() || !_idType.Allocate())
		return false;

	if (_hasCategory)
	{
		if (_wndCategory == 0)
		{
			if (!_wndCategory.Create(WS_EX_CLIENTEDGE, _T("XTWINCOMBOBOX"), _T(""),
					CBS_DROPDOWNLIST | WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DISABLENOSCROLL,
				200, 20 + 200, *Panel(), _idCategory))
			{
				return false;
			}

			SendMessage(_wndCategory, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);
		}
	}

	if (_wndType == 0)
	{
		if (!_wndType.Create(WS_EX_CLIENTEDGE, _T("XTWINCOMBOBOX"), _T(""),
				CBS_DROPDOWNLIST | WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DISABLENOSCROLL,
			200, 20 + 200, *Panel(), _idType))
		{
			return false;
		}

		SendMessage(_wndType, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);
	}

	PopulateAllType();
	PopulateCategory();

	SyncControls(true);

	return true;
}

void SnPtrHeader::OnMeasure(HDC ic)
{
	_size = WinSize(_hasCategory ? 408 : 200, 20);
}

void SnPtrHeader::OnLayout(ScarabLayoutContext* layoutContext)
{
	int x = _client.left;

	if (_hasCategory)
	{
		layoutContext->SetWindowPos(_wndCategory, _client.left, _client.top, 200, 20 + 200, SWP_SHOWWINDOW | SWP_NOSIZE);
		x += 208;
	}

	layoutContext->SetWindowPos(_wndType, x, _client.top, 200, 20 + 200, SWP_SHOWWINDOW | SWP_NOSIZE);
}

void SnPtrHeader::OnCommand(int id, int code)
{
	LarvaSchema*		schema	= Panel()->Db()->schema;
	Xt::DomComponent*	com		= AsComponent(_dom);

	if (com)
	{
		if ((id == _idCategory) && (code == CBN_SELCHANGE))
		{
			SyncControls(false);
		}
		else if ((id == _idType) && (code == CBN_SELCHANGE))
		{
			if (LtStruct* type = GetSelectedType())
			{
				SetType(com, type->name);

				SetDefaults(schema, type, com);
			}
			else
				SetType(com, kXtNullString);

			_ptr->OnTypeChanged();
		}
	}
}

void SnPtrHeader::SetDom(Xt::DomComponent* com)
{
	SetEnabled(Monitor(&_dom, com) != 0);

	SyncControls(true);
}

void SnPtrHeader::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnPtrHeader* node)
{
	if (event->code == Xt::kDomRemoved)
		node->SetDom(0);
	else if (*monitor == event->from)
	{
		node->SyncControls(true);
		node->_ptr->OnTypeChanged();
	}
}

bool SnPtrHeader::CompareCategory(const Category& lhs, const Category& rhs)
{
	return _stricmp(lhs.label, rhs.label) < 0;
}

bool SnPtrHeader::CompareType(LtStruct* lhs, LtStruct* rhs)
{
	return _stricmp(lhs->label, rhs->label) < 0;
}

void SnPtrHeader::SyncControls(bool syncCategory)
{
	LarvaSchema*		schema			= Schema();
	Xt::DomComponent*	com				= AsComponent(_dom);
	BOOL				enableControls	= com ? TRUE : FALSE;
	int					found			= -1;

	if (syncCategory && _hasCategory)
	{
		if (LtStruct* type = GetDomType())
		{
			for(XtUt::Array<Category>::Iterator it(&_itemsCategory); it; ++it)
			{
				if (it->name == type->category)
				{
					SendMessage(_wndCategory, CB_SETCURSEL, it.index, 0);
					break;
				}
			}
		}
	}

	PopulateType();

	if (com)
	{
		if (LtStruct* type = GetDomType())
		{
			for(XtUt::Array<LtStruct*>::Iterator it(&_itemsType); it; ++it)
			{
				if (type == it)
				{
					found = (int)it.index + 1;
					break;
				}
			}
		}
		else
			found = 0;
	}

	EnableWindow(_wndType, enableControls);

	SendMessage(_wndType, CB_SETCURSEL, found, 0);
}

void SnPtrHeader::PopulateAllType()
{
	LarvaSchema* schema = Panel()->Db()->schema;

	Free(&_itemsAllType);

	for(XtUt::Dictionary<XtString, LarvaType>::Iterator it(&schema->types); it; ++it)
	{
		if (it->kind != kLtStruct)
			continue;

		LtStruct* type = (LtStruct*)(LarvaType*)it;

		if (!IsBaseOf(_ptr->_field->baseType, type))
			continue;

		if (type->hidden)
			continue;

		AppendEx(&_itemsAllType, type);
	}
}

void SnPtrHeader::PopulateCategory()
{
	if (!_hasCategory)
		return;

	LarvaSchema* schema = Panel()->Db()->schema;

	Free(&_itemsCategory);

	for(XtUt::Array<LtStruct*>::Iterator itType(&_itemsAllType); itType; ++itType)
	{
		for(XtUt::Array<Category>::Iterator itCategory(&_itemsCategory); itCategory; ++itCategory)
		{
			if ((*itType)->category == itCategory->name)
				goto skip;
		}

		if (Category* cat = XtUt::AppendExN(&_itemsCategory, 1))
		{
			cat->name	= (*itType)->category;

			if (cat->name == "")
				cat->label = "(misc)";
			else
				cat->label = SplitIntercaps(cat->name);
		}

	skip:;
	}

	XtUt::MergeSort(&_itemsCategory, CompareCategory);

	SendMessage(_wndCategory, CB_RESETCONTENT, 0, 0);

	for(XtUt::Array<Category>::Iterator it(&_itemsCategory); it; ++it)
		SendMessage(_wndCategory, CB_ADDSTRING, 0, (LPARAM)(LPSTR)(const char*)it->label);
}

void SnPtrHeader::PopulateType()
{
	LarvaSchema* schema = Panel()->Db()->schema;

	Category* category = GetSelectedCategory();

	Free(&_itemsType);

	for(XtUt::Array<LtStruct*>::Iterator itType(&_itemsAllType); itType; ++itType)
	{
		LtStruct* type = (LtStruct*)itType;

		if (category && (type->category != category->name))
			continue;

		AppendEx(&_itemsType, type);
	}

	XtUt::MergeSort(&_itemsType, CompareType);

	SendMessage(_wndType, CB_RESETCONTENT, 0, 0);
	SendMessage(_wndType, CB_ADDSTRING, 0, (LPARAM)"(null)");

	for(XtUt::Array<LtStruct*>::Iterator it(&_itemsType); it; ++it)
		SendMessage(_wndType, CB_ADDSTRING, 0, (LPARAM)(LPSTR)(const char*)(*it)->label);
}

SnPtrHeader::Category* SnPtrHeader::GetSelectedCategory()
{
	if (!_hasCategory)
		return 0;

	int index = (int)SendMessage(_wndCategory, CB_GETCURSEL, 0, 0);

	return At(&_itemsCategory, index);
}

LtStruct* SnPtrHeader::GetSelectedType()
{
	int index = ((int)SendMessage(_wndType, CB_GETCURSEL, 0, 0)) - 1;

	LtStruct** type = At(&_itemsType, index);

	return type ? *type : 0;
}

LtStruct* SnPtrHeader::GetDomType()
{
	LarvaSchema* schema = Panel()->Db()->schema;

	if (Xt::DomComponent* com = AsComponent(_dom))
	{
		if (LtStruct* type = TypeOf(schema, _ptr->_field->baseType, com))
		{
			if (IsBaseOf(_ptr->_field->baseType, type))
			{
				return type;
			}
		}
	}

	return 0;
}