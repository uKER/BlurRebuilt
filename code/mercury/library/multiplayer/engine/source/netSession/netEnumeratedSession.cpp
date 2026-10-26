
#include "stdafx.h"
#include "netEnumeratedSession.h"
#include "netSession/netSessionQoS.h"
#include "netStream/netVector.h"
#include "netTransport/netTransport_Addr.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

static int s_sessionGuid = 0;

netEnumeratedSession::netEnumeratedSession() 
: bdGameInfo()
, m_enumeratedIndex(-1)
, m_guid(s_sessionGuid++)
, m_qos(new netSessionQoS)
{}

netEnumeratedSession::~netEnumeratedSession()
{}


bool netEnumeratedSession::serialise(class netStream* stream)
{
	netTransport::Addr addr;
	addr.fromAddr(m_hostAddr);

	m_qos->serialise(stream);
	stream->writeType(m_titleId);
	stream->writeType(m_secID);
	stream->writeType(m_secKey);
	stream->writeType(addr);
	return stream->ok();
}

bool netEnumeratedSession::deserialise(class netStream* stream)
{
	netTransport::Addr	addr;
	if(m_hostAddr.isNull()) m_hostAddr = new bdCommonAddr;
	
	m_qos->deserialise(stream);
	stream->readType(m_titleId);
	stream->readType(m_secID);
	stream->readType(m_secKey);
	stream->readType(addr);

	if(stream->ok()) addr.toAddr(m_hostAddr);
	return stream->ok();

}

/// Serializes the entire game info into a bitbuffer
/// \param bitBuffer[in|out] The bitbuffer into which the member variables are serialized.
void netEnumeratedSession::serialize(bdBitBuffer& bitBuffer) const
{
	netVector params(128);
	if(!m_qos->serialise(&params)) 
	{
		assert(false);
		return;
	}

	int paramsLength = params.getWriteByteCount();
	
	bdGameInfo::serialize(bitBuffer);
	bitBuffer.writeFull(paramsLength);
	bitBuffer.writeBits(params.buffer(), (paramsLength << 3));
	
}

/// Deserializes a game info from the given bitbuffer
bdBool netEnumeratedSession::deserialize(const bdCommonAddrRef me, bdBitBuffer& bitBuffer)
{
	int paramsLength;
	bool ok = true;
	ok = ok && bdGameInfo::deserialize(me, bitBuffer);
	ok = ok && bitBuffer.readFull(paramsLength);

	netVector params(paramsLength);
	ok = ok && bitBuffer.readBits(params.buffer(), (paramsLength << 3));
	params.setWriteByteCount(paramsLength);
	ok = ok && m_qos->deserialise(&params);
	return ok;
}

NetPlay::SessionSecurity netEnumeratedSession::security()
{
	NetPlay::SessionSecurity sec;
	sec.m_Id = getSecurityID();
	sec.m_Key = getSecurityKey();
	return sec;
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

