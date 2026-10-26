#ifndef GAMEMODE_LAYER_H
#define GAMEMODE_LAYER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Framework/Layer/Layer.h>

#include <GameLogic/GameModes/GameModeDefines.h>
#include <GameLogic/GameModes/GameModeBase.h>
#include <GameLogic/GameModes/GameModes.h>

#include <GameLogic/RaceModes/RaceModeDefines.h>
#include <GameLogic/RaceModes/RaceModeBase.h>
#include <GameLogic/RaceModes/RaceModes.h>

#include <GameLogic/Structures/DailyChallengeManager.h>
#include <GameLogic/DataGathering.h>

#include <Network/Core/NetLoadingThread.h>

namespace Game
{

enum E_LoadState
{
	E_LoadState_None = -1,

	E_LoadState_IntoUI,
	E_LoadState_BackToUI,
	E_LoadState_BackToLobby,
	E_LoadState_IntoRace,

	E_LoadState_Max,
};

class C_AmaxWorldLayer;
class C_RaceLogicLayer;

class C_RaceBookLayer : public Framework::C_Layer, public Messaging::C_MessageHandler
{
public:

	C_RaceBookLayer( Messaging::C_MessageSubscriberSystem* pSubscriberSystem );
	virtual ~C_RaceBookLayer();

	// For layer messages
	HRESULT										ProcessMessage(const Framework::E_LayerMessage message);

	HRESULT										Create();
	HRESULT										Destroy();
	HRESULT										Update();
	HRESULT										GlobalUpdate();
	HRESULT										RenderFullscreen();

	// For game messages
	bool										MessageHandler(const Messaging::C_MessageBase* pMessage);

	//Game mode specific stuff
	void										SetGameMode(E_GameMode gm);
	E_GameMode									GetGameMode(){return m_CurrentGameMode;};
	C_GameModeBase								*GetGameModePtr(){return gGameModeStore(m_CurrentGameMode);}
	void										SetupRace(void *data);

	// For UI testing
	void										SetupFakeResults();

	//Race mode specific stuff
	void										SetRaceMode(E_RaceMode rm);
	const E_RaceMode							GetRaceMode() const;

	C_RaceModeBase								*GetRaceModePtr(){return gRaceModeStore(m_CurrentRaceMode);};
	void										SetupResults();
	void										SetupQuitAndWinResults(lua_State *pState);

	C_AmaxWorldLayer							*GetAmaxWorldLayer() const {return m_pAmaxCurrentWorldLayer;}
	C_AmaxWorldLayer							*GetUIWorldLayer() const { return m_pUIWorldLayer; }
	C_RaceLogicLayer							*GetRaceLogicLayer() const { return m_pRaceLogicLayer; }
	void										LoadUIScene( bool bCreateAsync = false );

	bool										DumpLevel();

	bool										UtilDirCreatedSuccessfully() const {return m_UtilityDirectoryCreated;};

	void										SetCurrentSpTier(U32 currentTier) { m_CurrentSpTier = currentTier; }
	U32											GetCurrentSpTier() { return m_CurrentSpTier; }

	void										SetMpUICar(bool is_mp){m_UseMpUICar=is_mp;};
	bool										SetMpUICar(){return m_UseMpUICar;};

	void										SetFrontEndLoading(bool p_Loading) { m_LoadingFrontEnd = p_Loading; }
	bool										GetFrontEndLoading() { return m_LoadingFrontEnd; }

	void										SetLoadState(E_LoadState p_State) { m_LoadState = p_State; }
	E_LoadState									GetLoadState() { return m_LoadState; }

	DataGathering&								GetDataGathering();

	void										EnterNetworkLoad();
	void										ExitNetworkLoad();

	bool GetTitleStorageDataActive() { return m_MpTitleStorageDataActive; };
	void SetTitleStorageDataActive(bool active);

	MT::Mutex& GetUpdateStreamerOnLoadingScreenMutex();

private:
	void InitialiseNetServicesTitleStorage();
	void UpdateCurrentWorldLayer();

	E_GameMode									m_CurrentGameMode;
	E_RaceMode									m_CurrentRaceMode;

	C_AmaxWorldLayer*							m_pUIWorldLayer;
	C_AmaxWorldLayer*							m_pAmaxWorldLayer;
	C_AmaxWorldLayer*							m_pAmaxCurrentWorldLayer;

	RacerNet::C_NetLoadingThread				m_NetLoadingThread;		
	C_RaceLogicLayer*							m_pRaceLogicLayer;

	bool										m_UtilityDirectoryCreated;
	bool										m_LoadingFrontEnd;
	E_LoadState									m_LoadState;

	U32											m_CurrentSpTier;	//	Used by the Rich Presence
	bool										m_UseMpUICar;
	C_DailyChallengeManager						m_DailyChallengeManager;
	DataGathering								m_DataGathering;

	bool										m_MpTitleStorageDataActive;

	MT::Mutex									m_updateStreamerOnLoadingScreenMutex;
#ifndef CFG_GOLD
	char													m_CurrentUIScreen[128];
	Btl::Vector< float, Btl::VectorParameters_Array<10> >	m_UiScreensActiveTime;		
#endif
};

} // namespace Game

#endif // GAMEMODE_LAYER_H
