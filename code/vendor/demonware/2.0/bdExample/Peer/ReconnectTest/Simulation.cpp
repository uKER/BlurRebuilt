// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "Simulation.h"
#include "Player.h"
#include "UserCommand.h"

#define TITLE_ID (0x10000052)

#define SEND_INTERVAL (0.1f)

#define COMMAND_STEP (0.1f)

#define BD_DIS_H_PLAYER	BD_BUTTON1
#define BD_RE_H_PLAYER	BD_BUTTON2
#define BD_CON_H_ROUND	BD_BUTTON3
#define BD_DIS_C_ROUND	BD_BUTTON4

const bdNChar8* Simulation::m_screenInstructions =	"Disconnect from host [if applicable]: " BD_BUTTON1_DESC " \n"
													"Reconnect To Host:  " BD_BUTTON2_DESC " \n"
													"Make connection to Host: " BD_BUTTON3_DESC "\n"
													"Disconnect connection from Host: " BD_BUTTON4_DESC "\n \n";

#define BD_LOG_LEVEL "simulation"

bdBool Simulation::init(bdInt32 argc,
						bdNChar8* argv[])
{
	// initialize our member variables
	m_discoveryServer = BD_NULL;
	m_peer = BD_NULL;
	m_name = "Unnamed";
	m_game = BD_NULL;


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
	bdBool ok = (net->getStatus() == bdNetImpl::BD_NET_DONE);

	if(ok)
	{
		// if there is a second argument use it as the player name
		if(argc >= 2)
		{
			m_name = argv[1];
		}

		BD_REGISTER_GAME_INFO_CLASS(bdGameInfo);

		DiscoveryManager discoveryManager;
		if(argc >= 3)
		{
			// if there is a third argument it is an IP address to connect to
			m_game = discoveryManager.directConnect(TITLE_ID, argv[2]);
		}

		if(m_game.isNull())
		{
			// try to find any hosts running matching games on the LAN
			m_game = discoveryManager.discoverLanGames(TITLE_ID);
		}

		if(m_game.isNull())
		{
			// no matching games so create one
			m_game = discoveryManager.createGame(TITLE_ID);

			// create a discovery server to advertise the game
			m_discoveryServer = new bdLANDiscoveryServer;
			if(false == m_discoveryServer->start(m_game))
			{
				BD_ERR(BD_LOG_LEVEL , "Failed to start discovery server" );
			}
		}

		// register the classes that BitDemon will have to create
		BD_REGISTER_PLAYER_CLASS(Player);
		BD_REGISTER_COMMAND_CLASS(UserCommand);
		BD_REGISTER_SESSION_DATA_CLASS(bdSessionData);

		// Register the game secID before using it.
		bdSecurityKeyMap *const keyMap = net->getSocketRouter()->getKeyMap();

		const bdBool keyAdded = keyMap->registerKey(m_game->getSecurityID(), m_game->getSecurityKey());
		if(true == keyAdded)
		{

			// create a peer and connect to the host
			// register ourselves as the game interface
			m_peer = new bdPeer(this);
			m_peer->connect(m_game);

			// register ourselves as a player listener so we can respond
			// to players joining and leaving
			m_peer->getSession()->registerListener(this);

			// set the interval at which commands/messages will be sent
			m_peer->setSendInterval(SEND_INTERVAL);
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL, "Key registration failed, cannot continue");
			ok = false;
		}
	}

	return ok;
}

bdBool Simulation::step(const bdFloat32 time)
{
	// sample the inputs
	const bdBool quitKey = getScreen().wasKeyPressed(BD_QUIT);
	const bdBool disconnectFromHostKey = getScreen().wasKeyPressed(BD_DIS_H_PLAYER);
	const bdBool reconnectToHostKey = getScreen().wasKeyPressed(BD_RE_H_PLAYER);
	const bdBool makeConnectionToHostKey = getScreen().wasKeyPressed(BD_CON_H_ROUND);
	const bdBool removeConnectionToHostKey = getScreen().wasKeyPressed(BD_DIS_C_ROUND);

	bdNetImpl *const net = bdNet::getInstance();

	// receive messages from the network and dispatch them
	net->receiveAll();
	net->dispatchAll();


	// pump the discovery server
	if(m_discoveryServer)
	{
		m_discoveryServer->update();
	}

	if(disconnectFromHostKey)
	{
		m_peer->leave();
	}

	if(reconnectToHostKey)
	{
		m_peer->connect(m_game);
	}

	if(makeConnectionToHostKey)
	{
	}

	if(removeConnectionToHostKey)
	{
	}



	// print any text to the screen
	displayInfo();

	// pump the peer to allow it to send messages
	m_peer->step(time);

	// send all messages
	net->sendAll();

	return !quitKey;
}

void Simulation::quit()
{
	// clean up any allocated objects

	if(m_peer)
	{
		m_peer->getSession()->unregisterListener(this);
		delete m_peer;
	}

	if(m_discoveryServer)
	{
		m_discoveryServer->stop();
		delete m_discoveryServer;
	}

	// stop the networking
	bdNetImpl *const net = bdNet::getInstance();
	net->stop();

	bdExamplePlatformInit::quit();
}


void Simulation::displayInfo()
{
	bdNChar8 buf[512];
	bdUInt bytesWritten = 0;

	bytesWritten = static_cast<bdUInt>(bdStrlcpy(buf, m_screenInstructions, 512));

	// get info about the state of the session
	bytesWritten += bdSessionInfo::getInfo(m_peer->getSession(), buf + bytesWritten, sizeof(buf) - bytesWritten);
	if(bytesWritten < sizeof(buf))
	{
		buf[bytesWritten++] = '\n';
	}

	// get info about the state of the delivery manager
	bytesWritten += bdDeliveryManagerInfo::getInfo(m_peer->getDeliveryManager(), buf + bytesWritten, sizeof(buf) - bytesWritten);
	if(bytesWritten < sizeof(buf))
	{
		buf[bytesWritten++] = '\n';
	}

	// get info about the state of the round
	bytesWritten += bdRoundInfo::getInfo(m_peer->getRound(), buf + bytesWritten, sizeof(buf) - bytesWritten);
	if(bytesWritten < sizeof(buf))
	{
		buf[bytesWritten++] = '\n';
	}

	bytesWritten += bdSnprintf(buf + bytesWritten , sizeof(buf) - bytesWritten , m_statusString);
	if(bytesWritten < sizeof(buf))
	{
		buf[bytesWritten++] = '\n';
	}

	// print the text to the screen
	getScreen().printToScreen(buf);
}

void Simulation::onSessionConnectFail()
{
	m_statusString = "Failed To Connect To Session";
}

void Simulation::onSessionConnectSuccess()
{
	m_statusString = "Successfully Connected To Session";
}
