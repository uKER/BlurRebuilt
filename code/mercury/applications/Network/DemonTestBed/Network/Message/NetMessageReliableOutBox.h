// ***************************************************************
//  NetMessageReliableOutBox   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Stores outgoing reliable messages until it's time to send.
// ***************************************************************

#ifndef NETWORK_MESSAGE_RELIABLE_OUTBOX_H
#define NETWORK_MESSAGE_RELIABLE_OUTBOX_H

#ifdef _MSC_VER
#pragma once
#endif

#include "../Misc/NetDefines.h"
#include "../Misc/NetAllocated.h"
#include <bdSocket/bdAddrHandle.h>

namespace RacerNet
{
	// forward declerations.
	class C_NetMessage;
	struct DatagramAcknowledgementHeader;

	/// An object for storing reliable network messages that need to be sent.
	/** Multiple messages can be queued up but only a single message will be transmitted at any one time.
	 */
	class C_NetMessageReliableOutBox : public C_NetAllocated
	{
	public:
		C_NetMessageReliableOutBox();
		virtual ~C_NetMessageReliableOutBox();

		/// Allocate dynamic resources requird for operation.
		/** @param requiredSize the number of bytes that we wish to allocate for this outbox.
		 *	@return true if the resources were allocated.
		 */
		bool Initialise(const U32 requiredSize);

		/// Release the dynamic resources.
		/** @see Initialise()
		 */
		void Shutdown(void);

		/// Totally clear the contents of the outbox.
		/** Any pending messages will be lost.
		 */
		void Flush(void);

		/// Remove any messages from the outbox that where sent on the specified channel.
		/** Only call this when destroy a specific connection otherwise the reliable transport will go out
		 *	of synchronisation.
		 */
		void RemoveUnSentMessages(const bool partyMessages);

		/// Set how often to re transmit reliable data that's in the outbox.
		/** @param interval time in seconds to wait before re transmitting the reliable packets.
		 */
		void SetReSendInterval(const float interval);

		/// Poll the outbox to send any mesasges that are pending.
		/**	Since the type of message Party/Game is encoded we nned to supply both addresses. Which once will be used will be based on the 
		 *	message header, since it knows if it was a party or game message.
		 *	This function needs to be called every frame.
		 *	@param peerPartyAddress the address of the party connection for a specific peer if applicable
		 *	@param peergameAddress the address of the game connection for a specific peer if applicable
		 */
		void Poll(bdAddrHandleRef& peerPartyAddress, bdAddrHandleRef& peerGameAddress);

		/// Attempt to add a message to the outbox for sending later.
		/** This should only be used to send reliable messages, unreliable messages will not be added.
		 *	@param messageId the Identifier for this particular network message to the recipient can decode it correctly.
		 *	@param partyMessage should be true if the message we are creating will be sent on the party channel.
		 *	@param msg correctly configured reliable message data that we wish to send. 
		 *	@param localSenderId the identifier for the local player who is sending this message so the recipient knows who it originally came from.
		 *	@param initialConnection should be false for all other messages than join reply messages.
		 */
		bool Add(C_NetMessage& msg, const U8 messageId, const bool partyMessage, const U8 localSenderId, const bool initialConnection = false);

		/// We have received an acknowledgement message from a remote peer who we have previously sent a reliable message.
		/** If this acknowlegement matches what we are currectly sending then the packet will be removed from the outbox
		 *	and the next packet will be sent (if their is one).
		 */
		void ReceiveAcknowledgement(const DatagramAcknowledgementHeader& hdr);

	private:
		C_NetMessageReliableOutBox(const C_NetMessageReliableOutBox& original);				// private copy constructor
		C_NetMessageReliableOutBox& operator = (const C_NetMessageReliableOutBox rhs);		// private un coded assignment operator

		void WriteInternal(void* pData, const U32 size);

		bool m_Initialised;
		U8 *m_pData;						// pointer to dynamically allocated outbox
		U32 m_AllocatedDataSize;			// how many bytes did we allocate for the outbox
		U32	m_BytesInOutbox;				// how many bytes are currently written into the buffer to be sent out.
		bool m_ForceSend;					// ignore the packet timer and send the current apcket on the next update
		float m_ResendInterval;				// how often do we need to resend any reliable packets..
		bdStopwatch	m_Timer;				// for tracking when to send / resend packets
		U16 m_OutgoingPacketId_Party;		// for sequencing the outgoing party packets
		U16 m_OutgoingPacketId_Race;		// for sequencing the outgoing race packets
	};

	typedef Btl::Vector<C_NetMessageReliableOutBox, Btl::VectorParameters_Array<NET_MAX_VEHICLES-1> > NetMessageOutBoxArray;

} // end namespace RacerNet

#endif // NETWORK_MESSAGE_RELIABLE_OUTBOX_H
