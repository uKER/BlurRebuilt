#include "Core/XtWinPch.h"
#include "FinderImpl.h"

FrExpression::FrExpression(FinderDlg* finder) :
	FinderRow(finder)
{
	SetLabel("Matching &expression");

	_typeCtrl		= AddControl(new FcSelectType(this));
	_fieldCtrl		= AddControl(new FcSelectField(this));
	_compareCtrl	= AddControl(new FcCompare(this));
	_lowCtrl		= AddControl(new FcEdit(this));
	_highCtrl		= AddControl(new FcEdit(this));

	if (_typeCtrl && _fieldCtrl && _compareCtrl && _lowCtrl && _highCtrl)
	{
		_typeCtrl->SetWidth(80, 200, 1.0f);
		_fieldCtrl->SetWidth(80, 200, 1.0f);
		_compareCtrl->SetWidth(40, 80, 2.0f);
		_lowCtrl->SetWidth(40, 4096, 1.0f);
		_highCtrl->SetWidth(40, 4096, 1.0f);

		_typeCtrl->SetTypeFilter(FcSelectType::kStructs);

		_compareCtrl->SetEnabled(false);
		_lowCtrl->SetEnabled(false);
		_highCtrl->SetEnabled(false);
	}
}

FrExpression::~FrExpression()
{
}

void FrExpression::BuildQuery(Xt::Query* query)
{
	if (!_typeCtrl || !_fieldCtrl || !_compareCtrl || !_lowCtrl || !_highCtrl)
		return;

	LtStruct*				type	= AsStruct(_typeCtrl->GetSelectedType());
	Xt::FieldPath*			path	= _fieldCtrl->GetSelectedField();
	Xt::FieldExpressionCmp	cmp		= Xt::kFecNone;

	switch(_compareCtrl->GetSelectedCmp())
	{
		case kFinderCmp_Equal:			cmp = Xt::kFecEqual;			break;
		case kFinderCmp_NotEqual:		cmp = Xt::kFecNotEqual;			break;
		case kFinderCmp_Less:			cmp = Xt::kFecLess;				break;
		case kFinderCmp_LessEqual:		cmp = Xt::kFecLessEqual;		break;
		case kFinderCmp_Greater:		cmp = Xt::kFecGreater;			break;
		case kFinderCmp_GreaterEqual:	cmp = Xt::kFecGreaterEqual;		break;
		case kFinderCmp_Between:		cmp = Xt::kFecBetween;			break;
		case kFinderCmp_IsTrue:			cmp = Xt::kFecIsTrue;			break;
		case kFinderCmp_IsFalse:		cmp = Xt::kFecIsFalse;			break;
		case kFinderCmp_Contains:		cmp = Xt::kFecContains;			break;
		case kFinderCmp_DoesNotContain:	cmp = Xt::kFecDoesNotContain;	break;
		case kFinderCmp_StartsWith:		cmp = Xt::kFecStartsWith;		break;
		case kFinderCmp_EndsWith:		cmp = Xt::kFecEndsWith;			break;
		case kFinderCmp_IsNull:			cmp = Xt::kFecIsNull;			break;
		case kFinderCmp_IsNotNull:		cmp = Xt::kFecIsNotNull;		break;
		case kFinderCmp_Display:		cmp = Xt::kFecDisplay;			break;
	}

	if (!type || !path || (cmp == Xt::kFecNone))
		return;

	XtString low	= _lowCtrl->GetText();
	XtString high	= _highCtrl->GetText();

	query->AddElement(new Xt::QeFieldExpression(type, path, cmp, low, high));
}

class FrExpressionColumn : public FinderColumn
{
public:
	FrExpressionColumn(const XtString& title, Xt::DomDb* db, LarvaSchema* schema, Xt::FieldPath* path) :
		FinderColumn(title),
		_db(db),
		_schema(schema),
		_path(Clone(path))
	{
	}

	virtual ~FrExpressionColumn()
	{
		Free(_path);
	}

public:
	virtual XtString Evaluate(FinderResult* result)
	{
		Xt::FieldDom fd = At(_schema, result->type, AsBlock(result->dom), _path);

		return EvaluateSummaryText(_db, _schema, fd.field, fd.dom);
	}

	virtual Xt::FieldDom Get(FinderResult* result)
	{
		return Xt::Acquire(_schema, result->type, AsBlock(result->dom), _path);
	}

	virtual void Set(FinderResult* result, Xt::FieldDom value)
	{
		Xt::FieldDom fd = Xt::Acquire(_schema, result->type, AsBlock(result->dom), _path);

		if (Xt::DomField* dst = AsField(fd.dom))
		{
			if (Xt::DomField* src = AsField(value.dom))
			{
				Xt::Set(dst, src->value);
			}
		}
		else if (Xt::DomRef* dst = AsRef(fd.dom))
		{
			if (Xt::DomRef* src = AsRef(value.dom))
			{
				Xt::Set(dst, src->value);
			}
		}
	}

private:
	Xt::DomDb*		_db;
	LarvaSchema*	_schema;
	Xt::FieldPath*	_path;
};

void FrExpression::BuildColumns(XtUt::List<FinderColumn>* columns)
{
	Xt::FieldPath* path = _fieldCtrl->GetSelectedField();

	if (path)
	{
		FinderColumn* fc = new FrExpressionColumn(ToString(path), Finder()->Db(), Finder()->Schema(), path);
	
		XtUt::AppendEx(columns, fc);
	}
}

void FrExpression::OnEnableChanged()
{
	Sync();
}

void FrExpression::OnFocus(FinderControl* control)
{
	Sync();
}

void FrExpression::OnChanged(FinderControl* control)
{
	Sync();
}

void FrExpression::Sync()
{
	if (!_typeCtrl || !_fieldCtrl || !_compareCtrl || !_lowCtrl || !_highCtrl)
		return;

	_fieldCtrl->SetType(AsStruct(_typeCtrl->GetSelectedType()));

	bool compareEnabled	= false;
	bool lowEnabled		= false;
	bool highEnabled	= false;

	Xt::FieldPath* path = _fieldCtrl->GetSelectedField();

	if (path)
	{
		while(path->next)
			path = path->next;
	}

	if (path && path->field)
	{
		_compareCtrl->SetField(path->field);

		switch(path->field->kind)
		{
			case kLfInt:
			case kLfFloat:
				compareEnabled = true;
			break;

			case kLfString:
			case kLfWideString:
				compareEnabled = true;
			break;

			case kLfBool:
				compareEnabled = true;
			break;

			case kLfEnum:
				compareEnabled = true;
			break;

			case kLfHandle:
				compareEnabled = true;
			break;
		}
	}
	else
		_compareCtrl->SetField(0);

	FinderCmp cmp = _compareCtrl->GetSelectedCmp();

	switch(cmp)
	{
		case kFinderCmp_Equal:
		case kFinderCmp_NotEqual:
		case kFinderCmp_Less:
		case kFinderCmp_LessEqual:
		case kFinderCmp_Greater:
		case kFinderCmp_GreaterEqual:
		case kFinderCmp_Between:
		case kFinderCmp_Contains:
		case kFinderCmp_DoesNotContain:
		case kFinderCmp_StartsWith:
		case kFinderCmp_EndsWith:
			lowEnabled = true;
		break;
	}

	switch(cmp)
	{
		case kFinderCmp_Between:
			highEnabled = true;
		break;
	}

	_compareCtrl->SetEnabled(compareEnabled);
	_lowCtrl->SetEnabled(lowEnabled);
	_highCtrl->SetEnabled(highEnabled);
}