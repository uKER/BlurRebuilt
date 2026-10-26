#include "Core/XtWinPch.h"
#include "FinderImpl.h"

FcSelectRecord::FcSelectRecord(FinderRow* row) :
	FcCombo(row),
	_type(0)
{
}

FcSelectRecord::~FcSelectRecord()
{
}

void FcSelectRecord::SetType(LtStruct* type)
{
	if (_type == type)
		return;

	_type = type;

	Populate();
}

void FcSelectRecord::SetSelectedRecord(XtId recordId)
{
	for(XtUt::Array<Item>::Iterator it(&_records); it; ++it)
	{
		if (it->id == recordId)
		{
			SetSelection(it.index);
			return;
		}
	}

	SetSelection(0);
}

XtId FcSelectRecord::GetSelectedRecord()
{
	if (Item* type = At(&_records, GetSelection()))
		return type->id;

	return 0;
}

void FcSelectRecord::OnCreate()
{
	Populate();

	FcCombo::OnCreate();
}

void FcSelectRecord::OnFocus()
{
	Populate();

	FcCombo::OnFocus();
}

void FcSelectRecord::OnEnableChanged()
{
	Populate();

	FcCombo::OnEnableChanged();
}

bool FcSelectRecord::SortCompare(Item& lhs, Item& rhs)
{
	return _stricmp(lhs.label, rhs.label) < 0;
}

void FcSelectRecord::Populate()
{
	if (!_hwnd)
		return;

	XtId sel = GetSelectedRecord();

	Free(&_records);

	if (_type != 0)
	{
		if (Xt::DomDb* db = Row()->Finder()->Db())
		{
			for(Xt::DomDb::Iterator itPack(db); itPack; ++itPack)
			{
				for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
				{
					LtStruct* type = TypeOf(db->schema, itRecord);

					if (!type)
						continue;

					if (!IsBaseOf(_type, type))
						continue;

					if (Item* item = XtUt::AppendExN(&_records, 1))
					{
						item->label	= itRecord->name + "." + type->label;
						item->id	= itRecord->id;
					}
				}
			}

			XtUt::HeapSort(&_records, SortCompare);
		}
	}

	Clear();

	WinTextMeasurer wtm;
	WinSize sz;

	for(XtUt::Array<Item>::Iterator it(&_records); it; ++it)
	{
		AddString(it->label);
		sz = wtm.Measure(it->label, sz);
	}

	SetSelectedRecord(sel);
	SetDroppedWidth(sz.cx + 32);
}