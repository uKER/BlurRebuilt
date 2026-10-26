#include <StdAfx.h>
#include <Network/Core.h>
#include <Network/Session.h>
#include <Network/Securekey.h>
#include <systemprofile/SystemProfile.h>
#include <systemprofile/SystemProfileManager.h>
#include <systemprofile/GameInvite.h>
#include <Network/SessionParameter.h>
#include <Network/SessionEnumerator.h>
#include <Network/Demonwaretask.h>
#include <Network/SessionDeletion.h>
#include <bdNet/bdNet.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdMatchMaking/bdMatchMaking.h>
#include <bdSocket/bdQoS/bdQoSProbe.h>
#include <bdSocket/bdSocketRouter.h>
#include <Network/DemonwareSession.h>
#include <bdCore/bdTiming/bdStopwatch.h>
#include "../Shared/SessionCommand_demonware.h"

namespace NetPlay
{
	// constructor
	C_Session::C_Session()	:	m_Initialised(false), m_SessionState(eStateNotActive),
								m_SessionParameters(0), m_PublicSlots(0), m_PrivateSlots(0), m_UserIndex(0),
								m_CommandStorageSize(0), m_SessionCreated(false),
								m_Locked(false), m_QoSEnabled(false), m_pCommandStorage(NULL), m_pCommandPool(NULL), m_pCommandList(NULL), 
								m_Migrating(false), m_pDemonwareSession(NULL)
	{
	}

	// destructor
	C_Session::~C_Session()
	{
		CleanUp();
	}


	// attempt to initialize the object.
	bool C_Session::Initialise(const U32 maxPlayers, const U32 commandQueueSize)
	{
		if (m_Initialised)
			return true;

		if (0 == maxPlayers)
			return false;

		// store the maximun number of players we can create a session for..
		m_MaxPlayers = maxPlayers;

		// if we request 0 commands then allocate a default amount.
		m_CommandStorageSize		= (commandQueueSize != 0) ? commandQueueSize : 32;

		//allocate commands storage and linked lists..
		m_pCommandStorage = new C_SessionCommand[m_CommandStorageSize];
		m_pCommandPool = reinterpret_cast<void*> (new C_SessionCommandList);
		m_pCommandList = reinterpret_cast<void*> (new C_SessionCommandList);

		// allocate arbitration storage buffers..
		if (m_pCommandStorage && m_pCommandPool && m_pCommandList)
		{
			// put all normal command objects into pool 
			C_SessionCommandList* pPool = reinterpret_cast<C_SessionCommandList*> (m_pCommandPool);
			for (U32 i=0; i< m_CommandStorageSize; i++)
				pPool->PushBack(m_pCommandStorage[i]);

			m_Initialised = true;
		}
		else
			CleanUp();

		return m_Initialised;
	}

	bool C_Session::SetParameter(const U8 userIndex, const SessionParameter& data)
	{
		if (!m_Initialised)
			return false;

		if (NULL == m_pDemonwareSession)
			return false;

		bool paramSet = false;
		if (SessionParameter::eTypeProperty == data.m_ParameterType)
		{
			// property Id on PC is the array index within the demonware server matchmaking attributes.
			// the index is taken from what order the parameters appear in the server config IMP file.
			m_pDemonwareSession->m_Attributes[data.m_Data.m_PropertyData.m_Id] = data;
			paramSet = true;
		}
		else
		{
			LOGFATAL(g_Log, "C_Session::SetParameter() - Unable to use contexts on PC. All parameters must be configured as properties.");
		}
		return paramSet;
	}

	bool C_Session::Poll(void)
	{
		if (!m_Initialised)
			return true;

		if (eStateError != m_SessionState)
		{
			// next process any normal session commands..
			C_SessionCommandList* pList = reinterpret_cast<C_SessionCommandList*> (m_pCommandList);
			C_SessionCommandList* pPool = reinterpret_cast<C_SessionCommandList*> (m_pCommandPool);
			if (!pList->Empty())
			{
				bool startNextCommand = false;
				bool handleTaskError = false;
				C_SessionCommandList::Iterator i = pList->Begin();
				if (i->m_Active)
				{
					bool errorOccurred = false;
					// command is currently active so pump asynchronous task..
					if (!NetPlay::DemonwareRemoteTask::IsBusy(i->m_AsyncTask))
					{
						errorOccurred = (NetPlay::DemonwareRemoteTask::CheckState(i->m_AsyncTask) == NetPlay::DemonwareRemoteTask::eFailed) ? true : false;
						bdNetImpl *const net = bdNet::getInstance();
						bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();	
						bdMatchMaking* pMM = NULL;
						if (lobbyService)
							pMM = lobbyService->getMatchMaking();
						else
							errorOccurred = true;
						if (!errorOccurred && pMM)
						{
							switch (i->m_Command)
							{
							case C_SessionCommand::eCommandCreateSession:
								{
									m_SessionState		= eStateRunning;	// yippee session is now available..
									m_SessionCreated	= true;				// we must delete the session and not just close the handle..

									// register key to allow secure packet transfer.
									bdSecurityKeyMap* const keymap = net->getSocketRouter()->getKeyMap();
									assert(keymap);
									bdSecurityKey key;
									for (U32 z=0; z<BD_SECURITY_KEY_LENGTH; z++)
										key.ab[z] = m_pDemonwareSession->m_SecurityKey[z];

									// copy session id from tempory creation storage into actuall session.
									for (U32 z=0; z<BD_SECURITY_ID_LENGTH; z++)
										m_pDemonwareSession->m_sessionID.ab[z] = m_SessionId.m_sessionID.ab[z];

									// register security information.
									bool reg = keymap->registerKey(m_pDemonwareSession->m_sessionID, key);
									assert(reg);
								}
								break;

							case C_SessionCommand::eCommandAddPlayer:
								{
									SessionCommandNotification* p = reinterpret_cast<SessionCommandNotification*> (i->m_CommandData.m_Add.m_pNotification);
									if (p)
									{
										p->Reset();
										p->m_Finished = true;
										p->m_Id = i->m_CommandData.m_Add.m_NotificationId;
									}
								}
								break;

							case C_SessionCommand::eCommandRemovePlayer:
								{
									SessionCommandNotification* p = reinterpret_cast<SessionCommandNotification*> (i->m_CommandData.m_Remove.m_pNotification);
									if (p)
									{
										p->Reset();
										p->m_Finished = true;
										p->m_Id = i->m_CommandData.m_Remove.m_NotificationId;
									}
								}
								break;

							case C_SessionCommand::eCommandModifySession:
								{
									SessionCommandNotification* p = reinterpret_cast<SessionCommandNotification*> (i->m_CommandData.m_Modify.m_pNotification);
									if (p)
									{
										p->Reset();
										p->m_Finished = true;
										p->m_Id = i->m_CommandData.m_Modify.m_NotificationId;
									}
								}
								break;

							case C_SessionCommand::eCommandMigrateSession:
								{
									// register key to allow secure packet transfer.
									bdSecurityKeyMap* const keymap = net->getSocketRouter()->getKeyMap();
									assert(keymap);
					
									bdSecurityKey key;
									for (U32 z=0; z<BD_SECURITY_KEY_LENGTH; z++)
										key.ab[z] = m_pDemonwareSession->m_SecurityKey[z];

									// copy session id from tempory creation storage into actuall session.
									for (U32 z=0; z<BD_SECURITY_ID_LENGTH; z++)
										m_pDemonwareSession->m_sessionID.ab[z] = m_SessionId.m_sessionID.ab[z];

									// register security information.
									bool reg = keymap->registerKey(m_pDemonwareSession->m_sessionID, key);
									assert(reg);

									m_SessionCreated = true; // we must delete the session and not just close the handle..

									SessionCommandNotification* p = reinterpret_cast<SessionCommandNotification*> (i->m_CommandData.m_Migrate.m_pNotification);
									if (p)
									{
										p->Reset();
										p->m_Finished = true;
										p->m_Id = i->m_CommandData.m_Migrate.m_NotificationId;
									}
								}

								m_Migrating	= false;
								break;
							}

							// if everything is ok then close the task and give it back to the pool
							// if an error has occured in processing the task it will  be cleaned up below.
							if (!errorOccurred)
							{
								NetPlay::DemonwareRemoteTask::Close(i->m_AsyncTask);
								C_SessionCommandList::Iterator cur = i;
								i = pList->Erase(cur);
								pPool->PushBack(*cur);
								startNextCommand = true;
							}
						}

						if (errorOccurred || NULL == pMM)
						{
							if (NULL == pMM)
							{
								LOGWARN( g_Log, "C_Session : Command finished but bdMatchmaking is NULL !");
							}
							else
							{
								if (i->m_AsyncTask.notNull())
								{
									bdLobbyErrorCode ec = i->m_AsyncTask->getErrorCode();
									LOGWARN( g_Log, "C_Session : Command failed. Type :%d Error %d", i->m_Command, ec);
								}
								else
								{
									LOGWARN( g_Log, "C_Session : Command finished but task is NULL !");
								}
							}

							// crap.. command failed. 
							NetPlay::DemonwareRemoteTask::Close(i->m_AsyncTask);

							m_SessionState = eStateError;	// stop us adding new commands to the queue..

							// give all active commands back to the pool..
							while (!pList->Empty())
							{
								C_SessionCommandList::Iterator f = pList->Begin();
								pList->Erase(f);
								pPool->PushBack(*f);
							}
						}
					}
				}
				else
					startNextCommand = true;

				if (startNextCommand)
				{
					if (!pList->Empty())
					{
						bdNetImpl *const net = bdNet::getInstance();
						bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
						bdMatchMaking* pMM = NULL;
						if (lobbyService)
							pMM = lobbyService->getMatchMaking();
						if (pMM)
						{
							i = pList->Begin();
							switch (i->m_Command)
							{
							case C_SessionCommand::eCommandCreateSession:
								if (net->getStatus() == bdNetImpl::BD_NET_DONE && lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
								{
									if (C_SessionDeletion::Instance().ActiveCount() == 0)
									{
										// create a random key
										bdTrulyRandom::getInstance()->getRandomUByte8(m_pDemonwareSession->m_SecurityKey, BD_SECURITY_KEY_LENGTH);

										// zero key id as this will be created by the server
										for (U32 z=0; z<BD_SECURITY_ID_LENGTH; z++)
											m_SessionId.m_sessionID.ab[z] = 0;

										i->m_AsyncTask = pMM->createSession(m_pDemonwareSession, &m_SessionId);
										if (NetPlay::DemonwareRemoteTask::IsBusy(i->m_AsyncTask))
											i->m_Active = true;
										else
											m_SessionState = eStateError; // flag error.. no more commands will work in this state
									}
									else
									{
										LOGWARN(g_Log, "C_Session::Poll() - Delaying session creation until previous sessions are deleted.");
									}
								}
								else
									m_SessionState = eStateError; // flag error.. no more commands will work in this state
								break;

							case C_SessionCommand::eCommandAddPlayer:
								{
									if (net->getStatus() == bdNetImpl::BD_NET_DONE && lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
									{
										i->m_AsyncTask = pMM->notifyJoin(m_SessionId, i->m_CommandData.m_Add.m_Users, i->m_CommandData.m_Add.m_NumToAdd);
										if (NetPlay::DemonwareRemoteTask::IsBusy(i->m_AsyncTask))
											i->m_Active = true;
										else
											m_SessionState = eStateError;
									}
									else
										m_SessionState = eStateError;
								}
								break;

							case C_SessionCommand::eCommandRemovePlayer:
								{
									if (net->getStatus() == bdNetImpl::BD_NET_DONE && lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
									{
										i->m_AsyncTask = pMM->notifyLeave(m_SessionId, i->m_CommandData.m_Remove.m_Users, i->m_CommandData.m_Remove.m_NumToRemove);
										if (NetPlay::DemonwareRemoteTask::IsBusy(i->m_AsyncTask))
											i->m_Active = true;
										else
											m_SessionState = eStateError;
									}
									else
										m_SessionState = eStateError;
								}
								break;

							case C_SessionCommand::eCommandModifySession:
								if (net->getStatus() == bdNetImpl::BD_NET_DONE && lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
								{
									i->m_AsyncTask = pMM->updateSession(m_SessionId, m_pDemonwareSession);
									if (NetPlay::DemonwareRemoteTask::IsBusy(i->m_AsyncTask))
										i->m_Active = true;
									else
										m_SessionState = eStateError; // flag error.. no more commands will work in this state
								}
								else
									m_SessionState = eStateError; // flag error.. no more commands will work in this state
								break;

							case C_SessionCommand::eCommandMigrateSession:
								if (net->getStatus() == bdNetImpl::BD_NET_DONE && lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
								{
									if (C_SessionDeletion::Instance().ActiveCount() == 0)
									{
										// create a random key
										bdTrulyRandom::getInstance()->getRandomUByte8(m_pDemonwareSession->m_SecurityKey, BD_SECURITY_KEY_LENGTH);

										// zero key id as this will be created by the server
										for (U32 z=0; z<BD_SECURITY_ID_LENGTH; z++)
											m_SessionId.m_sessionID.ab[z] = 0;

										i->m_AsyncTask = pMM->createSession(m_pDemonwareSession, &m_SessionId);
										if (NetPlay::DemonwareRemoteTask::IsBusy(i->m_AsyncTask))
											i->m_Active = true;
										else
											m_SessionState = eStateError; // flag error.. no more commands will work in this state
									}
									else
									{
										LOGWARN(g_Log, "C_Session::Poll() - Delaying session migration until previous sessions are deleted.");
									}
								}
								else
									m_SessionState = eStateError; // flag error.. no more commands will work in this state
								break;

							default:
								LOGFATAL( g_Log, "C_Session::Poll() - Invalid Command Type : %d", i->m_Command);
								break;
							}
						}
						else
						{
							LOGWARN(g_Log, "C_Session::Poll() - lobbyService->getMatchMaking() returned NULL");
							m_SessionState = eStateError; // flag error.. no more commands will work in this state
						}
					}
				}
			}
		}

		return m_SessionState != eStateError;
	}

	bool C_Session::Create(const U8 userIndex, const U32 sessionFlags, const SessionParameter* pSessionParameters, const U32 numSessionParameters, const SlotConfig& slotConfig)
	{
		if (!m_Initialised)
			return false;

		if (!C_SessionDeletion::Instance().IsActive())
		{
			LOGFATAL(g_Log, "can't create online sessions until the C_SessionDeletion class has been configured.");
			return false;
		}

		if (C_SessionDeletion::Instance().Remaining() == 0)
		{
			LOGERROR(g_Log, "can't create online sessions as there is no available deletion slot within C_SessionDeletion.");
			return false;
		}

		if (eStateNotActive != m_SessionState)
			return false; // can't create new session until previous one is deleted.

		assert(NULL == m_pDemonwareSession);
		if (NULL != m_pDemonwareSession)
			return false; // this object is created only when a session is active, it will be releases when the session is deleted.

		if (!slotConfig.Validate(m_MaxPlayers))
		{
			LOGWARN(g_Log, "C_Session::Create() - Unable to create session as slot configuration is not valid.");
			return false;
		}

		// are we connected to demonware? The game code should check this and connect prior to creating the session.
		bdNetImpl *const net = bdNet::getInstance();
		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (net->getStatus() != bdNetImpl::BD_NET_DONE || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
		{
			LOGWARN(g_Log, "C_Session::Create() - unable to create online sesion as we are not connected to demonware yet.");
			return false;
		}

		// allocate a command object and setup for creating session.
		C_SessionCommandList* pList = reinterpret_cast<C_SessionCommandList*> (m_pCommandList);
		C_SessionCommandList* pPool = reinterpret_cast<C_SessionCommandList*> (m_pCommandPool);
		if (pPool->Empty())
			return false;

		// validate that we have atleast 1 property, the first property will be used as the game_type within bdmatchmaking info
		// it's a system field
		if (NULL == pSessionParameters || numSessionParameters < 1 || pSessionParameters[0].m_ParameterType != SessionParameter::eTypeContext)
		{
			LOGWARN(g_Log, "C_Session::Create() - Unable to create session as first supplied parameter is incorrectly configured to be the system game_type");
			return false;
		}
		
		U32 gameType = pSessionParameters[0].m_Data.m_ContextData.m_Value;

		//allocate demonware objects
		m_pDemonwareSession = new C_DemonwareSession;
		if (NULL == m_pDemonwareSession)
			return false;

		// setup the demonware session manager with the parameters and host info.
		// when the session is actually created on the server then m_SessionCreated will go to true.
		m_pDemonwareSession->Flush();

		if (SESSION_CREATE_USES_MATCHMAKING & sessionFlags)
		{
			// system defined parameters
			bdCommonAddrRef myAddress = net->getLocalCommonAddr();
			m_pDemonwareSession->setHostAddr(myAddress);
			m_pDemonwareSession->m_maxPlayers = slotConfig.m_MaxPublic + slotConfig.m_MaxPrivate;
			m_pDemonwareSession->m_gameType = static_cast<U32> (gameType);

			// game defined properties.
			m_pDemonwareSession->m_NumAttributesInUse = 0;
			for (U32 i=0; i< numSessionParameters; i++)
			{
				if (pSessionParameters[i].m_ParameterType == SessionParameter::eTypeProperty)
				{
					m_pDemonwareSession->m_NumAttributesInUse += 1;
					SetParameter(userIndex, pSessionParameters[i]);
				}
			}

			m_UserIndex			=	userIndex;
			m_PublicSlots		=	slotConfig.m_MaxPublic;
			m_PrivateSlots		=	slotConfig.m_MaxPrivate;
			m_SessionParameters	=	sessionFlags;
			m_SessionParameters |= SESSION_CREATE_HOST; // always assume host abilities when creating a new session.

			C_SessionCommandList::Iterator it = pPool->Begin();
			pPool->Erase(pPool->Begin());
			it->m_Command		=	C_SessionCommand::eCommandCreateSession;
			it->m_Active		=	false;
			pList->PushBack(*it);

			// flag session state as now creating;
			m_SessionState =	eStateCreating;
		}
		else
		{
			// non matchmaking sessions just run without server support. no key handling is supported in this mode.
			m_SessionState =	eStateRunning;
		}

		return true;
	}

	bool C_Session::CreatePeer(const U8 userIndex, const U32 sessionFlags, const SessionParameter* pSessionParameters, const U32 numSessionParameters, const SessionSecurity& securityData, const SlotConfig& slotConfig)
	{
		SysProfile::GameInvite inv;
		memset(&inv, 0, sizeof(inv));

		// blag a gamem invite and use that version of create peer
		inv.m_Active	= true;
		inv.m_Id		= securityData.m_Id;
		inv.m_Key		= securityData.m_Key;

		return CreatePeer(userIndex, sessionFlags,pSessionParameters, numSessionParameters, inv, slotConfig);
	}

	bool C_Session::CreatePeer(const U8 userIndex, const U32 sessionFlags, const SessionParameter* pSessionParameters, const U32 numSessionParameters, const C_SessionEnumerator& availableSessions, const U32 sessionIndexToJoin, const SlotConfig& slotConfig)
	{
		if (!m_Initialised)
			return false;

		if (!C_SessionDeletion::Instance().IsActive())
		{
			LOGFATAL(g_Log, "can't create online sessions until the C_SessionDeletion class has been configured.");
			return false;
		}

		if (C_SessionDeletion::Instance().Remaining() == 0)
		{
			LOGERROR(g_Log, "can't create online sessions as there is no available deletion slot within C_SessionDeletion.");
			return false;
		}

		if (eStateNotActive != m_SessionState)
			return false; // can't create new session until previous one is deleted.

		assert(NULL == m_pDemonwareSession);
		if (NULL != m_pDemonwareSession)
			return false; // this object is created only when a session is active, it will be releases when the session is deleted.

		if (sessionFlags&SESSION_CREATE_HOST)
		{
			LOGWARN(g_Log, "C_Session::CreatePeer() - can't specify SESSION_CREATE_HOST as a flag when creating a peer session.");
			return false;
		}

		if (!slotConfig.Validate(m_MaxPlayers))
		{
			LOGWARN(g_Log, "C_Session::Create() - Unable to create session as slot configuration is not valid.");
			return false;
		}

		// is session data still valid
		if (availableSessions.m_SearchState != C_SessionEnumerator::eSearchNotActive || 0 == availableSessions.NumSessionsEnumerated())
			return false;

		// have we actually enumerated sessions from server ?
		if (sessionIndexToJoin >= availableSessions.m_NumSessionEnumerated)
			return false;

		SessionSecurity sec;
		bdCommonAddrRef hostAddr;
		if (!availableSessions.GetHostInfoFromEnumerated(sessionIndexToJoin, sec, hostAddr))
			return false;

		// is player online
		SysProfile::FullProfile fp;
		SysProfile::C_Manager::Instance().GetProfile(userIndex, fp);
		if (SysProfile::eProfileStateOnline != fp.m_State)
		{
			LOGWARN(g_Log, "C_Session::Create() - unable to create online sesion as player is not online.");
			return false;
		}

		// are we connected to demonware? The game code should check this and connect prior to creating the session.
		bdNetImpl *const net = bdNet::getInstance();
		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (net->getStatus() != bdNetImpl::BD_NET_DONE || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
		{
			LOGWARN(g_Log, "C_Session::CreatePeer() - unable to create online sesion as we are not connected to demonware yet.");
			return false;
		}

		if (NULL == pSessionParameters || numSessionParameters < 1 || pSessionParameters[0].m_ParameterType != SessionParameter::eTypeContext)
		{
			LOGWARN(g_Log, "C_Session::CreatePeer() - Unable to create session as first supplied parameter is incorrectly configured to be the system game_type");
			return false;
		}

		U32 gameType = pSessionParameters[0].m_Data.m_ContextData.m_Value;

		//allocate demonware objects
		m_pDemonwareSession = new C_DemonwareSession;
		if (NULL == m_pDemonwareSession)
			return false;

		// setup the demonware session manager with the parameters and host info.
		// this session won't actually be running on the server so there is no need to delete it later.
		m_pDemonwareSession->Flush();

		// system defined parameters
		bdCommonAddrRef myAddress = net->getLocalCommonAddr();
		m_pDemonwareSession->setHostAddr(myAddress);
		m_pDemonwareSession->m_maxPlayers = slotConfig.m_MaxPublic + slotConfig.m_MaxPrivate;
		m_pDemonwareSession->m_gameType = static_cast<U32> (gameType);

		for (U32 z=0; z<BD_SECURITY_KEY_LENGTH; z++)
			m_pDemonwareSession->m_SecurityKey[z] = sec.m_Key.ab[z];

		for (U32 z=0; z<BD_SECURITY_ID_LENGTH; z++)
			m_pDemonwareSession->m_sessionID.ab[z] = sec.m_Id.ab[z];

		// register key to allow secure packet transfer.
		bdSecurityKeyMap* const keymap = net->getSocketRouter()->getKeyMap();
		assert(keymap);
		bool reg = keymap->registerKey(sec.m_Id, sec.m_Key);
		assert(reg);

		m_UserIndex			=	userIndex;
		m_PublicSlots		=	slotConfig.m_MaxPublic;
		m_PrivateSlots		=	slotConfig.m_MaxPrivate;
		m_SessionParameters	=	sessionFlags;

		// Peer sessions on PC don't really exist so just flag this as session as being active.
		// no serer calls will be made for peer sessions.
		m_SessionState		=	eStateRunning;

		return true;
	}

	bool C_Session::CreatePeer(const U8 userIndex, const U32 sessionFlags, const SessionParameter* pSessionParameters, const U32 numSessionParameters, SysProfile::GameInvite& inviteData, const SlotConfig& slotConfig)
	{
		if (!m_Initialised)
			return false;

		if (!C_SessionDeletion::Instance().IsActive())
		{
			LOGFATAL(g_Log, "can't create online sessions until the C_SessionDeletion class has been configured.");
			return false;
		}

		if (C_SessionDeletion::Instance().Remaining() == 0)
		{
			LOGERROR(g_Log, "can't create online sessions as there is no available deletion slot within C_SessionDeletion.");
			return false;
		}

		if (eStateNotActive != m_SessionState)
			return false; // can't create new session until previous one is deleted.

		assert(NULL == m_pDemonwareSession);
		if (NULL != m_pDemonwareSession)
			return false; // this object is created only when a session is active, it will be releases when the session is deleted.

		if (sessionFlags&SESSION_CREATE_HOST)
		{
			LOGWARN(g_Log, "C_Session::CreatePeer() - can't specify SESSION_CREATE_HOST as a flag when creating a peer session.");
			return false;
		}

		if (!slotConfig.Validate(m_MaxPlayers))
		{
			LOGWARN(g_Log, "C_Session::Create() - Unable to create session as slot configuration is not valid.");
			return false;
		}

		if (!inviteData.m_Active)
			return false; // game invite isn't valid..

		// is player online
		SysProfile::FullProfile fp;
		SysProfile::C_Manager::Instance().GetProfile(userIndex, fp);
		if (SysProfile::eProfileStateOnline != fp.m_State)
		{
			LOGWARN(g_Log, "C_Session::Create() - unable to create online sesion as player is not online.");
			return false;
		}

		// are we connected to demonware? The game code should check this and connect prior to creating the session.
		bdNetImpl *const net = bdNet::getInstance();
		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (net->getStatus() != bdNetImpl::BD_NET_DONE || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
		{
			LOGWARN(g_Log, "C_Session::CreatePeer() - unable to create online sesion as we are not connected to demonware yet.");
			return false;
		}

		if (NULL == pSessionParameters || numSessionParameters < 1 || pSessionParameters[0].m_ParameterType != SessionParameter::eTypeContext)
		{
			LOGWARN(g_Log, "C_Session::CreatePeer() - Unable to create session as first supplied parameter is incorrectly configured to be the system game_type");
			return false;
		}

		U32 gameType = pSessionParameters[0].m_Data.m_ContextData.m_Value;

		//allocate demonware objects
		m_pDemonwareSession = new C_DemonwareSession;
		if (NULL == m_pDemonwareSession)
			return false;

		// setup the demonware session manager with the parameters and host info.
		// this session won't actually be running on the server so there is no need to delete it later.
		m_pDemonwareSession->Flush();

		// system defined parameters
		bdCommonAddrRef myAddress = net->getLocalCommonAddr();
		m_pDemonwareSession->setHostAddr(myAddress);
		m_pDemonwareSession->m_maxPlayers = slotConfig.m_MaxPublic + slotConfig.m_MaxPrivate;
		m_pDemonwareSession->m_gameType = static_cast<U32> (gameType);

		for (U32 z=0; z<BD_SECURITY_KEY_LENGTH; z++)
			m_pDemonwareSession->m_SecurityKey[z] = inviteData.m_Key.ab[z];

		for (U32 z=0; z<BD_SECURITY_ID_LENGTH; z++)
			m_pDemonwareSession->m_sessionID.ab[z] = inviteData.m_Id.ab[z];

		// register key to allow secure packet transfer.
		bdSecurityKeyMap* const keymap = net->getSocketRouter()->getKeyMap();
		assert(keymap);
		bool reg = keymap->registerKey(inviteData.m_Id, inviteData.m_Key);
		assert(reg);

		m_UserIndex			=	userIndex;
		m_PublicSlots		=	slotConfig.m_MaxPublic;
		m_PrivateSlots		=	slotConfig.m_MaxPrivate;
		m_SessionParameters	=	sessionFlags;

		// Peer sessions on PC don't really exist so just flag this as session as being active.
		// no serer calls will be made for peer sessions.
		m_SessionState		=	eStateRunning;

		return true;
	}

	bool C_Session::EnableQos(const U32 qosBandwidth, U8* pData, const U32 dataSize)
	{
		if (!m_Initialised)
			return false;

		if (!m_SessionCreated)
			return false; 

		if (m_Migrating)
			return false;   // cant enable QoS while we are migrating session.

		if (dataSize > BIZ_MAX_QOS_DATA_SIZE)
		{
			LOGFATAL(g_Log, "trying to send too much data in the QoS, max is %d bytes.", BIZ_MAX_QOS_DATA_SIZE);
			return false;
		}

		bdNetImpl *const net = bdNet::getInstance();
		if (net->getStatus() == bdNetImpl::BD_NET_DONE)
		{
			bdQoSProbe* m_pQosProber = net->getSocketRouter()->getQoSProber();
			if (NULL == m_pQosProber)
				return false;

			if (!m_QoSEnabled)
				m_QoSEnabled =  m_pQosProber->listen(m_pDemonwareSession->m_sessionID, pData, dataSize); // listener is not active so create one.
			else
				m_pQosProber->setData(pData, dataSize); // listener is already running so just modify data
			return true;
		}

		return false;
	}

	bool C_Session::DisableQos(void)
	{
		if (!m_Initialised)
			return false;

		if (!m_QoSEnabled)
			return false;

		if (!m_SessionCreated)
			return false;

		bdNetImpl *const net = bdNet::getInstance();
		if (net->getStatus() == bdNetImpl::BD_NET_DONE)
		{
			bdQoSProbe* m_pQosProber = net->getSocketRouter()->getQoSProber();
			if (NULL == m_pQosProber)
				return false;
			m_pQosProber->shutdownListener();
			m_QoSEnabled = false;
			return true;
		}
		return false;
	}

	bool C_Session::Modify(const U32 modifiedSessionFlags, const SlotConfig& slotConfig, const SessionParameter* pSessionParameters, const U32 numSessionParameters, SessionCommandNotification* pCompleteNotification)
	{
		if (!m_Initialised)
			return false;

		if (eStateNotActive == m_SessionState || eStateError == m_SessionState || eStateCreating == m_SessionState || NULL == m_pDemonwareSession)
			return false;  // session not in a  state where we can modify

		if (!slotConfig.Validate(m_MaxPlayers))
		{
			LOGWARN(g_Log, "C_Session::Modify() - Unable to modify session as slot configuation is invalid.");
			return false;
		}

		if (NULL == pSessionParameters || numSessionParameters < 1 || pSessionParameters[0].m_ParameterType != SessionParameter::eTypeContext)
		{
			LOGWARN(g_Log, "C_Session::Modify() - Unable to modify session session as first supplied parameter is incorrectly configured to be the system game_type");
			return false;
		}

		U32 gameType = pSessionParameters[0].m_Data.m_ContextData.m_Value;

		// update demonware server data
		m_pDemonwareSession->m_maxPlayers = slotConfig.m_MaxPublic + slotConfig.m_MaxPrivate;
		m_pDemonwareSession->m_gameType = static_cast<U32> (gameType);

		m_PublicSlots	=	slotConfig.m_MaxPublic;
		m_PrivateSlots	=	slotConfig.m_MaxPrivate;

		if (0 != modifiedSessionFlags)
			m_SessionParameters	= modifiedSessionFlags;

		if (!m_SessionCreated)
		{
			// notify
			if (pCompleteNotification != NULL)
				pCompleteNotification->m_Finished = true;

			// this is curretnly a peer session that has no server presence.
			m_SessionParameters &= ~SESSION_CREATE_HOST; 
			return true; 
		}

		// update my local address in the demonware data
		bdNetImpl *const net = bdNet::getInstance();
		bdCommonAddrRef myAddress = net->getLocalCommonAddr();
		m_pDemonwareSession->setHostAddr(myAddress);

		// update session parameters
		m_pDemonwareSession->m_NumAttributesInUse = 0;
		for (U32 i=0; i< numSessionParameters; i++)
		{
			if (pSessionParameters[i].m_ParameterType == SessionParameter::eTypeProperty)
			{
				m_pDemonwareSession->m_NumAttributesInUse += 1;
				SetParameter(m_UserIndex, pSessionParameters[i]);
			}
		}

		m_SessionParameters |= SESSION_CREATE_HOST;

		C_SessionCommandList* pList = reinterpret_cast<C_SessionCommandList*> (m_pCommandList);
		C_SessionCommandList* pPool = reinterpret_cast<C_SessionCommandList*> (m_pCommandPool);

		if (pPool->Empty())
			return false;

		// allocate object and configure
		C_SessionCommandList::Iterator i = pPool->Begin();
		pPool->Erase(pPool->Begin());
		i->m_Command								=	C_SessionCommand::eCommandModifySession;
		i->m_CommandData.m_Modify.m_PublicSlotCount	=	slotConfig.m_MaxPublic;
		i->m_CommandData.m_Modify.m_PrivateSlotCount=	slotConfig.m_MaxPrivate;
		i->m_CommandData.m_Modify.m_SessionFlags	=	modifiedSessionFlags;
		if (NULL == pCompleteNotification)
		{
			i->m_CommandData.m_Modify.m_pNotification	=	NULL;
			i->m_CommandData.m_Modify.m_NotificationId	=	0;
		}
		else
		{
			i->m_CommandData.m_Modify.m_pNotification	=	pCompleteNotification;
			i->m_CommandData.m_Modify.m_NotificationId	=	pCompleteNotification->m_Id;
		}

		// clear the passed in tofifiction structure.
		if (NULL != pCompleteNotification)
			pCompleteNotification->Reset();

		// add to list for asynchronous processing.
		pList->PushBack(*i);

		return true;
	}

	void C_Session::Delete(void)
	{
		if (!m_Initialised)
			return;

		if (eStateNotActive == m_SessionState)
			return; // no session currently running.

		// stiop Qos Listener if its active.
		DisableQos();

		if (!m_SessionCreated)
		{
			// stop all active tasks and close session handle, since session was not successfully created.
			C_SessionCommandList* pList = reinterpret_cast<C_SessionCommandList*> (m_pCommandList);
			C_SessionCommandList* pPool = reinterpret_cast<C_SessionCommandList*> (m_pCommandPool);
			while (!pList->Empty())
			{
				C_SessionCommandList::Iterator i = pList->Begin();
				if (i->m_Active)
					NetPlay::DemonwareRemoteTask::Close(i->m_AsyncTask);

				// give back to pool;
				C_SessionCommandList::Iterator c = i;
				i = pList->Erase(c);
				pPool->PushBack(*c);
			}
		}
		else
		{
			// unregister current session key, game will have to unregister any keys that are still that are not current. ie session was migrated.
			// ideally game should unregister all keys that the players are conencted via when using demonware, when the sesion is closed.
			if (m_pDemonwareSession)
			{
				bdSecurityKeyMap *const keyMap = bdNet::getInstance()->getSocketRouter()->getKeyMap();
				bool unreg = keyMap->unregisterKey(m_pDemonwareSession->m_sessionID); 
			}

			// stop all active tasks and start deletion process..
			C_SessionCommandList* pList = reinterpret_cast<C_SessionCommandList*> (m_pCommandList);
			C_SessionCommandList* pPool = reinterpret_cast<C_SessionCommandList*> (m_pCommandPool);
			while (!pList->Empty())
			{
				C_SessionCommandList::Iterator i = pList->Begin();
				if (i->m_Active)
					NetPlay::DemonwareRemoteTask::Close(i->m_AsyncTask);

				// give back to pool;
				C_SessionCommandList::Iterator c = i;
				i = pList->Erase(c);
				pPool->PushBack(*c);
			}

			// queue for deletion.
			C_SessionDeletion::Instance().Add(*this);
		}

		// reset data to make it possible to create another session, even if the current one is deleting.
		if (m_pDemonwareSession)
			delete m_pDemonwareSession;
		m_pDemonwareSession = NULL;

		m_SessionCreated			=	false;
		m_SessionState				=	eStateNotActive;
		m_SessionParameters			=	0;
		m_PublicSlots				=	0;
		m_PrivateSlots				=	0;
		m_UserIndex					=	0;
		m_Locked					=	false;
		m_QoSEnabled				=	false;
		m_Migrating					=	false;
	}

	bool C_Session::AddPlayer(const SysProfile::UserIdentifier& player, const bool usePublicSlot, SessionCommandNotification* pCompleteNotification)
	{
		if (!m_Initialised)
			return false;

		SessionUserList list;
		SessionUser s;
		s.m_PublicSlot = usePublicSlot;
		s.m_User = player;

		list.PushBack(s);

		return AddPlayers(list, pCompleteNotification);
	}

	bool C_Session::AddPlayers(const C_Session::SessionUserList& playerList, SessionCommandNotification* pCompleteNotification)
	{
		if (!m_Initialised)
			return false;

		if (eStateNotActive == m_SessionState || eStateError == m_SessionState || eStateCreating == m_SessionState || NULL == m_pDemonwareSession)
			return false;  // session not in a  state where we can modify
		
		if (!m_SessionCreated)
		{
			// notify
			if (pCompleteNotification != NULL)
				pCompleteNotification->m_Finished = true;

			return true;  // peer sessions don't need to add the players (they will need to once migrated though.
		}

		// validate that we have at least 1 full player to inform the server about.
		// guest players are not sent to the server.
		U8 valid = 0;
		for (S32 q=0; q<playerList.Size(); q++)
		{	
			const SessionUser& su = playerList[q];
			if (!su.m_User.m_IsGuest)
				valid += 1;
		}

		if (0 == valid)
			return false;

		C_SessionCommandList* pList = reinterpret_cast<C_SessionCommandList*> (m_pCommandList);
		C_SessionCommandList* pPool = reinterpret_cast<C_SessionCommandList*> (m_pCommandPool);
		if (pPool->Empty())
		{
			m_SessionState = eStateError;
			return false; // no commands left to allocate.
		}

		// allocate object and configure
		C_SessionCommandList::Iterator i = pPool->Begin();
		pPool->Erase(pPool->Begin());

		i->m_Command = C_SessionCommand::eCommandAddPlayer;
		
		U8 numAdded = 0;
		for (S32 q=0; q<playerList.Size(); q++)
		{	
			const SessionUser& su = playerList[q];

			if (!su.m_User.m_IsGuest)
				i->m_CommandData.m_Add.m_Users[numAdded++] = su.m_User.m_Uid;
		}
		i->m_CommandData.m_Add.m_NumToAdd =	numAdded;

		if (NULL == pCompleteNotification)
		{
			i->m_CommandData.m_Add.m_pNotification	=	NULL;
			i->m_CommandData.m_Add.m_NotificationId	=	0;
		}
		else
		{
			i->m_CommandData.m_Add.m_pNotification	=	pCompleteNotification;
			i->m_CommandData.m_Add.m_NotificationId	=	pCompleteNotification->m_Id;

			// reset notification now as we have copied requested data for processing later.
			pCompleteNotification->Reset();
		}

		// add to list for asynchronous processing.
		pList->PushBack(*i);
		return true;
	}

	bool C_Session::RemovePlayer(const SysProfile::UserIdentifier& player, SessionCommandNotification* pCompleteNotification)
	{
		if (!m_Initialised)
			return false;

		SessionUserList list;
		SessionUser s;
		s.m_PublicSlot = true;
		s.m_User = player;

		list.PushBack(s);

		return RemovePlayers(list, pCompleteNotification);
	}

	bool C_Session::RemovePlayers(const C_Session::SessionUserList& playerList, SessionCommandNotification* pCompleteNotification)
	{
		if (!m_Initialised)
			return false;

		if (eStateNotActive == m_SessionState || eStateError == m_SessionState || eStateCreating == m_SessionState || NULL == m_pDemonwareSession)
			return false;  // session not in a  state where we can modify

		if (!m_SessionCreated)
		{
			// notify
			if (pCompleteNotification != NULL)
				pCompleteNotification->m_Finished = true;

			return true;  // peer sessions don't need to add the players (they will need to once migrated though.
		}

		// validate that we have at least 1 full player to inform the server about.
		// guest players are not sent to the server.
		U8 valid = 0;
		for (S32 q=0; q<playerList.Size(); q++)
		{	
			const SessionUser& su = playerList[q];
			if (!su.m_User.m_IsGuest)
				valid += 1;
		}

		if (0 == valid)
			return false;

		C_SessionCommandList* pList = reinterpret_cast<C_SessionCommandList*> (m_pCommandList);
		C_SessionCommandList* pPool = reinterpret_cast<C_SessionCommandList*> (m_pCommandPool);
		if (pPool->Empty())
		{
			m_SessionState = eStateError;
			return false; // no commands left to allocate.
		}

		// allocate object and configure
		C_SessionCommandList::Iterator i = pPool->Begin();
		pPool->Erase(pPool->Begin());

		i->m_Command = C_SessionCommand::eCommandRemovePlayer;
	
		U8 numAdded = 0;
		for (S32 q=0; q<playerList.Size(); q++)
		{	
			const SessionUser& su = playerList[q];
			if (!su.m_User.m_IsGuest)
				i->m_CommandData.m_Remove.m_Users[numAdded++] =	su.m_User.m_Uid;
		}
		i->m_CommandData.m_Remove.m_NumToRemove	= numAdded;

		if (NULL == pCompleteNotification)
		{
			i->m_CommandData.m_Remove.m_pNotification	=	NULL;
			i->m_CommandData.m_Remove.m_NotificationId	=	0;
		}
		else
		{
			i->m_CommandData.m_Remove.m_pNotification	=	pCompleteNotification;
			i->m_CommandData.m_Remove.m_NotificationId	=	pCompleteNotification->m_Id;

			// reset notification now as we have copied requested data for processing later.
			pCompleteNotification->Reset();
		}

		// add to list for asynchronous processing.
		pList->PushBack(*i);
		return true;
	}

	bool C_Session::Migrate(SessionCommandNotification* pCompleteNotification, const SessionParameter* pSessionParameters, const U32 numSessionParameters, const SlotConfig& slotConfig)
	{
		if (!m_Initialised)
			return false;

		if (eStateRunning != m_SessionState && eStatePlaying != m_SessionState)
		{
			LOGWARN(g_Log, "C_Session::Migrate() - Can't migrate when session is in an error state(created=%d, state=%d).", m_SessionCreated, m_SessionState);
			return false;  // session not in a good state
		}

		if (m_SessionCreated)
		{
			LOGWARN(g_Log, "C_Session::Migrate() - Can't migrate when we are already the host of the session !");
			return false;  
		}

		if (m_Migrating)
		{
			LOGWARN(g_Log, "C_Session::Migrate() - Unable to migrate when currently migrating!.");
			return false;
		}

		if (!slotConfig.Validate(m_MaxPlayers))
		{
			LOGWARN(g_Log, "C_Session::Migrate() - Unable to migrate session as slot configuration is not valid.");
			return false;
		}

		if (C_SessionDeletion::Instance().Remaining() == 0)
		{
			LOGERROR(g_Log, "can't migrate online session as there is no available deletion slot within C_SessionDeletion.");
			return false;
		}

		C_SessionCommandList* pList = reinterpret_cast<C_SessionCommandList*> (m_pCommandList);
		C_SessionCommandList* pPool = reinterpret_cast<C_SessionCommandList*> (m_pCommandPool);
		if (pPool->Empty())
			return false; // no commands left to allocate.

		if (NULL == pSessionParameters || numSessionParameters < 1 || pSessionParameters[0].m_ParameterType != SessionParameter::eTypeContext)
		{
			LOGWARN(g_Log, "C_Session::Migrate() - Unable to migrate session as first supplied parameter is incorrectly configured to be the system game_type");
			return false;
		}

		U32 gameType = pSessionParameters[0].m_Data.m_ContextData.m_Value;

		// update demonware system properties
		// update my local address in the demonware data
		bdNetImpl *const net = bdNet::getInstance();
		bdCommonAddrRef myAddress = net->getLocalCommonAddr();
		m_pDemonwareSession->setHostAddr(myAddress);
		m_pDemonwareSession->m_maxPlayers = slotConfig.m_MaxPublic + slotConfig.m_MaxPrivate;
		m_pDemonwareSession->m_gameType = static_cast<U32> (gameType);

		// game defined session parameters
		m_pDemonwareSession->m_NumAttributesInUse = 0;
		for (U32 i=0; i< numSessionParameters; i++)
		{
			if (pSessionParameters[i].m_ParameterType == SessionParameter::eTypeProperty)
			{
				m_pDemonwareSession->m_NumAttributesInUse += 1;
				SetParameter(m_UserIndex, pSessionParameters[i]);
			}
		}

		m_PublicSlots		=	slotConfig.m_MaxPublic;
		m_PrivateSlots		=	slotConfig.m_MaxPrivate;
		m_SessionParameters |= SESSION_CREATE_HOST; // always assume host abilities when migrating

		C_SessionCommandList::Iterator it = pPool->Begin();
		pPool->Erase(pPool->Begin());
		it->m_Command		=	C_SessionCommand::eCommandMigrateSession;
		it->m_Active		=	false;
		it->m_CommandData.m_Migrate.m_NewHost =	true;
		if (NULL != pCompleteNotification)
		{
			it->m_CommandData.m_Migrate.m_pNotification	= pCompleteNotification;
			it->m_CommandData.m_Migrate.m_NotificationId = pCompleteNotification->m_Id;
		}
		else
		{
			it->m_CommandData.m_Migrate.m_pNotification	= NULL;
			it->m_CommandData.m_Migrate.m_NotificationId = 0;
		}

		pList->PushBack(*it);

		DisableQos();

		if (NULL != pCompleteNotification)
			pCompleteNotification->Reset();

		m_Migrating = true;

		return true;
	}

	bool C_Session::GetMigrationInfo(MigrationInfo& outInfo)
	{
		if (!m_Initialised)
			return false;

		if (eStateNotActive == m_SessionState || !m_SessionCreated || NULL == m_pDemonwareSession)
			return false;  // session not in a good state

		for (U32 z=0; z<BD_SECURITY_KEY_LENGTH; z++)
			outInfo.m_Key.ab[z] = m_pDemonwareSession->m_SecurityKey[z];

		for (U32 z=0; z<BD_SECURITY_ID_LENGTH; z++)
			outInfo.m_Id.ab[z] = m_pDemonwareSession->m_sessionID.ab[z];

		return true;
	}

	bool C_Session::MigratePeer(const MigrationInfo& migrationInfo, SessionCommandNotification* pCompleteNotification, const SlotConfig& slotConfig)
	{
		if (!m_Initialised)
			return false;

		if  (eStateRunning != m_SessionState && eStatePlaying != m_SessionState)
			return false; // not in a good state to migrate.

		if (m_SessionCreated || NULL == m_pDemonwareSession)
			return false;  // session not in a good state, hosts should nto be calling this function..

		if (!slotConfig.Validate(m_MaxPlayers))
			return false;

		m_pDemonwareSession->m_maxPlayers = slotConfig.m_MaxPublic + slotConfig.m_MaxPrivate;

		for (U32 z=0; z<BD_SECURITY_KEY_LENGTH; z++)
			m_pDemonwareSession->m_SecurityKey[z] = migrationInfo.m_Key.ab[z];

		for (U32 z=0; z<BD_SECURITY_ID_LENGTH; z++)
			m_pDemonwareSession->m_sessionID.ab[z] = migrationInfo.m_Id.ab[z];

		// register key to allow secure packet transfer.
		bdNetImpl *const net = bdNet::getInstance();
		bdSecurityKeyMap* const keymap = net->getSocketRouter()->getKeyMap();
		assert(keymap);
		bool reg = keymap->registerKey(migrationInfo.m_Id, migrationInfo.m_Key);
		assert(reg);

		// peer migration info doesn't have to talk to the server so if a completion notification is supplied
		// then set it to commplete.
		if (pCompleteNotification)
			pCompleteNotification->m_Finished = true;

		return true;
	}

	/// NOT used on PS3 since no trueskill exists
	bool C_Session::ModifySkill(SessionCommandNotification* pCompleteNotification, const U8 numPlayers, const SysProfile::UserIdentifier* pPlayers)
	{
		if (!m_Initialised)
			return false;

		// not needed on ps3.
		if (pCompleteNotification)
			pCompleteNotification->m_Finished = true;

		return false;
	}

	bool C_Session::RegisterForArbitration(const ArbitrationNonce& arbitrationNonce, SessionCommandNotification* pCompleteNotification)
	{
		if (!m_Initialised)
			return false;

		// not used on ps3 so just flag notification as complete.
		if (pCompleteNotification)
			pCompleteNotification->m_Finished = true;

		return true;
	}

	bool C_Session::CheckPlayerIsInArbitrationList(const SysProfile::UserIdentifier& playeridentifier)
	{
		if (!m_Initialised)
			return false;

		// not used on ps3 so player is always deemed to be in arbitrated session.
		return true;
	}

	bool C_Session::ReleaseArbitrationRegistrationBuffer(void)
	{
		if (!m_Initialised)
			return false;

		return true;
	}

	bool C_Session::LockCurrentPlayers(const bool lock)
	{
		if (!m_Initialised)
			return false;

		if (eStateNotActive == m_SessionState || eStateError == m_SessionState || !m_SessionCreated)
			return false;

		if (!(m_SessionParameters&SESSION_CREATE_USES_ARBITRATION))
		{
			// the session we created was not setup for arbitration... so no need to lock unlock player
			return false;	
		}

		m_Locked = lock;

		return true;
	}

	bool C_Session::StartPlay(void)
	{
		if (!m_Initialised)
			return false;

		switch (m_SessionState)
		{
		case eStateRunning:
			m_SessionState = eStatePlaying;
			break;

		default:
			return false;
			break;
		}
		return true;
	}

	bool C_Session::EndPlay(void)
	{
		if (!m_Initialised)
			return false;

		switch (m_SessionState)
		{
		case eStatePlaying:
			m_SessionState = eStateRunning;
			break;

		default:
			return false;
			break;
		}
		return true;
	}

	bool C_Session::GetSecurityInfo(SessionSecurity& outKey)
	{
		if (eStateNotActive == m_SessionState || eStateCreating == m_SessionState || eStateError == m_SessionState || NULL == m_pDemonwareSession)
			return false;

		outKey.m_ArbitrationNonce	=	0;

		for (U32 z=0; z<BD_SECURITY_KEY_LENGTH; z++)
			outKey.m_Key.ab[z] = m_pDemonwareSession->m_SecurityKey[z];

		for (U32 z=0; z<BD_SECURITY_ID_LENGTH; z++)
			outKey.m_Id.ab[z] = m_pDemonwareSession->m_sessionID.ab[z];

		return true;
	}

	bool C_Session::IsIdle(void)
	{
		if (!m_Initialised)
			return true;

		if (eStateNotActive == m_SessionState || eStateError == m_SessionState)
			return true;

		C_SessionCommandList* pList = reinterpret_cast<C_SessionCommandList*> (m_pCommandList);

		if (pList->Size() == 0)
			return true;
		return false;

	}

	void C_Session::ShowPlayerReviewUI(const U8 userindex, const SysProfile::UserIdentifier& reviewTarget)
	{
		if (!m_Initialised)
			return;
	}

	void C_Session::CleanUp(void)
	{
		Delete();

		if (m_pDemonwareSession)
			delete m_pDemonwareSession;
		m_pDemonwareSession = NULL;

		// remove any items from the command list
		if (m_pCommandList)
		{
			C_SessionCommandList* p = reinterpret_cast<C_SessionCommandList*> (m_pCommandList);
			p->Clear();
			delete p;
			m_pCommandList = NULL;
		}

		// remove any items from the command pool
		if (m_pCommandPool)
		{
			C_SessionCommandList* p = reinterpret_cast<C_SessionCommandList*> (m_pCommandPool);
			p->Clear();
			delete p;
			m_pCommandPool = NULL;
		}

		// release command storage..
		if (m_pCommandStorage)
		{
			delete [] m_pCommandStorage;
			m_pCommandStorage = NULL;
		}

		m_Initialised				=	false;
		m_SessionState				=	eStateNotActive;
		m_SessionParameters			=	0;
		m_PublicSlots				=	0;
		m_PrivateSlots				=	0;
		m_CommandStorageSize		=	0;
		m_UserIndex					=	0;
		m_MaxPlayers				=	0;
	}
}
