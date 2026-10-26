// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "Simulation.h"
#include "PlayerInput.h"

bdBool Simulation::init(bdInt32,
						  bdNChar8* [])
{

	// we want to hear about all warnings and errors.
	m_logSubscriber.addChannel("warn");
	m_logSubscriber.addChannel("err");
	m_logSubscriber.addChannel("info");

	// create a sphere for the player to control
	Sphere *sphere = new Sphere;
	m_player.setControlledSphere(sphere);
	return true;
}

bdBool Simulation::step(const bdFloat32 time)
{	
	Sphere *const sphere = m_player.getControlledSphere();

	// get user inputs
	PlayerInput command;
	if (sphere && getUserInputs(command))
	{
		// apply the inputs to the controlled sphere
		const bdFloat32 xFactor = (command.m_keys[0]?-1.0f:0) + (command.m_keys[1]?1.0f:0);
		const bdFloat32 yFactor = (command.m_keys[3]?-1.0f:0) + (command.m_keys[2]?1.0f:0);		

		const bdFloat32 vel[2] = { xFactor * 0.8f,
								   yFactor * 0.8f };
		sphere->setVel(vel);

		sphere->move(time);
	}

	return true;
}

void Simulation::quit()
{
	Sphere *const sphere = m_player.getControlledSphere();
	if(sphere)
	{
		delete sphere;
	}
}

bdBool Simulation::getUserInputs(PlayerInput& command)
{
	bdBool cmd = false;
	if (getScreen().isKeyDown(BD_MOVE_LEFT))
	{
		command.m_keys[0] = true;
		cmd = true;
	}
	
	if (getScreen().isKeyDown(BD_MOVE_RIGHT))
	{
		command.m_keys[1] = true;
		cmd = true;
	}
	
	if (getScreen().isKeyDown(BD_MOVE_UP))
	{
		command.m_keys[2] = true;
		cmd = true;
	}

	if (getScreen().isKeyDown(BD_MOVE_DOWN))
	{
		command.m_keys[3] = true;
		cmd = true;
	}

	return cmd;	
}

