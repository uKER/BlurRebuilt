#include "Core/StdAfx.h"
#include <System/Artery/Artery.h>
#include <System/Artery/ArterySync.h>
#include <System/Debug/Debug.h>
#include <Xt/XtTool.h>
#include "Application/SorceryApp.h"
#include "Network/Network.h"

#define ARTERY_FOURCC(a, b, c, d) ((a << 24) | (b << 16) | (c << 8) | d)

XtSchema*	gXtSchema;
XtEndian	gXtTargetEndian;

const int kXtVarNetTimeOut = 3000;

XtVarNet::XtVarNet() :
	_app(0),
	_notifyWindow(0),
	_notifyMessage(0),
	_state(kStateDisconnected),
	_targetValid(false),
	_reconnect(false),
	_connectStartTime(0),
	_workOffline(false)
{
}

XtVarNet::~XtVarNet()
{
	assert(_app == 0);
}

bool XtVarNet::Create(SorceryApp* app, HWND notifyWindow, int notifyMessage)
{
	if (_app)
		return false;

	_app			= app;
	_notifyWindow	= notifyWindow;
	_notifyMessage	= notifyMessage;

	Pump();

	return true;
}

void XtVarNet::Destroy()
{
	if (!_app)
		return;

	_con.Close();

	_resolveThread.Close();

	_app = 0;
}

void XtVarNet::Reconnect()
{
	if (!_app)
		return;

	_reconnect = true;

	_con.Close();
}

void XtVarNet::WorkOffline()
{
	if (!_app)
		return;

	_workOffline = true;

	_con.Close();
}

void XtVarNet::WorkOnline()
{
	if (!_app)
		return;

	_workOffline = false;
}

void XtVarNet::Pump()
{
	if (!_app)
		return;

	_lock.Wait();

	switch(_state)
	{
		case kStateDisconnected:
			_targetValid	= false;
			_reconnect		= false;

			if (!_workOffline)
			{
				if (_resolveThread.Create(ResolveProc, this))
					_state = kStateResolving;
			}
		break;

		case kStateResolving:
		break;

		case kStateResolved:
			_resolveThread.Close();

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

XtString XtVarNet::TargetName()
{
	XtString result;

	_lock.Wait();

	if (_targetValid)
		result = _targetName;

	_lock.Release();

	return result;
}

ArteryConnection XtVarNet::Con()
{
	return _con;
}

void XtVarNet::Post(ArteryU32 msg, ArteryU32 param)
{
	PostMessage(_notifyWindow, _notifyMessage, msg, param);
}

void XtVarNet::ResolveProc(void* context)
{
	XtVarNet* dvn = (XtVarNet*)context;

	WsSockAddr addr;
	XtString name;

	bool result = MakeSockAddr(&gConnectToData, addr, name);

	dvn->_lock.Wait();

	if (result)
	{
		dvn->_targetValid	= true;
		dvn->_targetAddr	= addr;
		dvn->_targetName	= name;
	}

	dvn->_state = XtVarNet::kStateResolved;

	dvn->_lock.Release();

	dvn->Post(kNetResolved, 0);
}

void XtVarNet::ConProc(ArteryConnection con, void* context)
{
	XtVarNet* dvn = (XtVarNet*)context;

	if (con.IsConnected())
	{
		dvn->_lock.Wait();
		dvn->_state = XtVarNet::kStateConnected;
		dvn->_lock.Release();

		dvn->Post(kNetConnected, 0);

		ArteryWaitContext ctx;

		while(con.IsConnected())
		{
			con.Wait(ctx);

			dvn->Post(kNetMessageReceived, 0);
		}
	}

	dvn->_lock.Wait();
	dvn->_state = XtVarNet::kStateDisconnecting;
	dvn->_lock.Release();

	dvn->Post(kNetDisconnected, 0);
}