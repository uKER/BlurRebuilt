// ***************************************************************
//  NetRouterDefines   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	specific defines that are used by trhe C_NetRouter_xxx class.
// ***************************************************************

#ifndef NETWORK_ROUTER_DEFINES_H
#define NETWORK_ROUTER_DEFINES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace RacerNet
{
	/// enum used for tracking connection status to the platform servers.
	typedef enum
	{
		eConnectionStateNotActive = 0,		/**< not active. */
		eConnectionStateInProgress,			/**< currently trying to connect. */
		eConnectionStateFailed,				/**< unable to connect. */
		eConnectionStateComplete,			/**< connection complete. */
	}ePlatformConnectionState;

	/// enum for defining resons why we couldnt connect to the platform servers. 
	typedef enum
	{
		eConnectionResultNone = 0,			/**< no code set yet, maybe still conencting */
		eConnectionResultGeneric,			/**< generic timeout error */
		eConnectionResultNoServiceProvider,	/**< Not connected to the service provider. */
		eConnectionResultNoNetwork,			/**< No network addresses detected. PC only */
		eConnectionResultInvalidUserName,	/**< supplied username does not exist yet. PC only. */
		eConnectionResultInvalidPassword,	/**< supplied password is incorrect. PC only. */
	}ePlatformConnectionResult; 

	/// enum for defining errors associated with account creation.
	/** PC specific since the other platforms manage their online accounts through their own OS.
	 */
	typedef enum
	{
		eAccountCreationResultSuccess = 0,		/**< account was created. */
		eAccountCreationResultGenericError,		/**< non specific error code. */
		eAccountCreationResultNoNetwork,		/**< not connected to any network. */
		eAccountCreationResultUserNameExists,	/**< supplied username already exists. */
		eAccountCreationResultUserNameIllegal,	/**< supplied username already exists. */
		eAccountCreationResultUserNameVulgar,	/**< supplied username is classed as vulgar and can't be created. */
		eAccountCreationResultTooManyAccounts,	/**< already have the maximum number of accounts created for the supplied license code. */
		eAccountCreationResultLicenseInvalid,	/**< the supplied license code is not recognised. */
		eAccountCreationResultLicenseLocked,	/**< the supplied license code has been locked, no accounts can be created on it anymore. */
	}ePlatformAccountCreationResult;

} // end namespace RacerNet

#endif // NETWORK_ROUTER_DEFINES_H
