#ifndef NETGUID_H
#define NETGUID_H

#ifdef _MSC_VER
#pragma once
#endif


#include "netStream/netSerialise.h"

#define NET_GUID_NOT_SET	(0)
#define NET_GUID_START		(1000)
#define NET_GUID_MIGRATION  (1000)

class netGuid: public netSerialisable
{
public:
	netGuid(const netGuid& other);
	netGuid(unsigned int guid=NET_GUID_NOT_SET);

	static netGuid null();

	bool notNull() const { return (*this) != null(); }
	bool  isNull() const { return (*this) == null(); }

	bool operator >  (const netGuid& g) const;
	bool operator <  (const netGuid& g) const;
	bool operator == (const netGuid& g) const;
	bool operator != (const netGuid& g) const;

	bool serialise(netStream* stream);
	bool deserialise(netStream* stream);

	unsigned int m_guid;
};

class netGuidGenerator
{
public:
	netGuidGenerator(unsigned char header);

	void seed(unsigned int value);
	void seed(netGuid start, unsigned int padding=100);
	
	netGuid	generate();
	
	unsigned char	m_header;
	unsigned int	m_iterator;
};

#endif //NETGUID_H