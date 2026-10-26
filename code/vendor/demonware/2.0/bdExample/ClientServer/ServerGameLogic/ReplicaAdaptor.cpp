// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "ReplicaAdaptor.h"
#include "UserCommandAdaptor.h"

ReplicaAdaptor::ReplicaAdaptor()
{
	// set the type identifier for this class.
	m_type = SPHERE_REPLICA;
}

Sphere& ReplicaAdaptor::getSphere()
{
	return m_sphere;
}

void ReplicaAdaptor::initialize()
{
	bdRandom rand;
	// get a random position in the range [-4.5, 4.5] the crude way
	const bdFloat32 pos[2] = {-4.5f + (rand.nextUInt() % 10),
							  -4.5f + (rand.nextUInt() % 10)};

	// set the initial position
	m_sphere.setPos(pos);
}

void ReplicaAdaptor::applyCommand(bdUserCommand *const command,
								  const bdBool)
{
	// BitDemon invokes this method with a pointer to an 
	// abstract user command. Therefore we cast it into our concrete form.
	UserCommandAdaptor* cmd = static_cast<UserCommandAdaptor*>(command);

	// apply the command.
	const PlayerInput& input = cmd->getUserInput();
	const bdFloat32 xFactor = (input.m_keys[0]?-1.0f:0) + (input.m_keys[1]?1.0f:0);
	const bdFloat32 yFactor = (input.m_keys[3]?-1.0f:0) + (input.m_keys[2]?1.0f:0);

	bdFloat32 vel[2];
	m_sphere.getVel(vel);

	// calculate the new velocity
	const bdFloat32 accel = 2.5f;
	const bdFloat32 secs = cmd->getTime()/1000.0f;
	vel[0] += xFactor * accel * secs;
	vel[1] += yFactor * accel * secs;

	m_sphere.setVel(vel);
}

void ReplicaAdaptor::serialize(bdBitBufferRef buffer)
{
	// write the current state of our sphere
	bdFloat32 vec[2];
	m_sphere.getPos(vec);	
	buffer->writeFloat32(vec[0]);
	buffer->writeFloat32(vec[1]);

	m_sphere.getVel(vec);
	buffer->writeFloat32(vec[0]);
	buffer->writeFloat32(vec[1]);
}

bdBool ReplicaAdaptor::deserialize(bdBitBufferRef buffer)
{
	bdFloat32 pos[2];
	bdFloat32 vel[2];
	// read the new state of our sphere
	buffer->readFloat32(pos[0]);
	buffer->readFloat32(pos[1]);
	buffer->readFloat32(vel[0]);
	buffer->readFloat32(vel[1]);

	if (!buffer->failedRead())
	{
		m_sphere.setPos(pos);
		m_sphere.setVel(vel);
		return true;
	}
	return false;
}

void ReplicaAdaptor::extrapolate(const bdFloat32 secs, 
								 const bdBool)
{
	bdFloat32 origPos[2];
	m_sphere.getPos(origPos);

	// move the sphere
	m_sphere.move(secs);

	// check for collisions with the edge of the world
	m_sphere.collideBoxInternal(5, 5);

	// damp the velocity
	m_sphere.dampVel(0.4f * secs);

	bdFloat32 pos[2];
	m_sphere.getPos(pos);

	if((pos[0] != origPos[0]) || (pos[1] != origPos[1]))
	{
		// set ourselves as dirty so other clients will get updated
		setDirtyFlags(1);
	}
}

