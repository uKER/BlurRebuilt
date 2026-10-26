#ifndef NETLEADERBOARDREADQUERY_H
#define NETLEADERBOARDREADQUERY_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netTime/netTime.h"

#include <Network/ScoreboardStructures.h>
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>
#include <bdCore/bdContainers/bdLinkedList.h>


BD_REFERENCE(netLeaderboardReadQueryData);
class netLeaderboardReadQueryData: public bdReferencable
{
public:
	netLeaderboardReadQueryData(int leaderboardId, int gameModeId=-1);
	void clear();
	
	struct Column
	{
		Column(U32 id=0, int format=Scoreboard::Int32);
		U32	m_id;
		int	m_format;
	};

	void add(U32 id, int format);

	const int				m_leaderboardId;
	const int				m_gameModeId;
	bdLinkedList<Column>	m_columns;

	bool toColumns(Scoreboard::ColumnIdentifierList& columns, Scoreboard::ColumnTypeList& columnTypes);
};

BD_REFERENCE(netLeaderboardReadResults);
BD_REFERENCE(netLeaderboardReadQuery);
class netLeaderboardReadQuery: public bdReferencable
{
protected:
	netLeaderboardReadQuery();

public:
	enum
	{
		STATE_NOT_SET=-1,
	//--------------------
		STATE_PENDING,				// waiting in queue
		STATE_ENUMERATING_FRIENDS,	// enumerating friends
		STATE_SEARCHING,			// querying leaderboards
		STATE_ABORTED,				// aborted
		STATE_SUCCESS,				// finished, no error
		STATE_ERROR,				// finished, but with errors
	//--------------------
		STATE_COUNT,
	};

	enum
	{
		MODE_NOT_SET=-1,
	//--------------------
		MODE_USERS,			// search for friends entries
		MODE_RANK,			// RANK search
		MODE_PIVOT,			// range around a pivot user
		MODE_SKILL,			// RANK search
	//--------------------
		MODE_COUNT,			
	};

	static netLeaderboardReadQueryRef create();
	virtual bool addDefaultColumns(int leaderboardId)=0;
	virtual ~netLeaderboardReadQuery();
	
public:
	void clear();
	void addColumn(int leaderboardId, U32 columnId, int format);
	void addUser(SysProfile::UserIdentifier user);

	bool started () const { return (m_state > STATE_NOT_SET); }
	bool querying() const { return (m_state == STATE_ENUMERATING_FRIENDS || m_state == STATE_PENDING || m_state == STATE_SEARCHING); }
	bool success () const { return (m_state == STATE_SUCCESS); }
	bool aborted () const { return (m_state == STATE_ABORTED); }
	bool failed  () const { return (m_state == STATE_ERROR); }
	
	void  setRankSearch(const int* scoreboards, int scoreboardsCount, int start, int limit);
	void setPivotSearch(const int* scoreboards, int scoreboardsCount, const SysProfile::UserIdentifier& pivot, int range);
	void  setUserSearch(const int* scoreboards, int scoreboardsCount, const SysProfile::UserIdentifier* users, int count);
	void  setUserSearch(const int* scoreboards, int scoreboardsCount, const SysProfile::FullProfile& profile);
	void setSkillSearch(const int* scoreboards, const int* gameModes, int scoreboardsCount, const SysProfile::UserIdentifier* users, int userCount);
		
	bool searchBySkills()const { return (m_type == MODE_SKILL); }
	bool searchByRank()  const { return (m_type == MODE_PIVOT || m_type == MODE_RANK); }
	bool searchByUsers() const { return (m_type == MODE_USERS); }
	
	int rowCount();

	int												m_titleId;				// title to search
	int												m_type;					// list, rank, pivot
	int												m_batchSize;			// how many scoreboards we read in a row.
	netTime											m_batchWaitTime;		// how much we wait between two batched reads (around 1 second).

	bdLinkedList<netLeaderboardReadQueryDataRef>	m_leaderboards;			// leaderboards to search
	bdLinkedList<SysProfile::UserIdentifier>		m_users;				// users to search
	int												m_rankStart;			// minimum value for rank range search
	int												m_rankLimit;			// maximum value for rank range search
	int												m_pivotRange;			// how many entries around the pivot
	SysProfile::UserIdentifier						m_pivot;				// the pivot in pivot search.
	SysProfile::FullProfile							m_profile;				// search for friends + user of that profile first.

	// current query state, and final result.
	int												m_state;				// state of the search.
	netLeaderboardReadResultsRef					m_results;				// list of leaderboard search results
};

#endif //NETLEADERBOARDREADQUERY_H