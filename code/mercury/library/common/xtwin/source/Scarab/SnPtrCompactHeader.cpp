#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

SnPtrCompactHeader::SnPtrCompactHeader(SnPtr* ptr, bool hyperLink) :
	_ptr(ptr),
	_hyperLink(hyperLink),
	_idButton(this)
{
	_dom.SetCallback(DomCallback, this);
}

SnPtrCompactHeader::~SnPtrCompactHeader()
{
}

void SnPtrCompactHeader::OnBind(Xt::DomComponent* parentDom)
{
	SetDom(parentDom);
}

bool SnPtrCompactHeader::OnBuild()
{
	if (!_idButton.Allocate())
		return false;

	if (_button == 0)
	{
		if (_hyperLink)
		{
			if (!_button.Create(0, WC_LINK, L"", WS_CHILD | WS_TABSTOP, *Panel(), _idButton))
				return false;

			SendMessage(_button, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);
		}
		else
		{
			if (!_button.Create(0, WC_BUTTON, _T("*"), BS_FLAT | WS_CHILD | WS_TABSTOP, *Panel(), _idButton))
				return false;

			SendMessage(_button, WM_SETFONT, (WPARAM)Panel()->DrawContext()->iconFont, FALSE);

			SetWindowTextW(_button, L"7");
		}
	}

	PopulateTypeList();
	SyncControls();

	return true;
}

void SnPtrCompactHeader::OnMeasure(HDC ic)
{
	if (_hyperLink)
	{
		const XtString& label = GetTypeName();

		WinSize sz;
		GetTextExtentPoint32(ic, label, label.Length(), &sz);

		_hyperSize	= sz;
		_size		= WinSize(sz.cx, sz.cy + 7);
	}
	else
	{
		_hyperSize	= WinSize(16, 16);
		_size		= WinSize(16, 20);
	}
}

void SnPtrCompactHeader::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_button, _client.left, _client.top + (_hyperLink ? 3 : 2), _hyperSize.cx, _hyperSize.cy, SWP_SHOWWINDOW);
}

void SnPtrCompactHeader::OnCommand(int id, int code)
{
	LarvaSchema*		schema		= Panel()->Db()->schema;
	Xt::DomComponent*	component	= AsComponent(_dom);

	if ((id == _idButton) && (code == BN_CLICKED) && component)
		DoPopup();
}

LRESULT SnPtrCompactHeader::OnNotify(NMHDR* hdr)
{
	LarvaSchema*		schema	= Panel()->Db()->schema;
	Xt::DomComponent*	com		= AsComponent(_dom);

	if ((hdr->idFrom == _idButton) && (hdr->code == NM_CLICK) && com)
		DoPopup();

	return 0;
}

void SnPtrCompactHeader::SetDom(Xt::DomComponent* com)
{
	SetEnabled(Monitor(&_dom, com) != 0);

	SyncControls();
}

void SnPtrCompactHeader::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnPtrCompactHeader* node)
{
	if (event->code == Xt::kDomRemoved)
		node->SetDom(0);
	else if (*monitor == event->from)
	{
		node->SyncControls();
		node->_ptr->OnTypeChanged();
	}
}

bool SnPtrCompactHeader::TypeListCompare(LtStruct* lhs, LtStruct* rhs)
{
	return _stricmp(lhs->label, rhs->label) < 0;
}

void SnPtrCompactHeader::SyncControls()
{
	LarvaSchema*		schema			= Panel()->Db()->schema;
	Xt::DomComponent*	com				= AsComponent(_dom);
	BOOL				enableControls	= com ? TRUE : FALSE;

	EnableWindow(_button, enableControls);

	if (_hyperLink)
		SetWindowTextW(_button, Format(L"<A>%S</A>", GetTypeName().Text()));
}

void SnPtrCompactHeader::PopulateTypeList()
{
	LarvaSchema* schema = Panel()->Db()->schema;

	Free(&_typeList);

	for(XtUt::Dictionary<XtString, LarvaType>::Iterator it(&schema->types); it; ++it)
	{
		if (it->kind != kLtStruct)
			continue;

		LtStruct* type = (LtStruct*)(LarvaType*)it;

		if (!IsBaseOf(_ptr->_field->baseType, type))
			continue;

		if (type->hidden)
			continue;

		AppendEx(&_typeList, type);
	}

	XtUt::MergeSort(&_typeList, TypeListCompare);
}

XtString SnPtrCompactHeader::GetTypeName()
{
	LarvaSchema*		schema	= Panel()->Db()->schema;
	Xt::DomComponent*	com		= AsComponent(_dom);

	if (com)
	{
		LtStruct* type = TypeOf(schema, _ptr->_field->baseType, com);

		if (type)
		{
			for(XtUt::Array<LtStruct*>::Iterator it(&_typeList); it; ++it)
			{
				if (type == it)
					return type->label;
			}
		}
	}

	return "(null)";
}

void SnPtrCompactHeader::DoPopup()
{
	LarvaSchema*		schema	= Panel()->Db()->schema;
	Xt::DomComponent*	com		= AsComponent(_dom);

	if (com)
	{
		WinRect rc;
		GetWindowRect(_button, &rc);

		LtStruct* type = TypeOf(schema, _ptr->_field->baseType, com);

		HMENU menu = CreatePopupMenu();

		AppendMenu(menu, MF_STRING | ((type == 0) ? MF_CHECKED : 0), 100, "(null)");

		for(XtUt::Array<LtStruct*>::Iterator it(&_typeList); it; ++it)
			AppendMenu(menu, MF_STRING | ((it == type) ? MF_CHECKED : 0), 101 + it.index, (*it)->label);

		int result = TrackPopupMenu(menu, TPM_NONOTIFY | TPM_RIGHTBUTTON | TPM_RETURNCMD, rc.left, rc.bottom, 0, *Panel(), 0) - 100;

		DestroyMenu(menu);

		if ((result >= 0) && (result <= (int)_typeList.count))
		{
			if (result > 0)
			{
				LtStruct* type = _typeList[result - 1];

				SetType(com, type->name);

				SetDefaults(schema, type, com);
			}
			else
				SetType(com, kXtNullString);

			_ptr->OnTypeChanged();
		}
	}
}