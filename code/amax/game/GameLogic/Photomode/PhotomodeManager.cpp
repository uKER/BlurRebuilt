#include "stdafx.h"

#include <ContentServer/ContentServerManager.h>

#include <Network/Core.h>
#include <Network/DemonwareTask.h>
#include <bdLobby/bdContentStreaming/bdContentStreaming.h>
#include <GameLogic/Structures/LevelState.h>
#include <Game/Game.h>
#include <GameApp.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <ui/UiHelpers.h>

#include <Game\Achievements\AchievementSystem.h>

#include <GameLogic\Twitter\TwitterManager.h>
#include <GameLogic\Facebook\FacebookManager.h>

#include <Gamelogic\Photomode\PhotomodeManager.h>
#include <SystemProfile\SystemProfileManager.h>
#include "AppPhotoCapture.h"

namespace Game
{

C_PhotomodeManager C_PhotomodeManager::s_instance;

void C_PhotomodeManager::Init()
{


}

void C_PhotomodeManager::DownloadPhoto(int index)
{
	bdFileMetaData *md=C_ContentServerManager::Instance().GetMetaData(index);

	delete[] mPhotoDownloadBuffer;

	mPhotoDownloadBufferSize = md->m_fileSize;
	mPhotoDownloadBuffer = new BYTE[mPhotoDownloadBufferSize];

	C_ContentServerManager::Instance().SetSlot(md->m_fileSlot);
	C_ContentServerManager::Instance().DownloadData(md->m_fileID,mPhotoDownloadBuffer,mPhotoDownloadBufferSize);
}

void C_PhotomodeManager::DecodePhotoBuffer()
{
	PhotoMode::C_AppPhotoCapture& photoCapture = PhotoMode::AppPhotoCapture();

	mJpegDecoder.SetData(mPhotoDownloadBuffer,mPhotoDownloadBufferSize);
	
	U8* bits = 0;
	S32 pitch = 0, width = 0, height = 0, format = 0;
	photoCapture.LockBitmap(bits, pitch, width, height, format);


	mJpegDecoder.DecodeIntoBuffer(bits, pitch, width, height, format);


	photoCapture.UnlockBitmap();

	mJpegDecoder.Exit();
}

bool C_PhotomodeManager::UploadPhoto()
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	PhotoMode::C_AppPhotoCapture& photoCapture = PhotoMode::AppPhotoCapture();
	
	// Lock buffer and encode to JPEG:
	U8* bits = 0;
	S32 pitch = 0, width = 0, height = 0, format = 0;
	photoCapture.LockBitmap(bits, pitch, width, height, format);


	mJpegEncoder.EncodeFromBuffer(bits, pitch, width, height, format);


	photoCapture.UnlockBitmap();

	U32 race_position=0;
	U32 type;
	U32 location;
	U32 vehicles[MAX_CONTENT_SERVER_PHOTO_VEHICLE_TAGS];
	int powerups[MAX_CONTENT_SERVER_PHOTO_POWERUP_TAGS];

	if(!strncmp(mJpegTagStr.Chars(),"DOWNLOADED",10))
	{
		return false;
	}

	int items_read=sscanf(mJpegTagStr.Chars(),"%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,",
												&type,&location,
												&vehicles[0],&vehicles[1],&vehicles[2],&vehicles[3],&vehicles[4],
												&powerups[0],&powerups[1],&powerups[2],&powerups[3],&powerups[4],
												&race_position);

	if(items_read != 13)
	{
		return false;
	}

	C_ContentServerManager::Instance().ClearTags();
	bdTag tag;

	tag.set(Game::CONTENT_SERVER_TAGS_PHOTO_CONTENT_TYPE,type);
	C_ContentServerManager::Instance().AddTag(tag);

	tag.set(Game::CONTENT_SERVER_TAGS_LOCATION,location);
	C_ContentServerManager::Instance().AddTag(tag);

	for(int a=0;a<MAX_CONTENT_SERVER_PHOTO_VEHICLE_TAGS;a++)
	{
		tag.set(Game::CONTENT_SERVER_TAGS_VEHICLE,vehicles[a]);
		C_ContentServerManager::Instance().AddTag(tag);
	}

	for(int a=0;a<MAX_CONTENT_SERVER_PHOTO_POWERUP_TAGS;a++)
	{
		tag.set(Game::CONTENT_SERVER_TAGS_POWERUP,powerups[a]);
		C_ContentServerManager::Instance().AddTag(tag);
	}

	C_ContentServerManager::Instance().UploadData(mJpegEncoder.JpegData(), mJpegEncoder.JpegLength(), NULL, 0, CONTENT_SERVER_TYPE_PHOTO);

	return true;
}

}; // namespace Game
