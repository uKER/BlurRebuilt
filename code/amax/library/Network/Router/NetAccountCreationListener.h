// ***************************************************************
//  NetAccountCreationListener   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	game derrives a class from this to receive a callback
//			when the online account creation process has completed.
// ***************************************************************

#ifndef NETWORK_ACCOUNT_CREATION_LISTENER_H
#define NETWORK_ACCOUNT_CREATION_LISTENER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "NetRouterDefines.h"

namespace RacerNet
{

	/// Derrive your class from this and register it to receive the results of an online account creation request.
	class C_NetAccountCreationListener
	{
	public:
		explicit C_NetAccountCreationListener() {}
		virtual ~C_NetAccountCreationListener() {}

		/// callback function used to inform game if the online user account was created or not.
		virtual void Finished(const ePlatformAccountCreationResult code) = 0;
	};
} // end namespace RacerNet

#endif // NETWORK_ACCOUNT_CREATION_LISTENER_H
