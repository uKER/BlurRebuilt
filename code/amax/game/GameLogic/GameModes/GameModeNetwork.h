#ifndef GAMEMODE_NETWORK_H
#define GAMEMODE_NETWORK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/GameModes/GameModeMultiplayer.h>
#include <Network/Misc/NetRaceSettings.h>

#include <system/misc/Random.h>

#include <xt\XtCoreTypes.h>

// forward declerations 
namespace RacerNet
{
	struct NetRaceSettings;
	struct NetRaceSettingsQos;
	struct NetPlayerVehicle;
	class C_NetPlayerManager;
}
struct RaceBookCharacter; 

namespace Game
{
	class C_MultiplayerProfile;

class C_GameModeNetwork : public C_GameModeMultiPlayer
{
public:

	struct ChampionshipStanding
	{
		void Reset()
		{
			mJoinId = 0;
			mPlayerName = "";
			mTotalScore = 0;
			mTotalWins = 0;
		}

		U8						mJoinId;
		UI::C_WideString16		mPlayerName;
		S8						mTotalScore; // -1 = disconnected
		U8						mTotalWins;
	};

	typedef Btl::Vector<ChampionshipStanding> SortedChampionshipStandingContainer;

	C_GameModeNetwork();

	void Initialise();
	void Update();
	void Start();
	void StartRace();
	void InterpretRaceResults();

	bool IsNetwork(void) const { return true; }

	bool GetNetRaceSettings(RacerNet::NetRaceSettings& out);
	bool GetNetRaceSettingsQos(RacerNet::NetRaceSettingsQos& out);
	virtual void SetNetRaceSettings(RacerNet::NetRaceSettings& in);

	void ResetGameValues();	//	Reset any values that need to be initialised fresh every time we join a network game

	void PreSelectRandom();	//	Select a new vehicle and mod load-out (if these are randomized at the event level).

	bool ReceiveReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U8 senderId, const bool partyChannel);
	bool ReceiveUnReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U32 numUnEncryptedBytes, const U8 senderId, const bool partyChannel);

	virtual bool BuildStartRacePacket(RacerNet::NetStartRacePacket* packet);

	virtual void PlayerNotification(const WCHAR* pPlayerName, const bool join, const bool party);

	bool SendLobbyUpdateMessage( const UI::E_LobbyState eState, const F32 fTime );

	/// Returns the number of players in the game
	S32 GetNumberOfPlayers() const;

	void UpdateLobbyState( const UI::E_LobbyState eState, const F32 fTime );

	void ResetChampionship();

	void InitChampionshipStandingsStructure();

	const C_GameModeNetwork::SortedChampionshipStandingContainer& GetSortedChampionshipStandings() const;

	bool GetModsAllowed();

	bool IsTeamGame() const { return m_Settings.m_Teams; }
	S8	 GetLocalPlayerTeam() const;

private:
	/// This will modify the fan multiplyier based on the players legend and rank
	void DecreaseFanMultiplier( C_LevelState& levelState ) const;
	void InitStartPositions(RacerNet::C_NetPlayerManager& playerManager, bool teamDestruction);
	void SetTeamStartPosition( U8 i, int &teamA, int &teamAStep, int lastTeamA, int lastTeamB );

	// Helper funcs for building valid vehicle list
	bool HasLegendVehicleUnlocked(const XtId vehId) const;
	bool HasBossVehicleUnlocked(const XtId vehId) const;
	void SortLobbyRaceInfo();

protected:
	void Default(void);
	bool NetValid(void);
	bool IsHost();
	void MonitorSelectedVehicles(void);
	
	bool HasVehicleUnlocked(const int currentLevel, const AmaxVehicle* pVeh) const;
	bool ChooseNewVehicle(const U8 userIndex, RacerNet::NetPlayerVehicle& out, const bool bRandom = false);
	bool ChooseNewLoadOut(const U8 userIndex, RacerNet::NetPlayerLoadOut& out);

	void ProcessVehicleData(RacerNet::C_NetMessage& msg);
	void ProcessStartRace(RacerNet::C_NetMessage& msg);
	void ProcessVoteToSkipNextRace(RacerNet::C_NetMessage& msg);
	void ProcessVoteOnNextRace(RacerNet::C_NetMessage& msg);
	void ProcessVoteResults(RacerNet::C_NetMessage& msg);

	void UpdateLobbyRaceInfo(void);
	void ProcessLobbyRaceInfo(RacerNet::C_NetMessage& msg);
	bool ReceivePostRaceMessage(RacerNet::C_NetMessage& msg, const U8 senderId);
	
	void ReceiveLobbyUpdateMessage(RacerNet::C_NetMessage& msg, const U8 senderId);
	void ReceiveBeginVotingMessage(RacerNet::C_NetMessage& msg);

	void ReverseStartPositions();

	//	Methods to add AI drivers to the start race packet:
	void AddAiVehiclesToStartRacePacket(RacerNet::NetStartRacePacket* p_pPacket);
	void DistributeAiVehicles(RacerNet::NetStartRacePacket* p_pPacket);
	void NameAiRacers(RacerNet::NetStartRacePacket* p_pPacket);
	bool ChooseAiVehicles(RacerNet::NetStartRacePacket* p_pPacket);

	// overloaded implementations
	void OnSetupRace( lua_State* pState ) OVERRIDE;
	void OnSetupLevelState( lua_State* pState, C_LevelState& levelState ) OVERRIDE;
	void OnCalculateVoteIndicies() OVERRIDE;
	void OnCalculateVoteResults() OVERRIDE;
	void OnVoteOnRace() OVERRIDE;

	///	Calculate the end-of-race awards (and send the results around if we're the host)
	void CalculateAwards();

	const U16 GetPlayersCurrentChampionshipScore(U8 joinId) const;
	const U32 GetChampionshipFansWon(U8 finishPosition) const;
	const U32 GetChampionshipFinishBonus() const;
	const F32 GetChampionshipLobbyWaitTime() const;
	const U8  GetChampionshipStandingsIndex(U8 joinId) const;
	void GenerateSortedStandings();

	void PostRaceLogging(C_MultiplayerProfile* pProfile, C_LevelState& levelState);
protected:
	RacerNet::NetRaceSettings m_Settings;
	Btl::Vector<S8, Btl::VectorParameters_Array<RacerNet::NET_MAX_VEHICLES> > m_StartPositions;
	Btl::Vector<ChampionshipStanding, Btl::VectorParameters_Array<RacerNet::NET_MAX_VEHICLES> > m_ChampionshipStandings; //Indexed by joinID-1
	SortedChampionshipStandingContainer m_SortedChampionshipStandings;

	bool m_ChooseRandoms;
	F32	 m_RaceInfoSortTimer;

	Util::C_Random		m_CarRandomGenerator;
};


} // namespace Game

#endif // GAMEMODE_NETWORK_CUSTOM_H
