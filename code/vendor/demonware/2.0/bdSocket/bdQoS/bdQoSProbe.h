// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON WIN32 PS2 UNIX OSX PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_QOS_PROBE_H
#define BD_QOS_PROBE_H

#include <bdCore/bdCore.h>
#include <bdSocket/bdQoS/bdQoSRemoteAddr.h>

#if (!defined BD_PLATFORM_XBOX) && (!defined BD_PLATFORM_XENON)
#include <bdSocket/bdPacketInterceptor.h>
#include <bdSocket/bdNAT/bdNATTravListener.h>
#include <bdSocket/bdQoS/bdPacket/bdQoSReplyPacket.h>

class bdServiceBandwidthArbitrator;
class bdNATTravClient;
class bdQoSRequestPacket;

#endif

/// Default amount of bandwidth QoS uses, in bits per second.
#define BD_QOS_MAX_BANDWIDTH_DEFAULT (16384)

class bdQoSProbeListener;


/// A QoS prober class.
///
/// QoS is a facility for checking connectivity to hosts, obtaining
/// latest game information, and measuring latency. This class is the
/// primary interface to it.
///
/// On a server, \a listen() should be used to enable replying to probe
/// requests. The server will then respond to incoming QoS requests. Up-to-date
/// game data can be provided through \a setData().
///
/// When the server does not wish to answer QoS probes (for example, if
/// it is full), it should get disabled with \a disableListener(). In this
/// mode, no data will be transmitted in replies.
///
/// Clients can probe hosts with \a probe().
///
/// QoS also provides bandwidth management. \a setMaxBandwidth() can be used
/// to set maximum outgoing bandwidth used by QoS (including QoS-related
/// NAT traversal). In client-server games, amount of QoS bandwidth should be
/// inversely proportional to server utilization.
///
/// This class is instantiated by \a bdSocketRouter and should be
/// obtained from it. Only one instance of this class can exist
/// on a host at one time.
///
/// On XBox and XBox 360 this is simply an interface to XNetQos*.
class bdQoSProbe
#if (!defined BD_PLATFORM_XBOX) && (!defined BD_PLATFORM_XENON)
	: public bdNATTravListener,
	  public bdPacketInterceptor
#endif
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS;

		/// Defines the possible states of a bdNATTravClient.
		enum bdQoSProbeStatus
		{
			BD_QOS_PROBE_UNINITIALIZED,
			BD_QOS_PROBE_INITIALIZED,
			BD_QOS_PROBE_RUNNING
		};


		/// Changes the amount of outgoing bandwidth allocated to QoS.
		/// This sets the maximum amount of outgoing bandwidth QoS will use, in
		/// bits per second. It applies both to outgoing probes, and outgoing
		/// probe replies. It also limits outgoing NAT traversal initiated by
		/// QoS probes, but not incoming ones.
		/// \param maxBandwidth[in] Maximum bits per second to be used for QoS.
		void setMaxBandwidth(const bdUInt maxBandwidth);

		/// Initiate a single probe to a remote host.
		/// When probing more than one host, array version should be used instead.
		/// \param addr[in] Details of the remote host.
		/// \param listener[in] Listener for probe completion event.
		/// \result True, if the probe was successfully initiated.
		bdBool probe(bdQoSRemoteAddr &addr, bdQoSProbeListener *listener);

		/// Initiate multiple probes to remote hosts.  	 
		/// \param addrs[in] Array of remote host details. 	 
		/// \param listener[in] Listener for probe completion events. 	 
		/// \result True, if probes were successfully initiated. 	 
		bdBool probe(const bdArray<bdQoSRemoteAddr> &addrs, bdQoSProbeListener *listener);

		/// Interrupt all currently pending probes and free memory.
		void cancelProbes();

		/// Called every frame.
		void pump();

		/// Start listening for incoming probes.
		/// \param secid[in] Security ID this game instance uses to host. It should have
		///  been previously registered with \a bdKeyMap.
		/// \param data[in] Data block returned in probe replies. Can be \a NULL. This gets
		///  copied.
		/// \param dataSize[in] Number of bytes in the data block. Must be zero if \a data
		///  is \a NULL.
		/// \return True, if successful. False may indicate a data block that's too large,
		///  or a non-registered \a secid.
		bdBool listen(const bdSecurityID &secid, bdUByte8 *data, bdUInt dataSize);

		/// Change data returned in probe replies.
		/// \param data[in] Data block returned in probe replies. Can be \a NULL. This
		///  gets copied.
		/// \param dataSize[in] Number of bytes in the data block. Must be zero if \a data
		/// is \a NULL.
		/// \return True on success. False may indicate a data block that's too large.
		bdBool setData(bdUByte8 *data, bdUInt dataSize);

		/// Change listener's state to "disabled".
		/// Even in the disabled state, data will still be returned in QoS replies. To ensure
		/// minimum bandwidth usage, \a setData() should be used to remove it, or set a
		/// special short "disabled" data.
		void disableListener();

		/// Change listener's state to "enabled".
		void enableListener();

		/// Turn listener completely off.
		/// When the listener is off, it uses no memory and silently ignores all incoming
		/// QoS probes.
		void shutdownListener();

		/// \return True if there are currently probes in progress
		bdBool isProbing() const;

	protected:
		
		/// Bandwidth amount we were told to use. Value in bits.
		bdUInt m_maxBandwidth;

		/// Initiate a single probe to a remote host.
		/// When probing more than one host, array version should be
		/// used instead.
		/// \param addr[in] Address of the remote host to probe.
		/// \param id[in] Security ID the remote host is using.
		/// \param key[in] Security key the remote host is using.
		/// \param listener[in] Listener for probe completion event.
		bdBool probe(bdCommonAddrRef addr,
					 const bdSecurityID &id,
					 const bdSecurityKey &key,
					 bdQoSProbeListener *listener);

	public:
		
		/// Basic constructor.
		/// On non-Xbox platforms an instance of the class created with this 
		/// constructor cannot be used until init() has been called.
		bdQoSProbe();

#if (!defined BD_PLATFORM_XBOX) && (!defined BD_PLATFORM_XENON)
		/// Constructor.
		/// \param socket[in] Socket used for outgoing communication.
		/// \param natTrav[in] Pointer to the NAT Traversal client.
		/// \param bandArb[in] Pointer to the bandwidth arbitration object.
		bdQoSProbe(bdDefaultSocket *socket, bdNATTravClient *natTrav, bdServiceBandwidthArbitrator *bandArb);

		virtual ~bdQoSProbe();

		/// Initialize this class.
		/// This function must be called if the basic constructor was used to 
		/// create the class.
		/// \param socket[in] The socket used for sending packets.
		/// \param natTrav[in] The NAT trav client to use for NAT traversal.
		/// \param bandArb[in] A pointer to an instance of bandwidth 
		///					   arbitrator, shared between connection services 
		///					   (such as QoS). Pointer must be valid for the
		///					   entire lifetime of this class instance.
		/// \return
		///   - True if the class was initialized correctly.
		///   - False otherwise.
		bdBool init(bdDefaultSocket *socket, 
					bdNATTravClient *natTrav, 
					bdServiceBandwidthArbitrator *bandArb);

		/// Stops all activity in this class.
		/// This will reset the class to the uninitialized state.
		/// After this function has been called, it is safe to call init again.
		/// \return 
		///   - True if the quit was error free.
		///   - False otherwise.
		bdBool quit();

		/// Inherited from \a bdNATTravListener.
		virtual void onNATAddrDiscovery(bdCommonAddrRef remote, const bdAddr &realAddr);

		/// Inherited from \a bdNATTravListener.
		virtual void onNATAddrDiscoveryFailed(bdCommonAddrRef remote);

		/// Inherited from \a bdPacketInterceptor.
		virtual bdBool acceptPacket(bdDefaultSocket *socket, bdAddr addr, void *data, const bdUInt size, const bdUByte8 type);

	protected:

		bdBool handleRequest(bdQoSRequestPacket &packet, bdAddr &addr);
		bdBool handleReply(bdQoSReplyPacket &packet, bdAddr &addr);
		bdUInt shrinkSecId(const bdSecurityID &id) const;

		bdDefaultSocket *m_socket;
		bdNATTravClient *m_natTrav;
		bdServiceBandwidthArbitrator *m_bandArb;
		bdUInt32 m_lastProbeId;

		bdStopwatch m_probingTimer;

		bdUInt32 m_secid;

		bdQoSReplyPacket m_replyData;

		enum bdQoSListenerState
		{
			BD_QOS_OFF,
			BD_QOS_DISABLED,
			BD_QOS_ENABLED
		};

		bdQoSListenerState m_listenState;

		bdQoSProbeStatus m_status;

public:

		struct bdQoSProbeEntryWrapper
		{
			BD_DECLARE_NEW_AND_DELETE_OPERATORS;

			bdQoSProbeEntryWrapper()
				: m_listener(BD_NULL),
				  m_retries(0)
			{
				m_lastTry.reset();
			}

			bdCommonAddrRef m_addr;
			bdSecurityID m_id;
			bdSecurityKey m_key;
			bdQoSProbeListener *m_listener;
			bdAddr m_realAddr;
			bdUInt m_retries;
			bdStopwatch m_lastTry;
		};

		/// Container to keep track of all as yet unresolved probes
		bdHashMap<bdCommonAddrRef, bdArray<bdQoSProbeEntryWrapper> > m_probesResolving;

		/// Container to keep track of all active probes.
		bdHashMap<bdUInt32, bdQoSProbeEntryWrapper> m_probesProbbing;

#else // Xboxes

	protected:

		struct bdQoSProbeEntryWrapper
		{
			BD_DECLARE_NEW_AND_DELETE_OPERATORS;

			bdQoSProbeEntryWrapper(const bdArray<bdCommonAddrRef> &addrs,
								   XNQOS *xnqos,
								   bdQoSProbeListener *listener)
				: m_addrs(addrs),
				  m_xnqos(xnqos),
				  m_listener(listener),
				  m_firedEvents(0)
			{
			}

			bdArray<bdCommonAddrRef> m_addrs;
			XNQOS *m_xnqos;

			bdQoSProbeListener *m_listener;

			bdUInt m_firedEvents;

			protected:
			
				// Need assignment operator to make the compiler warning go away.
				bdQoSProbeEntryWrapper& operator=(const bdQoSProbeEntryWrapper&)
				{
					BD_ASSERT(false, "Assignment operator not implemented.");
				}
		};

		bdBool updateListenerBandwidth();

		bdLinkedList<bdQoSProbeEntryWrapper*> m_requests;
		bdSecurityID m_id;
		bdBool m_listenerSecIDInitialised;
#endif
};


#endif // BD_QOS_PROBE_H
