#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/DopeSheet.h"
#include "../ScarabImpl.h"
#include "SnDSImpl.h"

SnDsKey::SnDsKey(ScarabDopeSheet* sn, LarvaField* lf, Xt::DomComponent* dom) :
	_sn(sn),
	_lf(lf),
	_dom(DomCallback, this)
{
	Monitor(&_dom, dom);

	LoadTime();
	LoadDuration();
}

void SnDsKey::OnTimeChanged()
{
	if (LtStruct* type = AsStruct(TypeOf(_sn->_db->schema, _lf, _dom)))
	{
		if (Xt::DomComponent* com = AsComponent(_dom))
		{
			if (Xt::DomField* field = AcquireField(&com->data, type->GetString("DopeSheet", "time", "time")))
			{
				Set(field, Format("%f", Time()));
			}
		}
	}
}

void SnDsKey::LoadTime()
{
	if (LtStruct* type = AsStruct(TypeOf(_sn->_db->schema, _lf, _dom)))
	{
		if (Xt::DomComponent* com = AsComponent(_dom))
		{
			float time = (float)atof(AsText(At(&com->data, type->GetString("DopeSheet", "time", "time"))));

			SetTime(time);
		}
	}
}

void SnDsKey::LoadDuration()
{
	if (LtStruct* type = AsStruct(TypeOf(_sn->_db->schema, _lf, _dom)))
	{
		if (Xt::DomComponent* com = AsComponent(_dom))
		{
			float duration = (float)atof(AsText(At(&com->data, type->GetString("DopeSheet", "duration", "duration"))));

			SetDuration(duration);
		}
	}
}

XtId SnDsKey::Id()
{
	if (Xt::DomComponent* com = AsComponent(_dom))
		return com->id;

	return 0;
}

void SnDsKey::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnDsKey* key)
{
	if (event->code == Xt::kDomRemoved)
	{
		delete key;
		return;
	}

	key->LoadTime();
	key->LoadDuration();
}