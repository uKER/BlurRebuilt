#ifndef NETSESSIONSTATE_H
#define NETSESSIONSTATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netSessionEnums.h"
#include "netSessionQoS.h"

// data when in a session
BD_REFERENCE(netSessionState);
class netSessionState: public bdReferencable
{
public:
	netSessionState();
	~netSessionState();
	
	NetPlay::SessionParameter*		 findParameter(const NetPlay::SessionParameter& param);	
	NetPlay::C_Session::SessionUser* findUser(const NetPlay::C_Session::SessionUser& user);
	NetPlay::C_Session::SessionUser* findUser(const SysProfile::UserIdentifier& user);
	bool							 addUser(const NetPlay::C_Session::SessionUser& user);
	bool							 removeUser(const NetPlay::C_Session::SessionUser& user);

public:
	enum
	{
		RESULT_FLAG_MISMATCH_TYPE		= (1 << 0), // '1' if different type
		RESULT_FLAG_MISMATCH_ID			= (1 << 1), // '1' if different id
		RESULT_FLAG_MISMATCH_VALUE		= (1 << 2), // '1' if different value type
		RESULT_FLAG_MISMATCH_VALUE_TYPE	= (1 << 3), // '1' if different value
	};
	// use to find parameters in the list.
	static int compareParameters(const NetPlay::SessionParameter* a, const NetPlay::SessionParameter* b);	
	
public:
	// fluff
	int									m_pad;
	int									m_titleId;
	bool								m_online;
	bool								m_hosting;
	NetPlay::SessionSecurity			m_securityInfo;

	// migration
	NetPlay::MigrationInfo				m_migrationInfo;
	
	// arbitration
	NetPlay::ArbitrationNonce			m_arbitrationInfo;

	// session params
	int									m_flags;
	NetPlay::SessionParameterList		m_parameters;
	NetPlay::SessionUserList			m_players;
	NetPlay::SessionSlotConfig			m_slotConfig;
};
#endif //NETSESSIONSTATE_H