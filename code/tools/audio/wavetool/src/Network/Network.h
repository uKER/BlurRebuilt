#ifndef NETWORK_NETWORK_H
#define NETWORK_NETWORK_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Core/List.h"
#include <System/Artery/Artery.h>
#include <System/Artery/ArterySync.h>

#include "XtWin/ConnectTo.h"

extern ConnectToData gConnectToData;

//
// XtVarNet
//

class XtVarNet
{
public:
	XtVarNet();
	~XtVarNet();

public:
	bool Create(SorceryApp* app, HWND notifyWindow, int notifyMessage);
	void Destroy();

public:
	void Reconnect();
	void WorkOffline();
	void WorkOnline();

public:
	void Pump();

public:
	XtString TargetName();
	ArteryConnection Con();

public:
	bool IsWorkingOffline()
	{
		return _workOffline;
	}

private:
	void Post(ArteryU32 msg, ArteryU32 param);

private:
	static void ResolveProc(void* context);

private:
	static void ConProc(ArteryConnection con, void* context);

private:
	enum State
	{
		kStateDisconnected,
		kStateResolving,
		kStateResolved,
		kStateConnecting,
		kStateConnected,
		kStateDisconnecting,
		kStateWait
	};

private:
	SorceryApp*	_app;
	HWND		_notifyWindow;
	int			_notifyMessage;

private:
	ArteryLock _lock;

private:
	State _state;

private:
	ArteryThread	_resolveThread;
	bool			_targetValid;
	WsSockAddr		_targetAddr;
	XtString		_targetName;

private:
	ArteryConnection	_con;
	bool				_reconnect;
	DWORD				_connectStartTime;
	bool				_workOffline;
};

//
// NetSend*
//

void NetSendRecords(SorceryApp* app, Xt::DomDb* db, Xt::DomRecord** records, int count);
void NetSendIdle(SorceryApp* app);

#endif // NETWORK_NETWORK_H