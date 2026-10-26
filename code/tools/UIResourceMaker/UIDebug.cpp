#include "stdafx.h"
#include "commandline.h"
#include "UIDebug.h"

UIDebug gUIDebug;

void UIDebug::Init()
{
//#ifndef _DEBUG
	int error = WSAStartup( MAKEWORD(2,2), &wsaData );
	
	mConnected = false;

	if(error != NO_ERROR) 
	{
		gCommandLine.AddLine(TC_RED,"Error starting up Winsock!");
	}

	mSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( mSocket == INVALID_SOCKET ) 
	{
	    gCommandLine.AddLine(TC_RED,"Error starting up Socket!");
		WSACleanup();
	}

	//u_long temp=1;
	//ioctlsocket(mSocket,FIONBIO,&temp);

	mClientService.sin_family = AF_INET;

	char str[32];
	sprintf(str,"%d.%d.%d.%d",(gUI.Config.XboxIPAddress&0xff000000)>>24,(gUI.Config.XboxIPAddress&0x00ff0000)>>16,(gUI.Config.XboxIPAddress&0x0000ff00)>>8,(gUI.Config.XboxIPAddress&0x000000ff));

	mClientService.sin_addr.s_addr = inet_addr(str);
	mClientService.sin_port = htons( 27015 );

	if(connect(mSocket,(SOCKADDR*) &mClientService,sizeof(mClientService)) == SOCKET_ERROR) 
	{
		int error=WSAGetLastError();
	    gCommandLine.AddLine(TC_RED,"Error connecting to Socket!");
		WSACleanup();
	}

	mConnected = true;
//#endif
}

void UIDebug::SendMessage(UIDEBUG_MESSAGE mess,int value)
{
//#ifndef _DEBUG
	int bytesSent;
	int bytesRecv = SOCKET_ERROR;
	char sendbuf[32] = "";

	if(mess == UIDEBUG_MESSAGE_REINIT || mess == UIDEBUG_MESSAGE_RELOAD_LUA || mess == UIDEBUG_MESSAGE_RELOAD_TEXT)
	{
		sendbuf[0]=mess;
	}
	else if(mess == UIDEBUG_MESSAGE_SCREEN_CHANGE)
	{
		sendbuf[0]=mess;
		sendbuf[1]=value;
	}

	bytesSent = send( mSocket, sendbuf, strlen(sendbuf), 0 );
	gCommandLine.AddLineArg(TC_BLUE,"Bytes Sent: %ld", bytesSent );
//#endif
}


void UIDebug::Shutdown(void)
{
//#ifndef _DEBUG
	if (mConnected)
	{
		int res = closesocket(mSocket);
		mSocket = INVALID_SOCKET;
		WSACleanup();
	}

	
//#endif
}


