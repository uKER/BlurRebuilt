#include "Core/XtWinPch.h"
#include "FinderImpl.h"

FrUsingLocString::FrUsingLocString(FinderDlg* finder) :
	FinderRow(finder)
{
	SetLabel("Using &loc string");

	_recordCtrl = AddControl(new FcSelectRecord(this));
	_stringCtrl = AddControl(new FcSelectLocString(this));

	if (_recordCtrl && _stringCtrl)
	{
		_recordCtrl->SetType(AsStruct(FindTypeByName(finder->Schema(), "XtLocTable")));

		_recordCtrl->SetWidth(80, 200, 1.0f);
		_stringCtrl->SetWidth(120, 4096, 1.0f);
	}
}

FrUsingLocString::~FrUsingLocString()
{
}

void FrUsingLocString::BuildQuery(Xt::Query* query)
{
	if (!_recordCtrl || !_stringCtrl)
		return;

	if (XtId id = _stringCtrl->GetSelectedId())
		query->AddElement(new Xt::QeUsingLocString(id));
}

void FrUsingLocString::OnEnableChanged()
{
	_stringCtrl->SetRecord(_recordCtrl->GetSelectedRecord());
}

void FrUsingLocString::OnFocus(FinderControl* control)
{
	_stringCtrl->SetRecord(_recordCtrl->GetSelectedRecord());
}

void FrUsingLocString::OnChanged(FinderControl* control)
{
	_stringCtrl->SetRecord(_recordCtrl->GetSelectedRecord());
}
