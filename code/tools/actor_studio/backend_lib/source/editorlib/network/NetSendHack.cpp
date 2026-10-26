#include "stdafx.h"
//#include "SorceryView.h"

#include <EditorLib/Core.h>
#include <EditorLib/Network/NetworkManager.h>
#include <EditorLib/Xt/XtManager.h>

struct NetRec
{
	Xt::DomMonitor dom;
};

XtUt::List<NetRec> gNetRecs;

bool gNetPaused = false;

void NetSendHackPause()
{
	gNetPaused = true;
}

void NetSendHackResume()
{
	gNetPaused = false;
}

void NetSendHack()
{
	if (gNetPaused)
		return;

	XtUt::Array<Xt::DomRecord*> records;

	for(XtUt::List<NetRec>::Iterator it(&gNetRecs); it; ++it)
	{
		Xt::DomRecord* rec = AsRecord(it->dom);

		if (!rec)
			continue;

		if (Xt::DomPack* pack = PackOf(rec))
		{
			if (atoi(AsText(At(&pack->properties, "noDefaultExport"))) != 0)
				continue;

			if (atoi(AsText(At(&pack->properties, "noDynamicUpdate"))) != 0)
				continue;
		}

		bool found = false;

		for(XtUt::Array<Xt::DomRecord*>::Iterator it2(&records); it2; ++it2)
		{
			if (rec == it2)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			AppendEx(&records, rec);
		}
	}

	if (records.count > 0)
		EditorLib::C_Core::Instance().GetNetworkManager().NetSendRecords(EditorLib::C_Core::Instance().GetXtManager().GetDb(), records.elements, (int)records.count);

	Free(&gNetRecs);
}

void NetSendHackAll(bool onlyDirty)
{
	if (gNetPaused)
		return;

	XtUt::Array<Xt::DomRecord*> records;
	
	int count = 0;

	for(Xt::DomDb::Iterator itPack(EditorLib::C_Core::Instance().GetXtManager().GetDb()); itPack; ++itPack)
	{
		for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
			count++;
	}

	XtUt::Reserve(&records, count);

	for(Xt::DomDb::Iterator itPack(EditorLib::C_Core::Instance().GetXtManager().GetDb()); itPack; ++itPack)
	{
		if (atoi(AsText(At(&itPack->properties, "noDefaultExport"))) != 0)
			continue;

		if (atoi(AsText(At(&itPack->properties, "noDynamicUpdate"))) != 0)
			continue;

		for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
		{
			if (!onlyDirty || itRecord->dirty)
				AppendEx(&records, (Xt::DomRecord*)itRecord);
		}
	}

	if (records.count > 0)
		EditorLib::C_Core::Instance().GetNetworkManager().NetSendRecords(EditorLib::C_Core::Instance().GetXtManager().GetDb(), records.elements, (int)records.count);
}

void NetUpdateHack(Xt::DomRecord* record)
{
	if (!EditorLib::C_Core::Instance().GetNetworkManager().IsConnected())
		return;

	NetRec* nr = new NetRec;

	if (!nr)
		return;

	Monitor(&nr->dom, record);

	AppendEx(&gNetRecs, nr);
}
