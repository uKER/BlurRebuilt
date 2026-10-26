#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/ConnectTo.h"
#include "XtWin/RegWrap.h"
#include "XtWin/Resource.h"
#include "XtWin/WndCore.h"
#include "XtWin/Scarab.h"

#include <ws2tcpip.h>
#include <xbdm.h>

//
// FindMachineByName
//

struct FindMachineData
{
	const char*	name;
	WsSockAddr*	address;
	bool		found;
};

bool FindMachineByNameSub(const ArteryPeerInfo& info, void* context)
{
	FindMachineData* fmd = (FindMachineData*)context;

	if (_stricmp(info.machine, fmd->name) == 0)
	{
		*fmd->address	= info.address;
		fmd->found		= true;

		return false;
	}

	return true;
}

bool FindMachineByName(const char* name, WsSockAddr& address)
{
	FindMachineData fmd;

	fmd.name	= name;
	fmd.address	= &address;
	fmd.found	= false;

	if (!ArteryEnumeratePeers(FindMachineByNameSub, &fmd))
		return false;

	return fmd.found;
}

//
// CollectMachineData
//

struct MachineData
{
	XtString	name;
	XtString	game;
	XtString	machine;
	XtString	platform;
	WsSockAddr	address;
};

XtUt::List<MachineData> gMachineData;

bool CollectMachineDataSub(const ArteryPeerInfo& info, void* context)
{
	for(XtUt::List<MachineData>::Iterator it(&gMachineData); it; ++it)
	{
		if (it->address == info.address)
			return true;
	}

	MachineData* md = XtUt::AppendEx(&gMachineData, new MachineData);

	if (!md)
		return false;

	md->game		= info.game;
	md->machine		= info.machine;
	md->platform	= info.platform;
	md->address		= info.address;

	if (StartsWithI(md->game, "luntbuild-"))
		md->game = Substring(md->game, 10);

	md->name = md->platform + ": " + md->machine + " (" + md->game + ")";

	return true;
}

bool CollectMachineDataSort(const MachineData* lhs, const MachineData* rhs)
{
	return _stricmp(lhs->name, rhs->name) < 0;
}

bool CollectMachineData()
{
	Free(&gMachineData);

	if (!ArteryEnumeratePeers(CollectMachineDataSub, 0))
		return false;

	XtUt::MergeSort(&gMachineData, CollectMachineDataSort);

	return true;
}

//
// ConnectToDlg
//

ConnectToDlg::ConnectToDlg(HWND parentWindow) :
	_parentWindow(parentWindow)
{
}

ConnectToDlg::~ConnectToDlg()
{
}

bool ConnectToDlg::Modal()
{
	if (DialogBoxParam(GetXtWinIconDll(), MAKEINTRESOURCE(IDD_XTWIN_CONNECTTO), _parentWindow, ConnectToDlg::WndProc, (LPARAM)(INT_PTR)this) != 1)
		return false;

	Free(&gMachineData);

	return true;
}

INT_PTR CALLBACK ConnectToDlg::WndProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ConnectToDlg* ctd = (ConnectToDlg*)(INT_PTR)GetWindowLong(dlg, DWL_USER);

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			SetWindowLong(dlg, DWL_USER, (LONG)(INT_PTR)lParam);
			ctd = (ConnectToDlg*)lParam;

			CollectMachineData();

			// IDC_LOCAL, IDC_REMOTE, IDC_CUSTOM

			int toCheck = IDC_XTWIN_LOCAL;

			if (ctd->_data.method == kCmRemote)
				toCheck = IDC_XTWIN_REMOTE;
			else if (ctd->_data.method == kCmCustom)
				toCheck = IDC_XTWIN_CUSTOM;

			CheckRadioButton(dlg, IDC_XTWIN_LOCAL, IDC_XTWIN_CUSTOM, toCheck);

			// Method

			HWND ctrlMethod = GetDlgItem(dlg, IDC_XTWIN_METHOD);

			ComboBox_AddString(ctrlMethod, "Pc");
			ComboBox_AddString(ctrlMethod, "Xbox");

			ComboBox_SetCurSel(ctrlMethod, (int)ctd->_data.localMethod);

			// Host

			HWND ctrlHost = GetDlgItem(dlg, IDC_XTWIN_HOST);

			int found = -1;

			for(int i = 0; i < (int)gMachineData.count; i++)
			{
				MachineData* md = gMachineData[i];

				if (_stricmp(md->machine, ctd->_data.remoteHost) == 0)
					found = i;

				ComboBox_AddString(ctrlHost, md->name);
			}

			ComboBox_SetCurSel(ctrlHost, found);

			// Address, Port

			SetDlgItemText(dlg, IDC_XTWIN_ADDRESS, ctd->_data.customAddress);
			SetDlgItemText(dlg, IDC_XTWIN_PORT, ctd->_data.customPort);

			ctd->SyncFields(dlg);
		}
		return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						EndDialog(dlg, 1);
						return TRUE;
					}
				break;

				case IDCANCEL:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						EndDialog(dlg, 0);
						return TRUE;
					}
				break;

				case IDC_XTWIN_LOCAL:
				case IDC_XTWIN_REMOTE:
				case IDC_XTWIN_CUSTOM:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						if (IsDlgButtonChecked(dlg, IDC_XTWIN_LOCAL))
							ctd->_data.method = kCmLocal;
						else if (IsDlgButtonChecked(dlg, IDC_XTWIN_REMOTE))
							ctd->_data.method = kCmRemote;
						else if (IsDlgButtonChecked(dlg, IDC_XTWIN_CUSTOM))
							ctd->_data.method = kCmCustom;

						ctd->SyncFields(dlg);

						return TRUE;
					}
				break;

				case IDC_XTWIN_METHOD:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						HWND ctrlMethod = GetDlgItem(dlg, IDC_XTWIN_METHOD);
						ctd->_data.localMethod = (ConnectToLocalMethod)ComboBox_GetCurSel(ctrlMethod);
						ctd->SyncFields(dlg);
						return TRUE;
					}
				break;

				case IDC_XTWIN_HOST:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						HWND	ctrlHost	= GetDlgItem(dlg, IDC_XTWIN_HOST);
						int		remoteIndex	= ComboBox_GetCurSel(ctrlHost);

						if ((remoteIndex >= 0) && (remoteIndex < (int)gMachineData.count))
							ctd->_data.remoteHost = gMachineData[remoteIndex]->machine;

						ctd->SyncFields(dlg);

						return TRUE;
					}
				break;

				case IDC_XTWIN_ADDRESS:
					if (HIWORD(wParam) == EN_CHANGE)
					{
						HWND ctrlAddress = GetDlgItem(dlg, IDC_XTWIN_ADDRESS);
						ctd->_data.customAddress = SzGetWindowText(ctrlAddress);
						ctd->SyncFields(dlg);
						return TRUE;
					}
				break;

				case IDC_XTWIN_PORT:
					if (HIWORD(wParam) == EN_CHANGE)
					{
						HWND ctrlPort = GetDlgItem(dlg, IDC_XTWIN_PORT);
						ctd->_data.customPort = SzGetWindowText(ctrlPort);
						ctd->SyncFields(dlg);
						return TRUE;
					}
				break;
			}
		break;
	}

	return FALSE;
}

void ConnectToDlg::SyncFields(HWND dlg)
{
	HWND ctrlLocal			= GetDlgItem(dlg, IDC_XTWIN_LOCAL);
	HWND ctrlRemote			= GetDlgItem(dlg, IDC_XTWIN_REMOTE);
	HWND ctrlCustom			= GetDlgItem(dlg, IDC_XTWIN_CUSTOM);
	HWND ctrlMethod			= GetDlgItem(dlg, IDC_XTWIN_METHOD);
	HWND ctrlMethodText		= GetDlgItem(dlg, IDC_XTWIN_METHOD_TEXT);
	HWND ctrlHost			= GetDlgItem(dlg, IDC_XTWIN_HOST);
	HWND ctrlHostText		= GetDlgItem(dlg, IDC_XTWIN_HOST_TEXT);
	HWND ctrlAddress		= GetDlgItem(dlg, IDC_XTWIN_ADDRESS);
	HWND ctrlAddressText	= GetDlgItem(dlg, IDC_XTWIN_ADDRESS_TEXT);
	HWND ctrlPort			= GetDlgItem(dlg, IDC_XTWIN_PORT);
	HWND ctrlPortText		= GetDlgItem(dlg, IDC_XTWIN_PORT_TEXT);

	bool enableLocal	= _data.method == kCmLocal;
	bool enableRemote	= _data.method == kCmRemote;
	bool enableCustom	= _data.method == kCmCustom;

	EnableWindow(ctrlMethod,		enableLocal);
	EnableWindow(ctrlMethodText,	enableLocal);
	EnableWindow(ctrlHost,			enableRemote);
	EnableWindow(ctrlHostText,		enableRemote);
	EnableWindow(ctrlAddress,		enableCustom);
	EnableWindow(ctrlAddressText,	enableCustom);
	EnableWindow(ctrlPort,			enableCustom);
	EnableWindow(ctrlPortText,		enableCustom);
}

//
// MakeSockAddr
//

bool MakeSockAddr(const ConnectToData* ctd, WsSockAddr& sockAddr, XtString& targetName)
{
	assert(ctd != 0);

	switch(ctd->method)
	{
		case kCmLocal:
		{
			if (ctd->localMethod == kClPc)
			{
				sockAddr	= WsSockAddr(INADDR_LOOPBACK, 6317);
				targetName	= "localhost (local)";
			}
			else if (ctd->localMethod == kClXbox)
			{
				DWORD dwAddr;

				if (FAILED(DmGetAltAddress(&dwAddr)))
					return false;

				sockAddr = WsSockAddr(dwAddr, 6317);

				char	name[256];
				DWORD	length = sizeof(name) / sizeof(*name);

				if (SUCCEEDED(DmGetXboxName(name, &length)))
					targetName = XtString(name) + " (local)";
				else
					targetName = "*unknown-xbox* (local)";
			}
			else
			{
				// PS3 not yet supported

				return false;
			}
		}
		return true;

		case kCmRemote:
		{
			if (!FindMachineByName(ctd->remoteHost, sockAddr))
				return false;

			targetName = ctd->remoteHost + " (remote)";
		}
		return true;

		case kCmCustom:
		{
			addrinfo	hints		= { 0 };
			addrinfo*	response	= 0;

			hints.ai_flags		= AI_PASSIVE;
			hints.ai_family		= PF_INET;
			hints.ai_socktype	= SOCK_STREAM;
			hints.ai_protocol	= IPPROTO_TCP;

			if (getaddrinfo(ctd->customAddress, ctd->customPort, &hints, &response) != 0)
				return false;

			if (!response)
				return false;

			if (!response->ai_addr)
			{
				freeaddrinfo(response);
				return false;
			}

			sockAddr = WsSockAddr(((SOCKADDR_IN*)response->ai_addr)->sin_addr, ntohs(((SOCKADDR_IN*)response->ai_addr)->sin_port));

			targetName = Format("%i.%i.%i.%i:%i (custom)",
					sockAddr.Addr().S_un.S_un_b.s_b1, sockAddr.Addr().S_un.S_un_b.s_b2, sockAddr.Addr().S_un.S_un_b.s_b3, sockAddr.Addr().S_un.S_un_b.s_b4,
					sockAddr.Port()
				);
		}
		return true;
	}

	return false;
}

//
// LoadConnectToData
//

void LoadConnectToData(ConnectToData* ctd)
{
	RegWrap keySoftware(HKEY_CURRENT_USER, "Software");
	RegWrap keyBizarreCreations(keySoftware, "Bizarre Creations");
	RegWrap keyDebugView(keyBizarreCreations, "DebugView");
	RegWrap keyTarget(keyDebugView, "Target");

	XtString method			= keyTarget.Get("Method");
	XtString localMethod	= keyTarget.Get("LocalMethod");

	if (_stricmp(method, "local") == 0)
		ctd->method = kCmLocal;
	else if (_stricmp(method, "remote") == 0)
		ctd->method = kCmRemote;
	else if (_stricmp(method, "custom") == 0)
		ctd->method = kCmCustom;
	else
		ctd->method = kCmLocal;

	if (_stricmp(localMethod, "pc") == 0)
		ctd->localMethod = kClPc;
	else if (_stricmp(localMethod, "xbox") == 0)
		ctd->localMethod = kClXbox;
	else
		ctd->localMethod = kClXbox;

	ctd->remoteHost		= keyTarget.Get("RemoteHost");
	ctd->customAddress	= keyTarget.Get("CustomAddress");
	ctd->customPort		= keyTarget.Get("CustomPort");

	if (!*ctd->customAddress)
		ctd->customAddress = "127.0.0.1";

	if (!*ctd->customPort)
		ctd->customPort = "6317";
}

//
// LoadConnectToData
//

void SaveConnectToData(const ConnectToData* ctd)
{
	RegWrap keySoftware(HKEY_CURRENT_USER, "Software");
	RegWrap keyBizarreCreations(keySoftware, "Bizarre Creations");
	RegWrap keyDebugView(keyBizarreCreations, "DebugView");
	RegWrap keyTarget(keyDebugView, "Target");

	const char* method = "";

	switch(ctd->method)
	{
		case kCmLocal:	method = "local";	break;
		case kCmRemote:	method = "remote";	break;
		case kCmCustom:	method = "custom";	break;
	}

	const char* localMethod = "";

	switch(ctd->localMethod)
	{
		case kClPc:		localMethod = "pc";		break;
		case kClXbox:	localMethod = "xbox";	break;
	}

	keyTarget.Set("Method", method);
	keyTarget.Set("LocalMethod", localMethod);
	keyTarget.Set("RemoteHost", ctd->remoteHost);
	keyTarget.Set("CustomAddress", ctd->customAddress);
	keyTarget.Set("CustomPort", ctd->customPort);
}