// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdAuthService.h>
#include <bdLobby/bdLobbyCommon/bdAuthCommon.h>
#include <bdLobby/bdAuthTicket.h>
#include <bdConnection/bdMessage/bdMessage.h>
#include <bdCore/bdCrypto/bdHashTiger192.h>

#include <bdLobby/bdWiiAuthInfo.h>

bdBool bdAuthService::authorizeNASToken(const bdNChar8 *token,
										const bdMACAddr encryptionKey,
										const bdOnlineUserID userID,
										const bdNChar8* userName)
{
	if (m_status == BD_READY)
	{
		m_request =  makeAuthForWII(m_titleID, token , encryptionKey, userID, userName);
		startTask();
		return true;
	}
	return false;
}

bdBitBufferRef bdAuthService::makeAuthForWII(const bdTitleID titleID,
											 const bdNChar8 *token,
											 const bdMACAddr encryptionKey,
											 const bdOnlineUserID userID,
											 const bdNChar8* userName) const
{
	bdUByte8 msgType = BD_AUTH_WII_FOR_MMP_REQUEST;
	bdBitBufferRef msg = new bdBitBuffer(&msgType,8, true);
	msg->setTypeCheck(false);
	msg->writeBool(true);
	msg->setTypeCheck(true);

	// Some data gets written in plaintext.

	// Create the IV seed
	const bdUInt32 ivSeed = bdCryptoUtils::getNewIVSeed();
	// Write the seed to the message
	msg->writeUInt32(ivSeed);
	// Write the title ID to the message
	msg->writeUInt32(titleID);
	// Write the token to the message
	msg->writeString(token);


	// The rest is encrypted with m_userKey, which is a hash of the encryption key.

	bdAuthWiiInfo authInfo;
	authInfo.m_magicNumber = BD_MAGIC_NUMBER;
	authInfo.m_userID = userID;
	bdStrlcpy(authInfo.m_username , userName , BD_MAX_USERNAME_LENGTH);
	
	// Set up the buffer for the plaintext data we want to encrypt
	bdUByte8 msgCypherText[BD_WII_AUTH_ACCOUNT_CYPHER_TEXT_SIZE];
	bdMemset(msgCypherText, 0, BD_WII_AUTH_ACCOUNT_CYPHER_TEXT_SIZE);

	// seriali the auth info into the buffer
	authInfo.serialize(msgCypherText , BD_WII_AUTH_ACCOUNT_CYPHER_TEXT_SIZE);

	// Set up the buffer for the encryption
	bdUByte8 cypherText[BD_WII_AUTH_ACCOUNT_CYPHER_TEXT_SIZE];

	// Create initial vector
	bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
	bdCryptoUtils::calculateInitialVector(ivSeed, initialVector);

	bdHashTiger192 tiger;
	bdUInt tmpHashSize = BD_ENCRYPTION_KEY_SIZE;

	const bdBool hashResult = tiger.hash(encryptionKey.m_data, BD_MAC_ADDR_SIZE , (bdUByte8*)(m_userKey), tmpHashSize);

	if(hashResult == true )
	{
		// Encrypt the cypher text part of the message
		bdCryptoUtils::encrypt(m_userKey,initialVector,msgCypherText,cypherText, BD_WII_AUTH_ACCOUNT_CYPHER_TEXT_SIZE);

		// Write the cypher text part to the message
		msg->writeBits(cypherText, BD_NUM_BYTES_TO_NUM_BITS(BD_WII_AUTH_ACCOUNT_CYPHER_TEXT_SIZE));

	}
	else
	{
		// if the msg generation has failed then we will return null
		msg = BD_NULL;
	}

	return msg;
}




