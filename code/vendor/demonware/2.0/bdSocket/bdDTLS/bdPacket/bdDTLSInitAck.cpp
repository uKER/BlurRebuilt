// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdDTLS/bdPacket/bdDTLSInitAck.h>
#include <bdCore/bdCrypto/bdHMacSHA1.h>

bdDTLSInitAck::bdDTLSInitAck()
: m_timestamp(0),
m_signature(0),
m_initTag(0),
m_localTag(0),
m_peerTag(0),
m_localTieTag(0),
m_peerTieTag(0)
{
}

bdDTLSInitAck::bdDTLSInitAck(bdUInt16 vtag,
							 bdUInt16 initTag,
							 bdUInt16 localTag,
							 bdUInt16 peerTag,
							 bdUInt16 localTieTag,
							 bdUInt16 peerTieTag,
							 bdUInt32 timestamp,
							 const bdAddr &peerAddr,
							 bdSecurityID secID)
: bdDTLSHeader(BD_DTLS_INIT_ACK, vtag, 0),
m_timestamp(timestamp),
m_signature(0),
m_initTag(initTag),
m_localTag(localTag),
m_peerTag(peerTag),
m_localTieTag(localTieTag),
m_peerTieTag(peerTieTag),
m_peerAddr(peerAddr),
m_secID(secID)
{
}

bdDTLSInitAck::~bdDTLSInitAck()
{
}

void bdDTLSInitAck::sign(bdHMacSHA1 &hmac)
{
	bdUInt sigsize = sizeof(m_signature);
	m_signature = 0;

	hmac.process(reinterpret_cast<const bdUByte8*>(&m_timestamp), sizeof(m_timestamp));
	hmac.process(reinterpret_cast<const bdUByte8*>(&m_initTag), sizeof(m_initTag));
	hmac.process(reinterpret_cast<const bdUByte8*>(&m_localTag), sizeof(m_localTag));
	hmac.process(reinterpret_cast<const bdUByte8*>(&m_peerTag), sizeof(m_peerTag));
	hmac.process(reinterpret_cast<const bdUByte8*>(&m_localTieTag), sizeof(m_localTieTag));
	hmac.process(reinterpret_cast<const bdUByte8*>(&m_peerTieTag), sizeof(m_peerTieTag));
	bdUInt tmpAddrBufferUsedSize = 0;
	bdUByte8 tmpAddrBuffer[sizeof(bdAddr)];
	m_peerAddr.serialize(tmpAddrBuffer, sizeof(tmpAddrBuffer), tmpAddrBufferUsedSize, tmpAddrBufferUsedSize);
	hmac.process(tmpAddrBuffer, tmpAddrBufferUsedSize);

	hmac.getData(reinterpret_cast<bdUByte8*>(&m_signature), sigsize);

	BD_ASSERT(sigsize == sizeof(m_signature), "HMAC signature returned is smaller than the field.");
}

bdBool bdDTLSInitAck::verify(bdHMacSHA1 &hmac) const
{
	bdUInt32 psign;
	bdUInt sigsize = sizeof(psign);
	hmac.process(reinterpret_cast<const bdUByte8*>(&m_timestamp), sizeof(m_timestamp));
	hmac.process(reinterpret_cast<const bdUByte8*>(&m_initTag), sizeof(m_initTag));
	hmac.process(reinterpret_cast<const bdUByte8*>(&m_localTag), sizeof(m_localTag));
	hmac.process(reinterpret_cast<const bdUByte8*>(&m_peerTag), sizeof(m_peerTag));
	hmac.process(reinterpret_cast<const bdUByte8*>(&m_localTieTag), sizeof(m_localTieTag));
	hmac.process(reinterpret_cast<const bdUByte8*>(&m_peerTieTag), sizeof(m_peerTieTag));
	bdUInt tmpAddrBufferUsedSize = 0;
	bdUByte8 tmpAddrBuffer[sizeof(bdAddr)];
	m_peerAddr.serialize(tmpAddrBuffer, sizeof(tmpAddrBuffer), tmpAddrBufferUsedSize, tmpAddrBufferUsedSize);
	hmac.process(tmpAddrBuffer, tmpAddrBufferUsedSize);

	hmac.getData(reinterpret_cast<bdUByte8*>(&psign), sigsize);

	BD_ASSERT(sigsize == sizeof(psign), "HMAC signature returned is smaller than the field.");

	return m_signature == psign;
}

bdBool bdDTLSInitAck::serialize(void *data,
								const bdUInt size,
								const bdUInt offset,
								bdUInt &newOffset) const
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdDTLSHeader::serialize(data, size, newOffset, newOffset);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_timestamp);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_signature);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_initTag);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_localTag);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_peerTag);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_localTieTag);
	status = status && bdBytePacker::appendBasicType(data, size, newOffset, newOffset, m_peerTieTag);
	status = status && m_peerAddr.serialize(data, size, newOffset, newOffset);
	status = status && bdBytePacker::appendBuffer(data, size, newOffset, newOffset, m_secID.ab, sizeof(m_secID.ab));

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

bdBool bdDTLSInitAck::deserialize(const void* data,
								  const bdUInt size,
								  const bdUInt offset,
								  bdUInt &newOffset)
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdDTLSHeader::deserialize(data, size, newOffset, newOffset);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_timestamp);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_signature);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_initTag);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_localTag);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_peerTag);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_localTieTag);
	status = status && bdBytePacker::removeBasicType(data, size, newOffset, newOffset, m_peerTieTag);
	status = status && m_peerAddr.deserialize(data, size, newOffset, newOffset);
	status = status && bdBytePacker::removeBuffer(data, size, newOffset, newOffset, m_secID.ab, sizeof(m_secID.ab));

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

bdUInt32 bdDTLSInitAck::getTimestamp() const
{
	return m_timestamp;
}

bdUInt16 bdDTLSInitAck::getInitTag() const
{
	return m_initTag;
}

bdUInt16 bdDTLSInitAck::getLocalTag() const
{
	return m_localTag;
}

bdUInt16 bdDTLSInitAck::getPeerTag() const
{
	return m_peerTag;
}

bdUInt16 bdDTLSInitAck::getLocalTieTag() const
{
	return m_localTieTag;
}

bdUInt16 bdDTLSInitAck::getPeerTieTag() const
{
	return m_peerTieTag;
}

const bdAddr& bdDTLSInitAck::getPeerAddr() const
{
	return m_peerAddr;
}

void bdDTLSInitAck::getSecID(bdSecurityID& secID) const
{
	secID = m_secID;
}




