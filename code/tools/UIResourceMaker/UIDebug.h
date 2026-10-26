#pragma once

#include <WinSock2.h>
#include "UIDefines.h"

class UIDebug
{
public:
	void Init();

	void SendMessage(UIDEBUG_MESSAGE mess,int value=0);
	
	void Shutdown(void);

private:
	WSADATA wsaData;
	SOCKET mSocket;
	sockaddr_in mClientService;
	bool	mConnected;
		
};

extern UIDebug gUIDebug;