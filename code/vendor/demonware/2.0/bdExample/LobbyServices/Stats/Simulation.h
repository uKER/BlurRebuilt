// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef SIMULATION_H
#define SIMULATION_H

#include "LeaderBoardInfo.h"

#include <bdExample/bdExampleCommonLobby/bdExampleCommonLobby.h>
#include <bdLobby/bdStats/bdSessionNonce.h>

#define MAX_NUM_DISPLAY_STATS (25u)

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
			CREATE_ACCOUNT,			
			AUTHENTICATE_ACCOUNT,
			LOBBY_CONNECT,
			WRITE_STATS,
			READ_STATS_BY_PIVOT,
			READ_STATS_BY_ID,
			READ_STATS_BY_RANGE,
			DELETE_STATS,
			START_ARBITRATED_SESSION,
			WRITE_ARBITRATED_STATS,
			LOBBY_DISCONNECT,
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

		const bdAuthInfo *getAuthInfo();

		void checkAuthServiceError();

		void pumpAuthServiceTask();

		// Create account

		void createAccountDescription(bdString& description);

		void startCreateAccount();

		void pumpCreateAccount();

		// Authenticate account

		void authenticateAccountDescription(bdString& description);

		void startAuthenticateAccount();

		void pumpAuthenticateAccount();

		// Lobby service connect

		void lobbyServiceConnectDescription(bdString& description);

		void startLobbyServiceConnect();

		void pumpLobbyServiceConnect();

		// Stats

		void printStatsList(bdString& string) const;

		// Write stats

		void writeStatsDescription(bdString& description);

		void startWriteStats();

		void pumpWriteStats();

		// Read stats by pivot

		void readStatsByPivotDescription(bdString& description);

		void startReadStatsByPivot();

		void pumpReadStatsByPivot();

		
		// Read stats by ID

		void readStatsByIDDescription(bdString& description);

		void startReadStatsByID();

		void pumpReadStatsByID();


		// Read stats by range

		void readStatsByRangeDescription(bdString& description);

		void startReadStatsByRange();

		void pumpReadStatsByRange();

		// Read highest kill rate stats

		void readHighestKillRateDescription(bdString& description);

		void startReadHighestKillRate();

		void pumpReadHighestKillRate();

		// Delete stats

		void deleteStatsDescription(bdString& description);

		void startDeleteStats();

		void pumpDeleteStats();

		// Start arbitrated session

		void startArbitratedSessionDescription(bdString& description);

		void startArbitratedSession();

		void pumpStartArbitratedSession();

		// Write arbitrated stats

		void startWriteArbitratedStatsDescription(bdString& description);

		void startWriteArbitratedStats();

		void pumpWriteArbitratedStats();


		// Lobby disconnect

		void lobbyServiceDisconnectDescription(bdString& description);

		void startLobbyServiceDisconnect();

		void pumpLobbyServiceDisconnect();

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
		bdLobbyService *m_lobbyService;

		bdInetAddr m_lsgAddr;
		bdGetHostByName *m_lsgGetHostByName;

		LeaderBoardInfo  m_stats;
		LeaderBoardInfo2 m_stats2;
		LeaderBoardInfo m_leaderboard[MAX_NUM_DISPLAY_STATS];
		bdRemoteTaskRef  m_remoteTask;

		LeaderBoardInfo3 m_arbStats1;
		LeaderBoardInfo3 m_arbStats2;
		LeaderBoardInfo  *m_arbStats[2];
		bdSessionNonce m_sessionNonce;
		bdSessionID m_arbSessionID;

		PrintableList<LeaderBoardInfo, MAX_NUM_DISPLAY_STATS> m_statsList;

		bdBool m_pumpLobbyConnection;
		bdLobbyErrorCode m_lobbyErrorCode;
};

#endif // SIMULATION_H
