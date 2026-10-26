#include "Core/XtWinPch.h"
#include "FinderImpl.h"

FcCompare::FcCompare(FinderRow* row) :
	FcCombo(row),
	_field(0)
{
}

FcCompare::~FcCompare()
{
}

void FcCompare::SetField(LarvaField* field)
{
	if (_field == field)
		return;

	_field = field;

	Populate();
}

void FcCompare::SetSelectedCmp(FinderCmp cmp)
{
	for(XtUt::Array<FinderCmp>::Iterator it(&_cmps); it; ++it)
	{
		if (it == cmp)
		{
			SetSelection(it.index);
			return;
		}
	}

	SetSelection(0);
}

FinderCmp FcCompare::GetSelectedCmp()
{
	if (FinderCmp* cmp = At(&_cmps, GetSelection()))
		return *cmp;

	return kFinderCmp_None;
}

void FcCompare::OnCreate()
{
	Populate();

	FcCombo::OnCreate();
}

void FcCompare::OnFocus()
{
	Populate();

	FcCombo::OnFocus();
}

void FcCompare::OnEnableChanged()
{
	Populate();

	FcCombo::OnEnableChanged();
}

void FcCompare::Populate()
{
	if (!_hwnd)
		return;

	FinderCmp sel = GetSelectedCmp();

	Free(&_cmps);

	if (_field)
	{
		switch(_field->kind)
		{
			case kLfInt:
			case kLfFloat:
				XtUt::AppendEx(&_cmps, kFinderCmp_Between);
				XtUt::AppendEx(&_cmps, kFinderCmp_Equal);
				XtUt::AppendEx(&_cmps, kFinderCmp_NotEqual);
				XtUt::AppendEx(&_cmps, kFinderCmp_Less);
				XtUt::AppendEx(&_cmps, kFinderCmp_LessEqual);
				XtUt::AppendEx(&_cmps, kFinderCmp_Greater);
				XtUt::AppendEx(&_cmps, kFinderCmp_GreaterEqual);
			break;

			case kLfString:
			case kLfWideString:
				XtUt::AppendEx(&_cmps, kFinderCmp_Contains);
				XtUt::AppendEx(&_cmps, kFinderCmp_DoesNotContain);
				XtUt::AppendEx(&_cmps, kFinderCmp_StartsWith);
				XtUt::AppendEx(&_cmps, kFinderCmp_EndsWith);
				XtUt::AppendEx(&_cmps, kFinderCmp_Equal);
				XtUt::AppendEx(&_cmps, kFinderCmp_NotEqual);
			break;

			case kLfBool:
				XtUt::AppendEx(&_cmps, kFinderCmp_IsTrue);
				XtUt::AppendEx(&_cmps, kFinderCmp_IsFalse);
			break;

			case kLfEnum:
				XtUt::AppendEx(&_cmps, kFinderCmp_Equal);
				XtUt::AppendEx(&_cmps, kFinderCmp_NotEqual);
			break;

			case kLfHandle:
				XtUt::AppendEx(&_cmps, kFinderCmp_IsNull);
				XtUt::AppendEx(&_cmps, kFinderCmp_IsNotNull);
			break;
		}

		XtUt::AppendEx(&_cmps, kFinderCmp_Display);
	}

	Clear();

	WinTextMeasurer wtm;
	WinSize sz;

	for(XtUt::Array<FinderCmp>::Iterator it(&_cmps); it; ++it)
	{
		const char* text = "?cmp?";

		switch(*it)
		{
			case kFinderCmp_Equal:			text = "==";			break;
			case kFinderCmp_NotEqual:		text = "!=";			break;
			case kFinderCmp_Less:			text = "<";				break;
			case kFinderCmp_LessEqual:		text = "<=";			break;
			case kFinderCmp_Greater:		text = ">";				break;
			case kFinderCmp_GreaterEqual:	text = ">=";			break;
			case kFinderCmp_Between:		text = "between";		break;
			case kFinderCmp_IsTrue:			text = "is true";		break;
			case kFinderCmp_IsFalse:		text = "is false";		break;
			case kFinderCmp_Contains:		text = "contains";			break;
			case kFinderCmp_DoesNotContain:	text = "does not contain";	break;
			case kFinderCmp_StartsWith:		text = "starts with";		break;
			case kFinderCmp_EndsWith:		text = "ends with";			break;
			case kFinderCmp_IsNull:			text = "is null";			break;
			case kFinderCmp_IsNotNull:		text = "is not null";		break;
			case kFinderCmp_Display:		text = "(display)";		break;
		}

		AddString(text);
		sz = wtm.Measure(text, sz);
	}

	SetSelectedCmp(sel);
	SetDroppedWidth(sz.cx + 32);
}
