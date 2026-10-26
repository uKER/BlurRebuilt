// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef SIMULATION_H
#define SIMULATION_H

#include <bdExample/bdExampleCommonLobby/bdExampleCommonLobby.h>

#define RVL
#include <dwc.h>
#include <svl/dwc_svl.h>


class Simulation : public bdDemo
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		Simulation();

		~Simulation();

		virtual bdBool init(bdInt argc,
							bdNChar8* argv[]);

		virtual bdBool step(const bdFloat32 time);

		virtual void quit();

	protected:

		enum Task
		{
			START_NETWORK,
			CHECK_DNS,
			NAS_LOGON,
			GET_SVL_TOKEN,
			AUTHENTICATE_ACCOUNT,
			STOP_NETWORK
		};

		void startTask();

		void pumpTask();

		void displayTaskResult();

		// Network startup

		void startNetworkDescription(bdString& description);

		void startNetwork();

		void pumpStartNetwork();

		// DNS lookup

		void checkDNSDescription(bdString& description);

		void startCheckDNS();

		void pumpCheckDNS();

		// Authentication service

		bdAuthService* getAuthService();
		void checkAuthServiceError();

		// Logon to NAS 
		void NASLogonDescription(bdString& description);
		void startNASLogon();
		void pumpNASLogon();

		// Get SVL token 
		void getSVLTokenDescription(bdString& description);
		void startGetSVLToken();
		void pumpGetSVLToken();

		// Authenticate account 
		void authenticateAccountDescription(bdString& description);
		void startAuthenticateAccount();
		void pumpAuthenticateAccount();

		// Network stop

		void stopNetworkDescription(bdString& description);

		void stopNetwork();

	protected:

		// receives log messages
		bdLogSubscriber m_logSubscriber;

		TaskStatus m_taskStatus;
		Task m_task;
		bdString m_lastTaskDescription;

		bdBool m_changedScreen;
		bdString m_screenText;

		bdInetAddr m_authAddr;
		bdGetHostByName *m_authGetHostByName;

		bdAuthService *m_authService;

		bdLobbyErrorCode m_lobbyErrorCode;

		DWCSvlResult m_NASTokenResult;
};

#endif // SIMULATION_H
