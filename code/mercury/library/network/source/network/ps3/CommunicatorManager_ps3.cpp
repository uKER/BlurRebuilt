#include <StdAfx.h>
#include <System/Time/Time.h>

#include <sdk_version.h>
#include <sysutil/sysutil_bgmplayback.h>

#if CELL_SDK_VERSION >= 0x250001
#if 1
#define PS3_VOICE_ENABLED
#endif
#endif // CELL_SDK_VERSION

#if defined ( PS3_VOICE_ENABLED )
#include <cell/voice.h>
#include <cell/voiceTypes.h> 
#include <cell/atomic.h>
#endif

#include <Network/CommunicatorManager.h>
#include <systemprofile/SystemProfile.h>
#include <Network/CommunicatorVoiceBuffer.h>
#include "../Shared/Remotetalker.h"

namespace Communicator
{
#if defined ( PS3_VOICE_ENABLED )
	
	const CellVoiceBitRate BIT_RATE = CELLVOICE_BITRATE_7300;		// 19 byte frame size.
	//const CellVoiceBitRate BIT_RATE = CELLVOICE_BITRATE_14400;	// 27 byte frame size.
	//const CellVoiceBitRate BIT_RATE = CELLVOICE_BITRATE_16000;	// 20 byte frame size.
	//const CellVoiceBitRate BIT_RATE = CELLVOICE_BITRATE_22533;	// 43 byte frame size.

	static S32 CreateMicPort(U32 *port, const U32 playerIndex)
	{
		S32							ret;
		CellVoicePortParam			portArgs;
		portArgs.portType           = CELLVOICE_PORTTYPE_IN_MIC;
		portArgs.bMute              = false;
		portArgs.threshold          = 100;                   
		portArgs.volume             = 1.0f;
		portArgs.device.playerId	= playerIndex;

		ret = cellVoiceCreatePort(port, &portArgs);
		return ret;
	}

	static S32 CreateHeadsetPort(U32 *port, const U32 playerIndex)
	{
		S32						 ret;
		CellVoicePortParam		 portArgs;
		portArgs.portType        = CELLVOICE_PORTTYPE_OUT_SECONDARY;
		portArgs.bMute           = false;
		portArgs.threshold       = 100;                  
		portArgs.volume          = 1.0f;
		portArgs.device.playerId = playerIndex;

		ret = cellVoiceCreatePort(port, &portArgs);
		return ret;
	}

	static S32 CreateVoiceInPort(U32 *port)
	{
		S32                     ret;
		CellVoicePortParam      portArgs;
		portArgs.portType       = CELLVOICE_PORTTYPE_IN_VOICE;
		portArgs.bMute          = false;
		portArgs.threshold      = 100;
		portArgs.volume         = 1.0f;
		portArgs.voice.bitrate	= BIT_RATE;

		ret = cellVoiceCreatePort(port, &portArgs);
		return ret;
	}

	static S32 CreateVoiceOutPort(U32 *port)
	{
		S32                     ret;
		CellVoicePortParam      portArgs;
		portArgs.portType       = CELLVOICE_PORTTYPE_OUT_VOICE;
		portArgs.bMute          = false;
		portArgs.threshold      = 100;
		portArgs.volume         = 1.0f;
		portArgs.voice.bitrate  = BIT_RATE;

		ret = cellVoiceCreatePort(port, &portArgs);
		return ret;
	}

	static S32 RegisterPortForEnergyRetrieval(const U32 port)
	{
		U32 energyLevel = 1;
		return cellVoiceSetPortAttr(port, CELLVOICE_ATTR_ENERGY_LEVEL, &energyLevel);
	}

	static S32 ConnectInPortToOutPort(const U32 inputPort, const U32 outputPort)
	{
		return cellVoiceConnectIPortToOPort(inputPort, outputPort);
	}

	static S32 DisconnectInPortFromOutPort(const U32 inputPort, const U32 outputPort)
	{
		return cellVoiceDisconnectIPortFromOPort(inputPort, outputPort);
	}

	static S32 DeletePort(const U32 port)
	{
		return cellVoiceDeletePort(port);
	}
#endif 

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	// static instance..
	C_Core	C_Core::s_CommunicatorCore;

	const U64 VOICE_EVENT_ID = 0xAEF0123;

	C_VoiceThread::C_VoiceThread() : m_Quit(false), m_Active(false), m_State(0)
	{
	}

	C_VoiceThread::~C_VoiceThread()
	{
#if defined ( PS3_VOICE_ENABLED )
		Leave();
#endif
	}

	void C_VoiceThread::Enter(const sys_event_queue_t& queue)
	{
#if defined ( PS3_VOICE_ENABLED )
		if (!m_Active)
		{
			m_State = 0;
			m_Queue = queue;
			m_Quit = false;
			Start(0x2000, 0, "network voice comms");
			m_Active = true;
		}
#endif
	}

	void C_VoiceThread::Leave(void)
	{
#if defined ( PS3_VOICE_ENABLED )
		if (m_Active)
		{
			m_Quit = true;
			Join();	
			m_Quit = false;	
			m_Active = false;
			m_State = 0;
		}
#endif
	}

	void C_VoiceThread::ThreadMain()
	{
#if defined ( PS3_VOICE_ENABLED )
		while (!m_Quit)
		{
			sys_event_t	event;
			int ret = sys_event_queue_receive(m_Queue, &event, 200*1000); // 200ms default timeout
			switch (ret)
			{
			case ETIMEDOUT:
				break;

			case CELL_OK:
				if (VOICE_EVENT_ID == event.source)
				{
					switch (event.data1)
					{
					case CELLVOICE_EVENT_PORT_ATTACHED: // test for correct mic port (event.data2)
						break;
					case CELLVOICE_EVENT_PORT_DETACHED: // test for correct mic port (event.data2)
						break;

					case CELLVOICE_EVENT_SERVICE_ATTACHED:
						cellAtomicOr32(&m_State, 1<<eStatesServiceAttached);
						break;
					case CELLVOICE_EVENT_SERVICE_DETACHED:
						cellAtomicAnd32(&m_State, ~(1<<eStatesServiceAttached));						
						break;

					case CELLVOICE_EVENT_DATA_ERROR:
						break;
					}
				}
				break;

			default:
				m_Quit = true;
				break;
			}
		}
#endif
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	// constructor
	C_Core::C_Core() :	m_MaxLocalTalkers(0), m_MaxRemoteTalkers(0),
						m_LocalRegisteredIndexes(0), m_pRemoteTalkerStorage(NULL), m_pRemoteTalkerList(NULL),
						m_pRemoteTalkerPool(NULL), m_Initialised(false), m_Systems(0), m_StartAsap(false), m_Running(false)
	{
	}


	// destructor 
	C_Core::~C_Core()
	{
		Cleanup();
	}


	// call once to initialise all dynamic resources and startup voice engine.
	bool C_Core::Initialise(const U32 maxLocalPlayers, const U32 maxRemotePlayers)
	{
		if (m_Initialised)
			return true; 

		cellSysutilEnableBgmPlayback();

		if (0 == maxLocalPlayers)
			return false; // must have local players, and some storage for the voice data..

		m_MaxLocalTalkers	=	1;	// on ps3 we can only have one voice talker at the moment.
		m_MaxRemoteTalkers	=	maxRemotePlayers;

		if (m_MaxRemoteTalkers)
		{
			m_pRemoteTalkerStorage	=	new C_RemoteTalker[m_MaxRemoteTalkers];
			m_pRemoteTalkerList		=	reinterpret_cast<void*> (new C_RemoteTalkerList);
			m_pRemoteTalkerPool		=	reinterpret_cast<void*> (new C_RemoteTalkerList);

			if (NULL == m_pRemoteTalkerStorage || NULL == m_pRemoteTalkerList || NULL == m_pRemoteTalkerPool)
			{
				// swang, can't allocate memory..
				Cleanup();
				return false;
			}

			// setup pool objects
			C_RemoteTalkerList* pPool = reinterpret_cast<C_RemoteTalkerList*> (m_pRemoteTalkerPool);
			for (U32 i=0; i<m_MaxRemoteTalkers; i++)
				pPool->PushBack(m_pRemoteTalkerStorage[i]);
		}

		// go on ya dink... start up.
		if (!CreateEngine())
			Cleanup();
		else
			m_Initialised = true;

		return m_Initialised;
	}

	
	// release all resources
	void C_Core::Shutdown(void)
	{
		UnRegisterAllPlayers();
		Cleanup();
	}


	// register local player for voice communications.
	bool C_Core::RegisterLocalTalker(const SysProfile::FullProfile& localTalker, const bool enforcePrivileges)
	{
		if (!m_Initialised)
			return false;

		bool ok = false;
#if defined ( PS3_VOICE_ENABLED )
		// gamer profile must be valid for voice..
		switch (localTalker.m_State)
		{
		case SysProfile::eProfileStateOffline:
		case SysProfile::eProfileStateOnline:
		case SysProfile::eProfileStateBlagged:
			break;
		default:
			return false;
			break;
		}

		// validate userindex
		if (localTalker.m_UserIndex >= 7)
		{
			LOGFATAL( g_Log,  "Communicator::C_Core::RegisterLocalTalker() - Invalid profile user index");
			return false;
		}

		// we can only have a single mic running at the moment so check only a single player is registered.
		U8 bitsSet = 0;
		for (U8 i=0; i<7; i++)
		{
			if (i&1<<i)
				bitsSet += 1;
		}

		if (bitsSet >= 1)
		{
			LOGWARN( g_Log,  "Communicator::C_Core::RegisterLocalTalker() - Only 1 mic is supported in the current sce libs.");
			return false;
		}

		// is user already registered..
		if (m_LocalRegisteredIndexes&(1<<localTalker.m_UserIndex))
			return true;

		// make sure player has voice priviliges
		if (enforcePrivileges)
		{
			if (!localTalker.m_CommunicationsPrivilege)
			{
				if (!localTalker.m_CommunicationsFriendsOnlyPrivilege)
					return false; // local player cant use voice at all !!
			}
		}

		LocalPlayerPortData portData;

		// NOTE - it seems we since libvoice dont support multiple headsets we always need to create the ports with index 0.

		if (CreateMicPort(&portData.m_MicPort, 0/*localTalker.m_UserIndex*/) == CELL_OK)
		{
			if (CreateVoiceOutPort(&portData.m_VoiceOutPort) == CELL_OK)
			{
				if (ConnectInPortToOutPort(portData.m_MicPort, portData.m_VoiceOutPort) == CELL_OK)
				{
					if (CreateHeadsetPort(&portData.m_HeadsetPort, 0/*localTalker.m_UserIndex*/) == CELL_OK)
					{
						m_LocalRegisteredIndexes |= 1<<(localTalker.m_UserIndex);
						m_LocalPortData[localTalker.m_UserIndex] = portData;
						ok = true;

						cellSysutilDisableBgmPlayback();
						m_StartAsap = true;
					}
					else
					{
						LOGWARN(g_Log, "Communicator::C_Core::RegisterLocalTalker() - Unable to create headset port.");
						DisconnectInPortFromOutPort(portData.m_MicPort, portData.m_VoiceOutPort);
						DeletePort(portData.m_VoiceOutPort);
						DeletePort(portData.m_MicPort);
					}
				}
				else
				{
					LOGWARN(g_Log, "Communicator::C_Core::RegisterLocalTalker() - Unable to link mic port to voice out port");
					DeletePort(portData.m_VoiceOutPort);
					DeletePort(portData.m_MicPort);
				}
			}
			else
			{
				LOGWARN(g_Log, "Communicator::C_Core::RegisterLocalTalker() - Unable to create voice out port.");
				DeletePort(portData.m_MicPort);
			}
		}
		else
		{
			LOGWARN(g_Log, "Communicator::C_Core::RegisterLocalTalker() - Unable to create mic port.");
		}

#endif

		return ok;
	}


	// change mode for players communicator
	bool C_Core::SetLocalMode(const U8 userIndex, const eCommunicatorMode newMode)
	{
		if (!m_Initialised)
			return false;

#if defined ( PS3_VOICE_ENABLED )
#endif

		return true;
	}


	// try and unregister a local talker from the voice manager.
	bool C_Core::UnRegisterLocalTalker(const U8 userIndex)
	{
		if (!m_Initialised)
			return false;

#if defined ( PS3_VOICE_ENABLED )
		if (userIndex < 7 && m_LocalRegisteredIndexes&(1<<userIndex))
		{
			C_RemoteTalkerList* pList = reinterpret_cast<C_RemoteTalkerList*> (m_pRemoteTalkerList);
			if (pList->Size())
			{
				LOGFATAL(g_Log, "Communicator::C_Core::UnRegisterLocalTalker() - there is still active remote talkers, these must be unregistered before the local players.");
			}

			LocalPlayerPortData blankPortData;
			LocalPlayerPortData& portData = m_LocalPortData[userIndex];

			const S32 headsetRes	= DeletePort(portData.m_HeadsetPort);
			const S32 disconRes		= DisconnectInPortFromOutPort(portData.m_MicPort, portData.m_VoiceOutPort);
			const S32 outRes		= DeletePort(portData.m_VoiceOutPort);
			const S32 micRes		= DeletePort(portData.m_MicPort);

			if (CELL_OK != headsetRes)
			{
				LOGWARN(g_Log, "Communicator::C_Core::UnRegisterLocalTalker() - Unable to delete headset port for player %d.", userIndex);	
			}
			if (CELL_OK != disconRes)
			{
				LOGWARN(g_Log, "Communicator::C_Core::UnRegisterLocalTalker() - Unable to disconnect output port for player %d.", userIndex);	
			}
			if (CELL_OK != outRes)
			{
				LOGWARN(g_Log, "Communicator::C_Core::UnRegisterLocalTalker() - Unable to delete output port for player %d.", userIndex);	
			}
			if (CELL_OK != micRes)
			{
				LOGWARN(g_Log, "Communicator::C_Core::UnRegisterLocalTalker() - Unable to delete mic port for player %d.", userIndex);	
			}


			m_LocalRegisteredIndexes &= ~(1<<userIndex);
			portData = blankPortData;

			U8 bitsSet = 0;
			for (U8 i=0; i<7; i++)
			{
				if (i&1<<i)
					bitsSet += 1;
			}

			// if no more local players registeded then stop voice system.
			if (0 == bitsSet)
				m_StartAsap = false;
		}
#endif

		return true;
	}


	// register a remote talker so we can decode any voice packets they send.
	bool C_Core::RegisterRemoteTalker(const SysProfile::UserIdentifier& remoteTalker)
	{
		if (!m_Initialised)
			return false;

		if (0 == m_MaxRemoteTalkers)
			return false;	// we specified no remote players on startup

#if defined ( PS3_VOICE_ENABLED )

		if (IsRemoteTalkerRegistered(remoteTalker))
			return true;

		C_RemoteTalkerList* pPool = reinterpret_cast<C_RemoteTalkerList*> (m_pRemoteTalkerPool);
		C_RemoteTalkerList* pList = reinterpret_cast<C_RemoteTalkerList*> (m_pRemoteTalkerList);

		if (pPool->Empty())
			return false; // not enough open slots top add new player to remote talkers list

		C_RemoteTalkerList::Iterator it;
		it = pPool->Begin();
		pPool->Erase(pPool->Begin());
		it->m_LocalMuted	= 0;		
		it->m_RemoteMuted	= 0;
		it->m_NeedsUpdate	= false;
		it->m_UserData		= remoteTalker;
		it->m_Ports.Clear();
		pList->PushBack(*it);

		bool error = false;
		U8 count = 0;
		for (U8 i=0; i<7 && !error; i++)
		{
			if (m_LocalRegisteredIndexes&(1<<i))
			{
				const LocalPlayerPortData& localPort = m_LocalPortData[i];

				if (it->m_Ports.Size() != it->m_Ports.Capacity())
				{
					it->m_Ports.PushBack();
					PS3RemotePort& remotePort = it->m_Ports.Back();
					if (CreateVoiceInPort(&remotePort.m_VoiceInPort) == CELL_OK)
					{
						remotePort.m_InPortCreated = 1;
						if (ConnectInPortToOutPort(remotePort.m_VoiceInPort, localPort.m_HeadsetPort) == CELL_OK)
						{
							remotePort.m_ConnectionActive = true;
							remotePort.m_LocalPlayerConnection = i;
							count += 1;
						}
						else 
							error = true;
					}
					else
						error = true;
				}
				else
					error = true;
			}
		}

		if (error || 0 == count)
		{
			UnRegisterRemoteTalker(remoteTalker);
			return false;
		}

		return true;
#endif

		return false;
	}

	bool C_Core::IsRemoteTalkerRegistered(const SysProfile::UserIdentifier& remoteTalker)
	{
		if (!m_Initialised)
			return false;

#if defined ( PS3_VOICE_ENABLED )
		C_RemoteTalkerList* pList = reinterpret_cast<C_RemoteTalkerList*> (m_pRemoteTalkerList);
		C_RemoteTalkerList::Iterator it;
		for (it = pList->Begin(); it != pList->End(); ++it)
		{
			// for offline ps3 we won't have a valid SceNpId as it will be blagged to the IP address of the console,
			// or to a randon number.
			if (0 == memcmp(&remoteTalker.m_Uid, &(it->m_UserData.m_Uid), sizeof(remoteTalker.m_Uid)))
				return true;
		}
#endif

		return false;
	}

	// unregister a remote talker to stop voice being decoded for this player.
	bool C_Core::UnRegisterRemoteTalker(const SysProfile::UserIdentifier& remoteTalker)
	{
		if (!m_Initialised)
			return false;

#if defined ( PS3_VOICE_ENABLED )
		C_RemoteTalkerList* pList = reinterpret_cast<C_RemoteTalkerList*> (m_pRemoteTalkerList);
		C_RemoteTalkerList* pPool = reinterpret_cast<C_RemoteTalkerList*> (m_pRemoteTalkerPool);

		C_RemoteTalkerList::Iterator it;
		for (it = pList->Begin(); it != pList->End(); ++it)
		{
			// for offline ps3 we won't have a valid SceNpId as it will be blagged to the IP address of the console,
			// or to a randon number.
			if (0 == memcmp(&remoteTalker.m_Uid, &(it->m_UserData.m_Uid), sizeof(remoteTalker.m_Uid)))
			{
				for (S32 i=0; i<it->m_Ports.Size(); i++)
				{
					if (it->m_Ports[i].m_ConnectionActive)
					{
						// disconnect port from local headset.. local player must still be registered.
						const LocalPlayerPortData& localPortData = m_LocalPortData[it->m_Ports[i].m_LocalPlayerConnection];
						S32 res = DisconnectInPortFromOutPort(it->m_Ports[i].m_VoiceInPort, localPortData.m_HeadsetPort);
						if (CELL_OK != res)
						{
							LOGWARN(g_Log, "Communicator::C_Core::UnRegisterRemoteTalker() - Unable to disconnect voice input port from headset port.");
						}
					}

					if (it->m_Ports[i].m_InPortCreated)
					{
						// delete voice input port.
						S32 res = DeletePort(it->m_Ports[i].m_VoiceInPort);
						if (CELL_OK != res)
						{
							LOGWARN(g_Log, "Communicator::C_Core::UnRegisterRemoteTalker() - Unable to delete voice input port.");
						}
					}
				}

				// remove player from list and give back to pool.
				C_RemoteTalkerList::Iterator cur = it;
				it = pList->Erase(cur);
				pPool->PushBack(*cur);
				return true;
			}
		}
#endif

		return false;
	}

		// Unregister all players both local and remote.
	bool C_Core::UnRegisterAllPlayers(void)
	{
		if (!m_Initialised)
			return false;

		C_RemoteTalkerList* pPool = static_cast<C_RemoteTalkerList*> (m_pRemoteTalkerPool);
		C_RemoteTalkerList* pList = static_cast<C_RemoteTalkerList*> (m_pRemoteTalkerList);

		assert(pList);
		assert(pPool);

		C_RemoteTalkerList::Iterator it;
		while (pList->Size() > 0)
			UnRegisterRemoteTalker(pList->Begin()->m_UserData);

		for (U8 i=0;i<7; i++)
		{
			if (m_LocalRegisteredIndexes&(1<<i))
				UnRegisterLocalTalker(i);
		}

		return true;
	}

	// change a remote players decoding priority
	bool C_Core::SetDecodingPriority(const U8 userIndex, const SysProfile::UserIdentifier& remoteTalker, const U8 decodePriority, const bool muteRemoteTalker)
	{
		if (!m_Initialised)
			return false;

#if defined ( PS3_VOICE_ENABLED )
#endif

		return true;
	}


	// does a specific player have a communicator connected.
	bool C_Core::CommunicatorConnected(const U8 userIndex)
	{
		if (!m_Initialised)
			return false;

#if defined ( PS3_VOICE_ENABLED )
		if (m_Running)
		{
			if (userIndex < 7 && m_LocalRegisteredIndexes&(1<<userIndex))
			{
				const LocalPlayerPortData& portData = m_LocalPortData[userIndex];

				CellVoiceBasePortInfo info;
				memset(&info, 0, sizeof(info));
				if (cellVoiceGetPortInfo(portData.m_MicPort, &info) == CELL_OK)
				{
					switch (info.state)
					{
					case CELLVOICE_PORTSTATE_READY:
					case CELLVOICE_PORTSTATE_RUNNING:
						return true;
						break;
					}
				}
			}
		}
#endif

		return false;
	}

	
	// is a registered local player currently talking..
	bool C_Core::IsLocalTalking(const U8 userIndex)
	{
		if (!m_Initialised)
			return false;

#if defined ( PS3_VOICE_ENABLED )
		if (m_Running)
		{
			if (userIndex < 7 && m_LocalRegisteredIndexes&(1<<userIndex))
			{
				const LocalPlayerPortData& portData = m_LocalPortData[userIndex];

				CellVoiceBasePortInfo info;
				memset(&info, 0, sizeof(info));
				if (cellVoiceGetPortInfo(portData.m_VoiceOutPort, &info) == CELL_OK)
				{
					switch (info.state)
					{
					case CELLVOICE_PORTSTATE_RUNNING:
						return true;
						break;
					}
				}
			}
		}
#endif

		return false;
	}


	// is a registered remote player talking..
	bool C_Core::IsRemoteTalking(const SysProfile::UserIdentifier& remoteTalker)
	{
		if (!m_Initialised)
			return false;

#if defined ( PS3_VOICE_ENABLED )
		C_RemoteTalkerList* pList = reinterpret_cast<C_RemoteTalkerList*> (m_pRemoteTalkerList);
		C_RemoteTalkerList::Iterator it;
		for (it = pList->Begin(); it != pList->End(); ++it)
		{
			// for offline ps3 we won't have a valid SceNpId as it will be blagged to the IP address of the console,
			// or to a randon number.
			if (0 == memcmp(&remoteTalker.m_Uid, &(it->m_UserData.m_Uid), sizeof(remoteTalker.m_Uid)))
			{
				for (S32 i=0; i<it->m_Ports.Size(); i++)
				{
					if (it->m_LocalMuted & (1<<it->m_Ports[i].m_LocalPlayerConnection) || it->m_RemoteMuted & (1<<it->m_Ports[i].m_LocalPlayerConnection))
					{
						// this remote player should be muted.
					}
					else
					{
						CellVoiceBasePortInfo info;
						memset(&info, 0, sizeof(info));
						if (cellVoiceGetPortInfo(it->m_Ports[i].m_VoiceInPort, &info) == CELL_OK)
						{
							switch (info.state)
							{
							case CELLVOICE_PORTSTATE_RUNNING:
								return true;
								break;
							}
						}
					}
				}
			}
		}
#endif

		return false;
	}

	U32 C_Core::AudioFrameSize(void) const
	{
#if defined ( PS3_VOICE_ENABLED )
		switch(BIT_RATE)
		{
		case CELLVOICE_BITRATE_7300:
			return 19;
			break;
		case CELLVOICE_BITRATE_14400:
			return 27;
			break;
		case CELLVOICE_BITRATE_16000:
			return 20;
			break;
		case CELLVOICE_BITRATE_22533:
			return 43;
			break;
		default:
			LOGFATAL(g_Log, "Communicator::C_Core::AudioFrameSize() - invalid bit rate specified.");
			break;
		}
		return 0;
#else
		return 20;
#endif
	}

	// encode voice for a local player
	bool C_Core::GetLocalVoiceData(const U8 userIndex, U8* pOutputBuffer, U32 outputBufferSize, U32& numBytesEncoded)
	{
		numBytesEncoded = 0;

		if (!m_Initialised)
			return false;

		if (NULL == pOutputBuffer)
			return false; // wtf.. no buffer specified.

		if (outputBufferSize < AudioFrameSize())
		{
			LOGWARN( g_Log,  "Communicator::C_Core::GetLocalVoiceData() - Buffer size too small to hold a single encoded voice packet");
			return false;
		}

#if defined ( PS3_VOICE_ENABLED )
		if (m_Running)
		{
			if (userIndex < 7 && m_LocalRegisteredIndexes&(1<<userIndex))
			{
				const LocalPlayerPortData& portData = m_LocalPortData[userIndex];

				U32 size = outputBufferSize;
				if (cellVoiceReadFromOPort(portData.m_VoiceOutPort, pOutputBuffer, &size) == CELL_OK)
				{
					if (size > 0)
						numBytesEncoded = size;
				}
			}
		}
#endif

		return true;
	}

	
	bool C_Core::GetLocalVoiceData(const U8 userIndex, C_VoiceBuffer& outputBuffer, U32& numBytesEncoded)
	{
		numBytesEncoded = 0;

		if (!m_Initialised)
			return false;

		if (!outputBuffer.IsInitialised())
			return false;

		const U32 remaining = outputBuffer.Capacity() - outputBuffer.Size();
		if (remaining < AudioFrameSize())
		{
			LOGWARN( g_Log,  "Communicator::C_Core::GetLocalVoiceData() - Buffer size too small to hold a single encoded voice packet");
			return false;
		}

#if defined ( PS3_VOICE_ENABLED )
		if (m_Running)
		{
			if (userIndex < 7 && m_LocalRegisteredIndexes&(1<<userIndex))
			{
				const LocalPlayerPortData& portData = m_LocalPortData[userIndex];

				U32 size = remaining;
				if (cellVoiceReadFromOPort(portData.m_VoiceOutPort, &outputBuffer.m_pData[outputBuffer.m_CurrentSize], &size) == CELL_OK)
				{
					if (size > 0)
					{
						numBytesEncoded = size;
						outputBuffer.m_CurrentSize += size;
					}
				}
			}
		}
#endif

		return true;
	}


	// submit remote voice data for decoding and playback.
	bool C_Core::SubmitRemoteVoiceData(const SysProfile::UserIdentifier& remoteTalker, U8* pData, const U32 dataSize, U32* pDataSizeDecoded)
	{
		if(pDataSizeDecoded)
			*pDataSizeDecoded = dataSize;

		if (!m_Initialised)
			return false;

		if (0 == m_MaxRemoteTalkers)
			return false;	// we specified no remote players on startup.

		if (0 == dataSize || NULL == pData)
			return false;

#if defined ( PS3_VOICE_ENABLED )
		if (m_Running)
		{
			C_RemoteTalkerList* pList = reinterpret_cast<C_RemoteTalkerList*> (m_pRemoteTalkerList);
			C_RemoteTalkerList::Iterator it;
			for (it = pList->Begin(); it != pList->End(); ++it)
			{
				// for offline ps3 we won't have a valid SceNpId as it will be blagged to the IP address of the console,
				// or to a randon number.
				if (0 == memcmp(&remoteTalker.m_Uid, &(it->m_UserData.m_Uid), sizeof(remoteTalker.m_Uid)))
				{
					for (U8 i=0; i<it->m_Ports.Size(); i++)
					{
						CellVoiceBasePortInfo   portInfo;
						memset(&portInfo, 0, sizeof(portInfo));
						if (cellVoiceGetPortInfo(it->m_Ports[i].m_VoiceInPort, &portInfo) == CELL_OK)
						{
							U32 size = (dataSize > portInfo.numByte) ? portInfo.numByte : dataSize;
							if (size != dataSize)
							{
								LOGWARN(g_Log, "Communicator::C_Core::SubmitRemoteVoiceData() - not enough room in input buffer (%d %d) to decode all voice data. ", dataSize, portInfo.numByte);
							}

							S32 res = cellVoiceWriteToIPort(it->m_Ports[i].m_VoiceInPort, pData, &size);
						}
					}
				}
			}
		}
#endif

		return true;
	}


	bool C_Core::LocalMuted(const U8 userIndex, const SysProfile::UserIdentifier& remoteTalker)
	{
		if (!m_Initialised)
			return false;

#if defined ( PS3_VOICE_ENABLED )
		if (userIndex < 7 && m_LocalRegisteredIndexes&(1<<userIndex))
		{
			C_RemoteTalkerList* pList = reinterpret_cast<C_RemoteTalkerList*> (m_pRemoteTalkerList);
			C_RemoteTalkerList::Iterator it;
			for (it = pList->Begin(); it != pList->End(); ++it)
			{
				// for offline ps3 we won't have a valid SceNpId as it will be blagged to the IP address of the console,
				// or to a randon number.
				if (0 == memcmp(&remoteTalker.m_Uid, &(it->m_UserData.m_Uid), sizeof(remoteTalker.m_Uid)))
					return it->m_LocalMuted&(1<<userIndex) ? true : false;
			}
		}
#endif

		return false;
	}


	bool C_Core::RemoteMuted(const U8 userIndex, const SysProfile::UserIdentifier& remoteTalker)
	{
		if (!m_Initialised)
			return false;

#if defined ( PS3_VOICE_ENABLED )
		if (userIndex < 7 && m_LocalRegisteredIndexes&(1<<userIndex))
		{
			C_RemoteTalkerList* pList = reinterpret_cast<C_RemoteTalkerList*> (m_pRemoteTalkerList);
			C_RemoteTalkerList::Iterator it;
			for (it = pList->Begin(); it != pList->End(); ++it)
			{
				// for offline ps3 we won't have a valid SceNpId as it will be blagged to the IP address of the console,
				// or to a randon number.
				if (0 == memcmp(&remoteTalker.m_Uid, &(it->m_UserData.m_Uid), sizeof(remoteTalker.m_Uid)))
					return it->m_RemoteMuted&(1<<userIndex) ? true : false;
			}
		}
#endif

		return false;
	}


	void C_Core::SetMutedRemotely(const SysProfile::UserIdentifier& remoteTalker, const U8 localMutedUserIndex, const bool mute, const bool forceUpdate)
	{
		if (!m_Initialised)
			return;

#if defined ( PS3_VOICE_ENABLED )
		if (localMutedUserIndex < 7 && m_LocalRegisteredIndexes&(1<<localMutedUserIndex))
		{
			C_RemoteTalkerList* pList = reinterpret_cast<C_RemoteTalkerList*> (m_pRemoteTalkerList);
			C_RemoteTalkerList::Iterator it;
			for (it = pList->Begin(); it != pList->End(); ++it)
			{
				// for offline ps3 we won't have a valid SceNpId as it will be blagged to the IP address of the console,
				// or to a randon number.
				if (0 == memcmp(&remoteTalker.m_Uid, &(it->m_UserData.m_Uid), sizeof(remoteTalker.m_Uid)))
				{
					for (S32 i=0; i<it->m_Ports.Size(); i++)
					{
						if (localMutedUserIndex == it->m_Ports[i].m_LocalPlayerConnection)
						{
							if (mute)
							{
								if (it->m_RemoteMuted&(1<<localMutedUserIndex))
									return; // player is already muted

								if (CELL_OK == cellVoiceSetMuteFlag(it->m_Ports[i].m_VoiceInPort, mute))
									it->m_RemoteMuted |= (1<<localMutedUserIndex);
								else
								{
									LOGWARN(g_Log, "Communicator::C_Core::SetMutedRemotely() - Unable to remote mute a player.");
								}
							}
							else
							{
								if (!(it->m_RemoteMuted&(1<<localMutedUserIndex)))
									return; // player is already un-muted

								if (CELL_OK == cellVoiceSetMuteFlag(it->m_Ports[i].m_VoiceInPort, mute))
									it->m_RemoteMuted &= ~(1<<localMutedUserIndex);
								else
								{
									LOGWARN(g_Log, "Communicator::C_Core::SetMutedRemotely() - Unable to remove remote mute a player.");
								}
							}
						}
					}
				}
			}
		}
#endif
	}


	void C_Core::MuteUpdate(void)
	{		
		if (!m_Initialised)
			return;

#if defined ( PS3_VOICE_ENABLED )
		if (m_StartAsap)
		{
			if (!m_Running)
			{
				const S32 res = cellVoiceStart();
				if (CELL_OK == res || CELL_VOICE_ERROR_SERVICE_ATTACHED == res)
				{
					m_Running = true;
					LOGINFO(g_Log, "Communicator::C_Core : cellVoiceStart() - successful.");
				}
			}

			if (m_Running)
			{
				// any local player processing that needs to be done when voice service is active.
				for (U8 i=0; i<7; i++)
				{
					if (m_LocalRegisteredIndexes&(1<<i))
					{
						LocalPlayerPortData& portData = m_LocalPortData[i];
					}
				}
			}
		}
		else
		{
			if (m_Running)
			{
				const S32 res = cellVoiceStop();
				if (CELL_OK == res || CELL_VOICE_ERROR_SERVICE_DETACHED == res)
				{
					cellSysutilEnableBgmPlayback();
					m_Running = false;
					LOGINFO(g_Log, "Communicator::C_Core : cellVoiceStop() - successful.");
				}
			}
		}
#endif
	}


	// create voice engine
	bool C_Core::CreateEngine(void)
	{
		assert(!m_Initialised);
		bool started = false;

#if defined ( PS3_VOICE_ENABLED )
		CellVoiceInitParam Params;
		Params.eventMask   = (CellVoiceEventType)0;
		S32 ret = cellVoiceInit(&Params);
		if (CELL_OK == ret)
		{
			m_Systems |= (1<<0); // flag voice init has been called

			ret = cellVoiceCreateNotifyEventQueue(&m_VoiceQueue, &m_VoiceEventKey);
			if (CELL_OK == ret)
			{
				m_Systems |= (1<<1); // flag event queue created

				ret = cellVoiceSetNotifyEventQueue(m_VoiceEventKey, VOICE_EVENT_ID);
				if (CELL_OK == ret)
				{
					m_Systems |= (1<<2); // flag event queue now running.

					m_VoiceThread.Enter(m_VoiceQueue);
					started = true;

					LOGINFO(g_Log,  "Communicator::C_Core::CreateEngine() - Voice Communicator Engine Initialised.");
				}
			}
		}
#endif

		return started;
	}


	// private cleanup function.. Releases all dynamic resources and shuts down voice engine
	void C_Core::Cleanup(void)
	{
		UnRegisterAllPlayers();
		
		C_RemoteTalkerList* pPool = reinterpret_cast<C_RemoteTalkerList*> (m_pRemoteTalkerPool);
		C_RemoteTalkerList* pList = reinterpret_cast<C_RemoteTalkerList*> (m_pRemoteTalkerList);

		// release pool
		if (pPool)
		{
			pPool->Clear();
			delete pPool;
			m_pRemoteTalkerPool = NULL;
		}

		// release list
		if (pList)
		{
			pList->Clear();
			delete pList;
			m_pRemoteTalkerList = NULL;
		}

		// delete storage
		if (m_pRemoteTalkerStorage)
		{
			delete [] m_pRemoteTalkerStorage;
			m_pRemoteTalkerStorage = NULL;
		}

#if defined ( PS3_VOICE_ENABLED )
		// if event queue running then stop it
		if (m_Systems&(1<<2))
			cellVoiceRemoveNotifyEventQueue(m_VoiceEventKey);

		//if event queue created then delete it here.
		// by specifying SYS_EVENT_QUEUE_DESTROY_FORCE any thread waiting on events will be woken up and and ECANCELLED will be returned
		// from sys_event_queue_receive.
		if (m_Systems&(1<<1))
			sys_event_queue_destroy(m_VoiceQueue, SYS_EVENT_QUEUE_DESTROY_FORCE);

		// wait for thread to terminate if running.
		m_VoiceThread.Leave();

		// if voice has been initialised shut it down.
		if (m_Systems&(1<<0))
		{
			if (m_Running)
				cellVoiceEnd();
			LOGINFO(g_Log,  "Communicator::C_Core::Cleanup() - Voice Communicator Engine Closed.");
		}
#endif

		m_StartAsap						=	false;
		m_Running						=	false;
		m_Systems						=	0;
		m_Initialised					=	false;
		m_MaxLocalTalkers				=	0;
		m_MaxRemoteTalkers				=	0;
		m_LocalRegisteredIndexes		=	0;
	}
}
