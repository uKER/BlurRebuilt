#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/Timeline.h"
#include "../ScarabImpl.h"
#include "SnTlImpl.h"

SnTlRow::SnTlRow(ScarabTimeline* sn, LarvaField* lf, Xt::DomElement* dom, XtId comId, XtId comIndex) :
	_link(this),
	_sn(sn),
	_lf(lf),
	_dom(DomCallback, this),
	_comId(comId),
	_comIndex(comIndex)
{
	XtUt::PushBack(&_sn->_rows, &_link);

	Monitor(&_dom, dom);

	LoadName();
}

void SnTlRow::LoadName()
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

SnTlKey* SnTlRow::FindKey(XtId id)
{
	if (!id)
		return 0;

	for(XtUt::LinkList<TlKey>::Iterator itKey = Keys(); itKey; ++itKey)
	{
		SnTlKey* key = (SnTlKey*)(TlKey*)itKey;

		if (key->Id() == id)
			return key;
	}

	return 0;
}

void SnTlRow::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnTlRow* row)
{
	if (event->code == Xt::kDomRemoved)
	{
		delete row;
		return;
	}

	row->LoadName();
}