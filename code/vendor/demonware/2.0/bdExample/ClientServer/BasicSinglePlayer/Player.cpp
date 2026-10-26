// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "Player.h"

Player::Player()
: m_sphere(BD_NULL)
{
}

void Player::setControlledSphere(Sphere *const sphere)
{
	m_sphere = sphere;
}

Sphere* Player::getControlledSphere() const
{
	return m_sphere;
}

