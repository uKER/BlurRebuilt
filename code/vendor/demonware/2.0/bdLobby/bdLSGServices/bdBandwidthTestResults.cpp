// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdLSGServices/bdBandwidthTestResults.h>

bdBandwidthTestResults::bdBandwidthTestResults()
: m_bytesReceived(0),
  m_receivePeriodMs(1),
  m_avgSeqNumber(0),
  m_minSeqNumber(0),
  m_maxSeqNumber(0)
{
}

void bdBandwidthTestResults::clear()
{
	m_bytesReceived = 0;
	m_receivePeriodMs = 1;
	m_avgSeqNumber = 0;
	m_minSeqNumber = 0;
	m_maxSeqNumber = 0;
}

bdBool bdBandwidthTestResults::serialize(bdUByte8 *const buffer, 
										 const bdUInt bufferSize) const
{
	bdUInt offset = 0;
	bdBool ok = bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_bytesReceived);
	ok = ok && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_receivePeriodMs);
	ok = ok && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_avgSeqNumber);
	ok = ok && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_minSeqNumber);
	ok = ok && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_maxSeqNumber);
	BD_ASSERT(ok, "bdBandwidthTestResults failed to serialize.");
	return ok;
}

bdBool bdBandwidthTestResults::deserialize(const bdByteBufferRef buffer)
{
	bdBool ok = buffer->read(m_bytesReceived);
	ok = ok && buffer->read(m_receivePeriodMs);
	ok = ok && buffer->read(m_avgSeqNumber);
	ok = ok && buffer->read(m_minSeqNumber);
	ok = ok && buffer->read(m_maxSeqNumber);
	BD_ASSERT(ok, "bdBandwidthTestResults failed to deserialize.");
	return ok;
}

bdFloat32 bdBandwidthTestResults::getBandwidth() const
{
	const bdFloat32 receiveTimeSecs = (BD_MAX(m_receivePeriodMs, 1) / 1000.0f);
	const bdFloat32 kiloBitsPerSec = ((m_bytesReceived * 8)/ receiveTimeSecs) / 1024;
	return kiloBitsPerSec;
}
