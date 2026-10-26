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
#include "GameInfo.h"

#define TITLE_ID (0x10000001)

#define SEND_INTERVAL (0.1f)

#define BD_ADD_PLAYER	BD_BUTTON1
#define BD_REM_PLAYER	BD_BUTTON2
#define BD_START_ROUND	BD_BUTTON3
#define BD_END_ROUND	BD_BUTTON4
#define BD_CHANGE_NAME	BD_BUTTON5

const bdNChar8* Simulation::m_screenInstructions =	"Add Player: " BD_BUTTON1_DESC "/ "
													"Remove Player: " BD_BUTTON2_DESC "/ "
													"Start Round: " BD_BUTTON3_DESC "\n"
													"End Round: " BD_BUTTON4_DESC "/ "
													"Alternate Name: " BD_BUTTON5_DESC "\n \n";

// set the command step to be the same as the send interval, this will be the
// case given perfect network conditions so is the best we can hope for.
#define COMMAND_STEP SEND_INTERVAL

#define BD_LOG_LEVEL "simulation"

bdBool Simulation::init(bdInt32 argc,
						bdNChar8* argv[])
{
	// initialize our member variables
	m_discoveryServer = BD_NULL;
	m_peer = BD_NULL;
	m_name = "Unnamed";


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

		BD_REGISTER_GAME_INFO_CLASS(GameInfo);
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

			// cast to the appropriate derived type
			const GameInfoRef myGame = reference_cast<GameInfo>(game);

			bdNChar8 name[] = "A Game To Be Proud Of.";
			bdMemcpy(myGame->m_gameName , name, sizeof(name));
			m_alternativeGameNameFlag = true;

			// create a discovery server to advertise the game
			m_discoveryServer = new bdLANDiscoveryServer;
			m_discoveryServer->start(game);
		}

		// register the classes that BitDemon will have to create
		BD_REGISTER_PLAYER_CLASS(Player);
		BD_REGISTER_COMMAND_CLASS(UserCommand);
		BD_REGISTER_SESSION_DATA_CLASS(bdSessionData);

		// Register the game secID before using it.
		bdSecurityKeyMap *const keyMap = net->getSocketRouter()->getKeyMap();

		const bdBool keyAdded = keyMap->registerKey(game->getSecurityID(), game->getSecurityKey());
		if(true == keyAdded)
		{

			// create a peer and connect to the host
			// register ourselves as the game interface
			m_peer = new bdPeer(this);
			m_peer->connect(game);

			m_game = game;

			// register ourselves as a player listener so we can respond
			// to players joining and leaving
			m_peer->getPlayerManager()->registerListener(this);

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
	const bdBool addPlayerKey = getScreen().wasKeyPressed(BD_ADD_PLAYER);
	const bdBool removePlayerKey = getScreen().wasKeyPressed(BD_REM_PLAYER);
	const bdBool startRoundKey = getScreen().wasKeyPressed(BD_START_ROUND);
	const bdBool endRoundKey = getScreen().wasKeyPressed(BD_END_ROUND);
	const bdBool alternateNameKey = getScreen().wasKeyPressed(BD_CHANGE_NAME);

	bdNetImpl *const net = bdNet::getInstance();

	// receive messages from the network and dispatch them
	net->receiveAll();
	net->dispatchAll();


	// pump the discovery server
	if(m_discoveryServer)
	{
		m_discoveryServer->update();
	}

	if(startRoundKey)
	{
		m_peer->startRound();
	}

	if(endRoundKey)
	{
		m_peer->endRound();
	}

	if(addPlayerKey)
	{
		Player player(m_name);
		m_peer->addPlayer(player);
	}

	if(removePlayerKey)
	{
		removePlayer();
	}

	if(alternateNameKey)
	{
		alternateGameName();
		}

	// if the peer is ready add commands ready to be sent
	// this will generate a command each frame, you may want to limit
	// commands to one per send interval, to do this use the return value
	// of bdPeer::step to trigger adding a new command
	if(m_peer->okToSendCommands())
	{
		generateCommands();
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
		m_peer->getPlayerManager()->unregisterListener(this);
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

void Simulation::removePlayer()
{
	// there can be multiple players per peer if we are told to remove one
	// just pick the first we come across.
	bdBool removed = false;
	bdPlayerManager *const playerManager = m_peer->getPlayerManager();
	const bdUInt numPlayers = playerManager->getNumPlayers();
	for(bdUInt i=0; !removed && (i < numPlayers); i++)
	{
		const bdPlayerRef player = playerManager->getPlayer(i);

		// it is possible to remove players created by other peers
		// so check this player belongs to us
		if(player->isLocal())
		{
			removed = true;
			playerManager->remove(player);
		}
	}
}

bdBool Simulation::generateCommands()
{
	bdBool commandGenerated = false;

	UserCommand command;

	// sample the inputs
	// note that we only add a command if one is generated to save bandwidth
	if(getUserInputs(command))
	{
		commandGenerated = true;
		// there can be multiple players per peer so just apply the command to
		// all local players
		const bdPlayerManager *const playerManager = m_peer->getPlayerManager();
		const bdUInt numPlayers = playerManager->getNumPlayers();
		for(bdUInt i=0; i < numPlayers; i++)
		{
			const bdPlayerRef player = playerManager->getPlayer(i);
			if(player->isLocal())
			{
				m_peer->addCommand(player, command);

				// as the commands are very simple and there is no interaction
				// between players we can preform client side prediction here by
				// applying the command immediately

				// cast to the appropriate derived type
				const PlayerRef myPlayer = reference_cast<Player>(player);
				myPlayer->applyCommand(&command, COMMAND_STEP);
			}
		}
	}

	return commandGenerated;
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

void Simulation::onRoundStart(const bdBitBufferRef userData)
{
	// create a sphere for each player
	const bdPlayerManager *const playerManager = m_peer->getPlayerManager();
	bdRandom *const random = m_peer->getRound()->getRandom();
	const bdUInt numPlayers = playerManager->getNumPlayers();
	for(bdUInt i=0; i < numPlayers; i++)
	{
		const bdPlayerRef player = playerManager->getPlayer(i);
		createSphere(player, random);
	}
}

void Simulation::onRoundEnd()
{
	// remove the spheres for all players
	const bdPlayerManager *const playerManager = m_peer->getPlayerManager();
	const bdUInt numPlayers = playerManager->getNumPlayers();
	for(bdUInt i=0; i < numPlayers; i++)
	{
		const bdPlayerRef player = playerManager->getPlayer(i);
		// cast to the appropriate derived type
		const PlayerRef myPlayer = reference_cast<Player>(player);
		myPlayer->destroySphere();
	}
}

void Simulation::processCommand(const bdPlayerRef player,
								const bdCommandRef command)
{
	// a command has arrived so get the player to process it
	// in this example players process commands directly as they
	// can only ever control a sphere, if you want to switch the
	// objects/avatars players control retrieve the object from
	// the player and allow it to process the command

	// only apply commands from remote players as local player commands
	// have already been applied as client side pediction
	if(!player->isLocal())
	{
		const PlayerRef myPlayer = reference_cast<Player>(player);
		UserCommandRef myCommand = reference_cast<UserCommand>(command);
		myPlayer->applyCommand(myCommand, COMMAND_STEP);
	}
}

void Simulation::onPlayerJoin(const bdPlayerRef player)
{
	// a new player has join if we are mid-round create sphere
	if(m_peer->getRound()->getStatus() == bdRound::BD_IN_ROUND)
	{
		bdRandom *const random = m_peer->getRound()->getRandom();
		createSphere(player, random);
	}
}

void Simulation::createSphere(const bdPlayerRef player,
							  bdRandom *const rand) const
{
	// get a random position in the range [-4.5, 4.5] the crude way
	const bdFloat32 pos[2] = {-4.5f + (rand->nextUInt() % 10),
							  -4.5f + (rand->nextUInt() % 10)};

	// get a random color
	bdVector4 color;
	color.setX(static_cast<bdFloat32>(rand->nextUInt())/BD_UINT32_MAX);
	color.setY(static_cast<bdFloat32>(rand->nextUInt())/BD_UINT32_MAX);
	color.setZ(static_cast<bdFloat32>(rand->nextUInt())/BD_UINT32_MAX);
	color.setW(1);

	// cast to the appropriate derived type
	const PlayerRef myPlayer = reference_cast<Player>(player);

	// create the sphere for the player to control
	myPlayer->createSphere(pos, color);
}

void Simulation::displayInfo()
{
	bdNChar8 buf[512];
	bdUInt bytesWritten = 0;
	bytesWritten = static_cast<bdUInt>(bdStrlcpy(buf, m_screenInstructions, 512));

	// cast to the appropriate derived type
	const GameInfoRef myGame = reference_cast<GameInfo>(m_game);

	bdMemcpy(buf, &(myGame->m_gameName[0]) , GAME_NAME_SIZE);
	bytesWritten += GAME_NAME_SIZE;

	// get info about the state of the round
	bytesWritten += bdRoundInfo::getInfo(m_peer->getRound(), buf + bytesWritten, sizeof(buf) - bytesWritten);
	if(bytesWritten < sizeof(buf))
	{
		buf[bytesWritten++] = '\n';
	}

	// get a list of players
	bytesWritten += getPlayerInfo(buf + bytesWritten, sizeof(buf) - bytesWritten);

	// print the text to the screen
	getScreen().printToScreen(buf);
}

bdUInt Simulation::getPlayerInfo(bdNChar8 *buf,
								 const bdUInt length) const
{
	// iterate all players and print their details to the string
	bdNChar8 *const end = buf + length;
	bdPlayerManager *const playerManager = m_peer->getPlayerManager();

	for(bdUInt i = 0; i < playerManager->getNumPlayers(); i++)
	{
		const PlayerRef player = reference_cast<Player>(playerManager->getPlayer(i));
		const bdNChar8 *const local = "local";
		const bdNChar8 *const remote = "remote";

		const bdNChar8 *location = remote;
		if(player->isLocal())
		{
			location = local;
		}

		bdWord spaceRemaining = end - buf;
		if (spaceRemaining <= 0 )
		{
			break;
		}

		const bdInt written = bdSnprintf(buf, spaceRemaining, "%u %s(%u) %s\n", i, player->getName().getBuffer(), player->getId(), location);
		if(written > 0)
		{
			buf += written;
		}
		else
		{
			buf = end - 1;
		}
	}

	if(length > 0)
	{
		*buf = '\0';
	}

	const bdUInt bytesWritten = length - (end - buf);
	return bytesWritten;
}

void Simulation::alternateGameName()
{
	if(BD_NULL != m_discoveryServer)
	{
		bdGameInfoRef game = m_discoveryServer->getGameInfo();
		const GameInfoRef myGame = reference_cast<GameInfo>(game);
		if(false == m_alternativeGameNameFlag)
		{
			bdNChar8 name[] = "A Game To Be Proud Of.";
			bdMemcpy(myGame->m_gameName , name, sizeof(name));
		}
		else
		{
			bdNChar8 name[] = "A Game Of Distinction.";
			bdMemcpy(myGame->m_gameName , name, sizeof(name));
		}
		m_alternativeGameNameFlag = !m_alternativeGameNameFlag;
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Only the host can change the advertised game name.");
	}
}
