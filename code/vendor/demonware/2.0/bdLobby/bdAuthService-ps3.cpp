// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS3
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdAuthService.h>
#include <bdLobby/bdLobbyCommon/bdAuthCommon.h>
#include <bdLobby/bdAuthTicket.h>
#include <bdLobby/bdPS3AuthInfo.h>

const bdByte8 *bdAuthService::getPS3Cookie(bdUInt &size)
{
	size = BD_ENCRYPTION_KEY_SIZE;
	bdMemset(m_PS3CookieKey, BD_NULL, size);
	bdTrulyRandom::getInstance()->getRandomUByte8(reinterpret_cast<bdUByte8*>(m_PS3CookieKey), size);
	// XXX: Network platform cannot currently handle non-alphanumeric cookies.
	// need to translate the cookie into alphanumeric chars.
	// Once network platform is fixed, this should be fixed too (for greater
	// security).
	bdUInt i;
	for(i=0; i<size; i++)
	{
		// Translate each byte of the cookie into the alphanumeric ASCII range.
		// Note that the probability distribution after translation is biased,
		// because 62 is not a factor of 256, which decreases cookie entropy.
		bdByte8 cookieKey = m_PS3CookieKey[i];
		bdUByte8 absCookieKey = cookieKey > 0 ? cookieKey : bdUByte8(-cookieKey);
		bdUByte8 b = absCookieKey % 62;
		if(b < 26)
		{
			// translate into a lower-case alphabetic
			m_PS3CookieKey[i] = static_cast<bdByte8>('a' + b);
		}
		else if(b >= 26 && b < 52)
		{
			// translate into an upper-case alphabetic
			m_PS3CookieKey[i] = static_cast<bdByte8>('A' + (b - 26));
		}
		if(b >= 52 && b < 62)
		{
			// translate into a numeric
			m_PS3CookieKey[i] = static_cast<bdByte8>('0' + (b - 52));
		}
	}
	return m_PS3CookieKey;
}

bdBool bdAuthService::authorizePS3Ticket(	const bdUByte8 *ticket, 
											bdUInt ticketSize,
											bdPS3AuthInfoRef PS3Info)
{
	m_PS3AuthInfo = PS3Info;
	if (m_status == BD_READY)
	{
		m_request =  makeAuthForPS3(m_titleID, ticket, ticketSize);
		startTask();
		return true;
	}
	return false;
}

void bdAuthService::createPS3Cookie(bdByte8* cookieBuf)
{
	
	bdMemset(cookieBuf, BD_NULL, BD_ENCRYPTION_KEY_SIZE);
	bdTrulyRandom::getInstance()->getRandomUByte8(reinterpret_cast<bdUByte8*>(cookieBuf), BD_ENCRYPTION_KEY_SIZE);
	// XXX: Network platform cannot currently handle non-alphanumeric cookies.
	// need to translate the cookie into alphanumeric chars.
	// Once network platform is fixed, this should be fixed too (for greater
	// security).
	bdUInt i;
	for(i=0; i<BD_ENCRYPTION_KEY_SIZE; i++)
	{
		// Translate each byte of the cookie into the alphanumeric ASCII range.
		// Note that the probability distribution after translation is biased,
		// because 62 is not a factor of 256, which decreases cookie entropy.
		bdByte8 cookieKey = cookieBuf[i];
		bdUByte8 absCookieKey = cookieKey > 0 ? cookieKey : bdUByte8(-cookieKey);
		bdUByte8 b = absCookieKey % 62;
		if(b < 26)
		{
			// translate into a lower-case alphabetic
			cookieBuf[i] = static_cast<bdByte8>('a' + b);
		}
		else if(b >= 26 && b < 52)
		{
			// translate into an upper-case alphabetic
			cookieBuf[i] = static_cast<bdByte8>('A' + (b - 26));
		}
		if(b >= 52 && b < 62)
		{
			// translate into a numeric
			cookieBuf[i] = static_cast<bdByte8>('0' + (b - 52));
		}
	}
}

void bdAuthService::setPS3Cookie(bdByte8* cookie)
{
	bdUInt i;
	for(i=0; i<BD_ENCRYPTION_KEY_SIZE; i++)
	{
		m_PS3CookieKey[i] = cookie[i];
	}
}

bdBitBufferRef bdAuthService::makeAuthForPS3(const bdTitleID titleID, 
										   const bdUByte8 *ticket, 
										   bdUInt ticketSize) const
{
	bdUByte8 msgType = BD_AUTH_PS3_FOR_MMP_REQUEST;
	bdBitBufferRef msg = new bdBitBuffer(&msgType,8, true);
	msg->setTypeCheck(false);
	msg->writeBool(true);
	msg->setTypeCheck(true);

	bdUInt32 ivSeed = bdCryptoUtils::getNewIVSeed();
	msg->writeUInt32(ivSeed);

	msg->writeUInt32(titleID);

	msg->writeUInt32(ticketSize);
	
	msg->writeBits(ticket, BD_NUM_BYTES_TO_NUM_BITS(ticketSize));

	return msg;
}

bdBool bdAuthService::handlePS3Reply(bdBitBufferRef buffer)
{
	bdBool ok = true;

	// Read the initial vector seed
	bdUInt32 IVSeed = 0;
	ok = buffer->readUInt32(IVSeed);
	
	// Calculate the initial vector from the seed
	bdUByte8 initialVector[BD_TIGER_HASH_SIZE];
	bdCryptoUtils::calculateInitialVector(IVSeed, initialVector);
	
	// Set up the buffer to read the ticket data into
	const bdUInt cipherBufferSize = BD_NEXT_MULTIPLE_OF_M(BD_PS3_AUTH_INFO_SIZE + BD_AUTH_TICKET_SIZE, 8);
	bdUByte8 cipherBuffer[cipherBufferSize];

	// Read the DemonWare ticket
	ok = ok && buffer->readBits(cipherBuffer, BD_NUM_BYTES_TO_NUM_BITS(cipherBufferSize));

	bdCryptoUtils::decrypt(m_PS3CookieKey, initialVector, cipherBuffer, cipherBuffer, cipherBufferSize);
	// Deserialize the decrypted buffer into a sensible structure
	bdAuthTicket ticket;
	if(ok)
	{
		ticket.deserialize(cipherBuffer);
	}
	// Deserialize the decrypted buffer into a sensible structure
	if(ok && m_PS3AuthInfo.notNull())
	{
		m_PS3AuthInfo->deserialize(cipherBuffer + BD_AUTH_TICKET_SIZE, BD_PS3_AUTH_INFO_SIZE);
	}

	ok = ok && buffer->readBits(m_authInfo.m_data, BD_NUM_BYTES_TO_NUM_BITS(BD_AUTH_TICKET_SIZE));

	// THE REMAINER OF THE MESSAGE MUST BE SENT TO MATCHMAKING+
	// The IV must also be sent... and the TitleID
	if(ok && ticket.m_magicNumber == BD_MAGIC_NUMBER)
	{
		bdMemcpy(m_authInfo.m_sessionKey, ticket.m_sessionKey, BD_ENCRYPTION_KEY_SIZE);
		m_authInfo.m_titleID = ticket.m_titleID;
		m_authInfo.m_IVSeed = IVSeed;
	}

	return ok;
}

