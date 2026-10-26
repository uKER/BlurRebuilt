// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: An OS independent wrapper around network calls.

#ifndef BD_PLATFORM_SOCKET_H
#define BD_PLATFORM_SOCKET_H

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>
#include <bdPlatform/bdPlatformSocket/bdInAddr.h>

/// The overhead of a UDP and IP headers in bytes.
#define BD_UDP_IP_OVERHEAD (28)

#define BD_INVALID_SOCKET_HANDLE (-1)

/// Status codes for socket calls.
enum bdSocketStatusCode
{
	/// worked.
	BD_NET_SUCCESS = 1,

	// Connection was closed gracefully
	BD_NET_CONNECTION_CLOSED = 0,

	/// unknown error.
	BD_NET_ERROR = -1,

	/// socket is nonblocking and call would
	/// make it block.
	BD_NET_WOULD_BLOCK = -2,

	/// some part of the network subsystem has failed.
	BD_NET_SUBSYTEM_ERROR = -3,

	/// for binding, the address is already in use.
	BD_NET_ADDRESS_IN_USE = -4,

	/// connection reset. Usually occurs when the remote socket.
	/// is not in the listening state
	BD_NET_CONNECTION_RESET = -5,

	/// the buffer passed was too small for the data.
	BD_NET_MSG_SIZE = -6,

	/// the address given is invalid.
	BD_NET_ADDRESS_INVALID = -7,

	/// A blocking call was canceled.
	BD_NET_BLOCKING_CALL_CANCELED = -8,

	/// The socket has yet to bind; therefore receiving won't work.
	BD_NET_NOT_BOUND = -9,

	/// The socket handle is invalid.
	BD_NET_INVALID_HANDLE = -10,

	/// The socket is not connected.
	BD_NET_NOT_CONNECTED = -11
};

/// A platform independent interface to standard socket routines.
class bdPlatformSocket
{
	public:

		/// Creates a unbound socket.
		/// \param broadcast[in]
		/// \param blocking [in] Optional param to indicate if the socket should 
		///						 be set to blocking or not.
		/// \return A signed integer representing:
		///  - If positive,
		///  - If negative then an error from
		static bdInt BD_CALL create(const bdBool blocking,
									const bdBool broadcast);

		/// Bind to local address and port.
		/// \param handle[in]
		/// \param addr[in] The IP address and port to bind to.
		/// \param port[in]
		/// \return A bdSocketStatusCode.
		static bdSocketStatusCode BD_CALL bind(bdInt &handle,
											   const bdInAddr addr,
											   const bdUInt16 port);

		/// Send data to addr.
		/// \param handle[in]
		/// \param addr[in]		The address and port to send the data to.
		/// \param port[in]
		/// \param data[in]		A pointer to the data.
		/// \param length[in]	The length of the data.
		/// \return A signed integer representing:
		///  - If positive, the amount of amount of data sent.
		///  - If negative then an error from bdSocketStatusCode.
		static bdInt BD_CALL sendTo(const bdInt handle,
									const bdInAddr addr,
									const bdUInt16 port,
									const void *data,
									const bdUInt length);

		/// Receive data.
		/// Make sure size is greater than the max packet size or data will be
		/// lost.
		/// \param handle[out]
		/// \param addr[out]	The address of the sender.
		/// \param port[out]
		/// \param data[out]	A pointer to a buffer into which the data read
		///						from the socket will be read.
		/// \param size[out]
		/// \return A signed integer representing:
		///  - If positive, the amount of amount of data read into buffer.
		///  - If negative then an error from bdSocketStatusCode.
		static bdInt BD_CALL receiveFrom(const bdInt handle,
										 bdInAddr &addr,
										 bdUInt16 &port,
										 void *data,
										 const bdUInt size);

		/// Close the socket.
		/// \return True if successful.
		static bdBool BD_CALL close(bdInt& handle);

		/// Unregisters a thread from using the socket API.
		/// This function needs to be called from within every thread that
		/// has made use of the socket API in order to free up resources
		/// This version unregisters the calling thread
		static void BD_CALL unregisterThread();

		/// Unregisters a thread from using the socket API.
		/// This function needs to be called from within every thread that
		/// has made use of the socket API in order to free up resources
		/// \param threadHandle : the handle of the thread that needs to be unregistered
		static void BD_CALL unregisterThread(bdInt threadHandle);

		/// Returns the total number of bytes sent across all sockets.
		static bdUInt64 BD_CALL getBytesSent();

		/// Returns the total number of bytes received across all sockets
		static bdUInt64 BD_CALL getBytesReceived();

		/// Returns the total number of packets sent across all sockets.
		static bdUInt64 BD_CALL getPacketsSent();

		/// Returns the total number of packets received across all sockets;
		static bdUInt64 BD_CALL getPacketsRecvd();

		/// Returns a handle that is used for subsequent DNS lookups (getHostByName
		/// and abortGetHostByName). On most platforms this is simply a thread handle,
		/// so this function should be called from the thread that is performing
		/// the name lookup.
		/// \return A handle for querying the Domain Name Server
		static bdDNSHandle getDNSHandle(void *buf = BD_NULL, bdInt bufLen = 0);

		/// Resolve a character string as a DNS domain name. This
		/// function will block for several seconds while the resolution
		/// is taking place.
		/// \param name[in] DNS hostname to resolve.
		/// \param addresses[out] A list of addresses of size numAdresses to which
		///		the host name resolves to
		/// \param numAdresses[in] Maximum number of adresses that can be stored for this
		///		host name.
		/// \param handle[in] A handle to be associated with this lookup.
		/// \return - The actual number of IP addresses that match the hostname
		/// Wii WARNING: Threading Nintendo socket library calls has been known to cause issues.
		/// It has caused DNS lookups to fail intermittently and has caused SoShutdown to 
		/// fail because it thinks it still has memory allocated. In particular, doing simultaneous
		/// DNS lookups has been particularily error prone.
		static bdUInt BD_CALL getHostByName(const bdNChar8* const name,
											bdInAddr *adresses,
											bdUInt numAdresses,
											const bdDNSHandle handle);

		/// Cancels a Domain Name Service lookup that is still in progress
		/// \param dnsHandle[in] The DNS handle associated with this lookup
		static void BD_CALL abortGetHostByName(bdDNSHandle dnsHandle);

		static void BD_CALL setExclusiveAccess(bdInt handle);

	private:

		/// Private constructor. There should be no need to
		/// new this class as everything is static.
		bdPlatformSocket();

		static bdUInt64 m_totalBytesSent;

		static bdUInt64 m_totalPacketsSent;

		static bdUInt64 m_totalBytesRecvd;

		static bdUInt64 m_totalPacketsRecvd;
};

#endif // BD_PLATFORM_SOCKET_H


