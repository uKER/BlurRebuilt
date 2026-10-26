// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdDTLS/bdPacket/bdDTLSData.h>
#include <bdCore/bdCrypto/bdHMacSHA1.h>
#include <bdCore/bdCrypto/bdHashTiger192.h>
#include <bdCore/bdCrypto/bdCypher.h>

#define BD_LOG_LEVEL "bdSocket/dtls"

bdDTLSData::bdDTLSData()
{
}

bdDTLSData::bdDTLSData(bdUInt16 vtag, bdUInt16 counter)
: bdDTLSHeader(BD_DTLS_DATA, vtag, counter)
{
}

bdDTLSData::~bdDTLSData()
{
}

bdBool bdDTLSData::serialize(void *outData,
							 const bdUInt outDataSize,
							 const bdUInt outDataOffset,
							 bdUInt &outDataNewOffset,
							 const bdSequenceNumber &lastSequenceNumber,
							 const bdUByte8 sharedKey[BD_SHARED_KEY_SIZE],
							 const bdUByte8 *const inData,
							 const bdUInt inDataSize,
							 bdCypher *const cypher)
{
	bdBool status = true;
	bdUByte8 iv[BD_SHARED_KEY_SIZE];
	bdUInt inDataOffset = 0;
	bdUInt16 encryptedDataSize = 0;

	status = status && bdBytePacker::removeBasicType(inData, inDataSize, inDataOffset, inDataOffset, encryptedDataSize);
	BD_ASSERT(status, "Packet format is invalid.");

	// Write out the data header (with invalid HMAC).
	status = status && serialize(outData, outDataSize, outDataOffset, outDataNewOffset);
	// Write out encrypted data size
	status = status && bdBytePacker::appendBasicType(outData, outDataSize,
		outDataNewOffset, outDataNewOffset, static_cast<bdUInt16>(encryptedDataSize));

	const bdUByte8 *const inPayloadStart = reinterpret_cast<const bdUByte8*>(inData) + inDataOffset;
	const bdUByte8 *const inPlainPayloadStart = inPayloadStart + encryptedDataSize;

	const bdUInt plainDataSize = (inDataSize - sizeof(encryptedDataSize)) - encryptedDataSize;
	const bdUInt encryptedPaddedDataSize = BD_NEXT_MULTIPLE_OF_M(encryptedDataSize, 8);
	const bdUInt encryptedPaddingSize = encryptedPaddedDataSize - encryptedDataSize;
	const bdUInt outFinalOffset = outDataNewOffset + encryptedPaddedDataSize + plainDataSize;

	bdUByte8 *const outPayloadStart = reinterpret_cast<bdUByte8*>(outData) + outDataNewOffset;
	bdUByte8 *const outPlainPayloadStart = outPayloadStart + encryptedPaddedDataSize;

	// This checks that in data format is valid.
	if (static_cast<bdUInt>(inPayloadStart - inData) > inDataSize ||
		static_cast<bdUInt>(inPlainPayloadStart - inData) > inDataSize ||
		sizeof(encryptedDataSize) + encryptedDataSize > inDataSize ||
		sizeof(encryptedDataSize) + encryptedDataSize + plainDataSize > inDataSize)
	{
		BD_WARN(BD_LOG_LEVEL, "Packet format is invalid.");
		outDataNewOffset = outDataOffset;
		return false;
	}

	if (!status ||
		(outDataNewOffset - outDataOffset) + encryptedPaddedDataSize + plainDataSize > outDataSize)
	{
		BD_WARN(BD_LOG_LEVEL, "Insufficient space in the destination buffer.");
		outDataNewOffset = outDataOffset;
		return false;
	}

	{
		// Calculate the IV
		bdHashTiger192 tigerHash;
		bdUInt sharedKeySize = BD_SHARED_KEY_SIZE;
	
		const bdInt32 seqNum = bdSequenceNumber(lastSequenceNumber, m_counter, sizeof(m_counter) << 3).getValue();
		const bdUInt seqNumSize = sizeof(seqNum);
		bdUByte8 seqNumData[seqNumSize];
		bdUInt tmpOffset = 0;
		BD_ASSERT(bdBytePacker::appendBasicType(seqNumData, seqNumSize, 0, tmpOffset, seqNum),
			"Failed to save seqnum");

		status = status && tigerHash.hash(seqNumData, seqNumSize, iv, sharedKeySize);
	}


	// Now copy the payload into its place.
	bdMemcpy(outPayloadStart, inPayloadStart, encryptedDataSize);
	bdMemset(outPayloadStart + encryptedDataSize, 1, encryptedPaddingSize); // FIXME: is this safe?
	bdMemcpy(outPlainPayloadStart, inPlainPayloadStart, plainDataSize);

	// Encrypt the payload
	bdUInt encryptBlockSize = encryptedPaddedDataSize;
	status = status && cypher->encrypt(iv,
									  outPayloadStart,
									  outPayloadStart,
									  encryptBlockSize);
	BD_ASSERT(encryptBlockSize == encryptedPaddedDataSize, "Encrypted into unexpected space.");

	{
		// Calculate HMAC
		bdHMacSHA1 hmac(sharedKey, BD_SHARED_KEY_SIZE);
		status = status && hmac.process(reinterpret_cast<const bdUByte8*>(outData), bdDTLSHeader::getSerializedSize());
		//status = status && hmac.process(outPayloadStart - sizeof(encryptedDataSize), sizeof(encryptedDataSize));
		status = status && hmac.process(outPayloadStart, encryptedPaddedDataSize + plainDataSize);

		bdUInt hmacResultSize = BD_DTLS_HMAC_SIZE;
		if (!hmac.getData(m_hmac, hmacResultSize))
		{
			BD_WARN(BD_LOG_LEVEL, "Failed to get HMAC!");
			outDataNewOffset = outDataOffset;
			return false;
		}
		BD_ASSERT(hmacResultSize == BD_DTLS_HMAC_SIZE, "Hmac produced unexpected amount of data.");
	}

	// Write the packet header again, this time with the correct HMAC
	status = status && serialize(outData, outDataSize, outDataOffset, outDataNewOffset);

	BD_ASSERT(reinterpret_cast<const bdUByte8*>(outData) + outDataNewOffset + sizeof(encryptedDataSize) == outPayloadStart,
		"BUG: Lost track of payload.");

	if (!status)
	{
		outDataNewOffset = outDataOffset;
	}
	else
	{
		outDataNewOffset = outFinalOffset;
	}

	return status;
}

bdBool bdDTLSData::deserialize(const void *inData,
							   const bdUInt inDataSize,
							   const bdUInt inDataOffset,
							   bdUInt &inDataNewOffset,
							   const bdSequenceNumber &lastSequenceNumber,
							   const bdUByte8 sharedKey[BD_SHARED_KEY_SIZE],
							   bdUByte8 *outData,
							   const bdUInt outDataMaxSize,
							   bdUInt &outDataSize,
							   bdCypher *const cypher)
{
	bdBool status = true;
	bdUByte8 iv[BD_SHARED_KEY_SIZE];
	bdUInt16 encryptedDataSize = 0;

	status = status && deserialize(inData, inDataSize, inDataOffset, inDataNewOffset);
	status = status && bdBytePacker::removeBasicType(inData, inDataSize, inDataNewOffset, inDataNewOffset, encryptedDataSize);

	const bdUInt paddedEncryptedDataSize = BD_NEXT_MULTIPLE_OF_M(encryptedDataSize, 8);
	//const bdUInt paddingDataSize = paddedEncryptedDataSize - encryptedDataSize;
	const bdUInt plainDataSize = (inDataSize - inDataNewOffset) - paddedEncryptedDataSize;

	const bdUByte8 *const inPayloadStart = reinterpret_cast<const bdUByte8*>(inData) + inDataNewOffset;
	const bdUByte8 *const inPlainPayloadStart = reinterpret_cast<const bdUByte8*>(inData) + inDataNewOffset + paddedEncryptedDataSize;

	bdUByte8 *const outPayloadStart = reinterpret_cast<bdUByte8*>(outData) + sizeof(encryptedDataSize);
	bdUByte8 *const outPlainPayloadStart = outPayloadStart + encryptedDataSize;
	outDataSize = sizeof(encryptedDataSize) + encryptedDataSize + plainDataSize;

	if (!status)
	{
		BD_WARN(BD_LOG_LEVEL, "Failed to deserialize header.");
		inDataNewOffset = inDataOffset;
		outDataSize = 0;
		return false;
	}	
	
	// Make sure there's enough data in the message
	if (inDataNewOffset + paddedEncryptedDataSize + plainDataSize > inDataSize)
	{
		BD_WARN(BD_LOG_LEVEL, "Truncated packet.");
		inDataNewOffset = inDataOffset;
		outDataSize = 0;
		return false;
	}

	// Make sure there's enough space in the result buffer
	// We need at most 8 bytes of extra space for decryption.
	if (outDataSize + 8 > outDataMaxSize)
	{
		BD_WARN(BD_LOG_LEVEL, "Insufficient space in the destination buffer.");
		inDataNewOffset = inDataOffset;
		outDataSize = 0;
		return false;
	}

	{
		// Calculate the iv
		bdHashTiger192 tigerHash;
		const bdUInt sharedKeySize = BD_SHARED_KEY_SIZE;
	
		const bdInt32 seqNum = bdSequenceNumber(lastSequenceNumber, m_counter, sizeof(m_counter) << 3).getValue();
		const bdUInt seqNumSize = sizeof(seqNum);
		bdUByte8 seqNumData[seqNumSize];
		bdUInt tmpOffset = 0;
		BD_ASSERT(bdBytePacker::appendBasicType(seqNumData, seqNumSize, 0, tmpOffset, seqNum),
			"Failed to save seqnum");
	
		bdUInt hashResult = sharedKeySize;
		status = status && tigerHash.hash(seqNumData, seqNumSize, iv, hashResult);
		BD_ASSERT(hashResult == sharedKeySize, "BUG: Hash produced unexpected amount of data.");
	}

	{
		// Check HMAC
		bdHMacSHA1 hmac(sharedKey, BD_SHARED_KEY_SIZE);
		status = status && hmac.process(reinterpret_cast<const bdUByte8*>(inData), bdDTLSHeader::getSerializedSize());
		//status = status && hmac.process(inPayloadStart - sizeof(encryptedDataSize), sizeof(encryptedDataSize));
		status = status && hmac.process(inPayloadStart, paddedEncryptedDataSize + plainDataSize);

		bdUInt tmpHmacSize = BD_DTLS_HMAC_SIZE;
		bdUByte8 tmpHmac[BD_DTLS_HMAC_SIZE];

		status = status && hmac.getData(tmpHmac, tmpHmacSize);
		BD_ASSERT(tmpHmacSize == BD_DTLS_HMAC_SIZE, "Hmac produced unexpected amount of data.");

		status = status && bdMemcmp(m_hmac, tmpHmac, tmpHmacSize) == 0;

		if (!status)
		{
			//Failure to decrypt packet is now expected behaviour due 
			//to testing multiple security IDs for valid data
			inDataNewOffset = inDataOffset;
			outDataSize = 0;
			return false;
		}
	}

	{
		// Ok, now decrypt.
		bdUInt encryptedBlockSize = paddedEncryptedDataSize;
		status = status && cypher->decrypt(iv,
										  inPayloadStart,
										  outPayloadStart,
										  encryptedBlockSize);
		BD_ASSERT(encryptedBlockSize == paddedEncryptedDataSize, "Decrypted into an unexpected data size.");
	}

	// Copy unencrypted data into its place.
	bdMemcpy(outPlainPayloadStart, inPlainPayloadStart, plainDataSize);
	// And specify encrypted part size.
	bdUInt tmpOffset = 0;
	status = status && bdBytePacker::appendBasicType(outData, outDataMaxSize, 0, tmpOffset, encryptedDataSize);

	if (!status)
	{
		BD_WARN(BD_LOG_LEVEL, "Decryption failed.");
		inDataNewOffset = inDataOffset;
		outDataSize = 0;
	}

	return status;
}

bdBool bdDTLSData::serialize(void *data,
							 const bdUInt size,
							 const bdUInt offset,
							 bdUInt &newOffset) const
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdDTLSHeader::serialize(data, size, newOffset, newOffset);
	status = status && bdBytePacker::appendBuffer(data, size, newOffset,
			newOffset, m_hmac, BD_DTLS_HMAC_SIZE);

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

bdBool bdDTLSData::deserialize(const void* data,
							   const bdUInt size,
							   const bdUInt offset,
							   bdUInt &newOffset)
{
	bdBool status = true;
	newOffset = offset;

	status = status && bdDTLSHeader::deserialize(data, size, newOffset, newOffset);
	status = status && bdBytePacker::removeBuffer(data, size, newOffset,
			newOffset, m_hmac, BD_DTLS_HMAC_SIZE);

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}


