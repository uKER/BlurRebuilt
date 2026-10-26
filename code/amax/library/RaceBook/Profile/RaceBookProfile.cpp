#include "stdafx.h"

#include <RaceBook/Profile/RaceBookProfile.h>
#include <RaceBook\RaceBookManager.h>
#include <GameLogic\Profile\GameProfileManager.h>
#include <GameLogic\AchievementsManager.h>
#include "Automation\GameDataLogging.h"
#include <Ui\game\Multiplayer\MultiplayerDefines.h>

#include <Game/Game.h>

#include <algorithm>

#ifndef CFG_GOLD
#include "Ui\UiManager.h"
extern Debug::Enum<UI::C_UiManager::E_LaunchMode> g_UiDebugLaunchMode;
#endif

namespace RaceBook
{

Debug::Logger &g_RaceBookProfileLogger( Debug::GetLogger("RaceBook.Profile") );

C_RaceBookProfile::~C_RaceBookProfile()
{
	for (int i=0; i < mDialogueItems.Size(); i++)
	{
		Game::CharacterConversationVec& vec = *(mDialogueItems[i]);

		for (int j=0; j < vec.Size(); j++)
		{
			delete vec[j];
		}
	}
}

void C_RaceBookProfile::Create(RaceBookCharacter *c)
{
	mID=c->ID;
	mType=c->type;

	SetSubscriberSystem( C_RaceBookManager::Instance().GetSubscriberSystem() );

	mNumFans = 0;
	mNumPreviousFans = 0;

	mCurrentCarIdx	= -1;
	mCash			= c->cash;
	mLevel			= c->level;
	mStatus			= c->status;
	mPerkLevelPoints= c->perkLevel;
	mRacesLost		= c->racesLost;
	mRacesWon		= c->racesWon;
	mAvatarID		= c->avatarID;

	mDialogueGroup = NULL;
		
	mBankBalance=0.f;
	mPreviousBankBalance=0.f;
	mTotalCareerEarnings=0.f;
	mUiCashEarned=0.f;
	mUiFansEarned=0;

	if( c->garage.count )
	{
		mCurrentCarID = c->garage[0]->ID;

		//also add them to the garage

		mGarage.SetCapacity(c->garage.count);

		for(int a=0;a<c->garage.count;a++)
		{
			mGarage.PushBack(c->garage[a]->ID);
		}

		Game::C_GameProfile* gameProf = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
		if(gameProf)
		{
			for( Btl::Set<U32>::Iterator it = gameProf->mVehicleUnlocks.Begin(); it != gameProf->mVehicleUnlocks.End(); it++ )
				mGarage.PushBack(*it);
		}
	}
	else
	{
		LOGWARN(g_RaceBookProfileLogger, "(RaceBookProfile) RaceBookCharacter %s does not have a starting vehicle", c->tag.Text());
		mCurrentCarID = C_RaceBookData::Instance().mVehicles[0]->ID;
	}

	// dialogueItems

	Game::CharacterConversationVec* dialogueItems[] = 
	{
		&mPlayerCollidesWithDriverItems,
		&mDriverCollidesWithPlayerItems,
		&mDriverIsFatallyDamagedItems,
		&mPlayerOvertakesDriverItems,
		&mDriverOvertakesPlayerItems,
		&mPlayerHitByPerkItems,
		&mDriverHitByPerkItems,
		&mPlayerShieldedPerkItems,
		&mDriverShieldedPerkItems,
		&mPlayerMissedPerkItems,
		&mDriverMissedPerkItems,
		&mDriverIsLowHealthItems
	};

	mDialogueItems.SetCapacity(lengthof(dialogueItems));

	for(int i = 0; i < lengthof(dialogueItems); i++)
		mDialogueItems.PushBack(dialogueItems[i]);

	// dialogueRaceItems

	Game::CharacterConversationTracker* dialogueRaceItems[] =
	{
		&mPlayerCollidesWithDriverRaceItems,
		&mDriverCollidesWithPlayerRaceItems,
		&mDriverIsFatallyDamagedRaceItems,
		&mPlayerOvertakesDriverRaceItems,
		&mDriverOvertakesPlayerRaceItems,
		&mPlayerHitByPerkRaceItems,
		&mDriverHitByPerkRaceItems,
		&mPlayerShieldedPerkRaceItems,
		&mDriverShieldedPerkRaceItems,
		&mPlayerMissedPerkRaceItems,
		&mDriverMissedPerkRaceItems,
		&mDriverIsLowHealthRaceItems
	};

	mDialogueRaceItems.SetCapacity(lengthof(dialogueRaceItems));

	for(int i = 0; i < lengthof(dialogueRaceItems); i++)
		mDialogueRaceItems.PushBack(dialogueRaceItems[i]);

	mLifetimeStatistics.Initialise(this, c);

	C_ProfileBase::Init();
}

void C_RaceBookProfile::SendToLua( lua_State* pState )
{
	RaceBookCharacter* pCharacter = C_RaceBookData::Instance().mCharacters[ (GAMEDATA_ID)mID ];

	Lua::AddDataMember( pState, "id",		mID );
	Lua::AddDataMember( pState, "avatar",	mAvatarID );

	if ( pCharacter && ( pCharacter->type == E_CHARACTER_TYPE_PLAYER ) )
	{
		Util::C_String32 s;
		s.Printf( "PROFILE_PAD%d_NAME", SysProfile::C_Manager::Instance().GetPrimaryPadNo() );
		Lua::AddDataMember( pState, "name", s.Chars() );
	}
	else
		Lua::AddDataMember( pState, "name",	( ( pCharacter ) ? pCharacter->loc_string.id : 0 ) );
}

void C_RaceBookProfile::PostRaceUpdate()
{
	int previous_level=mLevel;

	mHasLeveledUp=false;

	for(int a=mLevel;a<C_RaceBookData::Instance().mDefaults[0]->experience_per_level.count;a++)
	{
		if(mLifetimeStatistics.mXpPoints >= C_RaceBookData::Instance().mDefaults[0]->experience_per_level[a])
		{
			mLevel=a;

			if(previous_level!=mLevel)
			{
				mPerkLevelPoints+=C_RaceBookData::Instance().mDefaults[0]->perk_level_points_gained_per_level[mLevel];
				mHasLeveledUp=true;
				previous_level=mLevel;
			}
		}
	}

}

bool C_RaceBookProfile::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	return false;
}

void C_RaceBookProfile::AddCarToGarage(XtId car, bool purchased)
{
	bool add=true;
	for(int a=0;a<mGarage.Size();a++)
	{
		if(mGarage[a].mId == car)
			add=false;
	}

	if(add)
	{
		mGarage.PushBack(car);

		if(GetID() == C_RaceBookManager::Instance().GetPlayerProfile()->GetID())
		{
			Game::C_GameProfileManager::Instance().GetPrimaryProfile()->mUnlockedCar=car;
		}

		UI::C_WideString32 wLocStr;
		wLocStr.Format(L"%s", UI::C_TextStringID(C_RaceBookData::Instance().mVehicles[(GAMEDATA_ID)car]->loc_string.id).String());

		Util::C_String32 locStr = Util::C_String32( wLocStr.AsString() );			

		Game::C_GameProfile* pProf = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
		if(pProf)
		{
			UI::C_WideString64 wLastEvStr;
			//if(pProf->mCurrentEvent)
			//	wLastEvStr.Add(pProf->mCurrentEvent->GetEventName());
#ifndef CFG_GOLD
			AmaxDataLogging::LogVehicleAcquired(locStr.Chars(), purchased, CalculateProgress(), wLastEvStr.AsString().Chars());
#endif
		}
	}
}

void C_RaceBookProfile::BuildLevelStateVehicleUpgrades( Game::LevelStateVehicleUpgrades& upgrades )
{
	upgrades.Clear();
}

// ------------------------------------------------------------------------------------------------

void C_RaceBookProfile::ClearRaceAudioData()
{
	for( int i = 0; i < mDialogueRaceItems.Size(); i++ )
		mDialogueRaceItems[i]->ClearData();
}

void C_RaceBookProfile::BuildRaceDialogue(Btl::Set<U32>* charactersInRace, Btl::Set<U32>* wavebankList)
{
	for( int i = 0; i < mDialogueRaceItems.Size(); i++ )
		BuildRaceDialogueType( charactersInRace, mDialogueItems[i],  mDialogueRaceItems[i], wavebankList );

	SortDialogue();
}

void C_RaceBookProfile::BuildRaceDialogueType( Btl::Set<U32>* charactersInRace, Game::CharacterConversationVec* dialogue, 
											  Game::CharacterConversationTracker* dialogueRace, Btl::Set<U32>* wavebankList )
{
	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	AmaxCity* city = C_GameData::Instance().mCities[(GAMEDATA_ID)levelState.mCity];
	assert(city);
	Util::C_String locationName(city->tag.string);
	locationName.ConvertToUpper();

	AmaxEvent* ev = C_GameEventData::Instance().mEvents[(GAMEDATA_ID)levelState.mEvent];
	assert(ev);
	Util::C_String eventName(ev->tag.string);
	eventName.ConvertToUpper();

	U32 locationNameHash	= Btl::Hash(locationName);
	U32 eventNameHash		= Btl::Hash(eventName);

	bool foundAllChar = false;
	for( int i = 0; i < dialogue->Size(); i++ )
	{
		Game::C_InGameDialogueConversation* conv = (*dialogue)[i];

		if(!conv || !conv->mDlgItemCount)
			continue;

		AmaxInGameDialogue* sorceryDialogue = C_GameData::Instance().mInGameDialogue[(GAMEDATA_ID)conv->mId];

		if(!sorceryDialogue || sorceryDialogue->items.count < (S32) conv->mDlgItemFirstIndex)
			continue;

		AmaxInGameDialogueItem* sorceryItem = (AmaxInGameDialogueItem*)sorceryDialogue->items[conv->mDlgItemFirstIndex];

		if(!sorceryItem)
			continue;

		if( sorceryItem->locationHash && sorceryItem->locationHash != locationNameHash )
			continue;

		if( sorceryItem->eventHash && sorceryItem->eventHash != eventNameHash )
			continue;

		if( !sorceryItem->eventHash && levelState.mSpecificAudioLinesOnly )
			continue;

		foundAllChar	= true;
		for( int j = 0; j < sorceryItem->charactersInvolved.count; j++ )
		{
			Btl::Set<U32>::Iterator charIt = charactersInRace->Find(sorceryItem->charactersInvolved[j]);

			if( charIt == charactersInRace->End() )
			{
				foundAllChar = false;
				break;
			}
		}

		if( !sorceryItem->charactersInvolved.count && conv->mDlgItemCount )
		{
			Btl::Set<U32>::Iterator charIt = charactersInRace->Find(sorceryItem->characterHash);

			if( charIt == charactersInRace->End() )
				foundAllChar = false;
		}

		if(foundAllChar)
		{
			wavebankList->Insert(sorceryItem->splitGroup);
			dialogueRace->mConversationList.PushBack(conv);
		}
	}

	dialogueRace->mCurrentConversationIdx = 0;
}

void C_RaceBookProfile::SortDialogue()
{
	for( int i = 0; i < mDialogueRaceItems.Size(); i++ )
		mDialogueRaceItems[i]->Shuffle();
}

// ------------------------------------------------------------------------------------------------

void C_RaceBookProfile::SerialiseDialogue( Sys::BinarySerialiserAutoEndian& stream, Game::CharacterConversationVec* vec )
{
	Sys::BinarySerialiserAutoEndian out( stream ); 

	stream << vec->Size();
	for(int i = 0; i < vec->Size(); i++)
	{
		stream << *((*vec)[i]); // Oh, hai there ugly pointer mess.
	}
}

void C_RaceBookProfile::DeserialiseDialogue( Sys::BinaryDeserialiserAutoEndian& stream, Game::CharacterConversationVec* vec )
{
	Sys::BinaryDeserialiserAutoEndian in( stream ); 

	int numItems; 
	stream >> numItems;

	for( int i = 0; i < vec->Size(); i++ )
	{
		SAFE_DELETE_PTR((*vec)[i]);
	}

	vec->Clear();
	vec->SetCapacity(numItems);
	for(int i = 0; i < numItems; i++)
	{
		Game::C_InGameDialogueConversation* conv = new Game::C_InGameDialogueConversation();
		assert(conv);
		stream >> *conv;
		vec->PushBack(conv);
	}
}

void C_RaceBookProfile::Serialise( Sys::BinarySerialiserAutoEndian& stream )  
{
	Sys::BinarySerialiserAutoEndian out( stream );
	
	stream << mCurrentCarID << mStatus.id << (S32&)mType << mCurrentCarIdx << mAvatarID;

	if(mType == E_CHARACTER_TYPE_PLAYER)
	{
		stream << mNumFans << mNumPreviousFans << mCash << mBankBalance << mPreviousBankBalance << mTotalCareerEarnings << mLifetimeStatistics;
		stream << mLevel << mPerkLevelPoints << mRacesWon << mRacesLost;
	}

	stream << mGarage.Size();
	for(int a=0;a<mGarage.Size();a++)
	{
		stream << mGarage[a].mId;
	}

	//stream << repIdx;
	//stream << mReputation.Size();
	//for( int i = 0; i < mReputation.Size(); i++ )
	//	stream << mReputation[i];

	for( int i = 0; i < mDialogueItems.Size(); i++ )
		SerialiseDialogue(stream, mDialogueItems[i]);
}

void C_RaceBookProfile::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream ) 
{
	Sys::BinaryDeserialiserAutoEndian in( stream ); 
	
	stream >> mCurrentCarID >> mStatus.id >> (S32&)mType >> mCurrentCarIdx >> mAvatarID;

	if(mType == E_CHARACTER_TYPE_PLAYER)
	{
		stream >> mNumFans >> mNumPreviousFans >> mCash >> mBankBalance >> mPreviousBankBalance >> mTotalCareerEarnings >> mLifetimeStatistics;
		stream >> mLevel >> mPerkLevelPoints >> mRacesWon >> mRacesLost;
	}

	mGarage.Clear();
	int num_cars;
	stream >> num_cars;

	if(num_cars)
		mGarage.SetCapacity(num_cars);

	for(int a=0;a<num_cars;a++)
	{
		U32 id;
		stream >> id;
		mGarage.PushBack(id);
	}

	//mReputation.Clear();
	//int repSize;
	//stream >> repIdx;
	//stream >> repSize;
	//mReputation.SetCapacity(repSize);
	//for( int i = 0; i < repSize; i++ )
	//{
	//	U32 reputation;
	//	stream >> reputation;
	//	mReputation.PushBack(reputation);
	//}

	for (int i = 0; i < mDialogueItems.Size(); i++)
	{
		DeserialiseDialogue(stream, mDialogueItems[i]);
	}
}

} // namespace RaceBook
