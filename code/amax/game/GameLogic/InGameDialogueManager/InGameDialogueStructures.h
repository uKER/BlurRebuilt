#ifndef INGAME_DIALOGUE_STRUCTURES_H
#define INGAME_DIALOGUE_STRUCTURES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>
#include <Xt/XtGame.h>

namespace Sys
{
	class BinarySerialiserAutoEndian;
	class BinaryDeserialiserAutoEndian;
}

namespace Game
{
	class C_InGameDialogueConversation
	{
	public:
		C_InGameDialogueConversation() : mDlgItemFirstIndex(0), mDlgItemCount(0), mSortKey(0), mTimesFired(0.0f), mId(0) { }

		void Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

		void FireConversation();

		U32								mDlgItemFirstIndex;
		U16								mDlgItemCount;
		U16								mSortKey;
		float							mTimesFired;
		XtId							mId;			// The sorcery item id
	};

	// ---------------------------------------------------------------------

	typedef Btl::Vector<C_InGameDialogueConversation*> CharacterConversationVec;

	struct CharacterConversationTracker
	{
		CharacterConversationTracker() : mCurrentConversationIdx(0) {};
		void SelectAndFireConversation();

		void ClearData()
		{
			mCurrentConversationIdx = 0;
			mConversationList.Clear();
		}

		void Shuffle();

		int							mCurrentConversationIdx;
		CharacterConversationVec	mConversationList;
	};
}

#endif //INGAME_DIALOGUE_STRUCTURES_H