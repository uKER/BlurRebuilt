// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE Manage & marshal the various connections, and their pumping.

#ifndef BD_CONNECTION_STORE_H
#define BD_CONNECTION_STORE_H

#include <bdCore/bdCore.h>
#include <bdConnection/bdDispatch/bdDispatcher.h>
#include <bdConnection/bdConnectionStoreConfig.h>
#include <bdSocket/bdAddrHandleRefWrapper.h>

// NOTE: Errors pointing to this line probably indicate that you need: 
//       #include <bdConnection/bdConnection.h>
//       in your *.cpp file
BD_REFERENCE(bdConnection);
BD_REFERENCE(bdInitChunk);

class bdConnectionListener;
class bdSocketRouter;
class bdAddressMap;


/// Stores the various connections.
/// This code is responsible for marshaling the various connections
/// and creating new outgoing & incoming ones.
/// It also manages sending, receiving & dispatching from the connections.
class bdConnectionStore
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Internal status of the connection store.
		enum bdConnectionStoreStatus
		{
			BD_CONNECTION_STORE_UNINITIALIZED,
			BD_CONNECTION_STORE_INITIALIZED,
			BD_CONNECTION_STORE_SHUTTING_DOWN,
			BD_CONNECTION_STORE_ERROR
		};

		/// Describes which approach the connection store will adopt when 
		/// shutting down.
		enum bdConnectionStoreShutdownType
		{
			BD_CONNECTION_STORE_SHUTDOWN_ELEGANT,
			BD_CONNECTION_STORE_SHUTDOWN_IMMEDIATE
		};

		/// Basic Constructor
		bdConnectionStore();

		/// Destructor
		~bdConnectionStore();

		/// Initialize the class.
		/// This function must be run before the class can be used.
		/// \param socket [in] The socket router instance to use.
		/// \param config [in] Optional configuration info for the class.
		/// \return 
		///  - True, if the class was successfully initialized.
		///  - False otherwise.
		bdBool init(bdSocketRouter *socket,
					const bdConnectionStoreConfig &config = bdConnectionStoreConfig());

		/// Call to flush all available messages to the network.
		/// This loops through all bdConnections and calls flush 
		/// on each of them in turn.
		/// \return 
		///  - True, if the flush was error free.
		///  - False otherwise.
		bdBool flushAll();

		/// Receive data from the network and pass it to the matching 
		/// connection.
		/// Where data is received that does not correspond to an
		/// existing connection this code will create a new connection add it
		///  to the connection store.
		/// This code will run until there is no more data to receive or until
		/// it has iterated N times, where N is configured by m_maxReceiveIterations
		/// in bdConnectionStoreConfig.
		/// \return 
		///  - True, if the receive was error free.
		///  - False otherwise.
		bdBool receiveAll();

		/// Dispatch messages.
		/// This code iterates the connections in the store and dispatches
		/// messages from each to the registered dispatch interceptors.
		void dispatchAll();

		/// Start shutdown of this class.
		/// Once the status transitions to BD_CONNECTION_STORE_UNINITIALIZED
		/// it is possible to call init() again.
		/// \note Calling this with BD_CONNECTION_STORE_SHUTDOWN_IMMEDIATE may
		///       result in inelegant disconnects for remotely connected 
		///       instances..
		/// \param shutdownType [in] Optional parameter to indicate how the shutdown 
		///                          should behave.
		/// \return 
		///  - True, if the shutdown was started without errors.
		///  - False otherwise.
		bdBool startShutdown(const bdConnectionStoreShutdownType shutdownType = BD_CONNECTION_STORE_SHUTDOWN_ELEGANT);

		/// Creating a connection to addr.
		/// \param addr [in] The address to connect to.
		/// \param secID [in] The security ID associated with the connection.
		/// \return A ref to the created connection or a null ref if the 
		///         connection was not created.
		bdConnectionRef create(	bdCommonAddrRef addr, 
								const bdSecurityID &secID);

		/// Get the status of the connection store.
		/// \return The current status of the connection store.
		bdConnectionStoreStatus getStatus() const;

		/// Registers an interceptor with the dispatcher.
		/// \param interceptor[in]	The interceptor to be registered with this
		///							dispatcher.
		void registerDispatchInterceptor(bdDispatchInterceptor *const interceptor);

		/// Removes an existing interceptor from the dispatcher.
		/// \param interceptor[in]	The interceptor to be removed from this
		///							dispatcher.
		void unregisterDispatchInterceptor(bdDispatchInterceptor *const interceptor);

		/// Accessor for the configuration info for this class.
		/// The configuration is set during initialization and cannot
		/// be altered afterwards.
		/// \param config[out] The config info for the connection store.
		void getConfig(bdConnectionStoreConfig &config) const;

#if defined BD_SOCKET_DO_BROADCAST && defined BD_PLATFORM_PSP_ADHOC
		/// This function improves PSP ad hoc performance substantially,
		/// as messages are not queued for a frame before being transmitted.
		void flushSendBuffer();
#endif //#if defined BD_SOCKET_DO_BROADCAST

	protected:

		/// Flushes all data available on connection.
		bdUInt flush(bdConnection& connection);

	protected:
	
		/// The socket router that the connection store should use.
		bdSocketRouter *m_socket;

		/// A map of all the current connections.
		bdHashMap<bdAddrHandleRefWrapper, bdConnectionRef, bdAddrHandleRefWrapper> m_connectionMap;

		/// The current status of the connection store.
		bdConnectionStoreStatus m_status;

		/// A class to dispatch packets to relevant dispatch interceptors.
		bdDispatcher m_dispatcher;

		/// A timer used during shutdown.
		bdStopwatch m_shutdownTimer;

		/// The address map instance that this class should use.
		bdAddressMap *m_addrMap;

		/// The configuration for the connection store, as set during init.
		bdConnectionStoreConfig m_config;
};


#endif // BD_CONNECTION_STORE_H
