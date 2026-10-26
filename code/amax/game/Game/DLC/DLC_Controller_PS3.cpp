#include "stdafx.h"

#include <Game\dlc\DLC_Controller.h>

#include <GameApp.h>
#include <GameLogic/GameData/GameData.h>

#include <GameLogic/GameData/GameVehicleData.h>

#include "GameLogic/GameData/MultiplayerData.h"

#include <GameLogic/Profile/GameProfile.h>
#include <GameLogic/Profile/GameProfileManager.h>

#include <cell/sysmodule.h>
#include <cell/cell_fs.h>
#include <sys/fs.h>
#include <sys/paths.h>
#include <sys/process.h>
#include <sys/ppu_thread.h>
#include <sys/timer.h>

#include <sysutil/sysutil_gamecontent.h>

#include <np.h>
#include <np/drm.h>

#define LICENSE_NAME		"-LICENSE"
#define DLC_INFO_FILE		"ToC.edat"
#define NUM_DLC_PACKS		32 

Debug::Logger &g_DLCLogger( Debug::GetLogger("DLC.Controller") );
Debug::Bool g_DebugLicenseAllDLC( "DLC.LicenseAllDLC", false, Debug::Persist() );
Debug::Int g_DebugNumItemsToEnumerate( "DLC.NumItemsToEnumerate", 10, Debug::Persist() );

namespace SonyNetworkPlatform
{
	// defined in PlatformCallbacks_ps3.cpp
	extern const char* get_amax_network_platform_service_id_prefix();

}

namespace PlatformPS3
{
	extern char g_ps3_titleId[];
}

namespace UI
{
	DLCController DLCController::s_instance;

	void DLCController::ThreadMain(void)
	{
		ClearDLC();
		Enumerate();
		LoadAndLicenseContent();

		// Re-cache the MP data now we have the DLC loaded, as it may have changed.
		C_MultiplayerData::Instance().PreCacheData();

		Game::C_GameProfile* pProf = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
		if(pProf)
			pProf->CacheDLCData();

#ifdef COMPILER_VC
		Close();
#endif
	}

	// ------------------------------------------------------------------------------------------------------------------------------------

	void DLCController::ClearDLC()
	{
		C_GameData::Instance().mDLCPacks.DestroyAll();
		mFilesToLoad.Destroy();

		for( int i = 0; i < mPacksOpen.Size(); i++ )
			XtPackClose(C_GameApp::GetInstance()->GetGlobalXtDatabase(), mPacksOpen[i]);

		// Cheat: I want all the DLC in the world ever. ALL OF IT.
		mPacksOpen.Clear();
		mDLCPacksInstalled.Clear();

		mCorruptFiles.Clear();
		mCorruptLocIds.Clear();
	}

	// ------------------------------------------------------------------------------------------------------------------------------------

	void DLCController::LoadAndLicenseContent()
	{
		int ret, fileHandle;

		for( int i = mFilesToLoad.Size() - 1; i >= 0 ; i-- )
		{
			// Open the file
			ret = cellFsOpen(mFilesToLoad[i].Chars(), CELL_FS_O_RDONLY, &fileHandle, NULL, 0);
			if (ret != CELL_FS_OK)
			{
				LOGWARN(g_DLCLogger, "Failed to open file handle for DLC information file.");
				continue;
			}

			// Get information on the file
			CellFsStat stat;
			ret = cellFsFstat(fileHandle, &stat); /* get net size (use cellFsFstat) */
			if (ret != CELL_FS_OK)
			{
				LOGWARN(g_DLCLogger, "Failed to get statistics on DLC information file.");
				cellFsClose(fileHandle);
				continue;
			}

			// Load the file into a buffer
			uint64_t file_size = stat.st_size;
			uint64_t rsize;
			char* readBuffer = new char[file_size];

			ret = cellFsRead(fileHandle, readBuffer, file_size, &rsize);

			if (ret != CELL_OK || rsize != file_size)
			{
				LOGWARN(g_DLCLogger, "Failed to read DLC information file.");
				cellFsClose(fileHandle);
				SAFE_DELETE_ARY(readBuffer);
				continue;
			}

			// Open this XT pack and process it
			XtPack* xtPack = XtPackOpen(C_GameApp::GetInstance()->GetGlobalXtDatabase(), readBuffer, file_size);

			if(xtPack)
			{
				bool licensedSomeFiles = false;
				mPacksOpen.PushBack(xtPack);

				for( U32 k = 0; k < xtPack->count; k++ )
				{
					if(xtPack->records[k].record->strongRef)
					{
						// Hardcoded the type, because there's no other way of getting it. Nice. 
						// We recursively search for the base class.
						for( U32 j = 0; j < xtPack->records[k].record->type->baseCount; j++ )
						{
							if(IsGameDataObject(xtPack->records[k].record->type->bases[j]))
							{
								XtHandle<AmaxGameDataObj> objHandle(xtPack->records[k].record);
								AmaxGameDataObj* objTest = (AmaxGameDataObj*)objHandle.Object();

								Util::C_String licenseStr;
								licenseStr.Printf("%s%s%s%09i", SonyNetworkPlatform::get_amax_network_platform_service_id_prefix(), PlatformPS3::g_ps3_titleId, LICENSE_NAME, objHandle->dlcLicenseBit);

								ret = sceNpDrmVerifyUpgradeLicense2(licenseStr.Chars());

								if(!objHandle->dlcPack || !objHandle->dlcLicenseBit || ret == 0 || g_DebugLicenseAllDLC)
									objHandle->dlcLicensed = true;
								else
									objHandle->dlcLicensed = false;

								if(objHandle->dlcLicensed)
									licensedSomeFiles = true;

								break;
							}
						}
					}
				}

				if(licensedSomeFiles)
				{
					mCorruptFiles.Erase(mCorruptFiles.Begin() + i);
					mCorruptLocIds.Erase(mCorruptLocIds.Begin() + i);
				}
				else
				{
					mCorruptLocIds[i] = UI::C_TextStringID("POP_DLC_UNLICENSED").Value();
				}
			}

			cellFsClose(fileHandle);

			SAFE_DELETE_ARY(readBuffer);
		}
	}

	// ------------------------------------------------------------------------------------------------------------------------------------

	void DLCController::Enumerate()
	{
		int ret, ret2, fileHandle;
		char contentInfoPath[CELL_FS_MAX_FS_PATH_LENGTH];
		char usrdirPath[CELL_FS_MAX_FS_PATH_LENGTH];
		CellGameContentSize size;

		SceNpDrmKey k_licensee[NUM_DLC_PACKS] = {
			{ 0xc0,0xa3,0xb3,0x64,0x1c,0x2a,0xd1,0xef,0x23,0x15,0x3a,0x48,0xa3,0xe1,0x2f,0xe7 },
			{ 0xc1,0xa4,0xb4,0x65,0x1d,0x2b,0xd2,0xe0,0x33,0x25,0x4a,0x58,0xb3,0xf1,0x3f,0xf7 },
			{ 0xd1,0xb4,0xc4,0x75,0x2d,0x3b,0xe2,0xf0,0x34,0x26,0x4b,0x59,0xb4,0xf2,0x30,0xf8 },
			{ 0xd2,0xb5,0xc5,0x76,0x2e,0x3c,0xe3,0xf1,0x44,0x36,0x5b,0x69,0xc4,0x02,0x40,0x08 },
			{ 0xe2,0xc5,0xd5,0x86,0x3e,0x4c,0xf3,0x01,0x54,0x37,0x5c,0x6a,0xc5,0x03,0x41,0x09 },
			{ 0xe3,0xc6,0xd6,0x87,0x3f,0x4d,0xf4,0x02,0x55,0x38,0x5d,0x6b,0xc6,0x04,0x44,0x0a },
			{ 0xf3,0xd6,0xe6,0x97,0x4f,0x5d,0x04,0x12,0x65,0x48,0x6d,0x7b,0xd6,0x14,0x54,0x1a },
			{ 0x04,0xe7,0xf7,0xa8,0x50,0x6e,0x15,0x23,0x76,0x59,0x7e,0x8c,0xe7,0x25,0x65,0x2b },
			{ 0x05,0xe8,0xf8,0xa9,0x51,0x6f,0x16,0x24,0x77,0x5a,0x7f,0x8d,0xe8,0x26,0x66,0x2c },
			{ 0x15,0xf8,0x08,0xb9,0x61,0x7f,0x26,0x34,0x87,0x6a,0x8f,0x9d,0xf8,0x36,0x76,0x3c },
			{ 0x25,0x08,0x18,0xc9,0x71,0x8f,0x36,0x44,0x97,0x7a,0x9f,0xad,0x08,0x46,0x86,0x4c },
			{ 0x25,0x09,0x18,0xca,0x71,0x80,0x36,0x45,0x97,0x7b,0x9f,0xae,0x08,0x47,0x86,0x4d },
			{ 0x35,0x09,0x28,0xca,0x81,0x80,0x46,0x45,0xa7,0x7b,0xaf,0xae,0xa8,0x47,0x96,0x4d },
			{ 0x36,0x0a,0x29,0xcb,0x82,0x81,0x47,0x46,0xa8,0x7c,0xa0,0xaf,0xa9,0x48,0x97,0x4e },
			{ 0x46,0x1a,0x39,0xdb,0x92,0x91,0x57,0x56,0xb8,0x8c,0xb0,0xbf,0xb9,0x58,0xa7,0x5e },
			{ 0x47,0x1b,0x3a,0xdc,0x93,0x92,0x58,0x57,0xb9,0x8d,0xb1,0xb0,0xba,0x59,0xa8,0x5f },
			{ 0x48,0x1c,0x3b,0xdd,0x94,0x93,0x59,0x58,0xba,0x8e,0xb2,0xb1,0xbb,0x5a,0xa9,0x50 },
			{ 0xba,0x8e,0xb2,0xb1,0xbb,0x5a,0xa9,0x50,0x48,0x1c,0x3b,0xdd,0x94,0x93,0x59,0x58 },
			{ 0xca,0x9e,0xc2,0xc1,0xcb,0x6a,0xb9,0x60,0x58,0x2c,0x4b,0xed,0xa4,0xa3,0x69,0x68 },
			{ 0xcb,0x9f,0xc3,0xc2,0xcc,0x6b,0xb0,0x61,0x59,0x2d,0x4c,0xee,0xa5,0xa4,0x6a,0x69 },
			{ 0xcc,0x90,0xc4,0xc3,0xcd,0x6c,0xb1,0x62,0x5a,0x2e,0x4d,0xef,0xa6,0xa5,0x6b,0x6a },
			{ 0xdc,0xa0,0xd4,0xd3,0xdd,0x7c,0xc1,0x72,0x6a,0x3e,0x5d,0xff,0xb6,0xb5,0x7b,0x7a },
			{ 0xec,0xb0,0xe4,0xe3,0xed,0x8c,0xd1,0x82,0x7a,0x4e,0x6d,0x0f,0xc6,0xc5,0x8b,0x8a },
			{ 0xdd,0xa1,0xd5,0xd4,0xde,0x7d,0xc2,0x73,0x6b,0x3f,0x5e,0xf0,0xb7,0xb6,0x7c,0x7b },
			{ 0xde,0x7d,0xc2,0x73,0x6b,0x3f,0x5e,0xf0,0xb7,0xb6,0x7c,0x7b,0xdd,0xa1,0xd5,0xd4 },
			{ 0xdd,0xa1,0xd5,0xd4,0xde,0x7d,0xc2,0x73,0xb7,0xb6,0x7c,0x7b,0x6b,0x3f,0x5e,0xf0 },
			{ 0xee,0xb2,0xe6,0xe5,0xef,0x8e,0xd3,0x84,0xc8,0xc7,0x8d,0x8c,0x7c,0x40,0x6f,0x01 },
			{ 0xef,0xb3,0xf6,0xf5,0xe0,0x8f,0xe3,0x94,0xc9,0xc8,0x9d,0x9c,0x7d,0x41,0x7f,0x11 },
			{ 0xe0,0xb4,0x06,0x05,0xe1,0x80,0xf3,0xa4,0xca,0xc9,0xad,0xac,0x7e,0x42,0x8f,0x21 },
			{ 0xf1,0xc4,0x06,0x19,0xe2,0x90,0xf4,0xab,0xca,0x01,0xae,0xba,0x7f,0x52,0x8f,0x31 },
			{ 0xe2,0xc4,0x16,0x20,0xf1,0x81,0xa3,0xbc,0xda,0x02,0xae,0xab,0x7f,0x62,0x8a,0x22 },
			{ 0xf3,0xb5,0xa6,0x21,0xa1,0x91,0xa9,0xcd,0xad,0x03,0xaf,0xfc,0x7f,0x72,0x80,0x5a },
		};

		// Check game data directory
		ret = cellGameDataCheck( CELL_GAME_GAMETYPE_GAMEDATA, PlatformPS3::g_ps3_titleId, &size);
		ret2 = cellGameContentPermit(contentInfoPath, usrdirPath);

		if (ret != CELL_GAME_RET_OK || ret == CELL_GAME_RET_NONE)
		{
			LOGWARN(g_DLCLogger, "Failed to get game directory for DLC.");
			return;
		}

		if(ret2 != CELL_GAME_RET_OK)
		{
			LOGWARN(g_DLCLogger, "Failed to get game directory for DLC.");
			return;
		}

		for( int i = 1; i <= NUM_DLC_PACKS; i++ )
		{
			// Check I can open the file
			char dlcInfoFile[CELL_FS_MAX_FS_PATH_LENGTH];
			snprintf(dlcInfoFile, sizeof(dlcInfoFile), "%s/DLC%i/%s", usrdirPath, i, DLC_INFO_FILE);

			mCorruptFiles.PushBack(UI::C_WideString128(dlcInfoFile));
			mCorruptLocIds.PushBack(UI::C_TextStringID("POP_DLC_CORRUPT").Value());

			ret = sceNpDrmIsAvailable2(&k_licensee[i-1], dlcInfoFile);
			if (ret != CELL_OK)
			{
				// File does not exist, so we don't want to record it as broken
				if(ret == ENOENT)
				{
					mCorruptFiles.PopBack();
					mCorruptLocIds.PopBack();
				}
				else
				{
					if(mCorruptLocIds.Size() > 0)
						mCorruptLocIds[mCorruptLocIds.Size()-1] = UI::C_TextStringID("POP_DLC_UNLICENSED").Value();

					LOGWARN(g_DLCLogger, "Failed to get DRM license for DLC information file.");
				}
				continue;
			}				

			// Open the file
			SceNpDrmOpenArg arg;
			arg.flag = SCE_NP_DRM_OPEN_FLAG;
			ret = cellFsOpen(dlcInfoFile, CELL_FS_O_RDONLY, &fileHandle, &arg, sizeof(arg));
			if (ret != CELL_FS_OK)
			{
				LOGWARN(g_DLCLogger, "Failed to open file handle for DLC information file.");
				continue;
			}

			// Get information on the file
			CellFsStat stat;
			ret = cellFsFstat(fileHandle, &stat);
			if (ret != CELL_FS_OK)
			{
				LOGWARN(g_DLCLogger, "Failed to get statistics on DLC information file.");
				cellFsClose(fileHandle);
				continue;
			}

			// Load the file into a buffer
			uint64_t file_size = stat.st_size;
			uint64_t rsize;
			char readBuffer[file_size];

			ret = cellFsRead(fileHandle, readBuffer, file_size, &rsize);

			if (ret != CELL_OK || rsize != file_size)
			{
				LOGWARN(g_DLCLogger, "Failed to read DLC information file.");
				cellFsClose(fileHandle);
				continue;
			}

			// Open the buffer as an XT Pack and then organise the raw files to load.
			XtPack* xtPack = XtPackOpen(C_GameApp::GetInstance()->GetGlobalXtDatabase(), readBuffer, file_size);

			if(xtPack && xtPack->records && xtPack->records->record)
			{
				AmaxDLCPack* dlcPack = (AmaxDLCPack*)xtPack->records->record->object;

				if(dlcPack)
				{
					Btl::Set<U32>::InsertionReturnType returnType = mDLCPacksInstalled.Insert(dlcPack->mPackID);
					mPacksOpen.PushBack(xtPack);

					for( int j = 0; j < dlcPack->mLoadableItems.count; j++ )
					{
						mFilesToLoad.PushBack();
						mFilesToLoad[mFilesToLoad.Size()-1].Printf("%s/DLC%i/%s", usrdirPath, i, dlcPack->mLoadableItems[j].Text());
					}
					cellFsClose(fileHandle);
				}
			}

		}
	}
}