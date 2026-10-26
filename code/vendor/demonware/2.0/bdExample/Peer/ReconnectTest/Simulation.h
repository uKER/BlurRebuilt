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

class Simulation : public bdDemo,
				   public bdGame,
				   public bdSessionListener
{

	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		virtual bdBool init(bdInt argc,
							bdNChar8* argv[]);

		virtual bdBool step(const bdFloat32 time);

		virtual void quit();

	protected:


		bdBool getUserInputs(UserCommand& command);

		void displayInfo();

		// cos I have to
		void onRoundStart(const bdBitBufferRef userData) {};

		void onSessionConnectFail();

		void onSessionConnectSuccess();

	protected:

		bdLANDiscoveryServer *m_discoveryServer;

		bdPeer *m_peer;

		bdString m_name;

		// receives log messages
		bdLogSubscriber m_logSubscriber;

		static const bdNChar8 *m_screenInstructions;

		bdGameInfoRef m_game;
		bdString m_statusString;

};

#endif // SIMULATION_H
