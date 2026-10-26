// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE:

#ifndef BD_AUTH_TICKET_H
#define BD_AUTH_TICKET_H

#include <bdCore/bdCore.h>
#include <bdCore/bdCrypto/bdCryptoConfig.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>

#define BD_AUTH_TICKET_HASH_MAGIC_NUM_SIZE (3u)
#define BD_AUTH_TICKET_HASH_SIZE (4u)

/// An auth ticket constitutes the first part of an authentication
/// ticket and is decrypted by the client.
class bdAuthTicket
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		enum bdTicketTypes
		{
			BD_AUTH_USER_TO_SERVICE_TICKET,
			BD_AUTH_HOST_TO_SERVICE_TICKET,
			BD_AUTH_USER_TO_HOST_TICKET,
		};
		bdAuthTicket();
		~bdAuthTicket();

		void serialize(void *buffer);
		void deserialize(const void *buffer);
		bdUInt32 m_magicNumber;
		bdUByte8 m_type;
		bdTitleID m_titleID;
		bdUInt32 m_timeIssued;
		bdUInt32 m_timeExpires;
		bdUInt64 m_licenseID;
		bdOnlineUserID m_userID;
		bdNChar8 m_username[BD_MAX_USERNAME_LENGTH];
		bdUByte8 m_sessionKey[BD_ENCRYPTION_KEY_SIZE];

	private:
		// this is a backwards comptible addition
		// if m_usingHashMagicNumber contains a special string of bytes, m_hash
		// will contain a tiger hash of the rest of the ticket.
		// if m_usingHashMagicNumber contains anything else, the hash is to be ignored
		bdUByte8 m_usingHashMagicNumber[BD_AUTH_TICKET_HASH_MAGIC_NUM_SIZE];
		bdUByte8 m_hash[BD_AUTH_TICKET_HASH_SIZE];
};
#define BD_AUTH_TICKET_SIZE	128u

/// An auth info constitutes the second part of an authentication
/// ticket and is passed to the lobby server for decryption and verification.
class bdAuthInfo 
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		bdAuthInfo() 
		{
			m_titleID = 0;
			m_IVSeed = 0;
			bdMemset(m_data, 0, BD_AUTH_TICKET_SIZE);
			bdMemset(m_sessionKey, 0, BD_ENCRYPTION_KEY_SIZE);
		};

		~bdAuthInfo(){};

		bdTitleID m_titleID;
		bdUInt32  m_IVSeed;
		bdByte8 m_data[BD_AUTH_TICKET_SIZE];
		bdUByte8 m_sessionKey[BD_ENCRYPTION_KEY_SIZE];
};

#endif //BD_AUTH_TICKET_H
