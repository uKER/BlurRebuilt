// ***************************************************************
//  PlatformCallbacks   version:  1.0   ·  date: 06/19/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Register & Unrgister callbacks to OS functions.
// ***************************************************************

#ifndef __PLATFORM_CALLBACKS_H__
#define __PLATFORM_CALLBACKS_H__
        
#ifdef _MSC_VER  
#pragma once       
#endif          

namespace PlatformCallbacks
{
	/// Register callbacks with OS to start receiving notifications.
	void Register(void);

	/// Unregister callbacks to stop receiving OS notifications.
	/** @see Register()
	 */
	void UnRegister(void);

	/// Call once per frame to carry out any callback processing.
	/** On PS3 this calls cellSysutilCheckCallback()
	*/
	void Poll(void);
	
	/// This is used to denote XMB menu open
	/** ON PS3 this is flagged when the PS3 menu is opened and is 
	*/
#ifdef PLATFORM_PS3
	extern bool SystemMenuOpen;
#endif
}
        
#endif    // __PLATFORM_CALLBACKS_H__
