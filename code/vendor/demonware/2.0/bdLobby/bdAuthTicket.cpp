// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdAuthTicket.h>
#include <bdCore/bdCrypto/bdHashTiger192.h>
#include <bdCore/bdCore.h>

bdAuthTicket::bdAuthTicket()
: m_magicNumber(0),
  m_type(0),
  m_titleID(0),
  m_timeIssued(0),
  m_timeExpires(0),
  m_licenseID(0),
  m_userID(0)
{
	bdMemset(m_username, 0, BD_MAX_USERNAME_LENGTH);
	bdMemset(m_sessionKey, 0, BD_ENCRYPTION_KEY_SIZE);
	m_usingHashMagicNumber[0] = 0x55;
	m_usingHashMagicNumber[1] = 0x33;
	m_usingHashMagicNumber[2] = 0x22;
	bdMemset(m_hash, 0, BD_AUTH_TICKET_HASH_SIZE);
}

bdAuthTicket::~bdAuthTicket()
{
}

void bdAuthTicket::serialize(void *buffer)
{
	// FIXME: Buffer size must be passed in
	const bdUInt bufferSize = BD_AUTH_TICKET_SIZE;

	bdUInt offset = 0;
	bdBool result = true;
	result = result && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_magicNumber);
	result = result && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_type);
	result = result && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_titleID);
	result = result && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_timeIssued);
	result = result && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_timeExpires);
	result = result && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_licenseID);
	result = result && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_userID);
	result = result && bdBytePacker::appendBuffer(buffer, bufferSize, offset, offset, m_username, BD_MAX_USERNAME_LENGTH);
	result = result && bdBytePacker::appendBuffer(buffer, bufferSize, offset, offset, m_sessionKey, BD_ENCRYPTION_KEY_SIZE);

	// now, encode the hash of the start of the ticket on the fly
	result = result && bdBytePacker::appendBuffer(buffer, bufferSize, offset, offset, m_usingHashMagicNumber, BD_AUTH_TICKET_HASH_MAGIC_NUM_SIZE);

	bdHashTiger192 tigerHash;
	bdUInt hashSize = 24;
	bdUByte8 fullHash[24];
	bdUInt bfrSize = BD_AUTH_TICKET_SIZE - (BD_AUTH_TICKET_HASH_SIZE + BD_AUTH_TICKET_HASH_MAGIC_NUM_SIZE);
	
	tigerHash.hash((bdUByte8*)buffer, bfrSize, fullHash, hashSize);
	
	bdMemcpy(m_hash, fullHash, BD_AUTH_TICKET_HASH_SIZE);
	result = result && bdBytePacker::appendBuffer(buffer, bufferSize, offset, offset, m_hash, BD_AUTH_TICKET_HASH_SIZE);

	BD_ASSERT(offset == BD_AUTH_TICKET_SIZE, "Encoded auth ticket was the wrong size.");
	BD_ASSERT(result, "Failed to serialize the ticket.");
}

void bdAuthTicket::deserialize(const void *buffer)
{
	// FIXME: Buffer size must be passed in.
	const bdUInt bufferSize = BD_AUTH_TICKET_SIZE;
	bdUInt offset = 0;
	bdBool result = true;

	result = result && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_magicNumber);
	result = result && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_type);
	result = result && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_titleID);
	result = result && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_timeIssued);
	result = result && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_timeExpires);
	result = result && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_licenseID);
	result = result && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_userID);
	result = result && bdBytePacker::removeBuffer(buffer, bufferSize, offset, offset, m_username, BD_MAX_USERNAME_LENGTH);
	result = result && bdBytePacker::removeBuffer(buffer, bufferSize, offset, offset,
		m_sessionKey, BD_ENCRYPTION_KEY_SIZE);
	result = result && bdBytePacker::removeBuffer(buffer, bufferSize, offset, offset, m_usingHashMagicNumber, BD_AUTH_TICKET_HASH_MAGIC_NUM_SIZE);
	result = result && bdBytePacker::removeBuffer(buffer, bufferSize, offset, offset, m_hash, BD_AUTH_TICKET_HASH_SIZE);

	BD_ASSERT(offset == BD_AUTH_TICKET_SIZE, "Decoded auth ticket was the wrong size.");
}
