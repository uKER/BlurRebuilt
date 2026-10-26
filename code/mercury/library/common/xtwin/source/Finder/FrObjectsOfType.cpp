#include "Core/XtWinPch.h"
#include "FinderImpl.h"

FrObjectsOfType::FrObjectsOfType(FinderDlg* finder) :
	FinderRow(finder)
{
	SetLabel("&All objects of type");

	if ((_typeCtrl = AddControl(new FcSelectType(this))) != 0)
	{
		_typeCtrl->SetTypeFilter(FcSelectType::kStructs);
	}
}

FrObjectsOfType::~FrObjectsOfType()
{
}

void FrObjectsOfType::BuildQuery(Xt::Query* query)
{
	if (!_typeCtrl)
		return;

	if (LtStruct* type = AsStruct(_typeCtrl->GetSelectedType()))
		query->AddElement(new Xt::QeByObjectType(type));
}