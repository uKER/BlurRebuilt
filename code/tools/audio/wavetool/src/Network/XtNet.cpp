#include "Core/StdAfx.h"
#include <System/Artery/Artery.h>
#include <Xt/XtTool.h>
#include <xbdm.h>
#include "Application/SorceryApp.h"
#include "Application/SorceryView.h"

extern XtEndian gXtTargetEndian;

void SendNetworkSub(ArteryConnection con, Xt::DomDb* db, Xt::DomRecord** records, int count)
{
	XtUt::Block data;

	XtTargetProfile p;

	p.endian = gXtTargetEndian;

	XtPackBuildParams pbp;

	if (!XtPackBuild(&p, db, records, count, &data, &pbp))
	{
		DebugLn("Failed to build pack!");
		return;
	}

	ArteryMessageBuilder b;

	if (!b.Write(data.start, (int)Capacity(&data)))
	{
		DebugLn("Failed to write pack!");
		return;
	}

	ArteryMessage msg;

	if (!b.Build(msg))
	{
		DebugLn("Failed to build message!");
		return;
	}

	if (!con.Send(msg))
	{
		DebugLn("Failed to send message!");
	}
}

void NetSendRecords(SorceryApp* app, Xt::DomDb* db, Xt::DomRecord** records, int count)
{
	ArteryConnection con = app->_xtNet.Con();

	if (con.IsConnected())
		SendNetworkSub(con, db, records, count);
}

void NetSendIdle(SorceryApp* app)
{
	ArteryConnection con = app->_xtNet.Con();

	if (con.IsConnected())
	{
		ArteryMessageBuilder b;

		if (b.Write(ArteryU32('IDLE')))
		{
			ArteryMessage msg;

			if (b.Build(msg))
				con.Send(msg);
		}
	}
}