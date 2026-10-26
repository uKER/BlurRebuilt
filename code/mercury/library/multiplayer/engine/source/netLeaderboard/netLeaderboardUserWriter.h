#ifndef NETLEADERBOARDUSERWRITER_H
#define NETLEADERBOARDUSERWRITER_H

#ifdef _MSC_VER
#pragma once
#endif


#include "network/scoreboardStructures.h"
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>
#include <bdCore/bdContainers/bdLinkedList.h>

BD_REFERENCE(netSession);
BD_REFERENCE(netLeaderboardWriteData);
BD_REFERENCE(netLeaderboardUserWriter);
class netLeaderboardUserWriter: public bdReferencable
{
public:
	netLeaderboardUserWriter();
	~netLeaderboardUserWriter();

	enum
	{
		STATE_NOT_SET=-1,
		STATE_PENDING,
		STATE_WRITING,
		STATE_SUCCESS,
		STATE_ABORTED,
		STATE_FAILED,
	};
	int											m_state;
	SysProfile::UserIdentifier					m_user;
	netSessionRef								m_session;
	bdLinkedList<netLeaderboardWriteDataRef>	m_leaderboards;

	netLeaderboardWriteDataRef findLeaderboard(int scoreboardId);
	netLeaderboardWriteDataRef createLeaderboard(int scoreboardId);

	bool started() const { return (m_state != STATE_NOT_SET); }
	bool busy() const { return (m_state == STATE_PENDING) || (m_state == STATE_WRITING); }
	bool success() const { return (m_state == STATE_SUCCESS); }
	bool failed() const { return (m_state == STATE_FAILED); }
	bool aborted() const { return (m_state == STATE_ABORTED); }
	
	bool setRowData		(int scoreboardId, Scoreboard::RowData* row);
	bool setRating		(int scoreboardId, S64 rating);
	bool setWriteType	(int scoreboardId, Scoreboard::eWriteType writeType);
	bool setPropertyS32	(int scoreboardId, U32 fieldId, S32 value);
	bool setPropertyF32	(int scoreboardId, U32 fieldId, F32 value);
	bool setPropertyS64	(int scoreboardId, U32 fieldId, S64 value);
	bool setPropertyF64	(int scoreboardId, U32 fieldId, F64 value);
};

#endif //NETLEADERBOARDUSERWRITER_H