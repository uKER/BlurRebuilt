#include "stdafx.h"

/*
	By default this does nothing. Game code can declare this function so we can
	send messages we want back through a debug channel.
*/
void Automation_SendMessageToDebugChannel(const char* message)
{
	// This does nothing unless overriden
	int mrf = 10;
}
