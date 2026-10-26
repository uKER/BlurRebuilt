#pragma once

#include <bdPlatform\bdPlatformSocket\bdInAddr.h>
#include <bdNet\bdGetHostByName\bdGetHostByName.h>
#include <bdLobby\bdAuthTicket.h>

typedef enum
{
	eTestStateBegin=0,
	eTestStateNetStart,
	
	eTestStateDNSLookupStart,
	eTestStateDNSlookupContinue,

	eTestStateAccountCreationStart,
	eTestStateAccountCreationContinue,

	eTestStateAuthStart,
	eTestStateAuthContinue,

	eTestStatelobbyConnectStart,
	eTestStatelobbyConnectContinue,

	eTestStateSessionCreateStart,
	eTestStateSessionCreateContinue,

	eTestStateSesionEnumerationDelay,

	eTestStateSessionEnumerateStart,
	eTestStateSessionEnumerateContinue,

	eTestStateScoreboardWriteStart,
	eTestStateScoreboardWriteContinue,

	eTestStateScoreboardReadStart,
	eTestStateScoreboardReadContinue,

	eTestStateUserFileWriteStart,
	eTestStateUserFileWriteContinue,

	eTestStateUserFileReadStart,
	eTestStateUserFileReadContinue,

	eTestStateSkillWriteStart,
	eTestStateSkillWriteContinue,

	eTestStateSkillReadStart,
	eTestStateSkillReadContinue,


	eTestStateDisconnect,
	eTestStateIdle,
}eTestState;

namespace NetPlay { class C_Session; class C_SessionEnumerator; class C_PerUserServerStorageWriter; class C_PerUserServerStorageReader; class C_PlayerSkillReader; 
					class C_PlayerSkillWriter; }
namespace Scoreboard { class C_Writer; class C_ReaderByRank; class C_ReaderByUsers; }

class bdAuthService;


class C_DemonwareTests
{
public:
	C_DemonwareTests(void);
	~C_DemonwareTests(void);

	void Start(void);
	void Poll(const F32 frameTime);
	void Stop(void);

private:
	void NetStartBegin(void);
	void NetStartContinue(void);

	void DNSLookupBegin(void);
	void DNSLookupContinue(void);

	void AccountCreationBegin(void);
	void AccountCreationContinue(void);
	
	void AccountAuthenticationBegin(void);
	void AccountAuthenticationContinue(void);

	void LobbyConnectBegin(void);
	void LobbyConnectContinue(void);

	void SessionCreateBegin(void);
	void SessionCreateContinue(void);

	void SessionEnumerateBegin(void);
	void SessionEnumerateContinue(void);
	
	void ScoreboardWriteBegin(void);
	void ScoreboardWriteContinue(void);

	void ScoreboardReadBegin(void);
	void ScoreboardReadContinue(void);

	void UserFileWriteBegin(void);
	void UserFileWriteContinue(void);

	void UserFileReadBegin(void);
	void UserFileReadContinue(void);

	void SkillWriteBegin(void);
	void SkillWriteContinue(void);

	void SkillReadBegin(void);
	void SkillReadContinue(void);


	void Disconnect(void);

	void CreateAuthService(void);

private:
	bool				m_Started;
	bool				m_PumpLobbyConnection;	
	eTestState			m_State;

	NetPlay::C_Session	*m_pSession;
	NetPlay::C_SessionEnumerator	*m_pEnumerator;

	Scoreboard::C_Writer	*m_pScoreboardWriter;
	Scoreboard::C_ReaderByRank	*m_pScoreboardReaderByRank;
	Scoreboard::C_ReaderByUsers *m_pScoreboardReaderByUsers;

	NetPlay::C_PerUserServerStorageWriter	*m_pUserFileWriter;
	NetPlay::C_PerUserServerStorageReader	*m_pUserFileReader;

	NetPlay::C_PlayerSkillReader *m_pSkillReader;
	NetPlay::C_PlayerSkillWriter *m_pSkillWriter;

	// authentication..
	bdInetAddr m_authAddr;
	bdGetHostByName *m_pAuthGetHostByName;
	// lobby secure gateway
	bdInetAddr m_lsgAddr;
	bdGetHostByName *m_pLsgGetHostByName;

	bdAuthService	*m_pAuthService;
	bdAuthInfo		m_AuthInfo;

	F32 m_DelayTime;
};
