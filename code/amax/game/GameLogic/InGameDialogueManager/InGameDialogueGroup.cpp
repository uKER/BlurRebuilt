#include "stdafx.h"

#include <GameLogic\InGameDialogueManager\InGameDialogueGroup.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>

namespace Game
{

	void C_InGameDialogueGroup::ClearRaceItems()
	{
		mAIHappyItemsRaceItems.ClearData();
		mAIDefendsItemsRaceItems.ClearData();
		mAIUnhappyItemsRaceItems.ClearData();
		mAIMissesItemsRaceItems.ClearData();
		mAIFatallyDamagedRaceItems.ClearData();
	}

	void C_InGameDialogueGroup::ClearAllItems()
	{
		for( int i = 0; i < mAIMissesItems.Size(); i++ )
			C_InGameDialogueManager::Instance().ClearDataItems(&mAIMissesItems);

		for( int i = 0; i < mAIHappyItems.Size(); i++ )
			C_InGameDialogueManager::Instance().ClearDataItems(&mAIHappyItems);

		for( int i = 0; i < mAIUnhappyItems.Size(); i++ )
			C_InGameDialogueManager::Instance().ClearDataItems(&mAIUnhappyItems);

		for( int i = 0; i < mAIDefendsItems.Size(); i++ )
			C_InGameDialogueManager::Instance().ClearDataItems(&mAIDefendsItems);

		for( int i = 0; i < mAIFatallyDamagedItems.Size(); i++ )
			C_InGameDialogueManager::Instance().ClearDataItems(&mAIFatallyDamagedItems);
	}

	void C_InGameDialogueGroup::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{
		Sys::BinarySerialiserAutoEndian out( stream );

		stream << mAIDefendsItems.Size();
		for(int i = 0; i < mAIDefendsItems.Size(); i++)
			stream << *mAIDefendsItems[i];

		stream << mAIHappyItems.Size();
		for(int i = 0; i < mAIHappyItems.Size(); i++)
			stream << *mAIHappyItems[i];

		stream << mAIMissesItems.Size();
		for(int i = 0; i < mAIMissesItems.Size(); i++)
			stream << *mAIMissesItems[i];

		stream << mAIUnhappyItems.Size();
		for(int i = 0; i < mAIUnhappyItems.Size(); i++)
			stream << *mAIUnhappyItems[i];

		stream << mAIFatallyDamagedItems.Size();
		for(int i = 0; i < mAIFatallyDamagedItems.Size(); i++)
			stream << *mAIFatallyDamagedItems[i];

		stream << mMembers.Size();
		for(int i = 0; i < mMembers.Size(); i++)
			stream << mMembers[i];
	}

	void C_InGameDialogueGroup::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		Sys::BinaryDeserialiserAutoEndian in( stream ); 

		int size;

		stream >> size;
		mAIDefendsItems.Reserve(size);
		for( int i = 0; i < size; i++)
		{
			C_InGameDialogueConversation* conv = new C_InGameDialogueConversation();
			stream >> *conv;
			mAIDefendsItems.PushBack(conv);
		}

		stream >> size;
		mAIHappyItems.Reserve(size);
		for( int i = 0; i < size; i++)
		{
			C_InGameDialogueConversation* conv = new C_InGameDialogueConversation();
			stream >> *conv;
			mAIHappyItems.PushBack(conv);
		}

		stream >> size;
		mAIMissesItems.Reserve(size);
		for( int i = 0; i < size; i++)
		{
			C_InGameDialogueConversation* conv = new C_InGameDialogueConversation();
			stream >> *conv;
			mAIMissesItems.PushBack(conv);
		}

		stream >> size;
		mAIUnhappyItems.Reserve(size);
		for( int i = 0; i < size; i++)
		{
			C_InGameDialogueConversation* conv = new C_InGameDialogueConversation();
			stream >> *conv;
			mAIUnhappyItems.PushBack(conv);
		}

		stream >> size;
		mAIFatallyDamagedItems.Reserve(size);
		for( int i = 0; i < size; i++)
		{
			C_InGameDialogueConversation* conv = new C_InGameDialogueConversation();
			stream >> *conv;
			mAIFatallyDamagedItems.PushBack(conv);
		}

		stream >> size;
		mMembers.Reserve(size);
		for( int i = 0; i < size; i++ )
		{
			XtId id;
			stream >> id;
			mMembers.PushBack(id);
		}
	}

}