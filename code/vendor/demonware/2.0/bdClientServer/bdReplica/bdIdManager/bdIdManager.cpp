// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdReplica/bdIdManager/bdIdManager.h>

#define BD_LOG_LEVEL "bdClientServer/idmanager"

bdIdManager::bdIdManager()
: m_idCounter(0)
{
}

bdReplicaId bdIdManager::bind()
{
	bdReplicaId id;
	if(m_freedIds.getSize())
	{
		id = m_freedIds[0];
		m_freedIds.removeAt(0);
	}
	else
	{
		id = m_idCounter + 1;
		// check for wraping
		if(id < m_idCounter)
		{
			BD_ERR(BD_LOG_LEVEL, "All the objects ids are in use");
		}

		m_idCounter = id;
	}

	return id;
}

void bdIdManager::unbind(const bdReplicaId& id)
{
	if(id)
	{
		if(id == m_idCounter)
		{
			m_idCounter--;
		}
		else if(id < m_idCounter)
		{
			// check the id has not already been unbound
			bdUInt32 index;
			if(!m_freedIds.findFirst(id, index))
			{
				m_freedIds.pushBack(id);
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL,"id is already unbound");
			}
		}
	}
}
