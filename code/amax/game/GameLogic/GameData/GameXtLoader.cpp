#include "stdafx.h"

#include <Game/Game.h>
#include <GameApp.h>

#include <misc_shared/messaging/GameMessaging.h>
#include <Xt/XtGame.h>

#include "GameXtLoader.h"

#include <Game/Messaging/GameflowMessages.h>

#include <GameLogic/GameData/LocData.h>

#include <GameSystems.h>


static Debug::Logger &g_GameXtLoaderLog = Debug::GetLogger("Amax.GameXtLoader");

Debug::Bool g_CheckForSorceryUpdatesOnDisk("Game.CheckForSorceryDataExportsWhileRunning", false, Debug::Persist());

namespace Game
{

GameXtLoader GameXtLoader::s_instance;

void GameXtLoader::Init()
{
	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );

	AddSubscriber( GameMessaging::E_GameMsg_Update );
}

void GameXtLoader::Update()
{
	if(g_CheckForSorceryUpdatesOnDisk)
		ReLoadXt();
}

bool GameXtLoader::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	bool handledMessage = true;

	switch( pMessage->Type() )
	{
		case GameMessaging::E_GameMsg_Update:
			Update();
			break;

		default :
			handledMessage=false;
			break;
	}

	return handledMessage;
}

bool GameXtLoader::AddPack(E_XTPACK type,Util::C_String pack_name)
{
	GameXtPack newPack;

	newPack.Initialise();
	newPack.meType = type;
	newPack.mXtPackFilePath = pack_name;
	newPack.mLoaded=false;

	mGameXtPacks.PushBack(newPack);

	return false;
}

GameXtPack* GameXtLoader::GetPack(E_XTPACK type)
{
	for (S32 i = 0; i < mGameXtPacks.Size(); ++i)
	{
		if (mGameXtPacks[i].meType == type)
			return &mGameXtPacks[i];
	}

	return NULL;
}

void XtPackOpenCallback(XtPackOpenStatus* status, void* context)
{
	LOGDEBUG(g_GameXtLoaderLog, "GameXtLoader.cpp > %s", status->message);
}

HRESULT GameXtLoader::LoadXt()
{
	MEMORY_TRACKER_PUSH("game.xt");

	HRESULT result = S_OK;

	for(int i = 0; i < mGameXtPacks.Size(); i++)
	{
#ifndef PLATFORM_PS3
		DWORD start = GetTickCount();
#endif

		if(!mGameXtPacks[i].mLoaded)
		{
			HRESULT loadResult = LoadXtPackFile(&mGameXtPacks[i]);

			if (FAILED(loadResult))
			{
				LOGERROR(g_GameXtLoaderLog, "LoadXt: Error loading pack '%s' with result(0x%x)", mGameXtPacks[i].mXtPackFilePath.Chars(), loadResult);

				result = loadResult;
				//return result;
			}
			else
			{
				mGameXtPacks[i].mLoaded=true;
			}
		}

#ifndef PLATFORM_PS3
		DWORD end = GetTickCount();
		LOGDEBUG(g_GameXtLoaderLog, "GameXtLoader::LoadXt() - %s: %.2f", mGameXtPacks[i].mXtPackFilePath.Chars(), float(end - start) / 1000.0f);
#endif
	}
	
	MEMORY_TRACKER_POP("game.xt");

	// If we're here then all's fine
	return result;
}

//Loads an xt pack file in
HRESULT GameXtLoader::LoadXtPackFile(GameXtPack* pGamePack)
{
	HRESULT result;
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	Sys::I_File* pFile = fs.Open( pGamePack->mXtPackFilePath );

	if ( pFile )
	{
		U32 length = (U32)pFile->Length();
		U8* pData = new U8[ length ];

		if ( pData )
		{
			if ( pFile->Read( pData, length ) )
			{
				XtSchema* pXtSchema = XtGetApplicationSchema();

				if ( pXtSchema )
				{
					XtPackOpenParams oo;

					oo.callback = XtPackOpenCallback;
					oo.context = 0;

					pGamePack->mXtAmaxPack = XtPackOpen(	C_GameApp::GetInstance()->GetGlobalXtDatabase(), 
															pData, 
															length, 
															&oo );

					//Store the file time when the file was created...this will allow us to check whether we need to reload it
					Sys::C_FileInfo info;
					fs.FileInfo(pGamePack->mXtPackFilePath, &info);

					pGamePack->mFileTime = info.m_FileTime;
				}				
			}

			delete [] pData;
		}

		delete pFile;

		result = S_OK;
	}
	else
		result = E_FAIL;

	return result;
}

HRESULT GameXtLoader::ReLoadXt()
{
	//Go through each of the register game packs
	HRESULT result;
	int noXtPacksReloaded = 0;
	for(int i = 0; i < mGameXtPacks.Size(); i++)
	{
		result = ReLoadXtPackFile( &mGameXtPacks[i], noXtPacksReloaded );
		if( FAILED(result) )
		{
			//Should probably not assert...although this is a pretty significant fail
			BIZ_ASSERT_FAIL("GameXtLoader::ReLoadXt() > Failed loading a pack...");
			return result;
		}
	}

	if(noXtPacksReloaded > 0)
	{
		GetSubscriberSystem().BroadcastMessage( Game::E_GameflowMessage_SorceryReloaded );
	}

	return S_OK;
}

HRESULT GameXtLoader::ReLoadXtPackFile(GameXtPack* pGamePack, int& noXtPacksReloaded)
{
	HRESULT result = S_OK;

#ifndef CFG_GOLD
	static int checkXt = 0;

	checkXt = (checkXt + 1) % 10;

	//if (!checkXt)
	{
		Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
		Sys::C_FileInfo info;
		if (fs.FileInfo(pGamePack->mXtPackFilePath, &info))
		{
			if(pGamePack->mFileTime != info.m_FileTime)
			{
				pGamePack->mFileTime = info.m_FileTime;

				result = LoadXtPackFile(pGamePack);

				
				if(result == S_OK)
				{
					//Special case, bit of a hack I'll admit but it works
					if(pGamePack->meType == E_XTPACK_GAMEDATA)
					{
						C_LocData::Instance().Delete();
						C_LocData::Instance().Init();
					}

					//Increment the number of Xt packs that have been reloaded
					noXtPacksReloaded++;
				}
			}
		}
	}
#endif
	return result;
}

}	// namespace Game
