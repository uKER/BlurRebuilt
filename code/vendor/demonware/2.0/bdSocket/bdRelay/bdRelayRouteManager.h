// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_RELAY_ROUTE_MANAGER_H
#define BD_RELAY_ROUTE_MANAGER_H

// These defines are used for choosing the optimal relayer. The decision is
// based on both load and latency. If the route through the new relayer has an
// estimated RTT less than the current by more than the threshold it is chosen.
// If its estimated RTT is not lower by at least the threshold but its load is
// lower by the load threshold it will also be chosen over the current relayer.

#define BD_RELAY_LATENCY_THRESHOLD (20u) // 20 milliseconds
#define BD_RELAY_LOAD_THRESHOLD (40u)
#define BD_MAX_RELAYER_LOAD (100u)

#define BD_RELAY_DEFAULT_LATENCY (250) // 250 milliseconds

#include <bdCore/bdCore.h>
#include <bdSocket/bdRelay/bdPacket/bdRelayerInfo.h>

class bdRelayRouteManager
{
public:

	BD_DECLARE_NEW_AND_DELETE_OPERATORS;

	/// Basic Constructor
	bdRelayRouteManager();

	/// Basic Destructor
	~bdRelayRouteManager();

	/// Clears arrays
	void quit();

	/// Pump the relay route manager, rebuilding the routes table
	void pump();

	/// Adds a direct route to the destinationEndpoint
	/// \param destinationEndpoint[in]	The endpoint we have a direct connection to.
	/// \return Whether the route was added. Will return false if the route already exists.
	bdBool addDirectRoute( const bdEndpoint &destinationEndpoint );

	/// Adds a relayed route through the relayer to the destination.
	/// \param relayerEndpoint[in]		The relayer
	/// \param destinationEndpoint[in]	The destination
	/// \return Whether the relay route was added.
	bdBool addRelayedRoute( const bdEndpoint &relayerEndpoint, const bdEndpoint &destinationEndpoint );

	/// Remove a direct route.
	/// \param destinationEndpoint[in]	The endpoint we have a direct connection to.
	/// \return Whether the route was successfully removed.
	bdBool removeDirectRoute( const bdEndpoint &destinationEndpoint );

	/// Removes a relayed route through the relayer to the destination.
	/// \param relayerEndpoint[in]		The relayer
	/// \param destinationEndpoint[in]	The destination
	/// \return Whether the relay route was successfully removed.
	bdBool removeRelayedRoute( const bdEndpoint &relayerEndpoint, const bdEndpoint &destinationEndpoint );

	/// Get the currently selected relayer for the target.
	/// \param targetEndpoint[in]	The desired target.
	/// \param relayerEndpoint[out] The current relayer.
	/// \return Whether the relayer exists.
	bdBool getRelayer( const bdEndpoint &targetEndpoint, bdEndpoint &relayerEndpoint ) const;

	/// Sets the load for the relayer.
	/// \param relayerEndpoint[in]	The relayer
	/// \param load[in]				The new load.
	void updateRelayerLoad( const bdEndpoint &relayerEndpoint, bdUInt32 load );

	/// Find the relayer for the target
	bdBool findRoute( const bdEndpoint &targetEndpoint, bdEndpoint &relayerEndpoint ) ;

	/// Get the load associated with a relayer.
	/// \param relayerEndpoint[in]  The relayer
	/// \param load[out]			The relayer's load
	/// \return Whether the load exists
	bdBool getRelayerLoad( const bdEndpoint &relayerEndpoint, bdUInt &load );

	/// Set the latency between a relayer and a target.
	/// \param targetEndpoint[in]	The target
	/// \param relayerEndpoint[in]	The relayer
	/// \param newLatency[in]		The latency between the relayer and the target.
	void updateEndpointLatency( const bdEndpoint &targetEndpoint, const bdEndpoint &relayerEndpoint, bdUInt16 newLatency );

	/// Get the latency between a relayer and a target.
	/// \param targetEndpoint[in]	The target
	/// \param relayerEndpoint[in]	The relayer
	/// \param latency[out]		The latency between the relayer and the target.
	/// \return Whether the latency could be found
	bdBool getEndpointLatency( const bdEndpoint &targetEndpoint, const bdEndpoint &relayerEndpoint, bdUInt16 &latency );

	/// Get an array of endpoints that we connected to through a relayer.
	/// \return The array of relayed endpoints.
	bdArray <bdEndpoint> getRelayedEndpoints();

	/// Get an array of endpoints that we connected to directly.
	/// \return The array of relayed endpoints.
	bdArray <bdEndpoint> getDirectEndpoints();

	/// Serialize the maps containing route information (the routes used and the routes available)
	/// Meant for debug and reporting purposes
	/// \param buffer[in]		Where to write the data
	/// \param bufferSize[in]	The size of the bufffer
	/// \return the number of bytes written.
	bdUInt serializeRoutes( void * buffer, const bdUInt bufferSize ) const;

	/// Print the current state of the routes table.
	/// Meant for debug and reporting purposes
	void print();

	void addReturnRoute(const bdEndpoint &relayerEndpoint, const bdEndpoint &destinationEndpoint, const bdCommonAddrRef localCommonAddr);
	
protected:

	bdBool isAuthorativeOverRoute(const bdCommonAddrRef localCommonAddr, const bdCommonAddrRef remoteCommonAdd);

	bdBool getRelayerFromAnotherSession(bdEndpoint &relayer,  const bdEndpoint &destination  );


	bdHashMap<bdEndpoint /*Target*/, bdEndpoint /*Relayer*/, bdEndpointHashingClass> m_incomingRoutes;


	// The map representing, for each endpoint, all the neighbors of that endpoint and their
	// latencies. From this map, more than one path to any given destination could possibly
	// be derived
	bdHashMap<bdEndpoint /*Relayer*/, bdArray<bdTargetLatency> /*Relayer's Direct Connections*/, bdEndpointHashingClass> m_connections;

	// The map representing the relayer for each target endpoint
	bdHashMap<bdEndpoint /*Target*/, bdEndpoint /*Relayer*/, bdEndpointHashingClass> m_routes;

	// The map representing, for each endpoint, the load that the endpoint is experiencing
	// When we expand to multi-relay routes, the cost of the route should be related to endpoint1_load + endpoint2_load ...
	bdHashMap<bdEndpoint /*Relayer*/, bdUInt /*Relayer's load*/, bdEndpointHashingClass> m_relayerLoads;

};

#endif // BD_RELAY_ROUTE_MANAGER_H