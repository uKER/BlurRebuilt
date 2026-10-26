// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON WIN32 PS2 UNIX OSX PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_QOS_PROBE_LISTENER_H
#define BD_QOS_PROBE_LISTENER_H

#include <bdCore/bdCore.h>

/// Structure containing information about a successful QoS probe.
class bdQoSProbeInfo
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS;

		/// Common address of the host that was probed.
		bdCommonAddrRef m_addr;
		/// Actual address that was used for communication, acquired through 
		/// NAT traversal.
		bdAddr			m_realAddr;
		/// Average latency experienced.
		/// \note (XBox): On XBox, corresponds to median RTT.
		bdFloat32		m_latency;
		/// Pointer to the data returned in the probe. Can be NULL.
		/// Client code must not store a pointer to this data.
		bdUByte8*		m_data;
		/// Number of bytes in the data.
		bdUInt			m_dataSize;
		/// True, if the remote host is in the disabled state.
		bdBool			m_disabled;

		/// XBox only: estimated downstream bandwidth to the host.
		bdUInt			m_bandwidthDown;
		/// XBox only: estimated upstream bandwidth to the host.
		bdUInt			m_bandwidthUp;
};

/// Listener for QoS probe events.
class bdQoSProbeListener
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdQoSProbeListener();

		virtual ~bdQoSProbeListener();

		/// Called on successful completion of a single QoS probe.
		/// \param info[in] Data gained from the probe.
		virtual void onQoSProbeSuccess(const bdQoSProbeInfo &info) = 0;

		/// Called on failure of a single QoS probe to complete.
		/// \param addr[in] The original address supplied to probe.
		virtual void onQoSProbeFail(bdCommonAddrRef addr) = 0;

};

#endif //BD_QOS_PROBE_LISTENER_H
