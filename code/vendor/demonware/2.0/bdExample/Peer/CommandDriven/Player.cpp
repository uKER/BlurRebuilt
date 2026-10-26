// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "Player.h"
#include "UserCommand.h"

Player::Player()
: m_name("Unknown"),
  m_sphere(BD_NULL)
{
}

Player::Player(const bdNChar8* const name)
: m_name(name),
  m_sphere(BD_NULL)
{
}

Player::~Player()
{
	delete m_sphere;
}

const bdString& Player::getName() const
{
	return m_name;
}

void Player::serialize(bdBitBufferRef buffer)
{
	bdPlayer::serialize(buffer);
	buffer->writeString(m_name);
}

bdBool Player::deserialize(bdBitBufferRef buffer)
{
	bdPlayer::deserialize(buffer);
	buffer->readString(m_name);
	return !buffer->failedRead();
}

void Player::createSphere(const bdFloat32 *const pos,
						  const bdVector4& color)
{
	if(!m_sphere)
	{
		m_sphere = new Sphere;		
	}
	
	m_sphere->setPos(pos);
	m_sphere->setColor(color);
}

void Player::destroySphere()
{
	delete m_sphere;
	m_sphere = BD_NULL;
}

void Player::applyCommand(const UserCommand *const command)
{
	if(m_sphere)
	{
		const bdBool *const keys = command->m_keys;
		const bdFloat32 xFactor = (keys[0]?-1.0f:0) + (keys[1]?1.0f:0);
		const bdFloat32 yFactor = (keys[3]?-1.0f:0) + (keys[2]?1.0f:0);

		bdFloat32 vel[2];
		m_sphere->getVel(vel);

		// calculate the new velocity
		const bdFloat32 accel = 2.5f;
		const bdFloat32 time = command->m_time / 1000.0f;
		vel[0] += xFactor * accel * time;
		vel[1] += yFactor * accel * time;

		m_sphere->setVel(vel);

		m_sphere->move(time);

		m_sphere->dampVel(0.1f * time);
	}
}

