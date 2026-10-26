// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "Simulation.h"
#include "UserCommandAdaptor.h"

#define TITLE_ID (0x0000000A)

bdBool Simulation::init(bdInt32 argc,
						bdNChar8* argv[])
{
	// register the classes that BitDemon will have to create
	BD_REGISTER_CLASS(SPHERE_REPLICA, ReplicaAdaptor);
	BD_REGISTER_COMMAND_CLASS(UserCommandAdaptor);
	BD_REGISTER_PLAYER_CLASS(bdPlayer);

	m_discoveryServer = BD_NULL;
	m_client = BD_NULL;
	m_server = BD_NULL;

	bdNetImpl *const net = bdNet::getInstance();
	// try to start bdNet


	// we want to hear about all warnings and errors.
	m_logSubscriber.addChannel("warn");
	m_logSubscriber.addChannel("err");
	m_logSubscriber.addChannel("info");

	// try to start bdNet
	bdNetStartParams params;
	params.m_onlineGame = true;

	// platformParams is used to store platform specific startup parameters
	bdPlatformParams platformParams;

	bdExamplePlatformInit::init(params, &platformParams);

	if(net->start(params))
	{
		// wait for bdNetImpl::start to complete
		// for code clarity we do an active wait
		while(net->getStatus() == bdNetImpl::BD_NET_PENDING)
		{
			net->pump();
		}
	}

	// check bdNet has been started successfully
	const bdBool ok = (net->getStatus() == bdNetImpl::BD_NET_DONE);

	if (ok)
	{
		BD_REGISTER_GAME_INFO_CLASS(bdGameInfo);
		bdGameInfoRef game;

		DiscoveryManager discoveryManager;
		if(argc >= 2)
		{
			// if there is a second argument it is an IP address to connect to
			game = discoveryManager.directConnect(TITLE_ID, argv[1]);
		}

		if(game.isNull())
		{
			// try to find any hosts running matching games on the LAN
			game = discoveryManager.discoverLanGames(TITLE_ID);
		}

		if(game.isNull())
		{
			//no matching games so create one
			game = discoveryManager.createGame(TITLE_ID);

			// create a discovery server to advertise the game
			m_discoveryServer = new bdLANDiscoveryServer;
			m_discoveryServer->start(game);

			m_server = new bdServer();
			m_server->init(game);
			// register ourselves as a listener for player join/leave notifications
			m_server->registerPlayerListener(this);

			m_server->setUpdateInterval(0.5);

		}
		else
		{
			m_client = new bdClient;

			m_client->connect(game);
			// create a local player
			m_player = new bdPlayer;

			// ask the player to join the server
			m_client->join(m_player);

			// register ourselves as a source of commands
			m_client->setCommandGenerator(this);

			// register ourselves as a listener for replica create/delete notifications
			m_client->getReplicaManager()->registerReplicaListener(this);

		}
	}

	bdPrintf("Hit '%s' to exit.\n", BD_QUIT_DESC);
	return ok;
}


bdBool Simulation::step(const bdFloat32 time)
{
	bdNetImpl *const net = bdNet::getInstance();
	net->receiveAll();
	net->dispatchAll();


	if(m_server)
	{
		stepWorld(time);
		m_server->step(time);
		if (m_discoveryServer)
		{
			m_discoveryServer->update();
		}
	}

	if(m_client)
	{
		m_client->step(time);
	}

	if (getScreen().wasKeyPressed(BD_QUIT))
	{
		return false;
	}

	net->sendAll();
	return true;
}

void Simulation::quit()
{
	if(m_client && m_player)
	{
		m_client->leave(m_player);
	}
	if (m_discoveryServer)
	{
		delete m_discoveryServer;
	}
	bdNetImpl *const net = bdNet::getInstance();
	net->stop();

	bdExamplePlatformInit::quit();
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

bdBool Simulation::generateCommand(bdPlayerRef player,
								   bdUserCommand *const command)
{
	PlayerInput input;
	getUserInputs(input);
	UserCommandAdaptor *cmdAdaptor = static_cast<UserCommandAdaptor*>(command);
	cmdAdaptor->setUserInput(input);
	return true;
}

void Simulation::onServerPlayerJoin(bdServer *const,
									bdPlayerRef player)
{
	// a new player has joined the server
	bdPrintf("Player %u joined the server.\n", player->getId());

	// create a replica for them to control
	ReplicaAdaptorRef adaptor = new ReplicaAdaptor;

	// inform the replica manager of the new object
	m_server->getReplicaManager()->add(adaptor);
	// let the new player control the new object
	m_server->setControlledReplica(player, adaptor);

	// add sphere to replica list
	m_replicaList.addTail(adaptor);
}

void Simulation::onServerPlayerLeave(bdServer *const,
									 bdPlayerRef player)
{
	// a player has left the server
	bdPrintf("Player %u left has left the server\n", player->getId());

	// remove the sphere they were controlling
	bdReplicaRef replica = player->getControlledReplica();
	if(replica)
	{
		m_server->getReplicaManager()->remove(replica);

		// remove the sphere from the replica list
		bdLinkedList<ReplicaAdaptorRef>::Position pos = m_replicaList.find(replica);
		if (pos)
		{
			m_replicaList.removeAt(pos);
		}
	}
}

void Simulation::onClientInitialize(bdReplicaRef replica)
{
	// create an interpolation target
	bdInterpolationData *const interpolationData = replica->getClientData()->createInterpolationData();
	ReplicaAdaptor *target = new ReplicaAdaptor;
	Sphere &targetSphere = target->getSphere();
	targetSphere.setVisible(false);
	interpolationData->setTarget(target);
	interpolationData->setActive(true, replica);
	interpolationData->setInterpolationInterval(1);
}

void Simulation::stepWorld(const bdFloat32)
{
	bdLinkedList<ReplicaAdaptorRef>::Position headPos = m_replicaList.getHeadPosition();
	// Iterate through all spheres
	while(headPos)
	{
		bdFloat32 pos[2];
		ReplicaAdaptorRef head = m_replicaList.forward(headPos);

		Sphere &sphere = head->getSphere();
		// Check whether the position of the sphere is outside
		// the perimeter of the grid.
		sphere.getPos(pos);
		if (pos[0] > 5)
		{
			pos[0] = -5;
			head->setDirtyFlags(BD_INTERPOLATION_BREAK);
		}
		if (pos[0] < -5)
		{
			pos[0] = 5;
			head->setDirtyFlags(BD_INTERPOLATION_BREAK);
		}
		if (pos[1] > 5)
		{
			pos[1] = -5;
			head->setDirtyFlags(BD_INTERPOLATION_BREAK);
		}
		if (pos[1] < -5)
		{
			pos[1] = 5;
			head->setDirtyFlags(BD_INTERPOLATION_BREAK);
		}
		sphere.setPos(pos);

		// Perform collision detection between any two spheres
		bdLinkedList<ReplicaAdaptorRef>::Position nextPos = headPos;

		while(nextPos)
		{
			ReplicaAdaptorRef other = m_replicaList.forward(nextPos);

			Sphere& headSphere = head->getSphere();
			Sphere& otherSphere = other->getSphere();

			if (headSphere.collideSphere(&otherSphere))
			{
				head->setDirtyFlags(BD_INTERPOLATION_BREAK);
				other->setDirtyFlags(BD_INTERPOLATION_BREAK);
			}
		}
	}
}


