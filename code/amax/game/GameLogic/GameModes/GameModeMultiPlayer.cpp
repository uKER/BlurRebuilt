#include "stdafx.h"

#include <Game/Game.h>
#include <GameApp.h>
#include <GameLogic/GameModes/GameModeMultiPlayer.h>
#include <GameLogic/GameModes/GameModeMultiPlayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Profile/GameProfileManager.h>
#include "GameLogic/GameModes/GameModeMultiPlayer.h"
#include "GameLogic/RaceLogicCore/Database/RLDatabase.h"
#include "Game/GameActors/Components/GameComponents.h"
#include "Game/GameActors/Components/VehicleLogic/HandOfGod/VLC_HandOfGod.h"

#include <ui/Game/UILUA.h>

Debug::Logger &g_MpLog(Debug::GetLogger("Multiplayer"));

namespace Game
{

C_MultiplayerAwards C_GameModeMultiPlayer::m_RaceAwards;
C_CustomRace C_GameModeMultiPlayer::m_CustomRace;
C_Playlist C_GameModeMultiPlayer::m_Playlist;

C_GameModeMultiPlayer::C_GameModeMultiPlayer()
{
	SetInternalNameAndID("MultiPlayer",E_GameMode_MultiPlayer);
}

void C_GameModeMultiPlayer::Initialise()
{
	m_PlayMode = E_PlayMode_Nothing;
	m_Playlist.Initialise();
	m_CustomRace.Initialise();
	m_RaceAwards.Init();
	m_VoteInfo.Reset();

	m_RandomVoteResult.Seed((U32)Time::GetCurrentTime());
	m_GeneralRandomNumberGenerator.Seed((U32)(Time::GetCurrentTime() * 17));
}

void C_GameModeMultiPlayer::SetupRace( void *pData )
{
	BIZ_ASSERT( pData );
	if ( !pData )
		return;

	// Initialise playmode data
	if ( m_PlayMode == E_PlayMode_CustomRace )
	{
		m_CustomRace.SetupRace( (lua_State*)pData );
		
		//Data gathering time
		bdOnlineUserID userID;
		SysProfile::C_Manager::Instance().GetDemonwareUserId(SysProfile::C_Manager::Instance().GetPrimaryPadNo(), userID);

		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		if(pLayer)
		{
			pLayer->GetDataGathering().FlagPostEvent(Game::DATA_GATHERING_RECORD_TYPE_CUSTOM_RACE_SETTINGS);
		}
	}
	else
	if ( m_PlayMode == E_PlayMode_Playlist )
	{
		m_VoteInfo.Reset();

		m_Playlist.Shuffle( (U32)Time::GetCurrentTime() );
		m_Playlist.ResetCycle( GetNumPlayersFromLua( (lua_State*)pData ) );
	}

	OnSetupRace( (lua_State*)pData );
}

void C_GameModeMultiPlayer::SetupLevelState( void* pData, C_LevelState& levelState )
{
	BIZ_ASSERT( pData );
	if ( !pData )
		return;

	// Always clear this bad boy
	levelState.Clear();

	// Initialise playmode data
	if ( m_PlayMode == E_PlayMode_CustomRace )
	{
		m_CustomRace.SetupLevelState( levelState );
	}
	else
	if ( m_PlayMode == E_PlayMode_Playlist )
	{
		m_Playlist.SetupLevelState( levelState );
		m_VoteInfo.Clear();
	}

	// Shared level state setup
	levelState.mPickupProbabilities.PushBack(Game::LevelPickupProbability( E_PICKUP_TYPE_FAN, 0.0 ) );// TODO_MP_CUSTOM_RACE

	levelState.mHoGResetCriteria = Components::C_VehicleComponent_HandOfGod::E_HandOfGodResetCriteria_All;
	if(levelState.mRaceMode == E_RaceMode_Multiplayer_Destruction)
	{			
		levelState.mAllowPickupsOnLapZero = true;
		levelState.mPowerUpDamageMultiplier = 2.5f;
			
		//If we're in destruction mode we want a slightly reduced HoG set of criteria
		levelState.mHoGResetCriteria = (Components::C_VehicleComponent_HandOfGod::E_HandOfGodResetCriteria_Flipped | 
										Components::C_VehicleComponent_HandOfGod::E_HandOfGodResetCriteria_OffSpline |
										Components::C_VehicleComponent_HandOfGod::E_HandOfGodResetCriteria_StuckOffGround);
	}


	// Call overloaded function
	OnSetupLevelState( (lua_State*)pData, levelState );

	// dump to output.
	levelState.Finished();
	levelState.Dump();
}

S32 C_GameModeMultiPlayer::GetNumPlayersFromLua( lua_State* pState )
{
	BIZ_ASSERT( pState );

	S32 numPlayers = 0;
	if ( pState && lua_type( pState, 1 ) == LUA_TTABLE )
		Lua::GetDataMember( pState, "num_players", numPlayers );

	BIZ_ASSERT( numPlayers );
	return numPlayers;
}

void C_GameModeMultiPlayer::CyclePlaylist( const S32 sNumPlayers )
{
	BIZ_ASSERT( m_PlayMode == E_PlayMode_Playlist );

	m_Playlist.CycleRaceIndex( sNumPlayers, (U32)Time::GetCurrentTime() );
}

const bool C_GameModeMultiPlayer::IsCharacterRestricted(RaceBookCharacter* pRaceBookChar) const
{
	XtSz8 tag = pRaceBookChar->tag;
	const char* name = tag.Text(); 

	// These strcmps are horrible.. but as its only once on race load... 
	// Would be nicer if MP just disabled achievements for killing certain characters...
	if ( 0 == strcmp(name,"Carver")   || 0 == strcmp(name,"Ajay")    ||
		 0 == strcmp(name, "Bobby")   || 0 == strcmp(name,"Khan")	 ||
		 0 == strcmp(name, "Shannon") || 0 == strcmp(name,"Laraya")  ||
		 0 == strcmp(name, "Natalya") || 0 == strcmp(name,"Drayke")  ||
		 0 == strcmp(name, "ShannonEvil") )
	{
		return true;
	}
	else
	{
		return false;
	}
}

const AmaxVehicle* C_GameModeMultiPlayer::GetRandomVehicle(const VehicleList& validVehicles)
{
	//Just choose a random vehicle from the list
	int randomVehicleIndex = m_GeneralRandomNumberGenerator.Rand(validVehicles.Size());
	return validVehicles[randomVehicleIndex];
}

const U32 C_GameModeMultiPlayer::GetRandomCharacterID()
{
	// Start at any point in the array of chars and ++ so that (if possible)
	// all chars have different drivers. If reach end, loop around. 
	// Start at random point so that drivers arent locked to vehicle index (i.e. dont want to force veh 1 to driver 1 etc)
	MultiplayerConfig* pConfig = C_MultiplayerData::Instance().m_pConfig;
	if(pConfig)
	{
		BIZ_ASSERT_MSG(pConfig->m_AiCharacters.count > 0, "C_GameModeMultiPlayer::GetRandomCharacterID() > - error - C_MultiplayerData::Instance().m_pConfig.m_AICharacters is empty!");

		XtId defaultCharID = pConfig->m_AiCharacters[0]->ID; //initialise to char 0
		int numCharsAvailable = pConfig->m_AiCharacters.count; 

		// Ensure vehicles arent always given the same character (none of that 'aww i hate always being driver 3, they look stuff' here!)
		int charIndexToTry = m_GeneralRandomNumberGenerator.Rand(numCharsAvailable-1);

		// Start at any point in the number of characters and modulo around
		int numTries = 0; 
		while(numTries <= numCharsAvailable) // Make sure we can never have infinite loop.. shouldnt happen.. but... just in case!
		{
			// Try next char
			charIndexToTry++; 
			if(charIndexToTry > numCharsAvailable-1)
			{
				//Ensure we roll over back to start if hit end of array of available characters
				charIndexToTry = charIndexToTry % (numCharsAvailable-1); 
			}

			RaceBookCharacter* nextChar = pConfig->m_AiCharacters[charIndexToTry]; 
			if(!IsCharacterRestricted(nextChar))
			{
				return nextChar->ID; 
			}
			else
			{
				numTries++;
			}
		}

		BIZ_ASSERT_MSG_SPEED(defaultCharID != 0, "C_GameModeMultiPlayer::GetRandomCharacterID() > Character ID is zero, *PLEASE* grab Jon B or Gavin Y.");

		// just in case all chars restricted.
		return defaultCharID;
	}

	// Oops mpconfig not ready yet
	return 0;
}


AmaxCarClass C_GameModeMultiPlayer::GetRaceCarClass() const
{
	AmaxCarClass currentClass = AmaxCarClass_ANY;

	if ( m_PlayMode == E_PlayMode_Playlist )
	{
		currentClass = (AmaxCarClass)m_Playlist.GetCarClass();
	}
	else
	if ( m_PlayMode == E_PlayMode_CustomRace )
	{
		currentClass = (AmaxCarClass)m_CustomRace.GetCarClass();
	}

	return currentClass;
}

bool C_GameModeMultiPlayer::IsCarClassValid( AmaxCarClass validClass, AmaxCarClass vehicleClass ) const
{
	return((validClass == AmaxCarClass_ANY) || (validClass == vehicleClass));
}

void C_GameModeMultiPlayer::CalculateVoteIndices()
{
	BIZ_ASSERT( m_PlayMode == E_PlayMode_Playlist );

	// If the current race index doesnt match any of the previous vote indicies, then use the race index otherwise pass in vote b so that we dont get the same vote candidates again.
	const S16 currentRaceIndex = ( m_Playlist.GetRaceIndex() != m_VoteInfo.indexA && m_Playlist.GetRaceIndex() != m_VoteInfo.indexB ) ? m_Playlist.GetRaceIndex() : m_VoteInfo.indexB;
	m_Playlist.FindValidVotingIndicies( GetNumberOfPlayers(), (U32)Time::GetCurrentTime(), currentRaceIndex, m_VoteInfo.indexA, m_VoteInfo.indexB );

	OnCalculateVoteIndicies();
}

void C_GameModeMultiPlayer::CalculateVoteResults()
{
	BIZ_ASSERT( m_PlayMode == E_PlayMode_Playlist );

	// If we have a tie then randomly pick a race to use
	if ( m_VoteInfo.countA == m_VoteInfo.countB )
	{
		S32 indicies[] =
		{
			m_VoteInfo.indexA,
			m_VoteInfo.indexB,
		};

		m_VoteInfo.result = m_RandomVoteResult.Rand()&1;
		m_Playlist.SetRaceIndex( indicies[m_VoteInfo.result] );
	}
	else
	{
		m_VoteInfo.result = ( m_VoteInfo.countA > m_VoteInfo.countB ) ? 0 : 1;
		m_Playlist.SetRaceIndex( ( m_VoteInfo.countA > m_VoteInfo.countB ) ? m_VoteInfo.indexA : m_VoteInfo.indexB );
	}

	OnCalculateVoteResults();

	LOGINFO( g_MpLog, "Vote finished. Vote %d won, new race index is %d", m_VoteInfo.result, m_Playlist.GetRaceIndex() );
}

void C_GameModeMultiPlayer::VoteOnRace( const S8 sVote )
{
	BIZ_ASSERT( m_PlayMode == E_PlayMode_Playlist );

	m_VoteInfo.countA += !sVote;
	m_VoteInfo.countB += sVote;

	OnVoteOnRace();
}

void C_GameModeMultiPlayer::GetValidVehicles( VehicleList& validVehicles, const bool allowSpecialVehicles /*= true*/ ) const
{
	// Build a list of valid vehicles
	AmaxCarClass validClass = GetRaceCarClass();

	if(IsNetwork())
	{
		//	We only have teams in network games, and need to disable some vehicles for teams
		bool teamGame = false;
		if(m_PlayMode == E_PlayMode_Playlist && m_Playlist.GetPlaylistData())
		{
			teamGame = m_Playlist.GetPlaylistData()->teams;
		}
		Btl::Vector< XtHandle<AmaxVehicle> >& mpVehicles = C_MultiplayerData::Instance().m_MultiplayerVehicles;
		for(int i = 0; i < mpVehicles.Size(); i++)
		{
			if( IsCarClassValid(validClass, mpVehicles[i].Object()->carClass))						//	Only allow vehicles of the appropriate class(es).
			{
				if(!teamGame || (teamGame && C_MultiplayerData::Instance().ValidateVehicleForTeams(mpVehicles[i]->ID)))//	Not a team game OR team game and this vehicle is valid for teams.
				{
					//	Don't include any vehicles that we didn't ship on the disc (can always alter this in a title update to allow
					//	any contentPack IDs that we can guarantee players have (included in said title update, for example)
					if(0 == mpVehicles[i]->contentPack)
					{
						//	Only add vehicles with parent vehicles (boss & Legend cars) if we're allowing them - used to stop AI using these vehicles
						if(!mpVehicles[i].Object()->m_ParentVehicle || (mpVehicles[i].Object()->m_ParentVehicle && allowSpecialVehicles))
							validVehicles.PushBack(mpVehicles[i].Object() );
					}
				}
			}
		}
	}
	else
	{
		XtArray< XtHandle<AmaxVehicle> >& splitscreenVehicles = C_MultiplayerData::Instance().m_pConfig->m_SplitScreenVehicles;
		for ( S32 i = 0; i < splitscreenVehicles.count; i++ )
		{
			const AmaxVehicle* pVehicle = splitscreenVehicles[i];
			if ( pVehicle )
			{
				//Check this is a valid class
				if( IsCarClassValid(validClass, pVehicle->carClass ) )
				{
					validVehicles.PushBack(pVehicle);
				}
			}
		}
	}
}

/// Returns the current playmode base object
const I_PlayModeBase& C_GameModeMultiPlayer::GetPlayModeBase() const
{
	return ( m_PlayMode == E_PlayMode_Playlist ) ? (const I_PlayModeBase&)m_Playlist : (const I_PlayModeBase&)m_CustomRace;
}

} // namespace Game
