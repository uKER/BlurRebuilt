// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class to represent a Message.

#ifndef BD_MESSAGE_H
#define BD_MESSAGE_H

#include <bdCore/bdCore.h>

#include <bdConnection/bdMessage/bdMessageTypes.h>

BD_REFERENCE(bdMessage);

/// A class to represent a Message. A message consists of a type
/// and a payload.
class bdMessage : public bdReferencable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Creates a Message of a particular type
		/// \param type[in] The Message type
		/// \param typeChecked[in]	A flag to indicate whether the encrypted 
		///							payload should contain additional type 
		///							information (Default = false).
		bdMessage(const bdMessageType type,
				  const bdBool typeChecked = false);

		/// Creates a message of a particular type and payload. The payload
		/// is copied, size indicates the number of bytes in the payload. This
		/// message also has an unencrypted payload.
		/// \param type[in]					The message type.
		/// \param payload[in]				A pointer to a byte array containing  
		///									the payload.
		/// \param size[in]					The number of bytes in the payload.
		/// \param hasTypeCheckedBit[in]	Specifies that the payload should be
		///									strongly typed. This is useful for 
		///									detecting serialization mismatches,
		///									but consumes extra bandwidth. 
		///									(Default = true)
		/// \param unencPayload[in]			A pointer to a byte array containing
		///									the unencrypted payload.
		/// \param unencSize[in]			The number of bytes in the unencrypted
		///									payload.
		bdMessage(const bdMessageType type, 
				  const bdUByte8 *const payload, 
				  const bdUInt32 size,
				  const bdBool hasTypeCheckedBit = true,
				  const bdUByte8 *const unencPayload = BD_NULL, 
				  const bdUInt32 unencSize = 0);

		/// Destructor.
		~bdMessage();
	
		/// Returns the type of the Message.
		/// \return The type.
		bdMessageType getType() const;
	
		/// Returns the payload if it exists. 
		/// If it doesn't exist the an instance will be created.
		/// \return The payload.
		bdBitBufferRef getPayload();

		/// True if this message has an encrypted payload.
		bdBool hasPayload() const;

		/// Create an unencrypted payload buffer of size \c size.
		/// If an unencrypted payload buffer already exists it will be lost. 
		/// The payload can be gotten from getUnencryptedPayload().
		void createUnencryptedPayload(const bdUInt size);

		/// Returns the unencrypted payload if it exists. 
		/// If it doesn't exist the value returned will be a smart pointer to BD_NULL.
		/// \return The unencrypted payload.
		bdByteBufferRef getUnencryptedPayload() const;

		/// True if this message has an unencrypted payload.
		bdBool hasUnencryptedPayload() const;

	protected:
		
		/// The message's type.
		bdMessageType m_type;

		/// The data/payload of this message.
		bdBitBufferRef m_payload;

		// Indicates if the encrypted payload is conatins type checking data.
		// Used to allow lazy creation of message payloads.
		bdBool m_payloadTypeChecked;
 
		/// The unencrypted data/payload of this message.
		bdByteBufferRef m_unencPayload;
};

#endif // BD_MESSAGE_H
