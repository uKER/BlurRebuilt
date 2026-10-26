#include "SharedActorsPch.h"

#include <btl/containers/algorithm.h>
#include <btl/containers/list.h>
#include <shared_actors/XtDialogue.h>

#include "btl/utility/string.h"
#include "btl/containers/map.h"
#include <btl/containers/multimap.h>

#include "StringHash.h"	// Audio::StringHash

namespace Dialogue
{

typedef Btl::List<C_XtDialogue *> C_DialogueList;

C_DialogueList g_DialogueList;

const C_XtDialogueItemBase *GetDialogueItemFromLocID(const U32 locID)
{
	C_DialogueList::ConstIterator it = g_DialogueList.Begin();
	while(it != g_DialogueList.End())
	{
		const C_XtDialogueItemBase *item = (*it)->GetDialogueItemFromLocID(locID);

		if(item)
			return item;

		++it;
	}
	return NULL;
}

const C_XtDialogueItemBase *C_XtDialogue::GetDialogueItemFromLocID(U32 locID) const
{
	for(S32 itemNo = 0; itemNo < m_Items.count; itemNo++)
	{
		if(m_Items[itemNo] && (m_Items[itemNo]->m_XtId.id == locID))
			return m_Items[itemNo];
	}
	return NULL;
}

// generate soundtable indices for each character.
// this is so when we get a trigger for a dialogue line for a character we know 
// the index into that character's soundtable.
void C_XtDialogue::GenerateSoundTableIndices(void)
{
	// create hashmap
	Btl::MultiMap<Btl::String, U32> characterIndexMap;

	for(S32 itemNo = 0; itemNo < m_Items.count; itemNo++)
	{
		C_XtDialogueItemBase *pItem = m_Items[itemNo];

		if (pItem)
		{
			Btl::String characterString(pItem->m_Character);

			// check if character is in hashmap
			Btl::MultiMap<Btl::String, U32>::Iterator it = characterIndexMap.Find(characterString);
			
			// if not initialise entry to 0
			if(it == characterIndexMap.End())
				it = characterIndexMap.Insert(characterString, 0);
			
			// use current entry
			pItem->m_AudioCharacterIndex = it->GetValue();

			// increment entry
			it->GetValue()++;
		}
	}
}

void C_XtDialogue::GenerateAudioEntityExternalNames(void)
{
	for(S32 itemNo = 0; itemNo < m_Items.count; itemNo++)
	{
		C_XtDialogueItemBase *pItem = m_Items[itemNo];
		
		if (pItem)
		{
			// construct audio entity external names
			Btl::String strExternal;

			// create stringhashes of the names 
			strExternal.Printf("PlayDialogue_%s", pItem->m_Character.Text());	
			pItem->m_hashEntityExternalAction = Audio::StringHash(strExternal);

			#ifndef CFG_SPEED
			pItem->m_strEntityExternalAction = strExternal;
			#endif

			strExternal.Printf("Index_%s", pItem->m_Character.Text());
			pItem->m_hashEntityExternalParam = Audio::StringHash(strExternal);

			#ifndef CFG_SPEED
			pItem->m_strEntityExternalParam = strExternal;
			#endif
		}
	}
}


void DialogueXtCallback(XtEventType type, XtRecord* record, C_XtDialogue* dialogue, XtCallbackContext* context)
{
	switch(type)
	{
	case kXtEtConstruct:
		// Add to Dialogue record list
		g_DialogueList.PushBack(dialogue);
		dialogue->GenerateSoundTableIndices();
		dialogue->GenerateAudioEntityExternalNames();
		break;

	case kXtEtDestruct:
		C_DialogueList::Iterator it = Btl::Find(g_DialogueList, dialogue);

		// Remove from Dialogue record list
		if(it!=g_DialogueList.End())
			g_DialogueList.Erase(it);
		break;
	}
}

}

// XT Annotation

// Item Base
XT_TYPE(Dialogue::C_XtDialogueItemBase, "DialogueItemBase")
XT_FIELD(m_XtId, "TextId")
XT_FIELD(m_Character, "Character")
XT_END_TYPE

// Cutscene Dialogue Item
XT_TYPE(Dialogue::C_XtCutsceneDialogueItem, "CutSceneDialogueItem")
XT_BASE(Dialogue::C_XtDialogueItemBase)
XT_END_TYPE

// Triggered Dialogue Item
XT_TYPE(Dialogue::C_XtTriggeredDialogueItem, "TriggeredDialogueItem")
XT_BASE(Dialogue::C_XtDialogueItemBase)
XT_END_TYPE

// Dialogue Record Type
XT_TYPE(Dialogue::C_XtDialogue, "Dialogue")
XT_CALLBACK(Dialogue::DialogueXtCallback)
XT_FIELD(m_Items, "Items")
XT_END_TYPE

XT_LIBRARY(XtDialogue)
XT_REGISTER_TYPE(Dialogue::C_XtDialogueItemBase)
XT_REGISTER_TYPE(Dialogue::C_XtCutsceneDialogueItem)
XT_REGISTER_TYPE(Dialogue::C_XtTriggeredDialogueItem)
XT_REGISTER_TYPE(Dialogue::C_XtDialogue)
XT_END_LIBRARY