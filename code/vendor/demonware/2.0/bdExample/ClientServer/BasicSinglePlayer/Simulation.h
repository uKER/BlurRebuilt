// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef SIMULATION_H
#define SIMULATION_H

#include <bdExample/bdExampleCommon/bdExampleCommon.h>
#include "Player.h"

class PlayerInput;

class Simulation : public bdDemo
{
	public:

		virtual bdBool init(bdInt32 argc, bdNChar8* argv[]);

		virtual bdBool step(const bdFloat32 time);

		virtual void quit();

	protected:

		bdBool getUserInputs(PlayerInput& command);
		
		// A player in the game
		Player m_player;

		// receives log messages
		bdLogSubscriber m_logSubscriber;

};

#endif // SIMULATION_H
