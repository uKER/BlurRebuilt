#include "Core/XtWinPch.h"
#include "FinderImpl.h"

FcSelectLocString::FcSelectLocString(FinderRow* row) :
	FcCombo(row),
	_recordId(0)
{
}

FcSelectLocString::~FcSelectLocString()
{
}

void FcSelectLocString::SetRecord(XtId recordId)
{
	if (_recordId == recordId)
		return;

	_recordId = recordId;

	Populate();
}

void FcSelectLocString::SetSelectedId(XtId locId)
{
	for(XtUt::Array<Item>::Iterator it(&_strings); it; ++it)
	{
		if (it->id == locId)
		{
			SetSelection(it.index);
			return;
		}
	}

	SetSelection(0);
}

XtId FcSelectLocString::GetSelectedId()
{
	if (Item* string = At(&_strings, GetSelection()))
		return string->id;

	return 0;
}

void FcSelectLocString::OnCreate()
{
	Populate();

	FcCombo::OnCreate();
}

void FcSelectLocString::OnFocus()
{
	Populate();

	FcCombo::OnFocus();
}

void FcSelectLocString::OnEnableChanged()
{
	Populate();

	FcCombo::OnEnableChanged();
}

bool FcSelectLocString::SortCompare(Item& lhs, Item& rhs)
{
	return _stricmp(lhs.label, rhs.label) < 0;
}

void FcSelectLocString::Populate()
{
	if (!_hwnd)
		return;

	XtId sel = GetSelectedId();

	Free(&_strings);

	if (_recordId != 0)
	{
		if (Xt::DomDb* db = Row()->Finder()->Db())
		{
			if (Xt::DomRecord* locTable = Xt::FindRecordById(db, _recordId))
			{
				if (Xt::DomArray* strings = AsArray(At(&locTable->root.data, "strings")))
				{
					for(Xt::DomArray::ElementIterator it(strings); it; ++it)
					{
						if (Item* item = XtUt::AppendExN(&_strings, 1))
						{
							item->label	= AsText(At(AsBlock(it), "name"));
							item->id	= it.Key();

						}
					}
				}
			}

			XtUt::HeapSort(&_strings, SortCompare);
		}
	}

	Clear();

	WinTextMeasurer wtm;
	WinSize sz;

	for(XtUt::Array<Item>::Iterator it(&_strings); it; ++it)
	{
		AddString(it->label);
		sz = wtm.Measure(it->label, sz);
	}

	SetSelectedId(sel);
	SetDroppedWidth(sz.cx + 32);
}