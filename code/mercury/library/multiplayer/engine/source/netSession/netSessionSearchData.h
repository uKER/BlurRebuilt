#ifndef NETSESSIONSEARCHDATA_H
#define NETSESSIONSEARCHDATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netSessionEnums.h"

// data to start a search (either by invites, sessionInfo or filters).
BD_REFERENCE(bdGameInfo);
BD_REFERENCE(netInvite);
BD_REFERENCE(netEnumeratedSession);
BD_REFERENCE(netSessionSearchData);
class netSessionSearchData: public bdReferencable
{
public:
	netSessionSearchData();
	~netSessionSearchData();

	bdGameInfoRef					m_session;
	int								m_pad;
	int								m_flags;
	bool							m_online;
	int								m_titleId;
	bool							m_qos;
	float							m_lanSearchTime;
	int								m_queryId;
	int								m_maxResults;	
	NetPlay::SessionParameterList	m_parameters;
	NetPlay::SesionQueryReturnTypes	m_returnTypes;
};


#endif //NETSESSIONSEARCHDATA_H