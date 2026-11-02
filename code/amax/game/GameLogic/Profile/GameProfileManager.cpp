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

// [Blur2PC] T261: retail account fields (StoreOnlineAccountDetails, 0x00c1cba0).
// Credentials belong to the selected game's save, never an external text file.
char* C_GameProfileManager::GetDemonwareUserName(void)
{
#if defined (PLATFORM_PC)
    C_GameProfile* profile = GetPrimaryProfile();
    if (profile && profile->mOptions.mOnlineLoginName[0])
        return profile->mOptions.mOnlineLoginName;
#endif
    return NULL;
}

char* C_GameProfileManager::GetDemonwarePassword(void)
{
#if defined (PLATFORM_PC)
    C_GameProfile* profile = GetPrimaryProfile();
    if (profile && profile->mOptions.mOnlineLoginPassword[0])
        return profile->mOptions.mOnlineLoginPassword;
#endif
    return NULL;
}

}; // namespace Game
