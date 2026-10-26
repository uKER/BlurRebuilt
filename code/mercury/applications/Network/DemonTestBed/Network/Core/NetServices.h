// ***************************************************************
//  NetServices   version:  1.0   ·  date: 04/11/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	handles the platform specific network management,
//			also manages all structures required for network play,
//			sessions, scoreboards etc.
// ***************************************************************

#ifndef __NET_SERVICES_H__
#define __NET_SERVICES_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "../Message/NetMessageListener.h"

// mercury forward declerations.
namespace Scoreboard { class C_ReaderByRank; class C_ReaderByUsers; class C_Writer; }

namespace RacerNet
{
	// forward declerations.
	class C_NetRouterBase;
	class C_NetPlayerManager;
	class C_NetRace;
	class C_NetParty;
	class C_NetTimer;

	/// Single place to access all network services from. 
	class C_NetServices : public C_NetMessageListener
	{
	public:
		/// get a reference to the static class instance
		static C_NetServices& Instance(void)	{return s_NetServices;}

		/// Initialise dynamic resources required for network services.
		/**	@param networkStackAlreadyStarted have we started the network stack externally (for the debug tool for example)
		 *	@return true if resources were alloacted.
		 */
		bool Initialise(const bool networkStackAlreadyStarted);

		/// has the netwoek services been correctly initialised.
		/** @return true if all dynamic services have been allocated and initialised.
		 */
		bool IsInitialised(void) const {return m_Initialised;}

		/// Shutdown network services and release all dynamic resources.
		/** This will also disconnect from all game servers.
		 */
		void Shutdown(void);

		/// Poll network services.
		/** Need to call this each frame.
		 */
		void Poll(void);

		/// Access to the network router.
		/** Only call this function if the net services have been initialised. 
		 *	@see Initialise()
		 *	@see IsInitialised()
		 *	@return reference to the object for sending receiving .
		 */
		C_NetRouterBase& Router(void) {return *m_pRouter;}

		/// Access to the network player manager.
		/** Only call this function if the net services have been initialised. 
		 *	@see Initialise()
		 *	@see IsInitialised()
		 *	@return reference to the object for tracking the local and remote network players.
		 */
		C_NetPlayerManager&	PlayerManager(void) {return *m_pPlayerManager;}

		/// Access to the network race manager.
		/** Only call this function if the net services have been initialised. 
		 *	@see Initialise()
		 *	@see IsInitialised()
		 *	@return reference to the object for controlling the network race.
		 */
		C_NetRace& Race(void) {return *m_pNetRace;}

		/// Access to the network party manager.
		/** Only call this function if the net services have been initialised. 
		 *	@see Initialise()
		 *	@see IsInitialised()
		 *	@return reference to the object for controlling the network party.
		 */
		C_NetParty& Party(void) {return *m_pNetParty;}

		/// Access to the internal scoreboard reader by rank.
		/** Only call this function if the net services have been initialised. 
		 *	@see Initialise()
		 *	@see IsInitialised()
		 *	@return reference to the object for reading scoreboards based on ranks.
		 */
		Scoreboard::C_ReaderByRank& ScoreboardReaderByRank(void) {return *m_pScoreboardReaderByRank;}

		/// Access to the internal scoreboard reader by users.
		/** Only call this function if the net services have been initialised. 
		 *	@see Initialise()
		 *	@see IsInitialised()
		 *	@return reference to the object for reading specific player(s) data from a scoreboard.
		 */
		Scoreboard::C_ReaderByUsers& ScoreboardReaderByUsers(void) {return *m_pScoreboardReaderByUsers;}

		/// Access to the internal scoreboard writer.
		/** Only call this function if the net services have been initialised. 
		 *	@see Initialise()
		 *	@see IsInitialised()
		 *	@return reference to the object for writing data to a scoreboard for a specific player.
		 */
		Scoreboard::C_Writer& ScoreboardWriter(void) {return *m_pScoreboardWriter;}

		/// Stops any scoreboard read / write operations that are currently in progress.
		void StopScoreboardRequests(void);

		/// When the game gets a notification from the system that mute information has changed this function should be called.
		/** The purpose of this function is to send updated mute information to either the party session or the race session so
		 *	the mute actually happens.
		 */
		void CommunicatorMuteInformationChanged(void);

	protected:
		// platform socket router passes incomming messages to these functions when we call C_NetServices::Poll()
		bool ReceiveNetMessage(C_NetMessage& msg, const DatagramUnReliablePacketHeader& hdr, const U32 numUnEncryptedBytes,  bdAddrHandleRef& addr);
		bool ReceiveNetMessage(C_NetMessage& msg, const DatagramReliablePacketHeader& hdr,  bdAddrHandleRef& addr);
		bool ReceiveNetMessage(const DatagramAcknowledgementHeader& hdr,  bdAddrHandleRef& addr);

	private:
		explicit C_NetServices();
		virtual ~C_NetServices();
		C_NetServices(const C_NetServices& original);				// private copy constructor
		C_NetServices& operator = (const C_NetServices rhs);		// private un coded assignment operator

		bool CreateRouter(void);
		bool CreatePlayerManager(void);
		bool CreateScoreboardReaders(void);
		bool CreateScoreboardWriter(void);
		bool CreateRace(void);
		bool CreateParty(void);
		bool CreateMisc(void);

	private:
		static C_NetServices			s_NetServices;
		bool							m_Initialised;
		C_NetRouterBase*				m_pRouter;
		C_NetPlayerManager*				m_pPlayerManager;
		Scoreboard::C_ReaderByRank* 	m_pScoreboardReaderByRank;
		Scoreboard::C_ReaderByUsers*	m_pScoreboardReaderByUsers;
		Scoreboard::C_Writer*			m_pScoreboardWriter;
		C_NetRace*						m_pNetRace;
		C_NetParty*						m_pNetParty;
		C_NetTimer*						m_pFrameTimer;
	};

}// end namespace RacerNet

#endif //__NET_SERVICES_H__
