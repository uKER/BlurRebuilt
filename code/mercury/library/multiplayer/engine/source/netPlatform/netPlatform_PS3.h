#ifndef NETPLATFORM_PS3_H
#define NETPLATFORM_PS3_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netPlatform_Core.h"

namespace netPlatform
{
	// A class to perform platform specific initialization and clean up 
	class PS3: public Core
	{
	public:
		PS3();
		virtual ~PS3();

		virtual bool			profileIdentifiable(const SysProfile::FullProfile& profile);
		virtual bdOnlineUserID	getUserId(const SysProfile::UserIdentifier &user);
		virtual bool			sameUser(const SysProfile::UserIdentifier &a, const SysProfile::UserIdentifier &b);		
		virtual const char*		getHandleInfo(const bdAddrHandleRef &handle, char* str, int len);
		virtual int				getHandleStatus(const bdAddrHandleRef& handle);	
		virtual const char*		getAddrInfo(const bdCommonAddrRef& addr, char* str, int len);
		virtual const char*		getConsoleName(char* name, int len);

		// platform functions for state machine
		virtual int				platformNATtype();
		virtual int				platformTitleId();	
		virtual void			platformInitialise(); 
		virtual void			platformShutdown();
		virtual void			platformUpdate();
		virtual bool			platformRequiresSignIn();
		virtual bool			platformRequiresAuthentication();
		virtual bool			platformRequiresTicket();
		virtual bool			platformRequiresLobby();
		virtual const char*		platformAuthAddress();
		virtual int				platformAuthPort();
		virtual const char*		platformLsgAddress();
		virtual int				platformLsgPort();
		virtual bool			platformSignIn();
		virtual bool			platformNetParams(bdNetStartParams& params);
		virtual bool			platformRequestTicket();
		virtual bool			platformAuthentication();
		virtual bool			platformCleanup();
		virtual bool			platformHasOnlineAccount();

	public:
		void initialiseCallbacks();
		void shutdownCallbacks();
		void updateCallbacks();
		
		int				m_sysutilCallbackSlot;
		char*			m_Ps3Cookie;
		U32				m_Ps3CookieSize;
		
		static netPlatform::PS3* s_instance;
		static netPlatform::PS3* instance();
		static void sysutilCallback		(U64 status, U64 param, void *userdata);
		static void npManagerCallback	(int event, int result, void *arg);
		static int  npBasicEventHandler	(int event, int retCode, U32 reqId, void *arg);		
	};
};
#endif //NETPLATFORM_PS3_H