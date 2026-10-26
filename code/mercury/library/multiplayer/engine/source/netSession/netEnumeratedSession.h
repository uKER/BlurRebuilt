#ifndef NETENUMERATEDSESSION_H
#define NETENUMERATEDSESSION_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netSessionEnums.h"

#include <bdNet/bdLANDiscovery/bdGameInfo.h>
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>

BD_REFERENCE(netSessionQoS);
BD_REFERENCE(netEnumeratedSession);
class netEnumeratedSession: public bdGameInfo, public netSerialisable
{
public:
	netEnumeratedSession();
	~netEnumeratedSession();
	
	virtual bool serialise(class netStream* stream);
	virtual bool deserialise(class netStream* stream);

	void serialize(bdBitBuffer& bitBuffer) const;
	bdBool deserialize(const bdCommonAddrRef me, bdBitBuffer& bitBuffer);

	NetPlay::SessionSecurity security();
	
	netSessionQoSRef	m_qos;				// quality of service data.
	int					m_enumeratedIndex;	// index in the enumeration list.
	const int			m_guid;				// used by UI to pinpoint a session.
};

#endif //NETENUMERATEDSESSION_H