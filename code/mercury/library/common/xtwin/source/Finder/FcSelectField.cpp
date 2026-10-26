#include "Core/XtWinPch.h"
#include "FinderImpl.h"

FcSelectField::FcSelectField(FinderRow* row) :
	FcCombo(row),
	_type(0)
{
}

FcSelectField::~FcSelectField()
{
}

void FcSelectField::SetType(LtStruct* type)
{
	if (_type == type)
		return;

	_type = type;

	SetSelection(0);

	Populate();
}

void FcSelectField::SetSelectedField(Xt::FieldPath* path)
{
	for(XtUt::List<Item>::Iterator it(&_fields); it; ++it)
	{
		if (Equal(&it->path, path))
		{
			SetSelection(it.index);
			return;
		}
	}

	SetSelection(0);
}

Xt::FieldPath* FcSelectField::GetSelectedField()
{
	if (Item* item = At(&_fields, GetSelection()))
		return &item->path;

	return 0;
}

void FcSelectField::OnCreate()
{
	Populate();

	FcCombo::OnCreate();
}

void FcSelectField::OnFocus()
{
	Populate();

	FcCombo::OnFocus();
}

void FcSelectField::OnEnableChanged()
{
	Populate();

	FcCombo::OnEnableChanged();
}

namespace
{
	bool SortCompare(LarvaField* lhs, LarvaField* rhs)
	{
		return _stricmp(lhs->label, rhs->label) < 0;
	}
}

namespace
{
	Xt::FieldPath* AddToList(XtUt::List<FcSelectField::Item>* list, Xt::FieldPath* path)
	{
		if (path->next)
		{
			Xt::FieldPath* root = AddToList(list, path->next);

			if (!root)
				return 0;

			root->next = new Xt::FieldPath(path->field);

			return root->next;
		}

		FcSelectField::Item* item = XtUt::AppendEx(list, new FcSelectField::Item);

		if (!item)
			return 0;

		item->path.field = path->field;

		return &item->path;
	}

	void BuildList(LtStruct* type, Xt::FieldPath* root, XtUt::List<FcSelectField::Item>* list)
	{
		if (!type || !list)
			return;

		if (type->base)
			BuildList(type->base->type, root, list);

		for(LtStruct::FieldIterator itField(&type->fields); itField; ++itField)
		{
			Xt::FieldPath path(itField, root);

			switch(itField->kind)
			{
				case kLfStruct:
					BuildList(((LfStruct*)&*itField)->type, &path, list);
				break;

				case kLfInt:
				case kLfFloat:
				case kLfString:
				case kLfWideString:
				case kLfBool:
				case kLfEnum:
				//case kLfFlags:
				//case kLfStrictPtr:
				//case kLfPtr:
				//case kLfWeakPtr:
				case kLfHandle:
				//case kLfArray:
				//case kLfLocString:
					AddToList(list, &path);
				break;
			}
		}
	}
}

void FcSelectField::Populate()
{
	if (!_hwnd)
		return;

	Free(&_fields);

	BuildList(_type, 0, &_fields);

	for(XtUt::List<Item>::Iterator it(&_fields); it; ++it)
		it->text = ToString(&it->path);

	int sel = GetSelection();

	Clear();

	WinTextMeasurer wtm;
	WinSize sz;

	for(XtUt::List<Item>::Iterator it(&_fields); it; ++it)
	{
		AddString(it->text);
		sz = wtm.Measure(it->text, sz);
	}

	SetSelection(Clamp<int>(sel, 0, _fields.count - 1));
	SetDroppedWidth(sz.cx + 32);
}
