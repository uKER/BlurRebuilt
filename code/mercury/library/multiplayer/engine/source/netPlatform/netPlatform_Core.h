#ifndef NETPLATFORM_CORE_H
#define NETPLATFORM_CORE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netEnums/netEnums.h"

#include <misc_shared/Messaging/Messaging.h>
#include <misc_shared/Messaging/MessageBase.h>
#include <misc_shared/Messaging/MessageHandler.h>
#include <misc_shared/Messaging/MessageMultiHandler.h>
#include <SystemProfile/SystemProfileManager.h>
#include <bdLobby/bdAuthService.h>
#include <bdSocket/bdAddrHandle.h>
#include <bdCore/bdSocket/bdSecurityKey.h>
#include <bdCore/bdSocket/bdSecurityID.h>
#include <bdCore/bdSocket/bdCommonAddr.h>

class bdGetHostByName;
class bdNetStartParams;

namespace NetPlay	{ class C_BandwidthEvaluator; }
namespace SysProfile{ struct FullProfile; }
namespace Demonware { class C_LobbyEventHandler; }

namespace netPlatform
{
	enum
	{
		NAT_TYPE_NOT_SET,
		NAT_TYPE_OPEN,
		NAT_TYPE_MODERATE,
		NAT_TYPE_STRICT,
	//--------------------
		NAT_TYPE_COUNT,
	};
	
	enum
	{
	//---------------------------------
		ADDRESS_STATUS_IDLE,
		ADDRESS_STATUS_PENDING,
		ADDRESS_STATUS_CONNECTED,
		ADDRESS_STATUS_LOST,
	};

	enum 
	{ 
		TICKET_NOT_SET,
		TICKET_PENDING,
		TICKET_SUCCESS,
		TICKET_ERROR,
	};
	
	enum 
	{ 
		SIGNIN_DIALOG_NOT_STARTED,
		SIGNIN_DIALOG_STARTED,
		SIGNIN_DIALOG_SUCCESS,
		SIGNIN_DIALOG_FAILED,
	};

	enum
	{
		STATE_NOT_SET=-1,
		STATE_RESTART,
		STATE_SIGNING_IN,
		STATE_SIGNED_IN,
		STATE_STARTING_NETWORK,
		STATE_AUTHDNS_CHECK,
		STATE_TICKET_REQUEST,
		STATE_AUTHENTICATION,
		STATE_LSGDNS_CHECK,
		STATE_LOBBY_CONNECT,
		STATE_SYSTEMLINK,
		STATE_ONLINE,
		STATE_OFFLINE,
		STATE_ERROR,		
	};

	enum
	{
		STATUS_NOT_SET,
		STATUS_SUCCESS,
		STATUS_ERROR,
		STATUS_PLATFORM_NOT_SUPPORTED,
		STATUS_SIGNIN_FAILED,
		STATUS_NETWORK_FAILED,
		STATUS_ONLINE_FAILED,
		STATUS_AUTH_FAILED,
		STATUS_LOBBY_FAILED,
		STATUS_LOBBY_DISCONNECTED,
	};

	// A class to perform platform specific initialization and clean up 
	class Core: public Messaging::C_MessageMultiHandler
	{
	//--------------------------------------------
	// instance
	//--------------------------------------------	
	public:
		static Core* instance();

		static Debug::Logger& g_Log;
		static void createInstance();
		static void destroyInstance();

	protected:
		static Core* s_instance;
		Core();
		virtual ~Core();

	//--------------------------------------------
	// interface
	//--------------------------------------------	
	public:
		void initialise();
		void shutdown();
		void update();
		bool MessageHandler(const Messaging::C_MessageBase* pMessage); 
		
		bool cableConnected();
		bool error();
		bool connecting();

		bool signedIn();
		bool startSignInDialog();
		bool continueSignInDialog(bool& error);
		
		bool requestMode(int mode);
		bool runningMode(int mode);
		
		bool runningSystemLink();
		bool runningOnline();
		bool runningOffline();
		
		bool getBandwidthEvaluation(float& upstreamKBps, float& downstreamKBps);

	public:
		virtual int				NATtype			();
		virtual int				titleId			() { return platformTitleId(); }
		virtual bool			sameSecurityKey	(const bdSecurityKey& key1, const bdSecurityKey& key2);
		virtual bool			sameSecurityId	(const bdSecurityID& id1, const bdSecurityID& id2);
		virtual bool			sameCommonAddr	(const bdCommonAddrRef& addr1, const bdCommonAddrRef& addr2);
		virtual bool			sameAddrHandle	(const bdAddrHandleRef& handle1, const bdAddrHandleRef& handle2);
		
		// randomisers
		virtual void			random(void* data, int len);
		virtual int				randomInt(int range = 10000);
		virtual int				randomInt(int min, int max);
		virtual float			randomFloat(float range=1.0f);
		virtual void			generateRandom	(void* ptr, int len);
		virtual netNonce		generateNonce	();
		virtual bool			generateKey		(bdSecurityID& secId, bdSecurityKey& secKey);
		virtual bool			sameNonce		(const netNonce& nonce1, const netNonce& nonce2);
		virtual const char*		nonceToString	(const netNonce& nonce);

		// local address
		virtual bdCommonAddrRef myCommonAddr();
		
		virtual int				getHandleStatus	(const bdAddrHandleRef& handle);
		virtual const char*		getHandleInfo	(const bdAddrHandleRef &handle, char* str, int len);
		virtual const char*		getAddrInfo		(const bdCommonAddrRef& addr, char* str, int len);
		virtual const char*		getConsoleName	(char* name, int len);
		virtual const WCHAR*	getConsoleName	(WCHAR* name, int len);
		
		virtual bool			registerKey			(bool online, const bdSecurityID& secId, const bdSecurityKey& secKey);
		virtual bool			unregisterKey		(bool online, const bdSecurityID& secId, const bdSecurityKey& secKey);
		virtual bdAddrHandleRef	registerAddress		(const bdCommonAddrRef& addr, const bdSecurityID& secId);
		virtual bool			unregisterAddress	(bdAddrHandleRef addr);
		virtual bool			connectAddress		(const bdCommonAddrRef& addr, bdAddrHandleRef &addrHandle);
		virtual bdOnlineUserID	getUserId			(const SysProfile::UserIdentifier &user);
		virtual bool			sameUser			(const SysProfile::UserIdentifier &a, const SysProfile::UserIdentifier &b);
		virtual bool			profileIdentifiable	(const SysProfile::FullProfile& profile);
		
	protected:
		// platform functions for state machine
		virtual int						platformNATtype();
		virtual void					platformInitialise(); 
		virtual void					platformShutdown();
		virtual void					platformUpdate();
		virtual bool					platformRequiresSignIn();
		virtual bool					platformRequiresAuthentication();
		virtual bool					platformRequiresTicket();
		virtual bool					platformRequiresLobby();
		virtual int						platformTitleId();
		virtual const char* 			platformAuthAddress();
		virtual int						platformAuthPort();
		virtual const char* 			platformLsgAddress();
		virtual int						platformLsgPort();
		virtual bool					platformSignIn();
		virtual bool					platformNetParams(bdNetStartParams& params);
		virtual bool					platformRequestTicket();
		virtual bool					platformAuthentication();
		virtual bool					platformCleanup();
		virtual bool					platformHasOnlineAccount();
		
		
	public:
		void updateNet();
		void updateNetPlay();
		void updateCable();
		void cleanup();
		
		bool updateState();
		bool setState(int state);
		bool setStatus(int status);

		int				m_signInState;
		
		int				m_onlineMode;
		int				m_onlineModeRequested;
		bool			m_startBandwidthEvaluation;
		bool			m_BandwidthEvaluationStarted;
		U64				m_lastBandwidthEvaluationTime;

		int				m_state;
		int				m_step;
		int				m_status;

		// pc / ps3 services
		int				m_ticketState;
		U64				m_AuthenticationTimer;
		const char*		m_Lsg;
		bdInetAddr		m_AuthAddr;
		bdGetHostByName*m_pAuthGetHostByName;
		bdAuthService*	m_pAuthService;
		bdInetAddr		m_LsgAddr;
		bdGetHostByName*m_pLsgGetHostByName;
		bdAddr			m_ContentStreamingAddress;

		// network cable notifications
		bool			m_cableConnected;
		bool			m_cableConnectChecked;
		
		// bandwidth evaluator
		F32										m_UpstreamBandwithKBpS;
		F32										m_DownstreamBandwithKBpS;
		NetPlay::C_BandwidthEvaluator*			m_pBandwidthEvaluator;
		Demonware::C_LobbyEventHandler*			m_pLobbyEventHandler;
		Messaging::C_MessageSubscriberSystem	m_subscriberSystem;

		#if !defined (CFG_GOLD)
		bdLogSubscriber m_logSubscriber;
		#endif
		
		void updateBandwidthEvaluation();
		bool startBandwidthEvaluation(void);
		bool bandwidthEvaluationInProgress(void);
		bool continueBandwidthEvaluation(bool& error);
		bool stopBandwidthEvaluation();

		void updateLobbyEventHandler();
		void stopLobbyEventHandler();

		const char* stateToString(int state);
		const char* statusToString(int status);
		const char* modeToString(int mode);
	};
};
#endif //NETPLATFORM_CORE_H