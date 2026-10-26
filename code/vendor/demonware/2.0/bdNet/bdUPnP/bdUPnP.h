// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 PS3 PSP-INFRA UNIX OSX IPHONE WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: UPnP Nat traversal module

#ifndef BD_UPNP_H
#define BD_UPNP_H

#include <bdCore/bdCore.h>
#include <bdNet/bdUPnP/bdUPnPDevice.h>

/// DemonWare's UPnP Nat traversal module.
/// The sole purpose of this class is setup NAT mappings on UPnP capable
/// devices, and subsequently verify that the mapping is useful.
class bdUPnP
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// The current internal state of the UPnP code. 
		enum bdUPnPState
		{
			BD_UPNP_UNINITIALISED = 0, 
			BD_UPNP_INITIALISED,
			BD_UPNP_DISCOVERING_DEVICES,
			BD_UPNP_DISCOVERED_DEVICE,
			BD_UPNP_CONFIGURING_DEVICE,
			BD_UPNP_SHUTTING_DOWN,
			BD_UPNP_FINISHED,
			BD_UPNP_FAILED
		};

		/// Default constructor. 
		bdUPnP();

		/// Default destructor.
		~bdUPnP();

		/// Initialize the class. Discovery of devices will begin with the 
		/// first pump. This class needs to be pumped at regular intervals 
		/// using pump(). getStatus() should be checked regularly to detect 
		/// when device setup has completed.
		/// \param port [in] The port for which the mapping is to be setup.
		/// \param localAddrs [in] An array of local address(es)
		/// \param config [in] Optional configuration info for the class.
		/// \return 
		///  - True, if the class was successfully initialized
		///  - False otherwise.
		/// \note This function can only be called again after a call to 
		///       startShutdown has reverted the class to a 
		///       BD_UPNP_UNINITIALISED state
		bdBool init(bdPort port,
					bdArray<bdInetAddr> *localAddrs,
					bdUPnPConfig config = bdUPnPConfig());
	
		/// Pump the class.
		/// This allows the class to continue any pending asynchronous tasks.
		/// This should be called at regular intervals while the class is
		/// running.
		/// \note It is only necessary to call this function when the UPnP code
		///		  is not in the finished, failed or uninitialized states
		void pump();

		bdBool isRunning();

		/// Start the shutdown of the UPnP class.
		/// Depending on the shutdown type specified this call may revert the 
		/// class to the uninitialized state immediately, or it may perform a 
		/// more elegant shutdown involving removing mappings that were created.
		/// \param shutdownType [in] Optional specifier to indicate shutdown 
		///                          urgency. When called with 
		///                          BD_UPNP_SHUTDOWN_IMMEDIATE it is safe to 
		///                          immediately call init() again or destroy 
		///                          the class without leaking memory.
		/// \return 
		///  - True, if the shutdown was started without any errors.
		///  - False otherwise.
		/// \note It is not necessary to call this class more than once, unless
		///       a different shutdown strategy is to be employed.
		/// \note This class does not have any timeout on shutdown and it is the calling
		///       code's responsibility to prevent it from running for too long.
		/// \note If performing an elegant shutdown, the pump must be called until the 
		///       upnp status becomes uninitialized.
		void startShutdown(const bdUPnPDevice::bdUPnPDeviceShutdownType shutdownType = bdUPnPDevice::BD_UPNP_DEVICE_SHUTDOWN_ELEGANT);

		bdUPnPState getState() const;

		bdUPnPDevice::bdUPnPPortStatus getPortStatus() const;

		bdBool getExternalAddr(bdAddr& externalAddr ) const;

	protected:

		bdBool setupDiscoverySocket();

		bdBool startDiscovery();

		bdBool checkForDiscoveredDevices( bdUPnPDevice& discoveredDevice );

		bdBool handleDiscoveryResponse( bdUPnPDevice& discoveredDevice );

		bdBool extractDeviceInfo( bdAddr& deviceAddr, bdByte8** fetchLoc, bdUInt& fetchLen );

	protected:

		/// Current status of the UPnP code.
		bdUPnPState m_state;

		/// The port we wish to create a mapping for.
		bdPort m_gamePort;

		/// The local addresses of the instance we want to create a mapping for.
		/// This is needed to detect collisions with other instances that may 
		/// also be using the NAT.
		bdArray<bdInetAddr> *m_localAddrs;

		/// Configuration info on how the UPnP code should behave. 
		bdUPnPConfig m_config;

		/// UDP socket used for UPnP NAT discovery
		bdSocket m_discoverySocket;

		bdByte8 m_readBuffer[BD_MAX_UPNP_DISCOVERY_REPLY + 1];

		bdUInt m_bytesReceived;

		bdUPnPDevice m_device;
		
		bdUPnPDevice m_cachedDevice;

		/// Timer to track how long discovery has been running.
		bdStopwatch m_discoveryTimer;

		bdUInt m_numDiscoveryAttempts;

		bdBool m_shutdownRequested;
};
#endif


