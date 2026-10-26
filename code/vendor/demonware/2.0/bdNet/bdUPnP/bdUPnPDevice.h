// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 PS3 PSP-INFRA UNIX OSX IPHONE WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Internal representation of a UPnP capable NAT

#ifndef BD_UPNP_DEVICE_H
#define BD_UPNP_DEVICE_H

#include <bdCore/bdCore.h>
#include <bdCore/bdSocket/bdStreamSocket.h>
#include <bdNet/bdUPnP/bdUPnPConfig.h>
#include <bdNet/bdUPnP/bdUPnPCommon.h>
#include <bdCore/bdString/bdString.h>

/// This is a class to represent a UPnP device on a users LAN.
/// It will go through a series of steps based on the startup type
/// and results of its querys.
///
///                     +-------------+
///                     | Determine   |
///                     | External IP |
///                     +-------------+
///                            |
///                            |
///                            V
///                         /-------\`        
///                      Y /Startmode\ N    
///     Finished  <-------/    ==     \---------|    
///        +              \ Get Extern/         |
///   Port Not Mapped      \ IP Only /          |
///                         \-------/           |
///                                             |
///                                             |
///                                             V
///                                        +-------------+
///                                        | Get Port    |
///                                        | Mapping For |
///                                        | Game Port   |
///                                        +-------------+
///                                               |
///                                               V
///                                            /------\`      
///                           +--------+   N  /  Does  \`     
///           Finished  <-----| Create |<--- / The Port \`    
///               +           |  Port  |     \ Mapping  /
///          Port Mapped      | Mapping|      \ Exist? /
///                           +--------+       \------/
///                                               | Y 
///                                               |
///                                               V
///                                            /-------\`
///                                         N /   Is    \`
///                           Finished <-----/ The Port  \`
///                               +          \  Mapped   /
///                        Collision Found    \ By Me?  /
///                                            \-------/
///                                                | Y
///                                                |
///                                                V
///                                            Finished
///                                                +
///                                           Port Mapped
///
///
/// Shutting down elegantly means the code will attempt to remove exisiting mappings,
/// while immediate shutdown will revert the class to the uninitialized state as 
/// quickly as possible.
///

class bdUPnPDevice
{

	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Representation of state
		enum bdUPnPDeviceState
		{
			BD_UPNP_DEVICE_UNINITIALISED,			// Default State
			BD_UPNP_DEVICE_INITIALISED,
			BD_UPNP_DEVICE_SENDING_DESCRIPTION_REQ, 
			BD_UPNP_DEVICE_GETTING_DESCRIPTION,     // Retrieving Device Description
			BD_UPNP_DEVICE_GOT_DESCRIPTION,
			BD_UPNP_DEVICE_SENDING_IP_REQ,
			BD_UPNP_DEVICE_DISCOVERING_IP,			// Discovering external IP
			BD_UPNP_DEVICE_DISCOVERED_IP,
			BD_UPNP_DEVICE_SENDING_PORT_QUERY_REQ,
			BD_UPNP_DEVICE_DISCOVERING_PORT_MAPPING,
			BD_UPNP_DEVICE_DISCOVERED_PORT_MAPPING,
			BD_UPNP_DEVICE_SENDING_MAPPING_REQ,
			BD_UPNP_DEVICE_MAPPING_PORT,
			BD_UPNP_DEVICE_MAPPED_PORT,
			BD_UPNP_DEVICE_SENDING_UNMAPPING_REQ,
			BD_UPNP_DEVICE_UNMAPPING_PORT,         // Initial state of elegant shutdown 
			BD_UPNP_DEVICE_UNMAPPED_PORT,
			BD_UPNP_DEVICE_SHUTTING_DOWN,
			BD_UPNP_DEVICE_FINISHED,
			BD_UPNP_DEVICE_FAILED
		};

		/// Represents the strategy to adopt when shutting down
		enum bdUPnPDeviceShutdownType
		{
			BD_UPNP_DEVICE_SHUTDOWN_ELEGANT,
			BD_UPNP_DEVICE_SHUTDOWN_IMMEDIATE
		};

		/// The status of the port
		enum bdUPnPPortStatus
		{
			BD_UPNP_PORT_STATUS_UNKNOWN,
			BD_UPNP_PORT_COLLISION_FOUND,
			BD_UPNP_PORT_NOT_MAPPED,
			BD_UPNP_PORT_MAPPED
		};

		/// Default constructor
		bdUPnPDevice();

		/// Destructor.
		~bdUPnPDevice();

		/// Initialize this class.
		/// \param config				[in] Configuration information.
		/// \param localAddrs			[in] The local address(es) of class.
		/// \param deviceAddress		[in] The address of the UPnP device.
		/// \param fetchLocation		[in] The URL segment indicating where the
		///									 device should be retrieved from.
		/// \param fetchLocationLength	[in] Length of the fetchLocation string.
		/// \param port					[in] The port to be mapped.
		/// \return
		///  - True, if the class was successfully initialized.
		///  - False otherwise.
		bdBool init(const bdUPnPConfig config ,
					bdArray<bdInetAddr> *localAddrs,
					const bdAddr deviceAddress,
					const bdByte8 *const fetchLocation,
					const bdUInt fetchLocationLength,
					const bdPort port);

		/// Pump the active task. if any.
		void pump();

		/// Shutdown the class.
		/// This call may be asymmetric, depending on the shutdownType
		/// specified, as removing mappings takes a non trivial amount of time
		/// to complete. The progress can be checked by polling getStatus().
		/// During shutdown pump() should be invoked at regular intervals to
		/// drive the code. When complete the class will be in a state where
		/// init can be called again. The call will also ensure that all
		/// allocations are deallocated.
		/// \param shutdownType [in] Optional configuration info to dictate the
		///                          shutdown strategy.
		void startShutdown(const bdUPnPDeviceShutdownType shutdownType = BD_UPNP_DEVICE_SHUTDOWN_ELEGANT);

		/// Accessor for the device address.
		/// This is used as a unique identifier for the device!
		/// \return The device address.
		/// \todo: Look into possibility of INet part of the addr changing during setup.
		/// Since we use the INet part as a UID this would be bad.
		bdAddr getDeviceAddr() const;

		/// Accessor for the external address of the device.
		/// Check that the address is valid before using since we may
		/// have faild the IP discovery phase.
		/// \return the external address of the device
		bdAddr getExternalAddr() const;

		/// Accessor for the current status of the port.
		/// \return The current status of the port.
		bdUPnPPortStatus getPortStatus() const;

		/// Accessor for the current status of the port.
		/// \return The current status of the port.
		bdUPnPDeviceState getDeviceState() const;

	protected:
		
		bdBool connectToDevice();
		
		bdBool setLocalConnectedAddr();

		void setupReceive();

		bdBool pumpSendRequest( bdBool& isConnected );

		bdBool pumpReceive();

		bdBool confirmHttpSuccess();

		bdBool checkForFinishedSoapResponse();

		void genUPnPCommand(	const bdByte8 *const command, 
								const bdByte8 *const arguments );

		bdBool extractTag( bdByte8* tag, bdByte8* buffer, bdByte8** value, bdUInt& valueLen);

		void prepareGetDescriptionRequest();

		bdBool parseDescriptionResponse();

		bdBool extractURLBase( bdByte8** baseLoc, bdUInt& baseLen );
		
		bdBool extractServiceType( bdBool& serviceType );

		bdBool extractControlURL( bdByte8** controlLoc, bdUInt& controlLen );

		void prepareGetExternalIPRequest();

		bdBool parseGetExternalIPResponse();

		bdBool extractExternalAddress( bdInetAddr& externalAddress );

		void prepareGetMappingsRequest();

		bdBool parseGetMappingsResponse(bdBool& mappingExists, bdBool& mappingIsMine );

		bdBool extractMappingOwner( bdInetAddr& owner );

		void prepareCreatePortMappingRequest();

		void preparePortUnMappingRequest();

	protected:

		/// State of this device.
		bdUPnPDeviceState m_state;

		bdBool m_shutdownRequested;

		bdUPnPDeviceShutdownType m_shutdownType;

		/// The status of the port
		bdUPnPPortStatus m_portStatus;

		/// Address of the device.
		bdAddr m_deviceAddr; 

		/// The local addresses of the instance that wishes to create the mapping.
		bdArray<bdInetAddr> *m_localAddrs;

		/// Address associated with the TCP connection to this device.
		/// this address is required to generate port mapping requests
		/// on systems that are capable of multiple NICs with multiple IPs
		/// For the sake of simplicity this is used on all platforms.
		bdInetAddr m_localConnectedAddr;

		/// TCP socket connection to Router
		/// This connection will transition from the description URL to the
		/// control URL as UPnP traversal progresses.
		bdStreamSocket m_streamSocket;

		/// Indicates whether forwarding mechanism is IP or PPP
		bdBool m_isIp;

		/// The configuration data.
		bdUPnPConfig m_config;

		/// The port to find/create the mapping for.
		bdPort m_gamePort;

		/// External IP of the device
		bdInetAddr m_externalDeviceAddr;

		/// The string used to send http commands to the UPnP device.
		bdByte8 m_requestBuffer[BD_UPNP_MAX_DEVICE_REQUEST_SIZE];

		bdUInt m_requestSize;

		/// buffer to hold TCP data returned from UPnP NAT
		bdByte8 m_readBuffer[BD_UPNP_MAX_DEVICE_REPLY_SIZE];
		
		bdUInt m_bytesReceived;

		bdByte8 m_fetchLocation[BD_UPNP_FETCH_LOCATION_SIZE];

		/// The string to use when sending control requests to the device.
		bdByte8 m_controlURL[BD_UPNP_CONTROL_URL_SIZE];

		bdStopwatch m_connectTimer; 

		/// timer to track response wait.
		bdStopwatch m_responseTimer; 
};

#endif //#ifndef BD_UPNP_DEVICE_H
