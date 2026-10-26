#ifndef GAME_WORLD_LAYER_H
#define GAME_WORLD_LAYER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Framework/Layer/Layer.h>
#include <misc_shared/Messaging/MessageHandler.h>
#include <Resource/Resource.h>
#include <game_resource/LevelResource.h>
#include <GameLogic/GameData/GameEventData.h>
#include "system/misc/Random.h"

extern bool g_bPhysicsMaterialsReload;

#ifndef CFG_GOLD

namespace GameDebug
{
	class C_GameDebug;
}

#endif

class C_LevelActorCreationDataList;

struct XtPack;
struct XtDb;

namespace Automation
{
	class AutomationStatus;
}

namespace ActorStudioMsgGame
{
	class C_ActorStudioMessaging;
}

namespace Game
{

class C_GameWorld;

namespace Actors
{
	class RaceLogicCore;
}

enum E_WorldState
{
	E_WorldState_Uninitialised,

	E_WorldState_LoadingRace,
	E_WorldState_LoadedRace,
	E_WorldState_RestartingRace,

	E_WorldState_Max,
};

enum E_CutSceneState
{
	E_CutSceneState_Uninitialised,

	E_CutSceneState_IntroPrepping,
	E_CutSceneState_IntroPrepped,
	E_CutSceneState_IntroStarted,
	E_CutSceneState_IntroFinished,
	E_CutSceneState_Idle,
	E_CutSceneState_OutroPrepping,
	E_CutSceneState_OutroPrepped,
	E_CutSceneState_OutroStarted,
	E_CutSceneState_OutroFinished,

	E_CutSceneState_Max,
};

class C_RandomNumberTracker
{
public:

	C_RandomNumberTracker() { Clear(); }

	void Clear()
	{
		m_Numbers.Clear();
		m_Index = -1;
	}

	void Push(U32 p_Num)
	{
		m_Numbers.PushBack(p_Num);
		m_Index++;
	}

	void Reset()
	{
		m_Index = 0;
	}

	U32 Pop()
	{
		if (m_Index < 0 || m_Index >= m_Numbers.Size())
		{
			BIZ_ASSERT(m_Index >= 0 && m_Index < m_Numbers.Size());
			return 0;
		}

		return m_Numbers[m_Index++];
	}

	S32						m_Index;
	Btl::Vector<U32>		m_Numbers;
};

class C_AmaxWorldLayer : public Framework::C_Layer, public Messaging::C_MessageHandler
{
//Functions
public:

	C_AmaxWorldLayer( Messaging::C_MessageSubscriberSystem* pSubscriberSystem );
	virtual										~C_AmaxWorldLayer();

	HRESULT										ProcessMessage(const Framework::E_LayerMessage message);

	HRESULT										Create(bool quickRestart = false);
	HRESULT										Destroy(bool quickRestart = false);
	HRESULT										Update();
	HRESULT										GlobalUpdate();
	HRESULT										RenderSplitscreenWithAA();
	HRESULT										RenderSplitscreenWithoutAA();
	HRESULT										RenderFullscreen();
	HRESULT										PostRender();

	void										InstantiateLevel();
	//For game messages
	bool										MessageHandler(const Messaging::C_MessageBase* pMessage);
	C_GameWorld									*GetGameWorld() const;
	Actors::RaceLogicCore						*GetRaceLogicCore() const;
	GameResource::C_LevelInstance				*GetLevelInstance() {return(m_pLevelInstance);}
	GameResource::C_LevelResource				*GetLevelResource() {return m_pLevelResource; }

	void										LoadAllIntoStreamer();

	void										NotifyActorStudioDataCreated();

	bool										CanRender() { return mCanRender; }

	void										StartStreamerPurge(S32 p_PurgeAmountBytes);

	void										PrepareOutroCutScene();
	E_CutSceneState								GetCutSceneState() { return m_CutSceneState; }
	
	void										ExitSloMo(const int viewportIndex = 0);

	bool										GetResourcesLoaded() { return mRLCReady; }

	E_WorldState								GetWorldState() { return m_WorldState; }
	Util::C_Random								m_RandomGenerator;
	C_RandomNumberTracker						m_RandomNumberTracker;

	void										LoadUILutRamPak();
	void										UnloadUILutRamPak();

//Functions
private:
	void										LoadActorStudioActors();
	void										LoadOutroCutsceneActors();

	void										AddCutsceneActors(C_LevelActorCreationDataList* list, AmaxCutScene *acs);

	void										EnableRumble(bool from_paused);
	void										DisableRumble(bool from_paused);

	bool										mRumbleEnded;

//Variables
private:
	C_GameWorld*								m_pGameWorld;

	ActorStudioMsgGame::C_ActorStudioMessaging* m_pActorStudioMessaging;

	GameResource::C_LevelHandle					m_LevelHandle;
	GameResource::C_LevelResource				*m_pLevelResource;
	GameResource::C_LevelInstance				*m_pLevelInstance;
	
	#ifndef CFG_GOLD
	GameDebug::C_GameDebug*						m_pGameDebug;
	#endif

	Actors::RaceLogicCore						*m_pRLC;

	E_WorldState								m_WorldState;
	E_CutSceneState								m_CutSceneState;	

	bool										mRLCReady,
												mAIReady,
												mActorsLoaded,
												mCanRender;

	XtPack*										m_LevelActorCreationPack;	///> Pointer to Xt PAK file
	XtDb*										m_pXtDb;
	bool										m_lutRamPakMounted;
};

} // namespace Game

#endif // GAME_LAYER_H
