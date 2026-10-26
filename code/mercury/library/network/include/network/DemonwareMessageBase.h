// ***************************************************************
//  DemonwareMessageBase   version:  1.0   ·  date: 07/17/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Base structure that we derrive all message sent via 
//			the demonware custom messaging system so we can identify
//			messages from different sub systems.
// ***************************************************************

#ifndef __DEMONWARE_MESSAGE_BASE__
#define __DEMONWARE_MESSAGE_BASE__

#ifdef _MSC_VER
#pragma once
#endif

namespace NetPlay
{	
	const U32 DEMONWARE_MESSAGE_TYPE_NAME_LEN =	(32);

	/// Derrive every game message sent from the demonware message store from this structure.
	/** On PS3 and PC game invite messages are sent via the demonware message store. If the game code
	 *	wishes to use the message store also for other custom messages then these messages should also derrive
	 *	their message data structures from this.
	 *	@see bdMessageStore
	 */
 	struct DemonwareMessageBase
	{
		DemonwareMessageBase() {m_MsgType[0] = '\0';}
		DemonwareMessageBase(const char * msgType) 
		{
			if (msgType && (strlen(msgType)+1) <= DEMONWARE_MESSAGE_TYPE_NAME_LEN)
				sprintf(m_MsgType, msgType);
			else
				m_MsgType[0] = '\0';
		}

		char m_MsgType[DEMONWARE_MESSAGE_TYPE_NAME_LEN];
	};
} // namespace NetPlay

#endif // __DEMONWARE_MESSAGE_BASE__
