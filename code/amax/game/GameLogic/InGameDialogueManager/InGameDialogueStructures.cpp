#include "stdafx.h"
#include <Game/Game.h>	// [Blur2PC]

#include <GameLogic\InGameDialogueManager\InGameDialogueStructures.h>
#include <GameLogic\InGameDialogueManager\InGameDialogueManager.h>
#include <Racebook\RaceBookManager.h>
#include <Ui\UiMessages.h>
#include <Automation\GameDataLogging.h>

#include <algorithm>

extern Debug::Float g_debugDialogueFinishTimeout;
extern Debug::Logger &g_InGameDialogueLogger;

namespace Game
{
	class C_InGameDialogueManager;

	void CharacterConversationTracker::SelectAndFireConversation()
	{
		C_InGameDialogueManager::Instance().mCurrentDialogueIdx = 0;

		if(mCurrentConversationIdx >= mConversationList.Size())
			return;

		AmaxInGameDialogue* sorceryDialogue = C_GameData::Instance().mInGameDialogue[(GAMEDATA_ID)mConversationList[mCurrentConversationIdx]->mId];

		if(!sorceryDialogue || !sorceryDialogue->items[0])
			return;

		if( mCurrentConversationIdx < mConversationList.Size() )
		{
			mConversationList[mCurrentConversationIdx]->FireConversation();
			C_InGameDialogueManager::Instance().mCurrentConversation = mConversationList[mCurrentConversationIdx];

			if(sorceryDialogue->items[0]->oneShot)
			{
				SAFE_DELETE_PTR(mConversationList[mCurrentConversationIdx]);
				mConversationList.Swap<C_InGameDialogueConversation*>(mConversationList.Begin() + mCurrentConversationIdx, mConversationList.End() - 1);
				mConversationList.PopBack();
			}
		}		

		if( ++mCurrentConversationIdx >= mConversationList.Size() )
			mCurrentConversationIdx = 0;
	}

	struct InGameDialogueConversationSort
	{
	public:
		bool operator()(C_InGameDialogueConversation* lhs, C_InGameDialogueConversation* rhs)
		{
			if (fabsf(lhs->mTimesFired - rhs->mTimesFired) < 0.01f)
				return lhs->mSortKey < rhs->mSortKey;

			return lhs->mTimesFired < rhs->mTimesFired;
		}
	};

	void CharacterConversationTracker::Shuffle()
	{
		if (mConversationList.Size() <= 0)
			return;

		// We sort the list into order of times fired, and then randomise each sub-group.

		for( int i = 0; i < mConversationList.Size(); i++ )
			mConversationList[i]->mSortKey = rand();

		std::sort(mConversationList.Begin(), mConversationList.End(), InGameDialogueConversationSort());
	}

	// --------------------------------------------------------------------------

	void C_InGameDialogueConversation::FireConversation()
	{
		int currIdx = C_InGameDialogueManager::Instance().mCurrentDialogueIdx++;

		AmaxInGameDialogue* sorceryDialogue = C_GameData::Instance().mInGameDialogue[(GAMEDATA_ID)mId];

		if(!sorceryDialogue || !sorceryDialogue->items[mDlgItemFirstIndex + currIdx])
		{
			LOGERROR(g_InGameDialogueLogger, "!sorceryDialogue || !sorceryDialogue->items[mDlgItemFirstIndex(%d) + currIdx(%d)]", mDlgItemFirstIndex, currIdx);
			return;
		}

		AmaxInGameDialogueItem* sorceryItem = (AmaxInGameDialogueItem*)sorceryDialogue->items[mDlgItemFirstIndex + currIdx];
		RaceBook::C_RaceBookProfile* prof = NULL;

		if(!sorceryItem || !sorceryItem->weighting || mDlgItemFirstIndex >= (U32)sorceryDialogue->items.count)
		{
			LOGERROR(g_InGameDialogueLogger, "!sorceryItem || !sorceryItem->weighting || mDlgItemFirstIndex > sorceryDialogue->items.count");
			return;
		}

		mTimesFired += sorceryItem->weighting;

		RBCharacterHashMap::Iterator it = C_InGameDialogueManager::Instance().mHashCharacterMap.Find(sorceryItem->characterHash);

		if( it != C_InGameDialogueManager::Instance().mHashCharacterMap.End() )
		{
			prof = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)it->GetValue());
		}
		else
		{
			Btl::Map<U32, C_InGameDialogueGroup*>::Iterator groupIt = C_InGameDialogueManager::Instance().mDialogueHashGroupMap.Find(sorceryItem->characterHash);

			if(groupIt != C_InGameDialogueManager::Instance().mDialogueHashGroupMap.End())
			{
				C_InGameDialogueGroup* group = groupIt->GetValue();
				prof = group->mCurrentCharacter;
			}
		}

		if(prof)
		{
			// Last minute catch for repeated dialogue (except intro)
			if(sorceryItem->locId.id == C_InGameDialogueManager::Instance().mLastLocId && sorceryItem->triggerHash != C_InGameDialogueManager::Instance().mHash_PlayerObjectiveIntro)
			{
				LOGERROR(g_InGameDialogueLogger, "sorceryItem->locId.id == C_InGameDialogueManager::Instance().mLastLocId");
				return;
			}

			C_InGameDialogueManager::Instance().mLastLocId = sorceryItem->locId.id;

			// Tell Audio
			InGameDialogueEvent dialogueMsg(sorceryItem->splitIndex, sorceryItem->splitGroup);
			C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&dialogueMsg);

			// Tell UI
			UI::C_UiGameMsg_Dialogue uiDialogueMsg( (GAMEDATA_ID)prof->GetID(), UI::C_TextStringID(sorceryItem->locId.id) );
			_UI_MESSAGE_FULL( uiDialogueMsg );			

			C_InGameDialogueManager::Instance().mCurrentlyPlayingAudio = true;
			C_InGameDialogueManager::Instance().mTimeout = g_debugDialogueFinishTimeout;

#ifndef CFG_GOLD
			LOGDEBUG(g_InGameDialogueLogger, "%ls : %ls : SplitIdx %u : SplitGroup %u : %s : %s : %s : %s : Play weighting %f\n", 
				UI::C_TextStringID(prof->GetName().id).String(), UI::C_TextStringID(sorceryItem->locId.id).String(),
				sorceryItem->splitIndex, sorceryItem->splitGroup, sorceryItem->triggerName.Text(), sorceryItem->wavFile.Text(),
				sorceryItem->locationName.Text(), sorceryItem->eventName.Text(), mTimesFired);

			UI::C_WideString16 speaker(UI::C_TextStringID(prof->GetName().id).String());
			UI::C_WideString512 dialogueLine(UI::C_TextStringID(sorceryItem->locId.id).String());

			Util::C_String32 triggerName;
			C_InGameDialogueManager::Instance().GetTriggerNameFromHash(triggerName, sorceryDialogue->items[currIdx]->triggerHash);

			AmaxDataLogging::LogFireDialogue(speaker.AsString().Chars(), triggerName.Chars(), dialogueLine.AsString().Chars());
#endif
		}
	}

	void C_InGameDialogueConversation::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{ 
		stream << mDlgItemFirstIndex << mDlgItemCount << mId << mTimesFired;
	}

	void C_InGameDialogueConversation::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		stream >> mDlgItemFirstIndex >> mDlgItemCount >> mId >> mTimesFired;
	}

}