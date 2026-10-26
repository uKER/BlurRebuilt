#ifndef NETLEADERBOARDWRITER_H
#define NETLEADERBOARDWRITER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/ScoreboardWriter.h>
#include <Network/PlayerSkillWriter.h>
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>
#include <bdCore/bdContainers/bdLinkedList.h>

BD_REFERENCE(netPlayerSkillWriter);
class netPlayerSkillWriter: public NetPlay::C_PlayerSkillWriter, public bdReferencable
{
public:
	netPlayerSkillWriter(int maxPlayers);
	~netPlayerSkillWriter();
};


BD_REFERENCE(netLeaderboardUserWriter);
BD_REFERENCE(netLeaderboardWriter);
class netLeaderboardWriter: public Scoreboard::C_Writer, public bdReferencable
{
public:
	netLeaderboardWriter();
	~netLeaderboardWriter();
	netLeaderboardUserWriterRef m_currentData;
};

BD_REFERENCE(netLeaderboardPublisher);
class netLeaderboardPublisher: public bdReferencable
{
public:
	netLeaderboardPublisher();
	~netLeaderboardPublisher();

	bool update();
	bool clear();

	bool publish(netLeaderboardUserWriterRef data);
	bool publishing();

	enum { MAX_WRITERS = 8 };
	bdLinkedList<netLeaderboardUserWriterRef>	m_records;
	netLeaderboardWriterRef						m_writers[MAX_WRITERS];
};

#endif //NETLEADERBOARDWRITER_H