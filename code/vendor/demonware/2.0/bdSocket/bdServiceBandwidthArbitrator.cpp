// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PSP-ADHOC PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdServiceBandwidthArbitrator.h>

#define BD_SLICE_QUOTA_MULTIPLIER (2)

bdServiceBandwidthArbitrator::bdServiceBandwidthArbitrator()
: m_allowedSend(0),
  m_sentThisSlice(false)
{
}

void bdServiceBandwidthArbitrator::reset()
{
	m_allowedSend = 0;
	m_sentThisSlice = false;
}

void bdServiceBandwidthArbitrator::addSliceQuota(bdUInt bytes)
{
	// only increase the quota if it's not too high currently,
	// and we either have no quota at the moment or we just used
	// some of it up.
	if ((m_allowedSend / static_cast<bdInt>(bytes) < BD_SLICE_QUOTA_MULTIPLIER)
		&& ((m_allowedSend <= 0) || m_sentThisSlice))
	{
		m_allowedSend += bytes;
	}
	m_sentThisSlice = false;
}

bdBool bdServiceBandwidthArbitrator::allowedSend(bdUInt) const
{
	return m_allowedSend > 0;
}

void bdServiceBandwidthArbitrator::send(bdUInt bytes)
{
	m_allowedSend -= bytes;
	m_sentThisSlice = true;
}

