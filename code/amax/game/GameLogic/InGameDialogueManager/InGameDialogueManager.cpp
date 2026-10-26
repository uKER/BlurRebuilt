#include "stdafx.h"

#include <GameLogic\InGameDialogueManager\InGameDialogueManager.h>
#include <GameLogic\Messaging\GameLogicMessages.h>
#include <Game\Game.h>
#include <GameLogic\RaceRules\RaceRulesManager.h>
#include <Game\Messaging\VehicleActorMessages.h>
#include <RaceBook\Data\RaceBookData.h>
#include <RaceBook\RaceBookManager.h>
#include <GameLogic\Messaging\InGameDialogueMessages.h>
#include <Game\Messaging\PickupActorMessages.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include <GameLogic\RaceLogicCore\Core\RaceLogicCore.h>

#include <GameApp.h>
#include <Game/Game.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceLogicLayer.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <ai/AI_Messages.h>

#include <Game/GameActors/Components/Messages/AudioMessages.h>
#include <GameLogic\ActorStudioActors\CutSceneMessages.h>
#include <GameLogic\Messaging\GameLogicMessages.h>

#include <Game\Messaging\GameflowMessages.h>
#include <system/serialise/TextSerialise.h>

#include "Automation\GameDataLogging.h"
#include "Ui/UiMessages.h"

#include <algorithm>

#if 0
#define RECORDTIME() U64 timeStart = (U64)(Time::GetPerformanceCounter());
#define SHOWTIME(msg) do { U64 t = (U64)(Time::GetPerformanceCounter()); DebugLn("*** %s %i", msg, (DWORD)((t - timeStart) / 1000)); timeStart = t; } while(0)
#else
#define RECORDTIME()
#define SHOWTIME(msg)
#endif

Debug::Logger &g_InGameDialogueLogger( Debug::GetLogger("Dialogue") );

Debug::Bool  g_debugDialogueDisable("DialogueManager.DisableDialogue", false, Debug::Persist());
Debug::Bool  g_debugEnableTimeout("DialogueManager.EnableTimeout", false);
Debug::Float g_debugDialogueFinishTimeout("DialogueManager.FinishTimeout_NonPersist", 30);
Debug::Float g_debugDialogueOvertakeDistance("DialogueManager.OvertakeDistance", 6, Debug::Persist());

static void PlayAllInGameDialogue(void *)
{
	Game::C_InGameDialogueManager::Instance().PlayAllDialogue();
}
Debug::Function g_debugPlayAllInGameDialogue("DialogueManager.PlayAll", PlayAllInGameDialogue, 0);

namespace Game
{
	void C_InGameDialogueThread::ThreadMain()
	{
		//XSetThreadProcessor(GetCurrentThread(), 1);

		//C_InGameDialogueManager::Instance().InitRaceInThread();

#ifdef COMPILER_VC
		Close();
#endif
	}

	void C_InGameDialogueManager::JoinThread()
	{
		if (mInGameDialogueThread)
		{
			mInGameDialogueThread->Join();
			SAFE_DELETE_PTR(mInGameDialogueThread);
		}
	}

	// --------------------------------------------------

	void C_InGameDialogueManager::Initialise()
	{
		Game::CharacterConversationVec* dialogueItems[] = 
		{
			&mPlayerCollidesWithEnvironmentItems,
			&mPlayerIsFatallyDamagedItems,
			&mPlayerIsLowHealthItems,
			&mPlayerObjectivePassingSecondLapItems,
			&mPlayerObjectivePassingLastLapItems,
			&mPlayerObjectivePassingLastLapLastSplitItems,
			&mPlayerObjectiveFailingSecondLapItems,
			&mPlayerObjectiveFailingLastLapItems,
			&mPlayerObjectiveFailingLastLapLastSplitItems,
			&mPlayerObjectiveIntroItems,
			&mPlayerObjectivePassedItems,
			&mPlayerObjectiveFailedItems,
			&mPlayerObjectiveRetryItems,
		};

		mDialogueItems.SetCapacity(lengthof(dialogueItems));

		for(int i = 0; i < lengthof(dialogueItems); i++)
			mDialogueItems.PushBack(dialogueItems[i]);

		Game::CharacterConversationTracker* dialogueRaceItems[] =
		{
			&mPlayerCollidesWithEnvironmentRaceItems,
			&mPlayerIsFatallyDamagedRaceItems,
			&mPlayerIsLowHealthRaceItems,
			&mPlayerObjectivePassingSecondLapRaceItems,
			&mPlayerObjectivePassingLastLapRaceItems,
			&mPlayerObjectivePassingLastLapLastSplitRaceItems,
			&mPlayerObjectiveFailingSecondLapRaceItems,
			&mPlayerObjectiveFailingLastLapRaceItems,
			&mPlayerObjectiveFailingLastLapLastSplitRaceItems,
			&mPlayerObjectiveIntroRaceItems,
			&mPlayerObjectivePassedRaceItems,
			&mPlayerObjectiveFailedRaceItems,
			&mPlayerObjectiveRetryRaceItems,
		};

		mDialogueRaceItems.SetCapacity(lengthof(dialogueRaceItems));

		for(int i = 0; i < lengthof(dialogueRaceItems); i++)
			mDialogueRaceItems.PushBack(dialogueRaceItems[i]);

		// Major AI
		mHash_DriverCollidesWithPlayer					= Btl::Hash("AICOLLIDESWITHPLAYER");
		mHash_PlayerCollidesWithDriver					= Btl::Hash("PLAYERCOLLIDESWITHAI");
		mHash_PlayerIsFatallyDamaged					= Btl::Hash("PLAYERFATALLYDAMAGED");
		mHash_PlayerIsLowHealth							= Btl::Hash("PLAYERLOWHEALTH");
		mHash_DriverIsLowHealth							= Btl::Hash("AILOWHEALTH");
		mHash_PlayerOvertakesDriver						= Btl::Hash("PLAYEROVERTAKESAI");
		mHash_DriverOvertakesPlayer						= Btl::Hash("AIOVERTAKESPLAYER");
		mHash_PlayerHitByPerk							= Btl::Hash("AIHITSPLAYERWITHPERK");
		mHash_DriverHitByPerk							= Btl::Hash("PLAYERHITSAIWITHPERK");
		mHash_PlayerShieldedPerk						= Btl::Hash("PLAYERDEFENDSWITHSHIELD");
		mHash_DriverShieldedPerk						= Btl::Hash("AIDEFENDSWITHSHIELD");
		mHash_PlayerMissedPerk							= Btl::Hash("PLAYERMISSEDPERK");
		mHash_DriverMissedPerk							= Btl::Hash("AIMISSEDPERK");

		// Minor AI
		mHash_AIUnhappy									= Btl::Hash("AIUNHAPPY");
		mHash_AIHappy									= Btl::Hash("AIHAPPY");
		mHash_AIMissesPlayer							= Btl::Hash("AIMISSESPLAYER");
		mHash_AIDefendsPlayer							= Btl::Hash("AIDEFENDSPLAYER");

		// Shared AI
		mHash_DriverIsFatallyDamaged					= Btl::Hash("AIFATALLYDAMAGED");

		// Globals
		mHash_PlayerCollidesWithEnvironment				= Btl::Hash("PLAYERCOLLIDESWITHENVIRONMENT");
		mHash_PlayerObjectiveSecondLapPassing			= Btl::Hash("EVENTSECONDLAPPASS");
		mHash_PlayerObjectiveLastLapPassing				= Btl::Hash("EVENTLASTLAPPASS");
		mHash_PlayerObjectiveLastLapLastSplitPassing	= Btl::Hash("EVENTLASTSPLITPASS");
		mHash_PlayerObjectiveSecondLapFailing			= Btl::Hash("EVENTSECONDLAPFAIL");
		mHash_PlayerObjectiveLastLapFailing				= Btl::Hash("EVENTLASTLAPFAIL");
		mHash_PlayerObjectiveLastLapLastSplitFailing	= Btl::Hash("EVENTLASTSPLITFAIL");
		mHash_PlayerObjectiveIntro						= Btl::Hash("INTRO");
		mHash_PlayerObjectiveRetry						= Btl::Hash("RETRYRACE");
		mHash_PlayerObjectivePassed						= Btl::Hash("WONEVENT");
		mHash_PlayerObjectiveFailed						= Btl::Hash("FAILEDRACE");

		mInGameDialogueThread = NULL;
		mForcePlayAllDialogue = false;
	}

	C_InGameDialogueManager C_InGameDialogueManager::s_instance;

	void C_InGameDialogueManager::InitProfiles()
	{
		ClearAllData();

		// --------------------------------------------------
		// Initialise all the hash values we will need later.
		// --------------------------------------------------
		for( int i = 0; i < C_RaceBookData::Instance().mCharacters.Number(); i++ )
		{
			RaceBookCharacter* character = C_RaceBookData::Instance().mCharacters[i];

			if ( character->type != E_CHARACTER_TYPE_NORMAL && character->type != E_CHARACTER_TYPE_PLAYER )
			{
				Util::C_String characterName(character->tag.Text());
				characterName.ConvertToUpper();

				U32 hash													= Btl::Hash(characterName);
				Btl::InsertionResult<RBCharacterHashMap::Iterator> result	= mCharacterHashMap.Insert(character->ID, hash);
				Btl::InsertionResult<RBCharacterHashMap::Iterator> result2	= mHashCharacterMap.Insert(hash, character->ID);

				if( !result.inserted || !result2.inserted )
					LOGWARN(g_InGameDialogueLogger, "InGameDialogueManager failed to add character %s to the hash map.\n", character->tag.string);
			}
			else if( character->type == E_CHARACTER_TYPE_NORMAL )
			{
				Util::C_String characterName(character->dialogueGroup.Text());
				characterName.ConvertToUpper();

				U32 dialogueHash = Btl::Hash(characterName.Chars());
				Btl::Map<U32, C_InGameDialogueGroup*>::Iterator it = mDialogueHashGroupMap.Find(dialogueHash);

				RaceBook::C_RaceBookProfile* prof = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)character->ID);

				if(it != mDialogueHashGroupMap.End())
				{
					C_InGameDialogueGroup* group = it->GetValue();
					prof->mDialogueGroup = group;
					group->mMembers.PushBack(prof->GetID());
				}
				else
				{
					C_InGameDialogueGroup* group = new C_InGameDialogueGroup();
					mDialogueHashGroupMap.Insert(dialogueHash, group);
					prof->mDialogueGroup = group;
					group->mMembers.PushBack(prof->GetID());
				}
			}
		}

		// --------------------------------------------------
		//    Sort all our records into sub-categories. 
		// --------------------------------------------------

		int problemsFound = 0;

		for( int i = 0; i < C_GameData::Instance().mInGameDialogue.Number(); i++ )
		{
			AmaxInGameDialogue* dialogue = C_GameData::Instance().mInGameDialogue[i];

			if(dialogue->ignorePack)
				continue;

			C_InGameDialogueConversation*	currentConversation		= 0;
			U32								currentConversationId	= 0;

			for( int j = 0; j < dialogue->items.count; j++ )
			{
				AmaxInGameDialogueItemBase* dlgItem = dialogue->items[j];

				if (dlgItem && (dlgItem->GetType() == E_AmaxDialogueItemType_Basic))
				{
					AmaxInGameDialogueItem* item = (AmaxInGameDialogueItem*)dlgItem;

					if( !item->characterHash || !item->triggerHash || !item->weighting )
					{
						LOGDEBUG(g_InGameDialogueLogger, "InGameDialogueManager item %u missing character or trigger hash or has zero weighting.\n", item->uid);
						currentConversation = 0;
						problemsFound++;
						continue;
					}

					if(item->uid == 5499)
					{
						int xx = 0;
					}

					if( !currentConversation || (currentConversationId != item->conversationId))
					{
						if(CharacterConversationVec* charConvVec = GetCharacterConversationMapFromHash(item))
						{
							C_InGameDialogueConversation* conversation = new C_InGameDialogueConversation();

							conversation->mDlgItemFirstIndex	= (U32)j;
							conversation->mDlgItemCount			= 1;
							conversation->mTimesFired			= 0.0f;
							conversation->mId					= dialogue->ID;

							currentConversation		= conversation;
							currentConversationId	= item->conversationId;

							charConvVec->PushBack(conversation);
						}
						else
						{
							currentConversation = 0;
							problemsFound++;
						}
					}
					else
						currentConversation->mDlgItemCount++;
				}
				else
				{
					LOGDEBUG(g_InGameDialogueLogger, "InGameDialogueManager failed to handle dialogue type.\n");
					currentConversation = 0;
					problemsFound++;
				}
			}
		}

		if (problemsFound)
		{
			LOGWARN(g_InGameDialogueLogger, "InGameDialogueManager: %d problems found. Change logging to 'debug' for 'dialogue' to see problems", problemsFound);
		}
	}

	// --------------------------------------------------------------------------

	void C_InGameDialogueManager::InitRace()
	{
		if(!mInGameDialogueThread)
		{
			mInGameDialogueThread = new C_InGameDialogueThread();
			mInGameDialogueThread->Start(0, 0, "InGameDialogueManager");
		}

		//InitRaceInThread();
	}

	void C_InGameDialogueManager::InitRaceInThread()
	{
		SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );
		AddSubscriber( E_GameflowMessage_RaceStarted );
		AddSubscriber( E_GameflowMessage_RaceFinished );
		AddSubscriber( ActorMsg::E_CutSceneMsg_Start321 );
		AddSubscriber( E_GameflowMessage_QuitRace );
		AddSubscriber( E_GameflowMessage_QuitAndWinRace );
		AddSubscriber( E_GameflowMessage_QuitLevel );
		AddSubscriber( GameMessaging::E_GameMsg_Update );
		AddSubscriber( E_GameLogicMessage_Passed_All_Rule );
		AddSubscriber( E_GameLogicMessage_Failed_Rule );
		AddSubscriber( E_VehicleMessage_FatallyDamaged );
		AddSubscriber( ActorMsg::E_AudioMsg_DialogueFinish);
		AddSubscriber( E_GameLogicMessage_LapIncremented );
		AddSubscriber( E_VehicleLogicMsg_HitByPerk );
		AddSubscriber( E_GameLogicMessage_GameplayImpact );
		AddSubscriber( E_VehicleMessage_OvertakeWithNitro );
		AddSubscriber( E_VehicleMessage_Overtake );
		AddSubscriber( E_VehicleMessage_ChangedHealthLevel );
		AddSubscriber( E_GameLogicMessage_SplitCrossed );
		AddSubscriber( E_VehicleLogicMsg_ShieldedPerk );
		AddSubscriber( E_AIMessage_AttemptedSideSwipe );
		AddSubscriber( E_VehicleLogicMsg_MissedPerk );

		mEnabled				= false;
		mCurrentlyPlayingAudio	= false;
		mRetry					= false;

		mRandGenerator.Seed((U32)Time::GetCurrentTime());

		ClearRaceData();

		// Build the list of everybody involved in the race
		Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
		for( U32 i = 0; i < levelState.mNumCars; i++ )
		{
			RaceBook::C_RaceBookProfile* prof = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)levelState.mCars[i].mCharacterID);

			if(!prof)
				continue;

			if(prof->mType == E_CHARACTER_TYPE_NORMAL)
			{
				mMinorCharIdsInvolvedInRace.Insert(prof->GetID());
			}
			else
			{
				RBCharacterHashMap::Iterator it = mCharacterHashMap.Find(prof->GetID());

				if(it != mCharacterHashMap.End())
				{
					mCharactersInvolvedInRace.Insert(it->GetValue());
				}
			}
		}

		InGameDialogueAddWavebank wavebankMsg;
		
		// Set up the character dialogue for each race that involves the character directly
		for( U32 i = 0; i < levelState.mNumCars; i++ )
		{
			RaceBook::C_RaceBookProfile* prof = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)levelState.mCars[i].mCharacterID);

			if( prof && prof->mType != E_CHARACTER_TYPE_NORMAL && prof->mType != E_CHARACTER_TYPE_PLAYER ) 
				prof->BuildRaceDialogue(&mCharactersInvolvedInRace, &wavebankMsg.mSplitGroupList);
		}

		// Set up the character dialogue for each race that does not involve the character directly
		for( int i = 0; i < mDialogueItems.Size(); i++ )
			BuildAndSortRaceDialogue( mDialogueItems[i], mDialogueRaceItems[i], &wavebankMsg.mSplitGroupList );

		// Minor AI group dialogue
		for ( Btl::Map<U32, C_InGameDialogueGroup*>::Iterator it = mDialogueHashGroupMap.Begin(); it != mDialogueHashGroupMap.End(); it++ )
		{
			C_InGameDialogueGroup* group = it->GetValue();

			bool noCharsInRace = true;
			for( int i = 0; i < group->mMembers.Size(); i++ )
			{
				Btl::Set<U32>::Iterator inRaceIt = mMinorCharIdsInvolvedInRace.Find(group->mMembers[i]);

				if(inRaceIt != mMinorCharIdsInvolvedInRace.End())
				{
					noCharsInRace = false;
					break;
				}
			}

			if(noCharsInRace)
				continue;

			BuildAndSortRaceDialogue( &group->mAIDefendsItems,			&group->mAIDefendsItemsRaceItems,	&wavebankMsg.mSplitGroupList );
			BuildAndSortRaceDialogue( &group->mAIHappyItems,			&group->mAIHappyItemsRaceItems,		&wavebankMsg.mSplitGroupList );
			BuildAndSortRaceDialogue( &group->mAIMissesItems,			&group->mAIMissesItemsRaceItems,	&wavebankMsg.mSplitGroupList );
			BuildAndSortRaceDialogue( &group->mAIUnhappyItems,			&group->mAIUnhappyItemsRaceItems,	&wavebankMsg.mSplitGroupList );
			BuildAndSortRaceDialogue( &group->mAIFatallyDamagedItems,	&group->mAIFatallyDamagedRaceItems,	&wavebankMsg.mSplitGroupList );
		}

		// Tell audio to prepare the wavebanks
		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&wavebankMsg);

		for( Btl::Set<U32>::Iterator it = wavebankMsg.mSplitGroupList.Begin(); it != wavebankMsg.mSplitGroupList.End(); it++ )
			LOGDEBUG(g_InGameDialogueLogger, "Telling audio to load split group %u.", (U32)*it);

		// Cache the overall event probability
		mCurrentEventProbability	= 0.f;
		AmaxEvent* pAmaxEv			=  C_GameEventData::Instance().mEvents[(GAMEDATA_ID)levelState.mEvent];

		if(pAmaxEv)
			mCurrentEventProbability = pAmaxEv->dialogueProbability;
	}

	void C_InGameDialogueManager::BuildAndSortRaceDialogue( Game::CharacterConversationVec* dialogue, Game::CharacterConversationTracker* raceDialogue, 
		Btl::Set<U32>* wavebankList )
	{
		Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
		bool foundAllChar = false;

		AmaxCity* city = C_GameData::Instance().mCities[(GAMEDATA_ID)levelState.mCity];
		assert(city);
		Util::C_String locationName(city->tag.string);
		locationName.ConvertToUpper();

		AmaxEvent* ev = C_GameEventData::Instance().mEvents[(GAMEDATA_ID)levelState.mEvent];
		assert(ev);
		Util::C_String eventName(ev->tag.string);
		eventName.ConvertToUpper();

		U32 locationNameHash	= Btl::Hash(locationName.Chars());
		U32 eventNameHash		= Btl::Hash(eventName.Chars());

		for( int i = 0; i < dialogue->Size(); i++ )
		{
			foundAllChar = true;
			C_InGameDialogueConversation* conv = (*dialogue)[i];

			AmaxInGameDialogue* sorceryDialogue = C_GameData::Instance().mInGameDialogue[(GAMEDATA_ID)conv->mId];
			
			if (!sorceryDialogue)
			{
				LOGERROR(g_InGameDialogueLogger, "sorceryDialogue == NULL");
				continue;
			}

			if (conv->mDlgItemFirstIndex >= (U32)sorceryDialogue->items.count)
			{
				LOGERROR(g_InGameDialogueLogger, "conv->mDlgItemFirstIndex(%d) >= sorceryDialogue->items.count(%d)", conv->mDlgItemFirstIndex, sorceryDialogue->items.count);
				continue;
			}

			AmaxInGameDialogueItem* sorceryItem = (AmaxInGameDialogueItem*)sorceryDialogue->items[conv->mDlgItemFirstIndex];

			if(!sorceryItem)
			{
				LOGERROR(g_InGameDialogueLogger, "sorceryItem == NULL");
				continue;
			}

			if(sorceryItem->locationHash && sorceryItem->locationHash != locationNameHash)
				continue;

			if(sorceryItem->eventHash && sorceryItem->eventHash != eventNameHash)
				continue;

			if(!sorceryItem->eventHash && levelState.mSpecificAudioLinesOnly)
				continue;

			for( int j = 0; j < sorceryItem->charactersInvolved.count; j++ )
			{
				Btl::Set<U32>::Iterator charIt = mCharactersInvolvedInRace.Find(sorceryItem->charactersInvolved[j]);

				if( charIt == mCharactersInvolvedInRace.End() )
				{
					Btl::Map<U32, C_InGameDialogueGroup*>::Iterator groupIt = mDialogueHashGroupMap.Find(sorceryItem->charactersInvolved[j]);

					if(groupIt == mDialogueHashGroupMap.End())
					{
						foundAllChar = false;
						break;
					}
				}
			}

			if( !sorceryItem->charactersInvolved.count && conv->mDlgItemCount )
			{
				Btl::Set<U32>::Iterator charIt = mCharactersInvolvedInRace.Find(sorceryItem->characterHash);

				if( charIt == mCharactersInvolvedInRace.End() )
				{
					Btl::Map<U32, C_InGameDialogueGroup*>::Iterator groupIt = mDialogueHashGroupMap.Find(sorceryItem->characterHash);

					if(groupIt == mDialogueHashGroupMap.End())
					{
						foundAllChar = false;
					}
				}
			}

			if(foundAllChar)
			{
				wavebankList->Insert(sorceryItem->splitGroup);
				raceDialogue->mConversationList.PushBack(conv);
			}
		}

		raceDialogue->Shuffle();
	}

	void C_InGameDialogueManager::ClearAllData()
	{
		RECORDTIME();

		mCharacterHashMap.Clear();
		mHashCharacterMap.Clear();

		SHOWTIME("Dlg::Clear::Hash");

		for( int i = 0; i < mDialogueItems.Size(); i++ )
			ClearDataItems(mDialogueItems[i]);

		SHOWTIME("Dlg::Clear::Items");

		for( Btl::Map<U32, C_InGameDialogueGroup*>::Iterator it = mDialogueHashGroupMap.Begin(); it != mDialogueHashGroupMap.End(); it++ )
		{
			C_InGameDialogueGroup* group = it->GetValue();
			group->ClearAllItems();
		}

		SHOWTIME("Dlg::Clear::Groups");

		for (Btl::Map<U32, C_InGameDialogueGroup*>::Iterator it = mDialogueHashGroupMap.Begin(); it != mDialogueHashGroupMap.End(); it++)
		{
			delete it->GetValue();
		}

		mDialogueHashGroupMap.Clear();

		SHOWTIME("Dlg::Clear::GroupHash");

		ClearRaceData();

		SHOWTIME("Dlg::Clear::Race");
	}

	void C_InGameDialogueManager::ClearRaceData()
	{
		mCurrentConversation = NULL;
		mCurrentDialogueIdx = -1;
		mLastLocId = 0;

		mCharactersInvolvedInRace.Clear();
		mMinorCharIdsInvolvedInRace.Clear();

		for( int i = 0; i < mDialogueRaceItems.Size(); i++ )
			mDialogueRaceItems[i]->ClearData();

		for( Btl::Map<U32, C_InGameDialogueGroup*>::Iterator it = mDialogueHashGroupMap.Begin(); it != mDialogueHashGroupMap.End(); it++ )
		{
			C_InGameDialogueGroup* group = it->GetValue();
			group->ClearRaceItems();
		}

		for( int i = 0; i < RaceBook::C_RaceBookManager::Instance().NumProfiles(); i++ )
			RaceBook::C_RaceBookManager::Instance().GetProfileByIndex(i)->ClearRaceAudioData();
	}

	void C_InGameDialogueManager::ClearDataItems(CharacterConversationVec* vec)
	{
		for( int i = 0; i < vec->Size(); i++ )
			SAFE_DELETE_PTR((*vec)[i]);

		vec->Clear();
	}



	CharacterConversationVec* C_InGameDialogueManager::GetCharacterConversationMapFromHash(AmaxInGameDialogueItem* basicDialogueItem)
	{
		RBCharacterHashMap::Iterator charIt = mHashCharacterMap.Find(basicDialogueItem->characterHash);

		if( charIt != mHashCharacterMap.End() )
		{
			RaceBook::C_RaceBookProfile* prof = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)charIt->GetValue());
			RaceBookCharacter* character = C_RaceBookData::Instance().mCharacters[(GAMEDATA_ID)prof->GetID()];

			if(character->type != E_CHARACTER_TYPE_NORMAL)
			{
				if( basicDialogueItem->triggerHash == mHash_PlayerCollidesWithDriver )
					return &prof->mPlayerCollidesWithDriverItems;
				else if( basicDialogueItem->triggerHash == mHash_DriverCollidesWithPlayer )
					return &prof->mDriverCollidesWithPlayerItems;
				else if( basicDialogueItem->triggerHash == mHash_PlayerCollidesWithEnvironment )
					return &mPlayerCollidesWithEnvironmentItems;
				else if( basicDialogueItem->triggerHash == mHash_PlayerObjectiveSecondLapPassing )
					return &mPlayerObjectivePassingSecondLapItems;
				else if( basicDialogueItem->triggerHash == mHash_PlayerObjectiveLastLapPassing )
					return &mPlayerObjectivePassingLastLapItems;
				else if( basicDialogueItem->triggerHash == mHash_PlayerObjectiveLastLapLastSplitPassing )
					return &mPlayerObjectivePassingLastLapLastSplitItems;
				else if( basicDialogueItem->triggerHash == mHash_PlayerObjectiveSecondLapFailing )
					return &mPlayerObjectiveFailingSecondLapItems;
				else if( basicDialogueItem->triggerHash == mHash_PlayerObjectiveLastLapFailing )
					return &mPlayerObjectiveFailingLastLapItems;
				else if( basicDialogueItem->triggerHash == mHash_PlayerObjectiveLastLapLastSplitFailing )
					return &mPlayerObjectiveFailingLastLapLastSplitItems;
				else if( basicDialogueItem->triggerHash == mHash_PlayerHitByPerk )
					return &prof->mPlayerHitByPerkItems;
				else if( basicDialogueItem->triggerHash == mHash_DriverHitByPerk )
					return &prof->mDriverHitByPerkItems;
				else if( basicDialogueItem->triggerHash == mHash_PlayerShieldedPerk )
					return &prof->mPlayerShieldedPerkItems;
				else if( basicDialogueItem->triggerHash == mHash_DriverShieldedPerk )
					return &prof->mDriverShieldedPerkItems;
				else if( basicDialogueItem->triggerHash == mHash_PlayerIsFatallyDamaged )
					return &mPlayerIsFatallyDamagedItems;
				else if( basicDialogueItem->triggerHash == mHash_DriverIsFatallyDamaged )
					return &prof->mDriverIsFatallyDamagedItems;
				else if( basicDialogueItem->triggerHash == mHash_DriverIsLowHealth )
					return &prof->mDriverIsLowHealthItems;
				else if( basicDialogueItem->triggerHash == mHash_PlayerIsLowHealth )
					return &mPlayerIsLowHealthItems;
				else if( basicDialogueItem->triggerHash == mHash_PlayerOvertakesDriver )
					return &prof->mPlayerOvertakesDriverItems;
				else if( basicDialogueItem->triggerHash == mHash_DriverOvertakesPlayer )
					return &prof->mDriverOvertakesPlayerItems;
				else if( basicDialogueItem->triggerHash == mHash_PlayerMissedPerk )
					return &prof->mPlayerMissedPerkItems;
				else if( basicDialogueItem->triggerHash == mHash_DriverMissedPerk )
					return &prof->mDriverMissedPerkItems;
				else if( basicDialogueItem->triggerHash == mHash_PlayerObjectiveIntro)
					return &mPlayerObjectiveIntroItems;
				else if( basicDialogueItem->triggerHash == mHash_PlayerObjectiveRetry)
					return &mPlayerObjectiveRetryItems;
				else if( basicDialogueItem->triggerHash == mHash_PlayerObjectivePassed)
					return &mPlayerObjectivePassedItems;
				else if( basicDialogueItem->triggerHash == mHash_PlayerObjectiveFailed)
					return &mPlayerObjectiveFailedItems;
			}
		}
		else
		{
			Btl::Map<U32, C_InGameDialogueGroup*>::Iterator groupIt = mDialogueHashGroupMap.Find(basicDialogueItem->characterHash);

			if(groupIt != mDialogueHashGroupMap.End())
			{
				C_InGameDialogueGroup* group = groupIt->GetValue();

				if( basicDialogueItem->triggerHash == mHash_AIHappy )
					return &group->mAIHappyItems;
				else if( basicDialogueItem->triggerHash == mHash_AIUnhappy )
					return &group->mAIUnhappyItems;
				else if( basicDialogueItem->triggerHash == mHash_AIMissesPlayer )
					return &group->mAIMissesItems;
				else if( basicDialogueItem->triggerHash == mHash_AIDefendsPlayer )
					return &group->mAIDefendsItems;
				else if( basicDialogueItem->triggerHash == mHash_DriverIsFatallyDamaged )
					return &group->mAIFatallyDamagedItems;
			}
			else
			{
#ifndef CFG_GOLD
				const char* triggerName = basicDialogueItem->triggerName.Text();
#else
				const char* triggerName = "";
#endif

				LOGDEBUG(g_InGameDialogueLogger, "InGameDialogueManager: Failed to find the character - uid(%lu) characterHash(%lu)  trigger(%s)", basicDialogueItem->uid, basicDialogueItem->characterHash, triggerName);
				return NULL;
			}
		}

#ifndef CFG_GOLD
		const char* triggerName = basicDialogueItem->triggerName.Text();
#else
		const char* triggerName = "";
#endif

		LOGDEBUG(g_InGameDialogueLogger, "InGameDialogueManager: Failed to handle trigger - uid(%lu) characterHash(%lu) trigger(%s)", basicDialogueItem->uid, basicDialogueItem->characterHash, triggerName);

		return NULL;
	}

	// ---------------------------------------------------------------------------------------------------------------

	void C_InGameDialogueManager::GetTriggerNameFromHash(Util::C_String32& triggerName, U32 hash)
	{
		if( hash == mHash_PlayerCollidesWithDriver )
			triggerName.Printf("PlayerCollidesWithDriver");
		else if( hash == mHash_DriverCollidesWithPlayer )
			triggerName.Printf("DriverCollidesWithPlayer");
		else if( hash == mHash_PlayerCollidesWithEnvironment )
			triggerName.Printf("PlayerCollidesWithEnvironment");
		else if( hash == mHash_PlayerObjectiveSecondLapPassing )
			triggerName.Printf("PlayerObjectiveSecondLapPassing");
		else if( hash == mHash_PlayerObjectiveLastLapPassing )
			triggerName.Printf("PlayerObjectiveLastLapPassing");
		else if( hash == mHash_PlayerObjectiveLastLapLastSplitPassing )
			triggerName.Printf("PlayerObjectiveLastLapLastSplitPassing");
		else if( hash == mHash_PlayerObjectiveSecondLapFailing )
			triggerName.Printf("PlayerObjectiveSecondLapFailing");
		else if( hash == mHash_PlayerObjectiveLastLapFailing )
			triggerName.Printf("PlayerObjectiveLastLapFailing");
		else if( hash == mHash_PlayerObjectiveLastLapLastSplitFailing )
			triggerName.Printf("PlayerObjectiveLastLapLastSplitFailing");
		else if( hash == mHash_PlayerHitByPerk )
			triggerName.Printf("PlayerHitByPerk");
		else if( hash == mHash_DriverHitByPerk )
			triggerName.Printf("DriverHitByPerk");
		else if( hash == mHash_PlayerShieldedPerk )
			triggerName.Printf("PlayerDefendsWithShield");
		else if( hash == mHash_DriverShieldedPerk )
			triggerName.Printf("DriverDefendsWithShield");
		else if( hash == mHash_PlayerIsFatallyDamaged )
			triggerName.Printf("PlayerIsFatallyDamaged");
		else if( hash == mHash_DriverIsFatallyDamaged )
			triggerName.Printf("DriverIsFatallyDamaged");
		else if( hash == mHash_DriverIsLowHealth )
			triggerName.Printf("DriverIsLowHealth");
		else if( hash == mHash_PlayerIsLowHealth )
			triggerName.Printf("PlayerIsLowHealth");
		else if( hash == mHash_PlayerOvertakesDriver )
			triggerName.Printf("PlayerOvertakesDriver");
		else if( hash == mHash_DriverMissedPerk )
			triggerName.Printf("DriverMissedPerk");
		else if( hash == mHash_PlayerMissedPerk )
			triggerName.Printf("PlayerMissedPerk");
		else if( hash == mHash_AIHappy )
			triggerName.Printf("AIHappy");
		else if( hash == mHash_AIUnhappy )
			triggerName.Printf("AIUnhappy");
		else if( hash == mHash_AIMissesPlayer )
			triggerName.Printf("AIMissesPlayer");
		else if( hash == mHash_AIDefendsPlayer )
			triggerName.Printf("AIDefendsPlayer");
	}

	// ---------------------------------------------------------------------------------------------------------------

	void C_InGameDialogueManager::CheckImpactDialogue(const Game::C_GameplayImpactMsg* pMsg)
	{
		C_LevelState&			state			= C_Game::GetInstance()->GetLevelState();
		const LevelStateCar*	vehicle1		= &state.mCars[pMsg->mVehicleIdx];
		const LevelStateCar*	vehicle2		= NULL;
		U32						playerId		= RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->GetID();

		if(	pMsg->mOtherVehicleIdx != INVALID_VEHICLE_IDX )
			vehicle2 = &state.mCars[pMsg->mOtherVehicleIdx]; 

		if(!C_GameData::Instance().mDialogueTriggerControls.Number())
		{
			LOGWARN(g_InGameDialogueLogger, "Bad dialogue trigger controls - cannot resolve impact dialogue.");
			return;
		}

		if(vehicle2)
		{
			for( int i = 0; i < C_GameData::Instance().mDialogueTriggerControls[0]->impactTriggerDialogue.characterProbabilities.count; i++ )
			{
				if( pMsg->mImpactLevel == C_GameData::Instance().mDialogueTriggerControls[0]->impactTriggerDialogue.characterProbabilities[i].impactLevel )
				{
					if( vehicle1->mCharacterID == playerId )
					{
						RaceBookCharacter* character = C_RaceBookData::Instance().mCharacters[(GAMEDATA_ID)vehicle2->mCharacterID];
						RaceBook::C_RaceBookProfile* prof = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)vehicle2->mCharacterID);

						float randNum = mRandGenerator.FRand(0.f, 1.f);
						if(randNum > character->dialogueProbability)
							return;

						if( pMsg->mAggressiveVehicleIdx == pMsg->mVehicleIdx )
						{
							if(randNum > C_GameData::Instance().mDialogueTriggerControls[0]->impactTriggerDialogue.characterProbabilities[i].probabilityPlayer)
								return;

							if(character->type == E_CHARACTER_TYPE_NORMAL && prof->mDialogueGroup)
							{
								prof->mDialogueGroup->mCurrentCharacter = prof;
								prof->mDialogueGroup->mAIUnhappyItemsRaceItems.SelectAndFireConversation();
							}
							else
							{				
								prof->mPlayerCollidesWithDriverRaceItems.SelectAndFireConversation();
							}
						}
						else
						{
							if(randNum > C_GameData::Instance().mDialogueTriggerControls[0]->impactTriggerDialogue.characterProbabilities[i].probabilityAI)
								return;

							if(character->type == E_CHARACTER_TYPE_NORMAL && prof->mDialogueGroup)
							{
								prof->mDialogueGroup->mCurrentCharacter = prof;
								prof->mDialogueGroup->mAIHappyItemsRaceItems.SelectAndFireConversation();
							}
							else
							{				
								prof->mDriverCollidesWithPlayerRaceItems.SelectAndFireConversation();
							}
						}
					}
					else if( vehicle2->mCharacterID == playerId )
					{
						RaceBookCharacter* character = C_RaceBookData::Instance().mCharacters[(GAMEDATA_ID)vehicle1->mCharacterID];
						RaceBook::C_RaceBookProfile* prof = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)vehicle1->mCharacterID);

						float randNum = mRandGenerator.FRand(0.f, 1.f);
						if(randNum > character->dialogueProbability)
							return;

						if( pMsg->mAggressiveVehicleIdx == pMsg->mOtherVehicleIdx )
						{
							if(randNum > C_GameData::Instance().mDialogueTriggerControls[0]->impactTriggerDialogue.characterProbabilities[i].probabilityPlayer)
								return;

							if(character->type == E_CHARACTER_TYPE_NORMAL && prof->mDialogueGroup)
							{
								prof->mDialogueGroup->mCurrentCharacter = prof;
								prof->mDialogueGroup->mAIUnhappyItemsRaceItems.SelectAndFireConversation();
							}
							else
							{				
								prof->mPlayerCollidesWithDriverRaceItems.SelectAndFireConversation();
							}
						}
						else
						{
							if(randNum > C_GameData::Instance().mDialogueTriggerControls[0]->impactTriggerDialogue.characterProbabilities[i].probabilityAI)
								return;

							if(character->type == E_CHARACTER_TYPE_NORMAL && prof->mDialogueGroup)
							{
								prof->mDialogueGroup->mCurrentCharacter = prof;
								prof->mDialogueGroup->mAIHappyItemsRaceItems.SelectAndFireConversation();
							}
							else
							{				
								prof->mDriverCollidesWithPlayerRaceItems.SelectAndFireConversation();
							}
						}
					}
					return;
				}
			}
		}
		else if( vehicle1->mCharacterID == playerId )
		{
			for( int i = 0; i < C_GameData::Instance().mDialogueTriggerControls[0]->impactTriggerDialogue.environmentProbabilities.count; i++ )
			{
				if( pMsg->mImpactLevel == C_GameData::Instance().mDialogueTriggerControls[0]->impactTriggerDialogue.environmentProbabilities[i].impactLevel )
				{
					if(mRandGenerator.FRand(0.f, 1.f) > C_GameData::Instance().mDialogueTriggerControls[0]->impactTriggerDialogue.environmentProbabilities[i].probability)
						return;

					mPlayerCollidesWithEnvironmentRaceItems.SelectAndFireConversation();

					return;
				}
			}
		}
	}

	void C_InGameDialogueManager::CheckObjectiveDialogue(bool lastSplit, int vehIdx, int lapNum, int splitNum)
	{
		const C_LevelState& levelState				= C_Game::GetInstance()->GetLevelState();
		const LevelStateCar* vehicle				= &levelState.mCars[vehIdx];

		if( vehicle->mCharacterID != RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->GetID() )
			return;


		bool passing = true;
		for( int i = 0; i < C_RaceRulesManager::Instance().GetNumPassRules(); i++ )
		{
			if( !C_RaceRulesManager::Instance().HasPassed(vehIdx, i, true) )
			{
				passing = false;
				break;
			}
		}
		
		if( splitNum == 0 )
		{
			if( lapNum == levelState.mLaps )
			{
				if( passing )
					mPlayerObjectivePassingLastLapRaceItems.SelectAndFireConversation();
				else
					mPlayerObjectiveFailingLastLapRaceItems.SelectAndFireConversation();
			}
			else if( lapNum == 2 )
			{
				if( passing )
					mPlayerObjectivePassingSecondLapRaceItems.SelectAndFireConversation();
				else
					mPlayerObjectiveFailingSecondLapRaceItems.SelectAndFireConversation();
			}
		}
		else if( lapNum == levelState.mLaps && splitNum == 2 )
		{
			if( passing )
				mPlayerObjectivePassingLastLapLastSplitRaceItems.SelectAndFireConversation();
			else
				mPlayerObjectiveFailingLastLapLastSplitRaceItems.SelectAndFireConversation();
		}
	}

	void C_InGameDialogueManager::CheckPerkDialogue(U8 firingVehIdx, U8 hitVehIdx, bool special)
	{
		C_LevelState&			state			= C_Game::GetInstance()->GetLevelState();
		const LevelStateCar*	firingVehicle	= &state.mCars[firingVehIdx];
		const LevelStateCar*	hitVehicle		= &state.mCars[hitVehIdx]; 
		U32						playerId		= RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->GetID();

		if( !firingVehicle || !hitVehicle )
			return;

		if( firingVehicle->mCharacterID == playerId )
		{
			RaceBookCharacter* character = C_RaceBookData::Instance().mCharacters[(GAMEDATA_ID)hitVehicle->mCharacterID];
			RaceBook::C_RaceBookProfile* prof = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)hitVehicle->mCharacterID);

			float randNum = mRandGenerator.FRand(0.f, 1.f);
			if(randNum > character->dialogueProbability)
				return;

			AmaxDialogueTriggerControls* pTest = C_GameData::Instance().mDialogueTriggerControls[0];

			if((!special && randNum > C_GameData::Instance().mDialogueTriggerControls[0]->hitByPerkTriggerDialogue.probabilityAI) || 
			   (special && randNum > C_GameData::Instance().mDialogueTriggerControls[0]->hitByPerkTriggerDialogue.probabilityAISpecial))
			{
				return;
			}

			if(character->type == E_CHARACTER_TYPE_NORMAL && prof->mDialogueGroup)
			{
				prof->mDialogueGroup->mCurrentCharacter = prof;
				prof->mDialogueGroup->mAIUnhappyItemsRaceItems.SelectAndFireConversation();
			}
			else
			{				
				prof->mDriverHitByPerkRaceItems.SelectAndFireConversation();
			}
		}
		else if( hitVehicle->mCharacterID == playerId )
		{
			RaceBookCharacter* character = C_RaceBookData::Instance().mCharacters[(GAMEDATA_ID)firingVehicle->mCharacterID];
			RaceBook::C_RaceBookProfile* prof = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)firingVehicle->mCharacterID);

			float randNum = mRandGenerator.FRand(0.f, 1.f);
			if(randNum > character->dialogueProbability)
				return;

			if((!special && randNum > C_GameData::Instance().mDialogueTriggerControls[0]->hitByPerkTriggerDialogue.probabilityAI) || 
			   (special && randNum > C_GameData::Instance().mDialogueTriggerControls[0]->hitByPerkTriggerDialogue.probabilityAISpecial))
			{
				return;
			}

			if(character->type == E_CHARACTER_TYPE_NORMAL && prof->mDialogueGroup)
			{
				prof->mDialogueGroup->mCurrentCharacter = prof;
				prof->mDialogueGroup->mAIHappyItemsRaceItems.SelectAndFireConversation();
			}
			else
			{				
				prof->mPlayerHitByPerkRaceItems.SelectAndFireConversation();
			}
		}
	}

	void C_InGameDialogueManager::CheckMissedPerkDialogue(U8 firingVehIdx, U8 hitVehIdx)
	{
		C_LevelState&			state			= C_Game::GetInstance()->GetLevelState();
		U32						playerId		= RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->GetID();

		if( firingVehIdx == INVALID_VEHICLE_IDX || hitVehIdx == INVALID_VEHICLE_IDX || firingVehIdx > state.mNumCars || hitVehIdx > state.mNumCars )
			return;

		const LevelStateCar*	firingVehicle	= &state.mCars[firingVehIdx];
		const LevelStateCar*	hitVehicle		= &state.mCars[hitVehIdx]; 

		if( firingVehicle->mCharacterID == playerId )
		{
			RaceBookCharacter* character = C_RaceBookData::Instance().mCharacters[(GAMEDATA_ID)hitVehicle->mCharacterID];
			RaceBook::C_RaceBookProfile* prof = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)hitVehicle->mCharacterID);

			if(!character || !prof)
				return;

			float randNum = mRandGenerator.FRand(0.f, 1.f);
			if(randNum > character->dialogueProbability)
				return;

			if(character->type == E_CHARACTER_TYPE_NORMAL && prof->mDialogueGroup)
			{
				prof->mDialogueGroup->mCurrentCharacter = prof;
				prof->mDialogueGroup->mAIHappyItemsRaceItems.SelectAndFireConversation();
			}
			else
			{				
				prof->mPlayerMissedPerkRaceItems.SelectAndFireConversation();
			}
		}
		else if( hitVehicle->mCharacterID == playerId )
		{
			RaceBookCharacter* character = C_RaceBookData::Instance().mCharacters[(GAMEDATA_ID)firingVehicle->mCharacterID];
			RaceBook::C_RaceBookProfile* prof = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)firingVehicle->mCharacterID);

			if(!character || !prof)
				return;

			float randNum = mRandGenerator.FRand(0.f, 1.f);
			if(randNum > character->dialogueProbability)
				return;

			if(character->type == E_CHARACTER_TYPE_NORMAL && prof->mDialogueGroup)
			{
				prof->mDialogueGroup->mCurrentCharacter = prof;
				prof->mDialogueGroup->mAIMissesItemsRaceItems.SelectAndFireConversation();
			}
			else
			{				
				prof->mDriverMissedPerkRaceItems.SelectAndFireConversation();
			}
		}
	}

	void C_InGameDialogueManager::CheckOvertakeDialogue(U8 overtakerIdx, U8 overtakeeIndex, bool withNitro)
	{
		C_LevelState& state							= C_Game::GetInstance()->GetLevelState();

		const LevelStateCar* overtakerVehicle = &state.mCars[overtakerIdx];
		const LevelStateCar* overtakeeVehicle = &state.mCars[overtakeeIndex]; 

		Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(overtakerIdx);
		float dist = pLogicActor->mVehicleProximities[overtakeeIndex].mDistanceSquared;
		if(dist >= g_debugDialogueOvertakeDistance * g_debugDialogueOvertakeDistance)
			return;

		if( overtakerVehicle->mCharacterID == RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->GetID() )
		{
			RaceBookCharacter* character = C_RaceBookData::Instance().mCharacters[(GAMEDATA_ID)overtakeeVehicle->mCharacterID];

			float randNum = mRandGenerator.FRand(0.f, 1.f);
			if(randNum > character->dialogueProbability)
				return;

			if((!withNitro && randNum > C_GameData::Instance().mDialogueTriggerControls[0]->overtakeTriggerDialogue.probabilityPlayer) || 
				(withNitro && randNum > C_GameData::Instance().mDialogueTriggerControls[0]->overtakeTriggerDialogue.probabilityWithNitroPlayer))
			{
				return;
			}

			RaceBook::C_RaceBookProfile* prof =RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)overtakeeVehicle->mCharacterID);
			prof->mPlayerOvertakesDriverRaceItems.SelectAndFireConversation();
		}
		else if ( overtakeeVehicle->mCharacterID == RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->GetID() )
		{
			RaceBookCharacter* character = C_RaceBookData::Instance().mCharacters[(GAMEDATA_ID)overtakerVehicle->mCharacterID];

			float randNum = mRandGenerator.FRand(0.f, 1.f);
			if(randNum > character->dialogueProbability)
				return;

			if((!withNitro && randNum > C_GameData::Instance().mDialogueTriggerControls[0]->overtakeTriggerDialogue.probabilityAI) || 
				(withNitro && randNum > C_GameData::Instance().mDialogueTriggerControls[0]->overtakeTriggerDialogue.probabilityWithNitroAI))
			{
				return;
			}

			RaceBook::C_RaceBookProfile* prof =RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)overtakerVehicle->mCharacterID);
			prof->mDriverOvertakesPlayerRaceItems.SelectAndFireConversation();
		}
	}

	void C_InGameDialogueManager::CheckFatallyDamagedDialogue(U8 vehIdx)
	{
		C_LevelState& state								= C_Game::GetInstance()->GetLevelState();
		const LevelStateCar* vehicle					= &state.mCars[vehIdx];

		if( vehicle->mCharacterID != RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->GetID() )
		{
			RaceBook::C_RaceBookProfile* prof	= RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)vehicle->mCharacterID);
			RaceBookCharacter* character		= C_RaceBookData::Instance().mCharacters[(GAMEDATA_ID)vehicle->mCharacterID];

			if(character->type == E_CHARACTER_TYPE_NORMAL && prof->mDialogueGroup)
			{
				prof->mDialogueGroup->mCurrentCharacter = prof;
				prof->mDialogueGroup->mAIFatallyDamagedRaceItems.SelectAndFireConversation();
			}
			else
				prof->mDriverIsFatallyDamagedRaceItems.SelectAndFireConversation();
		}
		else
		{
			mPlayerIsFatallyDamagedRaceItems.SelectAndFireConversation();
		}
	}

	void C_InGameDialogueManager::CheckLowHealthDialogue(U8 vehIdx, E_HEALTH_LEVEL healthLevel)
	{
		if( healthLevel == E_HEALTH_LEVEL_LOW )
		{
			C_LevelState& state								= C_Game::GetInstance()->GetLevelState();
			const LevelStateCar* vehicle					= &state.mCars[vehIdx];

			if( vehicle->mCharacterID != RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->GetID() )
			{
				RaceBook::C_RaceBookProfile* prof =RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)vehicle->mCharacterID);
				prof->mDriverIsLowHealthRaceItems.SelectAndFireConversation();
			}
			else
			{
				mPlayerIsLowHealthRaceItems.SelectAndFireConversation();
			}
		}
	}

	void C_InGameDialogueManager::CheckShieldedPerkDialogue(U8 firingVehIdx, U8 shieldedVehIdx)
	{
		C_LevelState&			state				= C_Game::GetInstance()->GetLevelState();
		const LevelStateCar*	firingVehicle		= &state.mCars[firingVehIdx];
		const LevelStateCar*	shieldingVehicle	= &state.mCars[shieldedVehIdx]; 
		U32						playerId			= RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->GetID();

		if( !firingVehicle || !shieldingVehicle )
			return;

		if( shieldingVehicle->mCharacterID == playerId )
		{
			RaceBookCharacter* character = C_RaceBookData::Instance().mCharacters[(GAMEDATA_ID)firingVehicle->mCharacterID];
			RaceBook::C_RaceBookProfile* prof = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)firingVehicle->mCharacterID);

			float randNum = mRandGenerator.FRand(0.f, 1.f);
			if(randNum > character->dialogueProbability)
				return;

			if(randNum > C_GameData::Instance().mDialogueTriggerControls[0]->shieldTriggerDialogue.probabilityPlayer)
				return;

			if(character->type == E_CHARACTER_TYPE_NORMAL && prof->mDialogueGroup)
			{
				prof->mDialogueGroup->mCurrentCharacter = prof;
				prof->mDialogueGroup->mAIMissesItemsRaceItems.SelectAndFireConversation();
			}
			else
			{				
				prof->mPlayerShieldedPerkRaceItems.SelectAndFireConversation();
			}
		}
		else if( firingVehicle->mCharacterID == playerId )
		{
			RaceBookCharacter* character = C_RaceBookData::Instance().mCharacters[(GAMEDATA_ID)shieldingVehicle->mCharacterID];
			RaceBook::C_RaceBookProfile* prof = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)shieldingVehicle->mCharacterID);

			float randNum = mRandGenerator.FRand(0.f, 1.f);
			if(randNum > character->dialogueProbability)
				return;

			if(randNum > C_GameData::Instance().mDialogueTriggerControls[0]->shieldTriggerDialogue.probabilityAI)
				return;

			if(character->type == E_CHARACTER_TYPE_NORMAL && prof->mDialogueGroup)
			{
				prof->mDialogueGroup->mCurrentCharacter = prof;
				prof->mDialogueGroup->mAIDefendsItemsRaceItems.SelectAndFireConversation();
			}
			else
			{				
				prof->mDriverShieldedPerkRaceItems.SelectAndFireConversation();
			}
		}
	}

	void C_InGameDialogueManager::CheckSideSwipeDialogue(U8 doingIdx, U8 againstIdx)
	{
		C_LevelState&			state			= C_Game::GetInstance()->GetLevelState();
		const LevelStateCar*	doingVehicle	= &state.mCars[doingIdx];
		const LevelStateCar*	againstVehicle	= &state.mCars[againstIdx]; 
		U32						playerId		= RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->GetID();

		if( !doingVehicle || !againstVehicle )
			return;

		if( againstVehicle->mCharacterID == playerId )
		{
			// Temp to remind designers that audio is needed for this.
			RaceBook::C_RaceBookProfile* prof = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)doingVehicle->mCharacterID);
			LOGDEBUG(g_InGameDialogueLogger, "Potential Audio: %ls - %s\n", UI::C_TextStringID(prof->GetName().id).String(), "I'm going to sideswipe you!");
		}
	}

	// ---------------------------------------------------------------------------------------

	void C_InGameDialogueManager::Serialise( Sys::BinarySerialiserAutoEndian& stream ) 
	{
		for( int i = 0; i < mDialogueItems.Size(); i++ )
			SerialiseItems(stream, mDialogueItems[i]);

		stream << mCharacterHashMap.Size();
		for( Game::RBCharacterHashMap::Iterator it = mCharacterHashMap.Begin(); it != mCharacterHashMap.End(); it++ )
			stream << it->GetKey() << it->GetValue();
		
		stream << mHashCharacterMap.Size();
		for( Game::RBCharacterHashMap::Iterator it = mHashCharacterMap.Begin(); it != mHashCharacterMap.End(); it++ )
			stream << it->GetKey() << it->GetValue();

		stream << mDialogueHashGroupMap.Size();
		for( Btl::Map<U32, C_InGameDialogueGroup*>::Iterator it = mDialogueHashGroupMap.Begin(); it != mDialogueHashGroupMap.End(); it++ )
			stream << it->GetKey() << *it->GetValue();
	}

	void C_InGameDialogueManager::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		RECORDTIME();

		ClearAllData();

		SHOWTIME("Dlg::Clear");

		for( int i = 0; i < mDialogueItems.Size(); i++ )
			DeserialiseItems(stream, mDialogueItems[i]);

		SHOWTIME("Dlg::Items");

		U32 key;
		U32 value;
		int size;

		mCharacterHashMap.Clear();
		stream >> size;
		for( int i = 0; i < size; i++ )
		{
			stream >> key >> value;
			mCharacterHashMap.Insert(key, value);
		}


		SHOWTIME("Dlg::CharHash");

		mHashCharacterMap.Clear();
		stream >> size;
		for( int i = 0; i < size; i++ )
		{
			stream >> key >> value;
			mHashCharacterMap.Insert(key, value);
		}

		SHOWTIME("Dlg::HashChar");

		for (Btl::Map<U32, C_InGameDialogueGroup*>::Iterator it = mDialogueHashGroupMap.Begin(); it != mDialogueHashGroupMap.End(); it++)
		{
			delete it->GetValue();
		}
		mDialogueHashGroupMap.Clear();
		stream >> size;
		for( int i = 0; i < size; i++ )
		{
			C_InGameDialogueGroup* group = new C_InGameDialogueGroup();
			stream >> key >> *group;
			mDialogueHashGroupMap.Insert(key, group);

			for( int i = 0; i < group->mMembers.Size(); i++ )
			{
				RaceBook::C_RaceBookProfile* prof = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)group->mMembers[i]);
				prof->mDialogueGroup = group;
			}
		}

		SHOWTIME("Dlg::Groups");
	}

	void C_InGameDialogueManager::SerialiseItems( Sys::BinarySerialiserAutoEndian& stream, Game::CharacterConversationVec* item )
	{
		Sys::BinarySerialiserAutoEndian out( stream ); 

		out << item->Size();
		for( int i = 0; i < item->Size(); i++ )
		{
			out << *((*item)[i]);
		}
	}

	void C_InGameDialogueManager::DeserialiseItems( Sys::BinaryDeserialiserAutoEndian& stream, Game::CharacterConversationVec* item )
	{
		Sys::BinaryDeserialiserAutoEndian in( stream ); 
		int numItems;

		in >> numItems;
		item->Reserve(numItems);
		for( int i = 0; i < numItems; i++ )
		{
			C_InGameDialogueConversation* conv = new C_InGameDialogueConversation();
			in >> *conv;
			item->PushBack(conv);
		}
	}

	// ---------------------------------------------------------------------------------------

	void C_InGameDialogueManager::PlayAllDialogue()
	{
		mEnabled = false;
		mForcePlayAllDialogue = true;
		mDialogueIdx = mProfIdx = mProfDialogueIdx = mGroupIdx = 0;
	}

	void C_InGameDialogueManager::PlayNextDialogue()
	{
		if( mDialogueIdx < mDialogueRaceItems.Size() )
		{
			mDialogueRaceItems[mDialogueIdx]->SelectAndFireConversation();
			if( !mDialogueRaceItems[mDialogueIdx]->mCurrentConversationIdx )
				mDialogueIdx++;
		}
		else if( mProfIdx < RaceBook::C_RaceBookManager::Instance().NumProfiles() )
		{
			RaceBook::C_RaceBookProfile* prof = RaceBook::C_RaceBookManager::Instance().GetProfileByIndex(mProfIdx);

			prof->mDialogueRaceItems[mProfDialogueIdx]->SelectAndFireConversation();

			if( !prof->mDialogueRaceItems[mProfDialogueIdx]->mCurrentConversationIdx)
				mProfDialogueIdx++;

			if( mProfDialogueIdx >= prof->mDialogueRaceItems.Size() )
			{
				mProfIdx++;
				mProfDialogueIdx = 0;
			}
		}
		else if( mGroupIdx < mDialogueHashGroupMap.Size() )
		{
			Btl::Map<U32,C_InGameDialogueGroup*>::Iterator it = mDialogueHashGroupMap.Begin();

			// Stupid stupid stupid. Why can't I just +=, or add it to the iterator in the line above, or use []? Stupid.
			for( int i = 0; i < mGroupIdx; i++ )
				it++;

			C_InGameDialogueGroup* group = it->GetValue();
			group->mCurrentCharacter = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)group->mMembers[0]);

			switch(mGroupIdx)
			{
			case 0:
				{
					group->mAIDefendsItemsRaceItems.SelectAndFireConversation();

					if( !group->mAIDefendsItemsRaceItems.mCurrentConversationIdx)
						mGroupIdx++;

					break;
				}

			case 1:
				{
					group->mAIHappyItemsRaceItems.SelectAndFireConversation();

					if( !group->mAIHappyItemsRaceItems.mCurrentConversationIdx)
						mGroupIdx++;

					break;
				}

			case 2:
				{
					group->mAIMissesItemsRaceItems.SelectAndFireConversation();

					if( !group->mAIMissesItemsRaceItems.mCurrentConversationIdx)
						mGroupIdx++;

					break;
				}

			case 3:
				{
					group->mAIUnhappyItemsRaceItems.SelectAndFireConversation();

					if( !group->mAIUnhappyItemsRaceItems.mCurrentConversationIdx)
						mGroupIdx++;

					break;
				}

			case 4:
				{
					group->mAIFatallyDamagedRaceItems.SelectAndFireConversation();

					if( !group->mAIFatallyDamagedRaceItems.mCurrentConversationIdx)
						mGroupIdx++;

					break;
				}
			}
		}
		else
		{
			//mEnabled = true;
			mForcePlayAllDialogue = false;
		}
	}

	// ---------------------------------------------------------------------------------------

	bool C_InGameDialogueManager::MessageHandler(const Messaging::C_MessageBase* pMessage)
	{
		bool handledMessage = true;

		switch( pMessage->Type() )
		{

		case GameMessaging::E_GameMsg_Update:
			{
				if(mForcePlayAllDialogue && !mCurrentlyPlayingAudio)
				{
					PlayNextDialogue();
				}

				// We cannot trust audio to always finish properly.
				if(mCurrentlyPlayingAudio && g_debugEnableTimeout)
				{
					if(mTimeout >= 0.0f)
						mTimeout -= C_GameApp::GetInstance()->GetTimeStep();
					else
					{
						LOGWARN(g_InGameDialogueLogger, "Audio failed to play - DialogueManager timeout expired.\n");
						mCurrentlyPlayingAudio = false;
					}
				}

				break;
			}

		case ActorMsg::E_CutSceneMsg_Start321:
			{
				mCurrentlyPlayingAudio = false;
				mCurrentConversation = NULL;
				mTimeout = 0;

				//if(C_GameProfileManager::Instance().GetPrimaryProfile()->mRetryingEvent)
				//	mPlayerObjectiveRetryRaceItems.SelectAndFireConversation();
				//else
				
				mPlayerObjectiveIntroRaceItems.SelectAndFireConversation();

				break;
			}

		case E_GameflowMessage_RaceStarted:
			{
				//mEnabled = true;
				break;
			}

		case E_VehicleMessage_FatallyDamaged:
			{
				const Actors::C_VehicleFatallyDamagedMsg* pMsg	= Messaging::MessageCast<Actors::C_VehicleFatallyDamagedMsg>(pMessage);

				if(!mCurrentlyPlayingAudio && mEnabled && !g_debugDialogueDisable)
					CheckFatallyDamagedDialogue(pMsg->m_VehicleIndex);

				break;
			}

		case E_GameLogicMessage_Passed_All_Rule:
			{
				const C_PassedAllRuleMsg* pMsg = Messaging::MessageCast<C_PassedAllRuleMsg>(pMessage);

				if(!pMsg->mVehicleIndex)
					mPlayerObjectivePassedRaceItems.SelectAndFireConversation();

				mEnabled = false;
				break;
			}

		case E_GameLogicMessage_Failed_Rule:
			{
				const C_FailedRuleMsg* pMsg = Messaging::MessageCast<C_FailedRuleMsg>(pMessage);

				if(!pMsg->mVehicleIndex)
					mPlayerObjectiveFailedRaceItems.SelectAndFireConversation();

				mEnabled = false;
				break;
			}

		case E_GameflowMessage_QuitAndWinRace:
			{
				mPlayerObjectivePassedRaceItems.SelectAndFireConversation();
				// Drop through
			}

		case E_GameflowMessage_RaceFinished:
		case E_GameflowMessage_QuitRace:
		case E_GameflowMessage_QuitLevel:
			{
				mEnabled = false;
				break;
			}
		case E_GameLogicMessage_SplitCrossed:
			{
				const C_SplitCrossedMsg* pMsg = Messaging::MessageCast<C_SplitCrossedMsg>(pMessage);

				if(!mCurrentlyPlayingAudio && mEnabled && !g_debugDialogueDisable)
					CheckObjectiveDialogue(true, pMsg->mVehicleIndex, pMsg->mLap, pMsg->mSplit);

				break;
			}
		case E_GameLogicMessage_LapIncremented:
			{
				const C_LapIncrementedMsg* pMsg = Messaging::MessageCast<C_LapIncrementedMsg>(pMessage);

				if(!mCurrentlyPlayingAudio && mEnabled && !g_debugDialogueDisable)
					CheckObjectiveDialogue(false, pMsg->mVehicleIndex, pMsg->mLap, 0);

				break;
			}

		case E_GameLogicMessage_GameplayImpact:
			{
				float randF = mRandGenerator.FRand(0.f, 1.f);

				if(!mCurrentlyPlayingAudio && mEnabled && !g_debugDialogueDisable && randF <= mCurrentEventProbability)
				{
					const C_GameplayImpactMsg* pMsg = Messaging::MessageCast<C_GameplayImpactMsg>(pMessage);
					CheckImpactDialogue(pMsg);
				}

				break;
			}

		// Currently unused ...
		case E_VehicleLogicMsg_HitByPerkSpecialEventFlipped:
			{
				//if(!mCurrentlyPlayingAudio && mEnabled && !g_debugDialogueDisable)
				//{
				//	const C_VehicleLogicMsg_HitByPerkSpecialEventFlippedMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_HitByPerkSpecialEventFlippedMsg>(pMessage);
				//	CheckPerkDialogue(pMsg->mFiringVehicleIndex, pMsg->mHitVehicleIndex, true);
				//}

				break;
			}

		// Currently unused ...
		case E_VehicleLogicMsg_HitByPerkSpecialEventCollided:
			{
				//if(!mCurrentlyPlayingAudio && mEnabled && !g_debugDialogueDisable)
				//{
				//	const C_VehicleLogicMsg_HitByPerkSpecialEventCollidedMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_HitByPerkSpecialEventCollidedMsg>(pMessage);
				//	CheckPerkDialogue(pMsg->mFiringVehicleIndex, pMsg->mHitVehicleIndex, true);
				//}

				break;
			}

		case E_VehicleLogicMsg_HitByPerk:
			{
				float randF = mRandGenerator.FRand(0.f, 1.f);

				if(!mCurrentlyPlayingAudio && mEnabled && !g_debugDialogueDisable && randF <= mCurrentEventProbability)
				{
					const C_VehicleLogicMsg_HitByPerkMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_HitByPerkMsg>(pMessage);
					CheckPerkDialogue(pMsg->mFiringVehicleIndex, pMsg->mVehicleIndex, pMsg->mLevel > 0);
				}

				break;
			}

		case E_VehicleLogicMsg_ShieldedPerk:
			{
				float randF = mRandGenerator.FRand(0.f, 1.f);

				if(!mCurrentlyPlayingAudio && mEnabled && !g_debugDialogueDisable && randF <= mCurrentEventProbability)
				{
					const C_VehicleLogicMsg_ShieldedPerkMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_ShieldedPerkMsg>(pMessage);
					CheckShieldedPerkDialogue(pMsg->mFiringVehicleIndex, pMsg->mDefendingVehicleIndex);
				}

				break;
			}

		case E_VehicleMessage_Overtake:
			{
				PERF_EVENT("C_InGameDialogueManager.Overtake");

				if(!C_GameData::Instance().mDialogueTriggerControls.Number())
				{
					LOGWARN(g_InGameDialogueLogger, "Bad dialogue trigger controls - cannot resolve overtake dialogue.");
					break;
				}

				float randF = mRandGenerator.FRand(0.f, 1.f);

				if(!mCurrentlyPlayingAudio && mEnabled && !g_debugDialogueDisable && randF <= mCurrentEventProbability)
				{
					const Actors::C_VehicleOvertakeMsg* pMsg	= Messaging::MessageCast<Actors::C_VehicleOvertakeMsg>(pMessage);
					CheckOvertakeDialogue(pMsg->mOvertakerVehicleIndex, pMsg->mOvertakeeVehicleIndex, false);
				}
				break;
			}

		case E_VehicleMessage_OvertakeWithNitro:
			{
				float randF = mRandGenerator.FRand(0.f, 1.f);

				if(!mCurrentlyPlayingAudio && mEnabled && !g_debugDialogueDisable && randF <= mCurrentEventProbability)
				{
					const Actors::C_VehicleOvertakeWithNitroMsg* pMsg	= Messaging::MessageCast<Actors::C_VehicleOvertakeWithNitroMsg>(pMessage);
					CheckOvertakeDialogue(pMsg->mOvertakerVehicleIndex, pMsg->mOvertakeeVehicleIndex, true);
				}
				break;
			}

		case E_VehicleMessage_ChangedHealthLevel:
			{
				if(!mCurrentlyPlayingAudio && mEnabled && !g_debugDialogueDisable)
				{
					const Actors::C_VehicleHealthLevelChangedMsg* pMsg	= Messaging::MessageCast<Actors::C_VehicleHealthLevelChangedMsg>(pMessage);
					CheckLowHealthDialogue(pMsg->mVehicleIndex, pMsg->mHealthlevel);
				}

				break;
			}

		case E_AIMessage_AttemptedSideSwipe:
			{
				float randF = mRandGenerator.FRand(0.f, 1.f);

				if(!mCurrentlyPlayingAudio && mEnabled && !g_debugDialogueDisable && randF <= mCurrentEventProbability)
				{
					const AIAmax::AISideSwipeMessage * pMsg	= Messaging::MessageCast<AIAmax::AISideSwipeMessage >(pMessage);
					CheckSideSwipeDialogue(pMsg->m_aiDoing, pMsg->m_aiAgainst);
				}
				break;
			}

		case E_VehicleLogicMsg_MissedPerk:
			{
				float randF = mRandGenerator.FRand(0.f, 1.f);

				if(!mCurrentlyPlayingAudio && mEnabled && !g_debugDialogueDisable && randF <= mCurrentEventProbability)
				{
					const C_VehicleLogicMsg_MissedPerkMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_MissedPerkMsg>(pMessage);
					CheckMissedPerkDialogue(pMsg->mFiringVehicleIndex, pMsg->mDefendingVehicleIndex);
				}
				break;
			}

		case ActorMsg::E_AudioMsg_DialogueFinish:
			{
				mCurrentlyPlayingAudio = false;

				float prob = C_GameData::Instance().mDialogueTriggerControls[0]->nextLineProbability;
				float randNum = mRandGenerator.FRand(0.f, 1.f);

				if(mCurrentConversation && mCurrentDialogueIdx < mCurrentConversation->mDlgItemCount && randNum <= prob)
					mCurrentConversation->FireConversation();

				break;
			}
			

		default :
			handledMessage = false;
			break;
		}

		return handledMessage;
	}

}; // namespace Game
