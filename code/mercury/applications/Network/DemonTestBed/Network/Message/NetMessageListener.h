// ***************************************************************
//  NetMessageListener   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	game derrives a class from this to receive incoming
//			network mesages from our router.
// ***************************************************************

#ifndef NETWORK_MESSAGE_LISTENER_H
#define NETWORK_MESSAGE_LISTENER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/core.h>
#include <bdSocket/bdAddrHandle.h>

namespace RacerNet
{
	//forward declerations
	class C_NetMessage;
	struct DatagramUnReliablePacketHeader;
	struct DatagramReliablePacketHeader;
	struct DatagramAcknowledgementHeader;


	/// Derrive your class from this and register it to receive incomming network messages.
	/**	The game needs to register for unreliable and reliable incomming data through this object.
	 *	Multiple game systems can listen for incomming data but once that data has been processed it will not be passed
	 *	to the remaining listeners.
	 */
	class C_NetMessageListener
	{
	public:
		explicit C_NetMessageListener() {}
		virtual ~C_NetMessageListener() {}

		/// An unreliable message has been received from the network.
		/**	Acccept and process the incomming message or pass it onto the next registered listener.
		 *	Override this function to implement custom message processing.
		 *	@param msg will containg the message data.
		 *	@param hdr will contain the message header so the sender can be identified.
		 *	@param numUnEncryptedBytes will contain the number of un encrypyed bytes that this unreliable message contains.
		 *	@param addr latest address of the client who sent this message.
		 *	@return true if the message was processed successfully, if you return false then the message will be passed onto the next registed listener.
		 */
		virtual bool ReceiveNetMessage(C_NetMessage& msg, const DatagramUnReliablePacketHeader& hdr, const U32 numUnEncryptedBytes, bdAddrHandleRef& addr) {return false;}

		/// A reliable message has been received from the network.
		/**	Acccept and process the incomming message or pass it onto the next registered listener.
		 *	Override this function to implement custom message processing.
		 *	@param msg will containg the message data.
		 *	@param hdr will contain the message header so the sender can be identified.
		 *	@param addr latest address of the client who sent this message.
		 *	@return true if the message was processed successfully, if you return false then the message will be passed onto the next registed listener.
		 */
		virtual bool ReceiveNetMessage(C_NetMessage& msg, const DatagramReliablePacketHeader& hdr, bdAddrHandleRef& addr) {return false;}
		
		/// A reliable message has been received from the network.
		/**	Acccept and process the incomming message or pass it onto the next registered listener.
		 *	Override this function to implement custom message processing.
		 *	@param hdr will contain the message header so we can remove the outgoing message from the clients queue as it has been received.
		 *	@param addr latest address of the client who sent this message.
		 *	@return true if the message was processed successfully, if you return false then the message will be passed onto the next registed listener.
		 */
		virtual bool ReceiveNetMessage(const DatagramAcknowledgementHeader& hdr, bdAddrHandleRef& addr) {return false;}
	};
} // end namespace RacerNet

#endif // NETWORK_MESSAGE_LISTENER_H
