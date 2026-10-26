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

namespace RacerNet
{
	/// Tracks all secure keys that may have been used during a single session.
	/** Each time the session migrates the host generates new security information. Any playre in the session will use the currect security
	 *	whilst any new players to join will use the new security. For this reason we need to track old secure keys so we can unregister them when
	 *	we have no connections using them anymore. This is not relevant on Xbox360 since it manages all the key stuff in the OS.
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
		void Add(const bdSecurityID& keyId);

		/// call once per frame to unregister any key Ids that are no longer in use.
		/** @param inUse list of key ids that the game is still tracking. 
		 *	any key id that we have previously been using will be unregistered if it is not in the inUse list 
		 */
		void Poll(const SecurityKeyIds& inUse);

		/// Remove all keys from the store.
		/** Any keys that we were tracking will now be unregistered
		 */
		void Flush(void);

	private:
		void Unregister(const bdSecurityID& id, const bool warn = false);

	private:
		bool m_Active;
#if !defined (PLATFORM_XENON)
		bdSecurityID m_Current;
		SecurityKeyIds m_Previous;
#endif
	};
}// end namespace RacerNet

#endif // __NET_SESSION_KEY_TRACKER_H__
