#ifndef CONNECTTO_H
#define CONNECTTO_H

#ifdef _MSC_VER
#pragma once
#endif

#include <System/Artery/Artery.h>

enum ConnectToMethod
{
	kCmLocal,
	kCmRemote,
	kCmCustom
};

enum ConnectToLocalMethod
{
	kClPc,
	kClXbox
};

struct ConnectToData
{
	ConnectToMethod			method;
	ConnectToLocalMethod	localMethod;
	XtString				remoteHost;
	XtString				customAddress;
	XtString				customPort;

public:
	inline ConnectToData() :
		method(kCmLocal),
		localMethod(kClXbox),
		customAddress("127.0.0.1"),
		customPort("6317")
	{
	}
};

class ConnectToDlg
{
public:
	ConnectToDlg(HWND parentWindow);
	~ConnectToDlg();

public:
	bool Modal();

public:
	inline ConnectToData& Data()
	{
		return _data;
	}

private:
	static INT_PTR CALLBACK WndProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void SyncFields(HWND dlg);

private:
	HWND _parentWindow;

private:
	ConnectToData _data;
};

bool MakeSockAddr(const ConnectToData* ctd, WsSockAddr& sockAddr, XtString& targetName);

void LoadConnectToData(ConnectToData* ctd);
void SaveConnectToData(const ConnectToData* ctd);

#endif // CONNECTTO_H