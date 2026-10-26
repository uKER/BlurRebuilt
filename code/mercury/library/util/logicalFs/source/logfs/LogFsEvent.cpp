#include "LogicalFsPch.h"

#include <LogFs/LogFsManager.h>
#include <LogFs/LogFsEvent.h>

namespace LogicalFs
{
	C_LogFsEvent::C_LogFsEvent(const char* name)
	{
		C_LogFsManager::Instance().PushEvent(name);
	}

	C_LogFsEvent::~C_LogFsEvent()
	{
		C_LogFsManager::Instance().PopEvent();
	}
}
