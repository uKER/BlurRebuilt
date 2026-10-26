#include "AdvertisingPch.h"

#include "Advertising\Advertising.h"
#include "Advertising\AdvertManager.h"
#include "Advertising\AdvertObject.h"
#include "Advertising\AdvertResource.h"

#include "ui/Game/Profiles/ProfileInterfaceMessages.h"

#include "display\view\view.h"

namespace Advertising
{



bool Initialise(const char* skuName, const char* skuVersion)
{
	return AdvertManager::Instance().Initialise(skuName, skuVersion);
}

void Read(Sys::I_File& file)
{
	AdvertManager::Instance().LoadAdvertResource(file);
}

void Shutdown()
{
	AdvertManager::Instance().Shutdown();
}

void LoggedIn()
{
	AdvertManager::Instance().LoggedIn();
}

#if defined(ENABLE_IGA) && defined(PLATFORM_PS3)

void SetCacheFolder(const Btl::String& cacheFolder)
{
	AdvertManager::Instance().SetCacheFolder(cacheFolder);
}

#endif

void Update(bool onLoadingScreen)
{
	AdvertManager::Instance().Update(onLoadingScreen);
}

void Render(const Display::C_View* view)
{
	AdvertManager::Instance().Render(view);
}

void EnterZone(bool quickRestart, const Btl::String& zoneName, S32 route, bool enabled)
{
	AdvertManager::Instance().EnterZone(quickRestart, zoneName, route, enabled);
}

void ExitZone(bool forceQuit)
{
	AdvertManager::Instance().ExitZone(forceQuit);
}

void RestartZone(void)
{
	AdvertManager::Instance().RestartZone();
}

void TidyZone(void)
{
	AdvertManager::Instance().TidyZone();
}

void ResumeMakingMoney()
{
	AdvertManager::Instance().ResumeMakingMoney();
}

void SuspendMakingMoney()
{
	AdvertManager::Instance().SuspendMakingMoney();
}

void EnterZone_PostLoad(GameResource::C_LevelResource* pLevelResource, GameResource::C_LevelInstance* pLevelInstance)
{
	AdvertManager::Instance().EnterZone_PostLoad(pLevelResource, pLevelInstance);
}

void SetJumpToZoneCallback(JumpToZoneCallback* jtzCallback)
{
	AdvertManager::Instance().SetJumpToZoneCallback(jtzCallback);
}

// Handle messages from the game
MessageController::MessageController( Messaging::C_MessageSubscriberSystem* pSubscriberSystem )
{
	SetSubscriberSystem(pSubscriberSystem);

	AddSubscriber(UI::E_ProfMsg_SignInChanged);
}

MessageController::~MessageController()
{
}

bool MessageController::MessageHandler( const Messaging::C_MessageBase *pMessage )
{
	bool handledMessage = true;

	switch (pMessage->Type())
	{
	case UI::E_ProfMsg_SignInChanged:
		AdvertManager::Instance().LoggedIn();
		break;

	default:
		handledMessage = false;
		break;
	}

	return handledMessage;
}


}
