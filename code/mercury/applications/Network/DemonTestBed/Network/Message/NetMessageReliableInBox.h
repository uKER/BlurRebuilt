// ***************************************************************
//  NetMessageReliableInBox   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	stores incoming reliable network messages for later processing.
//			allows large messages to be reconstructed from their 
//			datagrams before giving the complete message to the game.
// ***************************************************************

#ifndef NETWORK_MESSAGE_RELIABLE_INBOX_H
#define NETWORK_MESSAGE_RELIABLE_INBOX_H

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

	/// An object for storing reliable network messages that have been received before actually processing them.
	/** This allows the remote client to send a large message as a few packets and the message to be reconstructed herre
	 * before the complete message is given to the player for processing.
	 */
	class C_NetMessageReliableInBox : public C_NetAllocated
	{
	public:
		C_NetMessageReliableInBox();
		virtual ~C_NetMessageReliableInBox();

		/// Allocate dynamic resources requird for operation.
		/** @param requiredSize the number of bytes that we wish to allocate for this in.
		 *	@return true if the resources were allocated.
		*/
		bool Initialise(const U32 requiredSize);

		/// Release the dynamic resources.
		/** @see Initialise()
		 */
		void Shutdown(void);

		/// Totally clear the contents of the inbox.
		/** Any pending messages will be lost.
		 */
		void Flush(void);

		/// Attempt to add a received reliable message to the inbox.
		/** An acknowledgement message will be sent back to the sender if the message was added or if we have already processed the message. In the process of sending
		 *	back the acknowlegement the contents of the original msg will be overwritten.
		 *	@param msg the actual message data received through the router.
		 *	@param acknowledgeAddress the return address of the sender so we can send the acknowledgement.
		 *	@param localSenderId the identifier for the local player who is will send the acknowledgement so the recipient knows who it originally came from.
		 */
		bool Add(C_NetMessage& msg, bdAddrHandleRef& acknowledgeAddress, const U8 localSenderId);

		/// Poll the queue to receive any incomming reliable messages.
		/** This function will only return a network message once we have received all the message date from the sender.
		 *	The supplied message shiould have been created large enough to hold whatever message is in the inbox, if not then the function will fail.
		 *	Call once per frame to process any messages waiting in the inbox.
		 *	@param msg network message that on exit function will contain valid data if there was a complete message in the inbox.
		 *	@param msgType if msg contains data then this will be the type of message that was originally sent.
		 *	@param partyChannel true if this message was received on the party channel.
		 *	@return true if msg contains valid data or false if no network message was available in the inbox.
		 */
		bool Poll(C_NetMessage& msg, U8& msgType, bool& partyChannel);

		/// This is automatically called when this object is removed form the linked list.
		/** The game code MUST NOT call this function directly.
		*/
		void Destroy(void) {Flush();}

	private:
		C_NetMessageReliableInBox(const C_NetMessageReliableInBox& original);				// private copy constructor
		C_NetMessageReliableInBox& operator = (const C_NetMessageReliableInBox rhs);		// private un coded assignment operator

		bool m_Initialised;
		U8 *m_pData;						// pointer to dynamically allocated outbox
		U32 m_AllocatedDataSize;			// how many bytes did we allocate for the outbox
		U32	m_BytesInInbox;					// how many bytes does the inbox currently contain.
		U16 m_LastProcessedPacketId_Party;	// what was the last packet Id we have received from the remote peer (party session)	
		U16 m_LastProcessedPacketId_Race;	// what was the last packet Id we have received from the remote peer (race session)	
	};

	typedef Btl::Vector<C_NetMessageReliableInBox, Btl::VectorParameters_Array<NET_MAX_VEHICLES-1> > NetMessageInBoxArray;

} // end namespace RacerNet

#endif // NETWORK_MESSAGE_RELIABLE_INBOX_H
