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
				   public bdPlayerManagerListener
{

	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		virtual bdBool init(bdInt argc,
							bdNChar8* argv[]);

		virtual bdBool step(const bdFloat32 time);

		virtual void quit();

	protected:

		void removePlayer();

		bdBool generateCommands();

		bdBool getUserInputs(UserCommand& command);

		virtual void onRoundStart(const bdBitBufferRef userData);

		virtual void onRoundEnd();

		virtual void processCommand(const bdPlayerRef player,
									const bdCommandRef command);

		virtual void onPlayerJoin(const bdPlayerRef player);

		void createSphere(const bdPlayerRef player,
						  bdRandom *const rand) const;

		void displayInfo();

		bdUInt getPlayerInfo(bdNChar8 *buf,
							 const bdUInt length) const;

		void alternateGameName();

	protected:

		bdLANDiscoveryServer *m_discoveryServer;

		bdPeer *m_peer;

		bdString m_name;

		// receives log messages
		bdLogSubscriber m_logSubscriber;

		static const bdNChar8 *m_screenInstructions;

		bdGameInfoRef m_game;

		bdBool m_alternativeGameNameFlag;
};

#endif // SIMULATION_H
