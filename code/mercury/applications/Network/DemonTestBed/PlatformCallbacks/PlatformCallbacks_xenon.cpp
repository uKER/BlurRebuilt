#include <stdafx.h>
#include <Core/core.h>
#include <network/Core.h>
#include "Platformcallbacks.h"


namespace PlatformCallbacks
{	
	Debug::Logger &g_CallbacksLogger = Debug::GetLogger("racer.platform.callbacks");

	void Register(void)
	{
	}

	void UnRegister(void)
	{
	}

	void Poll(void)
	{
	}
} // end namespace PlatformCallbacks
