// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "QOSManager.h"
#include <bdSocket/bdQoS/bdQoSProbe.h>

QOSManager::QOSManager()
:m_playerCount(1)
{
}

QOSManager::~QOSManager()
{
}

void QOSManager::init(	bdQoSProbe *probe,
						const bdSecurityID &secID)
{
	bdRandom rand (secID.ab[0]);
	m_playerCount = rand.nextUInt();
	bdUByte8 *countPointer = reinterpret_cast<bdUByte8*>(&m_playerCount);
	m_qosProber = probe;
	m_qosProber->listen(secID, countPointer, sizeof(m_playerCount));
}
