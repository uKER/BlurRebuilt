#include "Stdafx.h"

#include "ProfileInterface.h"
#include "SaveSystem.h"

#include <ui/GameSaveSystem.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace UI
{

// -----------------
// C_AsyncLoadThread
// -----------------
void C_AsyncLoadThread::ThreadMain()
{
	// TODO - the actual loading
}

// -----------------
// C_AsyncSaveThread
// -----------------
void C_AsyncSaveThread::ThreadMain()
{
	// TODO - the actual saving
}

// ------------
// C_SaveSystem
// ------------
bool C_SaveSystem::DeleteSave( const S32 sPadNo )
{
	// TODO - the actual deleting
	return false;
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
