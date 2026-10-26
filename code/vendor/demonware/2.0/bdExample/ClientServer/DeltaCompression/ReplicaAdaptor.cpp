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
	bdFloat32 origVel[2];
	m_sphere.getVel(vel);
	m_sphere.getVel(origVel);

	// calculate the new velocity
	const bdFloat32 accel = 2.5f;
	const bdFloat32 secs = cmd->getTime()/1000.0f;
	vel[0] += xFactor * accel * secs;
	vel[1] += yFactor * accel * secs;

	m_sphere.setVel(vel);

	if(origVel[0] != vel[0])
	{
		setDirtyFlags(velX);
	}
	if(origVel[1] != vel[1])
	{
		setDirtyFlags(velY);
	}
}

void ReplicaAdaptor::serialize(bdBitBufferRef buffer)
{
	// write the current state of our sphere
	bdFloat32 vec[2];
	m_sphere.getPos(vec);
	if (buffer->writeBool(testDirtyFlags(ReplicaAdaptor::posX)))
	{
		buffer->writeFloat32(vec[0]);
	}
	if (buffer->writeBool(testDirtyFlags(ReplicaAdaptor::posY)))
	{
		buffer->writeFloat32(vec[1]);
	}

	m_sphere.getVel(vec);
	if (buffer->writeBool(testDirtyFlags(ReplicaAdaptor::velX)))
	{
		buffer->writeFloat32(vec[0]);
	}
	if (buffer->writeBool(testDirtyFlags(ReplicaAdaptor::velY)))
	{
		buffer->writeFloat32(vec[1]);
	}
}

bdBool ReplicaAdaptor::deserialize(bdBitBufferRef buffer)
{
	bdFloat32 vec[2];

	// get the current position
	m_sphere.getPos(vec);	
	
	// read any changes in position
	if(buffer->testBool())
	{
		buffer->readFloat32(vec[0]);
	}
	if(buffer->testBool())
	{
		buffer->readFloat32(vec[1]);
	}

	// set the new position
	m_sphere.setPos(vec);

	// get the current velocity
	m_sphere.getVel(vec);	

	// read any changes in velocity
	if(buffer->testBool())
	{
		buffer->readFloat32(vec[0]);
	}
	if(buffer->testBool())
	{
		buffer->readFloat32(vec[1]);
	}

	// set the new velocity
	m_sphere.setVel(vec);

	return !buffer->failedRead();
}

void ReplicaAdaptor::extrapolate(const bdFloat32 secs, 
								 const bdBool)
{
	// backup any serialized state they may change
	bdFloat32 origPos[2];
	bdFloat32 origVel[2];
	m_sphere.getPos(origPos);
	m_sphere.getVel(origVel);

	// move the sphere
	m_sphere.move(secs);

	// damp the velocity
	m_sphere.dampVel(0.4f * secs);

	// get the new serialized state
	bdFloat32 pos[2];
	bdFloat32 vel[2];
	m_sphere.getPos(pos);
	m_sphere.getVel(vel);
	
	if(origPos[0] != pos[0])
	{
		setDirtyFlags(posX);
	}
	if(origPos[1] != pos[1])
	{
		setDirtyFlags(posY);
	}
	if(origVel[0] != vel[0])
	{
		setDirtyFlags(velX);
	}
	if(origVel[1] != vel[1])
	{
		setDirtyFlags(velY);
	}
}

