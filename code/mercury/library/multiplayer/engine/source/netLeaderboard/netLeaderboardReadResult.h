#ifndef NETLEADERBOARDREADRESULT_H
#define NETLEADERBOARDREADRESULT_H

#ifdef _MSC_VER
#pragma once
#endif


#include "network/scoreboardStructures.h"
#include "network/playerSkill.h"

#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>
#include <bdCore/bdContainers/bdLinkedList.h>

#include "netEnums/netEnums.h"

BD_REFERENCE(netRowData);
class netRowData: public Scoreboard::RowData, public NetPlay::PlayerSkill, public bdReferencable
{
public:
	netRowData()
	{}

	virtual ~netRowData()
	{}

	Scoreboard::ColumnData* find(int fieldId)
	{
		for (S32 i=0; i< m_Columns.Size(); i++)
		{
			if(m_Columns[i].m_FieldId == fieldId)
			{
				return &(m_Columns[i]);
			}
		}
		return NULL;
	}


	template<typename TYPE>
	bool get(int fieldId, TYPE& val)
	{
		Scoreboard::ColumnData* c = find(fieldId);
		if(c == NULL) 
			return false;
		
		switch(c->m_DataType)
		{
		default: assert(false); return false;
		case Scoreboard::Int32:			val = (TYPE)c->m_Data.m_S32; return true;
		case Scoreboard::Int64:			val = (TYPE)c->m_Data.m_S64; return true;
		case Scoreboard::Float32:		val = (TYPE)c->m_Data.m_F32; return true;
		case Scoreboard::Float64:		val = (TYPE)c->m_Data.m_F64; return true;
		}
	}

	bool setS64(int fieldId, S64 value)
	{
		Scoreboard::ColumnData* c = find(fieldId);
		if(c == NULL)
		{
			Scoreboard::ColumnData cd;
			cd.SetS64(fieldId, value);
			m_Columns.PushBack(cd);
			return true;
		}
		c->SetS64(fieldId, value);
		return true;		
	}

	bool setS32(int fieldId, S32 value)
	{
		Scoreboard::ColumnData* c = find(fieldId);
		if(c == NULL)
		{
			Scoreboard::ColumnData cd;
			cd.SetS32(fieldId, value);
			m_Columns.PushBack(cd);
			return true;
		}
		c->SetS32(fieldId, value);
		return true;
	}
};

BD_REFERENCE(netLeaderboardReadData);
class netLeaderboardReadData: public bdReferencable
{
protected:
	netLeaderboardReadData(int id);

public:
	static netLeaderboardReadDataRef create(int id);
	virtual bool columnIdsToPropertyIds(netRowData* rowData)=0;
	virtual ~netLeaderboardReadData();
	
public:
	void clear();
	void updateRow(Scoreboard::RowData* row);
	void updateSkill(const SysProfile::UserIdentifier& user, netUserId userId, NetPlay::PlayerSkill* skill);

	netRowDataRef findRow(netUserId user);
	netRowDataRef getRow(int index);
	netRowDataRef findLeader();

	int countRows();

	const int					m_leaderboardId;
	int							m_totalEntries;
	bdLinkedList<netRowDataRef>	m_entries;
};

/////////////////////////////////////////////////////////////////////////////
// Class       : netLeaderboardReadResults
//---------------------------------------------------------------------------
// Created By  : Genesis Network Team
// Created On  : 05 June 2008
// Description : [TO DO : add class description here.]
/////////////////////////////////////////////////////////////////////////////
BD_REFERENCE(netLeaderboardReadResults);
class netLeaderboardReadResults: public bdReferencable
{
public:
	netLeaderboardReadResults();

	void clear();
	netLeaderboardReadDataRef 	createLeaderboard(int leaderboardId);
	netLeaderboardReadDataRef	getLeaderboard(int leaderboardId);
	netRowDataRef				findRowData(int leaderboardId, netUserId userId);
	netRowDataRef				getRowData(int leaderboardId, int index);
	int							countLeaderboardEntries(int leaderboardId);

	bdLinkedList<netLeaderboardReadDataRef>	m_leaderboards;	// leaderboards cache
};

#endif //NETLEADERBOARDREADRESULT_H