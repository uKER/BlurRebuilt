#include "Core/XtWinPch.h"
#include "FinderImpl.h"

FrRecordsNamed::FrRecordsNamed(FinderDlg* finder) :
	FinderRow(finder)
{
	SetLabel("Which are na&med");

	if ((_editCtrl = AddControl(new FcEdit(this))) != 0)
	{
	}
}

FrRecordsNamed::~FrRecordsNamed()
{
}

void FrRecordsNamed::BuildQuery(Xt::Query* query)
{
	if (!_editCtrl)
		return;

	XtString text = _editCtrl->GetText();

	if (*text)
		query->AddElement(new Xt::QeByRecordName(text));
}