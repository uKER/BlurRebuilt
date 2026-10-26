#include "stdafx.h"

#include <System/Artery/Artery.h>
#include <System/Artery/ArterySync.h>
#include <System/Debug/Debug.h>
#include <Xt/XtTool.h>
#include "Network.h"

#include "../Frame.h"

const int kNetworkTimeOut = 3000;

C_XtNetCon::C_XtNetCon() :
	m_pFrame(0),
	m_State(E_State_Disconnected),
	m_TargetValid(false),
	m_Reconnect(false),
	m_Connected(false),
	m_ConnectStartTime(0)
{
}

C_XtNetCon::~C_XtNetCon()
{
	assert(m_pFrame == 0);
}

bool C_XtNetCon::Create(C_Frame* app)
{
	if (m_pFrame)
		return false;

	m_pFrame			= app;
	

	Pump();

	return true;
}

void C_XtNetCon::Destroy()
{
	if (!m_pFrame)
		return;

	m_Connection.Close();

	m_ResolveThread.Close(true);

	m_pFrame = 0;
}

void C_XtNetCon::Reconnect()
{
	if (!m_pFrame)
		return;

	m_Reconnect = true;

	m_Connection.Close();
}

void C_XtNetCon::Pump()
{
	if (!m_pFrame)
		return;

	m_Lock.Wait();

	switch(m_State)
	{
		case E_State_Disconnected:
			m_TargetValid	= false;
			m_Reconnect		= false;

			if (m_ResolveThread.Create(ResolveProc, this))
				m_State = E_State_Resolving;
		break;

		case E_State_Resolving:
		break;

		case E_State_Resolved:
			m_ResolveThread.Close(true);

			if (m_Reconnect)
				m_State = E_State_Disconnected;
			else if (m_TargetValid && ArteryConnect(m_Connection, m_TargetAddr, "Sorcery", ConProc, this))
			{
				m_State				= E_State_Connecting;
				m_ConnectStartTime	= GetTickCount();
			}
			else
				m_State = E_State_Disconnected;
		break;

		case E_State_Connecting:
			if ((GetTickCount() - m_ConnectStartTime) >= kNetworkTimeOut)
				Reconnect();
		break;

		case E_State_Connected:
			if(!m_Connected)
			{
				wxLogMessage("Connected to DisplayWindow successfully");
				m_Connected = true;
			}
			break;

		case E_State_Disconnecting:
			m_Connection.Close();

			m_Connected			= false;
			m_State				= E_State_Wait;
			m_ConnectStartTime	= GetTickCount();
		break;

		case E_State_Wait:
			if (m_Reconnect || ((GetTickCount() - m_ConnectStartTime) >= kNetworkTimeOut))
				m_State = E_State_Disconnected;
		break;
	}

	m_Lock.Release();

	while(m_Connection.IsConnected())
	{
		ArteryMessage msg;
		
		if (!m_Connection.TryReceive(msg))
			break;

		//XtVar_ProcessMessage(m_pFrame, msg);
	}
}

XtString C_XtNetCon::TargetName()
{
	XtString result;

	m_Lock.Wait();

	if (m_TargetValid)
		result = m_TargetName;

	m_Lock.Release();

	return result;
}

ArteryConnection C_XtNetCon::Con()
{
	return m_Connection;
}

void C_XtNetCon::Post()
{
	wxCommandEvent event( wxEVT_MY_EVENT, 0 );

	if(m_pFrame)
		m_pFrame->GetEventHandler()->AddPendingEvent(event);
}

DWORD C_XtNetCon::ResolveProc(void* context)
{
	C_XtNetCon* dvn = (C_XtNetCon*)context;

	WsSockAddr addr(INADDR_LOOPBACK, 6317);
	XtString name;

	bool result = true;//MakeSockAddr(&gConnectToData, addr, name);

	dvn->m_Lock.Wait();

	if (result)
	{
		dvn->m_TargetValid	= true;
		dvn->m_TargetAddr	= addr;
		dvn->m_TargetName	= name;
	}

	dvn->m_State = C_XtNetCon::E_State_Resolved;

	dvn->m_Lock.Release();

	dvn->Post();
	//dvn->Post(kNetResolved, 0);

	return 0;
}

void C_XtNetCon::ConProc(ArteryConnection con, void* context)
{
	C_XtNetCon* dvn = (C_XtNetCon*)context;

	if (con.IsConnected())
	{
		dvn->m_Lock.Wait();
		dvn->m_State = C_XtNetCon::E_State_Connected;
		dvn->m_Lock.Release();

		dvn->Post();
		//dvn->Post(kNetConnected, 0);

		ArteryWaitContext ctx;

		while(con.IsConnected())
		{
			con.Wait(ctx);

			dvn->Post();
			//dvn->Post(kNetMessageReceived, 0);
		}
	}

	dvn->m_Lock.Wait();
	dvn->m_State = C_XtNetCon::E_State_Disconnecting;
	dvn->m_Lock.Release();

	dvn->Post();
	//dvn->Post(kNetDisconnected, 0);
}

void C_XtNetCon::NetSendRecords(Xt::DomDb* db, Xt::DomRecord** records, int count)
{
	if (m_Connection.IsConnected())
	{
		XtUt::Block data;
		XtTargetProfile p;

		p.endian = kXtLittleEndian;

		XtResult result;

		if (!XtPackBuild(&p, db, records, count, &data, &result))
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

		if (!m_Connection.Send(msg))
		{
			DebugLn("Failed to send message!");
		}

	}
}

void C_XtNetCon::NetSendIdle()
{
	if (m_Connection.IsConnected())
	{
		ArteryMessageBuilder b;

		if (b.Write(ArteryU32('IDLE')))
		{
			ArteryMessage msg;

			if (b.Build(msg))
				m_Connection.Send(msg);
		}
	}
}