// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef SIMULATION_H
#define SIMULATION_H

#include <bdExample/bdExampleCommonPeer/bdExampleCommonP2P.h>

class UserCommand;
class SphereEx;

BD_REFERENCE(bdConnection);

class Simulation : public bdDemo,
				   public bdSessionListener,
				   public bdSessionInterceptor
{

	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		virtual bdBool init(bdInt argc,
							bdNChar8* argv[]);

		virtual bdBool step(const bdFloat32 time);

		virtual void quit();

	protected:

		virtual void onSessionConnectSuccess();

		virtual void onSessionConnect(const bdConnectionRef connection);

		virtual void onSessionDisconnect(const bdConnectionRef connection);

		virtual bdBool acceptSession(const bdReceivedMessage& receivedMsg);

		void handleCreateSphere(const bdReceivedMessage& createSphereMsg);

		void handleUpdate(const bdReceivedMessage& updateMsg);

		bdMessageRef createSphereMsg(const Sphere sphere);

		bdBool sendStateUpdate();

		bdBool sendOthers(const bdMessageRef message,
						  const bdBool reliable);

		bdBool getUserInputs(UserCommand& command);

		void displayInfo();

	protected:

		bdLANDiscoveryServer *m_discoveryServer;

		bdSession *m_session;
		bdSecurityID m_secID;

		SphereEx* m_localSphere;
		bdFastArray<SphereEx*> m_spheres;

		bdStopwatch m_updateTimer;

		// receives log messages
		bdLogSubscriber m_logSubscriber;

		bdBool m_leavingSession;
};

#endif // SIMULATION_H
