// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_SOCKET_H
#define BD_SOCKET_H

#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdSocket/bdPort.h>
#include <bdCore/bdSocket/bdAddr.h>
#include <bdCore/bdSocket/bdSocketConfig.h>

#include <bdPlatform/bdPlatformSocket/bdPlatformSocket.h>

class bdSocket
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS;

		/// Default constructor
		bdSocket();

		/// Virtual destructor to let implementations clean up after themselves.
		virtual ~bdSocket();

		/// Creates a socket.
		/// \param blocking[in] Flag to indicate if socket should block.
		/// \param broadcast[in] Flag to indicate if socket is a broadcast socket.
		/// \param exclusiveAccess[in] Flag to indicate if exclusive access is required.
		/// \return
		///  - True : The socket was created successfully.
		///	 - False: An error occurred.
		virtual bdBool create(	const bdBool blocking ,
								const bdBool broadcast = true);

		/// Bind to port (any local address).
		/// This will call bind with port specified and IP
		/// address of "0.0.0.0" meaning bind to any local
		/// IP address.
		///
		/// \todo(psp-adhoc): This function does not work on PSP Adhoc.
		/// \param port[in] The port to bind to.
		/// \return A bdSocketStatusCode.
		virtual bdSocketStatusCode bind(const bdPort port);

		/// Bind to local address and port.
		/// \param addr[in] The IP address and port to bind to.
		/// \return A bdSocketStatusCode.
		virtual bdSocketStatusCode bind(const bdAddr &addr);

		/// Send data to addr.
		/// \param addr[in]		The address and port to send the data to.
		/// \param data[in]		A pointer to the data.
		/// \param length[in]	The length of the data.
		/// \return An signed integer representing:
		///  - If positive, the amount of amount of data sent.
		///  - If negative then an error from bdSocketStatusCode.
		virtual bdInt	sendTo(const bdAddr &addr,
								const void *data,
								const bdUInt length);

		/// Receive data.
		/// Make sure size is greater than the max packet size or data will be
		/// lost.
		/// \param addr[out]	The address of the sender.
		/// \param data[out]	A pointer to a buffer into which the data read
		///						from the socket will be read.
		/// \param size[out]
		/// \return A signed integer representing:
		///  - If positive, the amount of amount of data read into buffer.
		///  - If negative then an error from bdSocketStatusCode.
		virtual bdInt	receiveFrom(bdAddr &addr,
									void *data,
									const bdUInt size);

		/// Close the socket.
		/// \return
		///  - True if successful.
		///  - False otherwise.
		virtual bdBool close();

		bdInt getHandle() const;

		void setExclusiveAccess();

	protected:

		/// Internal handle to the socket
		bdInt m_handle;

};
#endif // BD_SOCKET_H
