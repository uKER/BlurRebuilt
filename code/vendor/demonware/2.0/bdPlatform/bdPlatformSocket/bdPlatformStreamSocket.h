// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: An OS independent wrapper around network calls.

#ifndef BD_PLATFORM_STREAM_SOCKET_H
#define BD_PLATFORM_STREAM_SOCKET_H

#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>
#include <bdPlatform/bdPlatformSocket/bdInAddr.h>
#include <bdPlatform/bdPlatformSocket/bdPlatformSocket.h>

/// A platform independent interface to standard socket routines.
class bdPlatformStreamSocket
{
	public:

		/// Creates a unbound socket.
		static bdInt BD_CALL create(const bdBool blocking);

		/// Bind to local address and port.
		/// \param addr[in] The IP address and port to bind to.
		/// \return A bdSocketStatusCode.
		static bdSocketStatusCode BD_CALL bind(const bdInt handle,
											   const bdInAddr addr,
											   const bdUInt16 port);

		/// Connect to address and port.
		/// \param addr[in] The IP address and port to bind to.
		/// \return A bdSocketStatusCode.
		static bdSocketStatusCode BD_CALL connect(const bdInt handle,
												  const bdInAddr addr,
												  const bdUInt16 port);


		/// Accept an incoming connection.
		/// \param addr[in] The IP address and port of the new connection.
		/// \return A bdSocketStatusCode.
		static bdBool BD_CALL accept(const bdInt handle,
									 bdInt &newHandle,
									 bdInAddr &addr,
									 bdUInt16 &port);

		/// Send data.
		/// \param data[in]		A pointer to the data.
		/// \param length[in]	The length of the data.
		/// \return An signed integer representing:
		///  - If positive, the amount of amount of data sent.
		///  - If negative then an error from bdSocketStatusCode.
		static bdInt BD_CALL send(const bdInt handle,
								  const void *const data,
								  const bdUInt length);

		/// Receive data.
		/// Make sure size is greater than the max packet size or data will be
		/// lost.
		/// \param data[out]	A pointer to a buffer into which the data read
		///						from the socket will be read.
		/// \return An signed integer representing:
		///  - If positive, the amount of amount of data read into buffer.
		///  - If negative then an error from bdSocketStatusCode.
		static bdInt BD_CALL receive(const bdInt handle,
									 void *const data,
									 const bdUInt size);

		/// Close the socket.
		/// \return True if successful.
		static bdBool BD_CALL close(const bdInt handle);

		/// Unregisters a thread from using the socket API.
		/// This function needs to be called from within every thread that
		/// has made use of the socket API in order to free up resources
		/// \param threadHandle : the handle of the thread that needs to be unregistered
		static void BD_CALL unregisterThread();

		/// Returns the total number of bytes sent across all sockets.
		/// This count is application-level, so it doesn't include any
		/// protocol headers.
		static bdUInt64 BD_CALL getBytesSent();

		/// Returns the total number of bytes received across all sockets
		/// This count is application-level, so it doesn't include any
		/// protocol headers.
		static bdUInt64 BD_CALL getBytesReceived();

		/// Checks if there's an exception on a socket through select() call.
		/// This method can be used to check the status of a non-blocking
		/// socket that is in process of connecting, for example.
		/// \param handle[in] Socket handle to check.
		/// \return
		///  - True, if there is are exceptions on the socket.
		///  - False, otherwise.
		static bdBool BD_CALL checkSocketException(const bdInt handle);

		static bdBool BD_CALL isWritable(const bdInt handle);

#if defined(BD_PLATFORM_WIN32)
		/// retrieves the local IP bound to the given socket and returns true on success
		/// \param socketAddr[out] contains the retrieved address
		static bdBool BD_CALL getSocketAddr(const bdInt handle,
											bdInAddr& socketAddr);
#endif

	private:

		/// Private constructor. There should be no need to
		/// new this class as everything is static.
		bdPlatformStreamSocket();


	private:

		static bdUInt64 m_totalBytesSent;

		static bdUInt64 m_totalBytesRecvd;
};

#endif // BD_PLATFORM_STREAM_SOCKET_H


