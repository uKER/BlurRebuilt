// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdExample/bdExampleCommon/bdExampleCommon.h>
#include <bdConnection/bdConnectionStore.h>

#define BD_LOG_LEVEL "Reconnect test"
#define TITLE_ID (0x34)
#define WAIT_FOR_DISCONNECT

class SimpleListener : public bdConnectionListener
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum Status
		{
			UNKNOWN,
			CONNECTED,
			CONNECTION_FALIED,
			DISCONNECTED
		};

		SimpleListener()
		: m_status(UNKNOWN)
		{
		}

		Status getStatus()
		{
			return m_status;
		}

		virtual void onConnect(bdConnectionRef connection)
		{
			m_status = CONNECTED;
		}

		virtual void onConnectFailed(bdConnectionRef connection)
		{
			m_status = CONNECTION_FALIED;
		}

		virtual void onDisconnect(bdConnectionRef connection)
		{
			m_status = DISCONNECTED;
		}

		Status m_status;

};

bdBool netStartUp()
{
	bdNetImpl *const net = bdNet::getInstance();

	// try to start bdNet
	bdNetStartParams params;

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
	return ok;
}

void netShutdown()
{
	bdNetImpl *const net = bdNet::getInstance();
	net->stop();
	bdExamplePlatformInit::quit();
}

void runHost(bdGameInfoRef game)
{
	bdLANDiscoveryServer discoveryServer;
	discoveryServer.start(game);

	bdNetImpl *const net = bdNet::getInstance();
	const bdBool addedKey = net->getSocketRouter()->getKeyMap()->registerKey(game->getSecurityID(), game->getSecurityKey());

	bdBool running = true;
	while(running)
	{
		discoveryServer.update();
		net->receiveAll();
		net->dispatchAll();
		net->sendAll();
	}

	if(addedKey)
	{
		net->getSocketRouter()->getKeyMap()->unregisterKey(game->getSecurityID());
	}

}

bdBool connectAndDisconnect(bdGameInfoRef game)
{
	bdBool ok = false;

	bdNetImpl *const net = bdNet::getInstance();
	const bdBool keyAdded = net->getSocketRouter()->getKeyMap()->registerKey(game->getSecurityID(), game->getSecurityKey());
	bdConnectionRef connection = bdNet::getInstance()->getConnectionStore()->create(game->getHostAddr(), game->getSecurityID());
	if(connection.notNull())
	{
		SimpleListener listener;

		if(connection->connect())
		{
			connection->registerListener(&listener);

			do
			{
				net->receiveAll();
				net->dispatchAll();
				net->sendAll();
			}
			while(listener.getStatus() == SimpleListener::UNKNOWN);

			switch(listener.getStatus())
			{
				case SimpleListener::CONNECTED:
				{
					BD_INFO(BD_LOG_LEVEL, "Connected.");
					// if connected disconnect
					connection->disconnect();
#ifdef WAIT_FOR_DISCONNECT
					// wait for the connection to close
					while(connection->getStatus() != bdConnection::BD_DISCONNECTED)
					{
						net->receiveAll();
						net->dispatchAll();
						net->sendAll();
					}
#endif // WAIT_FOR_DISCONNECT
					BD_INFO(BD_LOG_LEVEL, "Disconnected.");

					ok = true;
				}
				break;
				case SimpleListener::CONNECTION_FALIED:
				{
					BD_INFO(BD_LOG_LEVEL, "Connection Failed.");
				}
				break;
				case SimpleListener::DISCONNECTED:
				{
					BD_INFO(BD_LOG_LEVEL, "Disconnected.");
				}
				break;
				default:
				{
					BD_ERR(BD_LOG_LEVEL, "Unknown status encountered .");
				}
				break;
			}

			connection->unregisterListener(&listener);
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Unable to initiate connection.");
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "Unable to create connection.");
	}

	if(keyAdded)
	{
		net->getSocketRouter()->getKeyMap()->unregisterKey(game->getSecurityID());
	}

	return ok;
}

int BD_CALL main(int argc , char* argv[] )
{
	bdBool run = true;
	bdUInt startCount = 0;
	while(run)
	{
		printf("\n\n***Starting %u\n", startCount);

		bdCore::init();
		{
			bdLogSubscriber logSubscriber;

			if(netStartUp())
			{
				BD_REGISTER_GAME_INFO_CLASS(bdGameInfo);

				DiscoveryManager discoveryManager;
				bdGameInfoRef game = discoveryManager.discoverLanGames(TITLE_ID);

				if(game.isNull())
				{
					logSubscriber.addChannel("warn");
					logSubscriber.addChannel("err");
					logSubscriber.addChannel("info");

					BD_INFO(BD_LOG_LEVEL, "Server with verbose logging.");

					game = discoveryManager.createGame(TITLE_ID);
					runHost(game);
				}
				else
				{
					logSubscriber.addChannel("warn");
					logSubscriber.addChannel("err");
					logSubscriber.addChannel("info/Reconnect test");

					BD_INFO(BD_LOG_LEVEL, "Client will repeatedly reconnect and restart.");

					for(bdUInt j = 0; run && (j < 3);  j++)
					{
						BD_INFO(BD_LOG_LEVEL, "\n\n***Connecting %u", j);
						run = connectAndDisconnect(game);
					}
				}
			}
			else
			{
				BD_INFO(BD_LOG_LEVEL, "Failed to start bdNet.");
				run = false;
			}

			netShutdown();
		}

		bdCore::quit();

		startCount++;
	}

	return 0;
}
