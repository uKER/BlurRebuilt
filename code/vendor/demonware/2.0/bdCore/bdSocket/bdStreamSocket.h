// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_STREAM_SOCKET_H
#define BD_STREAM_SOCKET_H

#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdSocket/bdPort.h>
#include <bdCore/bdSocket/bdAddr.h>

#include <bdCore/bdSocket/bdSocketConfig.h>
#include <bdPlatform/bdPlatformSocket/bdPlatformStreamSocket.h>


class bdStreamSocket
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS;

		bdStreamSocket();

		/// Virtual destructor to let implementations clean up after themselves.
		~bdStreamSocket();

		/// Creates a unbound socket.
		bdBool create(const bdBool blocking);

		/// Bind to port (any local address).
		/// This will call bind with port specified and IP
		/// address of "0.0.0.0" meaning bind to any local
		/// IP address.
		/// \param port[in] The port to bind to.
		/// \return A bdSocketStatusCode.
		bdSocketStatusCode bind(const bdPort port) const;

		/// Bind to local address and port.
		/// \param addr[in] The IP address and port to bind to.
		/// \return A bdSocketStatusCode.
		bdSocketStatusCode bind(const bdAddr addr) const;

		// connect to address.	
		bdSocketStatusCode connect(const bdAddr addr) const;

		bdBool isConnected() const;

		/// Send data.
		/// \param data[in]		A pointer to the data.
		/// \param length[in]	The length of the data.
		/// \return An signed integer representing:
		///  - If positive, the amount of amount of data sent.
		///  - If negative then an error from bdSocketStatusCode.
		bdInt send(const void *data,
				   const bdUInt length) const;

		/// Receive data.
		/// \param data[out]	A pointer to a buffer into which the data read
		///						from the socket will be read.
		/// \return An signed integer representing:
		///  - If positive, the amount of amount of data read into buffer.
		///  - If negative then an error from bdSocketStatusCode.
		bdInt recv(void *data,
				   const bdUInt size) const;

		/// Close the socket.
		void close();

#ifdef BD_PLATFORM_WIN32
		/// retrieves the local IP bound to this socket and returns true on success
		/// \param socketAddr[out] contains the retrieved address
		bdBool getSocketAddr(bdInAddr& socketAddr) const;
#endif

	protected:

		bdInt m_handle;
};
#endif // BD_STREAM_SOCKET_H
