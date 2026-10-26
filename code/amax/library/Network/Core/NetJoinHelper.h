// ***************************************************************
//  NetJoinHelper   version:  1.0   ·  date: 08/18/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Contains information relating to joining a network game.
//			This is the interface so the UI can track the join procedure.
// ***************************************************************

#ifndef __NET_JOIN_HELPER_H__
#define __NET_JOIN_HELPER_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <bdSocket/bdAddrHandle.h>
#include <btl/Containers/vector.h>
#include "../Core/NetTimer.h"
#include "../Misc/NetJoinErrors.h"
#include "../Misc/NetJoinStates.h"
#include <SystemProfile/SystemProfile.h>

// forward declerations 
namespace NetPlay { struct SessionSecurity; }


namespace RacerNet
{
	// forward declerations.
	class C_NetRouterBase;

	/// Wrapper class containing all the required information when joining a network game.
	/** UI can get this structure from the party or race session to track a join procedure.
	 *	party and race objects should derrive from this class.
	 */
	class C_NetJoinHelper
	{
	public:
		// internally used to track join failures.
		struct ConnectionFailure
		{
			ConnectionFailure()
			{
				m_FailCode = 0;
				m_LastTick = 0;
				m_Count = 0;
				m_SenderId = 0;
				m_MsgType = 0;
				m_PartyMessage = false;
			}

			bdAddrHandleRef m_Addr;
			SysProfile::BaseProfile m_Name;
			U64 m_LastTick;	
			U64 m_ConsoleId;
			U8 m_FailCode;
			U8 m_Count;
			U8 m_SenderId;
			U8 m_MsgType;
			bool m_PartyMessage;
		};

		typedef Btl::Vector<ConnectionFailure, Btl::VectorParameters_Static<10> > Connectionfailures;

		C_NetJoinHelper();
		virtual ~C_NetJoinHelper();

		/// Get the current status of the join request.
		eNetJoinSessionStates JoinGetState(void) const {return m_JoinState;}

		/// Get the last error we encountered while trying to join a network game.
		/** This will only be valid if GetJoinState() returns eJoinStateFailed.
		 */
		eNetJoinSessionErrors JoinGetLastError(void) const {return m_LastJoinError;}

		/// Are we in the process of joining a network game.
		bool IsJoining(void) const;

		/// UI can call this to determine if we need to show the name of the peer who we could not connect to during the join.
		bool IsPeerJoinFailureDataValid(void) const { return m_PeerFailureNameValid ;}

		/// Gets the profile data (including name) of the peer who we failed to join with.
		/** only call if IsPeerJoinFailureDatValid() returns true.
		 */
		SysProfile::BaseProfile PeerJoinFailureData(void) const { return m_PeerFailureUser; }

	protected:
		// polls any join failure connections we have active.
		void JoinPollFailures(C_NetRouterBase& router);

		// this connection has failed to join for a specific reason
		void JoinFailConnection(const U8 returnMsgType, const U8 reason, const U64 consoleId, const U8 senderId, bdAddrHandleRef& addr, C_NetRouterBase& router, bool partySession);

		// this connection has failed to join for a specific reason with a specified peers name to return.
		void JoinFailConnectionWithName(const U8 returnMsgType, const U8 reason, const SysProfile::BaseProfile& name, const U64 consoleId, const U8 senderId, bdAddrHandleRef& addr, C_NetRouterBase& router, bool partySession);

		// can test if we habe this console listed in out connection failure list
		bool JoinIsConsoleFlaggedToFail(const U64 consoleId);

		/// totally kills the join helper data
		void FlushJoinHelper(const bool resetAll, C_NetRouterBase& router);

		/// Set a specific join state
		void JoinSetState(eNetJoinSessionStates s) {m_JoinState = s;}

		/// stop the join and flag why it failed.
		void JoinSetFailed(eNetJoinSessionErrors e, C_NetRouterBase& router) {FlushJoinHelper(true, router), JoinSetState(eJoinStateFailed), m_LastJoinError = e;}

		// try and start the join process going by connecting to the host.
		bool JoinStart(const NetPlay::SessionSecurity& security, const U8* pHostCommonAddress, const U64 hostConsoleId);

		// connection to host has been completed, now connecting to peers
		bool JoinEnterPeerConnectionPhase(void);

		// Call once the join procedure ahs successfully finished.
		void JoinSetFinished(void);

		// shouldwe time out the join ?
		bool JoinTimedOut(const float maxTimeInSeconds);

		// do we have a the initial connection to the host.
		bool JoinHostConnectionComplete(void);

		/// get a reference to the hosts address.
		bdAddrHandleRef JoinGetHostAddress(void) {return m_HostAddress;}

		// get the console id of the host so we can send it in the join request for verification purposes.
		U64 JoinGetHostConsoleId(void) const {return m_HostConsoleId;}

		// do we need to resend a join request datagram to the host ?
		bool JoinReSendRequired(const float timeSlice);

		// join logic can set the players profile data who we failed to connect to.
		void JoinSetPeerJoinFailureName(const SysProfile::BaseProfile& prof) { m_PeerFailureUser = prof; }
		void JoinSetPeerJoinFailureNameSetValid(const bool v) { m_PeerFailureNameValid = v; }

	private:
		eNetJoinSessionStates	m_JoinState;					// current join state
		eNetJoinSessionErrors	m_LastJoinError;				// the join error of the last game we failed to join.
		U32						m_EnumeratedSessionIndexToJoin;	// index of the game to join.
		U64						m_HostConsoleId;				// console identifier of the session host.
		bdAddrHandleRef			m_HostAddress;					// the address of the host player for initial connection purposes.
		C_NetTimer				m_JoinTimer;					// tracks how long we have been attempting to join for.
		C_NetTimer				m_ReSendTimer;					// used for resending the initial join request to the host once connected.
		Connectionfailures		m_ConnectionFailures;			// track failed connections	

		// this is to support showing what player we could not connect to from the UI.
		bool					m_PeerFailureNameValid;
		SysProfile::BaseProfile	m_PeerFailureUser;
	};
}// end namespace RacerNet

#endif //__NET_JOIN_HELPER_H__
