#include "stdafx.h"

#include <RaceBook/RaceBookManager.h>
#include <RaceBook/Data/RaceBookData.h>

#include <GameApp.h>
#include <Game/Messaging/GameflowMessages.h>
#include <GameLogic\Profile\GameProfileManager.h>
#include "../Network/Core/NetServices.h"
#include "Demonware/DemonwareLobbyEventHandler.h"

#include <GameLogic/GameData/GameEventData.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>

#include <GameLogic\ContentServer\KeyArchiveManager.h>
#include <GameLogic\ContentServer\RecordEventManager.h>
#include <GameLogic\ContentServer\LinkCodeManager.h>

namespace RaceBook
{

Debug::Logger &g_RaceBookManagerLogger( Debug::GetLogger("RaceBook.Manager") );
C_RaceBookManager C_RaceBookManager::s_instance;

Debug::Bool g_debugGivePlayerCash("RaceBook.GivePlayerOneMillionDollars",false);


C_RaceBookManager::C_RaceBookManager() : m_pDemonwareLobbyEvents(NULL)
{
}

void C_RaceBookManager::Init()
{
	MEMORY_TRACKER_PUSH("game");

	//C_RaceBookData::Instance().Init(C_GameApp::GetInstance()->GetGlobalXtDatabase());

	mbRaceBookActive=false;
	mEventUnlockId = 1;
	
	SetSubscriberSystem( C_RaceBookManager::Instance().GetSubscriberSystem() );
	AddSubscriber( Game::E_GameflowMessage_SaveGameLoaded );

	InitVehicles();

	for (int iProfile=0; iProfile < mProfiles.Size(); iProfile++)
	{
		delete mProfiles[iProfile];
	}

	mProfiles.Clear();
	mProfileMap.Clear();

	mProfiles.SetCapacity(C_RaceBookData::Instance().mCharacters.Number());

	// Make the hash table four times larger than necessary to avoid thashing on lookup, there's
	// better ways to go about this, but for the sake of 2k this'll do :)
	mProfileMap.Reserve(C_RaceBookData::Instance().mCharacters.Number() * 4); 

	for(int a=0;a<C_RaceBookData::Instance().mCharacters.Number();a++)
	{
		RaceBookCharacter* rbChar = C_RaceBookData::Instance().mCharacters[a];
		C_RaceBookProfile *prof = new C_RaceBookProfile;
		prof->Create(C_RaceBookData::Instance().mCharacters[a]);
		prof->repIdx = a;

		mProfiles.PushBack(prof);
		mProfileMap[prof->GetID()] = prof;

		if(rbChar->type == E_CHARACTER_TYPE_PLAYER)
		{
			mPlayerProfileIndex = a;

			if(Game::C_GameProfileManager::Instance().GetPrimaryProfile())
				Game::C_GameProfileManager::Instance().GetPrimaryProfile()->mRaceBookProfile = prof;
		}
	}

	InitCharacterLists();

	// don't want to allocate any demonware structures if the network services are not initialised, demonware memory manager will be invalid. 
	if (RacerNet::C_NetServices::Instance().IsInitialised())
	{
		if (NULL == m_pDemonwareLobbyEvents)
		{
			m_pDemonwareLobbyEvents = new Demonware::C_LobbyEventHandler();
			if (m_pDemonwareLobbyEvents)
				m_pDemonwareLobbyEvents->Register();
		}
	}

	for(int a=0;a<C_GameData::Instance().mRoutes.Number();a++)
	{
		AmaxRoute *r=C_GameData::Instance().mRoutes[a];

		//this is for time attack so add him
		if(r->best_times.count)
		{
			RaceBookTimeAttackStore rbtas;

			rbtas.mRouteID=r->ID;
			for(int b=0;b<MAX_RACEBOOK_TIME_ATTACK_ENTRIES;b++)
			{
				if(b < r->best_times.count)
				{
					RaceBookTimeAttackEntry entry;
					if(r->best_times[b].name)
						entry.mName = r->best_times[b].name->ID;
					else
					{
						entry.mName = 0;
						LOGWARN(g_RaceBookManagerLogger, "Invalid name in Time Attack entry for route %s", r->tag.Text());
					}

					entry.mTime = r->best_times[b].time_in_secs;
					rbtas.mEntries.PushBack(entry);
				}
				else
				{
					RaceBookTimeAttackEntry entry;
					entry.mName = 0;
					entry.mTime = 0;
					rbtas.mEntries.PushBack(entry);
				}
			}

			mTimeAttackStore.PushBack(rbtas);
		}
	}

	MEMORY_TRACKER_POP("game");
}


C_RaceBookProfile* C_RaceBookManager::GetProfileByIndex( U32 uIndex ) 
{
	if ( uIndex < (U32)mProfiles.Size() )
	{
		return mProfiles[uIndex];
	}
	else
	{
		LOGWARN( g_RaceBookManagerLogger, "Invalid index %u", uIndex );
		return NULL;
	}
}

RaceBookTimeAttackStore *C_RaceBookManager::GetTimeAttackStore(U32 routeID)
{
	for(int a=0;a<mTimeAttackStore.Size();a++)
	{
		if(mTimeAttackStore[a].mRouteID == routeID)
		{
			return &mTimeAttackStore[a];
		}
	}

	return NULL;
}

void C_RaceBookManager::Destroy()
{
	if (m_pDemonwareLobbyEvents)
	{
		m_pDemonwareLobbyEvents->UnRegister();
		delete m_pDemonwareLobbyEvents;
		m_pDemonwareLobbyEvents = NULL;
	}
}

void C_RaceBookManager::InitCharacterLists()
{
	// A bit of work now to get all the pointers from IDs 
	// saves having to keep doing an ID search later.

	C_RaceBookData& rbd = C_RaceBookData::Instance();

	for( int i = 0; i < mProfiles.Size(); i++ )
	{
		C_RaceBookProfile* prof = mProfiles[i];

		for( int j = 0; j < rbd.mCharacters.Number(); j++ )
		{
			RaceBookCharacter* character = rbd.mCharacters[j];

			if( character->ID != prof->GetID() )
				continue;
		}
	}
}

void C_RaceBookManager::InitVehicles()
{
	EmptyVehicles();

	mVehicles.SetCapacity(C_RaceBookData::Instance().mVehicles.Number());
	for ( int i = 0; i < C_RaceBookData::Instance().mVehicles.Number(); i++ )
	{
		RaceBookVehicle* vehicleRecord = C_RaceBookData::Instance().mVehicles[i];

		C_RaceBookVehicle* vehicle = new C_RaceBookVehicle();
		vehicle->mID = vehicleRecord->ID;
		vehicle->mCost = vehicleRecord->cost;
		vehicle->mIsForSale = vehicleRecord->isForSale;
		vehicle->mDisallowedInSplitscreen = vehicleRecord->disallowedInSplitscreen; 

		vehicle->mSellTextID = vehicleRecord->sell_text.id;

		if(vehicleRecord->seller)
			vehicle->mSellerID = vehicleRecord->seller->ID;

		if(vehicleRecord->put_up_for_sale_after)
			vehicle->mPutUpForSaleAfter = vehicleRecord->put_up_for_sale_after->ID;
		else
			vehicle->mPutUpForSaleAfter = NULL;

		if( vehicleRecord->vehicle )
		{
			vehicle->mBaseModelID = vehicleRecord->vehicle->ID;
		}
		else
		{
			vehicle->mBaseModelID = C_GameVehicleData::Instance().mVehicles[0]->ID;
			LOGWARN(g_RaceBookManagerLogger, "(RaceBookManager) RaceBookVehicle exists without a valid AmaxVehicle reference.");
		}

		vehicle->mNumberOfBaseSlots=vehicleRecord->mNumberOfBaseSlots;
		vehicle->mPerkSlots=vehicle->mNumberOfBaseSlots;

		vehicle->mAdditionalSlotsCost.SetCapacity(vehicleRecord->mAdditionalSlotsCost.count);

		for(int a=0;a<vehicleRecord->mAdditionalSlotsCost.count;a++)
		{
			vehicle->mAdditionalSlotsCost.PushBack(vehicleRecord->mAdditionalSlotsCost[a]);
		}

		mVehicles.PushBack(vehicle);
	}
}

void C_RaceBookManager::Update()
{
	PERF_EVENT("C_RaceBookManager::Update");

	if(g_debugGivePlayerCash)
	{
		g_debugGivePlayerCash=false;
		GetPlayerProfile()->mBankBalance+=1000000;
	}
}

void C_RaceBookManager::UpdateDemonware(void)
{
	if (m_pDemonwareLobbyEvents)
		m_pDemonwareLobbyEvents->Update();

	//[PK] test stuff
	//static bool do_read=false;
	//if(do_read)
	//{
	//	do_read=false;
	//	bool gotID=Game::C_KeyArchiveManager::Instance().SetUserIndex(0);	//need to get proper index

	//	if(gotID)
	//	{
	//		Game::C_KeyArchiveManager::Instance().ReadKeyArchive();
	//	}
	//}

	//static bool do_write=false;
	//if(do_write)
	//{
	//	do_write=false;
	//	bool gotID=Game::C_KeyArchiveManager::Instance().SetUserIndex(0);	//need to get proper index

	//	if(gotID)
	//	{
	//		Game::C_KeyArchiveManager::Instance().ClearKeyArchive();

	//		int num_pairs=27;
	//		for(int a=0;a<num_pairs;a++)
	//		{
	//			Game::C_KeyArchiveManager::Instance().AddKeyArchivePair((Game::KEY_ARCHIVE_INDEX_KEY)a,a+1000);
	//		}

	//		Game::C_KeyArchiveManager::Instance().WriteKeyArchive();
	//	}
	//}

	Game::C_LinkCodeManager::Instance().Update();
	Game::C_KeyArchiveManager::Instance().Update();
	Game::C_RecordEventManager::Instance().Update();
}

bool C_RaceBookManager::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	bool handledMessage = true;

	switch( pMessage->Type() )
	{
		case Game::E_GameflowMessage_SaveGameLoaded:
			{
				if(!Game::C_GameProfileManager::Instance().GetPrimaryProfile()->mRaceBookStarted)
				{
					Game::C_GameProfileManager::Instance().GetPrimaryProfile()->mRaceBookStarted=true;

					U32 playerID=0;
					for(int a=0;a<C_RaceBookData::Instance().mCharacters.Number();a++)
					{
						RaceBookCharacter *c=C_RaceBookData::Instance().mCharacters[a];
						if(c->type == E_CHARACTER_TYPE_PLAYER)
						{
							playerID=c->ID;
						}
					}

					for( int i = 0; i < C_GameData::Instance().mCheatPacks.Number(); i++)
					{
						AmaxCheatBase* cheatBase = C_GameData::Instance().mCheatPacks[i];
						if(cheatBase->GetType() == E_CHEAT_TYPE_VEHICLE && cheatBase->dlcLicensed)
						{
							AmaxCheatVehicle* cheatVehicle = (AmaxCheatVehicle*)cheatBase;

							if(cheatVehicle->mRaceBookVehicle.Object())
								GetPlayerProfile()->AddCarToGarage(cheatVehicle->mRaceBookVehicle->ID, false);
						}
					}
				}
			}
			break;

		default :
			break;
	}

	return handledMessage;
}

void C_RaceBookManager::CreatePlayerMessage( XtId msgLoc, XtId from, XtId to, bool isPrivate, bool isHot )
{

}

C_RaceBookProfile *C_RaceBookManager::GetProfileByID(GAMEDATA_ID id)
{
	if (!id.id)
		return 0;

	Btl::OpenHashMap<XtId, C_RaceBookProfile*>::Iterator result = mProfileMap.Find(id.id);

	if (result != mProfileMap.End())
	{
		return result->GetValue();
	}

	LOGWARN(g_RaceBookManagerLogger, "Attempted to find profile with invalid ID %08X", id.id);

	return NULL;
}

C_RaceBookVehicle *C_RaceBookManager::GetVehicle(GAMEDATA_ID pID)
{
	for(int a=0;a<mVehicles.Size();a++)
	{
		C_RaceBookVehicle* vehicle = mVehicles[a];
		RaceBookVehicle* vehicle2 = C_RaceBookData::Instance().mVehicles[(GAMEDATA_ID)pID];

		if(mVehicles[a]->mID == pID)
		{
			return mVehicles[a];
		}
	}

	return NULL;
}

bool C_RaceBookManager::CheckQuantifier(RaceBookQuantifier quantifier, float num, float parameter)
{
	switch( quantifier )
	{
	case E_QUANTIFIER_NULL:
		return ( num != 0 ); // Else we get an error. Stupid cast.
		break;

	case E_QUANTIFIER_LESSTHAN:
		return ( num < parameter );
		break;

	case E_QUANTIFIER_LESSTHANOREQUAL:
		return ( num <= parameter );
		break;

	case E_QUANTIFIER_EQUAL:
		return ( num == parameter );
		break;

	case E_QUANTIFIER_GREATERTHAN:
		return ( num > parameter );
		break;

	case E_QUANTIFIER_GREATERTHANOREQUAL:
		return ( num >= parameter );
		break;
	}

	return false;
}

GAMEDATA_ID C_RaceBookManager::WhoOwnsCars(GAMEDATA_ID vehicle_id)
{
	for(int a=0;a<mProfiles.Size();a++)
	{
		C_RaceBookProfile *p=mProfiles[a];

		if(p==NULL)
		{
			BIZ_ASSERT_FAIL("WhoOwnsCars: Profile is NULL for index %d",a);
			return GAMEDATA_ID();
		}

		for(int b=0;b<p->mGarage.Size();b++)
		{
			if(p->mGarage[b].mId == vehicle_id)
			{
				return (GAMEDATA_ID)p->GetID();
			}
		}
	}

	return GAMEDATA_ID();
}

} // namespace RaceBook
