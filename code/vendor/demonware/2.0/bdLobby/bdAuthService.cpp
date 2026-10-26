// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: UNIX OSX IPHONE WIN32 PSP-INFRA PS2 PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdLobby/bdAuthService.h>
#include <bdLobby/bdAuthTicket.h>
#include <bdLobby/bdLobbyCommon/bdAuthCommon.h>
#include <bdLobby/bdLobbyConnection.h>
#include <bdCore/bdCrypto/bdRSAKey.h>

#define BD_LOG_LEVEL "auth service"

bdAuthService::bdAuthService(const bdTitleID titleID,
							 const bdAddr &authServiceAddr)
: m_errorCode(BD_NO_ERROR),
  m_sendBuffer(BD_NULL),
  m_sendBufSize(0),
  m_receiveBuffer(BD_NULL),
  m_recvBufSize(0),
  m_cdKeyTimeToLiveSecs(0)
{

	m_titleID = titleID;
	m_authServiceAddr = new bdCommonAddr(authServiceAddr);
	m_status = BD_READY;
}

bdAuthService::bdAuthService(const bdTitleID titleID,
							 const bdAddr &authServiceAddr,
							 void *sendBuffer,
							 bdUWord sendBufSize,
							 void *recvBuffer,
							 bdUWord recvBufSize)
: m_errorCode(BD_NO_ERROR),
  m_sendBuffer(sendBuffer),
  m_sendBufSize(sendBufSize),
  m_receiveBuffer(recvBuffer),
  m_recvBufSize(recvBufSize)
{
	m_titleID = titleID;
	m_authServiceAddr = new bdCommonAddr(authServiceAddr);
	m_status = BD_READY;
}

bdAuthService::~bdAuthService()
{
	m_authServiceAddr = BD_NULL;
	m_request = BD_NULL;
}

bdBool bdAuthService::createAccount(const bdNChar8* desiredUsername, 
									const bdNChar8* password, 
									const bdNChar8* licenseCode)
{
	if (m_status == BD_READY)
	{
		m_request =  makeCreateAccount(m_titleID, desiredUsername, password, licenseCode);
		startTask();
		return true;
	}
	return false;
}

bdBool bdAuthService::changePassword(	const bdNChar8* username,
										const bdNChar8* oldPassword,
										const bdNChar8* newPassword)
{
	if (m_status == BD_READY)
	{
		m_request =  makeChangeUserKey(m_titleID, username, oldPassword, newPassword);
		startTask();
		return true;
	}
	return false;
}

bdBool bdAuthService::resetAccount(const bdNChar8* username,
					 const bdNChar8* newPassword,
					 const bdNChar8* licenseCode)
{
	if (m_status == BD_READY)
	{
		m_request =  makeResetAccount(m_titleID, username, newPassword, licenseCode);
		startTask();
		return true;
	}
	return false;
}

bdBool bdAuthService::deleteAccount(	const bdNChar8* username,
										const bdNChar8* code)
{
	if (m_status == BD_READY)
	{
		m_request =  makeDeleteAccount(m_titleID, username, code);
		startTask();
		return true;
	}
	return false;
}

bdBool bdAuthService::migrateAccount(	const bdNChar8* licenseCode,
										const bdTitleID oldTitleID,
										const bdNChar8* oldLicenseCode)
{
	if (m_status == BD_READY)
	{
		m_request =  makeMigrateAccount(m_titleID, licenseCode, oldTitleID, oldLicenseCode );
		startTask();
		return true;
	}
	return false;
}

bdBool bdAuthService::authorizeAccount(	const bdNChar8* username,
										const bdNChar8* password)
{
	bdAuthUtility::getUserKey(password, m_userKey);
	if (m_status == BD_READY)
	{
		m_request = makeAuthAccountForService(m_titleID, username );
		startTask();
		return true;
	}
	return false;
}

bdBool bdAuthService::authorizeAnonymous()
{
	if (m_status == BD_READY)
	{
		m_request = makeAuthAnonymousForService(m_titleID);
		startTask();
		return true;
	}
	return false;
}

bdBool bdAuthService::getUsernamesForLicense( const bdNChar8* licenseCode)
{
	bdAuthUtility::getLicenseKey(licenseCode, m_licenseKey);
	if(m_status == BD_READY)
	{
		m_request = makeGetUsernamesForLicense(m_titleID, licenseCode);
		startTask();
		return true;
	}
	return false;
}

bdBool bdAuthService::authorizeDedicatedHost(	const bdNChar8* licenseCode)
{
	bdAuthUtility::getLicenseKey(licenseCode, m_licenseKey);
	if (m_status == BD_READY)
	{
		m_request =  makeAuthHostForService(m_titleID, licenseCode);
		startTask();
		return true;
	}
	return false;
}

bdBool bdAuthService::authorizeDedicatedServer()
{	
	if (m_status == BD_READY)
	{
		m_request =  makeAuthRequestForDedicatedServer(m_titleID);
		startTask();
		return true;
	}
	return false;
}

bdBool bdAuthService::authorizeDedicatedServerRSA()
{	
	if (m_status == BD_READY)
	{
		m_request =  makeAuthRequestForDedicatedServerRSA(m_titleID);
		startTask();
		return true;
	}
	return false;
}

bdBool bdAuthService::authorizeAccount(	const bdNChar8* username,
										const bdUInt64 hostLicenseID,
										const bdNChar8* password)
{
	bdAuthUtility::getUserKey(password, m_userKey);
	if (m_status == BD_READY)
	{
		m_request =  makeAuthAccountForHost(m_titleID, username, hostLicenseID);
		startTask();
		return true;
	}
	return false;
}

void bdAuthService::startTask()
{
	if (m_sendBufSize)
	{
		m_connection = new bdLobbyConnection(m_authServiceAddr,
											 m_sendBuffer,
											 m_sendBufSize,
											 m_receiveBuffer,
											 m_recvBufSize,
											 this);
	}
	else
	{
		m_connection = new bdLobbyConnection(m_authServiceAddr, this);
	}
	
	if (!m_connection->connect(m_authInfo))
	{
		m_errorCode = BD_START_TASK_FAILED;
	}
	else
	{
		m_status = BD_CONNECTING;
	}
}

bdAuthService::bdStatus bdAuthService::getStatus()
{
	// todo : should this check the status of the underlying bdStreamConnection also ?
	bdBitBufferRef message;
	bdUByte8 messageType  = 0;
	if (m_connection.notNull() && m_connection->getMessageToDispatch(messageType, message))
	{
		m_errorCode =  handleReply(messageType, message);
		m_status = BD_READY;
		m_connection->close();
		m_connection = BD_NULL;
	}
	return m_status;
}

bdLobbyErrorCode bdAuthService::getErrorCode() const
{
	return m_errorCode;
}

const bdAuthInfo* bdAuthService::getAuthInfo() const
{
	return &m_authInfo;
}

const bdAuthTicket* bdAuthService::getAuthTicket() const
{
	return &m_authTicket;
}

void bdAuthService::onConnect(bdLobbyConnectionRef connection)
{
	if( (connection ) && (m_request.notNull()))
	{
		// Send a ping as a first message. This is to obfuscate
		// the protocol and to bypass any DPI systems
		bdUInt32 ping = 0;
		m_connection->sendRaw(&ping, sizeof(ping));
		m_connection->send(m_request->getData(), m_request->getDataSize(), false);
		m_status = BD_PENDING_REPLY;
	}
	else if(m_request.isNull())
	{
		m_status = BD_READY;
		m_errorCode = BD_START_TASK_FAILED;
		BD_WARN(BD_LOG_LEVEL , "Received onConnect callback for NULL connection" );
	}
	else
	{
		m_status = BD_READY;
		m_errorCode = BD_CONNECTION_RESET;
		BD_WARN(BD_LOG_LEVEL , "Received onConnect callback for NULL connection" );
	}
}

void bdAuthService::onConnectFailed(bdLobbyConnectionRef)
{
	if(BD_CONNECTING == m_status)
	{
		m_status = BD_READY;
		m_errorCode = BD_CONNECTION_RESET;
		BD_INFO(BD_LOG_LEVEL , "Received onConnectFailed callback for connection" );
	}
	else
	{
		const bdUInt tempStatus = m_status;
		BD_ERR(BD_LOG_LEVEL , "Unexpected value for m_status : %u " , tempStatus);
	}
}

void bdAuthService::onDisconnect(bdLobbyConnectionRef)
{
	if (m_status != BD_READY)
	{
		m_status = BD_READY;
		m_errorCode = BD_CONNECTION_RESET;
		BD_INFO(BD_LOG_LEVEL , "Received onDisconnect callback  for connection" );
	}
}


bdBitBufferRef bdAuthService::makeCreateAccount(	const bdTitleID titleID,
												const bdNChar8* desiredUsername,
												const bdNChar8* password,
												const bdNChar8* licenseCode) const
{
	// Create the message
	bdUByte8 msgType = BD_AUTH_CREATE_ACCOUNT_REQUEST;
	bdBitBufferRef msg = new bdBitBuffer(&msgType,8, true);
	msg->setTypeCheck(false);
	msg->writeBool(true);
	msg->setTypeCheck(true);


	// Create the IV seed and initial vector
	bdUInt32 ivSeed = bdCryptoUtils::getNewIVSeed();
	bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
	bdCryptoUtils::calculateInitialVector(ivSeed, initialVector);
	// Write the seed to the message
	msg->writeUInt32(ivSeed);
	// Write the title ID to the message
	msg->writeUInt32(titleID);

	// Fill out the plain text part of the message
	bdAuthCreateAccountPlainText msgPlainText;
	msgPlainText.m_licenseID = bdAuthUtility::getLicenseID(licenseCode);
	const bdUInt msgPlainTextBufferSize = BD_AUTH_CREATE_ACCOUNT_PLAIN_TEXT_SIZE;
	bdUByte8 msgPlainTextBuffer[msgPlainTextBufferSize];
	msgPlainText.serialize(msgPlainTextBuffer, msgPlainTextBufferSize);

	// Fill out the cypher text part of the message
	bdAuthCreateAccountCypherText msgCypherText;
	msgCypherText.m_magicNumber = BD_MAGIC_NUMBER;
	bdMemset(msgCypherText.m_username, 0, BD_MAX_USERNAME_LENGTH);
	bdStrlcpy(msgCypherText.m_username, desiredUsername, BD_MAX_USERNAME_LENGTH);
	bdAuthUtility::getUserKey(password, msgCypherText.m_userKey);
	const bdUInt msgCypherTextBufferSize = BD_AUTH_CREATE_ACCOUNT_CYPHER_TEXT_SIZE;
	bdUByte8 msgCypherTextBuffer[msgCypherTextBufferSize];
	msgCypherText.serialize(msgCypherTextBuffer, msgCypherTextBufferSize);

	// Set up the buffer for the encryption
	bdUByte8 cypherText[BD_AUTH_CREATE_ACCOUNT_CYPHER_TEXT_SIZE];

	// Get the key to encrypt with
	bdByte8 licenseKey[BD_ENCRYPTION_KEY_SIZE];
	bdAuthUtility::getLicenseKey(licenseCode,licenseKey);

	// Encrypt the cypher text part of the message
	bdCryptoUtils::encrypt(licenseKey,initialVector,msgCypherTextBuffer,cypherText, BD_AUTH_CREATE_ACCOUNT_CYPHER_TEXT_SIZE);

	// Write the plaintext and cyphertext parts to the message
	msg->writeBits(msgPlainTextBuffer, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_CREATE_ACCOUNT_PLAIN_TEXT_SIZE));
	msg->writeBits(cypherText, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_CREATE_ACCOUNT_CYPHER_TEXT_SIZE));
	return msg;
}

bdBitBufferRef bdAuthService::makeGetUsernamesForLicense(const bdTitleID titleID,
														 const bdNChar8* licenseCode) const
{
	// Create the message
	bdUByte8 msgType = BD_AUTH_GET_USERNAMES_BY_LICENSE_REQUEST;
	bdBitBufferRef msg = new bdBitBuffer(&msgType, 8, true);
	msg->setTypeCheck(false);
	msg->writeBool(true);
	msg->setTypeCheck(true);

	bdUInt32 ivSeed = bdCryptoUtils::getNewIVSeed();
	bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
	bdCryptoUtils::calculateInitialVector(ivSeed, initialVector);
	// Write the seed to the message
	msg->writeUInt32(ivSeed);
	// Write the title ID to the message
	msg->writeUInt32(titleID);

	bdAuthGetUsernamesForLicensePlainText msgPlainText;
	msgPlainText.m_licenseID = bdAuthUtility::getLicenseID(licenseCode);
	const bdUInt msgPlainTextBufferSize = BD_AUTH_GET_USERNAMES_FOR_LICENSE_PLAIN_TEXT_SIZE;
	bdUByte8 msgPlainTextBuffer[msgPlainTextBufferSize];
	msgPlainText.serialize(msgPlainTextBuffer, msgPlainTextBufferSize);

	// Write the plaintext part to the message
	msg->writeBits(msgPlainTextBuffer, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_CREATE_ACCOUNT_PLAIN_TEXT_SIZE));
	return msg;
}

bdBitBufferRef bdAuthService::makeChangeUserKey(	const bdTitleID titleID,
																const bdNChar8* username,
																const bdNChar8* oldPassword,
																const bdNChar8* newPassword) const
{
	// Create the message
	bdUByte8 msgType = BD_AUTH_CHANGE_USER_KEY_REQUEST;
	bdBitBufferRef msg = new bdBitBuffer(&msgType,8, true);
	msg->setTypeCheck(false);
	msg->writeBool(true);
	msg->setTypeCheck(true);


	// Create the IV seed and initial vector
	bdUInt32 ivSeed = bdCryptoUtils::getNewIVSeed();
	bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
	bdCryptoUtils::calculateInitialVector(ivSeed, initialVector);
	// Write the seed to the message
	msg->writeUInt32(ivSeed);
	// Write the title ID to the message
	msg->writeUInt32(titleID);

	// Fill out the plaintext part of the message
	bdAuthChangeUserKeyPlainText msgPlainText;
	msgPlainText.m_userID = bdAuthUtility::getUserID(username);
	const bdUInt msgPlainTextBufferSize = BD_AUTH_CHANGE_USER_KEY_PLAIN_TEXT_SIZE;
	bdUByte8 msgPlainTextBuffer[msgPlainTextBufferSize];
	msgPlainText.serialize(msgPlainTextBuffer, msgPlainTextBufferSize);

	// Fill out the cypher text part of the message
	bdAuthChangeUserKeyCypherText msgCypherText;
	msgCypherText.m_magicNumber = BD_MAGIC_NUMBER;
	bdAuthUtility::getUserKey(newPassword, msgCypherText.m_userKey);
	const bdUInt msgCypherTextBufferSize = BD_AUTH_CHANGE_USER_KEY_CYPHER_TEXT_SIZE;
	bdUByte8 msgCypherTextBuffer[msgCypherTextBufferSize];
	msgCypherText.serialize(msgCypherTextBuffer, msgCypherTextBufferSize);

	// Set up the buffer for the encryption
	bdUByte8 cypherText[BD_AUTH_CHANGE_USER_KEY_CYPHER_TEXT_SIZE];

	// Get the key to encrypt with
	bdByte8 userKey[BD_ENCRYPTION_KEY_SIZE];
	bdAuthUtility::getUserKey(oldPassword, userKey);

	// Encrypt the cypher text part of the message
	bdCryptoUtils::encrypt(userKey,initialVector,msgCypherTextBuffer,cypherText,BD_AUTH_CHANGE_USER_KEY_CYPHER_TEXT_SIZE);

	// Write the plaintext and cyphertext parts to the message
	msg->writeBits(msgPlainTextBuffer, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_CHANGE_USER_KEY_PLAIN_TEXT_SIZE));
	msg->writeBits(cypherText, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_CHANGE_USER_KEY_CYPHER_TEXT_SIZE));
	return msg;
}

bdBitBufferRef bdAuthService::makeResetAccount(	const bdTitleID titleID,
																const bdNChar8* username,
																const bdNChar8* newPassword,
																const bdNChar8* licenseCode) const
{
	// Create the message
	bdUByte8 msgType = BD_AUTH_RESET_ACCOUNT_REQUEST;
	bdBitBufferRef msg = new bdBitBuffer(&msgType,8, true);
	msg->setTypeCheck(false);
	msg->writeBool(true);
	msg->setTypeCheck(true);


	// Create the IV seed and initial vector
	bdUInt32 ivSeed = bdCryptoUtils::getNewIVSeed();
	bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
	bdCryptoUtils::calculateInitialVector(ivSeed, initialVector);
	// Write the seed to the message
	msg->writeUInt32(ivSeed);
	// Write the title ID to the message
	msg->writeUInt32(titleID);

	// Fill out the plaintext part of the message
	bdAuthResetAccountPlainText msgPlainText;
	msgPlainText.m_userID = bdAuthUtility::getUserID(username);
	const bdUInt msgPlainTextBufferSize = BD_AUTH_RESET_ACCOUNT_PLAIN_TEXT_SIZE;
	bdUByte8 msgPlainTextBuffer[msgPlainTextBufferSize];
	msgPlainText.serialize(msgPlainTextBuffer, msgPlainTextBufferSize);

	// Fill out the cypher text part of the message
	bdAuthResetAccountCypherText msgCypherText;
	msgCypherText.m_magicNumber = BD_MAGIC_NUMBER;
	bdAuthUtility::getUserKey(newPassword, msgCypherText.m_userKey);
	const bdUInt msgCypherTextBufferSize = BD_AUTH_RESET_ACCOUNT_CYPHER_TEXT_SIZE;
	bdUByte8 msgCypherTextBuffer[msgCypherTextBufferSize];
	msgCypherText.serialize(msgCypherTextBuffer, msgCypherTextBufferSize);

	// Set up the buffer for the encryption
	bdUByte8 cypherText[BD_AUTH_RESET_ACCOUNT_CYPHER_TEXT_SIZE];

	// Get the key to encrypt with
	bdByte8 licenseKey[BD_ENCRYPTION_KEY_SIZE];
	bdAuthUtility::getLicenseKey(licenseCode,licenseKey);

	// Encrypt the cypher text part of the message
	bdCryptoUtils::encrypt(licenseKey,initialVector,msgCypherTextBuffer,cypherText, BD_AUTH_RESET_ACCOUNT_CYPHER_TEXT_SIZE);

	// Write the plaintext and cyphertext parts to the message
	msg->writeBits(msgPlainTextBuffer, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_RESET_ACCOUNT_PLAIN_TEXT_SIZE));
	msg->writeBits(cypherText, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_RESET_ACCOUNT_CYPHER_TEXT_SIZE));
	return msg;
}

bdBitBufferRef bdAuthService::makeDeleteAccount(	const bdTitleID titleID,
												const bdNChar8* username,
												const bdNChar8* code) const
{

	// Create the message
	bdUByte8 msgType = BD_AUTH_DELETE_ACCOUNT_REQUEST;
	bdBitBufferRef msg = new bdBitBuffer(&msgType,8, true);
	msg->setTypeCheck(false);
	msg->writeBool(true);
	msg->setTypeCheck(true);

	// Create the IV seed and initial vector
	bdUInt32 ivSeed = bdCryptoUtils::getNewIVSeed();
	bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
	bdCryptoUtils::calculateInitialVector(ivSeed, initialVector);
	// Write the seed to the message
	msg->writeUInt32(ivSeed);
	// Write the title ID to the message
	msg->writeUInt32(titleID);

	// Fill out the plaintext part of the message
	bdAuthDeleteAccountPlainText msgPlainText;
	msgPlainText.m_userID = bdAuthUtility::getUserID(username);
	const bdUInt msgPlainTextBufferSize = BD_AUTH_DELETE_ACCOUNT_PLAIN_TEXT_SIZE;
	bdUByte8 msgPlainTextBuffer[msgPlainTextBufferSize];
	msgPlainText.serialize(msgPlainTextBuffer, msgPlainTextBufferSize);

	// Fill out the cypher text part of the message
	bdAuthDeleteAccountCypherText  msgCypherText;
	msgCypherText.m_magicNumber = BD_MAGIC_NUMBER;
	const bdUInt msgCypherTextBufferSize = BD_AUTH_DELETE_ACCOUNT_CYPHER_TEXT_SIZE;
	bdUByte8 msgCypherTextBuffer[msgCypherTextBufferSize];
	msgCypherText.serialize(msgCypherTextBuffer, msgCypherTextBufferSize);

	// Set up the buffer for the encryption
	bdUByte8 cypherText[BD_AUTH_DELETE_ACCOUNT_CYPHER_TEXT_SIZE];

	// Get the key to encrypt with
	bdByte8 key[BD_ENCRYPTION_KEY_SIZE];
	// We use getLicenseKey here - it does exactly the same thing as getUserKey,
	// so it will work in both cases (that code == a password or a license code).
	bdAuthUtility::getLicenseKey(code,key);

	// Encrypt the cypher text part of the message
	bdCryptoUtils::encrypt(key,initialVector,msgCypherTextBuffer,cypherText, BD_AUTH_DELETE_ACCOUNT_CYPHER_TEXT_SIZE);

	// Write the plaintext and cyphertext parts to the message
	msg->writeBits(msgPlainTextBuffer, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_DELETE_ACCOUNT_PLAIN_TEXT_SIZE));
	msg->writeBits(cypherText, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_DELETE_ACCOUNT_CYPHER_TEXT_SIZE));
	return msg;
}

bdBitBufferRef bdAuthService::makeMigrateAccount(	const bdTitleID titleID,
																const bdNChar8* licenseCode,
																const bdTitleID oldTitleID,
																const bdNChar8* oldLicenseCode) const
{
	bdUByte8 msgType = BD_AUTH_MIGRATE_ACCOUNTS_REQUEST;
	bdBitBufferRef msg = new bdBitBuffer(&msgType,8, true);
	msg->setTypeCheck(false);
	msg->writeBool(true);
	msg->setTypeCheck(true);

	// Create the IV seed and initial vector
	bdUInt32 ivSeed = bdCryptoUtils::getNewIVSeed();
	bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
	bdCryptoUtils::calculateInitialVector(ivSeed, initialVector);
	// Write the seed to the message
	msg->writeUInt32(ivSeed);
	// Write the title ID to the message
	msg->writeUInt32(titleID);

	// Fill out the plaintext part of the message
	bdAuthMigrateAccountPlainText msgPlainText;
	msgPlainText.m_licenseID = bdAuthUtility::getLicenseID(licenseCode);
	const bdUInt msgPlainTextBufferSize = BD_AUTH_MIGRATE_ACCOUNT_PLAIN_TEXT_SIZE;
	bdUByte8 msgPlainTextBuffer[msgPlainTextBufferSize];
	msgPlainText.serialize(msgPlainTextBuffer, msgPlainTextBufferSize);

	// Fill out the cypher text part of the message
	bdAuthMigrateAccountCypherText  msgCypherText;
	msgCypherText.m_magicNumber = BD_MAGIC_NUMBER;
	msgCypherText.m_oldLicenseID = bdAuthUtility::getLicenseID(oldLicenseCode);
	msgCypherText.m_oldTitleID = oldTitleID;
	const bdUInt msgCypherTextBufferSize = BD_AUTH_MIGRATE_ACCOUNT_CYPHER_TEXT_SIZE;
	bdUByte8 msgCypherTextBuffer[msgCypherTextBufferSize];
	msgCypherText.serialize(msgCypherTextBuffer, msgCypherTextBufferSize);

	// Set up the buffer for the encryption
	bdUByte8 cypherText[BD_AUTH_MIGRATE_ACCOUNT_CYPHER_TEXT_SIZE];

	// Get the key to encrypt with
	bdByte8 licenseKey[BD_ENCRYPTION_KEY_SIZE];
	bdAuthUtility::getLicenseKey(licenseCode,licenseKey);

	// Encrypt the cypher text part of the message
	bdCryptoUtils::encrypt(licenseKey,initialVector,msgCypherTextBuffer,cypherText, BD_AUTH_MIGRATE_ACCOUNT_CYPHER_TEXT_SIZE);

	// Write the plaintext and cyphertext parts to the message
	msg->writeBits(msgPlainTextBuffer, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_MIGRATE_ACCOUNT_PLAIN_TEXT_SIZE));
	msg->writeBits(cypherText, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_MIGRATE_ACCOUNT_CYPHER_TEXT_SIZE));
	return msg;
}

bdBitBufferRef bdAuthService::makeAuthAccountForService(	const bdTitleID titleID,
														const bdNChar8* username) const
{
	// Create the message
	bdUByte8 msgType = BD_AUTH_ACCOUNT_FOR_MMP_REQUEST;
	bdBitBufferRef msg = new bdBitBuffer(&msgType,8, true);
	msg->setTypeCheck(false);
	msg->writeBool(true);
	msg->setTypeCheck(true);

	// Create the IV seed and initial vector
	bdUInt32 ivSeed = bdCryptoUtils::getNewIVSeed();
	bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
	bdCryptoUtils::calculateInitialVector(ivSeed, initialVector);
	// Write the seed to the message
	msg->writeUInt32(ivSeed);
	// Write the title ID to the message
	msg->writeUInt32(titleID);

	// Fill out the message text
	bdAuthUserToServicePlainText msgText;
	msgText.m_userID = bdAuthUtility::getUserID(username);
	const bdUInt msgTextBufferSize = BD_AUTH_USER_TO_SERVICE_PLAIN_TEXT_SIZE;
	bdUByte8 msgTextBuffer[msgTextBufferSize];
	msgText.serialize(msgTextBuffer, msgTextBufferSize);

	// Write it to the message buffer
	msg->writeBits(msgTextBuffer, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_USER_TO_SERVICE_PLAIN_TEXT_SIZE));
	return msg;
}

bdBitBufferRef bdAuthService::makeAuthAnonymousForService(const bdTitleID titleID) const
{
	// Create the message
	bdUByte8 msgType = BD_AUTH_ANONYMOUS_FOR_MMP_REQUEST;
	bdBitBufferRef msg = new bdBitBuffer(&msgType,8, true);
	msg->setTypeCheck(false);
	msg->writeBool(true);
	msg->setTypeCheck(true);

	// Create the IV seed and initial vector
	bdUInt32 ivSeed = bdCryptoUtils::getNewIVSeed();
	bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
	bdCryptoUtils::calculateInitialVector(ivSeed, initialVector);
	// Write the seed to the message
	msg->writeUInt32(ivSeed);
	// Write the title ID to the message
	msg->writeUInt32(titleID);

	return msg;
}

bdBitBufferRef bdAuthService::makeAuthHostForService(	const bdTitleID titleID,
													const bdNChar8* licenseCode) const
{
	// Create the message
	bdUByte8 msgType = BD_AUTH_HOST_FOR_MMP_REQUEST;
	bdBitBufferRef msg = new bdBitBuffer(&msgType,8, true);
	msg->setTypeCheck(false);
	msg->writeBool(true);
	msg->setTypeCheck(true);

    // Create the IV seed and initial vector
    bdUInt32 ivSeed = bdCryptoUtils::getNewIVSeed();
	bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
	bdCryptoUtils::calculateInitialVector(ivSeed, initialVector);
	// Write the seed to the message
	msg->writeUInt32(ivSeed);
	// Write the title ID to the message
	msg->writeUInt32(titleID);

	// Fill out the message text
	bdAuthHostToServicePlainText msgText;
	msgText.m_licenseID = bdAuthUtility::getLicenseID(licenseCode);
	const bdUInt msgTextBufferSize = BD_AUTH_HOST_TO_SERVICE_PLAIN_TEXT_SIZE;
	bdUByte8 msgTextBuffer[msgTextBufferSize];
	msgText.serialize(msgTextBuffer, msgTextBufferSize);

	// Write it to the message buffer
	msg->writeBits(msgTextBuffer, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_HOST_TO_SERVICE_PLAIN_TEXT_SIZE));
	return msg;
}

bdBitBufferRef bdAuthService::makeAuthRequestForDedicatedServer(const bdTitleID titleID) const
{
	// Create the message
	bdUByte8 msgType = BD_AUTH_FOR_DEDICATED_SERVER_REQUEST;
	bdBitBufferRef msg = new bdBitBuffer(&msgType,8, true);
	msg->setTypeCheck(false);
	msg->writeBool(true);
	msg->setTypeCheck(true);

    // Create the IV seed and initial vector
    bdUInt32 ivSeed = bdCryptoUtils::getNewIVSeed();
	bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
	bdCryptoUtils::calculateInitialVector(ivSeed, initialVector);
	// Write the seed to the message
	msg->writeUInt32(ivSeed);
	// Write the title ID to the message
	msg->writeUInt32(titleID);

	return msg;

}

bdBitBufferRef bdAuthService::makeAuthRequestForDedicatedServerRSA(const bdTitleID titleID)
{
	// Create the message
	bdUByte8 msgType = BD_AUTH_FOR_DEDICATED_SERVER_REQUEST_RSA;
	bdBitBufferRef msg = new bdBitBuffer(&msgType,8, true);
	msg->setTypeCheck(false);
	msg->writeBool(true);
	msg->setTypeCheck(true);

    // Create the IV seed and initial vector
    bdUInt32 ivSeed = bdCryptoUtils::getNewIVSeed();
	bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
	bdCryptoUtils::calculateInitialVector(ivSeed, initialVector);
	// Write the seed to the message
	msg->writeUInt32(ivSeed);
	// Write the title ID to the message
	msg->writeUInt32(titleID);

	//Generate RSA public/private pair and export the private key
	bdUByte8 rsaKeyBuf[140u];
	bdMemset(rsaKeyBuf, BD_NULL, sizeof(rsaKeyBuf));
	bdULong rsaKeySize = sizeof(rsaKeyBuf);
	m_RSAKey.init();
	m_RSAKey.exportKey(rsaKeyBuf, rsaKeySize);
	msg->writeBits(rsaKeyBuf, BD_NUM_BYTES_TO_NUM_BITS(140u));

	return msg;
}

bdBitBufferRef bdAuthService::makeAuthAccountForHost(	const bdTitleID titleID,
																	const bdNChar8* username,
																	const bdUInt64 hostLicenseID) const
{
	// Create the message
	bdUByte8 msgType = BD_AUTH_ACCOUNT_FOR_HOST_REQUEST;
	bdBitBufferRef msg = new bdBitBuffer(&msgType,8, true);
	msg->setTypeCheck(false);
	msg->writeBool(true);
	msg->setTypeCheck(true);

    // Create the IV seed and initial vector
    bdUInt32 ivSeed = bdCryptoUtils::getNewIVSeed();
	bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
	bdCryptoUtils::calculateInitialVector(ivSeed, initialVector);
	// Write the seed to the message
	msg->writeUInt32(ivSeed);
	// Write the title ID to the message
	msg->writeUInt32(titleID);

	// Fill out the message text
	bdAuthUserToHostPlainText msgText;
	msgText.m_userID = bdAuthUtility::getUserID(username);
	msgText.m_hostLicenseID = hostLicenseID;
	const bdUInt msgTextBufferSize = BD_AUTH_USER_TO_HOST_PLAIN_TEXT_SIZE;
	bdUByte8 msgTextBuffer[msgTextBufferSize];
	msgText.serialize(msgTextBuffer, msgTextBufferSize);

	// Write it to the message buffer
	msg->writeBits(msgTextBuffer, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_USER_TO_HOST_PLAIN_TEXT_SIZE));
	return msg;
}

bdLobbyErrorCode bdAuthService::handleReply(bdUByte8 replyType, bdBitBufferRef reply)
{
	// Read the error code
	bdUInt32 errorCode = 0;
	if(!reply->readUInt32(errorCode))
	{
		BD_ERR(BD_LOG_LEVEL, "Could not read error code from authentication server response.");
	}
	else if(errorCode == BD_AUTH_NO_ERROR)
	{
		// Request completed successfully
		switch(replyType)
		{
			case BD_AUTH_ACCOUNT_FOR_MMP_REPLY:
			{
				// Read the initial vector seed
				bdUInt32 IVSeed = 0;
				reply->readUInt32(IVSeed);

				// Calculate the initial vector from the seed
				bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
				bdCryptoUtils::calculateInitialVector(IVSeed, initialVector);

				// Set up the buffer to read the cypher text into
				bdUByte8 cypherText[BD_AUTH_TICKET_SIZE];

				// Read the cypher text
				if(!reply->readBits(cypherText, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_TICKET_SIZE)))
				{
					BD_WARN(BD_LOG_LEVEL,"Failed to read user ticket.\n");
					break;
				}
				bdUByte8 deCypherText[BD_AUTH_TICKET_SIZE];

				// Decrypt
				bdCryptoUtils::decrypt(m_userKey, initialVector, cypherText, deCypherText, BD_AUTH_TICKET_SIZE);

				// Deserialize the decrypted buffer into a sensible structure
				m_authTicket.deserialize(deCypherText);

				// THE REMAINER OF THE MESSAGE MUST BE SENT TO MATCHMAKING+
				// The IV must also be sent... and the TitleID
				if(m_authTicket.m_magicNumber == BD_MAGIC_NUMBER)
				{
					bdMemcpy(m_authInfo.m_sessionKey, m_authTicket.m_sessionKey, BD_ENCRYPTION_KEY_SIZE);
					reply->readBits(m_authInfo.m_data, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_TICKET_SIZE));
					m_authInfo.m_titleID = m_authTicket.m_titleID;
					m_authInfo.m_IVSeed = IVSeed;
				}
				else
				{
					return BD_AUTH_INCORRECT_PASSWORD;
				}
			}
			break;
			case BD_AUTH_ANONYMOUS_FOR_MMP_REPLY:
			{
				// Read the initial vector seed
				bdUInt32 IVSeed = 0;
				reply->readUInt32(IVSeed);

				// Calculate the initial vector from the seed
				bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
				bdCryptoUtils::calculateInitialVector(IVSeed, initialVector);

				// Set up the buffer to read the ticket data into
				bdUByte8 plainText[BD_AUTH_TICKET_SIZE];

				// Read the cypher text
				if(!reply->readBits(plainText, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_TICKET_SIZE)))
				{
					BD_WARN(BD_LOG_LEVEL,"Failed to read user ticket.\n");
					break;
				}
				// Deserialize the decrypted buffer into a sensible structure

				m_authTicket.deserialize(plainText);

				// THE REMAINER OF THE MESSAGE MUST BE SENT TO MATCHMAKING+
				// The IV must also be sent... and the TitleID
				if(m_authTicket.m_magicNumber == BD_MAGIC_NUMBER)
				{
					bdMemcpy(m_authInfo.m_sessionKey, m_authTicket.m_sessionKey, BD_ENCRYPTION_KEY_SIZE);
					reply->readBits(m_authInfo.m_data, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_TICKET_SIZE));
					m_authInfo.m_titleID = m_authTicket.m_titleID;
					m_authInfo.m_IVSeed = IVSeed;
				}
				else
				{
					return BD_AUTH_UNKNOWN_ERROR;
				}
			}
			break;
			case BD_AUTH_HOST_FOR_MMP_REPLY:
			{
				// Read the initial vector seed
				bdUInt32 IVSeed = 0;
				reply->readUInt32(IVSeed);

				// Calculate the initial vector from the seed
				bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
				bdCryptoUtils::calculateInitialVector(IVSeed, initialVector);

				// Set up the buffer to read the cypher text into
				bdUByte8 cypherText[BD_AUTH_TICKET_SIZE];

				// Read the cypher text
				if(!reply->readBits(cypherText, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_TICKET_SIZE)))
				{
					BD_WARN(BD_LOG_LEVEL,"Failed to read user ticket.\n");
					break;
				}
				// Set up the buffer to read the cypher text into
				bdUByte8 deCypherText[BD_AUTH_TICKET_SIZE];

				// Decrypt
				bdCryptoUtils::decrypt(m_licenseKey, initialVector, cypherText, deCypherText, BD_AUTH_TICKET_SIZE);

				// Deserialize the decrypted buffer into a sensible structure

				m_authTicket.deserialize(deCypherText);

				// THE REMAINER OF THE MESSAGE MUST BE SENT TO MATCHMAKING+
				// The IV must also be sent... and the TitleID
				if(m_authTicket.m_magicNumber == BD_MAGIC_NUMBER)
				{
					bdMemcpy(m_authInfo.m_sessionKey, m_authTicket.m_sessionKey, BD_ENCRYPTION_KEY_SIZE);
					reply->readBits(m_authInfo.m_data, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_TICKET_SIZE));
					m_authInfo.m_titleID = m_authTicket.m_titleID;
					m_authInfo.m_IVSeed = IVSeed;
				}
				else
				{
					return BD_AUTH_INCORRECT_LICENSE_CODE;
				}

			}
			break;
			case BD_AUTH_ACCOUNT_FOR_HOST_REPLY:
			{
				// Read the initial vector seed
				bdUInt32 IVSeed = 0;
				reply->readUInt32(IVSeed);

				// Calculate the initial vector from the seed
				bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
				bdCryptoUtils::calculateInitialVector(IVSeed, initialVector);

				bdUByte8 cypherText[BD_AUTH_TICKET_SIZE];

				// Read the cypher text
				if(!reply->readBits(cypherText, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_TICKET_SIZE)))
				{
					BD_WARN(BD_LOG_LEVEL, "Failed to read user ticket.\n");
					break;
				}

				// Decrypt
				bdCryptoUtils::decrypt(m_userKey, initialVector, cypherText, m_authInfo.m_data, BD_AUTH_TICKET_SIZE);
				m_authInfo.m_titleID = 0;
				m_authInfo.m_IVSeed = IVSeed;

			}
			break;
			case BD_AUTH_FOR_DEDICATED_SERVER_REPLY:
			{
				// Read the initial vector seed
				bdUInt32 IVSeed = 0;
				reply->readUInt32(IVSeed);

				// Calculate the initial vector from the seed
				bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
				bdCryptoUtils::calculateInitialVector(IVSeed, initialVector);

				// Set up the buffer to read the cypher text into
				bdUByte8 cypherText[BD_AUTH_TICKET_SIZE];

				// Read the cypher text
				if(!reply->readBits(cypherText, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_TICKET_SIZE)))
				{
					BD_WARN(BD_LOG_LEVEL,"Failed to read user ticket.\n");
					break;
				}				

				if(!reply->readBits(m_authInfo.m_data, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_TICKET_SIZE)))
				{
					BD_WARN(BD_LOG_LEVEL,"Failed to read LSG ticket.\n");
					break;
				}

				bdMemset(m_cdKey, BD_NULL, sizeof(m_cdKey));

				if(!reply->readBits(m_cdKey, BD_NUM_BYTES_TO_NUM_BITS(sizeof(m_cdKey))))
				{
					BD_WARN(BD_LOG_LEVEL,"Failed to read LSG ticket.\n");
					break;
				}
				//Bad define name, re-org into cd_key_buf_size?
				bdNChar8 xorkey[MAX_RSA_DATA_SIZE] = "43FCB2ACF2D72593DD7CD1C69E0F03C07229F4C83166F7B05BA0C5FE3AA3A2D93EK2495783KDKN92939DK";
				for (int i=0; i < MAX_RSA_DATA_SIZE; i++)
					m_cdKey[i] ^= xorkey[i];

				m_cdKeyTimeToLiveSecs = 0;

				if(!reply->readInt32(m_cdKeyTimeToLiveSecs))
				{
					BD_WARN(BD_LOG_LEVEL,"Failed to read LSG ticket.\n");
					break;
				}


				bdAuthUtility::getLicenseKey(m_cdKey, m_licenseKey);

				// Set up the buffer to read the cypher text into
				bdUByte8 deCypherText[BD_AUTH_TICKET_SIZE];

				// Decrypt
				bdCryptoUtils::decrypt(m_licenseKey, initialVector, cypherText, deCypherText, BD_AUTH_TICKET_SIZE);

				// Deserialize the decrypted buffer into a sensible structure
				m_authTicket.deserialize(deCypherText);
				
				// The IV must also be sent... and the TitleID
				if(m_authTicket.m_magicNumber == BD_MAGIC_NUMBER)
				{
					bdMemcpy(m_authInfo.m_sessionKey, m_authTicket.m_sessionKey, BD_ENCRYPTION_KEY_SIZE);					
					m_authInfo.m_titleID = m_authTicket.m_titleID;
					m_authInfo.m_IVSeed = IVSeed;
				}
				else
				{
					return BD_AUTH_INCORRECT_LICENSE_CODE;
				}
			}
			break;
			case BD_AUTH_FOR_DEDICATED_SERVER_REPLY_RSA:
			{
				// Read the initial vector seed
				bdUInt32 IVSeed = 0;
				reply->readUInt32(IVSeed);

				// Calculate the initial vector from the seed
				bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
				bdCryptoUtils::calculateInitialVector(IVSeed, initialVector);

				// Set up the buffer to read the cypher text into
				bdUByte8 cypherText[BD_AUTH_TICKET_SIZE];

				// Read the cypher text
				if(!reply->readBits(cypherText, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_TICKET_SIZE)))
				{
					BD_WARN(BD_LOG_LEVEL,"Failed to read user ticket.\n");
					break;
				}				

				if(!reply->readBits(m_authInfo.m_data, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_TICKET_SIZE)))
				{
					BD_WARN(BD_LOG_LEVEL,"Failed to read LSG ticket.\n");
					break;
				}

				//Read out the encrypted CD KEY
				/* now let’s decrypt the encrypted key */
				bdUByte8 encCDKey[128u];
				bdULong encCDKeySize = sizeof(encCDKey);

				if(!reply->readBits(encCDKey, BD_NUM_BYTES_TO_NUM_BITS(128u)))
				{
					BD_WARN(BD_LOG_LEVEL,"Failed to read Encrypted cd key.\n");
					break;
				}

				bdULong cdKeySize = sizeof(m_cdKey);
				//Wipe our cdkey to zeroes
				bdMemset(m_cdKey, BD_NULL, sizeof(m_cdKey));

				//Decrypt the cdkey
				m_RSAKey.decrypt(encCDKey, encCDKeySize, reinterpret_cast<bdUByte8*>(m_cdKey), cdKeySize);

				m_cdKeyTimeToLiveSecs = 0;

				if(!reply->readInt32(m_cdKeyTimeToLiveSecs))
				{
					BD_WARN(BD_LOG_LEVEL,"Failed to read LSG ticket.\n");
					break;
				}


				bdAuthUtility::getLicenseKey(m_cdKey, m_licenseKey);

				// Set up the buffer to read the cypher text into
				bdUByte8 deCypherText[BD_AUTH_TICKET_SIZE];

				// Decrypt
				bdCryptoUtils::decrypt(m_licenseKey, initialVector, cypherText, deCypherText, BD_AUTH_TICKET_SIZE);

				// Deserialize the decrypted buffer into a sensible structure
				m_authTicket.deserialize(deCypherText);
				
				// The IV must also be sent... and the TitleID
				if(m_authTicket.m_magicNumber == BD_MAGIC_NUMBER)
				{
					bdMemcpy(m_authInfo.m_sessionKey, m_authTicket.m_sessionKey, BD_ENCRYPTION_KEY_SIZE);					
					m_authInfo.m_titleID = m_authTicket.m_titleID;
					m_authInfo.m_IVSeed = IVSeed;
				}
				else
				{
					return BD_AUTH_INCORRECT_LICENSE_CODE;
				}
			}
			break;
#if defined BD_PLATFORM_PS3
			case BD_AUTH_PS3_FOR_MMP_REPLY:
			{

				const bdBool ok = handlePS3Reply(reply);
				if(!ok)
				{
					return BD_AUTH_UNKNOWN_ERROR;
				}
			}
			break;
#endif //BD_PLATFORM_PS3
			case BD_AUTH_GET_USERNAMES_BY_LICENSE_REPLY:
				{
					bdUInt32 resultCount = 0;

					m_usernamesForLicenseResult.clear();

					bdBool status = reply->readUInt32(resultCount);

					if(!status)
					{
						BD_WARN(BD_LOG_LEVEL, "Failed to read result count from reply");
						errorCode = BD_AUTH_UNKNOWN_ERROR; // TODO: Add specific (for read failures) error code.
					}
					else
					{
						m_usernamesForLicenseResult.ensureCapacity(resultCount);

						for(bdUInt i=0; i<resultCount && status; i++)
						{
							bdNChar8 username[BD_MAX_USERNAME_LENGTH];
							status = status && reply->readBits(username, BD_NUM_BYTES_TO_NUM_BITS(BD_MAX_USERNAME_LENGTH));
							m_usernamesForLicenseResult.pushBack(username);
						}

						if(!status)
						{
							BD_WARN(BD_LOG_LEVEL, "Failed to read %u usernames from reply", resultCount);
							errorCode = BD_AUTH_UNKNOWN_ERROR; // TODO: Add specific (for read failures) error code.
						}
					}
				}
				break;
#if defined BD_PLATFORM_WII
			case BD_AUTH_WII_FOR_MMP_REPLY:
			{
				// Read the initial vector seed
				bdUInt32 IVSeed = 0;
				reply->readUInt32(IVSeed);

				// read out the RSA sig
				bdByte8 rsaSignature [BD_RSA_SIGNATURE_LEN] ;

				if(!reply->readBits(rsaSignature, BD_NUM_BYTES_TO_NUM_BITS(BD_RSA_SIGNATURE_LEN )))
				{
					BD_WARN(BD_LOG_LEVEL,"Failed to read RSA Signature.\n");
					break;
				}

				// Calculate the initial vector from the seed
				bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
				bdCryptoUtils::calculateInitialVector(IVSeed, initialVector);

				// Set up the buffer to read the cypher text into
				bdUByte8 cypherText[BD_AUTH_TICKET_SIZE];

				// Read the cypher text
				if(!reply->readBits(cypherText, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_TICKET_SIZE)))
				{
					BD_WARN(BD_LOG_LEVEL,"Failed to read user ticket.\n");
					break;
				}
				bdUByte8 deCypherText[BD_AUTH_TICKET_SIZE];

				// check rsa signature of cypherText
				bdRSAKey t_rsaKey;
				bdNChar8* key = AUTH_SERVER_KEY;
				const bdBool importResult = t_rsaKey.import(key);
				if(true == importResult )
				{
					const bdBool verifyResult = t_rsaKey.verifyHash(&rsaSignature[0] , &cypherText[0] , BD_AUTH_TICKET_SIZE);
					if(false == verifyResult)
					{
						BD_ERR(BD_LOG_LEVEL,"RSA signature verification failed.");
						break;
					}
				}
				else
				{
					BD_ERR(BD_LOG_LEVEL,"Failed to import RSA key.");
					break;
				}

				// Decrypt
				bdCryptoUtils::decrypt(m_userKey, initialVector, cypherText, deCypherText, BD_AUTH_TICKET_SIZE);

				// Deserialize the decrypted buffer into a sensible structure
				m_authTicket.deserialize(deCypherText);

				// THE REMAINER OF THE MESSAGE MUST BE SENT TO MATCHMAKING+
				// The IV must also be sent... and the TitleID
				if(m_authTicket.m_magicNumber == BD_MAGIC_NUMBER)
				{
					bdMemcpy(m_authInfo.m_sessionKey, m_authTicket.m_sessionKey, BD_ENCRYPTION_KEY_SIZE);
					reply->readBits(m_authInfo.m_data, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_TICKET_SIZE));
					m_authInfo.m_titleID = m_authTicket.m_titleID;
					m_authInfo.m_IVSeed = IVSeed;
				}
				else
				{
					return BD_AUTH_INCORRECT_PASSWORD;
				}
			}
			break;
#endif //BD_PLATFORM_WII
			default:
				break;
		}

		BD_INFO(BD_LOG_LEVEL, "Task completed successfully\n");
	}
	else
	{
		BD_INFO(BD_LOG_LEVEL, "Task returned with error code %u\n", errorCode);
	}
	return static_cast<bdLobbyErrorCode>(errorCode);
}
