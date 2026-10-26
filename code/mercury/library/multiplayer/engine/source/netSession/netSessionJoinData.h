#ifndef NETSESSIONJOINDATA_H
#define NETSESSIONJOINDATA_H

#ifdef _MSC_VER
#pragma once
#endif


#include "netSessionEnums.h"

// data creating a peer session
BD_REFERENCE(netEnumeratedSession);
BD_REFERENCE(netSessionJoinData);
BD_REFERENCE(netSessionState);
class netSessionJoinData: public bdReferencable
{
public:
	netSessionJoinData();
	~netSessionJoinData();
	netSessionStateRef toSessionState();

	netEnumeratedSessionRef				m_enumeratedSession; // this has to be set.
	NetPlay::SessionParameterList		m_parameters;
	NetPlay::SessionUserList			m_players;
	NetPlay::SessionSlotConfig			m_slotConfig;
	int									m_pad;
	bool								m_online;
	bool								m_party;
	int									m_titleId;
	int									m_flags;
};


#endif //NETSESSIONJOINDATA_H