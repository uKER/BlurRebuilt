// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:


#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdAuthTicket.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>
#include <bdLobby/bdMatchMaking/bdMatchMaking.h>
#include <bdLobby/bdStats/bdStats.h>
#include <bdLobby/bdProfiles/bdProfiles.h>
#include <bdLobby/bdTeams/bdTeams.h>
#include <bdLobby/bdFriends/bdFriends.h>
#include <bdLobby/bdStorage/bdStorage.h>
#include <bdLobby/bdContentUnlock/bdContentUnlock.h>
#include <bdLobby/bdTitleUtilities/bdTitleUtilities.h>
#include <bdLobby/bdKeyArchive/bdKeyArchive.h>
#include <bdLobby/bdMessaging/bdMessaging.h>
#include <bdLobby/bdMessaging/bdLobbyMessageType.h>
#include <bdLobby/bdCounter/bdCounter.h>
#include <bdLobby/bdGroup/bdGroup.h>
#include <bdLobby/bdContentStreaming/bdContentStreaming.h>
#include <bdLobby/bdTags/bdTags.h>
#include <bdLobby/bdVoteRank/bdVoteRank.h>
#include <bdLobby/bdTwitter/bdTwitter.h>
#include <bdLobby/bdFacebook/bdFacebook.h>
#ifdef BD_PLATFORM_IPHONE
#include <bdLobby/bdApplePushNotification/bdApplePushNotification.h>
#endif
#include <bdLobby/bdLinkCode/bdLinkCode.h>
#include <bdLobby/bdDML/bdDML.h>

#define BD_LOG_LEVEL "lobby service"

BD_REGISTER_SINGLETON(bdLobbyService)

#if (!defined BD_PLATFORM_XBOX) && (!defined BD_PLATFORM_XENON)
bdBool bdLobbyService::connect(bdAddr lobbyServiceAddr, 
                                   const bdAuthInfo *authInfo,
								   const bdBool useEncryption)
{
	bdBool ok = true;
	if (authInfo == BD_NULL)
	{
		BD_ERR(BD_LOG_LEVEL, "No authentication info specified");
		ok = false;
	}
	else
	{
		bdCommonAddrRef lsgAddr;
		m_encryptedConnection = useEncryption;
		m_authInfo = *authInfo;
		lsgAddr = new bdCommonAddr(lobbyServiceAddr);
		if (m_lobbyConnection.isNull())
		{
			m_lobbyConnection = new bdLobbyConnection( lsgAddr, this);
			ok = m_lobbyConnection->connect(m_authInfo);
		}
		else
		{
			// TODO: What is the correct behaviour in this case ?
			m_lobbyConnection->disconnect();
			m_lobbyConnection = BD_NULL;
			m_lobbyConnection = new bdLobbyConnection( lsgAddr, this);
			ok = m_lobbyConnection->connect(m_authInfo);
		}
	}
	return ok;
}

bdBool bdLobbyService::connect(bdAddr lobbyServiceAddr, 
								   const bdAuthInfo *authInfo,
								   void *sendBuffer,
								   bdUWord sendBufSize,
								   void *recvBuffer, 
								   bdUWord recvBufSize,
								   const bdBool useEncryption )
{
	bdBool ok = true;
	if (authInfo == BD_NULL)
	{
		BD_ERR(BD_LOG_LEVEL, "No authentication info specified");
		ok = false;
	}
	else
	{
		// TODO: We should check that the buffer are at least at certain size, 
		// as DemoNWare will not operate correctly with really small buffers!!
		m_encryptedConnection = useEncryption;
		m_authInfo = *authInfo;
		bdCommonAddrRef lsgAddr = new bdCommonAddr(lobbyServiceAddr);
		if (m_lobbyConnection.isNull())
		{
			m_lobbyConnection = new bdLobbyConnection( lsgAddr, sendBuffer, sendBufSize, recvBuffer, recvBufSize, this);
			ok = m_lobbyConnection->connect(m_authInfo);

		}	
	}
	return ok;
}

#else
#if (defined BD_PLATFORM_XBOX) && (!defined BD_PLATFORM_XENON)
bdBool bdLobbyService::connect(	bdCommonAddrRef lobbyServiceAddr, 
									bdSecurityID &id,
									const bdAuthInfo *authInfo)
{
	bdBool ok = true;
	if (authInfo == BD_NULL)
	{
		BD_ERR(BD_LOG_LEVEL, "No authentication info specified");
		ok = false;
	}
	else
	{
		m_encryptedConnection = false;
		m_authInfo = *authInfo;
		if (m_lobbyConnection.isNull())
		{
			m_lobbyConnection = new bdLobbyConnection( lobbyServiceAddr, id, this);
			ok = m_lobbyConnection->connect(m_authInfo);
		}
	}
	return ok;
}

#elif defined BD_PLATFORM_XENON
bdBool bdLobbyService::connect(bdAddr lobbyServiceAddr,
							   const bdAuthInfo *authInfo)
{
	bdBool ok = true;
	if (authInfo == BD_NULL)
	{
		BD_ERR(BD_LOG_LEVEL, "No authentication info specified");
		ok = false;
	}
	else
	{
		m_encryptedConnection = false;
		m_authInfo = *authInfo;
		if (m_lobbyConnection.isNull())
		{
			m_lobbyConnection = new bdLobbyConnection(lobbyServiceAddr, this);
			ok = m_lobbyConnection->connect(m_authInfo);

		}
	}
	return ok;
}

bdBool bdLobbyService::connect(	bdAddr lobbyServiceAddr,  
							   bdSecurityID&,
							   const bdAuthInfo *authInfo)
{
	return connect(lobbyServiceAddr,authInfo);
}

#endif
#endif

void bdLobbyService::disconnect()
{
	if(m_lobbyConnection.notNull())
	{
		m_lobbyConnection->disconnect();
	}

	m_titleID = 0;

	cleanup();
}


void bdLobbyService::pump()
{
	
	if (!m_lobbyConnection)
	{
		return;
	}
	bdByteBufferRef message;
	bdUByte8 messageType  = 0;
	while (m_lobbyConnection->getMessageToDispatch(messageType, message))
	{
		
		switch (messageType)
		{
			case BD_LSG_SERVICE_TASK_REPLY:
			{
				BD_INFO(BD_LOG_LEVEL, "Received message of type: BD_LSG_SERVICE_TASK_REPLY");

				if(m_taskManager && message.notNull())
				{
					m_taskManager->handleTaskReply(message);
				}	
				else
				{
					BD_WARN(BD_LOG_LEVEL, "No task manager.");
				}
			}
			break;
			case BD_LOBBY_SERVICE_TASK_REPLY:
			{
				BD_INFO(BD_LOG_LEVEL, "Received message of type: BD_LOBBY_SERVICE_TASK_REPLY");

				if(m_taskManager)
				{
					m_taskManager->handleLSGTaskReply(message);
				}	
				else
				{
					BD_WARN(BD_LOG_LEVEL, "No task manager.");
				}
			}
			break;
			case BD_LOBBY_SERVICE_PUSH_MESSAGE:
			{
				BD_INFO(BD_LOG_LEVEL, "Received message of type: BD_LOBBY_SERVICE_PUSH_MESSAGE");
				handlePushMessage(message);

			}
			break;
			case BD_LOBBY_ERROR_RESULT:
			{
				bdUInt32 errorCode;
				if (message->readUInt32(errorCode))
				{
					m_errorCode = errorCode;
					BD_INFO(BD_LOG_LEVEL, "Received LSG error: %u", errorCode);
				}
				else
				{
					BD_WARN(BD_LOG_LEVEL, "Received LSG error message, but cannot read out errorCode.");
				}
			}
			break;
			case BD_LOBBY_CONNECTION_ID:
				{
					bdUInt64 connectionID;
					if (message->readUInt64(connectionID))
					{
						if (m_taskManager)
						{
							BD_INFO(BD_LOG_LEVEL, "Received LSG connection ID:" BD_UINT_64_FORMAT, connectionID);
							m_taskManager->m_connectionID = connectionID;
						}
						else
						{
							BD_WARN(BD_LOG_LEVEL, "No task manager to assign connection ID to.");
						}
					}
					else
					{
						BD_WARN(BD_LOG_LEVEL, "Received LSG connection ID header, but couldn't read ID.");
					}
				}
				break;
			default:
			{
				BD_WARN(BD_LOG_LEVEL, "Received unknown message type: %u.", messageType);
			}
		}		
	}
}

bdLobbyConnection::Status bdLobbyService::getStatus()
{
	if (!m_lobbyConnection)
	{
		return bdLobbyConnection::BD_NOT_CONNECTED;
	}
	return m_lobbyConnection->getStatus();
}

void bdLobbyService::registerEventHandler(bdLobbyEventHandler *eventHandler)
{
	m_eventHandler = eventHandler;
}

bdProfiles* bdLobbyService::getProfiles()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected.");
		return BD_NULL;
	}
	if (!m_profiles)
	{
		m_profiles = new bdProfiles(m_taskManager);
	}

	return m_profiles;
}

bdMessaging* bdLobbyService::getMessaging()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected.");
		return BD_NULL;
	}
	if (!m_messaging)
	{
		m_messaging = new bdMessaging(m_taskManager);
	}
	return m_messaging;
}


bdMatchMaking* bdLobbyService::getMatchMaking()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected.");
		return BD_NULL;
	}
	if (!m_matchMaking)
	{
		m_matchMaking = new bdMatchMaking(m_taskManager);
	}

	return m_matchMaking;
}
bdStats* bdLobbyService::getStats()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected.");
		return BD_NULL;
	}
	if (!m_statsManager)
	{
		m_statsManager = new bdStats(m_taskManager);
	}

	return m_statsManager;
}

bdTeams* bdLobbyService::getTeams()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected.");
		return BD_NULL;
	}
	if (!m_teamsManager)
	{
		m_teamsManager = new bdTeams(m_taskManager);
	}

	return m_teamsManager;
}

bdFriends* bdLobbyService::getFriends()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected.");
		return BD_NULL;
	}
	if (!m_friendsManager)
	{
		m_friendsManager = new bdFriends(m_taskManager);
	}

	return m_friendsManager;
}


bdStorage* bdLobbyService::getStorage()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected.");
		return BD_NULL;
	}
	if (!m_storageManager)
	{
		m_storageManager = new bdStorage(m_taskManager);
	}

	return m_storageManager;
}

bdContentUnlock* bdLobbyService::getContentUnlock()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected.");
		return BD_NULL;
	}
	if (!m_contentUnlockManager)
	{
		m_contentUnlockManager = new bdContentUnlock(m_taskManager);
	}

	return m_contentUnlockManager;
}

bdTitleUtilities* bdLobbyService::getTitleUtilities()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected.");
		return BD_NULL;
	}
	if (!m_titleUtilitiesManager)
	{
		m_titleUtilitiesManager = new bdTitleUtilities(m_taskManager);
	}

	return m_titleUtilitiesManager;
}

bdKeyArchive* bdLobbyService::getKeyArchive()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected.");
		return BD_NULL;
	}
	if (!m_keyArchive)
	{
		m_keyArchive = new bdKeyArchive(m_taskManager);
	}

	return m_keyArchive;
}

bdCounter* bdLobbyService::getCounter()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected.");
		return BD_NULL;
	}
	if (!m_counter)
	{
		m_counter = new bdCounter(m_taskManager);
	}

	return m_counter;
}

bdGroup* bdLobbyService::getGroup()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected.");
		return BD_NULL;
	}
	if (!m_group)
	{
		m_group = new bdGroup(m_taskManager);
	}

	return m_group;
}

bdContentStreaming* bdLobbyService::getContentStreaming()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected to LSG. Connection to Content Streaming aborted.");
		return BD_NULL;
	}
	if (!m_contentStreamingManager)
	{
		m_contentStreamingManager = new bdContentStreaming(m_taskManager);
	}

	return m_contentStreamingManager;
}

void bdLobbyService::deleteContentStreaming()
{
	if (m_contentStreamingManager)
	{
		delete m_contentStreamingManager;
		m_contentStreamingManager = BD_NULL;
	}
}

bdTags* bdLobbyService::getTags()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected.");
		return BD_NULL;
	}
	if (!m_tags)
	{
		m_tags = new bdTags(m_taskManager);
	}

	return m_tags;
}

bdVoteRank* bdLobbyService::getVoteRank()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected.");
		return BD_NULL;
	}
	if (!m_voteRankManager)
	{
		m_voteRankManager = new bdVoteRank(m_taskManager);
	}

	return m_voteRankManager;
}

bdTwitter* bdLobbyService::getTwitter()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected.");
		return BD_NULL;
	}
	if (!m_twitter)
	{
		m_twitter = new bdTwitter(m_taskManager);
	}

	return m_twitter;
}

bdFacebook* bdLobbyService::getFacebook()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected.");
		return BD_NULL;
	}
	if (!m_facebook)
	{
		m_facebook = new bdFacebook(m_taskManager);
	}

	return m_facebook;
}

#ifdef BD_PLATFORM_IPHONE
bdApplePushNotification* bdLobbyService::getApplePushNotification()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected.");
		return BD_NULL;
	}
	if (!m_applePushNotification)
	{
		m_applePushNotification = new bdApplePushNotification(m_taskManager);
	}
	
	return m_applePushNotification;
}
#endif

bdLinkCode* bdLobbyService::getLinkCode()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected.");
		return BD_NULL;
	}
	if (!m_linkCode)
	{
		m_linkCode = new bdLinkCode(m_taskManager);
	}

	return m_linkCode;
}

bdDML* bdLobbyService::getDML()
{
	if (!m_lobbyConnection || (m_lobbyConnection->getStatus() != bdLobbyConnection::BD_CONNECTED))
	{
		BD_WARN(BD_LOG_LEVEL, "Not connected.");
		return BD_NULL;
	}
	if (!m_DML)
	{
		m_DML = new bdDML(m_taskManager);
	}

	return m_DML;
}

bdRemoteTaskManager* bdLobbyService::getTaskManager()
{
	return m_taskManager;
}

bdUInt bdLobbyService::getErrorCode()
{
	return m_errorCode;
}

bdLobbyService::bdLobbyService()
: m_taskManager(BD_NULL),
  m_profiles(BD_NULL),
  m_messaging(BD_NULL),
  m_matchMaking(BD_NULL),
  m_statsManager(BD_NULL),
  m_friendsManager(BD_NULL),
  m_teamsManager(BD_NULL),
  m_storageManager(BD_NULL),
  m_contentUnlockManager(BD_NULL),
  m_titleUtilitiesManager(BD_NULL),
  m_keyArchive(BD_NULL),
  m_counter(BD_NULL),
  m_group(BD_NULL),
  m_contentStreamingManager(BD_NULL),
  m_tags(BD_NULL),
  m_voteRankManager(BD_NULL),
  m_twitter(BD_NULL),
  m_facebook(BD_NULL),
#ifdef BD_PLATFORM_IPHONE
  m_applePushNotification(BD_NULL),
#endif
  m_linkCode(BD_NULL),
  m_DML(BD_NULL),
  m_titleID(0),
  m_eventHandler(BD_NULL),
  m_errorCode(BD_NO_ERROR)
{
}


bdLobbyService::~bdLobbyService()
{
	cleanup();
}

void bdLobbyService::onConnect(bdLobbyConnectionRef connection)
{
	// Send client specifications (protocol version and receive buffer size) as a separate message
	bdByteBuffer byteBuffer(8);

	byteBuffer.write((bdUInt32)BD_PROTOCOL_VERSION);
	byteBuffer.write(connection->getReceiveBufferSize());
	connection->sendRaw(byteBuffer.getData(), 8);


	// Once we are connected we register our title id

	bdUByte8 serviceID = BD_AUTH_SERVICE_ID;
	bdBitBufferRef buffer = new bdBitBuffer(&serviceID, 8, true); 
	buffer->setTypeCheck(false);
	buffer->writeBool(true);
	buffer->setTypeCheck(true);

	// All platforms care about the title id
	buffer->writeUInt32(m_authInfo.m_titleID);

	if (m_encryptedConnection)
	{
		buffer->writeUInt32(m_authInfo.m_IVSeed);
	}
	else
	{
		buffer->writeUInt32(0);
	}
	buffer->writeBits(m_authInfo.m_data, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_TICKET_SIZE));

	connection->send(buffer->getData(), buffer->getDataSize(), static_cast<bdBool>(BD_SEND_PLAINTEXT));

#if !defined BD_PLATFORM_XBOX && !defined BD_PLATFORM_XENON
	connection->setSessionKey(m_authInfo.m_sessionKey);
#endif  //#ifndef BD_PLATFORM_XBOX

    // create a task manager
	m_taskManager = new bdRemoteTaskManager(connection, m_encryptedConnection);
	
	BD_INFO(BD_LOG_LEVEL, "Connected to MatchMaking Service.");
}


void bdLobbyService::onDisconnect(bdLobbyConnectionRef /*connection*/)
{
	BD_INFO(BD_LOG_LEVEL, "Disconnected from MatchMaking Service.");
}

void bdLobbyService::cleanup()
{	
	if (m_taskManager)
	{
		delete m_taskManager;
		m_taskManager = BD_NULL;
	}
	if (m_profiles)
	{
		delete m_profiles;
		m_profiles = BD_NULL;
	}
	if (m_messaging)
	{
		delete m_messaging;
		m_messaging = BD_NULL;
	}
	
	if (m_matchMaking)
	{
		delete m_matchMaking;
		m_matchMaking = BD_NULL;
	}

	if (m_statsManager)
	{
		delete m_statsManager;
		m_statsManager = BD_NULL;
	}

	if (m_friendsManager)
	{
		delete m_friendsManager;
		m_friendsManager = BD_NULL;
	}

	if (m_teamsManager)
	{
		delete m_teamsManager;
		m_teamsManager = BD_NULL;
	}

	if (m_storageManager)
	{
		delete m_storageManager;
		m_storageManager = BD_NULL;
	}

	if (m_contentUnlockManager)
	{
		delete m_contentUnlockManager;
		m_contentUnlockManager = BD_NULL;
	}

	if (m_titleUtilitiesManager)
	{
		delete m_titleUtilitiesManager;
		m_titleUtilitiesManager = BD_NULL;
	}

	if (m_keyArchive)
	{
		delete m_keyArchive;
		m_keyArchive = BD_NULL;
	}

	if (m_counter)
	{
		delete m_counter;
		m_counter = BD_NULL;
	}

	if (m_group)
	{
		delete m_group;
		m_group = BD_NULL;
	}

	if (m_contentStreamingManager)
	{
		delete m_contentStreamingManager;
		m_contentStreamingManager = BD_NULL;
	}
	if (m_tags)
	{
		delete m_tags;
		m_tags = BD_NULL;
	}
	if (m_voteRankManager)
	{
		delete m_voteRankManager;
		m_voteRankManager = BD_NULL;
	}
	if (m_twitter)
	{
		delete m_twitter;
		m_twitter = BD_NULL;
	}
	if (m_facebook)
	{
		delete m_facebook;
		m_facebook = BD_NULL;
	}
#ifdef BD_PLATFORM_IPHONE
	if (m_applePushNotification)
	{
		delete m_applePushNotification;
		m_applePushNotification = BD_NULL;
	}
#endif
	if (m_linkCode)
	{
		delete m_linkCode;
		m_linkCode = BD_NULL;
	}
	if (m_DML)
	{
		delete m_DML;
		m_DML = BD_NULL;
	}
	m_lobbyConnection = BD_NULL;
}

void bdLobbyService::handlePushMessage(bdByteBufferRef message)
{
	if (!m_eventHandler)
	{
		return;
	}
	bdUInt msgType = 0;
	bdBool ok = message->readUInt32(msgType);
	bdEventType eventType = static_cast<bdEventType>(msgType);
	switch (eventType)
	{
		case BD_NEW_NOTIFICATION:
		{
			m_eventHandler->onNewNotification();
		}
		break;
		case BD_FRIENDSHIP_PROPOSAL:
		{
			m_eventHandler->onFriendshipProposal();
		}
		break;
		
		case BD_TEAM_PROPOSAL:
		{
			m_eventHandler->onTeamProposal();
		}
		break;
		case BD_FRIEND_CONNECTED:
		case BD_FRIEND_DISCONNECTED:
		{
			bdUInt64 friendUID=0;
			ok = ok && message->readUInt64(friendUID);

			bdNChar8 friendName[BD_MAX_USERNAME_LENGTH+1];
			bdMemset(friendName,0, sizeof(friendName));
			ok = ok && message->readString(friendName, BD_MAX_USERNAME_LENGTH);

			if (eventType == BD_FRIEND_CONNECTED)
			{
				m_eventHandler->onPlayerStatusUpdate(friendUID, friendName, BD_STATUS_ONLINE);
			}
			else
			{
				m_eventHandler->onPlayerStatusUpdate(friendUID, friendName, BD_STATUS_OFFLINE);
			}
		}
		break;
		case BD_TEAMMEMBER_CONNECTED:
		case BD_TEAMMEMBER_DISCONNECTED:
		{
			bdUInt64 senderUID=0;
			ok = ok && message->readUInt64(senderUID);

			bdNChar8 senderName[BD_MAX_USERNAME_LENGTH+1];
			bdMemset(senderName,0, sizeof(senderName));
			ok = ok && message->readString(senderName, BD_MAX_USERNAME_LENGTH);
			
			bdTeamID teamID=0;
			ok = ok && message->readUInt64(teamID);

			bdNChar8 teamName[BD_MAX_TEAMNAME_LENGTH+1];
			bdMemset(teamName,0, sizeof(teamName));
			ok = ok && message->readString(teamName, BD_MAX_TEAMNAME_LENGTH);
		
			
			if (eventType == BD_TEAMMEMBER_CONNECTED)
			{
				m_eventHandler->onTeamMemberStatusUpdate(senderUID, senderName, teamID, teamName, BD_STATUS_ONLINE);
			}
			else
			{
				m_eventHandler->onTeamMemberStatusUpdate(senderUID, senderName, teamID, teamName, BD_STATUS_OFFLINE);
			}
		}
		break;
		case BD_SESSION_INVITATION:
		{
			
			bdSessionInvite invite;
			ok = invite.deserialize(message);
			if(true == ok)
			{
				m_eventHandler->onSessionInvite(invite);
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL, "Failed to read session invite.");
			}	
		}
		break;
		case BD_CHANNEL_CHAT_BROADCAST_MSG:
		case BD_CHANNEL_CHAT_WHISPER_MSG:
		{
			bdUInt64 senderUID = 0;
			ok = ok && message->readUInt64(senderUID);
			bdNChar8 senderName[BD_MAX_USERNAME_LENGTH];
			ok = ok && message->readString(senderName, BD_MAX_USERNAME_LENGTH);
			bdUInt64 channelID=0;
			ok = ok && message->readUInt64(channelID);
			bdUByte8 msg[BD_MAX_CHAT_MESSAGE_SIZE];
			bdUInt32 msgLength = BD_MAX_CHAT_MESSAGE_SIZE;
			bdMemset(msg, 0, BD_MAX_CHAT_MESSAGE_SIZE);
			ok = ok && message->readBlob(msg, msgLength);
			m_eventHandler->onChatChannelMessage(channelID, senderUID, senderName, msg, msgLength);
		}
		break;
		case BD_CHANNEL_USER_SUBSCRIBED:
		case BD_CHANNEL_USER_UNSUBSCRIBED:
		{
			bdUInt64 senderID =0; 
			ok = ok && message->readUInt64(senderID);
			bdNChar8 senderName[BD_MAX_USERNAME_LENGTH];
			ok = ok && message->readString(senderName, BD_MAX_USERNAME_LENGTH);
			bdUInt64 channelID=0;
			ok = ok && message->readUInt64(channelID);
			bdByte8 status = (eventType == BD_CHANNEL_USER_SUBSCRIBED) ? BD_USER_SUBSCRIBED : BD_USER_UNSUBSCRIBED;
			m_eventHandler->onChatChannelUpdate(channelID, senderID, senderName, status);
		}
		break;
		case BD_FRIEND_CHAT_MSG:
		{
			bdUInt64 senderID =0; 
			ok = ok && message->readUInt64(senderID);
			bdNChar8 senderName[BD_MAX_USERNAME_LENGTH];
			bdMemset(senderName, 0, BD_MAX_USERNAME_LENGTH);
			ok = ok && message->readString(senderName, BD_MAX_USERNAME_LENGTH);
			
			bdUByte8 msg[BD_MAX_CHAT_MESSAGE_SIZE];
			bdUInt32 msgLength = BD_MAX_CHAT_MESSAGE_SIZE;
			bdMemset(msg, 0, BD_MAX_CHAT_MESSAGE_SIZE);
			ok = ok && message->readBlob(msg, msgLength);
			m_eventHandler->onInstantMessage(senderID, senderName, msg, msgLength);
		}
		break;
		case BD_TEAM_CHAT_MSG:
		{
			bdUInt64 senderID =0; 
			ok = ok && message->readUInt64(senderID);
			bdNChar8 senderName[BD_MAX_USERNAME_LENGTH];
			bdMemset(senderName, 0, BD_MAX_USERNAME_LENGTH);
			ok = ok && message->readString(senderName, BD_MAX_USERNAME_LENGTH);
			
			bdTeamID teamID=0;
			ok = ok && message->readUInt64(teamID);

			bdNChar8 teamName[BD_MAX_TEAMNAME_LENGTH+1];
			bdMemset(teamName,0, sizeof(teamName));
			ok = ok && message->readString(teamName, BD_MAX_TEAMNAME_LENGTH);

			bdUByte8 msg[BD_MAX_CHAT_MESSAGE_SIZE];
			bdUInt32 msgLength = BD_MAX_CHAT_MESSAGE_SIZE;
			bdMemset(msg, 0, BD_MAX_CHAT_MESSAGE_SIZE);
			ok = ok && message->readBlob(msg, msgLength);
			m_eventHandler->onInstantMessage(senderID, senderName, msg, msgLength, teamID);
		}
		break;
		case BD_FRIEND_RICH_PRESENCE_UPDATED:
		{
			bdUInt64 senderID =0; 
			ok = ok && message->readUInt64(senderID);
			bdNChar8 senderName[BD_MAX_USERNAME_LENGTH];
			bdMemset(senderName, 0, BD_MAX_USERNAME_LENGTH);
			ok = ok && message->readString(senderName, BD_MAX_USERNAME_LENGTH);
			bdUByte8 richPresence[BD_MAX_RICH_PRESENCE_UPDATE];
			bdUInt richPresenceSize = BD_MAX_RICH_PRESENCE_UPDATE;
			ok = ok && message->readBlob(richPresence, richPresenceSize);
			m_eventHandler->onRichPresenceUpdate(senderID, senderName, richPresence, richPresenceSize);
		}
		break;
		case BD_TEAM_RICH_PRESENCE_UPDATED:
		{
			bdTeamID teamID = 0;
			ok = ok && message->readUInt64(teamID);
			bdUInt64 teammateUID =0; 
			ok = ok && message->readUInt64(teammateUID);
			bdNChar8 senderName[BD_MAX_USERNAME_LENGTH];
			bdMemset(senderName, 0, BD_MAX_USERNAME_LENGTH);
			ok = ok && message->readString(senderName, BD_MAX_USERNAME_LENGTH);
			bdUByte8 richPresence[BD_MAX_RICH_PRESENCE_UPDATE];
			bdUInt richPresenceSize = BD_MAX_RICH_PRESENCE_UPDATE;
			ok = ok && message->readBlob(richPresence, richPresenceSize);
			m_eventHandler->onRichPresenceUpdate(teammateUID, senderName, richPresence, richPresenceSize, teamID);
		}
		break;
		case BD_NOTIFY_LEAVE:
		{
			bdOnlineUserID userID = 0;
			ok = ok && message->readUInt64(userID);
			// Sender name will be empty for this type of notification
			bdNChar8 senderName[BD_MAX_USERNAME_LENGTH];
			bdMemset(senderName, 0, BD_MAX_USERNAME_LENGTH);
			ok = ok && message->readString(senderName, BD_MAX_USERNAME_LENGTH);
			bdSessionID sessionID;
			bdUInt length = BD_SECURITY_ID_LENGTH;
			ok = ok && message->readBlob(&sessionID.m_sessionID.ab[0], length);
			if(true == ok)
			{
				m_eventHandler->onNotifyLeave(userID, sessionID);
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL, "Failed to read sessionID for session invite.");
			}

		}
		break;
		case BD_NEW_MAIL:
		{
			m_eventHandler->onNewMail();
		}
		break;
		case BD_MULTIPLE_LOGONS:
		{
			bdUInt64 recipientID = 0;
			ok = ok && message->readUInt64(recipientID);
			m_eventHandler->onMultipleLogon(recipientID);
		}
		break;
	}
}

