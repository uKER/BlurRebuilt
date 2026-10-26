// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPeer/bdDelivery/bdDeliveryMessage.h>

bdDeliveryMessage::bdDeliveryMessage()
{
}

bdDeliveryMessage::bdDeliveryMessage(const bdMessageRef message,
									 const bdCommonAddrRef from,
									 const bdCommonAddrRef about)
: m_message(message),
  m_fromAddr(from),
  m_aboutAddr(about)
{
}

bdDeliveryMessage::~bdDeliveryMessage()
{
}

bdMessageRef bdDeliveryMessage::getMessage() const
{
	return m_message;
}

bdCommonAddrRef bdDeliveryMessage::getFromAddr() const
{
	return m_fromAddr;
}

bdCommonAddrRef bdDeliveryMessage::getAboutAddr() const
{
	return m_aboutAddr;
}

void bdDeliveryMessage::setAboutAddr(const bdCommonAddrRef about)
{
	m_aboutAddr = about;
}

bdBool bdDeliveryMessage::operator< (const bdDeliveryMessage& other) const
{
	const bdBool lessThan = (*(m_aboutAddr.operator->()) < *(other.m_aboutAddr.operator->()));
	return lessThan;
}

