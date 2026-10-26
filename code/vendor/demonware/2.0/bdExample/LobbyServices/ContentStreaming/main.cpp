// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "Simulation.h"

int BD_CALL main(int argc, char* argv[])
{
	bdCore::init();
	{
		Simulation sim;
		sim.start(argc, argv);
	}
	bdCore::quit();

	return 0;
}
