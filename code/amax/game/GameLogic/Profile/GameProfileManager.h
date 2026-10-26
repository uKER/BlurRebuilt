#ifndef GAMEPROFILE_MANAGER_H
#define GAMEPROFILE_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>
#include <misc_shared/Messaging/Messaging.h>
#include <SystemProfile/SystemProfileManager.h>
#include <GameLogic/Profile/GameProfile.h>

// forward declerations.
namespace RacerNet { struct NetPlayerSharedProfile; }

namespace Game
{

static const int MAX_GAME_PROFILES=7;

class C_GameProfileManager : public Messaging::C_MessageHandler
{
public:

	C_GameProfileManager()
	{
//		mPrimaryProfile=-1;
	};

	void Init();

	static C_GameProfileManager& Instance()			{ return s_instance; }

	C_GameProfile*	GetPrimaryProfile();
	C_GameProfile*	GetProfile( const S32 sIndex );
	S32				GetNumProfiles();

	void BuildNetworkSharedProfileData(const S32 sIndex, RacerNet::NetPlayerSharedProfile& out, bool joinedPartyOrSession = false);

	// For game messages
	bool MessageHandler(const Messaging::C_MessageBase* pMessage);

	// only relevent on the PC build, return NULL if user has yet to create a demonware online account
	char* GetDemonwareUserName(void);
	char* GetDemonwarePassword(void);

private:
	static C_GameProfileManager s_instance;

//	char mPrimaryProfile;
	Btl::Vector<C_GameProfile,Btl::VectorParameters_Array<MAX_GAME_PROFILES> > mProfiles;
};


} // namespace Game

#endif // GAMEPROFILE_MANAGER_H
