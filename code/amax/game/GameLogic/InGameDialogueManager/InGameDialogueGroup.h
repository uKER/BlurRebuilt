#ifndef INGAME_DIALOGUE_GROUP_H
#define INGAME_DIALOGUE_GROUP_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>
#include <GameLogic\InGameDialogueManager\InGameDialogueStructures.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>

namespace RaceBook { class C_RaceBookProfile; };

namespace Game
{
	class C_InGameDialogueGroup
	{
	public:

		void Serialise( Sys::BinarySerialiserAutoEndian& stream ) ;
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

		void ClearRaceItems();
		void ClearAllItems();

		CharacterConversationVec		mAIUnhappyItems;	// PlayerCollidesWithAI, PlayerHitsAIWithPerk
		CharacterConversationVec		mAIHappyItems;		// AICollidesWithPlayer, AIHitsPlayerWithPerk
		CharacterConversationVec		mAIMissesItems;		// AIMissesPlayerWithPerk, PlayerShieldsPerk
		CharacterConversationVec		mAIDefendsItems;	// PlayerMissesAIWithPerk, AIShieldsPerk
		CharacterConversationVec		mAIFatallyDamagedItems;

		CharacterConversationTracker	mAIUnhappyItemsRaceItems;
		CharacterConversationTracker	mAIHappyItemsRaceItems;
		CharacterConversationTracker	mAIMissesItemsRaceItems;
		CharacterConversationTracker	mAIDefendsItemsRaceItems;
		CharacterConversationTracker	mAIFatallyDamagedRaceItems;

		Btl::Vector<XtId>				mMembers;
		RaceBook::C_RaceBookProfile*	mCurrentCharacter;
	};

} // namespace Game

#endif // INGAME_DIALOGUE_GROUP_H