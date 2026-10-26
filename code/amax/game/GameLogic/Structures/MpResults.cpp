#include "stdafx.h"

#include <GameLogic\Structures\MpResults.h>

#include <GameApp.h>
#include <Game/game.h>

#include <GameLogic\Structures\LevelState.h>

namespace Game
{

C_MpResults C_MpResults::s_Instance;

void C_MpResults::Clear()
{
	ClearPlayerIndices();
	for(int i = 0; i < m_LiveVehicleData.Size(); i++)
	{
		m_LiveVehicleData[i].Clear();
		m_SortedVehicleResults[i].Clear();
	}
	for(S32 i = 0; i < m_FinishedOrder.Size(); i++)
	{
		m_FinishedOrder[i].Clear();
	}
	m_FinishedOrderPlayerIndex = m_LastPreyIndex = m_LastNemesisIndex = -1;
	m_NumTimesHitPrey = m_NumTimesHitByNemesis = 0;
	for(S32 i = 0; i < m_Teams.Size(); i++)
	{
		m_Teams[i].Clear(true);
	}
	m_FinishedOrderPlayerIndex = m_LastPreyIndex = m_LastNemesisIndex = -1;
	m_TotalRaceLength = 0.0f;
	m_ResultsValid = false;
}

void C_MpResults::ClearPlayerIndices()
{
	for(S32 i = 0; i < m_PlayerSortedIndices.Size(); i++)
	{
		m_PlayerSortedIndices[i] = -1;
	}
}

void C_MpResults::ClearTeamScores()
{
	for(S32 i = 0; i < m_Teams.Size(); i++)
	{
		m_Teams[i].Clear(false);
	}
}

S8 C_MpResults::GetWinningTeam() const
{
	if(m_Teams[0].m_TeamScore > m_Teams[1].m_TeamScore)
		return 0;
	else if(m_Teams[1].m_TeamScore > m_Teams[0].m_TeamScore)
		return 1;
	else
		return -1;
}

bool C_MpResults::GetPlayerResults( const U8 localIndex, NetVehicleData& out ) const
{
	for (S32 i=0; i<m_SortedVehicleResults.Size(); i++)
	{
		if (localIndex == m_SortedVehicleResults[i].m_LocalIndex)
		{
			out = m_SortedVehicleResults[i];
			return true;
		}
	}

	// results for specified player not found.
	return false;
}

bool C_MpResults::GetPlayerResultsFromJoinId( const U8 joinId, NetVehicleData&out ) const
{
	for (S32 i=0; i<m_SortedVehicleResults.Size(); i++)
	{
		if (joinId == m_SortedVehicleResults[i].m_NetJoinId)
		{
			out = m_SortedVehicleResults[i];
			return true;
		}
	}

	// results for specified player not found.
	return false;
}

U8 C_MpResults::GetPlayerPosition( const U8 localIndex ) const
{
	for (S32 i=0; i<m_SortedVehicleResults.Size(); i++)
	{
		if (localIndex == m_SortedVehicleResults[i].m_LocalIndex)
		{
			return m_SortedVehicleResults[i].m_Position;
		}
	}

	// position for specified player not found.
	return 0;
}

S8 C_MpResults::GetVehicleIndexFromPosition( const U8 position ) const
{
	U8 resultsIndex = position - 1;
	S8 vehicleIndex = -1;

	if(resultsIndex >= 0 && resultsIndex < m_SortedVehicleResults.Size())
	{
		vehicleIndex = m_SortedVehicleResults[resultsIndex].m_LocalIndex;
	}
	return vehicleIndex;
}

U8 C_MpResults::GetCrossedFinishLinePosition(const U8 localIndex)
{
	U8 posNotFound = -1; 
	BIZ_ASSERT_MSG(!m_FinishedOrder.Empty(), " C_MpResults::GetCrossedFinishLinePosition(const U8 localIndex) - Error - m_FinishedOrder empty");
	if(!m_FinishedOrder.Empty())
	{
		int index = 0; 
		while(index < m_FinishedOrder.Size())
		{
			if(m_FinishedOrder[index].m_Index == localIndex)
			{
				// Done
				return index+1; 
			}
			else
			{
				index++; 
			}
		}
	}

	BIZ_ASSERT_MSG(!m_FinishedOrder.Empty(), " C_MpResults::GetCrossedFinishLinePosition(const U8 localIndex) - Error - localIndex Not found");
	return posNotFound;
}

F32 C_MpResults::GetPlayerRaceTime(const U8 localIndex)
{
	for (S32 i=0; i<m_SortedVehicleResults.Size(); i++)
	{
		if (localIndex == m_SortedVehicleResults[i].m_LocalIndex)
		{
			return m_SortedVehicleResults[i].m_Statistics[UI::E_MultiplayerStat_RaceTime];
		}
	}
	return 0.0f;
}

void C_MpResults::FinishOrderAddFinished( U8 p_Index, bool p_IsPlayer )
{
	int numCars = C_GameApp::GetInstance()->GetGame()->GetLevelState().mNumCars;
	int element = 0;
	while(m_FinishedOrder[element].m_Index != -1)
	{
		if(element >= numCars - 1)
			break;
		element++;
	}
	m_FinishedOrder[element] = VehicleFinishedInfo( p_Index );
	if(p_IsPlayer)
		m_FinishedOrderPlayerIndex = element;
}

void C_MpResults::FinishOrderAddFailed( U8 p_Index, bool p_IsPlayer )
{
	int numCars = C_GameApp::GetInstance()->GetGame()->GetLevelState().mNumCars;
	int element = numCars - 1;
	while(m_FinishedOrder[element].m_Index != -1)
	{
		if(element == 0)
			break;
		element--;
	}
	m_FinishedOrder[element] = VehicleFinishedInfo( p_Index, true );
	if(p_IsPlayer)
		m_FinishedOrderPlayerIndex = element;
}

void C_MpResults::SetPreyAndNemesis( S8 prey, S32 peryHits, U8 preyJoinId, S8 nemesis, S32 nemesisHits, U8 nemesisJoinId, SysProfile::UserIdentifier preyUid, SysProfile::UserIdentifier nemesisUid)
{
	m_LastPreyIndex = prey;
	m_NumTimesHitPrey = peryHits;
	m_PreyJoinId = preyJoinId;
	m_PreyUid.m_Uid = preyUid.m_Uid;

	m_LastNemesisIndex = nemesis;
	m_NumTimesHitByNemesis = nemesisHits;
	m_NemesisJoinId = nemesisJoinId;
	m_NemesisUid.m_Uid = nemesisUid.m_Uid;

}

} // namespace Game
