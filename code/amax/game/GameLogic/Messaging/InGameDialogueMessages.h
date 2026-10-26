#ifndef INGAMEDIALOGUE_MESSAGES_H
#define INGAMEDIALOGUE_MESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game/Messaging/GameMessages.h>
#include <GameLogic\GameData\AmaxDialogue.h>
#include <misc_shared/Messaging/GameMessaging.h>

#include <misc_shared/Messaging/MessagingDeclare.h>
#include <misc_shared/Messaging/MessageBase.h>

namespace Game
{
	class InGameDialogueEvent : public Messaging::C_MessageDerived<InGameDialogueEvent, E_AMAXDIALOGUE_PLAYAUDIO>
	{
	public:
		InGameDialogueEvent(U32 idx, U32 split) : mSplitIndex(idx), mSplitGroup(split) {};

		U32 mSplitIndex;
		U32 mSplitGroup;
	};

	class InGameDialogueAddWavebank : public Messaging::C_MessageDerived<InGameDialogueAddWavebank, E_AMAXDIALOGUE_ADDWAVEBANK>
	{
	public:
		InGameDialogueAddWavebank() {};

		Btl::Set<U32> mSplitGroupList;
	};

	

} // namespace Game

#endif // INGAMEDIALOGUE_MESSAGES_H