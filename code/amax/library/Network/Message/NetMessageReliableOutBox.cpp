#include <stdafx.h>
#include <Core/Core.h>
#include <bdNet/bdNet.h>
#include <Network/Network.h>
#include "../Misc/NetDebug.h"
#include  "NetMessageReliableOutBox.h"
#include "../Misc/NetDatagramHeader.h"
#include "../Misc/NetEncrypted.h"
#include "../Router/NetRouterBase.h"

#include "NetMessage.h"

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{
	C_NetMessageReliableOutBox::C_NetMessageReliableOutBox() :	m_Initialised(false), m_pData(NULL), m_AllocatedDataSize(0), m_BytesInOutbox(0),
																m_ResendInterval(0.0f), m_ForceSend(false), m_OutgoingPacketId_Party(0), m_OutgoingPacketId_Race(0)
	{
	}

	C_NetMessageReliableOutBox::~C_NetMessageReliableOutBox()
	{
		Shutdown();
	}

	bool C_NetMessageReliableOutBox::Initialise(const U32 requiredSize)
	{
		if (m_Initialised)
			return true;
		
		assert(requiredSize);
		if (0 == requiredSize)
			return false;

		m_AllocatedDataSize = requiredSize;
		m_pData = new U8[requiredSize];
		if (m_pData)
		{
			Flush();
			m_Initialised = true;
		}
		else
			Shutdown();

		return m_Initialised;
	}

	void C_NetMessageReliableOutBox::Shutdown(void)
	{
		Flush();
		if (m_pData)
			delete [] m_pData;
		m_pData = NULL;
		m_AllocatedDataSize = 0;
		m_OutgoingPacketId_Party = 0;
		m_OutgoingPacketId_Race = 0;
		m_Initialised = false;

		SetAvailable(true);
	}

	void C_NetMessageReliableOutBox::Flush()
	{
		SetReSendInterval(0.5f);
		m_BytesInOutbox = 0;
		m_ForceSend = false;
		m_Timer.reset();

		m_OutgoingPacketId_Party = 0;
		m_OutgoingPacketId_Race = 0;
	}

	void C_NetMessageReliableOutBox::SetReSendInterval(const float interval)
	{
		if (!m_Initialised)
			return;
		assert(interval > 0.0f);
		if (interval > 0.0f)
			m_ResendInterval = interval;
		else
			m_ResendInterval = 0.5f;	// go back to default
	}

	void C_NetMessageReliableOutBox::RemoveUnSentMessages(const bool partyMessages)
	{
		if (!m_Initialised)
			return;

		if (partyMessages)
			m_OutgoingPacketId_Party = 0;
		else
			m_OutgoingPacketId_Race = 0;

		if (0 == m_BytesInOutbox)
			return;

		const U32 totalBytesToProcess = m_BytesInOutbox;
		U32 bytesProcessed = 0;
		U32 offset = 0;

		do
		{
			U8* pCurrentAddress = m_pData + offset;
			const DatagramReliablePacketHeader* pCurrentPacket = reinterpret_cast<DatagramReliablePacketHeader*> (pCurrentAddress);

			bool remove = false;
			if (partyMessages)
				remove = pCurrentPacket->m_PartyChannel == 1 ? true : false;
			else
				remove = pCurrentPacket->m_PartyChannel == 0 ? true : false;

			if (remove)
			{
				// we need to remove this packet from the stream..
				const S32 bytesToMove = (totalBytesToProcess - bytesProcessed) - pCurrentPacket->m_PacketSize;

				// recalculate new outbox size before we loose the current packet.
				m_BytesInOutbox -= pCurrentPacket->m_PacketSize;

				// say that we have processed this amount of bytes..
				bytesProcessed += pCurrentPacket->m_PacketSize; 

				if (partyMessages)
				{
					LOGINFO(g_Log, "C_NetMessageReliableOutBox::RemoveUnSentRaceMessages() - removing un sent party message type %d", pCurrentPacket->m_PacketType);
				}
				else
				{
					LOGINFO(g_Log, "C_NetMessageReliableOutBox::RemoveUnSentRaceMessages() - removing un sent race message type %d", pCurrentPacket->m_PacketType);
				}

				// re jig memory
				if (bytesToMove)
					memmove(pCurrentAddress, pCurrentAddress + pCurrentPacket->m_PacketSize, bytesToMove);
			}
			else
			{
				// skip this datagram packet
				bytesProcessed += pCurrentPacket->m_PacketSize; 
				offset += pCurrentPacket->m_PacketSize;
			}
		}while (bytesProcessed != totalBytesToProcess);

		if (m_BytesInOutbox > 0)
			m_ForceSend = true;
		else
			m_Timer.reset();
	}

	void C_NetMessageReliableOutBox::Poll(bdAddrHandleRef& peerPartyAddress, bdAddrHandleRef& peerGameAddress, C_NetRouterBase& netRouter)
	{
		if (!m_Initialised)
			return;

		if (0 == m_BytesInOutbox)
			return; // no bytes pending to be sent just yet..

		bool send = false;
		if (m_ForceSend)
		{
			send = true;
			m_ForceSend = false;
			m_Timer.reset();
			m_Timer.start();
		}
		else if (m_Timer.getElapsedTimeInSeconds() >= m_ResendInterval)
		{
			send = true;
			m_Timer.reset();
			m_Timer.start();
		}

		if (send)
		{
			const DatagramReliablePacketHeader* pCurrentPacket = reinterpret_cast<DatagramReliablePacketHeader*> (m_pData);
			// send previously configured message data .
			netRouter.SendMessage(m_pData, pCurrentPacket->m_PacketSize, pCurrentPacket->m_PartyChannel == 1 ? peerPartyAddress : peerGameAddress);

			// reset timers so we wait before sending again.
			m_Timer.start();
		}
	}

	bool C_NetMessageReliableOutBox::Add(C_NetMessage& msg, const U8 messageId, const bool partyMessage, const U8 localSenderId, const bool initialConnection)
	{
		if (!m_Initialised)
			return false;

		if (0 == msg.DataSize() || !msg.IsTerminated())
			return false; // no data to send. or mesage not closed yet.

		if (!msg.IsReliable())
			return false; //can't send unreliable mesages through the reliable transport layer.

		// how many bytes are currently free in the outbox..
		const U32 outBoxRemainingSize = m_AllocatedDataSize - m_BytesInOutbox;

		// if this message already has a header then ignore it since a new header will be appended as we add the data into the outbox as individual packets.
		S32 baseMessageSize = msg.DataSize();
		if (msg.HasHeader())
			baseMessageSize -= sizeof(DatagramReliablePacketHeader);

		if (baseMessageSize <= 0)
		{
			LOGWARN(g_Log, "C_NetMessageReliableOutBox::Add() - can't send reliable messages that contain no data.");
			return false; // no point sending this message as it looks invalid from the size.
		}

		// this defines the max size that each packet of reliable data could be minus the packet header.
		const U32 maxReliablePacketDataSize = 1024; 

		// how many packets will we have to send to transmit the whole message
		U32 numPacketsRequired = (baseMessageSize / maxReliablePacketDataSize);
		numPacketsRequired += (baseMessageSize % maxReliablePacketDataSize) ? 1 : 0;

		// what is the total size of the data we will need to send (message data + headers).
		const U32 totalOutBoxSizeRequired = baseMessageSize + (numPacketsRequired * sizeof(DatagramReliablePacketHeader));

		if (outBoxRemainingSize < totalOutBoxSizeRequired)
			return false; // can't send mesasge since therre is not enough room in the outbox.

		// open the outgoing message for reading.
		if(!msg.ReadOpen())
			return false;

		// if the outbox was empty then send this message on the next update. don't wait for the send timer. 
		if (0 == m_BytesInOutbox)
		{
			m_ForceSend = true;
			m_Timer.reset();
			m_Timer.start();
		}

		// this will be the total size of the message when its received in the remote players inbox.
		// since only the first packet will stored with a header.
		const U32 totalRemoteMsgSize = baseMessageSize + sizeof(DatagramReliablePacketHeader);

		U32 totalBytesToPack		= baseMessageSize;
		bool firstPacket			= true;

		do
		{
			// how many bytes from the message message will be sent in this packet
			const U32 bytesToPack = (totalBytesToPack > maxReliablePacketDataSize) ? maxReliablePacketDataSize : totalBytesToPack;
			totalBytesToPack -= bytesToPack;

			DatagramReliablePacketHeader hdr;

			hdr.m_EncryptedBytes		=	0;	// will be generated later.
			hdr.m_Reliable				=	1;			
			hdr.m_PartyChannel			=	partyMessage;		
			hdr.m_SenderReference		=	localSenderId;	
			hdr.m_PacketType			=	messageId;			
		
			hdr.m_InitialConnection		=	initialConnection;
			hdr.m_FirstPacketInStream	=	firstPacket;
			hdr.m_PacketSize			=	bytesToPack + sizeof(hdr);
			hdr.m_TotalMessageSize		=	totalRemoteMsgSize;	
			if (partyMessage)
				hdr.m_PacketSequence	=	(initialConnection == false) ? ++m_OutgoingPacketId_Party : 0;		
			else
				hdr.m_PacketSequence	=	(initialConnection == false) ? ++m_OutgoingPacketId_Race : 0;	

			// set the encrypted bytes size.
			bdUInt16 encSize = (sizeof(hdr) + bytesToPack) - sizeof(hdr.m_EncryptedBytes);
			EncryptedSize(hdr.m_EncryptedBytes, encSize);

			// write header to outbox.
			WriteInternal(&hdr, sizeof(hdr));

			// copy a packets worth of data from our outgoing message into the outbox
			const bool readOk = msg.ReadBuffer(&m_pData[m_BytesInOutbox], bytesToPack);
			assert(readOk);
			m_BytesInOutbox += bytesToPack;

			// first packet for this message has been added to out box now.
			firstPacket					=	false;

		}while(totalBytesToPack);

		// read has finished now.
		msg.ReadClose();

		return true;
	}

	void C_NetMessageReliableOutBox::ReceiveAcknowledgement(const DatagramAcknowledgementHeader& hdr)
	{
		if (!m_Initialised || 0 == m_BytesInOutbox)
			return;

		const DatagramReliablePacketHeader* pCurrentPacket = reinterpret_cast<DatagramReliablePacketHeader*> (m_pData);

		if (hdr.m_PacketSequence == pCurrentPacket->m_PacketSequence)
		{
			// remote peer has received our message so we should remove it from the outbox..
			if (pCurrentPacket->m_PacketSize == m_BytesInOutbox)
			{
				// only one packet in buffer so quick remove
				m_BytesInOutbox = 0; 
				m_Timer.reset();
			}
			else
			{
				// multiple packets in outbox so we have to jig memory..
				const S32 bytesToMove = m_BytesInOutbox - pCurrentPacket->m_PacketSize;
				
				// recalculate new outbox size before we loose the current packet.
				m_BytesInOutbox -= pCurrentPacket->m_PacketSize;

				memmove(m_pData, m_pData + pCurrentPacket->m_PacketSize, bytesToMove);
			}

			// once packet is removed, if we have anymore packets waiting to be sent then send then on the next update.
			if (m_BytesInOutbox > 0)
				m_ForceSend = true; 
		}
	}

	void C_NetMessageReliableOutBox::WriteInternal(void* pData, const U32 size)
	{
		assert(m_Initialised);
		assert(pData);
		assert(size);

		// sanity check to make sure we dont overrun the allocated buffer.
		const S32 remain (m_AllocatedDataSize - (m_BytesInOutbox + size));
		assert(remain >= 0);

		memcpy(&m_pData[m_BytesInOutbox], pData, size);
		m_BytesInOutbox += size;
	}
} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
