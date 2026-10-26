// ***************************************************************
//  NetSessionKeyTracker   version:  1.0   ·  date: 06/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Storage class for tracking what secure keys have been 
//			registered during the current online session. When using
//			demonware, if we migrate then we need to track what keys
//			are still active and only unregister them when no 
//			players are connected via that security id anymore.
// ***************************************************************

#ifndef __NET_SESSION_KEY_TRACKER_H__
#define __NET_SESSION_KEY_TRACKER_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/vector.h>
#include "../Misc/NetDefines.h"
#include <Network/Securekey.h>

namespace RacerNet
{
	/// Tracks all secure keys that may have been used during a single session.
	/** Each time the session migrates the host generates new security information. Any players in the session will use the currect security
	 *	whilst any new players to join will use the new security. For this reason we need to track old secure keys so we can unregister them when
	 *	we have no connections using them anymore. This is not relevant.
	 *	On Xbox360 since it manages all the key stuff in the OS, so we only track the current session security info.
	 */
	class C_NetSessionKeyTracker
	{
	public:
		C_NetSessionKeyTracker();
		virtual ~C_NetSessionKeyTracker();

	private:
		C_NetSessionKeyTracker(const C_NetSessionKeyTracker& original);				// private copy constructor
		C_NetSessionKeyTracker& operator = (const C_NetSessionKeyTracker rhs);		// private un coded assignment operator

	public:
		/// Attempt to add a new security key id for tracking.
		/** This should be called when a new session is created and when we migrate a session.
		 *	@param sessionData the session security returned by the matching servers.
		 */
		void Add(const NetPlay::SessionSecurity& sessionData);

		/// Get the current session security information.
		/** Used when new players join so we always use the most up to date key data for their secure connection.
		 *	@param outData the most recent security info for the current matching session.
		 *	@return true if outData is valis or false if not.
		 */
		bool Current(NetPlay::SessionSecurity& outData);

		/// call once per frame to unregister any key Ids that are no longer in use.
		/** @param inUse list of key ids that the game is still tracking. 
		 *	any key id that we have previously been using will be unregistered if it is not in the inUse list 
		 */
		void Poll(const SecurityKeyIds& inUse);

		/// Remove all keys from the store.
		/** Any keys that we were tracking will now be unregistered
		 */
		void Flush(void);

		/** Check all registered security ids to see if the local player is already in the supplied session.
		 *	@param secId the security id to check against.
		 *	@return true if we already have the security Id registered.
		 */
		bool IsRegistered(const bdSecurityID& secId);

	private:
		void Unregister(const bdSecurityID& id, const bool warn = false);

	private:
		bool m_Active;
		NetPlay::SessionSecurity m_Current;

		SecurityKeyIds m_Previous;
	};
}// end namespace RacerNet

#endif // __NET_SESSION_KEY_TRACKER_H__
