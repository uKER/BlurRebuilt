#ifndef NET_START_RACE_PACKET_H
#define NET_START_RACE_PACKET_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/RaceModes/RaceModeDefines.h>
#include <Network/Misc/NetDefines.h>
#include <Network/Player/Data/NetPlayerRaceData.h>
#include <systemprofile/systemprofile.h>
#include <btl/Containers/containers.h>


namespace RacerNet
{
	struct NetStartRacePacketPlayer
	{
		SysProfile::BaseProfile	m_User;			// username + platform specific identifier.	
		U8						mJoinRef;		// Join reference
		U8						mPrimaryJoinRef;// Join reference of the vehicle that "owns" this vehicle - only applicable for AI vehicles
		U8						mStartPos;		// Start position
		U8						mTeam;			// What team is this player on?
		S8						mAiAvatarId;	// Index into the ai texture sheet
		U8						mAiMultiplayerRank;	//	ONLY used to fake a rank for AI drivers - this is NOT the one for players.

		NetPlayerVehicle		mVehicle;		// Players vehicle info
		NetPlayerLoadOut		mLoadOut;		// Player's upgrade loadout info
	};

	struct NetStartRacePacket
	{
		U64 mHostId;
		U32 mHostStartTime;

		U32	mRandomSeed;			// Seed we use on all games to keep ensure their "random" elements in fact produce identical results (initial pickup types, MP awards etc)

		U8	mNumPlayerVehicles;
		U8	mNumAIVehicles;

		U8	mOriginalHostJoinId;					// the join if of the host player who actually started the race.
		// List of rules needed

		U8 mEvntIndex;				// so clients can track the index of the current event in the group. makes migration function.	

		//Btl::Vector<U8							, Btl::VectorParameters_Array<RacerNet::NET_MAX_VEHICLES> >	mWaitAtStart;
		Btl::Vector<NetStartRacePacketPlayer	, Btl::VectorParameters_Array<RacerNet::NET_MAX_VEHICLES> >	mPlayer;
	};
}

#endif
