// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Queue for sending reliable mesages.

#ifndef BD_RELIABLE_SEND_WINDOW_H
#define BD_RELIABLE_SEND_WINDOW_H

#include <bdCore/bdCore.h>
#include <bdConnection/bdPacket/bdPacket.h>
#include <bdConnection/bdConnectionConfig.h>
#include <bdConnection/bdPacket/bdDataChunk.h>

BD_REFERENCE(bdSAckChunk);

/// Implements a message queue for outgoing reliable messages.
class bdReliableSendWindow
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdReliableSendWindow();

		/// Destructor.
		~bdReliableSendWindow();

		/// Set the timeout period for this window.
		/// This is the length of time after which a message shall be
		/// considered undelivered and retransmission attempted.
		/// \param secs[in] The timeout period, in seconds.
		void setTimeoutPeriod(const bdFloat32 secs);

		/// Accessor function for the timeout period for this window.
		/// This is the length of time after which a message shall be
		/// considered undelivered and retransmission attempted.
		/// \return The timeout period, in seconds, of this window.
		bdFloat32 getTimeoutPeriod() const;

		/// Insert a message into the queue.
		/// \param chunk[in] A reference to the message to be inserted.
		/// \return
		///  - True if the message could be inserted
		///  - False otherwise.
		bdBool add(bdDataChunkRef chunk);

		/// Retrieves the next message to be sent.
		/// \param packet[out] Will be set the the sequence number of
		///	the message to send.
		/// \return
		///  - The message to send if one exists.
		///  - BD_NULL if no such message exists.
		void getDataToSend(bdPacket &packet);

		/// Acknowledge a frame.
		/// \param chunk[in] The sequence number of message to be
		///	acknowledged.
		/// \param rtt[out]	If the acknowledgement was successful \a rrt
		///	will be set to the round trip time of the acknowledged message
		/// (in seconds). Otherwise it is left unchanged.
		/// \return
		/// - True, if the acknowledgement was successful.
		/// - False, otherwise.
		bdBool handleAck(bdSAckChunkRef chunk, bdFloat32 &rtt);

		bdBool isEmpty() const;

	protected:

		class bdMessageFrame
		{
			public:

				BD_DECLARE_NEW_AND_DELETE_OPERATORS

				bdMessageFrame()
				: m_sendCount(0),
 				  m_missingCount(0),
				  m_gapAcked(false)
				{
				}

				bdMessageFrame(bdDataChunkRef chunk)
				: m_chunk(chunk),
				  m_sendCount(0),
				  m_missingCount(0),
				  m_gapAcked(false)
				{
				}

				~bdMessageFrame()
				{
				}

				bdDataChunkRef m_chunk;
				bdStopwatch m_timer;
				bdUByte8 m_sendCount;
				bdUByte8 m_missingCount;
				bdBool m_gapAcked;
		};

		enum bdCongestionWindowDecreaseReason
		{
			BD_CWDR_PACKET_LOSS_DETECTED,
			BD_CWDR_RESEND_TIMER_EXPIRED,
			BD_CWDR_INACTIVE
		};

	protected:

		void increaseCongestionWindow(const bdUWord bytesAcked);

		void decreaseCongestionWindow(const bdCongestionWindowDecreaseReason reason);

	protected:

		/// Sequence number of the last acknowledged message.
		bdSequenceNumber m_lastAcked;

		/// One beyond the index of the last message added to the window.
		bdSequenceNumber m_nextFree;

		/// The timeout period after which a messages gets resent. (RTO)
		bdFloat32 m_timeoutPeriod;

		bdUByte8 m_retransmitCountThreshold;

		/// The message frames.
		/// Namely all the messages that are currently in the window.
		bdMessageFrame m_frame[BD_MAX_WINDOW_SIZE];

		/// This value represents the dedicated buffer space, in number of
		/// bytes, the remote side has reserved in association with
		/// this window.
		bdWord m_remoteReceiveWindowCredit;

		/// The amount of data in bytes sent but not acked.
		bdWord m_flightSize;

		bdWord m_partialBytesAcked;

		//Slow Start Threshold (ssthresh): An SCTP variable.  This is the
		//threshold the endpoint will use to determine whether to
		//perform slow start, or congestion avoidance on a particular
		//destination transport address.  Ssthresh is in number of bytes.
		bdWord m_slowStartThresh;

		//Congestion Window (cwnd): An SCTP variable that limits the data
		//in number of bytes, a sender can send to a particular destination
		//transport address, before receiving an acknowledgement.
		bdWord m_congestionWindow;

		bdStopwatch m_lastSent;
};

#endif // BD_RELIABLE_SEND_WINDOW_H
