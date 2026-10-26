// Application schema for dialogue manager records
#ifndef XT_DIALOGUE_H
#define XT_DIALOGUE_H

#include "xt/XtGame.h"
#include "StringHash.h"

namespace Dialogue
{
// Dialogue Item Base Class
class C_XtDialogueItemBase
{
public:


public:
	XtLocId								m_XtId;	// LocID of dialogue entry
	XtSz8								m_Character;

	// non Sorcery Values
	S32									m_AudioCharacterIndex;
	Audio::StringHash					m_hashEntityExternalAction;
	Audio::StringHash					m_hashEntityExternalParam;

	// for debugging
	#ifndef CFG_SPEED
	Btl::String							m_strEntityExternalAction;
	Btl::String							m_strEntityExternalParam;
	#endif
};

// Cutscene dialogue item
class C_XtCutsceneDialogueItem : public C_XtDialogueItemBase
{
public:


public:

};

// Triggered dialogue item
class C_XtTriggeredDialogueItem : public C_XtDialogueItemBase
{
public:


public:

};

// Dialogue Record
class C_XtDialogue
{
public:
	const C_XtDialogueItemBase *GetDialogueItemFromLocID(U32 locID) const;
	void		GenerateSoundTableIndices(void);
	void		GenerateAudioEntityExternalNames(void);
public:
	XtArray<C_XtDialogueItemBase *>	m_Items;
};

// Global functions
const C_XtDialogueItemBase *GetDialogueItemFromLocID(const U32 locID);

}

#endif