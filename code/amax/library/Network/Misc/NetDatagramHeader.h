// ***************************************************************
//  NetDatagramHeader   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	defines the packet headers that will be used when we
//			send any type of datagram through the network / party.
// ***************************************************************

#ifndef __NET_DATAGRAM_HEADER_H__
#define __NET_DATAGRAM_HEADER_H__

#ifdef _MSC_VER
#pragma once
#endif

namespace RacerNet
{

	/// base header used by reliable and unreliable datagrams.
	struct DatagramPacketHeader
	{
		DatagramPacketHeader() : m_EncryptedBytes(0), m_Reliable(0), m_PartyChannel(0), m_PacketType(0), m_SenderReference(0)
		{};

		U16 m_EncryptedBytes;		// must be the first 2 bytes in the data for every datagram we send !!!!
		U16	m_Reliable:1;			// is the data classed as reliable
		U16 m_PartyChannel:1;		// is this packet related to a party channel? if not then it must be related to the game channel.
		U16	m_SenderReference:6;	// id of the player who sent the message so receiver can lookup the sender when receiving packet, 63 max players (1->63, 0 is invalid and should never be used)
		U16	m_PacketType:8;			// specifies what data this packet relates to in game terms (256 different types)
	};

	/// Any unreliable data sent on the socket must have one of these headers.
	struct DatagramUnReliablePacketHeader : public DatagramPacketHeader
	{
		DatagramUnReliablePacketHeader() {};
		DatagramUnReliablePacketHeader(const U8 packetType, const U8 senderReference) { m_PacketType=packetType; m_SenderReference=senderReference;}
	};

	/// Any reliable data sent on the socket must have one of these headers.
	struct DatagramReliablePacketHeader : public DatagramPacketHeader
	{
		DatagramReliablePacketHeader() :	m_InitialConnection(0), m_FirstPacketInStream(0), 
											m_PacketSequence(0), m_PacketSize(0), m_TotalMessageSize(0)
		{m_Reliable = 1;}

		U16 m_InitialConnection:1;		// if 1 then this is a response to a join request
		U16 m_FirstPacketInStream:1;	// if 1 then this is the start of a new packet otherwise its data for an existing packet (original data was too bit to be sent in 1 packet)
		U16 m_PacketSize:14;			// how many bytes of data (including this header) will need to be transmitted when the packet is sent
		U16	m_TotalMessageSize;			// total size of message data including this header
		U16	m_PacketSequence;			// id of datagram packet so we can see if we have already received it or to acknowledge a reply from a datagram packet we sent.
	};

	/// Header for a packet sent back when a peer receives a reliable packet to acknowledge receipt.
	struct DatagramAcknowledgementHeader : public DatagramPacketHeader
	{
		DatagramAcknowledgementHeader() : m_PacketSequence(0)
		{}

		U16 m_PacketSequence;				// Id of packet that was originally sent so the sender can remove it from their outgoing packet queue.
	};

} // end namespace RacerNet

#endif // __NET_DATAGRAM_HEADER_H__
