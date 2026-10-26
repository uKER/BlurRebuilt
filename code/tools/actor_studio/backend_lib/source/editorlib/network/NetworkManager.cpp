#include "stdafx.h"

#include <EditorLib/Core.h>
#include <EditorLib/Network/NetworkManager.h>

#include <System/Artery/Artery.h>

namespace EditorLib
{

#define ARTERY_FOURCC(a, b, c, d) ((a << 24) | (b << 16) | (c << 8) | d)

XtSchema*	gXtSchema;

C_NetworkManager::C_NetworkManager() :
	kXtVarNetTimeOut(3000),
	_state(kStateDisconnected),
	_targetValid(false),
	_reconnect(false),
	_connectStartTime(0),
	_workOffline(false)
{
	_dirty = true;

	m_NetworkStateChangedCallback = 0;

	_resolveThread.Create(ResolveProc, this);
}

C_NetworkManager::~C_NetworkManager()
{
	
}

void C_NetworkManager::SetNetworkStateChangedCallback(void (*networkStateChangedCallback)(void))
{
	m_NetworkStateChangedCallback = networkStateChangedCallback;
}

void C_NetworkManager::Update()
{
	static int count = 0;
	static int count2 = 0;
	static int count3 = 0;

	if (++count >= 100)
	{
		NetSendIdle();
		count = 0;
	}

	if (_dirty)
	{
		void NetSendHackAll(bool onlyDirty);
		NetSendHackAll(true);

		_dirty = false;
	}
	
	if (++count2 >= 40 || _pump)
	{
		Pump();
		count2 = 0;
	}
}

void C_NetworkManager::SetDirty()
{
	_dirty = true;
}

bool C_NetworkManager::IsConnected()
{
	return _con.IsConnected();
}

// Post methods

void C_NetworkManager::NetResolved()
{
	EditorLib::C_Core::Instance().GetNetworkManager()._pump = true;
}

void C_NetworkManager::NetConnected()
{
	if (EditorLib::C_Core::Instance().GetNetworkManager().m_NetworkStateChangedCallback)
		EditorLib::C_Core::Instance().GetNetworkManager().m_NetworkStateChangedCallback();

	void NetSendHackAll(bool onlyDirty);
	NetSendHackAll(false);

	EditorLib::C_Core::Instance().GetNetworkManager()._pump = true;
}

void C_NetworkManager::NetMessageReceived()
{
	EditorLib::C_Core::Instance().GetNetworkManager()._pump = true;
}

void C_NetworkManager::NetDisconnected()
{
	if (EditorLib::C_Core::Instance().GetNetworkManager().m_NetworkStateChangedCallback)
		EditorLib::C_Core::Instance().GetNetworkManager().m_NetworkStateChangedCallback();

	EditorLib::C_Core::Instance().GetNetworkManager()._pump = true;
}


// End

void C_NetworkManager::SendNetworkSub(ArteryConnection con, Xt::DomDb* db, Xt::DomRecord** records, int count)
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

void C_NetworkManager::NetSendRecords(Xt::DomDb* db, Xt::DomRecord** records, int count)
{
	if (_con.IsConnected())
		SendNetworkSub(_con, db, records, count);
}

void C_NetworkManager::NetSendIdle()
{
	if (_con.IsConnected())
	{
		ArteryMessageBuilder b;

		if (b.Write(ArteryU32('IDLE')))
		{
			ArteryMessage msg;

			if (b.Build(msg))
				_con.Send(msg);
		}
	}
}

void C_NetworkManager::Destroy()
{
	_con.Close();

	while (_state != kStateDisconnected)
		Pump();

	_state = kStateExiting;

	_resolveThread.Close();
}

void C_NetworkManager::Reconnect()
{
	_reconnect = true;

	_con.Close();
}

bool C_NetworkManager::IsWorkingOffline()
{
	return _workOffline;
}

void C_NetworkManager::WorkOffline()
{
	_workOffline = true;

	_con.Close();
}

void C_NetworkManager::WorkOnline()
{
	_workOffline = false;
}

void C_NetworkManager::Pump()
{
	_lock.Wait();

	switch(_state)
	{
		case kStateDisconnected:
			_targetValid	= false;
			_reconnect		= false;

			if (!_workOffline)
			{
				_state = kStateResolving;
			}
		break;

		case kStateResolving:
		break;

		case kStateResolved:
			if (_reconnect || _workOffline)
				_state = kStateDisconnected;
			else if (_targetValid && ArteryConnect(_con, _targetAddr, "Sorcery", ConProc, this))
			{
				_state				= kStateConnecting;
				_connectStartTime	= GetTickCount();
			}
			else
				_state = kStateDisconnected;
		break;

		case kStateConnecting:
			if ((GetTickCount() - _connectStartTime) >= kXtVarNetTimeOut)
				Reconnect();
		break;

		case kStateConnected:
		break;

		case kStateDisconnecting:
			_con.Close();

			_state				= kStateWait;
			_connectStartTime	= GetTickCount();
		break;

		case kStateWait:
			if (_reconnect || _workOffline || ((GetTickCount() - _connectStartTime) >= kXtVarNetTimeOut))
				_state = kStateDisconnected;
		break;
	}

	_lock.Release();

	while(_con.IsConnected())
	{
		ArteryMessage msg;
		
		if (!_con.TryReceive(msg))
			break;

		DebugLn("Received schema: %i bytes", msg.DataLength());

		ArteryMessageReader r(msg);
		
		XtU32 endian;

		if (r.Read(endian))
			gXtTargetEndian = (endian == ARTERY_FOURCC('B', 'I', 'G', 'E')) ? kXtBigEndian : kXtLittleEndian;
		else
			gXtTargetEndian = kXtBigEndian;

		XtU32 length;

		if (!r.Read(length))
			length = 0;

		const XtU8* data = 0;

		if (r.Read(data, length))
		{
			if (XtSchema* schema = XtImportSchema(data, length))
			{
				DebugLn("Imported schema: types(%i) bases(%i) fields(%i)", schema->typeCount, schema->baseCount, schema->fieldCount);

				if (gXtSchema)
					XtDestroySchema(gXtSchema);

				gXtSchema = schema;
			}
		}
	}

	if (!_con.IsConnected() && gXtSchema)
	{
		XtDestroySchema(gXtSchema);
		gXtSchema = 0;
	}
}

XtString C_NetworkManager::TargetName()
{
	XtString result;

	_lock.Wait();

	if (_targetValid)
		result = _targetName;

	_lock.Release();

	return result;
}

ArteryConnection C_NetworkManager::Con()
{
	return _con;
}

void C_NetworkManager::ResolveProc(void* context)
{
	C_NetworkManager* dvn = (C_NetworkManager*)context;

	while (true)
	{
		dvn->_lock.Wait();

		if (dvn->_state == C_NetworkManager::kStateExiting)
		{
			dvn->_lock.Release();
			break;
		}

		if (dvn->_state != C_NetworkManager::kStateResolving)
		{
			dvn->_lock.Release();
			Sleep(1000);
			continue;
		}
		
		if (dvn->_state == C_NetworkManager::kStateResolving)
		{
			WsSockAddr addr = dvn->sockAddr;
			XtString name = dvn->targetName;	

			if (dvn->sockAddrValid)
			{
				dvn->_targetValid	= true;
				dvn->_targetAddr	= addr;
				dvn->_targetName	= name;
			}

			dvn->_state = C_NetworkManager::kStateResolved;

			NetResolved();
		}

		dvn->_lock.Release();
	}
}

void C_NetworkManager::ConProc(ArteryConnection con, void* context)
{
	C_NetworkManager* dvn = (C_NetworkManager*)context;

	if (con.IsConnected())
	{
		dvn->_lock.Wait();
		dvn->_state = C_NetworkManager::kStateConnected;
		dvn->_lock.Release();

		NetConnected();

		ArteryWaitContext ctx;

		while(con.IsConnected())
		{
			con.Wait(ctx);

			NetMessageReceived();
		}
	}

	dvn->_lock.Wait();
	dvn->_state = C_NetworkManager::kStateDisconnecting;
	dvn->_lock.Release();

	NetDisconnected();
}

}