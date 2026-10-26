// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A dummy wrapper around sockets.

#ifndef BD_DUMMY_SOCKET_H
#define BD_DUMMY_SOCKET_H

#include <bdCore/bdSocket/bdThreadedSocket.h>

/// A dummy wrapper around sockets. 
/// It pretends everything is working correctly, but in fact does nothing.
/// The purpose of this is to allow 
class bdDummySocket : public bdThreadedSocket
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor
		bdDummySocket();

		/// Virtual destructor:
		/// Closes the socket then deletes the reference passed in through
		/// the constructor.
		virtual ~bdDummySocket();

		/// Bind to port (any local address).
		/// Returns BD_NET_SUCCESS
		/// \param port[in] The port to bind to.
		/// \return A bdSocketStatusCode.
		virtual bdSocketStatusCode bind(const bdPort port);

		/// Bind to local address and port.
		/// Returns BD_NET_SUCCESS
		/// \param addr[in] The IP address and port to bind to.
		/// \return A bdSocketStatusCode.
		virtual bdSocketStatusCode bind(const bdAddr &addr);

		/// Pretends to send packets successfully.
		/// \param addr[in]		The address to send the packet to.
		/// \param data[in]		A pointer to the data to be sent.
		/// \param length[in]	The length of the data to be sent.
		/// \return length, indicating all data sent successfully.
		virtual bdInt	sendTo(const bdAddr &addr,
								const void *data,
								const bdUInt length);

		/// Pretends to receive packets successfully..
		/// \param addr[out]	The address the data was received from.
		/// \param data[out]	The buffer to put the received data into.
		/// \param size[out]
		/// \return An integer representing:
		///  - The number of bytes read (0).
		virtual bdInt	receiveFrom(bdAddr &addr,
									void *data,
									const bdUInt size);

		/// Close the socket.
		/// \return True.
		virtual bdBool close();


	public:

	protected:

};

#endif // BD_DUMMY_SOCKET_H
