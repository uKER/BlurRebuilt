#ifndef PLAY_MODE_PLAYLIST_H
#define PLAY_MODE_PLAYLIST_H

#include <system/misc/Random.h>

#include "GameLogic/PlayMode/PlayModeBase.h"
#include "UI/Game/Multiplayer/MultiplayerDefines.h"

// Forward declarations
struct MultiplayerEvent;
struct MultiplayerRace;

namespace Game
{
	class C_LevelState;
}
namespace RacerNet
{
	struct NetRaceSettings;
}

namespace Game
{

/*---------------------------------------------------------------------------------------
	Class	: C_Playlist

	Desc	: 
---------------------------------------------------------------------------------------*/
class C_Playlist : public I_PlayModeBase
{
public:
	C_Playlist();

	void 					Initialise();

							/// Returns the sorcery record of the current playlist
	const MultiplayerEvent*	GetPlaylistData() const;

							/// Returns the sorcery record of the given index
	const MultiplayerRace*	GetRaceData( const S16 sRaceIndex = -1 ) const;

							/// Fills out NetRaceSettings with the playlist data
	void 					SetupNetRaceSettings( RacerNet::NetRaceSettings& settings );

							/// Fills out the level state with the playlist data
	void 					SetupLevelState( C_LevelState& levelState );

							/// Shuffle the playlist using the given seed
	void 					Shuffle( const U32 uSeed );

							/// Set the current race index
	void 					SetRaceIndex( const S16 sIndex );

							/// Resets the cycle to zero if the race is valid, otherwise it will move on until one is found.
	void					ResetCycle( const S32 sNumPlayers );

							/// Returns the index to the next valid race
	S16						FindNextValidRace( const S32 sNumPlayers, const S16 sCurrentIndex );

							/// Calculates the next two valid races that are availble.
	void					FindValidVotingIndicies( const S32 sNumPlayers, const U32 uSeed, const S16 sRaceIndex, S16& sIndexA, S16& sIndexB );

							/// Returns the current race index.
	S16						GetRaceIndex() const { return m_RaceIndex; }

							/// Returns the sorcery playlist race index.
	S16						GetRaceDataIndex( const S16 sRaceIndex = -1 ) const;

							/// Returns the car class for the current race. Pass in the index to get the class for a specific race.
	UI::E_VehicleClass		GetCarClass( const S16 sRaceIndex = -1 ) const;

							/// Returns the seed used to shuffle the current playlist
	U32						GetSeed() const { return m_Seed; }

							/// Moves the current race index on. Returns true if cycle complete.
	void					CycleRaceIndex( const S32 sNumPlayers, const U32 uSeed );

							/// Returns true if its the last race in the playlist
	bool					IsLastRace() const
							{
								return ( m_RaceIndex >= (m_Races.Size() - 1) );
							}

	const bool				IsChampionship() const;
	const bool				IsChampionshipInProgress() const;
	void					StartChampionship();

							/// Must supply implementation of these functions
	UI::E_RaceType			GetRaceMode() const;	
	U32						GetCity() const;
	U32						GetRoute() const;
	S8						GetLapLimit() const;
	S32						GetTimeLimit() const;

private:
	bool					IsRaceValid( const MultiplayerEvent* pEvent, const S16 sRaceDataIndex, const S32 sNumPlayers );

	void					AddRacesRange( int start, int end, const MultiplayerEvent* pEvent );
	void					GenerateSingleRaces( const MultiplayerEvent* pEvent );
	void					GenerateChampionship( const MultiplayerEvent* pEvent );

private:
	struct RaceShuffleData
	{
		S16					m_Index;
		UI::E_VehicleClass	m_CarClass;
	};

	typedef Btl::Vector<RaceShuffleData> TRaceList;

private:
	S16						m_RaceIndex;			/// Index into the races list
	U32						m_Seed;					/// Seed used to shuffle the playlist

	TRaceList				m_Races;
	Util::C_Random			m_Random;

	bool					m_IsChampionship;
	bool					m_ChampionshipInProgress;
};

};// namespace Game

#endif // PLAY_MODE_PLAYLIST_H
