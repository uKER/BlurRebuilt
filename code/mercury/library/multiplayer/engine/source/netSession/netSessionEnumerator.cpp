#include "stdafx.h"
#include "netFriends/netInvite.h"
#include "./netSessionEnumerator.h"
#include "./netSessionQoS.h"
#include "./netSessionSearchData.h"
#include "netDebug/netDebug.h"
#include "netPlatform/netPlatform_Core.h"
#include "netStream/netBuffer.h"

#include <Network/SecureKey.h>
#include <SystemProfile/SystemProfile.h>
#include <SystemProfile/SystemProfileManager.h>

#include <bdNet/bdLANDiscovery/bdLANDiscoveryServer.h>
#include <bdNet/bdLANDiscovery/bdGameInfoFactory.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

netLANDiscoveryListener::netLANDiscoveryListener(netSessionSearch* search)
: m_search(search)
{}

void netLANDiscoveryListener::onDiscovery(bdGameInfoRef gameInfo)
{
	m_search->onDiscovery(gameInfo);
}

netSessionSearch::netSessionSearch()
: m_listener(this)
{
	m_state			= STATE_IDLE;
	m_step			= 0;
	m_status		= STATUS_NOT_SET;
	m_searchParams	= new netSessionSearchData;
	m_sessions		= new netSessionList;

	// lan enumerator
	m_lanEnumerator = new bdLANDiscoveryClient;
	m_lanEnumerator->registerListener(&m_listener);
	
	// online enumerator
	m_onlineEnumerator = new netSessionEnumerator;
	if(!m_onlineEnumerator->Initialise(10, 4, NET_MAX_ENUMERATED_SESSIONS))
	{
		BIZ_ASSERT_MSG(false, "failed to create online enumerator");
	}
}

netSessionSearch::~netSessionSearch()
{
	// tell servers we're out.
	assert(!searching());
	stop();
	update();
	if(m_lanEnumerator) 
		delete m_lanEnumerator;
	m_lanEnumerator = NULL;
}

void netSessionSearch::setStatus(int status)
{
	m_status = status;
}
void netSessionSearch::setState(int state)
{
	m_state = state;
	m_step = 0;
	m_stateWatch.start();
}

void netSessionSearch::onDiscovery(bdGameInfoRef gameInfo)
{	
	m_sessions->add((netEnumeratedSessionRef) gameInfo);
}

bool netSessionSearch::clear()
{
	m_sessions->clear();
	return true;
}
bool netSessionSearch::reset()
{
	stop();
	setStatus(STATUS_NOT_SET);
	return true;
}

bool netSessionSearch::stop()
{
	if(m_lanEnumerator) 
		m_lanEnumerator->unregisterListener(&m_listener);	

	if(m_onlineEnumerator.notNull())
		m_onlineEnumerator->KillSearch();

	setStatus(STATUS_ABORTED);
	setState(STATE_IDLE);
	return true;
}

bool netSessionSearch::start(netSessionSearchDataRef searchParams)
{
	// stop the current enumeration
	if(searching())
		stop();
	
	// start a new one
	m_sessions->clear();
	m_searchParams = searchParams;
	setStatus(STATUS_NOT_SET);
	setState(STATE_SEARCHING);
	return true;
}
	

bool netSessionSearch::update()
{
	if(m_lanEnumerator && (m_lanEnumerator->getStatus() == bdLANDiscoveryClient::BD_PENDING))
		m_lanEnumerator->update();

	if(m_onlineEnumerator.notNull())
		m_onlineEnumerator->Poll();

	switch(m_state)
	{
	default:
	case STATE_IDLE:
		{
			return true;
		}
	case STATE_SEARCHING:
		{
			if (m_step == 0)
			{
				if(m_searchParams->m_online)
				{
					// user of the online session
					int user = m_searchParams->m_pad;

					// profile
					SysProfile::FullProfile profile;
					if(!SysProfile::C_Manager::Instance().GetProfile(user, profile))
					{
						setStatus(STATUS_ERROR);
						setState(STATE_IDLE);
						assert(false);
						return false;
					}
		
					// start search via session
					if(m_searchParams->m_session.notNull())
					{
						SysProfile::GameInvite data;
						
						data.m_Id = m_searchParams->m_session->getSecurityID();
						data.m_Key = m_searchParams->m_session->getSecurityKey();

						char secIdStr[128];
						netDebug::getByteInfo(&data.m_Id, sizeof(data.m_Id), secIdStr, sizeof(secIdStr));
						LOGINFO(netDebug::g_Log, "<netSessionSearch> started invite search [%s]...", secIdStr);
												
						netInviteRef invite = new netInvite(data);
						if(!m_onlineEnumerator->StartSearch(profile, *invite, m_searchParams->m_returnTypes, true))
						{
							setStatus(STATUS_ERROR);
							setState(STATE_IDLE);
							assert(false);
							return false;
						}
					}
					else
					{
						// start search via parameters
						if(!m_onlineEnumerator->StartSearch(profile, 
															m_searchParams->m_queryId, 
															m_searchParams->m_maxResults, 
															m_searchParams->m_parameters.First(), 
															m_searchParams->m_parameters.Size(), 
															m_searchParams->m_returnTypes,
															m_searchParams->m_qos))
						{
							setStatus(STATUS_ERROR);
							setState(STATE_IDLE);
							assert(false);
							return false;
						}
						LOGINFO(netDebug::g_Log, "<netSessionSearch> started online search...");
					}
					m_step++;
				}
				else
				{
					// re-register listener
					m_lanEnumerator->registerListener(&m_listener);	

					if(!m_lanEnumerator->discover(m_searchParams->m_titleId, m_searchParams->m_lanSearchTime))
					{
						setStatus(STATUS_ERROR);
						setState(STATE_IDLE);
						assert(false);
						return false;
					}
					LOGINFO(netDebug::g_Log, "<netSessionSearch> started lan search...");
					m_step++;
				}
			}
			
			if(m_step == 1)
			{
				// lan search
				if(!m_searchParams->m_online)
				{
					if(m_lanEnumerator)
					{
						// still searching
						if(m_lanEnumerator->getStatus() == bdLANDiscoveryClient::BD_PENDING)
						{
						}
						// finished. wrap it up.
						else
						{
							if(m_lanEnumerator) 
								m_lanEnumerator->unregisterListener(&m_listener);	

							setStatus(STATUS_OK);
							setState(STATE_IDLE);
						}
					}
				}
				// online search
				else
				{
					// search finished
					if(!m_onlineEnumerator->IsSearching())
					{
						// enumerate sessions
						for(int i = 0; i < (int) m_onlineEnumerator->NumSessionsEnumerated(); i ++)
						{	
							// session data
							NetPlay::SessionSecurity						sec;
							bdCommonAddrRef									addr;
							NetPlay::SessionParameter						parameter;
							NetPlay::C_SessionEnumerator::SessionQoSData	qos;
							
							if(!m_onlineEnumerator->GetHostInfoFromEnumerated(i, sec, addr))
								continue;

							// get quality of service
							if(!m_onlineEnumerator->GetQoSDataFromEnumerated(i, qos))
							{
								BIZ_ASSERT_MSG(false, "No QoS enum");
								continue;
							}

							netEnumeratedSessionRef session = new netEnumeratedSession;
							session->m_enumeratedIndex = i;
							session->setTitleID(m_searchParams->m_titleId);
							session->setSecurityID(sec.m_Id);
							session->setSecurityKey(sec.m_Key);
							session->setHostAddr(addr);
							
							netBuffer qosStream((char*)qos.m_pQoSData, qos.m_QoSDataSize, qos.m_QoSDataSize);
							if(!session->m_qos->deserialise(&qosStream))
							{
								//assert(!"session Params deserialise error");
								continue;
							}

							session->m_qos->m_ping = (int)qos.m_Ping;
							LOGINFO(netDebug::g_Log, "<netSessionSearch> found session");
							onDiscovery(session);
						}
						LOGINFO(netDebug::g_Log, "<netSessionSearch> enumerated %d sessions", m_sessions->count());
						setStatus(STATUS_OK);
						setState(STATE_IDLE);
					}
				}
			}
			return true;
		}
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

