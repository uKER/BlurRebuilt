// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE : Represents a loopback connection to the local host.

#ifndef BD_LOOPBACK_CONNECTION_H
#define BD_LOOPBACK_CONNECTION_H

#include <bdCore/bdCore.h>

#include <bdConnection/bdConnection.h>

BD_REFERENCE(bdMessage);

BD_REFERENCE(bdLoopbackConnection);

/// Represents a loopback connection to the local host.
/// A loopback connection behaves like a normal connection and the only reason
/// it is required is because when we attempt to connect to ourselves we put the
/// connection in a state where it is awaiting a reply to the connection
/// proposal it has just sent, when the next message it has to deal with is the
/// connection proposal itself.
class bdLoopbackConnection : public bdConnection
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Destructor.
		virtual ~bdLoopbackConnection();

		/// Does nothing; inherited from base class bdConnection.
		/// Any data arriving destined for a loopback connection is invalid.
		virtual bdBool receive(const bdUByte8 *buffer,
							   const bdUInt32 bufferSize);

		// See documentation in base class bdConnection.
		virtual bdBool send(const bdMessageRef message, const bdBool reliable = false);

		// See documentation in base class bdConnection.
		virtual bdBool getMessageToDispatch(bdMessageRef &message);

	protected:

		/// Grant friendship to bdConnectionStore so that it can
		/// construct an instance of this class.
		friend class bdConnectionStore;

		/// Create a loopback connection to addr.
		/// \param addr[in] The endpoint to which the connection is to be established.
		/// \note The addr parameter must be a local address (i.e.
		///  addr.isLoopback() must return true).
		bdLoopbackConnection( bdCommonAddrRef addr);

		/// Never has any data to send; inherited from base class bdConnection.
		/// \return Zero always.
		virtual bdUInt getDataToSend(bdUByte8 *const buffer,
									 const bdUInt32 bufferSize);

		/// Update the internal status of the loop back connection.
		void updateStatus();

	protected:

		/// A container for all the messages that have been enqueued but
		/// not dispatched.
		bdQueue<bdMessageRef> m_messages;

};

#endif // BD_CONNECTION_H
