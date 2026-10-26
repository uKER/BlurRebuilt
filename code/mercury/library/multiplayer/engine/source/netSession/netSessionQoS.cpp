
#include "stdafx.h"
#include "netSessionQoS.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

// qos
netSessionQoS::netSessionQoS()
: m_ping(0)
{}

netSessionQoS::~netSessionQoS()
{
}

netSessionQoS& netSessionQoS::operator = (const netSessionQoS& other)
{
	#pragma message("cosnt cast away!")
	copy((netStream*)&other);
	m_ping	= other.m_ping;
	return *this;
}
	
bool netSessionQoS::operator == (const netSessionQoS& other) const
{
	if(getWriteByteCount() != other.getWriteByteCount()) 
		return false;
	
	const void* a = other.buffer();
	const void* b = buffer();
	int			c = getWriteByteCount();

	int result = bdMemcmp(a, b, c);
	return (result == 0);
}

bool netSessionQoS::serialise(class netStream* stream)
{
	stream->writeMarker("QOS1");
	netVector::serialise(stream);
	return stream->ok();
}

bool netSessionQoS::deserialise(class netStream* stream)
{
	stream->readMarker("QOS1");
	netVector::deserialise(stream);
	return stream->ok();
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

