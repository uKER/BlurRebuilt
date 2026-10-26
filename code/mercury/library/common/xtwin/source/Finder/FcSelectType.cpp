#include "Core/XtWinPch.h"
#include "FinderImpl.h"

FcSelectType::FcSelectType(FinderRow* row) :
	FcCombo(row),
	_filter(kNone)
{
}

FcSelectType::~FcSelectType()
{
}

void FcSelectType::SetTypeFilter(Filter filter)
{
	if (_filter == filter)
		return;

	_filter = filter;

	Populate();
}

void FcSelectType::SetSelectedType(LarvaType* type)
{
	for(XtUt::Array<LarvaType*>::Iterator it(&_types); it; ++it)
	{
		if (it == type)
		{
			SetSelection(it.index);
			return;
		}
	}

	SetSelection(0);
}

LarvaType* FcSelectType::GetSelectedType()
{
	if (LarvaType** type = At(&_types, GetSelection()))
		return *type;

	return 0;
}

void FcSelectType::OnCreate()
{
	Populate();

	FcCombo::OnCreate();
}

void FcSelectType::OnFocus()
{
	Populate();

	FcCombo::OnFocus();
}

void FcSelectType::OnEnableChanged()
{
	Populate();

	FcCombo::OnEnableChanged();
}

namespace
{
	bool SortCompare(LarvaType* lhs, LarvaType* rhs)
	{
		return _stricmp(lhs->label, rhs->label) < 0;
	}
}

void FcSelectType::Populate()
{
	if (!_hwnd)
		return;

	LarvaType* sel = GetSelectedType();

	Free(&_types);

	if (_filter != kNone)
	{
		if (LarvaSchema* schema = Row()->Finder()->Schema())
		{
			for(LarvaSchema::TypeIterator it(&schema->types); it; ++it)
			{
				LtStruct* type = AsStruct(it);

				if (!type)
					continue;

				if ((_filter == kRecords) && !type->isRecord)
					continue;

				XtUt::AppendEx(&_types, (LarvaType*)type);
			}

			XtUt::HeapSort(&_types, SortCompare);
		}
	}

	Clear();

	WinTextMeasurer wtm;
	WinSize sz;

	for(XtUt::Array<LarvaType*>::Iterator it(&_types); it; ++it)
	{
		AddString((*it)->label);
		sz = wtm.Measure((*it)->label, sz);
	}

	SetSelectedType(sel);
	SetDroppedWidth(sz.cx + 32);
}