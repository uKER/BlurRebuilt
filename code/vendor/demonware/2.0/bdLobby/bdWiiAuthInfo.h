// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII UNIX
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE:

#ifndef BD_WII_AUTH_H
#define BD_WII_AUTH_H

class bdAuthWiiInfo
{
	public:
	
		bdBool serialize(void *buffer, const bdUInt bufferSize) const
		{
			bdUInt offset = 0;

			bdBool ok = bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_magicNumber);
			ok = ok && bdBytePacker::appendBuffer(buffer, bufferSize, offset, offset, m_username, BD_MAX_USERNAME_LENGTH);
			ok = ok && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, m_userID );

			return ok;
		};
		
		bdBool deserialize(const void *buffer, const bdUInt bufferSize)
		{
			bdUInt offset = 0;
			bdBool ok = true;
			ok = ok && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset,m_magicNumber);
			ok = ok && bdBytePacker::removeBuffer(buffer, bufferSize, offset, offset, m_username, BD_MAX_USERNAME_LENGTH);
			ok = ok && bdBytePacker::removeBasicType(buffer, bufferSize, offset, offset, m_userID);
			ok = ok && (m_magicNumber == BD_MAGIC_NUMBER);
			return ok;
		}

		bdUInt	m_magicNumber;
		// Username should be padded with nulls
		bdNChar8 m_username[BD_MAX_USERNAME_LENGTH];
		bdOnlineUserID m_userID;

};

// Currently contains;
//		magic number -> bdUInt32 . 4 bytes
//		userID -> bdOnlineUserID . 8 bytes
//		userName -> BD_MAX_USERNAME_LENGTH. 64 bytes
//		padding -> 4 bytes to make total a multiple of 8 .
#define BD_WII_AUTH_ACCOUNT_CYPHER_TEXT_SIZE (80u)


#endif //BD_WII_AUTH_H
