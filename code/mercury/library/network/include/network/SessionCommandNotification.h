// ***************************************************************
//  SessionCommandNotification   version:  1.0   ·  date: 03/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Notifying game that a session command has completed.
// ***************************************************************

#ifndef NETWORK_SESSION_COMMAND_NOTIFICATION_H
#define NETWORK_SESSION_COMMAND_NOTIFICATION_H

#ifdef _MSC_VER
#pragma once
#endif

namespace NetPlay
{
	/// Used to flag session commands as being complete so game code can track certain commands.
	/** All session commands are asynchronous so this is a way for the game to be notified when a specific comand completes.
	 */
	struct SessionCommandNotification
	{
		SessionCommandNotification() {Reset();}
		void Reset(const U32 id = 0) {m_Finished = false; m_Id=id;}

		bool m_Finished;
		U32	m_Id;
	};
}

#endif


