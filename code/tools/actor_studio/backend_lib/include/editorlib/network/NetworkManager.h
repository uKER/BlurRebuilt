#ifndef EDITOR_LIB_NETWORKMANAGER_H
#define EDITOR_LIB_NETWORKMANAGER_H

#include <System/Artery/Artery.h>
#include <System/Artery/ArterySync.h>

namespace EditorLib
{

class C_NetworkManager
{
public:
	C_NetworkManager();
	~C_NetworkManager();

	void Destroy();

	void SetNetworkStateChangedCallback(void (*networkStateChangedCallback)(void));

	void Update();

	void SetDirty();

	void SendNetworkSub(ArteryConnection con, Xt::DomDb* db, Xt::DomRecord** records, int count);
	void NetSendRecords(Xt::DomDb* db, Xt::DomRecord** records, int count);
	void NetSendIdle();

	bool IsConnected();

	void Reconnect();
	
	bool IsWorkingOffline();
	void WorkOnline();
	void WorkOffline();

	XtString TargetName();

private:
	static void NetResolved();
	static void NetConnected();
	static void NetMessageReceived();
	static void NetDisconnected();

	void Pump();
	static void ResolveProc(void* context);
	static void ConProc(ArteryConnection con, void* context);
	ArteryConnection Con();

public:
	WsSockAddr sockAddr;
	XtString targetName;
	bool sockAddrValid;

private:
	enum State
	{
		kStateExiting,
		kStateDisconnected,
		kStateResolving,
		kStateResolved,
		kStateConnecting,
		kStateConnected,
		kStateDisconnecting,
		kStateWait
	};

private:
	XtEndian gXtTargetEndian;
	const int kXtVarNetTimeOut;
	void (*m_NetworkStateChangedCallback)(void);

	ArteryConnection _con;

	ArteryLock _lock;

	State _state;

	ArteryThread	_resolveThread;
	bool			_targetValid;
	WsSockAddr		_targetAddr;
	XtString		_targetName;

	bool				_dirty;
	bool				_pump;
	bool				_reconnect;
	DWORD				_connectStartTime;
	bool				_workOffline;
};

}

#endif