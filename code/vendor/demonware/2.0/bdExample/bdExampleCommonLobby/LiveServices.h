// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef LIVE_SIGN_ON_H
#define LIVE_SIGN_ON_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <xonline.h>

class LiveServices
{
	public:

		LiveServices();

		bdBool signOn(bdUInt32 titleServerID);
		bdBool pump();
		bdBool isSignedOn();
		bdBool resolveTitleAddr(bdUInt32 titleServerID,
								bdUInt16 port,
#if !defined BD_PLATFORM_XENON
								bdCommonAddrRef &addr,
#else
								bdAddr &addr,
#endif
								bdSecurityID &id,
								bdSecurityKey &key);

		bdUInt64 getXUserId();
		void getXUserName(bdNChar8 buffer[BD_MAX_USERNAME_LENGTH]);

	protected:

#if !defined BD_PLATFORM_XENON
		XONLINETASK_HANDLE m_logonTask;
		bdUInt64 m_userId;
		bdNChar8 m_userName[BD_MAX_USERNAME_LENGTH];
#else
		HANDLE				m_notifyListener;
#endif

		bdBool m_signedOn;

};


#endif // LIVE_SIGN_ON_H