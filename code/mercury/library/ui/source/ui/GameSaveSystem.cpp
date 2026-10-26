#include "stdafx.h"

#include "ui/GameSaveSystem.h"
#include "ui/Game/UIScreenControl.h"

#include "ui/Game/Gadgets/ButtonController.h"
#include "ui/Game/Gadgets/ButtonMessages.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

namespace UI
{

C_GameSaveSystem gGameSaveSystem;

void C_GameSaveSystem::Update()
{
	if ( mCheckforDeviceLocation )
	{
		_UI_MESSAGE( E_UiMsg_SaveLocationNotFound );
		mCheckforDeviceLocation = false;
	}

	if ( mEnumeratingSaves )
	{
		_UI_MESSAGE( E_UiMsg_SaveEnumerationFinished );
		mEnumeratingSaves = false;
	}
}

void C_GameSaveSystem::StartXenonLocationUI( const S32 sDataSize, const S32 sDeviceSelectorFlag )
{
}

S32 C_GameSaveSystem::CreateSave(const char *filename, const WCHAR *display_name, const bool roamable, const bool check_overwrites)
{
	mOpen = true;

	return GAMESAVE_SYSTEM_OK;
}

bool C_GameSaveSystem::LoadSave(const char *filename, const WCHAR *display_name)
{
	//must be ERROR_SUCCESS then (a new save)
	mOpen = true;
	return true;
}

bool C_GameSaveSystem::DeleteSave(const char *filename, const WCHAR *display_name)
{
	return true;
}

bool C_GameSaveSystem::CloseSave()
{

	mOpen = false;
	return true;
}


bool C_GameSaveSystem::Enumerate( const bool bAsync )
{
	return false;
}

bool C_GameSaveSystem::Flush()
{
	return true;
}

ULONGLONG C_GameSaveSystem::SpaceLeftOnCurrentDevice(int *error)
{
	ULONGLONG bytes=0;
	return bytes;
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
