// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class to hold message and the connection it came from.

#ifndef BD_RECEIVED_MESSAGE_H
#define BD_RECEIVED_MESSAGE_H

#include <bdCore/bdCore.h>

BD_REFERENCE(bdMessage);
BD_REFERENCE(bdConnection);

/// A class to hold message and the connection it came from.
class bdReceivedMessage
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor
		bdReceivedMessage(const bdMessageRef message,
						  const bdConnectionRef connection);

		/// Destructor
		~bdReceivedMessage();

		bdMessageRef getMessage() const;

		bdConnectionRef getConnection() const;

	protected:

		bdMessageRef m_message;
		bdConnectionRef m_connection;
};

#endif // BD_RECEIVED_MESSAGE_H
