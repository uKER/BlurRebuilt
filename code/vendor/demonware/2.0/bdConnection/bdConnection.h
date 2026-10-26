// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE : Represents a connection to a remote host.

#ifndef BD_CONNECTION_H
#define BD_CONNECTION_H

#include <bdCore/bdCore.h>
#include <bdConnection/bdConnectionConfig.h>
#include <bdSocket/bdAddrHandle.h>

// NOTE: Errors pointing to this line probably indicate that you need: 
//       #include <bdConnection/bdMessage/bdMessage.h>
//       in your *.cpp file
BD_REFERENCE(bdMessage);

class bdConnectionListener;

/// bdConnection represents a connection to a remote host. This class
/// is abstract.
class bdConnection : public bdReferencable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Defines the possible states of a bdConnection.
		enum Status
		{
			BD_NOT_CONNECTED,
			BD_CONNECTING,
			BD_CONNECTED,
			BD_DISCONNECTING,
			BD_DISCONNECTED
		};

		/// Destructor.
		virtual ~bdConnection();


		/// Attempts to insert incoming data into the connection.
		/// The data is parsed and used to create bdMessages that are then
		/// inserted into \a this connection.
		/// The data is in the format produced by bdConnection::getDataToSend.
		/// \param buffer[in] A pointer to a buffer containing the data
		///					  received for this connection.
		/// \param bufferSize[in] The size, in bytes, of the buffer pointed
		///						  to be \a buffer
		virtual bdBool receive(const bdUByte8* buffer,
							   const bdUInt bufferSize) = 0;

		/// Enqueues a message for delivery.
		/// \param message[in]	The message to be sent.
		/// \param reliable[in]	Whether or not to send reliably.
		/// \return
		///  - True if the message could be enqueued.
		///  - False otherwise.
		virtual bdBool send(const bdMessageRef message,
							const bdBool reliable = false) = 0;

		/// Gets the next message to be dispatched.
		/// \param message[out] A reference to the message that can be 
		///                     dispatched.
		/// \return
		///  - True if a message has been retrieved for dispatch.
		///  - False otherwise.
		virtual bdBool getMessageToDispatch(bdMessageRef &message) = 0;

		/// Returns the common address for this connection.
		// \return This connection's common address.
		bdCommonAddrRef getAddress() const;

		/// The address to send data to.
		/// \return The address handle for this connection.
		const bdAddrHandleRef &getAddressHandle() const;

		/// Set the address to send data to.
		/// \param addr[in] The address handle to be used.
		void setAddressHandle(const bdAddrHandleRef &addr);

		/// An accessor function for the status of the connection.
		/// \return The status of the connection.
		virtual Status getStatus() const;

		/// Sets the rate at which this connection can send data.
		/// \param bytesPerSecond[in] The number of bytes this connection can
		/// send per second.
		void setTransmissionRate(const bdUInt bytesPerSecond);

		/// Gets the rate at which this connection can send data.
		/// \return The number of bytes this connection can send per second.
		bdUInt getTransmissionRate() const;

		/// Register a connection listener.
		/// The listener will be notified when a connection is established
		/// or destroyed or fails to connect.
		/// \param listener[in]	A pointer to the listener that is to be
		///  registered.
		void registerListener(bdConnectionListener *const listener);

		/// Unregisters a previously registered connection listener.
		/// \param listener[in]	A pointer to the listener to be unregistered.
		void unregisterListener(bdConnectionListener *const listener);

		/// Put the connection into the connected state and allow messages
		/// to be enqueued.
		/// \return
		///  - True if the connection was successfully initiated.
		///  - False otherwise.
		/// \note Use getStatus() to make sure the connection is connected.
		virtual bdBool connect();

		/// Put the connection into the disconnected state but guarantee sending
		/// of any messages currently queued.
		/// \note Use getStatus() to make sure the connection is disconnected.
		virtual void disconnect();

		virtual void close();

	protected:

		/// Grant friendship to bdConnectionStore so that it can
		/// construct an instance of this class.
		friend class bdConnectionStore;

		bdConnection();

		/// Creates a connection to a given endpoint.
		/// The new connection is initially not connected,
		/// only after calling connect() will the connection be established.
		/// \param addr[in] The endpoint to which the connection is to be established.
		bdConnection(bdCommonAddrRef addr);

		/// Gets all data to be sent.
		/// Combines the all messages from \a this connection into a buffer.
		/// Unreliable messages are removed from \a this connection as they are
		/// copied into the buffer and reliable messages are marked as sent.
		/// Resends of timed out unreliable messages are also added to the buffer.
		/// \param buffer[in,out] A pointer to a buffer to receive the data.
		/// \param bufferSize[in] The size in bytes of the buffer pointed to
		///						  by \a buffer.
		/// \return The number of bytes written into the \a buffer. If zero
		/// bytes are returned no packet will be sent.
		virtual bdUInt getDataToSend(bdUByte8 *const buffer,
									   const bdUInt bufferSize) = 0;

	protected:

		bdCommonAddrRef m_addr;

		bdAddrHandleRef m_addrHandle;

		/// A container of all the registered connection listeners.
		bdFastArray<bdConnectionListener*> m_listeners;

		/// The data rate at which this connection can send data
		bdUInt m_maxTransmissionRate;

		// The current status
		Status m_status;
};

#endif // BD_CONNECTION_H
