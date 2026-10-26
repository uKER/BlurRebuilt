// ***************************************************************
//  NetRaceModeListener   version:  1.0   ·  date: 10/01/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	register an instance of this with the network race layer 
// to receive messages about disconnections etc.
// ***************************************************************

#ifndef NETWORK_RACE_MODE_LISTENER_H
#define NETWORK_RACE_MODE_LISTENER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace RacerNet
{
	/// derrive a race mode from this and register with the network layer to receive notifications.
	class C_NetRaceModeListener
	{
	public:
		explicit C_NetRaceModeListener() {}
		virtual ~C_NetRaceModeListener() {}

		/// This will be called when a player disconnects.
		/** @param vehicleIndex the vehicle in use by the player that needs to be disabled.
		 */
		virtual void VehicleLeftRace(const U8 vehicleIndex)  = 0;
	};
} // end namespace RacerNet

#endif // NETWORK_RACE_MODE_LISTENER_H
