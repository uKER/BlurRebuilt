// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdReplica/bdClientReplicaData.h>

#include <bdClientServer/bdReplica/bdInterpolationData.h>
#include <bdClientServer/bdReplica/bdUpdate/bdRollbackWindow.h>

#define BD_LOG_LEVEL "bdClientServer/clientreplicadata"

bdClientReplicaData::bdClientReplicaData()
: m_rollbackWindow(BD_NULL),
  m_interpolationData(BD_NULL)
{
}

bdClientReplicaData::~bdClientReplicaData()
{
	if (m_rollbackWindow)
	{
		delete m_rollbackWindow;
	}
	if (m_interpolationData)
	{
		delete m_interpolationData;
	}
}

bdInterpolationData* bdClientReplicaData::createInterpolationData()
{
	if (!m_interpolationData)
	{
		m_interpolationData = new bdInterpolationData;
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL,"Replica data already has bdInterpolationData.");
	}
	return m_interpolationData;	
}

bdInterpolationData* bdClientReplicaData::getInterpolationData()
{
	return m_interpolationData;
}

bdRollbackWindow* bdClientReplicaData::createRollbackWindow()
{
	if (!m_rollbackWindow)
	{
		m_rollbackWindow = new bdRollbackWindow;
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL,"Replica data already has a bdRollbackWindow.");
	}
	return m_rollbackWindow;
}


bdRollbackWindow* bdClientReplicaData::getRollbackWindow()
{
	return m_rollbackWindow;
}
