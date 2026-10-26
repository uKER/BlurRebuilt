#ifndef RACEBOOK_MANAGER_H
#define RACEBOOK_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>
#include <RaceBook\Profile\RaceBookProfile.h>
#include <RaceBOok\Objects\RaceBookVehicle.h>
#include <misc_shared/Messaging/Messaging.h>
#include <GameLogic\GameData\GameDataDefs.h>

namespace Demonware { class C_LobbyEventHandler; }

namespace RaceBook
{

const int MAX_RACEBOOK_PROFILES=40;

const int MAX_RACEBOOK_TIME_ATTACK_ENTRIES=10;

const int DEFAULT_REPUTATION=50;

struct RaceBookTimeAttackEntry
{
	U32		mName;
	float	mTime;
};

struct RaceBookTimeAttackStore
{
	U32 mRouteID;
	Btl::Vector<RaceBookTimeAttackEntry,Btl::VectorParameters_Static<MAX_RACEBOOK_TIME_ATTACK_ENTRIES> > mEntries;
};

class C_RaceBookGroup;

class C_RaceBookManager : public Messaging::C_MessageHandler
{
	public:
		C_RaceBookManager();

		void Init();
		void Destroy();
		void Update();

		void UpdateDemonware(void);
		
		static C_RaceBookManager& Instance(){return s_instance;};

		C_RaceBookProfile* GetProfileByIndex( U32 uIndex );
				
		int NumProfiles(){return mProfiles.Size();};
		
		C_RaceBookProfile *GetProfileByID(GAMEDATA_ID pID);
		C_RaceBookProfile *GetPlayerProfile() { return mProfiles[mPlayerProfileIndex]; };

		Messaging::C_MessageSubscriberSystem *GetSubscriberSystem(void) {return &mMessageSubscriberSystem;}

		void				InitVehicles();
		void AddVehicle( C_RaceBookVehicle* v ) { mVehicles.PushBack(v); };
		int					GetNumVehicles()	{ return mVehicles.Size(); };
		C_RaceBookVehicle*	GetVehicle( int v )	{ return mVehicles[v]; };
		C_RaceBookVehicle*	GetVehicle( GAMEDATA_ID v );
		void				EmptyVehicles()
		{
			for (int i=0; i < mVehicles.Size(); i++)
			{
				delete mVehicles[i];
			}
			mVehicles.Clear();
		}

		void InitCharacterLists();

		Btl::Vector<RaceBookTimeAttackStore> mTimeAttackStore;
		RaceBookTimeAttackStore *GetTimeAttackStore(U32 routeID);

		// Uses the messaging system to create a user generated message.
		void CreatePlayerMessage( XtId msgLoc, XtId from, XtId to, bool isPrivate, bool isHot=false );

		// For game messages
		bool MessageHandler(const Messaging::C_MessageBase* pMessage);

		bool CheckQuantifier(RaceBookQuantifier quantifier, float num, float parameter);

		const bool IsActive() const
		{
			return mbRaceBookActive;
		}

		void SetActive( const bool active )
		{
			mbRaceBookActive = active;
		}

		void LookingAtVehicle(GAMEDATA_ID id){mLookingAtVehicleID=id;};
		GAMEDATA_ID GetLookingAtVehicle(){return mLookingAtVehicleID;};
		
		GAMEDATA_ID WhoOwnsCars(GAMEDATA_ID vehicle_id);

		U8 GetUnlockId(){ return mEventUnlockId++; }

	private:
		static C_RaceBookManager s_instance;

		Messaging::C_MessageSubscriberSystem mMessageSubscriberSystem;

		Btl::Vector<C_RaceBookProfile*> mProfiles;
		Btl::OpenHashMap<XtId, C_RaceBookProfile*> mProfileMap;

		GAMEDATA_ID mLookingAtVehicleID;

		GAMEDATA_ID mPlayerProfileIndex;

		bool		mbRaceBookActive;
		U8			mEventUnlockId;

		// Seems to make more sense for the manager to have these as they are shared amongst profiles.
		Btl::Vector<C_RaceBookVehicle*> mVehicles;

		Demonware::C_LobbyEventHandler* m_pDemonwareLobbyEvents;
};

} // namespace RaceBook

#endif // RACEBOOK_MANAGER_H
