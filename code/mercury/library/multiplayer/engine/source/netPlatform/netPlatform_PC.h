#ifndef NETPLATFORM_PC_H
#define NETPLATFORM_PC_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netPlatform_Core.h"

namespace netPlatform
{
	// A class to perform platform specific initialization and clean up 
	class PC: public Core
	{
	public:
		PC();
		virtual ~PC();

		virtual bool			profileIdentifiable(const SysProfile::FullProfile& profile);
		virtual bdOnlineUserID	getUserId(const SysProfile::UserIdentifier &user);
		virtual bool			sameUser(const SysProfile::UserIdentifier &a, const SysProfile::UserIdentifier &b);		
		virtual int				getHandleStatus(const bdAddrHandleRef& handle);
		
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
		virtual	int				platformStartGetHostByName(const bdNChar8 *const domainNameStr, bdGetHostByName** getHostByName, const bdInetAddr resolvedAddr);
		virtual int				platformPumpGetHostByName(bdGetHostByName **getHostByName, bdInetAddr &resolvedAddr);

	};
};
#endif //NETPLATFORM_PC_H