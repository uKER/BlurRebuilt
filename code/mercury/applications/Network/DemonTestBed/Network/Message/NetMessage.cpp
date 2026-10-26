#include "StdAfx.h"
#include <Core/core.h>
#include "../Misc/NetDebug.h"
#include "../Message/NetMessage.h"
#include "../Misc/NetDatagramHeader.h"
#include "../Misc/NetDatagramPacketTypes.h"
#include <bdCore/bdCore.h>

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{

	C_NetMessage::C_NetMessage(void) :	m_Initialised(false), m_Reliable(false), m_HasHeader(false), m_EncryptedBytesSet(false),m_pData(NULL), m_MaxBufferSize(0), 
										m_CurrentBufferSize(0), m_BytesReadOut(0), m_AccessMode(eAccessModeNone)
	{
	}

	C_NetMessage::~C_NetMessage(void)
	{
		Destroy();
	}

	bool C_NetMessage::Initialise(const U32 maxMessageSize)
	{
		Destroy();

		const U32 minRequiredSize = sizeof(DatagramReliablePacketHeader) + sizeof(U32);

		if (0 == maxMessageSize || maxMessageSize < minRequiredSize)
			return false;

		m_MaxBufferSize = maxMessageSize;
		m_pData = new U8[m_MaxBufferSize];

		Flush();
		
		if (NULL == m_pData)
			Destroy();
		else
			m_Initialised = true;

		return m_Initialised;
	}

	void C_NetMessage::Destroy(void)	
	{
		if (NULL != m_pData)
			delete [] m_pData;
		m_pData = NULL;
		m_MaxBufferSize = 0;

		Flush();

		m_Initialised = false;
	}

	void C_NetMessage::Flush(void)
	{
		m_AccessMode = eAccessModeNone;
		m_CurrentBufferSize = 0;
		m_Reliable = false;
		m_HasHeader = false;
		m_EncryptedBytesSet = false;
		m_BytesReadOut = 0;
	}

	bool C_NetMessage::CreateUnreliable(const U8 messageType, const U8 senderReference, const bool partyMessage)
	{
		if (!m_Initialised)
			return false; // can't create message if buffers haven't been allocated.

		// kill any previous data in the buffer
		Flush();

		// set buffer up for writing..
		m_AccessMode = eAccessModeWriting;

		// remember that we are crating an unreliable message so we know we need to write the encrypted bytes when the user calls StartUnEncryptedData().
		m_Reliable = false;

		// remember that this message has the header in the data stream
		m_HasHeader = true;

		// we need to set this manually for unreliable message
		m_EncryptedBytesSet = false;

		// create header and add to buffer
		DatagramUnReliablePacketHeader hdr;
		hdr.m_EncryptedBytes	= 0;
		hdr.m_PacketType		= messageType;
		hdr.m_Reliable			= 0;
		hdr.m_PartyChannel		= partyMessage;
		hdr.m_SenderReference	= senderReference;

		if (WriteToBuffer(&hdr, sizeof(hdr)))
			return true;

		// can't create unreliable message
		Flush();
		return false;
	}

	bool C_NetMessage::StartUnEncryptedData(void)
	{
		if (!m_Initialised)
			return false;

		if (eAccessModeWriting != m_AccessMode)
			return false; // can only do this when in writing mode.

		if (m_EncryptedBytesSet)
			return true; // can only set encrypted bytes once..

		if (!m_Reliable && m_HasHeader)
		{
			if (m_CurrentBufferSize < sizeof(DatagramUnReliablePacketHeader))
				return false; 

			DatagramUnReliablePacketHeader* pHdr = reinterpret_cast<DatagramUnReliablePacketHeader*> (m_pData);

			//pHdr->m_EncryptedBytes = m_CurrentBufferSize - sizeof(pHdr->m_EncryptedBytes);

			// use demonware api to set the number of encrypted bytes.
			U32 offset = 0; 
			bdUInt16 encSize = m_CurrentBufferSize - sizeof(pHdr->m_EncryptedBytes);
			bdBytePacker::appendBasicType(pHdr, m_MaxBufferSize, offset, offset, encSize);

			m_EncryptedBytesSet = true;
		}

		return true;
	}

	bool C_NetMessage::CreateReliable(void)
	{
		if (!m_Initialised)
			return false; // can't create message if buffers haven't been allocated.

		// kill any previous data in the buffer
		Flush();

		// set buffer up for writing..
		m_AccessMode = eAccessModeWriting;

		// remember that we are crating a reliable message, no writing of encrypted bytes required.
		m_Reliable = true;

		// reliable headers are added when we add the message to a remote players outgoing queue
		m_HasHeader = false;

		// don't need to manually do this for reliable messages
		m_EncryptedBytesSet = true;

		return true;
	}

	bool C_NetMessage::CreateAcknowledgement(const U8 senderReference, const U16 packetSequenceAck, const bool partyMessage)
	{
		if (!m_Initialised)
			return false; // can't create message if buffers haven't been allocated.

		// kill any previous data in the buffer
		Flush();

		// acknowledgements are not send reliably.
		m_Reliable = false;

		// we have a header
		m_HasHeader = true;

		// will get done when the header is composed.
		m_EncryptedBytesSet = true;

		// no more writes allowed after packet header for acknowledgements.
		m_AccessMode = eAccessModeNone;

		// create header and add to buffer
		DatagramAcknowledgementHeader hdr;
		hdr.m_EncryptedBytes	= 0;
		hdr.m_PacketType		= eNetMessageTypeReliableAcknowlegement;
		hdr.m_Reliable			= 0;
		hdr.m_PartyChannel		= partyMessage;
		hdr.m_SenderReference	= senderReference;
		hdr.m_PacketSequence	= packetSequenceAck;

		if (WriteToBuffer(&hdr, sizeof(hdr)))
		{
			// encode encrypted size..
			U32 offset = 0; 
			bdUInt16 encSize = m_CurrentBufferSize - sizeof(hdr.m_EncryptedBytes);
			bdBytePacker::appendBasicType(m_pData, m_MaxBufferSize, offset, offset, encSize);

			return true;
		}

		Flush();
		return false;

	}

	void C_NetMessage::TerminateMessage(void)
	{
		if (!m_Initialised)
			return;

		if (eAccessModeWriting != m_AccessMode)
			return; // can only terminate messages that are currently being created.
		
		// do we need to forcefully set the number of encrypted bytes to send.. user should have done this manually if sending an unreliable message
		if (!m_EncryptedBytesSet)
		{
			if (!m_Reliable && m_HasHeader)
				StartUnEncryptedData();
		}

		m_AccessMode = eAccessModeNone;
	}

	bool C_NetMessage::WriteU8(U8 in)
	{
		if (!m_Initialised || eAccessModeWriting != m_AccessMode)
			return false;

		if (!EnoughSpaceToWrite(sizeof(U8)))
			return false;

		m_pData[m_CurrentBufferSize++] = in;
		return true;
	}

	bool C_NetMessage::WriteS8(S8 in)
	{
		if (!m_Initialised || eAccessModeWriting != m_AccessMode)
			return false;

		if (!EnoughSpaceToWrite(sizeof(S8)))
			return false;

		m_pData[m_CurrentBufferSize++] = in;
		return true;
	}

	bool C_NetMessage::WriteU16(U16 in)
	{
		if (!m_Initialised || eAccessModeWriting != m_AccessMode)
			return false;
		
		const U16 numBytes = sizeof(U16);

		if (!EnoughSpaceToWrite(numBytes))
			return false;

		U8* pIn = reinterpret_cast<U8*> (&in);
		for (U16 i=0; i<numBytes; i++)
			m_pData[m_CurrentBufferSize++] = pIn[i];
	
		return true;
	}

	bool C_NetMessage::WriteS16(S16 in)
	{
		if (!m_Initialised || eAccessModeWriting != m_AccessMode)
			return false;

		const U16 numBytes = sizeof(S16);

		if (!EnoughSpaceToWrite(numBytes))
			return false;

		U8* pIn = reinterpret_cast<U8*> (&in);
		for (U16 i=0; i<numBytes; i++)
			m_pData[m_CurrentBufferSize++] = pIn[i];

		return true;
	}

	bool C_NetMessage::WriteU32(U32 in)
	{
		if (!m_Initialised || eAccessModeWriting != m_AccessMode)
			return false;

		const U16 numBytes = sizeof(U32);

		if (!EnoughSpaceToWrite(numBytes))
			return false;

		U8* pIn = reinterpret_cast<U8*> (&in);
		for (U16 i=0; i<numBytes; i++)
			m_pData[m_CurrentBufferSize++] = pIn[i];

		return true;
	}

	bool C_NetMessage::WriteS32(S32 in)
	{
		if (!m_Initialised || eAccessModeWriting != m_AccessMode)
			return false;

		const U16 numBytes = sizeof(S32);

		if (!EnoughSpaceToWrite(numBytes))
			return false;

		U8* pIn = reinterpret_cast<U8*> (&in);
		for (U16 i=0; i<numBytes; i++)
			m_pData[m_CurrentBufferSize++] = pIn[i];

		return true;
	}

	bool C_NetMessage::WriteU64(U64 in)
	{
		if (!m_Initialised || eAccessModeWriting != m_AccessMode)
			return false;

		const U16 numBytes = sizeof(U64);

		if (!EnoughSpaceToWrite(numBytes))
			return false;

		U8* pIn = reinterpret_cast<U8*> (&in);
		for (U16 i=0; i<numBytes; i++)
			m_pData[m_CurrentBufferSize++] = pIn[i];

		return true;
	}

	bool C_NetMessage::WriteS64(S64 in)
	{
		if (!m_Initialised || eAccessModeWriting != m_AccessMode)
			return false;

		const U16 numBytes = sizeof(S64);

		if (!EnoughSpaceToWrite(numBytes))
			return false;

		U8* pIn = reinterpret_cast<U8*> (&in);
		for (U16 i=0; i<numBytes; i++)
			m_pData[m_CurrentBufferSize++] = pIn[i];

		return true;
	}

	bool C_NetMessage::WriteF32(F32 in)
	{
		if (!m_Initialised || eAccessModeWriting != m_AccessMode)
			return false;

		const U16 numBytes = sizeof(F32);

		if (!EnoughSpaceToWrite(numBytes))
			return false;

		U8* pIn = reinterpret_cast<U8*> (&in);
		for (U16 i=0; i<numBytes; i++)
			m_pData[m_CurrentBufferSize++] = pIn[i];

		return true;
	}

	bool C_NetMessage::WriteF64(F64 in)
	{
		if (!m_Initialised || eAccessModeWriting != m_AccessMode)
			return false;

		const U16 numBytes = sizeof(F64);

		if (!EnoughSpaceToWrite(numBytes))
			return false;

		U8* pIn = reinterpret_cast<U8*> (&in);
		for (U16 i=0; i<numBytes; i++)
			m_pData[m_CurrentBufferSize++] = pIn[i];

		return true;
	}

	bool C_NetMessage::WriteBuffer(void* pData, U32 dataSize)
	{
		if (!m_Initialised || eAccessModeWriting != m_AccessMode)
			return false;

		if (NULL == pData || 0 == dataSize)
			return false;

		return WriteToBuffer(pData, dataSize);
	}


	bool C_NetMessage::ReadOpen(const bool skipToUnEncrypted)
	{
		if (!m_Initialised)
			return false;

		if (0 == m_CurrentBufferSize)
			return false; // no data in the buffer so can't open for reading.
		
		if (eAccessModeReading == m_AccessMode)
			return true; // we are already reading..

		// read from start of buffer
		m_BytesReadOut = 0;

		// set correct mode.
		m_AccessMode = eAccessModeReading;

		// if data stream has a header then skip it.
		if (m_HasHeader)
		{
			if (m_Reliable)
			{
				const U16 hdrSize = sizeof(DatagramReliablePacketHeader);
				if (m_CurrentBufferSize < hdrSize)
					return false; // not enough data in buffer to skip header.. wtf

				// reliable messages should not have un encrypted data so this would be invalid.
				if (skipToUnEncrypted)
				{
					assert(false);
					ReadClose();
					return false;
				}

				m_BytesReadOut += hdrSize;
			}
			else
			{
				const U16 hdrSize = sizeof(DatagramUnReliablePacketHeader);
				if (m_CurrentBufferSize < hdrSize)
					return false; // not enough data in buffer to skip header.. wtf

				if (skipToUnEncrypted)
				{
					DatagramUnReliablePacketHeader* pHdr = reinterpret_cast<DatagramUnReliablePacketHeader*> (m_pData);
					
					bdUInt16 encryptedBytes = 0;
					U32 offset = 0; 

					if (bdBytePacker::removeBasicType(pHdr, m_CurrentBufferSize, offset, offset, encryptedBytes))
					{
						// add the 2 bytes since this is in the data buffer
						encryptedBytes += sizeof(pHdr->m_EncryptedBytes); 

						const S32 numUnEncrypted = m_CurrentBufferSize - encryptedBytes;
					
						if (numUnEncrypted > 0)
							m_BytesReadOut += encryptedBytes; // skip all encrypted data (including header) and jump straight to unencrypted data.
						else
						{
							// no unencrypted data to read out of the message.
							ReadClose();
							return false;
						}
					}
					else
					{
						// demonware function failed.
						ReadClose();
						return false;
					}

				}
				else
					m_BytesReadOut += hdrSize; // start reading straight after the header.
			}
		}

		return true;
	}


	bool C_NetMessage::ReadClose(void)
	{
		if (!m_Initialised)
			return false;

		if (eAccessModeReading != m_AccessMode)
			return false;

		// read from start of buffer
		m_BytesReadOut = 0;

		// set correct mode.
		m_AccessMode = eAccessModeNone;

		return true;
	}


	bool C_NetMessage::ReadU8(U8& out)
	{
		if (!m_Initialised || eAccessModeReading != m_AccessMode)
			return false;

		U16 numBytesToRead = sizeof(U8);
		if (m_BytesReadOut+numBytesToRead > m_CurrentBufferSize)
			return false; // not enough bytes left in our buffer to read out the requested data

		U8* pOut = reinterpret_cast<U8*> (&out);
		for (U16 i=0; i<numBytesToRead; i++)
			pOut[i] = m_pData[m_BytesReadOut++];
			
		return true;
	}

	bool C_NetMessage::ReadS8(S8& out)
	{
		if (!m_Initialised || eAccessModeReading != m_AccessMode)
			return false;

		U16 numBytesToRead = sizeof(S8);
		if (m_BytesReadOut+numBytesToRead > m_CurrentBufferSize)
			return false; // not enough bytes left in our buffer to read out the requested data

		U8* pOut = reinterpret_cast<U8*> (&out);
		for (U16 i=0; i<numBytesToRead; i++)
			pOut[i] = m_pData[m_BytesReadOut++];

		return true;
	}

	bool C_NetMessage::ReadU16(U16& out)
	{
		if (!m_Initialised || eAccessModeReading != m_AccessMode)
			return false;

		U16 numBytesToRead = sizeof(U16);
		if (m_BytesReadOut+numBytesToRead > m_CurrentBufferSize)
			return false; // not enough bytes left in our buffer to read out the requested data

		U8* pOut = reinterpret_cast<U8*> (&out);
		for (U16 i=0; i< numBytesToRead; i++)
			pOut[i] = m_pData[m_BytesReadOut++];

		return true;
	}

	bool C_NetMessage::ReadS16(S16& out)
	{
		if (!m_Initialised || eAccessModeReading != m_AccessMode)
			return false;

		U16 numBytesToRead = sizeof(S16);
		if (m_BytesReadOut+numBytesToRead > m_CurrentBufferSize)
			return false; // not enough bytes left in our buffer to read out the requested data

		U8* pOut = reinterpret_cast<U8*> (&out);
		for (U16 i=0; i< numBytesToRead; i++)
			pOut[i] = m_pData[m_BytesReadOut++];

		return true;
	}

	bool C_NetMessage::ReadU32(U32& out)
	{
		if (!m_Initialised || eAccessModeReading != m_AccessMode)
			return false;

		U16 numBytesToRead = sizeof(U32);
		if (m_BytesReadOut+numBytesToRead > m_CurrentBufferSize)
			return false; // not enough bytes left in our buffer to read out the requested data

		U8* pOut = reinterpret_cast<U8*> (&out);
		for (U16 i=0; i< numBytesToRead; i++)
			pOut[i] = m_pData[m_BytesReadOut++];

		return true;
	}

	bool C_NetMessage::ReadS32(S32& out)
	{
		if (!m_Initialised || eAccessModeReading != m_AccessMode)
			return false;

		U16 numBytesToRead = sizeof(S32);
		if (m_BytesReadOut+numBytesToRead > m_CurrentBufferSize)
			return false; // not enough bytes left in our buffer to read out the requested data

		U8* pOut = reinterpret_cast<U8*> (&out);
		for (U16 i=0; i< numBytesToRead; i++)
			pOut[i] = m_pData[m_BytesReadOut++];

		return true;
	}

	bool C_NetMessage::ReadU64(U64& out)
	{
		if (!m_Initialised || eAccessModeReading != m_AccessMode)
			return false;

		U16 numBytesToRead = sizeof(U64);
		if (m_BytesReadOut+numBytesToRead > m_CurrentBufferSize)
			return false; // not enough bytes left in our buffer to read out the requested data

		U8* pOut = reinterpret_cast<U8*> (&out);
		for (U16 i=0; i< numBytesToRead; i++)
			pOut[i] = m_pData[m_BytesReadOut++];

		return true;
	}

	bool C_NetMessage::ReadS64(S64& out)
	{
		if (!m_Initialised || eAccessModeReading != m_AccessMode)
			return false;

		U16 numBytesToRead = sizeof(S64);
		if (m_BytesReadOut+numBytesToRead > m_CurrentBufferSize)
			return false; // not enough bytes left in our buffer to read out the requested data

		U8* pOut = reinterpret_cast<U8*> (&out);
		for (U16 i=0; i< numBytesToRead; i++)
			pOut[i] = m_pData[m_BytesReadOut++];

		return true;
	}

	bool C_NetMessage::ReadF32(F32& out)
	{
		if (!m_Initialised || eAccessModeReading != m_AccessMode)
			return false;

		U16 numBytesToRead = sizeof(F32);
		if (m_BytesReadOut+numBytesToRead > m_CurrentBufferSize)
			return false; // not enough bytes left in our buffer to read out the requested data

		U8* pOut = reinterpret_cast<U8*> (&out);
		for (U16 i=0; i< numBytesToRead; i++)
			pOut[i] = m_pData[m_BytesReadOut++];

		return true;
	}

	bool C_NetMessage::ReadF64(F64& out)
	{
		if (!m_Initialised || eAccessModeReading != m_AccessMode)
			return false;

		U16 numBytesToRead = sizeof(F64);
		if (m_BytesReadOut+numBytesToRead > m_CurrentBufferSize)
			return false; // not enough bytes left in our buffer to read out the requested data

		U8* pOut = reinterpret_cast<U8*> (&out);
		for (U16 i=0; i< numBytesToRead; i++)
			pOut[i] = m_pData[m_BytesReadOut++];

		return true;
	}

	bool C_NetMessage::ReadBuffer(void* pData, U32 dataSize)
	{
		if (!m_Initialised || eAccessModeReading != m_AccessMode)
			return false;

		if (NULL == pData || 0 == dataSize)
			return false;

		if (m_BytesReadOut+dataSize > m_CurrentBufferSize)
			return false; // not enough bytes left in our buffer to read out the requested data

		memcpy(pData, &m_pData[m_BytesReadOut], dataSize);
		m_BytesReadOut += dataSize;
		return true;
	}

	bool C_NetMessage::WriteToBuffer(void* pData, const U16 numBytes)
	{
		// check we have enough space to write all requested data
		if (!EnoughSpaceToWrite(numBytes))
			return false; 

		memcpy(&m_pData[m_CurrentBufferSize], pData, numBytes);
		m_CurrentBufferSize += numBytes;

		return true;
	}
} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
