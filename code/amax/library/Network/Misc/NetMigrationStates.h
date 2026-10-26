// ***************************************************************
//  NetMigraationStates   version:  1.0   ·  date: 02/27/2009
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Lists all the states required to migrate a matching
//			server session to a new host player.
// ***************************************************************

#ifndef __NET_MIGRATION_STATES_H__
#define __NET_MIGRATION_STATES_H__
        
#ifdef _MSC_VER
#pragma once
#endif

namespace RacerNet
{
	/// List of possible states a we go through when migrating a server session
	typedef enum
	{
		eMigrationStateNotActive = 0,				// not currently migrating
		
		eMigrationStateHostInitialDelay,			// delay period before new host starts informing peers aboyt migration
		eMigrationStateHostStart,					// new host is about to migrate to a new matching session
		eMigrationStateHostContinue,				// new host waits for his matching session to be migrated
		eMigrationStateHostWaitForPeers,			// new host waits for replies back from peers when they have migrated
		
		eMigrationStatePeerWaitingForHost,			// peer is waiting for the host to inform him to start his migration.
		eMigrationStatePeerContinue,				// peer player waits for his migration to complete.
	}eNetMigrationStates;

}	// end namespace RacerNet

#endif    // __NET_MIGRATION_STATES_H__
