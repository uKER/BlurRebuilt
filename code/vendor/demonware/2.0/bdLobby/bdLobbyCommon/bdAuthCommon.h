// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: UNIX OSX IPHONE WIN32 PSP-INFRA PS2 PS3 WII XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE: Contains packet definitions for communicating with auth service,
//			and a utility class for auth-related conversions.

#ifndef BD_AUTH_COMMON_H
#define BD_AUTH_COMMON_H

#include <bdCore/bdCore.h>
#include <bdCore/bdCrypto/bdCryptoUtils.h>
#include <bdCore/bdCrypto/bdCryptoConfig.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>

/// This value is used by titles in place of an actual license ID when they do
/// not implement license management.
extern const bdUInt64 BD_MAGIC_LICENSE_ID;
/// This value is used internally for titles that do not implement license
/// management. It is used in conjunction with BD_MAGIC_LICENSE_ID
extern const bdUByte8 BD_MAGIC_LICENSE_KEY[BD_ENCRYPTION_KEY_SIZE];
/// This value is used to verify successful decryption of data. It is use at the
/// start of every encrypted section of data.
extern const bdUInt32 BD_MAGIC_NUMBER;

/// The message types that can be sent or received to/from the authorization
/// service.
enum bdAuthMsgTypes
{
	BD_AUTH_CREATE_ACCOUNT_REQUEST,
	BD_AUTH_CREATE_ACCOUNT_REPLY,

	BD_AUTH_CHANGE_USER_KEY_REQUEST,
	BD_AUTH_CHANGE_USER_KEY_REPLY,

	BD_AUTH_RESET_ACCOUNT_REQUEST,
	BD_AUTH_RESET_ACCOUNT_REPLY,

	BD_AUTH_DELETE_ACCOUNT_REQUEST,
	BD_AUTH_DELETE_ACCOUNT_REPLY,

	BD_AUTH_MIGRATE_ACCOUNTS_REQUEST,
	BD_AUTH_MIGRATE_ACCOUNTS_REPLY,

	BD_AUTH_ACCOUNT_FOR_MMP_REQUEST,
	BD_AUTH_ACCOUNT_FOR_MMP_REPLY,

	BD_AUTH_HOST_FOR_MMP_REQUEST,
	BD_AUTH_HOST_FOR_MMP_REPLY,

	BD_AUTH_ACCOUNT_FOR_HOST_REQUEST,
	BD_AUTH_ACCOUNT_FOR_HOST_REPLY,

	BD_AUTH_ANONYMOUS_FOR_MMP_REQUEST,
	BD_AUTH_ANONYMOUS_FOR_MMP_REPLY,

	BD_AUTH_PS3_FOR_MMP_REQUEST,
	BD_AUTH_PS3_FOR_MMP_REPLY,

	BD_AUTH_GET_USERNAMES_BY_LICENSE_REQUEST,
    BD_AUTH_GET_USERNAMES_BY_LICENSE_REPLY,

	BD_AUTH_WII_FOR_MMP_REQUEST,
	BD_AUTH_WII_FOR_MMP_REPLY,

	BD_AUTH_FOR_DEDICATED_SERVER_REQUEST,
	BD_AUTH_FOR_DEDICATED_SERVER_REPLY,

	BD_AUTH_FOR_DEDICATED_SERVER_REQUEST_RSA,
	BD_AUTH_FOR_DEDICATED_SERVER_REPLY_RSA,

};

class bdAuthUtility
{
	public:
		/// Get the license ID for a particular license code. This is a non-blocking
		/// function.
		/// \note	The license ID is the first 8 bytes of the tiger hash of the
		///			license code string.
		/// \param	licenseCode[in]	A pointer to the license code string.
		/// \return	The license ID.
		static bdUInt64 BD_CALL getLicenseID(const bdNChar8* licenseCode);

		/// Get the license key (used for encryption) for a particular license
		/// code string. This is a non-blocking function.
		/// \note	The license key is the complete tiger hash of the license
		///			code string.
		/// \param	licenseCode[in]	The license code string.
		/// \param	licenseKey[out]	Pointer to an array where the license key should
		///							be filled in. This array should be of size
		///							BD_ENCRYPTION_KEY_SIZE.
		static void BD_CALL getLicenseKey(const bdNChar8* licenseCode,
										  bdByte8 licenseKey[BD_ENCRYPTION_KEY_SIZE]);

		/// Get the user key (used for encryption) from the given password.
		/// This is a non-blocking function. The user's key is derived from
		/// the user's password using tiger hash.
		/// \param	password[in]	The user's password.
		/// \param	userKey[out]	Pointer to an array to fill in with the
		///							corresponding user's key. This array should
		///							be of size BD_ENCRYPTION_KEY_SIZE.
		static void BD_CALL getUserKey(const bdNChar8* password, bdByte8 userKey[BD_ENCRYPTION_KEY_SIZE]);

		/// Get the user ID from a username. This is a non-blocking function.
		/// The user ID is the first 8 bytes of the tiger hash of the username,
		/// and is guaranteed to be unique for all players in a given title
		/// family (usernames are rejected during the account creation process
		/// if their IDs would collide). User ID collision is highly improbable.
		/// \param	username[in]	The username.
		/// \return	The bdOnlineUserID for the username.
		static bdOnlineUserID BD_CALL getUserID(const bdNChar8* username);
};

// Packet definitions

// CREATE ACCOUNT MESSAGE
class bdAuthCreateAccountPlainText
{
public:
	void serialize(void *buffer, const bdUInt bufferSize) const;
	void deserialize(const void *buffer, const bdUInt bufferSize);
	bdUInt64 m_licenseID;
};
#define BD_AUTH_CREATE_ACCOUNT_PLAIN_TEXT_SIZE	8u

// encrypted with the license key
class bdAuthCreateAccountCypherText
{
public:
	void serialize(void *buffer, const bdUInt bufferSize) const;
	void deserialize(const void *buffer, const bdUInt bufferSize);
	bdUInt32 m_magicNumber;
	// Username should be padded with nulls
	bdNChar8 m_username[BD_MAX_USERNAME_LENGTH];
	bdByte8 m_userKey[BD_ENCRYPTION_KEY_SIZE];

};
#define BD_AUTH_CREATE_ACCOUNT_CYPHER_TEXT_SIZE	96u

// CHANGE USER KEY MESSAGE
class bdAuthChangeUserKeyPlainText
{
public:
	void serialize(void *buffer, const bdUInt bufferSize) const;
	void deserialize(const void *buffer, const bdUInt bufferSize);
	bdOnlineUserID m_userID;
};
#define BD_AUTH_CHANGE_USER_KEY_PLAIN_TEXT_SIZE	8u

// Encrypted with old user key
class bdAuthChangeUserKeyCypherText
{
public:
	void serialize(void *buffer, const bdUInt bufferSize) const;
	void deserialize(const void *buffer, const bdUInt bufferSize);
	bdUInt32 m_magicNumber;
	bdByte8 m_userKey[BD_ENCRYPTION_KEY_SIZE];
};
#define BD_AUTH_CHANGE_USER_KEY_CYPHER_TEXT_SIZE	32u

// RESET ACCOUNT MESSAGE
class bdAuthResetAccountPlainText
{
public:
	void serialize(void *buffer, const bdUInt bufferSize) const;
	void deserialize(const void *buffer, const bdUInt bufferSize);
	bdOnlineUserID m_userID;
};
#define BD_AUTH_RESET_ACCOUNT_PLAIN_TEXT_SIZE	8u

// Encrypted with license key
class bdAuthResetAccountCypherText
{
public:
	void serialize(void *buffer, const bdUInt bufferSize) const;
	void deserialize(const void *buffer, const bdUInt bufferSize);
	bdUInt32 m_magicNumber;
	bdByte8 m_userKey[BD_ENCRYPTION_KEY_SIZE];
};
#define BD_AUTH_RESET_ACCOUNT_CYPHER_TEXT_SIZE	32u

// DELETE ACCOUNT MESSAGE
class bdAuthDeleteAccountPlainText
{
public:
	void serialize(void *buffer, const bdUInt bufferSize) const;
	void deserialize(const void *buffer, const bdUInt bufferSize);
	bdOnlineUserID m_userID;
};
#define BD_AUTH_DELETE_ACCOUNT_PLAIN_TEXT_SIZE	8u

// Encrypted with user key
class bdAuthDeleteAccountCypherText
{
public:
	void serialize(void *buffer, const bdUInt bufferSize) const;
	void deserialize(const void *buffer, const bdUInt bufferSize);
	bdUInt32 m_magicNumber;
};
#define BD_AUTH_DELETE_ACCOUNT_CYPHER_TEXT_SIZE	8u

// MIGRATE ACCOUNT MESSAGE
class bdAuthMigrateAccountPlainText
{
public:
	void serialize(void *buffer, const bdUInt bufferSize) const;
	void deserialize(const void *buffer, const bdUInt bufferSize);
	bdUInt64 m_licenseID;
};
#define BD_AUTH_MIGRATE_ACCOUNT_PLAIN_TEXT_SIZE	8u

class bdAuthMigrateAccountCypherText
{
public:
	void serialize(void *buffer, const bdUInt bufferSize) const;
	void deserialize(const void *buffer, const bdUInt bufferSize);
	bdUInt32 m_magicNumber;
	bdTitleID m_oldTitleID;
	bdUInt64 m_oldLicenseID;
};
#define BD_AUTH_MIGRATE_ACCOUNT_CYPHER_TEXT_SIZE	16u

// AUTHORIZE USER TO MM+ MESSAGE
class bdAuthUserToServicePlainText
{
public:
	void serialize(void *buffer, const bdUInt bufferSize) const;
	void deserialize(const void *buffer, const bdUInt bufferSize);
	bdOnlineUserID m_userID;
};
#define BD_AUTH_USER_TO_SERVICE_PLAIN_TEXT_SIZE	8u

// AUTHORIZE HOST TO MM+ MESSAGE (E.G. FOR DEDICATED SERVERS)
class bdAuthHostToServicePlainText
{
public:
	void serialize(void *buffer, const bdUInt bufferSize) const;
	void deserialize(const void *buffer, const bdUInt bufferSize);
	bdUInt64 m_licenseID;
};
#define BD_AUTH_HOST_TO_SERVICE_PLAIN_TEXT_SIZE	8u

// AUTHORIZE USER TO HOST MESSAGE (E.G. FOR A DEDICATED SERVER TO AUTHENTICATE A USER)
class bdAuthUserToHostPlainText
{
public:
	void serialize(void *buffer, const bdUInt bufferSize) const;
	void deserialize(const void *buffer, const bdUInt bufferSize);
	bdOnlineUserID m_userID;
	bdUInt64 m_hostLicenseID;
};
#define BD_AUTH_USER_TO_HOST_PLAIN_TEXT_SIZE	16u

// Get usernames for license id
class bdAuthGetUsernamesForLicensePlainText
{
public:
	void serialize(void *buffer, const bdUInt bufferSize) const;
	void deserialize(const void *buffer, const bdUInt bufferSize);
	bdUInt64 m_licenseID;
};
#define BD_AUTH_GET_USERNAMES_FOR_LICENSE_PLAIN_TEXT_SIZE 8u

#endif //BD_AUTH_COMMON_H
