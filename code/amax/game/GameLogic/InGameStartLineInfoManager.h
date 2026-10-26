#ifndef INGAME_START_LINE_INFO_MANAGER_H
#define INGAME_START_LINE_INFO_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>

#include <misc_shared/Messaging/Messaging.h>
#include <misc_shared\Messaging\MessageMultiHandler.h>

#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>

#include <GameLogic/ActorStudioActors/StartLineInfoActor.h>

namespace Game
{

typedef struct ccInfoType
{
	::Actors::StartInfoType mType;
	Actor::C_ActorHandle mHandle;
} StartLineInfoType;


class C_InGameStartLineInfoManager : public Messaging::C_MessageMultiHandler
{
public:

	C_InGameStartLineInfoManager()
	{
	};

	void Init();
	void Update();
	void Clear();

	static C_InGameStartLineInfoManager& Instance(){return s_instance;};

	// For game messages
	bool MessageHandler(const Messaging::C_MessageBase* pMessage);

	void AddStartLineInfo(::Actors::StartInfoType type, Actor::C_ActorHandle handle);

	void RecheckViewports();

	Btl::Vector< StartLineInfoType >&	GetStore( const S32 sLocalPlayerIndex )				{ return mStartLineInfoTypeStore[ sLocalPlayerIndex ]; }

private:

	static C_InGameStartLineInfoManager s_instance;
	
	float mPreviousLogicPosition; // Used only in SinglePlayer

	Btl::Vector< Btl::Vector<StartLineInfoType> > mStartLineInfoTypeStore; // Each player has a vector of start line info actors
	bool			 mIsSplitScreen;
	bool			 mRegistrationOnCreationAllowed; 
	unsigned int	 mNumLocalPlayers; 

	Btl::Vector< ::Actors::StartInfoType > mSSPlayerStartInfo;  // In split screen mode store 'active' start line info on a per player basis. 
	Btl::Vector<float> mSSPreviousLogicPosition; 

	bool mStartLinePassed;

	// For activating/deactivating splitscreen info actors on a per player basis
	void SetupSplitScreenRaceStart(); 
	void SetupInfoActorsForLocalPlayer(unsigned int localPlayerIndex); 
	void UpdateSplitScreenStartLineInfo(); 
	void CloneStartLineInfoActors(const unsigned int numLocalPlayers);
	void BindActorEffectsToViewports(const unsigned int numLocalPlayers); 

};

} // namespace Game

#endif // INGAME_START_LINE_INFO_MANAGER_H