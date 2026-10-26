
#include "stdafx.h"
#include "./netLeaderboardWriter.h"
#include "./netLeaderboardWriteData.h"
#include "./netLeaderboardUserWriter.h"
#include "netDebug/netDebug.h"
#include "netSession/netSession.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

netPlayerSkillWriter::netPlayerSkillWriter(int maxPlayers)
{
	Initialise(maxPlayers); 
}

netPlayerSkillWriter::~netPlayerSkillWriter() 
{}

netLeaderboardWriter::netLeaderboardWriter() 
{}
	
netLeaderboardWriter::~netLeaderboardWriter()
{}


netLeaderboardPublisher::netLeaderboardPublisher()
{
	for(int i = 0; i < MAX_WRITERS; i ++)
	{
		m_writers[i] = new netLeaderboardWriter();
		m_writers[i]->Initialise(10, 32);
	}
}

netLeaderboardPublisher::~netLeaderboardPublisher()
{}

bool netLeaderboardPublisher::clear()
{
	for(int i = 0; i < MAX_WRITERS; i ++)
	{
		netLeaderboardUserWriterRef data = m_writers[i]->m_currentData;
		
		if (data.notNull())
		{
			assert(data->m_state == netLeaderboardUserWriter::STATE_WRITING);
			data->m_state = netLeaderboardUserWriter::STATE_ABORTED;
			data = NULL;
		}
		m_writers[i]->StopAndCleanup();
	}

	// check if the session is already present in the list
	// if so, replace the old record
	bdLinkedList<netLeaderboardUserWriterRef>::Position pos = m_records.getHeadPosition();

	while(pos)
	{
		netLeaderboardUserWriterRef record = m_records.forward(pos);

		if(record.isNull()) continue;
		assert(record->m_state == netLeaderboardUserWriter::STATE_PENDING);
		record->m_state = netLeaderboardUserWriter::STATE_ABORTED;
		record = NULL;
	}	
	m_records.clear();
	return true;
}

bool netLeaderboardPublisher::publish(netLeaderboardUserWriterRef data)
{
	assert(data->m_state == netLeaderboardUserWriter::STATE_NOT_SET);
	data->m_state = netLeaderboardUserWriter::STATE_PENDING;
	m_records.addTail(data);
	return true;
}

bool netLeaderboardPublisher::publishing()
{
	// writers still busy
	for(int i = 0; i < MAX_WRITERS; i ++)
	{
		netLeaderboardUserWriterRef data = m_writers[i]->m_currentData;
		
		if (data.notNull())
		{
			assert(data->m_state == netLeaderboardUserWriter::STATE_WRITING);
			return true;
		}
	}

	// still having some data to publish
	return (!m_records.isEmpty());
}

bool netLeaderboardPublisher::update()
{
	// update busy writers
	for(int i = 0; i < MAX_WRITERS; i ++)
	{
		if(!m_writers[i]->IsWriting())
		{
			assert(m_writers[i]->m_currentData.isNull());
			continue;
		}

		// writer finished?
		assert(m_writers[i]->m_currentData.notNull() && (m_writers[i]->m_currentData->m_state == netLeaderboardUserWriter::STATE_WRITING));

		bool error;
		if(!m_writers[i]->Continue(error)) continue;

		// error writing.
		if(error)
		{
			LOGERROR(netDebug::g_Log, "<netLeaderboardPublisher> error writing data.");
			m_writers[i]->m_currentData->m_state = netLeaderboardUserWriter::STATE_FAILED;
			m_writers[i]->m_currentData = NULL;
		}
		// error writing.
		else
		{
			m_writers[i]->m_currentData->m_state = netLeaderboardUserWriter::STATE_SUCCESS;
			m_writers[i]->m_currentData = NULL;
		}
	}

	// no penmding records
	if(m_records.isEmpty()) return true;

	// try to add pending records to idle writers
	bdLinkedList<netLeaderboardUserWriterRef>::Position pos = m_records.getHeadPosition();

	while(pos)
	{
		netLeaderboardUserWriterRef record = m_records.getAt(pos);
		assert(record.notNull() && (record->m_state == netLeaderboardUserWriter::STATE_PENDING));

		// find a idle writer
		for(int i = 0; i < MAX_WRITERS && (record->m_state == netLeaderboardUserWriter::STATE_PENDING); i ++)
		{
			// this writer is busy
			if(m_writers[i]->IsWriting())
			{
				assert(m_writers[i]->m_currentData.notNull() && (m_writers[i]->m_currentData->m_state == netLeaderboardUserWriter::STATE_WRITING));
				continue;
			}

			bdLinkedList<netLeaderboardWriteDataRef>::Position pos = record->m_leaderboards.getHeadPosition();

			while(pos)
			{
				netLeaderboardWriteDataRef leaderboard = record->m_leaderboards.forward(pos);

				Scoreboard::ColumnList columns;
				bdLinkedList<netColumnDataRef>::Position p = leaderboard->m_properties.getHeadPosition();
				while(p)
				{
					netColumnDataRef prop = leaderboard->m_properties.forward(p);
					columns.PushBack(*prop);
				}

				// failed to queue leaderboard
				if(!m_writers[i]->Queue(leaderboard->m_scoreboardId, leaderboard->m_rating, leaderboard->m_writeType, &(columns[0]), columns.Size()))
				{
					record->m_state = netLeaderboardUserWriter::STATE_FAILED;
					LOGERROR(netDebug::g_Log, "<netLeaderboardPublisher> failed to queue scoreboard.");
					break;
				}
			}

			// failed to start
			if((record->m_state == netLeaderboardUserWriter::STATE_FAILED) || !m_writers[i]->Start(record->m_user, *(record->m_session)))
			{
				record->m_state = netLeaderboardUserWriter::STATE_FAILED;
				LOGERROR(netDebug::g_Log, "<netLeaderboardPublisher> failed to start scoreboard write.");
				break;
			}
			else
			{
				// the croreboardes are being written
				record->m_state = netLeaderboardUserWriter::STATE_WRITING;
				m_writers[i]->m_currentData = record;
				break;
			}
		}

		// still pending.
		if(record->m_state == netLeaderboardUserWriter::STATE_PENDING)
		{
			m_records.forward(pos);
		}
		// record not pending anymore. It's not pending anymore, so remove it.
		else
		{
			m_records.removeAt(pos);
		}
	}
	return true;
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

