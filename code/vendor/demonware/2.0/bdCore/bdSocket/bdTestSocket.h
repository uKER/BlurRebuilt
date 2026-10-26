// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A wrapper around a socket to add simple lag, jitter, and dropping
//			for testing purposes.

#ifndef BD_TEST_SOCKET_H
#define BD_TEST_SOCKET_H

#include <bdCore/bdSocket/bdSocket.h>

#include <bdCore/bdContainers/bdByteBuffer.h>
#include <bdCore/bdContainers/bdPriorityHeap.h>
#include <bdCore/bdUtilities/bdRandom.h>
#include <bdCore/bdTiming/bdStopwatch.h>

/// A test wrapper around sockets. It allows simple testing of
/// real world (internet) situations such as packet lag,
/// dropping, etc.
class bdTestSocket : public bdSocket
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor
		bdTestSocket();

		/// Virtual destructor:
		/// Closes the socket then deletes the reference passed in through
		/// the constructor.
		virtual ~bdTestSocket();

		/// Set the seed for the random number generator which is used for the dropping
		/// and lagging of packets.
		/// \param seed[in] A 32 bit integer with which to seed the pseudo
		///	random number generator
		void setSeed(const bdUInt seed);

		/// Set the probability of packet drop.
		/// \param drop[in] A value between 0 and 1 indicating
		/// the percentage chance of a drop.
		void setDropFactor(bdFloat32 drop);

		/// Set the lag of a packet.
		/// The actual lag is worked out by using the average and
		/// a percentage chance that it will drop or climb up to the max
		/// or min specified.
		/// All packets are lagged when being sent and when received.
		/// So the actual round-trip time (RTT) for a packet between 2 hosts
		/// A and B is:
		/// A(send lag) + B(receive lag) + B(send lag) + A(receive lag).
		/// So if A has an average lag of 150ms and B has an average lag
		/// of 100 ms then the lag is:
		/// 150 + 100 + 100 + 150 = 500ms (which is a high RTT)
		/// \param min[in] The minimum lag. E.g. 0.1f for 100ms
		/// \param max[in] The maximum lag. E.g. 0.3f for 300ms
		/// \param avg[in] The average lag. E.g. 0.15f for 150ms
		void setLag(bdFloat32 min, bdFloat32 max, bdFloat32 avg);

		/// Adds lag to packets then sends them to m_socket->sendto().
		/// \param addr[in]		The address to send the packet to.
		/// \param data[in]		A pointer to the data to be sent.
		/// \param length[in]	The length of the data to be sent.
		/// \return A bdNetErrorCode.
		virtual bdInt	sendTo(const bdAddr &addr,
								const void *data,
								const bdUInt length);

		/// Adds lag to received packets.
		/// \param addr[out]	The address the data was received from.
		/// \param data[out]	The buffer to put the received data into.
		/// \param size[out]
		/// \return An integer representing:
		///  - If positive, the number of bytes read.
		///  - If negative, a bdNetErrorCode.
		virtual bdInt	receiveFrom(bdAddr &addr,
									void *data,
									const bdUInt size);

		/// Close the socket.
		/// This will ignore any specified packet delays by sending all packets
        /// immediately and then closing the socket.
		/// \return A bdNetErrorCode.
		virtual bdBool close();


	public:
		/// An internal class to represent a packet and the data associated
		/// with it.
		class PacketStore
		{
			public:

				BD_DECLARE_NEW_AND_DELETE_OPERATORS

				/// Default constructor:
				/// Creates a PacketStore with an invalid retval.
				inline PacketStore()
				{
					m_retval = 0xDEADBEEF;
				};

				/// Constructor taking parameters.
				/// \param addr[in]		The address to which the packet is to
				///						be sent.
				/// \param buffer[in]	A buffer containing the packet.
				/// \param waittime[in]	The time, until which, the packet
				///						should be delayed.
				/// \param retval[in]	The return value for this PacketStore.
				inline PacketStore(const bdAddr &addr,
					 			   bdByteBufferRef buffer,
								   bdFloat32 waittime,
								   bdInt retval = 0)
				: m_addr(addr),
				  m_buffer(buffer),
				  m_waittime(waittime),
				  m_retval(retval)
				{
					m_time.start();
				}

				/// Copy constructor.
				inline PacketStore(const PacketStore &other)
				{
					m_addr = other.m_addr;
					m_buffer = other.m_buffer;
					m_waittime = other.m_waittime;
					m_retval = other.m_retval;
					m_time = other.m_time;
				}

				/// Less than operator.
				/// Compares \a this to \a other.
				/// \param other[in]	The other PacketStore to compare.
				/// \return
				///  - True if \a this is less than \a other.
				///  - False if \a this is equal to or greater than \a other.
				inline bdBool operator < (const PacketStore & other) const
				{
					return timeUntilReady() < other.timeUntilReady();
				}

				/// Find how long until the packet will be ready to send.
				/// \return The length of time, in seconds, until the packet is
				///  due to be sent.
				inline bdFloat32 timeUntilReady() const
				{
					return m_waittime - m_time.getElapsedTimeInSeconds();
				}

				/// Find if the packet is ready to be sent.
				/// \return
				///  - True if the packet is ready to be sent.
				///  - False if the packet is not ready to be sent.
				inline bdBool isReady() const
				{
					return (timeUntilReady() <= 0);
				}

				/// The address to which the packet is to be sent.
				bdAddr m_addr;
				/// A buffer containing the packet.
				bdByteBufferRef m_buffer;
				/// A stopwatch to record the elapsed time.
				bdStopwatch m_time;
				/// The time, until which, the packet should be delayed.
				bdFloat32 m_waittime;
				/// The current return value of the packet.
				bdInt m_retval;
		};

	protected:

		/// Send all the packets that are ready to be sent.
		void flushOut();

		/// Get the next packet that is ready to be sent.
		/// \param addr[out]	The destination of the packet.
		/// \param data[out]	The buffer to which the packet data will be
		///						written.
		/// \return An integer to indicate what happened.
		bdInt	flushIn(bdAddr &addr,
						bdByteBufferRef &data);

		/// Set the internal lag based on various previously specified
		/// parameters.
		void calculateLag();


		/// The instance of bdRandom this object uses.
		bdRandom m_rand;
		/// A float between 0 & 1 to indicate what fraction of packets should
		/// be dropped.
		bdFloat32 m_drop;

		/// A container to store all the delayed outgoing packets.
		bdPriorityHeap<bdTestSocket::PacketStore> m_outDelayed;
		/// A container to store all the delayed incoming packets.
		bdPriorityHeap<bdTestSocket::PacketStore> m_inDelayed;

		/// The minimum allowable delay for a packet.
		bdFloat32 m_minDelay;
		/// The maximum allowable delay for a packet.
		bdFloat32 m_maxDelay;
		/// The average delay for a packet.
		bdFloat32 m_avgDelay;
		/// The amount of lag that this socket is simulating.
		bdFloat32 m_lag;
};

#endif // BD_TEST_SOCKET_H
