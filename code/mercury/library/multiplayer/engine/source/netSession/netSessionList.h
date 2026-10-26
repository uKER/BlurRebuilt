#ifndef NETSESSIONLIST_H
#define NETSESSIONLIST_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netSessionEnums.h"

#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>
#include <bdCore/bdContainers/bdLinkedList.h>
#include <bdNet/bdLANDiscovery/bdGameInfo.h>
#include <bdNet/bdLANDiscovery/bdGameInfo.h>

BD_REFERENCE(netEnumeratedSession);
BD_REFERENCE(netSessionSearch);
BD_REFERENCE(netSessionList);
class netSessionList: public bdReferencable
{
public:
	netSessionList();
	~netSessionList();
		
	int  count() const;
	
	void clear();
	netEnumeratedSessionRef get(int i);
	netEnumeratedSessionRef find(int guid);
	bool add(netEnumeratedSessionRef session);

	bdLinkedList<netEnumeratedSessionRef> m_sessions;
};

#endif //NETSESSIONLIST_H
