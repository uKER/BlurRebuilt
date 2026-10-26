#ifndef NETWORK_NETWORK_H
#define NETWORK_NETWORK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <System/Artery/Artery.h>
#include <System/Artery/ArterySync.h>
#include "xt\xttool.h"

class C_Frame;

//
// XtVarNet
//

class C_XtNetCon
{
public:
	C_XtNetCon();
	~C_XtNetCon();

public:
	bool Create(C_Frame* pFrame);
	void Destroy();

public:
	void Reconnect();

public:
	void Pump();
	void Post();

public:
	void				NetSendRecords(Xt::DomDb* db, Xt::DomRecord** records, int count);
	void				NetSendIdle();
	XtString			TargetName();
	ArteryConnection	Con();

private:
	static DWORD CALLBACK ResolveProc(void* context);

private:
	static void ConProc(ArteryConnection con, void* context);

private:
	enum E_State
	{
		E_State_Disconnected,
		E_State_Resolving,
		E_State_Resolved,
		E_State_Connecting,
		E_State_Connected,
		E_State_Disconnecting,
		E_State_Wait
	};

private:
	C_Frame*		m_pFrame;

private:
	ArteryLock		m_Lock;

private:
	E_State			m_State;

private:
	ArteryThread	m_ResolveThread;
	bool			m_TargetValid;
	WsSockAddr		m_TargetAddr;
	XtString		m_TargetName;

private:
	ArteryConnection	m_Connection;
	bool				m_Reconnect;
	bool				m_Connected;
	DWORD				m_ConnectStartTime;
};

#endif // NETWORK_NETWORK_H