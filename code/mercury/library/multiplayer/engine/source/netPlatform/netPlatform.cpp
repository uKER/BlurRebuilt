
#include "stdafx.h"
#include "./netPlatform.h"
#include "./netPlatform_Core.h"
#include "netTime/netTime.h"

#include <Network/Core.h>
#include <Network/SessionDeletion.h>
#include <SystemProfile/SystemProfileManager.h>
#include <Network/CommunicatorManager.h> 

namespace netPlatform
{
	static bool s_voiceInitialised = false;
		
	void Platform::InitialiseVoice()
	{
		if(!s_voiceInitialised)
		{
			// initialise voice comm
			s_voiceInitialised = Communicator::C_Core::Instance().Initialise(4, 16);
		}
	}

	void Platform::Startup()
	{
		MEMORY_TRACKER_PUSH("network.nightwing");

	// startup networking and bypass security layer as default..
	#ifdef CFG_GOLD
		NetPlay::C_Core::Instance().Initialise(true, false);
	#else
		NetPlay::C_Core::Instance().Initialise(true, true);
	#endif

		// initialise session deletion
		NetPlay::C_SessionDeletion::Instance().Initialise(2);

		// initialise profile stuff
		SysProfile::C_Manager::Instance().Initialise();

		netClock::createInstance();
		Core::createInstance();
		Core::instance()->initialise();

		MEMORY_TRACKER_POP("network.nightwing");
	}

	void Platform::Destroy()
	{
		Core::instance()->shutdown();
		Core::destroyInstance();
		netClock::destroyInstance();

		// shutdown voice comm
		Communicator::C_Core::Instance().Shutdown();
		s_voiceInitialised = false;

		// shutdown profile stuff
		SysProfile::C_Manager::Instance().Shutdown();

		// shutdown session deletion
		NetPlay::C_SessionDeletion::Instance().Shutdown();

		// shutown network
		NetPlay::C_Core::Instance().Shutdown();
	}
}