// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_PACKET_INTERCEPTOR_H
#define BD_PACKET_INTERCEPTOR_H

#include <bdCore/bdCore.h>

/// Packet interceptor pattern implementation. Allows code to intercept and either 
/// accept or ignore packets. 
/// An instance of packet interceptor should be registered with \a bdSocketRouter
/// by calling \a registerInterceptor. Then for every packet coming from the
/// network that is not picked up by DTLS layer, or by any of the other interceptors
/// (such as \a bdNATTravClient), \a acceptPacket will be called, giving this
/// interceptor a chance to accept and interpret the packet.
///
/// Packet interceptor can insert data back into the network by calling sendTo
/// on the root socket (\a bdNetStartParams.m_socket).
/// Care should be taken when implementing the packet interceptor pattern that the 
/// added code does not interfere with other registered interceptors.
/// \todo:	Add logic to the interceptor & socket router so that packet type range 
///			clashes are detected. [ideally at compile time !]
class bdPacketInterceptor
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdPacketInterceptor();

		/// Default destructor.
		virtual ~bdPacketInterceptor();

		/// Incoming packet handler.
		/// Called for every packet coming from the network that is not handled
		/// by DTLS.
		///
		/// \param socket[in] The socket packet came in from.
		/// \param addr[in] Remote address that sent the packet.
		/// \param data[in] Buffer containing the data received in the packet.
		/// \param size[in] Size of the \a data buffer in bytes.
		/// \return Result of handling the packet.
		///  - True, if the packet was accepted. This stops the packet processing.
		///  - False, if this interceptor is unable to accept the packet (the packet
		///    is not destined for this interceptor). Packet is then passed on to
		///    other interceptors.
		virtual bdBool acceptPacket(bdDefaultSocket *socket, 
									bdAddr addr,
									void *data,
									const bdUInt size,
									const bdUByte8 type) = 0;

	protected:

};


#endif // BD_PACKET_INTERCEPTOR_H
