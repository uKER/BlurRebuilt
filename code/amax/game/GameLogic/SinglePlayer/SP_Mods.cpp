#include "StdAfx.h"
#include "SinglePlayerImpl.h"

bool SP_IsModUnlocked(SP_SaveGame* sg, XtId modId)
{
	if (!sg)
		return false;

	SP_SaveMod* sm = SP_GetSaveMod(sg, modId);

	if (!sm)
		return false;

	return sm->IsUnlocked();
}

bool SP_IsModNew(SP_SaveGame* sg, XtId modId)
{
	if (!sg)
		return false;

	SP_SaveMod* sm = SP_GetSaveMod(sg, modId);

	if (!sm)
		return false;

	return sm->IsNew();
}