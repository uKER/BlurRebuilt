#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/ConnectTargetsDlg.h"
#include "XtWin/RegWrap.h"
#include "XtWin/Resource.h"
#include "XtWin/WndCore.h"
#include "XtWin/Scarab.h"

#include <ws2tcpip.h>
#include <xbdm.h>

bool ConnectTargetsDlg(ConnectTargets* ct)
{
	return false;
}

ConnectTargets* ImportConnectTargetsFromRegistry()
{
	return 0;
}

void ExportConnectTargetsToRegistry(ConnectTargets* ct)
{
}
