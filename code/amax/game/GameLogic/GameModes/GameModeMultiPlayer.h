#ifndef GAMEMODE_MULTIPLAYER_H
#define GAMEMODE_MULTIPLAYER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/GameModes/GameModeBase.h>
#include <GameLogic/GameData/GameVehicleData.h>

#include "GameLogic/PlayMode/CustomRace.h"
#include "GameLogic/PlayMode/Playlist.h"
#include "GameLogic/Structures/MultiplayerAwards.h"

// Forward declare
struct lua_State;

namespace Game
{

struct VoteInfo
{
	void Reset()
	{
		countA = 0;
		countB = 0;
		indexA = -1;
		indexB = -1;
		result = -1;
		myVote = -1;
	}

	void Clear()
	{
		countA = 0;
		countB = 0;
		result = -1;
		myVote = -1;
	}

	S8		myVote;
	S8		countA;
	S8		countB;			
	S16		indexA;
	S16		indexB;
	S16		result;
};

class C_GameModeMultiPlayer : public C_GameModeBase
{
protected:
	typedef Btl::Vector<const AmaxVehicle*> VehicleList;

public:
	C_GameModeMultiPlayer();

	void 							Initialise();

									/// Returns true because this mode is multiplayer
	bool							IsMultiplayer() const { return true; }

									/// Setup the game mode. This should be called once for playlists
	void 							SetupRace(void *data);

									/// Called just before the race load is started
	void							SetupLevelState( void* pData, C_LevelState& levelState );

									/// Sets the current playmode
	void							SetPlayMode( const E_PlayMode eMode ) { m_PlayMode = eMode; }

									/// Returns the current playmode
	E_PlayMode						GetPlayMode() const { return m_PlayMode; }

									/// Returns the playlist controller object
	const C_Playlist&				GetPlaylist() const { return m_Playlist; }

									/// Returns the custom race controller object
	const C_CustomRace&				GetCustomRace() const { return m_CustomRace; }

									/// Returns the current playmode base object
	const I_PlayModeBase&			GetPlayModeBase() const;

									/// Returns the Race awards controller
	const C_MultiplayerAwards&		GetRaceAwards() const { return m_RaceAwards; }

									/// Returns the car class for the current race
	AmaxCarClass					GetRaceCarClass() const;

									/// Returns whether the given vehicle class is allowed
	bool							IsCarClassValid(AmaxCarClass validClass, AmaxCarClass vehicleClass) const;
	bool							IsCarClassValid(AmaxCarClass vehicleClass) const { return IsCarClassValid(GetRaceCarClass(), vehicleClass); }

									/// Moves the playlist onto the next valid race
	void							CyclePlaylist( const S32 sNumPlayers );

									/// Returns the vote info structure
	const VoteInfo&					GetVoteInfo() const { return m_VoteInfo; }

									/// Increments the counter for the vote
	void							VoteOnRace( const S8 sVote );

									/// Gets the next 2 valid races from the playlist which will reshuffle if the cycle has ended
	void							CalculateVoteIndices();

									/// Calculates the results for the vote.
	void							CalculateVoteResults();

									/// Returns the number of players in the game
	virtual S32						GetNumberOfPlayers() const { return 0; }
	virtual const U32				GetRandomCharacterID();
	virtual bool					GetModsAllowed() { return false; }

private:
	S32								GetNumPlayersFromLua( lua_State* pState );

	const bool						IsCharacterRestricted(RaceBookCharacter* pRaceBookChar) const;

protected:
	virtual void					OnSetupRace( lua_State* pState ){};
	virtual void					OnSetupLevelState( lua_State* pState, C_LevelState& levelState ){};
	virtual void					OnCalculateVoteIndicies(){};
	virtual void					OnCalculateVoteResults(){};
	virtual void					OnVoteOnRace(){};

	virtual const AmaxVehicle*		GetRandomVehicle(const VehicleList& validVehicles);
	virtual void					GetValidVehicles(VehicleList& validVehicles, const bool allowSpecialVehicles = true) const;

protected:

	/// These are static since there is an object for each gamemode in memory from boot and there can only be one game mode active at once.
	static C_MultiplayerAwards 		m_RaceAwards;
	static C_CustomRace				m_CustomRace;	// If we add more playmodes then this would be better as an array which
	static C_Playlist				m_Playlist;		// we can index into using the play mode enum

	E_PlayMode						m_PlayMode;

	Util::C_Random					m_RandomVoteResult;
	VoteInfo						m_VoteInfo;


	Util::C_Random					m_GeneralRandomNumberGenerator; //Now there's an easy to remember name :-P


};


} // namespace Game

#endif // GAMEMODE_MULTIPLAYER_H
