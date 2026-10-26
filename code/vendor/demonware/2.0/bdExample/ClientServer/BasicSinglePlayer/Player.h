// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef PLAYER_H
#define PLAYER_H

#include <bdExample/bdExampleCommon/bdExampleCommon.h>

class Player
{
	public:

		Player();

		void setControlledSphere(Sphere *const sphere);

		Sphere* getControlledSphere() const;

	protected:

		Sphere* m_sphere;
};

#endif // PLAYER_H
