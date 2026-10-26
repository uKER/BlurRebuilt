#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

// Pastel coloured bars in background of edit boxes, allow middle click to drag this slider up and down

SnPropertySheet::SnPropertySheet(LfControl* field) :
	SnControl(field),
	_dom(DomCallback, this),
	_idSheet(this)
{
}

SnPropertySheet::~SnPropertySheet()
{
}

void SnPropertySheet::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(parentDom);
}

void SnPropertySheet::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireBlock(parentDom, index));
}

bool SnPropertySheet::OnBuild()
{
	if (!_idSheet.Allocate())
		return false;

	if ((HWND)_wndSheet == 0)
	{
		if (!_wndSheet.Create(*Panel(), _idSheet.IdAsHMenu()))
			return false;
	}

	return true;
}

void SnPropertySheet::OnMeasure(HDC ic)
{
	_size = WinSize(300, 300);
}

void SnPropertySheet::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_wndSheet, _client.left, _client.top, _client.Width(), _client.Height(), SWP_SHOWWINDOW);
}

void SnPropertySheet::SetDom(Xt::DomBlock* block)
{
	Monitor(&_dom, block);

	SyncControls();
}

void SnPropertySheet::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnPropertySheet* sheet)
{
	if ((event->code == Xt::kDomRemoved) && (event->from == monitor->element))
		sheet->SetDom(0);
}

typedef XtUt::List<Xt::DomMonitor> PsDomList;

class SnPsRow : public PsRow
{
public:
	SnPsRow(Xt::DomDb* db, LarvaType* type, LarvaField* field) :
		_db(db),
		_type(0),
		_field(field)
	{
		if (!_type && field)
			_type = field->Type();
	}

public:
	virtual void Bind()
	{
	}

public:
	PsDomList* Doms() { return &_doms; }

public:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnPsRow* row)
	{
		row->Bind();
	}

protected:
	Xt::DomDb*	_db;
	LarvaType*	_type;
	LarvaField*	_field;
	PsDomList	_doms;
};

bool Equal(LarvaField* field, Xt::DomElement* lhs, Xt::DomElement* rhs)
{
	if (!field || !lhs || !rhs)
		return false;

	switch(field->kind)
	{
		case kLfInt:
		case kLfFloat:
		case kLfString:
		case kLfWideString:
		return AsText(lhs) == AsText(rhs);
	}

	return false;
}

bool Equal(LarvaField* field, PsDomList* doms)
{
	if (!field || !doms)
		return false;

	PsDomList::Iterator it(doms);

	if (!it)
		return false;

	Xt::DomElement* first = *it;

	for(++it; it && first; ++it)
	{
		if (!Equal(field, first, *it))
			first = 0;
	}

	return first != 0;
}

class SnPsRow_String : public SnPsRow
{
public:
	SnPsRow_String(Xt::DomDb* db, LarvaType* type, LarvaField* field) :
		SnPsRow(db, type, field)
	{
		_edit = (PsEdit*)AddControl(new PsEdit);
	}

public:
	virtual void Bind()
	{
		if (!_edit)
			return;

		switch(Count(&_doms))
		{
			case 0:
				_edit->SetText(L"");
			break;

			case 1:
			{
				if (Xt::DomMonitor* mon = At(&_doms, 0))
					_edit->SetText(MbToWc(AsText(*mon)));
				else
					_edit->SetText(L"");
			}
			break;

			default:
			{
				if (Equal(_field, &_doms))
				{
					if (Xt::DomMonitor* mon = At(&_doms, 0))
						_edit->SetText(MbToWc(AsText(*mon)));
					else
						_edit->SetText(L"");
				}
				else
					_edit->SetText(L"(multiple)");
			}
			break;
		}
	}

private:
	PsEdit* _edit;
};

struct SnPropSheetItem
{
	LtStruct*		type;
	Xt::DomMonitor	dom;

public:
	SnPropSheetItem() :
		type(0)
	{
	}
};

void DoBuildStruct(Xt::DomDb* db, SnPsRow* parent, LtStruct* type);

struct PsRowBuilder : public LvField
{
	Xt::DomDb*	_db;
	SnPsRow*	_parent;
	SnPsRow*	_result;

public:
	PsRowBuilder(Xt::DomDb* db, SnPsRow* parent) :
		_db(db),
		_parent(parent),
		_result(0)
	{
	}

public:
	PsRow* SetupEx(LarvaField* field, SnPsRow* row)
	{
		if (row)
		{
			delete _result;

			_result = row;

			_parent->AddRow(row);

			// Label

			row->SetLabel(MbToWc(field->label));

			// Acquire children

			Reserve(row->Doms(), Count(_parent->Doms()));

			for(PsDomList::Iterator itDom(_parent->Doms()); itDom; ++itDom)
			{
				Xt::DomMonitor* mon = XtUt::Append(row->Doms(), new Xt::DomMonitor);

				if (!mon)
					return false;

				mon->SetCallback(SnPsRow::DomCallback, row);

				Monitor(mon, Acquire(AsBlock(*itDom), field));
			}

			// Bind
			
			row->Bind();
		}

		return row;
	}

	template<typename Type> Type* Setup(LarvaField* field, Type* row)
	{
		return (Type*)SetupEx(field, row);
	}

public:
	virtual bool Visit(LfStruct* field)
	{
		SnPsRow* row = Setup(field, new SnPsRow(_db, 0, field));

		if (!row)
			return false;

		DoBuildStruct(_db, row, field->type);

		return true;
	}

	virtual bool Visit(LfInt* field)		{ return Setup(field, new SnPsRow_String(_db, 0, field)) != 0; }
	virtual bool Visit(LfFloat* field)		{ return Setup(field, new SnPsRow_String(_db, 0, field)) != 0; }
	virtual bool Visit(LfString* field)		{ return Setup(field, new SnPsRow_String(_db, 0, field)) != 0; }
	virtual bool Visit(LfWideString* field)	{ return Setup(field, new SnPsRow_String(_db, 0, field)) != 0; }

	virtual bool Visit(LfBool* field)		{ return Setup(field, new SnPsRow(_db, 0, field)) != 0; }
	virtual bool Visit(LfEnum* field)		{ return Setup(field, new SnPsRow(_db, 0, field)) != 0; }
	virtual bool Visit(LfFlags* field)		{ return Setup(field, new SnPsRow(_db, 0, field)) != 0; }
	virtual bool Visit(LfStrictPtr* field)	{ return Setup(field, new SnPsRow(_db, 0, field)) != 0; }

	virtual bool Visit(LfPtr* field)
	{
		PsRow* row = Setup(field, new SnPsRow(_db, 0, field));

		if (!row)
			return false;

		if (PsRow* rowType = row->AddRow(new SnPsRow(_db, 0, field)))
		{
			rowType->SetLabel(L"Type"); // TODO: build data, multiple dom passed in...
		}

		return true;
	}

	virtual bool Visit(LfWeakPtr* field)	{ return Setup(field, new SnPsRow(_db, 0, field)) != 0; }
	virtual bool Visit(LfHandle* field)		{ return Setup(field, new SnPsRow(_db, 0, field)) != 0; }
	virtual bool Visit(LfArray* field)		{ return Setup(field, new SnPsRow(_db, 0, field)) != 0; }
	virtual bool Visit(LfLocString* field)	{ return Setup(field, new SnPsRow(_db, 0, field)) != 0; }

	virtual bool Visit(LfControl* field)
	{
		return true;
	}
};

void DoBuildStruct(Xt::DomDb* db, SnPsRow* parent, LtStruct* type)
{
	if (type->base && !type->base->hidden)
		DoBuildStruct(db, parent, type->base->type);

	for(LtStruct::FieldIterator itField(&type->fields); itField; ++itField)
	{
		if (itField->hidden)
			continue;

		PsRowBuilder b(db, parent);

		itField->Accept(&b);
	}
}

void SnPropertySheet::SyncControls()
{
	Xt::DomDb* db = Db();
	LarvaSchema* schema = Schema();
	LtStruct* rootType = _field->container;
	LfArray* fieldItem = (LfArray*)FindFieldByName(rootType, "item");
	Xt::DomArray* array = AsArray(At(AsBlock(_dom), "item"));

	XtUt::List<SnPropSheetItem> items;

	for(Xt::DomArray::IndexIterator it(array); it; ++it)
	{
		Xt::DomComponent* com = AsComponent(it);
		LtStruct* type = AsStruct(TypeOf(schema, fieldItem->field, com));

		if (com && type)
		{
			if (!atoi(AsText(At(&com->data, "active"))))
				continue;

			if (SnPropSheetItem* item = AppendEx(&items, new SnPropSheetItem))
			{
				item->type = type;
				Monitor(&item->dom, &com->data);
			}
		}
	}

	LtStruct* commonType = 0;

	if (Count(&items) > 0)
	{
		XtUt::List<SnPropSheetItem>::Iterator it(&items);

		commonType = it->type;

		for(++it; it && commonType; ++it)
			commonType = FindCommonBase(commonType, it->type);
	}

	if (commonType)
	{
		SnPsRow* rootRow = _wndSheet.Diagram()->SetRoot(new SnPsRow(db, commonType, 0));

		if (rootRow)
		{
			PsDomList* childDoms = rootRow->Doms();

			Reserve(childDoms, Count(&items));

			for(XtUt::List<SnPropSheetItem>::Iterator itDom(&items); itDom; ++itDom)
			{
				Xt::DomMonitor* mon = XtUt::Append(childDoms, new Xt::DomMonitor);

				if (!mon)
					break;

				Monitor(mon, AsBlock(itDom->dom));
			}

			DoBuildStruct(db, rootRow, commonType);
		}
	}
}