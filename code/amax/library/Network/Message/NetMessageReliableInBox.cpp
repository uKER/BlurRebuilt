#include <stdafx.h>
#include <Core/Core.h>
#include <bdNet/bdNet.h>
#include <Network/Network.h>
#include "../Misc/NetDebug.h"
#include  "NetMessageReliableInBox.h"
#include "NetMessage.h"
#include "../Misc/NetDatagramHeader.h"
#include "../Router/NetRouterBase.h"


#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{
	C_NetMessageReliableInBox::C_NetMessageReliableInBox() :	m_Initialised(false), m_pData(NULL), m_AllocatedDataSize(0), m_BytesInInbox(0),
																m_LastProcessedPacketId_Party(0), m_LastProcessedPacketId_Race(0)
	{
	}

	C_NetMessageReliableInBox::~C_NetMessageReliableInBox()
	{
		Shutdown();
	}

	bool C_NetMessageReliableInBox::Initialise(const U32 requiredSize)
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

	void C_NetMessageReliableInBox::Shutdown(void)
	{
		Flush();
		if (m_pData)
			delete [] m_pData;
		m_pData = NULL;
		m_AllocatedDataSize = 0;
		m_Initialised = false;

		SetAvailable(true);
	}

	void C_NetMessageReliableInBox::Flush(void)
	{
		m_BytesInInbox = 0;
		m_LastProcessedPacketId_Party = 0;	
		m_LastProcessedPacketId_Race = 0;
	}

	bool C_NetMessageReliableInBox::Add(C_NetMessage& msg, bdAddrHandleRef& acknowledgeAddress, const U8 localSenderId, C_NetRouterBase& router)
	{
		if (!m_Initialised)
			return false;

		if (msg.DataSize() <= sizeof(DatagramReliablePacketHeader) || !msg.IsTerminated() || !msg.IsReliable() || !msg.HasHeader())
			return false;

		// store off as few bits of the header that we will use when sending the acknowledgement.
		const DatagramReliablePacketHeader* pHdr = reinterpret_cast<DatagramReliablePacketHeader*> (msg.m_pData);
		const bool partyMessage = pHdr->m_PartyChannel ? true : false;
		const U16 receivedPacketSequence = pHdr->m_PacketSequence; 

		// how many bytes are currently free in the in..
		const U32 inBoxRemainingSize = m_AllocatedDataSize - m_BytesInInbox;

		// see if the incomming message is new or if we have already received the sequence number before.
		const U16 expectedPacketId = partyMessage ? m_LastProcessedPacketId_Party+1 : m_LastProcessedPacketId_Race+1;
		if (pHdr->m_PacketSequence == expectedPacketId)
		{
			S32 sizeToCopy = msg.DataSize();

			// pointer to the start of the original data
			U8 *pNewData = msg.m_pData;

			// if this packet is not the first in the stream then we don't copy the header into the receive buffer.
			if (!pHdr->m_FirstPacketInStream)
			{
				pNewData = &(msg.m_pData[sizeof(DatagramReliablePacketHeader)]);
				sizeToCopy -= sizeof(DatagramReliablePacketHeader);
			}

			// make sure there is enough room in the inboc to actually store the required data.
			if ((m_BytesInInbox + sizeToCopy) > m_AllocatedDataSize)
			{
				LOGWARN(g_Log, "C_NetMessageReliableInBox::Add() - Unable to add reliable message to inbox, not enough space. %d bytes required : %d bytes free", sizeToCopy, m_AllocatedDataSize - m_BytesInInbox);
				return false; // not enough space in inbox to store this packet.
			}

			// copy data into inbox and set new size.
			memcpy(&m_pData[m_BytesInInbox], pNewData, sizeToCopy);
			m_BytesInInbox += sizeToCopy;

			// set correct packet sequence so we can receive next message.
			if (partyMessage)
				m_LastProcessedPacketId_Party = pHdr->m_PacketSequence;
			else
				m_LastProcessedPacketId_Race = pHdr->m_PacketSequence;
		}
	
		// send acknowledgement.
		// use the same message that was supplied to build the acknowledgement..
		msg.Flush();
		if (msg.CreateAcknowledgement(localSenderId, receivedPacketSequence, partyMessage))
			router.SendMessage(msg, acknowledgeAddress);
		else
		{
			// wtf..
			assert(false);  
		}

		msg.Flush();
		return true;
	}

	bool C_NetMessageReliableInBox::Poll(C_NetMessage& msg, U8& msgType, bool& partyChannel)
	{
		if (!m_Initialised)
			return false;
	
		if (0 == msg.MaxDataSize())
			return false; // supplied message has not been configured yet.

		if (m_BytesInInbox <= sizeof(DatagramReliablePacketHeader))
			return false; // not enough data in buffer to consider it being a complete message

		// clear any old message contents.
		msg.Flush();

		DatagramReliablePacketHeader* pBuffer = reinterpret_cast<DatagramReliablePacketHeader*> (m_pData);
		if (m_BytesInInbox >= pBuffer->m_TotalMessageSize)
		{
			// we have a complete message in the inbox, check size..
			if (msg.MaxDataSize() >= pBuffer->m_TotalMessageSize)
			{
				// copy message type from header.
				msgType = pBuffer->m_PacketType;

				// party or race message ?
				partyChannel = pBuffer->m_PartyChannel ? true : false;

				// copy data from inbox to actual outgoing message
				memcpy(msg.m_pData, pBuffer, pBuffer->m_TotalMessageSize);

				// configure msg correctly so it can be read.
				msg.m_CurrentBufferSize		= pBuffer->m_TotalMessageSize;	
				msg.m_HasHeader				= true;				
				msg.m_EncryptedBytesSet		= true;				
				msg.m_Reliable				= true;

				// remove current message from the inbox now that it has been copied.
				if (pBuffer->m_TotalMessageSize == m_BytesInInbox)
				{
					// only one message in buffer so quick remove
					m_BytesInInbox = 0; 
				}
				else
				{
					// multiple messages in the inbox so we have to jig memory..
					const S32 bytesToMove = m_BytesInInbox - pBuffer->m_TotalMessageSize;

					// recalculate new inbox size before we loose the current message.
					m_BytesInInbox -= pBuffer->m_TotalMessageSize;

					memmove(m_pData, m_pData + pBuffer->m_TotalMessageSize, bytesToMove);
				}

				// message should now be ready for reading by the game code.
				return true;
			}
		}

		return false;
	}

	void C_NetMessageReliableInBox::RemoveUnSentMessages(const bool partyMessages)
	{
		if (!m_Initialised)
			return;

		if (partyMessages)
			m_LastProcessedPacketId_Party = 0;
		else
			m_LastProcessedPacketId_Race = 0;

		if (0 == m_BytesInInbox)
			return;

		const U32 totalBytesToProcess = m_BytesInInbox;
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

				// recalculate new inbox size before we loose the current packet.
				m_BytesInInbox -= pCurrentPacket->m_PacketSize;

				// say that we have processed this amount of bytes..
				bytesProcessed += pCurrentPacket->m_PacketSize; 

				if (partyMessages)
				{
					LOGINFO(g_Log, "C_NetMessageReliableInBox::RemoveUnSentRaceMessages() - removing un sent party message type %d", pCurrentPacket->m_PacketType);
				}
				else
				{
					LOGINFO(g_Log, "C_NetMessageReliableInBox::RemoveUnSentRaceMessages() - removing un sent race message type %d", pCurrentPacket->m_PacketType);
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
	}
} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
