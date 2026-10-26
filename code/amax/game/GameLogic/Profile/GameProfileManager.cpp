#include "stdafx.h"

#include <Game/Game.h>

#include <GameLogic/Profile/GameProfileManager.h>
#include <Game/Messaging/GameflowMessages.h>
#include <system/debug/BuildStamp.h>
#include <Network/Player/Data/NetPlayerProfileData.h>

namespace Game
{

C_GameProfileManager C_GameProfileManager::s_instance;

void C_GameProfileManager::Init()
{
	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );
	//AddSubscriber( Game::E_GameflowMessage_ResultsCalculated );
}
	
bool C_GameProfileManager::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	bool handledMessage = true;

	//switch( pMessage->Type() )
	//{

	//	default :
	//		break;
	//}

	return handledMessage;
}

C_GameProfile* C_GameProfileManager::GetPrimaryProfile()
{
	return GetProfile( SysProfile::C_Manager::Instance().GetPrimaryPadNo() );
}

C_GameProfile* C_GameProfileManager::GetProfile( const S32 sIndex )
{
	if ( ( sIndex < 0 ) || ( sIndex >= mProfiles.Size() ) )
		return NULL;

	return &mProfiles[ sIndex ];
}

S32 C_GameProfileManager::GetNumProfiles()
{
	return mProfiles.Size();
}

void C_GameProfileManager::BuildNetworkSharedProfileData( const S32 sIndex, RacerNet::NetPlayerSharedProfile& out, bool joinedPartyOrSession /*= false*/ )
{
	C_GameProfile* gameProf = GetProfile(sIndex);	
	BIZ_ASSERT(gameProf != NULL);
	if (gameProf)
		gameProf->mMultiplayerProfile.BuildNetworkSharedProfileData(out, joinedPartyOrSession);
	else
	{
		// no profile ? flush to default.
		out.Flush();
	}
}

char* C_GameProfileManager::GetDemonwareUserName(void)
{
#if defined (PLATFORM_PC)

#if !defined ( CFG_GOLD )
	if (!stricmp(Debug::GetBuildUser(), "dworswick"))
		return "dworswick";
#endif

#endif // platform pc

	return NULL;
}

char* C_GameProfileManager::GetDemonwarePassword(void)
{
#if defined (PLATFORM_PC)

#if !defined ( CFG_GOLD )
	if (!stricmp(Debug::GetBuildUser(), "dworswick"))
		return "bizarre";
#endif

#endif // platform pc

	return NULL;
}

}; // namespace Game
