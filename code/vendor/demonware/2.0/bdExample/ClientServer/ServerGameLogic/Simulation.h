// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef SIMULATION_H
#define SIMULATION_H

#include <bdExample/bdExampleCommonClientServer/bdExampleCommonCS.h>

#include "ReplicaAdaptor.h"

class PlayerInput;

class Simulation :	public bdDemo,
					public bdCommandGenerator,
					public bdServerPlayerListener
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		virtual bdBool init(bdInt32 argc,
							bdNChar8* argv[]);

		virtual bdBool step(const bdFloat32 time);

		virtual void quit();

	protected:

		bdBool getUserInputs(PlayerInput &command);

		virtual bdBool generateCommand(bdPlayerRef player,
									   bdUserCommand *const command);

		virtual void onServerPlayerJoin(bdServer *const server,
										bdPlayerRef player);

		virtual void onServerPlayerLeave(bdServer *const server,
										 bdPlayerRef player);

		void stepWorld(const bdFloat32 deltaTime);

	protected:

		/// The local player
		bdPlayerRef m_player;

		/// A reference to the server.
		bdServerRef m_server;

		/// A reference to the client.
		bdClientRef m_client;

		/// The replica list
		bdLinkedList<ReplicaAdaptorRef> m_replicaList;

		/// A pointer to the discovery server, if needed
		bdLANDiscoveryServer *m_discoveryServer;

		// receives log messages
		bdLogSubscriber m_logSubscriber;

};

#endif // SIMULATION_H
