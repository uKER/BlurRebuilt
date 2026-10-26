#include "stdafx.h"
#include "netSession/netSessionList.h"
#include "netSession/netEnumeratedSession.h"
#include "netSession/netSessionQoS.h"
#include "netStream/netVector.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

netSessionList::netSessionList()
{
}

netSessionList::~netSessionList()
{
	clear();
}

void netSessionList::clear()
{
	while(!m_sessions.isEmpty())
	{
		m_sessions.removeTail();
	}
}

netEnumeratedSessionRef netSessionList::find(int guid)
{
	bdLinkedList<netEnumeratedSessionRef>::Position pos = m_sessions.getHeadPosition();
	
	while(pos)
	{
		netEnumeratedSessionRef session = m_sessions.forward(pos);
		
		if(session.notNull() && session->m_guid == guid)
			return session;
	}
	return NULL;
}

netEnumeratedSessionRef netSessionList::get(int i) 
{ 
	int index = 0;
	bdLinkedList<netEnumeratedSessionRef>::Position pos = m_sessions.getHeadPosition();

	while(pos && index <= i)
	{
		netEnumeratedSessionRef session = m_sessions.forward(pos);
		if(session.isNull()) continue;
		if(i == index) return session;
		index++;
	}
	return NULL;
}


int netSessionList::count() const
{
	if(m_sessions.isEmpty())
		return 0;

	return m_sessions.getSize();
}
	
bool netSessionList::add(netEnumeratedSessionRef session)
{
	// Filter out duplicate game infos
	bdLinkedList<netEnumeratedSessionRef>::Position pos = m_sessions.getHeadPosition();
	
	bdBool isUnique = true;
	
	while (pos)
	{
		netEnumeratedSessionRef existingGame = m_sessions.forward(pos);

		if (*(existingGame->getHostAddr()) ==(*(session->getHostAddr())))
		{
			isUnique = false;
		}
	}
	if (isUnique)
	{
		m_sessions.addHead(session);
		return true;
	}
	return false;
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif


