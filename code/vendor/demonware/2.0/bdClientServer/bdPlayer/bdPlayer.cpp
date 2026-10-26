// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdPlayer/bdPlayer.h>

#include <bdClientServer/bdReplica/bdClientReplicaData.h>
#include <bdClientServer/bdReplica/bdInterpolationData.h>

#define BD_LOG_LEVEL "bdClientServer/player"

bdPlayer::bdPlayer()
: m_settings(BD_MAX_PLAYER_SETTING-1),
  m_id(0),
  m_clientCommandWindow(0),
  m_serverData(BD_NULL)
{	
}

bdPlayer::~bdPlayer()
{
	if(m_clientCommandWindow)
	{
		delete m_clientCommandWindow;
	}
	if(m_serverData)
	{
		delete m_serverData;
	}
}

void bdPlayer::setSettings(const bdUInt32 settings)
{
	m_settings |= settings;
}

void bdPlayer::clearSettings(const bdUInt32 settings)
{
	m_settings &= ~(settings);
}

bdBool bdPlayer::testSettings(const bdUInt32 settings) const
{
	return (m_settings & settings) == settings;
}

void bdPlayer::setControlledReplica(bdReplicaRef replica)
{

	if(replica && (m_controlledReplica == replica))
	{
		BD_WARN(BD_LOG_LEVEL,"already controlling specified replica.");
	}

	if(m_controlledReplica != replica)
	{
		if(m_clientCommandWindow)
		{
			m_clientCommandWindow->reset();
		}
		m_controlledReplica = replica;
	}
}

bdReplicaRef bdPlayer::getControlledReplica() const
{
	return m_controlledReplica;
}

void bdPlayer::setId(const bdPlayerId id)
{
	m_id = id;
}

bdPlayerId bdPlayer::getId() const
{
	return m_id;
}

void bdPlayer::serialize(bdBitBufferRef)
{
}

bdBool bdPlayer::deserialize(bdBitBufferRef)
{
	return true;
}

bdCommandWindow* bdPlayer::createClientCommandWindow()
{
	if(!m_clientCommandWindow)
	{
		m_clientCommandWindow = new bdCommandWindow;
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL,"Player already has a bdCommandWindow.");
	}

	return m_clientCommandWindow;
}

bdCommandWindow* bdPlayer::getClientCommandWindow()
{
	return m_clientCommandWindow;
}

bdServerPlayerData* bdPlayer::createServerData()
{
	if(!m_serverData)
	{
		m_serverData = new bdServerPlayerData;
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL,"Player already has a bdServerPlayerData.");
	}

	return m_serverData;
}

bdServerPlayerData* bdPlayer::getServerData()
{
	return m_serverData;
}

void bdPlayer::replay(const bdCommandSeqNum fromCommand) 
{
	if(m_controlledReplica)
	{
		// check the last command seen by the server is still in the window
		bdUserCommand *lastCommand = m_clientCommandWindow->get(fromCommand);
		if(!lastCommand)
		{
			BD_WARN(BD_LOG_LEVEL,"last command seen by server is very old. May not be able to replay subsequent commands.");
		}

			// replay any commands the sever hasn't yet seen
		bdCommandSeqNum nextCommand = fromCommand + 1;
		lastCommand = m_clientCommandWindow->get(nextCommand);
//		bdUInt32 replayCount = 0;
		while(lastCommand)
		{
			// replay command on the interpolation target if any
			// otherwise replay on the controlledReplica
			bdInterpolationInterface *target = m_controlledReplica;
			bdClientReplicaData *const clientData = m_controlledReplica->getClientData();
			if(clientData &&
			   clientData->getInterpolationData() &&
			   clientData->getInterpolationData()->getTarget() &&
			   clientData->getInterpolationData()->getActive())
			{
				target = clientData->getInterpolationData()->getTarget();
			}

			target->applyCommand(lastCommand, false);
			target->extrapolate(lastCommand->getTimeInSeconds(),  true);

			nextCommand++;
			lastCommand = m_clientCommandWindow->get(nextCommand);

//			replayCount++;
		}

//		if(replayCount)
//		{
//			bdPrintf("Replayed %u cmds\n", replayCount);
//		}
	}

}
