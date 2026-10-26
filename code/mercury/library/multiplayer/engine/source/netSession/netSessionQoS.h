#ifndef NETSESSIONQOS_H
#define NETSESSIONQOS_H

#ifdef _MSC_VER
#pragma once
#endif


#include "netSessionEnums.h"
#include "netStream/netVector.h"

// qos
BD_REFERENCE(netSessionQoS);
class netSessionQoS: public netVector
{
public:
	netSessionQoS();
	~netSessionQoS();
	
	netSessionQoS& operator = (const netSessionQoS& other);
	bool operator == (const netSessionQoS& other) const;
	bool operator != (const netSessionQoS& other) const { return !(*(this) == other); }

	virtual bool   serialise(class netStream* stream);
	virtual bool deserialise(class netStream* stream);
	
	int	m_ping;
};

#endif //NETSESSIONQOS_H