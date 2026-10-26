#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/DopeSheet.h"
#include "../ScarabImpl.h"
#include "SnDSImpl.h"

SnDsRow::SnDsRow(ScarabDopeSheet* sn, LarvaField* lf, Xt::DomElement* dom, XtId comId, XtId comIndex) :
	_link(this),
	_sn(sn),
	_lf(lf),
	_dom(DomCallback, this),
	_comId(comId),
	_comIndex(comIndex)
{
	XtUt::PushBack(&_sn->_rows, &_link);

	Monitor(&_dom, dom);

	if (LarvaType* type = TypeOf(_sn->_db->schema, lf, dom))
	{
		if (type->GetBool("DopeSheet", "blocks", false))
		{
			SetBlocks(true);
		}
	}

	LoadName();
}

void SnDsRow::LoadName()
{
	if (_dom)
	{
		if (!*_lf->label)
		{
			XtString summary = EvaluateSummaryText(_sn->_db, _sn->_db->schema, _lf, _dom);

			if (*summary)
				SetLabel(MbToWc(summary));
			else
				SetLabel(L"(unnamed)");
		}
		else
			SetLabel(MbToWc(_lf->label));
	}
}

SnDsKey* SnDsRow::FindKey(XtId id)
{
	if (!id)
		return 0;

	for(XtUt::LinkList<DsKey>::Iterator itKey = Keys(); itKey; ++itKey)
	{
		SnDsKey* key = (SnDsKey*)(DsKey*)itKey;

		if (key->Id() == id)
			return key;
	}

	return 0;
}

void SnDsRow::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnDsRow* row)
{
	if (event->code == Xt::kDomRemoved)
	{
		delete row;
		return;
	}

	row->LoadName();
}