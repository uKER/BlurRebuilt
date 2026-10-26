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

ReplicaAdaptor::~ReplicaAdaptor()
{

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

	const bdFloat32 vel[2] = { xFactor * 0.8f,
							   yFactor * 0.8f };
	m_sphere.setVel(vel);
	m_sphere.move(cmd->getTimeInSeconds());

	// set ourselves as dirty so other clients will get updated
	setDirtyFlags(1);
}

void ReplicaAdaptor::serialize(bdBitBufferRef buffer)
{
	bdFloat32 pos[2];
	m_sphere.getPos(pos);	
	// write the current state of our sphere
	buffer->writeFloat32(pos[0]);
	buffer->writeFloat32(pos[1]);
}

bdBool ReplicaAdaptor::deserialize(bdBitBufferRef buffer)
{
	bdFloat32 pos[2];
	// read the new state of our sphere
	buffer->readFloat32(pos[0]);
	buffer->readFloat32(pos[1]);
	if (buffer->failedRead())
	{			
		return false;
	}

	m_sphere.setPos(pos);
	return true;
}


