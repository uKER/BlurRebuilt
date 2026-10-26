// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "Simulation.h"
#include "UserCommand.h"
#include "SphereEx.h"

#define TITLE_ID (0x10000008)

#define CREATE_SPHERE_MSG	(BD_USER_MESSAGE_TYPE)
#define UPDATE_MSG			(BD_USER_MESSAGE_TYPE+1)

#define UPDATE_INTERVAL (0.1f)

#define BD_LOG_LEVEL "simulation"

bdBool Simulation::init(bdInt32 argc,
						bdNChar8* argv[])
{
	// initialize our member variables
	m_discoveryServer = BD_NULL;
	m_session = BD_NULL;
	m_localSphere = BD_NULL;
	m_leavingSession = false;


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

	if(ok)
	{
		BD_REGISTER_GAME_INFO_CLASS(bdGameInfo);
		BD_REGISTER_SESSION_DATA_CLASS(bdSessionData);
		bdGameInfoRef game;

		DiscoveryManager discoveryManager;
		if(argc >= 3)
		{
			// if there is a third argument it is an IP address to connect to
			game = discoveryManager.directConnect(TITLE_ID, argv[2]);
		}

		if(game.isNull())
		{
			// try to find any hosts running matching games on the LAN
			game = discoveryManager.discoverLanGames(TITLE_ID);
		}

		if(game.isNull())
		{
			// no matching games so create one
			game = discoveryManager.createGame(TITLE_ID);

			// create a discovery server to advertise the game
			m_discoveryServer = new bdLANDiscoveryServer;
			m_discoveryServer->start(game);
		}

		bdSecurityKeyMap *const keyMap = net->getSocketRouter()->getKeyMap();
		keyMap->registerKey(game->getSecurityID(), game->getSecurityKey());
		m_secID = game->getSecurityID();

		// create a session and connect
		m_session = new bdSession;
		m_session->registerListener(this);
		m_session->registerInterceptor(this);
		m_session->join(game->getHostAddr(),
						game->getSecurityID(),
						game->getSecurityKey());

	}

	return ok;
}

bdBool Simulation::step(const bdFloat32 time)
{
	// sample the inputs
	const bdBool quitKey = getScreen().wasKeyPressed(BD_QUIT);
	// check if session is fully disconnected, i.e. all connections in the session have been disconnected correctly
	bdBool disconnected = (m_session->getStatus() == bdSession::BD_SESSION_NOT_CONNECTED);

	bdNetImpl *const net = bdNet::getInstance();

	if(quitKey)
	{
		// Gracefully leave the session - we will need to pumpShutdown()
		// until we know all connections in the session have been disconnected correctly
		m_session->leaveGraceful();
		m_leavingSession = true;
	}

	if(m_leavingSession){		
		m_session->pumpShutdown();
	}

	// receive messages from the network and dispatch them
	net->receiveAll();
	net->dispatchAll();

	// pump the discovery server
	if(m_discoveryServer)
	{
		m_discoveryServer->update();
	}

	if(m_localSphere)
	{
		UserCommand command;
		if(getUserInputs(command))
		{
			const bdBool *const keys = command.m_keys;
			const bdFloat32 xFactor = (keys[0]?-1.0f:0) + (keys[1]?1.0f:0);
			const bdFloat32 yFactor = (keys[3]?-1.0f:0) + (keys[2]?1.0f:0);

			//Sphere& sphere = m_spheres[m_localSphereIndex].getSphere();

			bdFloat32 vel[2];
			m_localSphere->getSphere().getVel(vel);

			// calculate the new velocity
			const bdFloat32 accel = 1.5f;
			vel[0] += xFactor * accel * time;
			vel[1] += yFactor * accel * time;

			m_localSphere->getSphere().setVel(vel);
		}

		m_localSphere->getSphere().move(time);
		m_localSphere->getSphere().dampVel(0.2f*time);
	}

	if(m_updateTimer.getElapsedTimeInSeconds() > UPDATE_INTERVAL)
	{
		if(sendStateUpdate())
		{
			m_updateTimer.start();
		}
	}

	// print any text to the screen
	displayInfo();

	// send all messages
	net->sendAll();

	// Returns false (thus calling quit()) only when we have decided to leave the session
	// AND the session is fully disconnected
	return !(m_leavingSession && disconnected);
}

void Simulation::quit()
{
	bdNetImpl *const net = bdNet::getInstance();

	// If quit() has been called, we know all connections in the session are now disconnected,
	// so we can safely unregister the security key
	bdSecurityKeyMap *const keyMap = net->getSocketRouter()->getKeyMap();
	keyMap->unregisterKey(m_secID);

	// clean up any allocated objects
	if(m_session)
	{
		m_session->unregisterInterceptor(this);
		m_session->unregisterListener(this);
		delete m_session;
	}

	const bdUInt numSpheres = m_spheres.getSize();
	for(bdUInt i = 0; i < numSpheres; i++)
	{
		SphereEx *sphere = m_spheres[i];
		if(sphere)
		{
			delete sphere;
		}
	}

	m_spheres.clear();

	if(m_discoveryServer)
	{
		m_discoveryServer->stop();
		delete m_discoveryServer;
	}

	net->stop();

	bdExamplePlatformInit::quit();
}

void Simulation::onSessionConnectSuccess()
{
	// connected to the session

	// get a random position in the range [-4.5, 4.5] the crude way
	bdTrulyRandomImpl *const rand = bdTrulyRandom::getInstance();
	const bdFloat32 pos[2] = {-4.5f + (rand->getRandomUInt() % 10),
							  -4.5f + (rand->getRandomUInt() % 10)};

	// get a random colour
	bdVector4 color;
	color.setX(static_cast<bdFloat32>(rand->getRandomUInt())/BD_UINT32_MAX);
	color.setY(static_cast<bdFloat32>(rand->getRandomUInt())/BD_UINT32_MAX);
	color.setZ(static_cast<bdFloat32>(rand->getRandomUInt())/BD_UINT32_MAX);
	color.setW(1);

	SphereEx sphereEx;
	Sphere& sphere = sphereEx.getSphere();
	sphere.setPos(pos);
	sphere.setColor(color);

	// send a create sphere message
	const bdMessageRef createSphereMsg = new bdMessage(CREATE_SPHERE_MSG);
	const bdBitBufferRef buffer = createSphereMsg->getPayload();

	sphereEx.writeInitData(buffer);

	m_session->send(createSphereMsg, true);
}

void Simulation::onSessionConnect(const bdConnectionRef connection)
{
	// new peer has connected, tell them about our local sphere
	if(m_localSphere)
	{
		// send a create sphere message
		const bdMessageRef createSphereMsg = new bdMessage(CREATE_SPHERE_MSG);
		const bdBitBufferRef buffer = createSphereMsg->getPayload();

		m_localSphere->writeInitData(buffer);
		m_session->send(connection, createSphereMsg, true);
	}
}

void Simulation::onSessionDisconnect(const bdConnectionRef connection)
{
	// remove any spheres from this connection
	for(bdUInt i = 0; i < m_spheres.getSize(); i++)
	{
		SphereEx *const sphere = m_spheres[i];
		if(sphere && (sphere->getConnection() == connection))
		{
			m_spheres.removeAt(i);
			i--;
			delete sphere;
		}
	}
}

bdBool Simulation::acceptSession(const bdReceivedMessage& receivedMsg)
{
	const bdMessageRef message = receivedMsg.getMessage();
	const bdMessageType type = message->getType();
	bdBool accepted = true;

	switch(type)
	{
		case CREATE_SPHERE_MSG:
		{
			handleCreateSphere(receivedMsg);
		}
		break;
		case UPDATE_MSG:
		{
			handleUpdate(receivedMsg);
		}
		break;
		default:
		{
			accepted = false;
		}
	}

	return accepted;
}

void Simulation::handleCreateSphere(const bdReceivedMessage& createSphereMsg)
{
	SphereEx *const sphere = new SphereEx;

	const bdBool ok = sphere->applyInitData(createSphereMsg);

	if(ok)
	{
		m_spheres.pushBack(sphere);

		if(sphere->getConnection()->getAddress()->isLoopback())
		{
			m_localSphere = sphere;
			m_updateTimer.start();
		}
	}
	else
	{
		delete sphere;
	}
}

void Simulation::handleUpdate(const bdReceivedMessage& updateMsg)
{
	const bdConnectionRef connection = updateMsg.getConnection();

	// find the sphere from this connection
	bdBool found = false;
	const bdUInt numSpheres = m_spheres.getSize();
	for(bdUInt i = 0; !found && (i < numSpheres); i++)
	{
		SphereEx *const sphere = m_spheres[i];
		if(sphere && (sphere->getConnection() == connection))
		{
			found = true;
			sphere->applyUpdateData(updateMsg);
		}
	}
}

bdBool Simulation::sendStateUpdate()
{
	bdBool sent = false;

	if(m_localSphere)
	{
		// create a state update message
		const bdMessageRef updateMessage = new bdMessage(UPDATE_MSG);
		const bdBitBufferRef buffer = updateMessage->getPayload();

		m_localSphere->createUpdateData(buffer);

		sent = sendOthers(updateMessage, false);
	}

	return sent;
}

bdBool Simulation::sendOthers(const bdMessageRef message,
							  const bdBool reliable)
{
	bdBool sent = false;
	if(m_session)
	{
		sent = true;
		const bdUInt numPeers = m_session->getNumPeers();

		for(bdUInt i=0; i < numPeers; i++)
		{
			if(i != m_session->getLocalPeerIndex())
			{
				const bdConnectionRef connection = m_session->getConnection(i);
				sent = m_session->send(connection, message, reliable) && sent;
			}
		}
	}

	return sent;
}

bdBool Simulation::getUserInputs(UserCommand& command)
{
	// sample the local controls
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

void Simulation::displayInfo()
{
	bdNChar8 buf[512];
	bdUInt bytesWritten = 0;

	// get info about the state of the session
	bytesWritten += bdSessionInfo::getInfo(m_session, buf + bytesWritten, sizeof(buf) - bytesWritten);

	// print the text to the screen
	getScreen().printToScreen(buf);
}