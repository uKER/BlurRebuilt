// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PSP-ADHOC PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE: To allow QoS and NAT classes to coordinate their bandwidth usage.

#ifndef BD_SERVICE_BANDWIDTH_ARBITRATOR_H
#define BD_SERVICE_BANDWIDTH_ARBITRATOR_H

#include <bdCore/bdCore.h>

/// A simple class for sharing bandwidth usage information between "services",
/// such as \a bdNATTravClient and \a bdQoSProbe.
///
/// This class is not meant to be used by anything else.
///
/// \a addSliceQuota() should be called frequently, which adds a certain number
/// of bytes to "available" amount. \a send() can then be called to indicate
/// that data was sent, and available amount should be decreased. This class
/// attempts to be smart and not increase available amount too much.
class bdServiceBandwidthArbitrator
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS;

		/// Default constructor.
		bdServiceBandwidthArbitrator();

		/// Resets all counters to zero.
		void reset();

		/// Adds more available bandwidth.
		/// Indicates that additional number of bytes may be transmitted in the future.
		/// It is safe to call this function often, as quota will not be increased
		/// indefinitely.
		/// \param bytes[in] Number of bytes that can be transmitted.
		void addSliceQuota(bdUInt bytes);

		/// Checks if a message of certain size can be transmitted without breaking
		/// bandwidth constraints.
		/// \param bytes[in] Size of the message, including IP and UDP headers.
		/// \return True, if transmission is allowed.
		bdBool allowedSend(bdUInt bytes) const;

		/// Records that a message of certain size was transmitted.
		/// \param bytes[in] Size of the message, including IP and UDP headers.
		void send(bdUInt bytes);

	protected:

		/// This is the minimum amount of bytes we can send right now. If a message
		/// that's larger than this number needs to be sent, it can become negative.
		bdInt m_allowedSend;
		/// True, if any messages were transmitted during this timeslice.
		bdBool m_sentThisSlice;
};

#endif //BD_SERVICE_BANDWIDTH_ARBITRATOR_H

