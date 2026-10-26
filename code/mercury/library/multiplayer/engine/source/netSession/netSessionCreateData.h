#ifndef NETSESSIONCREATEDATA_H
#define NETSESSIONCREATEDATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netSessionEnums.h"
#include "netSessionQoS.h"

// data creating a host session
BD_REFERENCE(netSessionQoS);
BD_REFERENCE(netSessionCreateData);
BD_REFERENCE(netSessionState);
class netSessionCreateData: public bdReferencable
{
public:
	netSessionCreateData();
	~netSessionCreateData();

	netSessionStateRef toSessionState();

	int						m_pad;
	bool					m_online;
	bool					m_party;
	int						m_titleId;
	int						m_flags;
	netSessionQoS			m_qos;

	NetPlay::SessionParameterList	m_parameters;
	NetPlay::SessionUserList		m_players;
	NetPlay::SessionSlotConfig		m_slotConfig;
};

#endif //NETSESSIONCREATEDATA_H