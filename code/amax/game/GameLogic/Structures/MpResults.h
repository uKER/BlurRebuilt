#ifndef MP_RESULTS_H
#define MP_RESULTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\Structures\NetVehicleData.h>
#include <GameLogic\Structures\NetTeamData.h>
#include <ui/Game/Profiles/ProfileInterface.h>

namespace Game
{

struct VehicleFinishedInfo
{
	VehicleFinishedInfo()
		: m_Index( -1 ), m_Failed( false )
	{}
	VehicleFinishedInfo( S8 sIndex, bool bFailed = false )
		: m_Index( sIndex ), m_Failed( bFailed )
	{};

	void Clear()
	{
		m_Index = -1;
		m_Failed = false;
	}

	S8		m_Index;
	bool	m_Failed;
};

class C_MpResults
{
public:

	C_MpResults()
	{
		Clear();
	}

	~C_MpResults() {};

	//	Singleton accessor method
	static C_MpResults&	Instance()
	{
		return s_Instance;
	}

	void										Clear();
	void										ClearPlayerIndices();
	void										ClearTeamScores();
	S8											GetWinningTeam() const;
	bool										GetPlayerResults(const U8 localIndex, NetVehicleData& out) const;
	bool										GetPlayerResultsFromJoinId(const U8 joinId, NetVehicleData&out) const;
	U8											GetPlayerPosition(const U8 localIndex) const;
	S8											GetVehicleIndexFromPosition(const U8 position) const;
	U8											GetCrossedFinishLinePosition(const U8 localIndex);
	F32											GetPlayerRaceTime(const U8 localIndex);								
	//	Finish order:
	void										FinishOrderAddFinished(U8 p_Index, bool p_IsPlayer = false);
	void										FinishOrderAddFailed(U8 p_Index, bool p_IsPlayer = false);

	void										SetPreyAndNemesis( S8 prey, S32 peryHits, U8 preyJoinId, S8 nemesis, S32 nemesisHits, U8 nemesisJoinId, SysProfile::UserIdentifier preyUid, SysProfile::UserIdentifier nemesisUid);

	Btl::Vector<VehicleFinishedInfo, Btl::VectorParameters_Array<RacerNet::NET_MAX_VEHICLES> >	m_FinishedOrder;
	S8											m_FinishedOrderPlayerIndex;
	
	MpVehiclesData								m_LiveVehicleData;				//	Data we read/write during play
	MpVehiclesData								m_SortedVehicleResults;			//	Sorted vehicle results we use to create the Results screens

	TeamsData									m_Teams;
	Btl::Vector<S8, Btl::VectorParameters_Array<MAX_USERS> >	m_PlayerSortedIndices;

	F32											m_TotalRaceLength;

	S8											m_LastPreyIndex;
	S32											m_NumTimesHitPrey;
	U8											m_PreyJoinId;
	SysProfile::UserIdentifier					m_PreyUid;

	S8											m_LastNemesisIndex;
	S32											m_NumTimesHitByNemesis;
	U8											m_NemesisJoinId;	
	SysProfile::UserIdentifier					m_NemesisUid;

	bool										m_ResultsValid;					//	Set once the results are calculated, so we can use them to determine start positions for the next race if need be.
private:
	static C_MpResults s_Instance;
};






}	//	namespace Game


#endif // MP_RESULTS_H
