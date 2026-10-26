// ***************************************************************
//  NetRace   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	handles online and lan race session processing.
// ***************************************************************

#ifndef __NET_RACE_H__
#define __NET_RACE_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "../Misc/NetQoS.h"
#include "../Misc/NetJoinErrors.h"
#include <bdSocket/bdAddrHandle.h>

namespace RacerNet
{
	// forward declerations
	class C_NetPlayerManager;
	class C_NetRouterBase;
	class C_NetMessage;
	struct DatagramReliablePacketHeader;
	struct DatagramUnReliablePacketHeader;
	class C_LanDiscoveryManager;
	class C_NetSessionKeyTracker;

	/// main class for controlling the race network channel.
	class C_NetRace
	{
	public:
		C_NetRace();
		virtual ~C_NetRace();

		/// Call once after creating object to initialise any dynamic resources.
		/**	@param pRouter pointer to the socket router object address to use as transport.
		 *	@param pPlayerManager pointer to the player manager object address.
		 *	@return true if drnamic resources were allocated correctly.
		*/
		bool Initialise(C_NetRouterBase** ppRouter, C_NetPlayerManager **ppPlayerManager);

		/// Have we allocated the dynamic resources by calling Initialise() yet.
		/** @return true if resources have been allocated.
		 */
		bool IsInitialised(void) const {return m_Initialised;}

		/// Call when we no longer need any of the resources allocated.
		/** @see Initialise()
		 */
		void Shutdown();

		/// call once per frame to update tasks
		/** @param frameTime how long has passed since last called this function.
		 */
		void Poll(const F32 frameTimeInSeconds);

		/// This totally resets the internal data to default.
		/** If we are conencted we will disconnect and destroy any online session(s) that may be active.
		 */
		void DisconnectAndClear(void);

		/// receive a reliable network message on the race channel.
		/** @param msg received reliable message data.
		 *	@param msgType the type of message that the data relates to from ..\Misc\NetDatagramPacketTypes.h.
		 *	@param senderId the id of the player within the race channel who sent this message.
		 *	@return true if the message was processed.
		 */
		bool ReceiveReliable(C_NetMessage& msg, const U8 msgType, const U8 senderId);

		/// receive an unreliable network message on the race channel.
		/** @param msg received message data.
		 *	@param msgType the type of message that the data relates to from ..\Misc\NetDatagramPacketTypes.h.
		 *	@param senderId the id of the player within the race channel who sent this message.
		 *	@param addr the network address of the peer who sent this message.
		 *	@return true if the message was processed.
		 */
		bool ReceiveUnReliable(C_NetMessage& msg, const U8 msgType, const U32 numUnEncryptedBytes, const U8 senderId, const bdAddrHandleRef& addr);

		/// Receive a join session request.
		/** @param msg received message data.
		 *	@param hdr the packet header that contains some of the remote players data.
		 *	@param addr the network address of the peer who sent this message.
		 *	@return true if the message was processed.
		*/
		bool ReceiveJoinRequest(C_NetMessage& msg, const DatagramUnReliablePacketHeader& hdr, const bdAddrHandleRef& addr);

		/// Receive a join session reply from the host.
		/** @param msg received message data.
		 *	@param hdr the packet header that contains some of the remote players data.
		 *	@param addr the network address of the host who sent this message.
		 *	@return true if the message was processed.
		 */
		bool ReceiveJoinReply(C_NetMessage& msg, const DatagramReliablePacketHeader& hdr, const bdAddrHandleRef& addr);

	private:
		C_NetRace(const C_NetRace& original);				// private copy constructor
		C_NetRace& operator = (const C_NetRace rhs);		// private un coded assignment operator
		
		// unregister security keys that are no longer in use.
		void PollSecurityKeyTracker(void);

		// send a join failed message to the return address
		void SendJoinFailed(const eNetJoinSessionErrors reason, const bdAddrHandleRef& returnAddr);

	private:
		bool m_Initialised;
		bool m_OnlineMode;
		bool m_Running;
		NetQoSRace m_QoS;
		C_NetPlayerManager** m_ppPlayerMgr;
		C_NetRouterBase** m_ppRouter;
		C_LanDiscoveryManager* m_pLanDiscoveryMgr;
		C_NetSessionKeyTracker* m_pSessionKeyTracker;
	};

}// end namespace RacerNet

#endif //__NET_RACE_H__

