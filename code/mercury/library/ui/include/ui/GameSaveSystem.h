#ifndef GAMESAVESYSTEM_H
#define GAMESAVESYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/UIDefines.h"
#include "ui/Game/SaveSystem/SaveSystem.h"

namespace UI
{

enum GAMESAVE_SYSTEM
{
	GAMESAVE_SYSTEM_OK,
	GAMESAVE_SYSTEM_ERROR,
	GAMESAVE_SYSTEM_FILE_EXISTS,
	GAMESAVE_SYSTEM_DEVICE_DISCONNECTED,
};

class C_GameSaveSystem
{
public:
	C_GameSaveSystem()
	{
		mUserIndex = -1;
		mCheckforDeviceLocation = false;
		mEnumeratingSaves = false;
		strcpy_s( mSaveRoot, lengthof( mSaveRoot ), "SAVE" );
		mOpen = false;
	};

	void Update();

	bool Enumerate( const bool bAsync = true );
	bool Flush();

	void StartXenonLocationUI( const S32 sDataSize = -1, const S32 sDeviceSelectorFlag = -1 );

	S32 CreateSave(const char *filename, const WCHAR *display_name, const bool roamable=false, const bool check_overwrites=false);

	bool LoadSave(const char *filename, const WCHAR *display_name);
	bool DeleteSave(const char *filename, const WCHAR *display_name);

	bool CloseSave();

	void SetUserIndex(int i){mUserIndex=i;};
	void SetSaveRoot(const char *r){STRCPY(mSaveRoot,lengthof(mSaveRoot),r);};

	int GetLastError(){return mLastError;};

	bool EnumeratingSaves(){return mEnumeratingSaves;};

	ULONGLONG SpaceLeftOnCurrentDevice(int *error=NULL);


private:

	int					mLastError;
	char				mSaveRoot[12];
	char				mUserIndex;
	bool				mOpen;
	bool				mCheckforDeviceLocation;
	bool				mEnumeratingSaves;

};

extern C_GameSaveSystem gGameSaveSystem;

};		// namespace UI
#endif	// GAMESAVESYSTEM_H
