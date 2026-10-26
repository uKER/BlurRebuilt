#ifndef ADVERTISING_H
#define ADVERTISING_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/MessageHandler.h>

namespace Display
{
	class C_View;
}

namespace GameResource
{
	class C_LevelResource;
	class C_LevelInstance;
}

namespace Advertising
{
	typedef void JumpToZoneCallback(const char* zoneName);

	/*
		Initialise the advertising system - called once at the start of the game

		360 - This must be done after logging into Live
	*/
	bool Initialise(const char* skuName, const char* skuVersion);

	//deserialise data block
	void Read(Sys::I_File& file);

	// Shut down the advertising system
	void Shutdown();

	// Called when we log into the game and have a profile
	void LoggedIn();

	//// We've signed out of our profile
	//void LoggedOut();

#if defined(ENABLE_IGA) && defined(PLATFORM_PS3)
	void SetCacheFolder(const Btl::String& cacheFolder);
#endif

	// Must be called once per frame to update the stats
	void Update(bool onLoadingScreen);

	// Must be called once per viewport so we can calculate impression information
	void Render(const Display::C_View* view);

	void EnterZone(bool quickRestart, const Btl::String& zoneName, S32 route, bool enable);
	void ExitZone(bool forceQuit);
	void RestartZone();
	void TidyZone();

	// These two functions are called when we need to stop reporting impressions (say the screen is too dark for it to be valid)
	void ResumeMakingMoney();
	void SuspendMakingMoney();

	void SetJumpToZoneCallback(JumpToZoneCallback* jtzCallback);

	void EnterZone_PostLoad(GameResource::C_LevelResource* pLevelResource, GameResource::C_LevelInstance *pLevelInstance);

	// Messaging system controller we use to listen for profile sign-in changes
	class MessageController : public Messaging::C_MessageHandler
	{
	public:

		MessageController( Messaging::C_MessageSubscriberSystem* pSubscriberSystem );
		virtual ~MessageController();

		bool MessageHandler(const Messaging::C_MessageBase *pMessage);
	};
}

#endif
