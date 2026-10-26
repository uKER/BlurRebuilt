#include "Core/XtWinPch.h"
#include "FinderImpl.h"

FrDependsOn::FrDependsOn(FinderDlg* finder) :
	FinderRow(finder)
{
	SetLabel("Which &depend on record");

	_typeCtrl		= AddControl(new FcSelectType(this));
	_recordsCtrl	= AddControl(new FcCombo(this));

	if (_typeCtrl && _recordsCtrl)
	{
		_typeCtrl->SetWidth(80, 200, 1.0f);
		_recordsCtrl->SetWidth(120, 4096, 1.0f);

		_typeCtrl->SetTypeFilter(FcSelectType::kRecords);
	}
}

FrDependsOn::~FrDependsOn()
{
}

void FrDependsOn::BuildQuery(Xt::Query* query)
{
	if (!_typeCtrl || !_recordsCtrl)
		return;

	if (Item* item = At(&_records, _recordsCtrl->GetSelection()))
		query->AddElement(new Xt::QeDependsOnRecord(item->id));
}

class FrDependsOnColumn : public FinderColumn
{
	Xt::DomDb* _db;

public:
	FrDependsOnColumn(const XtString& title, Xt::DomDb* db) :
		FinderColumn(title),
		_db(db)
	{
	}

public:
	virtual XtString Evaluate(FinderResult* result)
	{
		if (Xt::DomRecord* record = FindRecordById(_db, Xt::ParseId(AsText(result->dom))))
		{
			LtStruct* type = TypeOf(_db->schema, record);

			return record->name + "." + (type ? type->label : record->root.type);
		}

		return kXtNullString;
	}
};

void FrDependsOn::BuildColumns(XtUt::List<FinderColumn>* columns)
{
	FinderColumn* fc = new FrDependsOnColumn("Value", Finder()->Db());

	XtUt::AppendEx(columns, fc);
}

void FrDependsOn::OnEnableChanged()
{
	if (IsEnabled())
		PopulateRecords();
}

void FrDependsOn::OnFocus(FinderControl* control)
{
	PopulateRecords();
}

void FrDependsOn::OnChanged(FinderControl* control)
{
	PopulateRecords();
}

void FrDependsOn::SelectRecord(Xt::DomRecord* record)
{
	if (!record)
		return;

	LarvaType* type = TypeOf(Finder()->Db()->schema, record);

	if (!type)
		return;

	if (_typeCtrl->GetSelectedType() != type)
	{
		_typeCtrl->SetSelectedType(type);

		PopulateRecords();
	}

	for(XtUt::Array<Item>::Iterator it(&_records); it; ++it)
	{
		if (it->id == record->id)
		{
			_recordsCtrl->SetSelection(it.index);
		}
	}
}

namespace
{
	bool SortCompare(const FrDependsOn::Item& lhs, const FrDependsOn::Item& rhs)
	{
		return _stricmp(lhs.text, rhs.text) < 0;
	}
}

void FrDependsOn::PopulateRecords()
{
	if (!_typeCtrl || !_recordsCtrl)
		return;

	LtStruct* filter = AsStruct(_typeCtrl->GetSelectedType());

	Free(&_records);

	Xt::DomDb* db = Finder()->Db();

	for(Xt::DomDb::Iterator itPack(db); itPack; ++itPack)
	{
		for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
		{
			if (filter && !IsBaseOf(filter, TypeOf(db->schema, itRecord)))
				continue;

			XtUt::AppendEx(&_records, Item(itRecord->name, itRecord->id));
		}
	}

	XtUt::HeapSort(&_records, SortCompare);

	int sel = _recordsCtrl->GetSelection();

	_recordsCtrl->Clear();

	WinTextMeasurer wtm;
	WinSize sz;

	for(XtUt::Array<Item>::Iterator it(&_records); it; ++it)
	{
		_recordsCtrl->AddString(it->text);
		sz = wtm.Measure(it->text, sz);
	}

	_recordsCtrl->SetSelection(Clamp<int>(sel, 0, _records.count - 1));
	_recordsCtrl->SetDroppedWidth(sz.cx + 32);
}