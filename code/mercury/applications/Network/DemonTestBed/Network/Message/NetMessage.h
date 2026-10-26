// ***************************************************************
//  NetMessage   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Class used for sending / receiving network messages.
// ***************************************************************

#ifndef NETWORK_MESSAGE_H
#define NETWORK_MESSAGE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace RacerNet
{

	/// Class for creating and receiving message data via the network.
	/** has a few interface classes to aid separate what functions are used for sending/receiving reliable and unreliable data.
	 */
	class C_NetMessage	
	{
	public:
		friend class C_NetRouterBase;
		friend class C_NetMessageReliableInBox;

		C_NetMessage(void);
		virtual ~C_NetMessage(void);

		typedef enum AccessMode
		{
			eAccessModeNone = 0,
			eAccessModeReading,
			eAccessModeWriting,
		};

		/// Allocate dynamic resources.
		/*	Only needs to be called once to setup the message buffers.
		 *	@param maxMessageSize what is the maximum number of bytes we will be sinding in a single message.
		 *	@return true if the buffers were allocated.
		 */
		bool Initialise(const U32 maxMessageSize);

		/// Destroy dynamic resources.
		/*	Kill the mesage buffers by removing any internal buffers. @see Initialise()
		 */
		void Destroy(void);

		/// Flush the contents of the message.
		/*	Any data inside the message buffers will be lost after a call to this.
		 */
		void Flush(void);

		/// Setup message headers and create a new mesage.
		/** @param messageType the Id of the message we will be sending.
		 *	@param senderReference the Id of the local player who will be sending this message.
		 *	@param partyMessage should be true if the message we are creating will be sent on the party channel.
		 *	@return true if the message was created or false if an error occurred.
		 */
		bool CreateUnreliable(const U8 messageType, const U8 senderReference, const bool partyMessage);
		
		/// Setup message for writing reliable data.
		/** This mesasge won't contain the headers required for sending, they will be added when the message is added to the remote
		*	player message queue for transmission.
		 *	@return true if the message was created or false if an error occurred.
		 */
		bool CreateReliable(void);

		/// Create an acknowledgement that we have received a specific reliable message from a peer.
		/**	Even though we have received a reliable message the acknowledgement is sent back unreliably.
		 *	@param senderReference this is the local players game reference so the peer knows who sent the message
		 *	@param packetSequenceAck the sequence number of the message that we are acknowleging.
		 *	@param partyMessage should be true if we are acknowleging a mesasge received on our party session. 
		 *	@return true if acknowledgement message was created successfully.
		 */
		bool CreateAcknowledgement(const U8 senderReference, const U16 packetSequenceAck, const bool partyMessage);
		
		/// Call this when all data has been written into the message.
		/** This MUST be called when all data this is required to be sent has been added to the message.
		 */
		void TerminateMessage(void);

		/// Start the unencrypted data section of the message.
		/** Any data written into this message after this call will be sent unencrypted.
		 *	This only applies to unreliable messages, we don't support sending unencrypted data in reliable messages.
		 *	@return true if it is valid to start writing unencrypted data.
		 */
		bool StartUnEncryptedData(void);

		/// Write a single U8 value into the message stream.
		/** @return true if data was written into the message.
		 */
		bool WriteU8(U8 in);

		/// Write a single S8 value into the message stream.
		/** @return true if data was written into the message.
		 */
		bool WriteS8(S8 in);

		/// Write a single U16 value into the message stream.
		/** @return true if data was written into the message.
		 */
		bool WriteU16(U16 in);

		/// Write a single S16 value into the message stream.
		/** @return true if data was written into the message.
		 */
		bool WriteS16(S16 in);

		/// Write a single U32 value into the message stream.
		/** @return true if data was written into the message.
		 */
		bool WriteU32(U32 in);

		/// Write a single S32 value into the message stream.
		/** @return true if data was written into the message.
		 */
		bool WriteS32(S32 in);

		/// Write a single U64 value into the message stream.
		/** @return true if data was written into the message.
		 */
		bool WriteU64(U64 in);

		/// Write a single S64 value into the message stream.
		/** @return true if data was written into the message.
		 */
		bool WriteS64(S64 in);

		/// Write a single F32 value into the message stream.
		/** @return true if data was written into the message.
		 */
		bool WriteF32(F32 in);

		/// Write a single F64 value into the message stream.
		/** @return true if data was written into the message.
		 */
		bool WriteF64(F64 in);

		/// Write a a stream of bytes into the message.
		/** @return true if data was written into the message.
		 */
		bool WriteBuffer(void* pData, U32 dataSize);

	
		/**************************************************
		* functions for reading from an incomming message * 
		**************************************************/

		/// Open the message before reading.
		/** This must be called before attempting to read any data out.
		 *	@param skipToUnEncrypted this should be left at false for reliable messages since thay should contain no un encrypted data
		 *	@return true if the message was opened successfully.
		 */
		bool ReadOpen(const bool skipToUnEncrypted = false);

		/// Closes the message once reading has finished.
		/** This can be usefull if you wish to re read some of the message by closing it and then re opening it. 
		 */
		bool ReadClose(void);

		/// Read a single U8 value from the message stream.
		/** @return true if data was read from the message..
		 */
		bool ReadU8(U8& out);

		/// Read a single S8 value from the message stream.
		/** @return true if data was read from the message..
		 */
		bool ReadS8(S8& out);

		/// Read a single U16 value from the message stream.
		/** @return true if data was read from the message..
		*/
		bool ReadU16(U16& out);

		/// Read a single S16 value from the message stream.
		/** @return true if data was read from the message..
		*/
		bool ReadS16(S16& out);

		/// Read a single U32 value from the message stream.
		/** @return true if data was read from the message..
		*/
		bool ReadU32(U32& out);

		/// Read a single S32 value from the message stream.
		/** @return true if data was read from the message..
		*/
		bool ReadS32(S32& out);

		/// Read a single U64 value from the message stream.
		/** @return true if data was read from the message..
		*/
		bool ReadU64(U64& out);

		/// Read a single S64 value from the message stream.
		/** @return true if data was read from the message..
		*/
		bool ReadS64(S64& out);

		/// Read a single F32 value from the message stream.
		/** @return true if data was read from the message..
		*/
		bool ReadF32(F32& out);

		/// Read a single F64 value from the message stream.
		/** @return true if data was read from the message..
		*/
		bool ReadF64(F64& out);

		/// Read a stream of bytes from the message.
		/** @return true if data was read from the message..
		*/
		bool ReadBuffer(void* pDataOut, U32 dataSize);

		/**************************************************
		* Misc functions								  * 
		**************************************************/

		/// How much data does the message contain.
		U32 DataSize(void) const {return m_CurrentBufferSize;}

		/// What is the maximum number of bytes this mesage can hold.
		U32 MaxDataSize(void) const {return m_MaxBufferSize;}

		/// Does the message stream contain a datagram header.
		bool HasHeader(void) const {return m_HasHeader;}

		/// Has the mesasge been closed after access.
		/** A message can only be sent when it has been closed after writing has finished.
		 * @see TerminateMessage()
		 */
		bool IsTerminated(void) const {return m_AccessMode == eAccessModeNone;}

		/// has this message been created as reliable.
		bool IsReliable(void) const {return m_Reliable;}

	private:
		C_NetMessage(const C_NetMessage& original);				// private copy constructor
		C_NetMessage& operator = (const C_NetMessage rhs);		// private uncoded assignment operator

		bool WriteToBuffer(void* pData, const U16 numBytes);
		bool EnoughSpaceToWrite(const U16 numBytes) const {return ((m_CurrentBufferSize+numBytes) > m_MaxBufferSize) ? false : true;}

		U8* Data(void) {return m_pData;}
	private:
		bool		m_Initialised;
		bool		m_Reliable;
		bool		m_HasHeader;
		bool		m_EncryptedBytesSet;
		U8			*m_pData;
		U16			m_MaxBufferSize;
		U16			m_CurrentBufferSize;
		U16			m_BytesReadOut;
		AccessMode	m_AccessMode;
	};

} // end namespace RacerNet

#endif // NETWORK_MESSAGE_H
