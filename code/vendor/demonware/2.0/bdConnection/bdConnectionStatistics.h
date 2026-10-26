// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class to collate and represent statistics about a network
//			connection

#ifndef BD_CONNECTION_STATS_H
#define BD_CONNECTION_STATS_H

#include <bdCore/bdCore.h>

/// A class to collate and represent statistics about a network
///	connection.
class bdConnectionStatistics
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default Constructor.
		bdConnectionStatistics();

		// Reset all previously recorded statistics.
		void reset();

		/// Process information about the number of bytes sent.
		/// \param bytes[in] The number of bytes to add to the sent total.
		void addBytesSent(const bdUInt bytes);

		/// Process information about the number of bytes received.
		/// \param bytes[in] The number of bytes to add to the received total.
		void addBytesRecv(const bdUInt bytes);

		/// Process information about the size of a sent packet.
		/// \param bytes[in] The size of the packet.
		void addPacketSizeSent(const bdUInt bytes);

		/// Process information about the size of a received packet.
		/// \param bytes[in] The size of the packet.
		void addPacketSizeRecv(const bdUInt bytes);

		/// Process information about the number of sent packets.
		/// \param numPackets[in] The number of packets sent.
		void addPacketsSent(const bdUInt numPackets);

		/// Process information about the number of received packets.
		/// \param numPackets[in] The number of packets received.
		void addPacketsRecv(const bdUInt numPackets);

		/// Process the last recorded value for the round trip time.
		/// \param time[in] The last round trip time.
		void setLastRTT(const bdFloat32 time);

		/// Accessor function for the number of bytes sent.
		/// \return The total number of bytes sent.
		bdUInt getBytesSent() const;

		/// Accessor function for the number of bytes sent per second.
		/// \return The number of bytes sent during the last second.
		bdUInt getBytesSentPerSecond() const;

		/// Accessor function for the number of bytes received.
		/// \return The total number of bytes received.
		bdUInt getBytesRecv() const;

		/// Accessor function for the number of packets sent.
		/// \return The total number of packets sent.
		bdUInt getPacketsSent() const;

		/// Accessor function for the number of packets received.
		/// \return The total number of packets received.
		bdUInt getPacketsRecv() const;

		/// Accessor function for the average round trip time.
		/// Note that the RTT value represents the RTT for the last
		/// ACKed reliable messages that was sent from this node.
		/// \return The average round trip time, in seconds.
		bdFloat32 getAvgRTT() const;

		void update(const bdFloat32 time);

	protected:

		/// Total bytes sent.
		bdUInt  m_bytesSent;

		/// Total bytes sent per second
		bdUInt m_bytesSentPerSecond;

		/// Total average bytes sent
		bdUInt m_avgBytesSent;

		/// Total last bytes sent
		bdUInt m_lastBytesSent;

		/// Total bytes received.
		bdUInt  m_bytesRecv;

		/// The size of the average bdNetPacket.
		bdUInt  m_avgPacketSentSize;

		/// The size of the average bdNetPacket.
		bdUInt  m_avgPacketRecvSize;

		/// The size of the max bdNetPacket sent.
		bdUInt  m_maxPacketSizeSent;
		/// The size of the min bdNetPacket sent.
		bdUInt  m_minPacketSizeSent;

		/// The size of the max bdNetPacket received.
		bdUInt  m_maxPacketSizeRecv;

		/// The size of the min bdNetPacket received.
		bdUInt  m_minPacketSizeRecv;

		/// Total number of bdNetPacket sent.
		bdUInt  m_packetsSent;
		/// Total number of bdNetPacket received.
		bdUInt  m_packetsRecv;

		// max/min/avg round trip times (RTT).
		/// Maximum round trip time.
		bdFloat32 m_maxRTT;
		/// Minimum round trip time.
		bdFloat32 m_minRTT;
		/// Average round trip time.
		bdFloat32 m_avgRTT;

};

#endif // BD_CONNECTION_STATS_H
