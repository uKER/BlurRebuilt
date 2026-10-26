#include "stdafx.h"
#include "netGuid/netGuid.h"
#include "netStream/netStream.h"

netGuid netGuid::null() 
{ 
	return netGuid(NET_GUID_NOT_SET); 
}

netGuid::netGuid(const netGuid& other)
: m_guid(other.m_guid)
{}
	
netGuid::netGuid(unsigned int guid)
{
	m_guid = guid;
}

bool netGuid::operator >  (const netGuid& g) const
{
	return m_guid > g.m_guid; 
}

bool netGuid::operator <  (const netGuid& g) const
{
	return m_guid < g.m_guid; 
}

bool netGuid::operator == (const netGuid& g) const 
{ 
	return m_guid == g.m_guid; 
}

bool netGuid::operator != (const netGuid& g) const 
{ 
	return m_guid != g.m_guid; 
}

bool netGuid::serialise(netStream* stream)
{
	stream->writeType(m_guid);
	return stream->ok();
}

bool netGuid::deserialise(netStream* stream)
{
	stream->readType(m_guid);
	return stream->ok();
}


netGuidGenerator::netGuidGenerator(unsigned char header)
{
	m_header = header;
	m_iterator = 0;
}

void netGuidGenerator::seed(netGuid start, unsigned int padding)
{
	assert((start.m_guid >> 24) == m_header);
	
	unsigned int seedValue = (start.m_guid & 0x00ffffff);

	assert(seedValue < (0x00ffffff - padding));

	seed(seedValue + padding);
}

void netGuidGenerator::seed(unsigned int value)
{
	m_iterator = value;
	assert(value < 0x00ffffff);
	m_iterator &= 0x00ffffff;
}

netGuid	netGuidGenerator::generate()
{
	assert(m_header > 0);
	assert(m_iterator < 0x00ffffff);
	m_iterator++;
	m_iterator &= 0x00ffffff;
	
	unsigned int guid = m_iterator | (m_header << 24);
	
	return netGuid(guid);
}