#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/Timeline.h"
#include "../ScarabImpl.h"
#include "SnTlImpl.h"

SnTlKey::SnTlKey(ScarabTimeline* sn, LarvaField* lf, Xt::DomComponent* dom) :
	_sn(sn),
	_lf(lf),
	_dom(DomCallback, this)
{
	Monitor(&_dom, dom);

	LoadTime();
}

void SnTlKey::OnTimeChanged()
{
	if (LtStruct* type = AsStruct(TypeOf(_sn->_db->schema, _lf, _dom)))
	{
		if (Xt::DomComponent* com = AsComponent(_dom))
		{
			if (Xt::DomField* field = AcquireField(&com->data, type->GetString("Timeline", "time", "time")))
			{
				Set(field, Format("%f", Time()));
			}
		}
	}
}

void SnTlKey::LoadTime()
{
	if (LtStruct* type = AsStruct(TypeOf(_sn->_db->schema, _lf, _dom)))
	{
		if (Xt::DomComponent* com = AsComponent(_dom))
		{
			float time = (float)atof(AsText(At(&com->data, type->GetString("Timeline", "time", "time"))));

			SetTime(time);
		}
	}
}

XtId SnTlKey::Id()
{
	if (Xt::DomComponent* com = AsComponent(_dom))
		return com->id;

	return 0;
}

void SnTlKey::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnTlKey* key)
{
	if (event->code == Xt::kDomRemoved)
	{
		delete key;
		return;
	}

	key->LoadTime();
}