// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "Simulation.h"
#include "UserCommandAdaptor.h"
#include "ReplicaAdaptor.h"
#include "InterestLevels.h"

#define TITLE_ID (0x00000009)

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


	// we want to hear about all warnings and errors.
	m_logSubscriber.addChannel("warn");
	m_logSubscriber.addChannel("err");
	m_logSubscriber.addChannel("info");

	bdNetImpl *const net = bdNet::getInstance();

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
			// register InterestLevels as generator of replica interest values
			// it will be deleted by bdServer in it's destructor.
			InterestLevels *t_interestLevel = new InterestLevels;
			// tell the interestLevels which replica table to use.
			t_interestLevel->registerReplicaTable(m_server->getReplicaTable());
			m_server->setInterest(t_interestLevel);

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
	}
}

