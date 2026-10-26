// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: UNIX OSX IPHONE WIN32 PSP-INFRA PS2 PS3 WII XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdLobbyCommon/bdAuthCommon.h>
#include <bdCore/bdCrypto/bdHashTiger192.h>

#define BD_LOG_LEVEL "bdCore/bdAuthUtility"

// This is the tiger192 hash of the magic serial key.
const bdUInt64 BD_MAGIC_LICENSE_ID = 0;

const bdUByte8 BD_MAGIC_LICENSE_KEY[BD_ENCRYPTION_KEY_SIZE] = {		0xDE,0xAD,0xBE,0xEF,
																	0xDE,0xAD,0xBE,0xEF,
																	0xDE,0xAD,0xBE,0xEF,
																	0xDE,0xAD,0xBE,0xEF};
// Displays in memory as 0xDEADBEEF
const bdUInt32 BD_MAGIC_NUMBER = 0xEFBDADDE;

bdUInt64 BD_CALL bdAuthUtility::getLicenseID(const bdNChar8* licenseCode)
{
	if(licenseCode == BD_NULL)
	{
		return BD_MAGIC_LICENSE_ID;
	}

	// The license ID is the first 8 bytes of the tiger hash of the licenseCode
	bdHashTiger192 tigerHash;
	bdUByte8 licenseHash[BD_TIGER_HASH_SIZE];
	bdUInt hashSize = BD_TIGER_HASH_SIZE;
	bdUInt licenseCodeLen;
	licenseCodeLen = static_cast<bdUInt>(bdStrlen(licenseCode));
	BD_ASSERT(tigerHash.hash(	reinterpret_cast<const bdUByte8*>(licenseCode),
		licenseCodeLen, licenseHash, hashSize	),
		"Hash function failed.");
	bdUInt64 licenseID = 0;
	// Need to extract licenseID with byte packer to ensure endian safeness, and no alignment problems.
	bdUInt offset = 0;
	bdBytePacker::removeBasicType(licenseHash, hashSize, 0, offset, licenseID);
	return licenseID;
}

void BD_CALL bdAuthUtility::getLicenseKey(const bdNChar8* licenseCode, 
										  bdByte8 licenseKey[BD_ENCRYPTION_KEY_SIZE])
{
	if(licenseCode == BD_NULL)
	{
		bdMemcpy(licenseKey, BD_MAGIC_LICENSE_KEY, BD_ENCRYPTION_KEY_SIZE);
	}
	else
	{
		// The license key is the last 16 bytes of the tiger hash of the licenseKey
		bdHashTiger192 tigerHash;
		bdUByte8 licenseHash[BD_TIGER_HASH_SIZE];
		bdUInt hashSize = BD_TIGER_HASH_SIZE;
		bdUInt licenseCodeLen;
		licenseCodeLen = static_cast<bdUInt>(bdStrlen(licenseCode));
		BD_ASSERT(tigerHash.hash(	reinterpret_cast<const bdUByte8*>(licenseCode),
			licenseCodeLen, licenseHash, hashSize	),
			"Hash function failed.");
		bdMemcpy(licenseKey, licenseHash, BD_ENCRYPTION_KEY_SIZE);
	}
}

void BD_CALL bdAuthUtility::getUserKey(const bdNChar8* password, bdByte8 userKey[BD_ENCRYPTION_KEY_SIZE])
{
	// The user key is the 1st 16 bytes of the tiger hash of the user's password.
	bdHashTiger192 tigerHash;
	bdUByte8 passwordHash[BD_TIGER_HASH_SIZE];
	bdUInt hashSize = BD_TIGER_HASH_SIZE;
	bdUInt passwordLen;
	passwordLen = static_cast<bdUInt>(bdStrlen(password));
	BD_ASSERT(tigerHash.hash(	reinterpret_cast<const bdUByte8*>(password),
		passwordLen, passwordHash, hashSize	),
		"Hash function failed.");
	bdMemcpy(userKey, passwordHash, BD_ENCRYPTION_KEY_SIZE);
}

bdOnlineUserID BD_CALL bdAuthUtility::getUserID(const bdNChar8* username)
{
	bdOnlineUserID userID = 0;
	
	// A buffer for a lowercase username.
	const bdUInt lwrUsernameBufferLength = BD_MAX_USERNAME_LENGTH+1;
	bdNChar8 lwrUsernameBuffer[lwrUsernameBufferLength];

	// Clear the lowercase buffer
	const bdUWord lwrUsernameBufferSize = sizeof(bdNChar8) * lwrUsernameBufferLength;
	bdMemset(lwrUsernameBuffer, BD_NULL, lwrUsernameBufferSize);

	// Copy in the username
	bdStrlcpy(lwrUsernameBuffer, username, lwrUsernameBufferLength);

	// Convert to lowercase
	bdStrlwr(lwrUsernameBuffer);

	// A buffer to store the hash
	const bdUInt hashBufferLength = BD_TIGER_HASH_SIZE;
	bdUByte8 hashBuffer[hashBufferLength];

	// Clear the hash buffer
	bdUInt hashBufferSize = sizeof(bdUByte8) * hashBufferLength;
	bdMemset(hashBuffer, BD_NULL, hashBufferSize);

	bdHashTiger192 tigerHash;

	// NOTE: only the characters in the username are hashed, and not any trailing NULLS.
	// Using the trailing NULLS may lead to a better hash, and would allow us to remove
	// this bdStrnlen, and use lwrUsernameBufferSize instead.
	const bdUInt lwrUsernameLength = bdStrnlen(lwrUsernameBuffer, lwrUsernameBufferLength);

	if(tigerHash.hash(reinterpret_cast<const bdUByte8*>(lwrUsernameBuffer),
					  lwrUsernameLength,
					  hashBuffer,
					  hashBufferSize))
	{
		// The user ID is the 1st 8 bytes of the tiger hash of the username.
		// Extract userID with byte packer to ensure endian safeness, and no alignment problems.
		bdUInt offset = 0;
		bdBytePacker::removeBasicType(hashBuffer, hashBufferSize, 0, offset, userID);
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL, "Hash failed.");
	}

	return userID;
}

void bdAuthCreateAccountPlainText::serialize(void *buffer, const bdUInt bufferSize) const
{
	bdUInt offset = 0;
	bdBool status = true;
	bdMemset(buffer, 0, bufferSize);
	status = status && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_licenseID);

	BD_ASSERT(status, "Failed to serialize.");
}

void bdAuthCreateAccountPlainText::deserialize(const void *buffer, const bdUInt bufferSize)
{
	bdUInt offset = 0;
	bdBool status = true;
	status = status && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_licenseID);

	BD_ASSERT(status, "Failed to deserialize.");
}

void bdAuthCreateAccountCypherText::serialize(void *buffer, const bdUInt bufferSize) const
{
	bdUInt offset = 0;
	bdBool status = true;
	bdMemset(buffer, 0, bufferSize);
	status = status && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_magicNumber);
	status = status && bdBytePacker::appendBuffer(buffer, bufferSize, offset, offset, m_username, BD_MAX_USERNAME_LENGTH);
	status = status && bdBytePacker::appendBuffer(buffer, bufferSize, offset, offset, m_userKey, BD_ENCRYPTION_KEY_SIZE);

	BD_ASSERT(status, "Failed to serialize.");
}

void bdAuthCreateAccountCypherText::deserialize(const void *buffer, const bdUInt bufferSize)
{
	bdUInt offset = 0;
	bdBool status = true;
	status = status && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_magicNumber);
	status = status && bdBytePacker::removeBuffer(buffer, bufferSize, offset, offset, m_username, BD_MAX_USERNAME_LENGTH);
	status = status && bdBytePacker::removeBuffer(buffer, bufferSize, offset, offset, m_userKey, BD_ENCRYPTION_KEY_SIZE);

	BD_ASSERT(status, "Failed to deserialize.");
}

void bdAuthChangeUserKeyPlainText::serialize(void *buffer, const bdUInt bufferSize) const
{
	bdUInt offset = 0;
	bdBool status = true;
	bdMemset(buffer, 0, bufferSize);
	status = status && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_userID);

	BD_ASSERT(status, "Failed to serialize.");
}

void bdAuthChangeUserKeyPlainText::deserialize(const void *buffer, const bdUInt bufferSize)
{
	bdUInt offset = 0;
	bdBool status = true;
	status = status && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_userID);

	BD_ASSERT(status, "Failed to deserialize.");
}

void bdAuthChangeUserKeyCypherText::serialize(void *buffer, const bdUInt bufferSize) const
{
	bdUInt offset = 0;
	bdBool status = true;
	bdMemset(buffer, 0, bufferSize);
	status = status && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_magicNumber);
	status = status && bdBytePacker::appendBuffer(buffer, bufferSize, offset, offset, m_userKey, BD_ENCRYPTION_KEY_SIZE); 

	BD_ASSERT(status, "Failed to serialize.");
}

void bdAuthChangeUserKeyCypherText::deserialize(const void *buffer, const bdUInt bufferSize)
{
	bdUInt offset = 0;
	bdBool status = true;
	status = status && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_magicNumber);
	status = status && bdBytePacker::removeBuffer(buffer, bufferSize, offset, offset, m_userKey, BD_ENCRYPTION_KEY_SIZE);

	BD_ASSERT(status, "Failed to deserialize.");
}

void bdAuthResetAccountPlainText::serialize(void *buffer, const bdUInt bufferSize) const
{
	bdUInt offset = 0;
	bdBool status = true;
	bdMemset(buffer, 0, bufferSize);
	status = status && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_userID);

	BD_ASSERT(status, "Failed to serialize.");
}

void bdAuthResetAccountPlainText::deserialize(const void *buffer, const bdUInt bufferSize)
{
	bdUInt offset = 0;
	bdBool status = true;
	status = status && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_userID);

	BD_ASSERT(status, "Failed to deserialize.");
}


void bdAuthResetAccountCypherText::serialize(void *buffer, const bdUInt bufferSize) const
{
	bdUInt offset = 0;
	bdBool status = true;
	bdMemset(buffer, 0, bufferSize);
	status = status && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_magicNumber);
	status = status && bdBytePacker::appendBuffer(buffer, bufferSize, offset, offset, m_userKey, BD_ENCRYPTION_KEY_SIZE);

	BD_ASSERT(status, "Failed to serialize.");
}

void bdAuthResetAccountCypherText::deserialize(const void *buffer, const bdUInt bufferSize)
{
	bdUInt offset = 0;
	bdBool status = true;
	status = status && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_magicNumber);
	status = status && bdBytePacker::removeBuffer(buffer, bufferSize, offset, offset, m_userKey, BD_ENCRYPTION_KEY_SIZE);

	BD_ASSERT(status, "Failed to deserialize.");
}

void bdAuthDeleteAccountPlainText::serialize(void *buffer, const bdUInt bufferSize) const
{
	bdUInt offset = 0;
	bdBool status = true;
	bdMemset(buffer, 0, bufferSize);
	status = status && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_userID);

	BD_ASSERT(status, "Failed to serialize.");
}

void bdAuthDeleteAccountPlainText::deserialize(const void *buffer, const bdUInt bufferSize)
{
	bdUInt offset = 0;
	bdBool status = true;
	status = status && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_userID);

	BD_ASSERT(status, "Failed to deserialize.");
}

void bdAuthDeleteAccountCypherText::serialize(void *buffer, const bdUInt bufferSize) const
{
	bdUInt offset = 0;
	bdBool status = true;
	bdMemset(buffer, 0, bufferSize);
	status = status && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_magicNumber);

	BD_ASSERT(status, "Failed to serialize.");
}

void bdAuthDeleteAccountCypherText::deserialize(const void *buffer, const bdUInt bufferSize)
{
	bdUInt offset = 0;
	bdBool status = true;
	status = status && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_magicNumber);

	BD_ASSERT(status, "Failed to deserialize.");
}


void bdAuthMigrateAccountPlainText::serialize(void *buffer, const bdUInt bufferSize) const
{
	bdUInt offset = 0;
	bdBool status = true;
	bdMemset(buffer, 0, bufferSize);
	status =  status && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_licenseID);

	BD_ASSERT(status, "Failed to serialize.");
}

void bdAuthMigrateAccountPlainText::deserialize(const void *buffer, const bdUInt bufferSize)
{
	bdUInt offset = 0;
	bdBool status = true;
	status = status && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_licenseID);

	BD_ASSERT(status, "Failed to deserialize.");
}

void bdAuthMigrateAccountCypherText::serialize(void *buffer, const bdUInt bufferSize) const
{
	bdUInt offset = 0;
	bdBool status = true;
	bdMemset(buffer, 0, bufferSize);
	status = status && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_magicNumber);
	status = status && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_oldTitleID);
	status = status && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_oldLicenseID);

	BD_ASSERT(status, "Failed to serialize.");
}

void bdAuthMigrateAccountCypherText::deserialize(const void *buffer, const bdUInt bufferSize)
{
	bdUInt offset = 0;
	bdBool status = true;
	status = status && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_magicNumber);
	status = status && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_oldTitleID);
	status = status && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_oldLicenseID);

	BD_ASSERT(status, "Failed to deserialize.");
}

void bdAuthUserToServicePlainText::serialize(void *buffer, const bdUInt bufferSize) const
{
	bdUInt offset = 0;
	bdBool status = true;
	bdMemset(buffer, 0, bufferSize);
	status = status && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_userID);

	BD_ASSERT(status, "Failed to serialize.");
}

void bdAuthUserToServicePlainText::deserialize(const void *buffer, const bdUInt bufferSize)
{
	bdUInt offset = 0;
	bdBool status = true;
	status = status && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_userID);

	BD_ASSERT(status, "Failed to deserialize.");
}

void bdAuthHostToServicePlainText::serialize(void *buffer, const bdUInt bufferSize) const
{
	bdUInt offset = 0;
	bdBool status = true;
	bdMemset(buffer, 0, bufferSize);
	status = status && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_licenseID);

	BD_ASSERT(status, "Failed to serialize.");
}

void bdAuthHostToServicePlainText::deserialize(const void *buffer, const bdUInt bufferSize)
{
	bdUInt offset = 0;
	bdBool status = true;
	status = status && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_licenseID);

	BD_ASSERT(status, "Failed to deserialize.");
}


void bdAuthUserToHostPlainText::serialize(void *buffer, const bdUInt bufferSize) const
{
	bdUInt offset = 0;
	bdBool status = true;
	bdMemset(buffer, 0, bufferSize);
	status = status && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_userID);
	status = status && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_hostLicenseID);

	BD_ASSERT(status, "Failed to serialize.");
}

void bdAuthUserToHostPlainText::deserialize(const void *buffer, const bdUInt bufferSize)
{
	bdUInt offset = 0;
	bdBool status = true;
	status = status && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_userID);
	status = status && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_hostLicenseID);

	BD_ASSERT(status, "Failed to deserialize.");
}

void bdAuthGetUsernamesForLicensePlainText::serialize(void *buffer, const bdUInt bufferSize) const
{
	bdUInt offset = 0;
	bdBool status = true;
	status = status && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_licenseID);

	BD_ASSERT(status, "Failed to serialize.");
}

void bdAuthGetUsernamesForLicensePlainText::deserialize(const void *buffer, const bdUInt bufferSize)
{
	bdUInt offset = 0;
	bdBool status = true;
	status = status && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_licenseID);

	BD_ASSERT(status, "Failed to deserialize.");
}
