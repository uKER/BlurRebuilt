#include "Core/XtWinPch.h"
#include "FinderImpl.h"

FrRecordOfType::FrRecordOfType(FinderDlg* finder) :
	FinderRow(finder)
{
	SetLabel("Only records of &type");

	if ((_typeCtrl = AddControl(new FcSelectType(this))) != 0)
	{
		_typeCtrl->SetTypeFilter(FcSelectType::kRecords);
	}
}

FrRecordOfType::~FrRecordOfType()
{
}

void FrRecordOfType::BuildQuery(Xt::Query* query)
{
	if (!_typeCtrl)
		return;

	if (LtStruct* type = AsStruct(_typeCtrl->GetSelectedType()))
		query->AddElement(new Xt::QeByRecordType(type));
}