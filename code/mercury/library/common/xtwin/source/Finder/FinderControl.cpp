#include "Core/XtWinPch.h"
#include "FinderImpl.h"

// FinderControlId

FinderControlId::FinderControlId(FinderControl* control) :
	_id(0),
	_control(control)
{
}

FinderControlId::~FinderControlId()
{
	Free();
}

bool FinderControlId::Allocate()
{
	if (!_id)
	{
		FinderRow* row = _control->Row();

		if (!row)
			return false;

		_id = row->AllocateControlId(_control);
	}

	return _id != 0;
}

void FinderControlId::Free()
{
	FinderRow* row = _control->Row();

	if (row && _id)
	{
		row->FreeControlId(_id);
		_id = 0;
	}
}

//
// FinderControl
//

FinderControl::FinderControl(FinderRow* row) :
	_row(row),
	_widthMin(32),
	_widthMax(4096),
	_widthWeight(1.0f),
	_widthActual(0),
	_enabled(true)
{
}

FinderControl::~FinderControl()
{
}

void FinderControl::SetWidth(int min, int max, float weight)
{
	_widthMin		= min;
	_widthMax		= max;
	_widthWeight	= weight;

	_row->Finder()->DeferLayout();
}

void FinderControl::SetEnabled(bool enabled)
{
	if (_enabled == enabled)
		return;

	bool wasEnabled = IsEnabled();

	_enabled = enabled;

	OnEnableChanged();

	if (wasEnabled != IsEnabled())
		_row->Finder()->DeferLayout();
}

bool FinderControl::Create()
{
	return true;
}

void FinderControl::Layout(WinRect& rc)
{
}

void FinderControl::OnCommand(int id, int code)
{
}

void FinderControl::OnCreate()
{
}

void FinderControl::OnFocus()
{
	if (_row)
		_row->OnFocus(this);
}

void FinderControl::OnChanged()
{
	if (_row)
		_row->OnChanged(this);
}

void FinderControl::OnEnableChanged()
{
}