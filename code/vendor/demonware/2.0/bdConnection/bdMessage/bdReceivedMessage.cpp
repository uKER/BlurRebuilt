// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdConnection/bdMessage/bdReceivedMessage.h>

#include <bdConnection/bdMessage/bdMessage.h>
#include <bdConnection/bdConnection.h>

bdReceivedMessage::bdReceivedMessage(const bdMessageRef message,
									 const bdConnectionRef connection)
: m_message(message),
  m_connection(connection)
{
}

bdReceivedMessage::~bdReceivedMessage()
{
}

bdMessageRef bdReceivedMessage::getMessage() const
{
	return m_message;
}

bdConnectionRef bdReceivedMessage::getConnection() const
{
	return m_connection;
}
