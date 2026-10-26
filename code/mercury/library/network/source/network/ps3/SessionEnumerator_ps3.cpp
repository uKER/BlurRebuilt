#include <StdAfx.h>
#include <Network/Core.h>
#include <Network/SessionEnumerator.h>
#include <Core/Utility/Utility.h>
#include <Network/SessionParameter.h>
#include <Network/Securekey.h>
#include <Systemprofile/SystemProfile.h>
#include <SystemProfile/GameInvite.h>
#include <Network/DemonwareSession.h>
#include <Network/Demonwaretask.h>
#include <bdNet/bdNet.h>
#include <bdSocket/bdQoS/bdQoSProbe.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdMatchMaking/bdMatchMaking.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>


namespace NetPlay
{
	// constructior
	C_SessionEnumerator::C_SessionEnumerator() :	m_Initialised(false), m_MaxSessionToEnumerate(0), m_NumSessionEnumerated(0), m_SearchState(eSearchNotActive),
		m_QoSHasDataStream(false), m_NumSessionsToReturnInQuery(0), m_CustomQuery(false),
		m_pEnumeratedSessionData(NULL), m_NumQoSProbeRepliesExpected(0), m_NumQoSProbeRepliesReceived(0)
	{
	}


	// destructor
	C_SessionEnumerator::~C_SessionEnumerator()
	{
		CleanUp();
	}

	/// registers a listener that listens to enumeration events.
	/**	@param listener object listening to enumeration events.
	 *	@return true if listener has been registered, false if already registered.
	 */
	bool C_SessionEnumerator::RegisterListener(C_SessionEnumeratorListener* listener)
	{
		for (	Btl::List<C_SessionEnumeratorListener*>::Iterator it = m_listeners.Begin();
				it != m_listeners.End(); 
				++it)
		{
			if((*it) == listener) 
				return false;
		}
		m_listeners.PushBack(listener);
		return true;
	}

	/// unregisters a listener that was previously registered.
	/**	@param listener we want to unregister.
	 *	@return true if listener has been unregistered succeffully, false if not found.
	 */
	bool C_SessionEnumerator::UnregisterListener(C_SessionEnumeratorListener* listener)
	{
		for (	Btl::List<C_SessionEnumeratorListener*>::Iterator it = m_listeners.Begin();
				it != m_listeners.End(); 
				++it)
		{
			if((*it) == listener) 
			{
				m_listeners.Erase(it);
				return true;
			}
		}
		return false;
	}

	// setup dynamic resources.
	bool C_SessionEnumerator::Initialise(const U32 maxSearchProperties, const U32 maxSearchContexts, const U32 maxSessionsToReturn)
	{
		if (m_Initialised)
			return true;

		m_MaxSessionToEnumerate	= maxSessionsToReturn;

		if (0 == maxSessionsToReturn)
			return false;

		m_Initialised = true;

		return m_Initialised;
	}


	// start a search request
	bool C_SessionEnumerator::StartSearch(const SysProfile::FullProfile& localPlayer, const U32 queryId, const U32 maxResults, const SessionParameter* pSearchParams, const U32 numSearchParams, const SesionQueryReturnTypes& returnTypes, const bool qosHasDataStream)
	{
		// kill any old searches and release data
		KillSearch();

		if (!m_Initialised)
			return false;

		// check player profile is valid.
		if (SysProfile::eProfileStateOnline != localPlayer.m_State)
			return false;

		if (0 == maxResults)
		{
			LOGFATAL( g_Log, "C_SessionEnumerator::StartSearch() - MUST pass in at least 1 for maxResults");
			return false;
		}
		else if (maxResults > m_MaxSessionToEnumerate)
		{
			LOGFATAL( g_Log, "C_SessionEnumerator::StartSearch() - maxResults is greater then what was specified for maxSessionsToReturn when Initialise() was called.");
			return false;
		}

		if (numSearchParams > BIZ_MAX_DEMONWARE_SESSION_ATTRIBUTES)
		{
			LOGFATAL( g_Log, "C_SessionEnumerator::StartSearch() - numSearchParams is greater than what it's possible to use.");
			return false;
		}

		// validate requested search parameters. They should all be of type property and defind as S32
		for (U32 i=0; i<numSearchParams; i++)
		{
			if (pSearchParams[i].m_ParameterType != SessionParameter::eTypeProperty)
			{
				LOGFATAL(g_Log, "C_SessionEnumerator::StartSearch() - Only property types are allowed when enumerating sessions.");
				return false;
			}
		}

		if (returnTypes.Size() == 0)
		{
			LOGWARN( g_Log, "C_SessionEnumerator::StartSearch() - no return types defined, Does this match the web Imp schema ?");
		}

		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();	
		if (NULL == lobbyService)
			return false;

		if (lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
			return false;

		bdMatchMaking* pMM = lobbyService->getMatchMaking();
		if (NULL == pMM)
			return false;

		// try and allocate results buffer.
		m_pEnumeratedSessionData = new C_EnumeratedDemonwareSession[m_MaxSessionToEnumerate];
		if (NULL == m_pEnumeratedSessionData)
			return false;

		// take note of how many game session attributes each session should return and the maximum number of sessions
		m_NumSessionsToReturnInQuery	=	maxResults;	
		m_QoSHasDataStream				=	qosHasDataStream;
		m_CustomQuery					=	true;

		// create the attributes thet we expect to be returned from the server as part of the query
		for (U32 i=0; i<m_MaxSessionToEnumerate; i++)
		{
			C_EnumeratedDemonwareSession& session = m_pEnumeratedSessionData[i];
			session.Flush();
			for (S32 z=0; z<returnTypes.Size(); z++)
			{
				SessionParameter p;
				switch (returnTypes[z])
				{
				case SessionParameter::Int32:
					p.CreatePropertyS32(z, 0);
					break;
				case SessionParameter::Int64:
					p.CreatePropertyS64(z, 0);
					break;
				case SessionParameter::Float32:
					p.CreatePropertyF32(z, 0.0f);
					break;
				case SessionParameter::Float64:
					p.CreatePropertyF64(z, 0);
					break;

				default:
					KillSearch();
					return false;
					break;
				}

				// add configured property to session structure.
				session.m_Attributes[session.m_NumAttributesInUse++] = p;
			}
		}

		// configure query.
		C_DemonwareQuery query;
		for (U32 i=0; i<numSearchParams; i++)
			query.m_Attributes[query.m_NumAttributesInUse++] = pSearchParams[i];

		m_AsyncTask = pMM->findSessions(queryId, 0, m_MaxSessionToEnumerate, &query, m_pEnumeratedSessionData);
		if (!NetPlay::DemonwareRemoteTask::IsBusy(m_AsyncTask))
			KillSearch();
		else
			m_SearchState = eSearching;


		// notify listeners
		if(m_SearchState != eSearchNotActive)
		{
			for (	Btl::List<C_SessionEnumeratorListener*>::Iterator it = m_listeners.Begin();
					it != m_listeners.End(); 
					++it)
			{
				(*it)->OnSessionEnumeratorStarted(this);
			}
		}		

		return m_SearchState != eSearchNotActive;
	}


	// Start searching for a a specific game session that we have been invited to.
	bool C_SessionEnumerator::StartSearch(const SysProfile::FullProfile& localPlayer, const SysProfile::GameInvite& gameInvite, const SesionQueryReturnTypes& returnTypes, const bool qosHasDataStream)
	{
		if (!m_Initialised)
			return false;

		// kill any old searches and release data
		KillSearch();

		// check player profile is valid.
		if (SysProfile::eProfileStateOnline != localPlayer.m_State)
			return false;

		if (!gameInvite.m_Active)
			return false; // game invite data isn't valid.

		if (returnTypes.Size() == 0)
		{
			LOGWARN( g_Log, "C_SessionEnumerator::StartSearch() - no return types defined, Does this match the web Imp schema ?");
		}

		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (NULL == lobbyService)
			return false;

		if (lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
			return false;

		bdMatchMaking* pMM = lobbyService->getMatchMaking();
		if (NULL == pMM)
			return false;

		// try and allocate results buffer. 
		m_pEnumeratedSessionData = new C_EnumeratedDemonwareSession[m_MaxSessionToEnumerate];
		if (NULL == m_pEnumeratedSessionData)
			return false;

		// this type of query brings back no game defined parameters !!
		m_NumSessionsToReturnInQuery	=	1;
		m_QoSHasDataStream				=	qosHasDataStream;
		m_CustomQuery					=	false;

		// create the attributes that we expect to be returned from the server as part of the query
		for (U32 i=0; i<m_MaxSessionToEnumerate; i++)
		{
			C_EnumeratedDemonwareSession& session = m_pEnumeratedSessionData[i];
			session.Flush();
			for (S32 z=0; z<returnTypes.Size(); z++)
			{
				SessionParameter p;
				switch (returnTypes[z])
				{
				case SessionParameter::Int32:
					p.CreatePropertyS32(z, 0);
					break;
				case SessionParameter::Int64:
					p.CreatePropertyS64(z, 0);
					break;
				case SessionParameter::Float32:
					p.CreatePropertyF32(z, 0.0f);
					break;
				case SessionParameter::Float64:
					p.CreatePropertyF64(z, 0);
					break;

				default:
					KillSearch();
					return false;
					break;
				}

				// add configured property to session structure.
				session.m_Attributes[session.m_NumAttributesInUse++] = p;
			}
		}

		bdSessionID sessionId;
		for (U32 z=0; z<BD_SECURITY_ID_LENGTH; z++)
			sessionId.m_sessionID.ab[z] = gameInvite.m_Id.ab[z];

		m_AsyncTask = pMM->findSessionFromID(sessionId, &m_pEnumeratedSessionData[0]);
		if (!NetPlay::DemonwareRemoteTask::IsBusy(m_AsyncTask))
			KillSearch();
		else
			m_SearchState = eSearching;

		// notify listeners
		if(m_SearchState != eSearchNotActive)
		{
			for (	Btl::List<C_SessionEnumeratorListener*>::Iterator it = m_listeners.Begin();
					it != m_listeners.End(); 
					++it)
			{
				(*it)->OnSessionEnumeratorStarted(this);
			}
		}		

		return m_SearchState != eSearchNotActive;
	}

	// Performs any asynchronous tasks, should call at all times
	void C_SessionEnumerator::Poll(void)
	{
		if (!m_Initialised)
			return;

		if (eSearchNotActive == m_SearchState)
			return;

		if (eSearching == m_SearchState)
		{
			if (NetPlay::DemonwareRemoteTask::IsBusy(m_AsyncTask))
				return;

			bool error = NetPlay::DemonwareRemoteTask::CheckState(m_AsyncTask) == NetPlay::DemonwareRemoteTask::eFailed ? true : false;
			if (error)
				KillSearch();
			else
			{
				m_NumSessionEnumerated = m_AsyncTask->getNumResults();
				if (0 == m_NumSessionEnumerated)
					KillSearch();
				else
				{
					// can close the task now
					NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);

					if (m_QoSHasDataStream)
					{
						// move on to Quality of service probes..
						m_SearchState = eQos;

						// start QoS scan on enumerated sessions.
						bdNetImpl *const net = bdNet::getInstance();
						if (net->getStatus() == bdNetImpl::BD_NET_DONE)
						{
							bdQoSProbe* pProbe = net->getSocketRouter()->getQoSProber();
							if (pProbe)
							{
								m_NumQoSProbeRepliesExpected = m_NumSessionEnumerated;
								m_NumQoSProbeRepliesReceived = 0;

								for(U32 i=0; i<m_NumSessionEnumerated; i++)
								{
									bdSecurityKey key;
									for (U32 z=0; z<BD_SECURITY_KEY_LENGTH; z++)
										key.ab[z] = m_pEnumeratedSessionData[i].m_SecurityKey[z];
									
									bdCommonAddrRef myCommonAddress = net->getLocalCommonAddr();
									bdQoSRemoteAddr qosAddr(m_pEnumeratedSessionData[i].getHostAddrAsCommonAddr(myCommonAddress), m_pEnumeratedSessionData[i].m_sessionID, key);
									if (!pProbe->probe(qosAddr, this))
									{
										KillQoS();
										KillSearch();
										return;
									}
								}
							}
							else
								KillSearch();
						}
						else
							KillSearch();
					}
					else
					{
						// notify listeners.
						if(!m_listeners.Empty())
						{
							
							// iterate all sessions
							for(int i = 0; i < NumSessionsEnumerated(); i ++)
							{							
								SessionQoSData qosData;
								U32 openSlots;
								U32 maxSlots;
								SessionSecurity security;
								bdCommonAddrRef address;
								Btl::Vector< NetPlay::SessionParameter > params;

								if(	GetQoSDataFromEnumerated(i, qosData) && 
									GetSlotInfoFromEnumerated(i, openSlots, maxSlots) && 
									GetHostInfoFromEnumerated(i, security, address) &&
									GetParametersFromEnumerated(i, params))
								{
									// notify listeners
									for (	Btl::List<C_SessionEnumeratorListener*>::Iterator it = m_listeners.Begin();
											it != m_listeners.End(); 
											++it)
									{
											(*it)->OnSessionEnumerated(	this, 
																		i, 
																		params,
																		qosData,
																		security,
																		address,
																		openSlots,
																		maxSlots);
									}
								}
							}

							// notify listeners end of enumeration.
							for (	Btl::List<C_SessionEnumeratorListener*>::Iterator it = m_listeners.Begin();
									it != m_listeners.End(); 
									++it)
							{
								(*it)->OnSessionEnumeratorFinished(this, m_NumSessionEnumerated);
							}
						}

						// No Qos Data to return so enumeration is now complete.
						m_SearchState = eSearchNotActive;
					}
				}
			}
		}
		else if (eQos == m_SearchState)
		{
			// wait until all Qos replies have been received or timedout.
			if (m_NumQoSProbeRepliesExpected == m_NumQoSProbeRepliesReceived)
			{
				// sort results.
				FinaliseResults();

				// game can now access search results.
				m_SearchState = eSearchNotActive;
			}
		}
	}


	// how many sessions did we enumerate..
	U32	C_SessionEnumerator::NumSessionsEnumerated(void) const
	{
		if (!m_Initialised)
			return 0;
		if (m_SearchState != eSearchNotActive)
			return 0;
		return m_NumSessionEnumerated;
	}


	// get the hosting part of the enumerated session
	bool C_SessionEnumerator::GetHostInfoFromEnumerated(const U32 sessionIndex, SessionSecurity& sessionSecurity, bdCommonAddrRef& sessionAddress) const
	{
		if (!m_Initialised)
			return false;

		//if (eSearchNotActive != m_SearchState)
		//	return false; // no search active

		//if (0 == m_NumSessionEnumerated)
		//	return false; // some sessions enumerated

		//if (sessionIndex >= m_NumSessionEnumerated)
		//	return false; 

		if (sessionIndex >= m_MaxSessionToEnumerate)
			return false; 

		C_EnumeratedDemonwareSession* pS = &m_pEnumeratedSessionData[sessionIndex];

		sessionSecurity.m_ArbitrationNonce = 0;

		for (U32 z=0; z<BD_SECURITY_KEY_LENGTH; z++)
			sessionSecurity.m_Key.ab[z] = pS->m_SecurityKey[z];

		for (U32 z=0; z<BD_SECURITY_ID_LENGTH; z++)
			sessionSecurity.m_Id.ab[z] = pS->m_sessionID.ab[z];

		bdNetImpl *const net = bdNet::getInstance();
		bdCommonAddrRef myCommonAddress = net->getLocalCommonAddr();

		sessionAddress = pS->getHostAddrAsCommonAddr(myCommonAddress);

		return true; 
	}


	// get open and max player slots for the specified session
	bool C_SessionEnumerator::GetSlotInfoFromEnumerated(const U32 sessionIndex, U32& openSlots, U32& maxSlots) const
	{
		if (!m_Initialised)
			return false;

		//if (eSearchNotActive != m_SearchState)
		//	return false; // no search active

		//if (0 == m_NumSessionEnumerated)
		//	return false; // some sessions enumerated

		//if (sessionIndex >= m_NumSessionEnumerated)
		//	return false; 

		if (sessionIndex >= m_MaxSessionToEnumerate)
			return false; 

		C_EnumeratedDemonwareSession* pS = &m_pEnumeratedSessionData[sessionIndex];
		openSlots	=	pS->m_maxPlayers - pS->m_numPlayers; 
		maxSlots	=	pS->m_maxPlayers;

		if (pS->m_numPlayers >= pS->m_maxPlayers)
			openSlots = 0;

		return true;
	}


	/// extract a specific parameter from the unumerated session data
	bool C_SessionEnumerator::GetSessionParameterFromEnumerated(const U32 sessionIndex, const U32 paramId, SessionParameter& paramData) const
	{	
		if (!m_Initialised)
			return false;

		//if (eSearchNotActive != m_SearchState)
		//	return false; // no search active

		//if (0 == m_NumSessionEnumerated)
		//	return false; // some sessions enumerated

		//if (sessionIndex >= m_NumSessionEnumerated)
		//	return false; 

		if (sessionIndex >= m_MaxSessionToEnumerate)
			return false; 

		// when using demonware its better to query your qos data for game attributes as we don't store any attributes
		// that are returned by the search query apart from the default ones. game type, session key etc.
		LOGWARN(g_Log, "C_SessionEnumerator::GetSessionParameterFromEnumerated() - not implemented");
		return false;
	}

	/// extract a specific parameter from the unumerated session data
	bool C_SessionEnumerator::GetParametersFromEnumerated(const U32 sessionIndex, Btl::Vector< SessionParameter>& paramData) const
	{
		if (!m_Initialised)
			return false;

		//if (eSearchNotActive != m_SearchState)
		//	return false; // no search active

		//if (0 == m_NumSessionEnumerated)
		//	return false; // some sessions enumerated

		//if (sessionIndex >= m_NumSessionEnumerated)
		//	return false; 

		if (sessionIndex >= m_MaxSessionToEnumerate)
			return false; 

		// when using demonware its better to query your qos data for game attributes as we don't store any attributes
		// that are returned by the search query apart from the default ones. game type, session key etc.
		return true;
	}

	// extract QOS data for specific session.
	bool C_SessionEnumerator::GetQoSDataFromEnumerated(const U32 sessionIndex, SessionQoSData& qosData) const 
	{
		if (!m_Initialised)
			return false;

		//if (eSearchNotActive != m_SearchState)
		//	return false; // no search active

		//if (0 == m_NumSessionEnumerated)
		//	return false; // some sessions enumerated

		//if (sessionIndex >= m_NumSessionEnumerated)
		//	return false; 

		if (sessionIndex >= m_MaxSessionToEnumerate)
			return false; 

		C_EnumeratedDemonwareSession* pS = &m_pEnumeratedSessionData[sessionIndex];

		qosData.m_Ping			=	static_cast<U32>(pS->m_Ping * 1000.0f);	// originally suplied in seconds by demonware
		qosData.m_QoSDataSize	=	pS->m_QoSDataSize;
		qosData.m_pQoSData		=	pS->m_QoSDataSize > 0 ? pS->m_QoSData : NULL;
		return true;
	}


	/// stop asynchronous search and release session storage
	void C_SessionEnumerator::KillSearch(void)
	{
		if (!m_Initialised)
			return;

		if(m_SearchState != eSearchNotActive)
		{
			// notify listeners
			for (	Btl::List<C_SessionEnumeratorListener*>::Iterator it = m_listeners.Begin();
					it != m_listeners.End(); 
					++it)
			{
				(*it)->OnSessionEnumeratorFinished(this, 0);
			}
		}

		KillQoS();

		NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);

		// destroy enumreated data.
		if (m_pEnumeratedSessionData)
			delete [] m_pEnumeratedSessionData;
		m_pEnumeratedSessionData = NULL;

		m_CustomQuery = false;
		m_NumSessionsToReturnInQuery = 0;
		m_NumSessionEnumerated = 0;
		m_SearchState = eSearchNotActive;
	}

	/// stop asynchronous search but keep session storage
	void C_SessionEnumerator::StopSearch(void)
	{
		if (!m_Initialised)
			return;

		// no search to stop.
		if(eSearchNotActive == m_SearchState)
			return;
		
		// stop enumeration task.
		NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);
		m_AsyncTask = NULL;

		// sort what session we have so far.
		FinaliseResults();
	}

	// filter unresolved qos, and sort sessions.
	void C_SessionEnumerator::FinaliseResults(void)
	{
		if (!m_Initialised)
			return;

		if(eSearchNotActive == m_SearchState)
			return;
		
		if(eQos != m_SearchState)
		{
			KillSearch();
			return;
		}

		// remove sessions who we failed to QoS With.
		U32 numValid = 0;
		for (U32 i=0; i<m_NumSessionEnumerated; i++)
		{
			if (m_pEnumeratedSessionData[i].m_QoSValid)
				numValid += 1;
		}

		if (0 == numValid)
		{
			KillSearch();
			return;
		}
		
		if (m_NumSessionEnumerated > 1)
		{ 
			// remove sessions that were not contacted by QoS.
			// these go to the end of the array.
			for (U32 outer=0; outer<m_NumSessionEnumerated-1; outer++)
			{
				if (!m_pEnumeratedSessionData[outer].m_QoSValid)
				{
					bool swapped = false;
					for (U32 inner=outer+1; inner<m_NumSessionEnumerated && !swapped; inner++)
					{
						if (m_pEnumeratedSessionData[inner].m_QoSValid)
						{
							swapped = true;
							Btl::Swap(m_pEnumeratedSessionData[outer], m_pEnumeratedSessionData[inner]);
						}
					}
				}
			}

			// set number of enumerated sessions after removal QoS failure sessions
			m_NumSessionEnumerated = numValid;

			// sort session data based on lowest ping.
			for (U32 outer=0; outer<m_NumSessionEnumerated-1; outer++)
			{
				S32 lowerPingIndex	= -1;
				float testPing		= m_pEnumeratedSessionData[outer].m_Ping;	

				for (U32 inner=outer+1; inner<m_NumSessionEnumerated; inner++)
				{
					if (testPing > m_pEnumeratedSessionData[inner].m_Ping)
					{
						lowerPingIndex	=	inner;
						testPing		=	m_pEnumeratedSessionData[inner].m_Ping;
					}
				}

				if (lowerPingIndex != -1) 
					Btl::Swap(m_pEnumeratedSessionData[outer], m_pEnumeratedSessionData[lowerPingIndex]);
			}
		}

		// notify listeners
		for (	Btl::List<C_SessionEnumeratorListener*>::Iterator it = m_listeners.Begin();
				it != m_listeners.End(); 
				++it)
		{
			(*it)->OnSessionEnumeratorFinished(this, m_NumSessionEnumerated);
		}
	
		// game can now access search results.
		m_SearchState = eSearchNotActive;
	}

	// termionate QoS scan and release QoS resources.
	void C_SessionEnumerator::KillQoS(void)
	{
		if (!m_Initialised)
			return;

		if (eQos != m_SearchState)
			return;

		bdNetImpl *const net = bdNet::getInstance();
		bdQoSProbe *pProbe = net->getSocketRouter()->getQoSProber();
		if (pProbe)
			pProbe->cancelProbes();

		m_NumQoSProbeRepliesExpected = 0;
		m_NumQoSProbeRepliesReceived = 0;
	}


	// private function to cleanup any dynamic resources.
	void C_SessionEnumerator::CleanUp(void)
	{
		KillSearch();

		m_Initialised = false;
	}


	void C_SessionEnumerator::onQoSProbeSuccess(const bdQoSProbeInfo &info)
	{
		if (!m_Initialised || m_SearchState != eQos)
			return;

		m_NumQoSProbeRepliesReceived += 1;

		bdNetImpl *const net = bdNet::getInstance();
		bdCommonAddrRef myCommonAddress = net->getLocalCommonAddr();

		// find this session by the host address and update the qos data.
		for (U32 i=0; i<m_NumSessionEnumerated; i++)
		{
			bdCommonAddrRef sessionHost = m_pEnumeratedSessionData[i].getHostAddrAsCommonAddr(myCommonAddress);
			if (*info.m_addr == *sessionHost)
			{
				if (info.m_dataSize > 0 && NULL != info.m_data)
				{
					if (info.m_dataSize <= BIZ_MAX_QOS_DATA_SIZE)
					{
						m_pEnumeratedSessionData[i].m_QoSValid		= true;
						m_pEnumeratedSessionData[i].m_Ping			= info.m_latency;
						m_pEnumeratedSessionData[i].m_QoSDataSize	= info.m_dataSize;
						memcpy(m_pEnumeratedSessionData[i].m_QoSData, info.m_data, info.m_dataSize);

						// notify listeners
						for (	Btl::List<C_SessionEnumeratorListener*>::Iterator it = m_listeners.Begin();
								it != m_listeners.End(); 
								++it)
						{
							SessionQoSData qosData;
							U32 openSlots;
							U32 maxSlots;
							SessionSecurity security;
							bdCommonAddrRef address;
							Btl::Vector< NetPlay::SessionParameter > params;

							if(	GetQoSDataFromEnumerated(i, qosData) && 
								GetSlotInfoFromEnumerated(i, openSlots, maxSlots) &&
								GetHostInfoFromEnumerated(i, security, address) &&
								GetParametersFromEnumerated(i, params))
							{
								(*it)->OnSessionEnumerated(	this, 
															i, 
															params,
															qosData,
															security,
															address,
															openSlots,
															maxSlots);
							}
						}
					}
				}
				return;
			}
		}
	}

	void C_SessionEnumerator::onQoSProbeFail(bdCommonAddrRef addr)
	{
		if (!m_Initialised || m_SearchState != eQos)
			return;

		m_NumQoSProbeRepliesReceived += 1;
	}
}
