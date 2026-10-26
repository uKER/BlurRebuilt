// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: UNIX OSX IPHONE WIN32 PSP-INFRA PS2 PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE: The client interface to the authentication service.

#ifndef BD_AUTH_SERVICE_H
#define BD_AUTH_SERVICE_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyConnectionListener.h>

#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdAuthTicket.h>
#include <bdCore/bdCrypto/bdRSAKey.h>

#if defined BD_PLATFORM_PS3
BD_REFERENCE(bdPS3AuthInfo);
#include <bdLobby/bdPS3AuthInfo.h>
#elif defined BD_PLATFORM_WII
#include <bdCore/bdSocket/bdMACAddr.h>

// Public RSA key for server authentication
#define AUTH_SERVER_KEY "MIGJAoGBAMcjqY0IyB9yQOMgx1DAdWuO/pC7MHDtorXZZ83JE18BmG/4ozYURHawFpaIo4N+SefS5l08FnnI+S46b2aXIU7fe2m6Tgv4/VK+GXvXrg62AfNlNawka55ekCYQQLGNWKjp3rloG0cag8eGJWhgtl8ZSnt3y7dwA9IgescYybkBAgMBAAE="

#endif //BD_PLATFORM_PS3

/// This class provides a client interface to the authentication service.
/// The authentication service is responsible for account management functions,
/// in particular:
/// - account creation
/// - changing account password
/// - resetting an account's password
/// - deleting accounts
/// - migrating accounts between linked titles.
/// As well as account management functions, it also provides authorization
/// for accessing lobby services. Titles that implement user accounts must first
/// authorize users via this class before they can connect to lobby services.
/// License management is an integrated feature of the authorization
/// service. If licenses are implemented by the title, license code strings
/// must be supplied to the relevant functions. If license management is not
/// implemented by the title, the licenseCode parameter may be omitted in certain
/// functions, whilst other function are simply irrelevant and should not be
/// invoked.
/// This class can only process one request at a time. After issuing a request,
/// you should pump it to completion with bdAuthService::getStatus(). When the 
/// status returns to bdAuthService::BD_READY, the request has completed. When a
/// request completes, you should always check the error code with 
/// bdAuthService::getErrorCode(). The possible error codes are defined in 
/// bdLobbyErrorCodes.h. Only if the error code is  BD_AUTH_NO_ERROR can you 
/// safely proceed.
class bdAuthService : public bdLobbyConnectionListener
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Defines the various states of this class. If the state is BD_READY
		/// a new request may be made. Once a request is made, the state will
		/// transition to BD_CONNECTING and then to BD_PENDING_REPLY. When the
		/// state returns to BD_READY, the error code can be checked.
		enum bdStatus
		{
			BD_READY,
			BD_CONNECTING,
			BD_PENDING_REPLY
		};

		/// Constructor.
		/// \param	titleID[in]			The title ID allocated for this title.
		/// \param	authServiceAddr[in]	The address of the authorization service.
		///								This address should be constructed with the
		///								DNS name provided by DemonWare support.
		bdAuthService(	const bdTitleID titleID,
						const bdAddr &authServiceAddr);

		/// \param titleID[in]			The title ID allocated for this title.
		/// \param authServiceAddr[in]	The address of the authorization service.
		/// \param sendBuffer[in]		A pointer to a chunk of memory to be used 
		///								as send buffer.
		/// \param sendBufSize[in]		size of the send buffer
		/// \param recvBuffer[in]		A pointer to a chunk of memory to be used 
		///								as receive buffer.
		/// \param recvBufSize[in]		Size of the receive buffer.
		bdAuthService(	const bdTitleID titleID,
						const bdAddr &authServiceAddr,
						void *sendBuffer,
						bdUWord sendBufSize,
						void *recvBuffer, 
						bdUWord recvBufSize);
		
		/// Destructor
		~bdAuthService();
			
		/// Create a new user account.
		/// \param	desiredUsername[in]	The desired name for this account.
		/// \param	password[in]		The password to use for this account.
		/// \param	licenseCode[in]		The license code string that this account
		///								should be created with. If the title
		///								does not support licenses, this parameter
		///								may be omitted, or specified as BD_NULL.
		/// \return
		/// - True, if the request was issued successfully.
		/// - False, if the request could not be made (e.g., status is not BD_READY).
		bdBool createAccount(	const bdNChar8* desiredUsername,
								const bdNChar8* password,
								const bdNChar8* licenseCode = BD_NULL);

		/// Change the password associated with an account.
		/// \param	username[in]	The name of the account to change to password
		///							for.
		/// \param	oldPassword[in]	The old password.
		/// \param	newPassword[in]	The new password.
		/// \return
		/// - True, if the request was issued successfully.
		/// - False, if the request could not be made (e.g., status is not BD_READY).
		bdBool changePassword(	const bdNChar8* username,
								const bdNChar8* oldPassword,
								const bdNChar8* newPassword);

		/// Reset the password on an account using the license code that the
		/// account was created with.
		/// This function is only relevant for titles that implement license
		/// management.
		/// \param	username[in]	The name of the account to reset the password
		///							for.
		/// \param	newPassword[in]	The new password to set for the account.
		///	\param	licenseCode[in]	The license code that the account was originally
		///							created with.
		/// \return
		/// - True, if the request was issued successfully.
		/// - False, if the request could not be made (e.g., status is not BD_READY).
		bdBool resetAccount(const bdNChar8* username,
							const bdNChar8* newPassword,
							const bdNChar8* licenseCode);

		/// Delete an account.
		/// This can be performed by either specifying the license that the
		/// account was originally created with, or the password for the account.
		/// \param	username[in]	The name of the account to delete.
		/// \param	code[in]		The password for the account, or the license
		///							code that the account was created with.
		/// \return
		/// - True, if the request was issued successfully.
		/// - False, if the request could not be made (e.g., status is not BD_READY).
		bdBool deleteAccount(	const bdNChar8* username,
								const bdNChar8* code);

		/// Migrate accounts that were created in a one title so that they
		/// can be used in another title. This function will only work if the
		/// two titles are configured to share accounts. It requires knowledge
		/// of a license for both titles, and is therefore only relevant to
		/// titles that implement licenses.
		/// \param	licenseCode[in]		The license for the current title (that the
		///								accounts will be migrated to).
		/// \param	oldTitleID[in]		The ID of the title that the accounts will
		///								be migrated from. The IDs of previous titles
		///								in the title family can be statically defined
		///								by the game code.
		/// \param	oldLicenseCode[in]	The license code that the account(s) were
		///								created with in the old title.
		/// \return
		/// - True, if the request was issued successfully.
		/// - False, if the request could not be made (e.g., status is not BD_READY).
		bdBool migrateAccount(	const bdNChar8* licenseCode,
								const bdTitleID oldTitleID,
								const bdNChar8* oldLicenseCode);

		/// Authorize an account to access the lobby services.
		/// This performs a Kerberos-like authentication. When this request
		/// completes successfully, a bdAuthInfo will have been filled in, which
		/// must then be passed into the lobby service API's connect call
		/// (bdLobbyServiceImpl::connect).
		/// \param	username[in]	The name of the account to authorize.
		/// \param	password[in]	The password for the account being authorized.
		/// \return
		/// - True, if the request was issued successfully.
		/// - False, if the request could not be made (e.g., status is not BD_READY).
		bdBool authorizeAccount(const bdNChar8* username,
								const bdNChar8* password);

		/// Authorize an account to access a dedicated game host.
		/// This performs a Kerberos-like authentication. When this request
		/// completes successfully, a bdAuthInfo will have been filled in, which
		/// must then be sent to the dedicated host using the game's network
		/// transport. The dedicated host will be able to decrypt the contents
		/// of the bdAuthInfo using its license code. The decrypted contents of
		/// the bdAuthInfo will reveal the authenticated identity of the client's
		/// account to the dedicated host.
		/// \param	username[in]		The name of the account to authorize.
		/// \param	hostLicenseID[in]	The license ID of dedicated host to
		///								authenticate the user to.
		/// \param	password[in]		The password for the user account being
		///								authorized.
		/// \return
		/// - True, if the request was issued successfully.
		/// - False, if the request could not be made (e.g., status is not BD_READY).
		bdBool authorizeAccount(const bdNChar8* username,
								const bdUInt64 hostLicenseID,
								const bdNChar8* password);

		/// Authorize anonymously to access the lobby services.
		/// This is for use by titles that do not implement user accounts, and
		/// will fail for other titles. The service generates a bdAuthInfo 
		/// that will enable the client to connect to the lobby services.
		/// \return
		/// - True, if the request was issued successfully.
		/// - False, if the request could not be made (e.g., status is not BD_READY).
		bdBool authorizeAnonymous();

		/// Authorize an dedicated host to access the lobby services.
		/// This performs a Kerberos-like authentication. Dedicated hosts do not
		/// have user accounts, so the authentication is based on the host's license
		/// code. When this request completes successfully, a bdAuthInfo will have
		/// been filled in, which must then be passed into the lobby service API.
		/// \param	licenseCode[in]	The license code of the dedicated host.
		/// \return
		/// - True, if the request was issued successfully.
		/// - False, if the request could not be made (e.g., status is not BD_READY).
		bdBool authorizeDedicatedHost(	const bdNChar8* licenseCode);

		bdBool authorizeDedicatedServer();
		bdBool authorizeDedicatedServerRSA();

		/// Get a list of usernames associated with a supplied license code.
		/// On completion, m_usernamesForLicenseResult in the bdAuthService
		/// instance will contain an bdFastArray of usernames, represented as
		/// bdNChar8[BD_MAX_USERNAME_LENGTH]s.
		/// \param licenseCode[in] The license code to find usernames for.
		/// \return
		/// - True, if the request was issued successfully.
		/// - False, if the request could not be made (e.g., status is not BD_READY).
		bdBool getUsernamesForLicense (const bdNChar8* licenseCode);

#if defined BD_PLATFORM_PS3

		/// Obtain a cookie to be passed into the sceNpManagerRequestGetTicket
		/// function.
		/// This must be called in order for the ticket to be accepted by 
		/// bdAuthService.
		/// Calling this function multiple times will overwrite the buffer the
		/// return pointer points to.
		/// \param	size[out]	The size of the obtained cookie
		/// \return		A pointer to the cookie buffer
		const bdByte8 *getPS3Cookie(bdUInt &size);

		/// Convert a ticket obtained from Sony's Network Platform for PS3 into
		/// a DemonWare ticket for use with bdLobbyServiceImpl. The Sony Network
		/// Platform ticket should have been obtained using Sony's 
		/// "sceNpManagerRequestTicket" function.
		/// \param	ticket[in]	The ticket obtained from sceNpManagerRequestGetTicket
		/// \param	ticketSize	The size of the ticket buffer
		/// \param	PS3Info[in]	An optional bdPS3AuthInfo, which will contain PS3
		///						-specific information about the signed-in user if
		///						the request completes successfully.
		/// \return 
		///		- true, if the request was started successfully.
		///		- false, otherwise.
		bdBool authorizePS3Ticket(	const bdUByte8 *ticket, 
									bdUInt ticketSize,
									bdPS3AuthInfoRef PS3Info);

		/// Creates an authentication cookie and stores it in the buffer specified by buf
		/// \param cookieBuf[in] Pointer to the buffer where the cookie is stored. The size of the buffer has to be
		///	               at least of BD_ENCRYPTION_KEY_SIZE (24) bytes
		static void createPS3Cookie(bdByte8* cookieBuf);

		/// Sets a specific cookie to be used with the authentication server.
		/// \param cookie[in] Pointer to the byte array containing the cookie.
		/// \note: Do not call this function while an authentication request is in progress
		/// as doing so will cause failure.
		void setPS3Cookie(bdByte8* cookie);

#elif defined BD_PLATFORM_WII

		/// Authorize a Wii using the SVL token.
		/// This token is retrieved from NAS using DWC_SvlGetTokenAsync()
		/// The token is a base64 encoded encrypted string.
		/// \param	token[in]	The token obtained from NAS.
		/// \param  WiiID[in]   The key used to encrypt the data [currently the Wii Mac Adrr]	
		/// \param  userID[in]   The userId of the player authenticating.
		/// \param  userName[in]   The userName of the player authenticating.
		/// \return		A boolean indicating success or failure.
		/// \note		This implementation is essentially authentication by assertion, 
		///				which is not all that secure but it'll do for now!
		bdBool authorizeNASToken(	const bdNChar8 *token,
									const bdMACAddr encryptionKey,
									const bdOnlineUserID userID,
									const bdNChar8* userName);

#endif //BD_PLATFORM_PS3

		/// Get the current status of the bdAuthService API. Use this to determine
		/// whether any requests are currently in progress, or if a request has
		/// completed. See bdAuthService::bdStatus for details on the different
		/// states that may be returned.
		/// \return	The current status of the API.
		bdAuthService::bdStatus getStatus();

		/// Get the error code returned from the last request.
		/// Error codes are defined by the bdLobbyErrorCode enumeration.
		/// \return The error code.
		bdLobbyErrorCode getErrorCode() const;

		/// Get the auth info after a successful authentication request.
		/// The auth info is the encrypted part of an authentication ticket and needs
		/// to be passed to the lobby server (see bdLobbyService::connect)
		/// \return A pointer to the auth info member variable.
		const bdAuthInfo *getAuthInfo() const;

		/// Get the AuthTicket member variable. The auth ticket is the unencrypted part
		/// of an authentication ticket.
		/// A call to bdAutherService::authorizeAccount must have completed before
		/// getting a ticket with valid info!
		/// \return	A pointer to the auth ticket member variable.
		const bdAuthTicket *getAuthTicket() const;

		bdFastArray<bdNChar8[BD_MAX_USERNAME_LENGTH]> m_usernamesForLicenseResult;

	protected:
		/// Connect to the authorization service and begin processing a request.
		void startTask();

		/// Event handler for connection related events
		void onConnect(bdLobbyConnectionRef connection);

		/// Event handler for connection related events
		void onDisconnect(bdLobbyConnectionRef connection);

		/// Event handler for connection related events
		void onConnectFailed(bdLobbyConnectionRef connection);

		bdBitBufferRef makeCreateAccount(	const bdTitleID titleID,
											const bdNChar8* desiredUsername,
											const bdNChar8* password,
											const bdNChar8* licenseCode = BD_NULL) const;

		bdBitBufferRef makeGetUsernamesForLicense(	const bdTitleID titleID,
													const bdNChar8* licenseCode) const;

		bdBitBufferRef makeChangeUserKey(	const bdTitleID titleID,
											const bdNChar8* username,
											const bdNChar8* oldPassword,
											const bdNChar8* newPassword) const;

		bdBitBufferRef makeResetAccount(const bdTitleID titleID,
										const bdNChar8* username,
										const bdNChar8* newPassword,
										const bdNChar8* licenseCode) const;

		bdBitBufferRef makeDeleteAccount(	const bdTitleID titleID,
											const bdNChar8* username,
											const bdNChar8* code) const;

		bdBitBufferRef makeMigrateAccount(	const bdTitleID titleID,
											const bdNChar8* licenseCode,
											const bdTitleID oldTitleID,
											const bdNChar8* oldLicenseCode) const;

		bdBitBufferRef makeAuthAccountForService(	const bdTitleID titleID,
													const bdNChar8* username) const;

		bdBitBufferRef makeAuthAnonymousForService(const bdTitleID titleID) const;

		bdBitBufferRef makeAuthHostForService(	const bdTitleID titleID,
												const bdNChar8* licenseCode) const;

		bdBitBufferRef makeAuthRequestForDedicatedServer(const bdTitleID titleID) const;
		bdBitBufferRef makeAuthRequestForDedicatedServerRSA(const bdTitleID titleID);

		bdBitBufferRef makeAuthAccountForHost(	const bdTitleID titleID,
												const bdNChar8* username,
												const bdUInt64 hostLicenseID) const;

		bdLobbyErrorCode handleReply(bdUByte8 replyType, bdBitBufferRef reply);

#if defined BD_PLATFORM_PS3
		bdBitBufferRef makeAuthForPS3(	const bdTitleID titleID, 
										const bdUByte8 *ticket, 
										bdUInt ticketSize) const;

		bdBool handlePS3Reply(bdBitBufferRef buffer);

#elif defined BD_PLATFORM_WII

		bdBitBufferRef makeAuthForWII(	const bdTitleID titleID,
										const bdNChar8 *token,
										const bdMACAddr encryptionKey,
										const bdOnlineUserID userID,
										const bdNChar8* userName) const ;

#endif //BD_PLATFORM_PS3

		bdLobbyConnectionRef m_connection;

		bdTitleID m_titleID;

		bdCommonAddrRef m_authServiceAddr;

		bdAuthService::bdStatus m_status;

		bdBitBufferRef m_request;

		bdAuthInfo m_authInfo;

		bdAuthTicket m_authTicket;

#ifdef BD_PLATFORM_PS3
		/// Contains Sony Network Platform specific authentication results.
		bdPS3AuthInfoRef m_PS3AuthInfo;

		bdByte8 m_PS3CookieKey[BD_ENCRYPTION_KEY_SIZE];
#endif //BD_PLATFORM_PS3

		bdByte8 m_userKey[BD_ENCRYPTION_KEY_SIZE];
		bdByte8 m_licenseKey[BD_ENCRYPTION_KEY_SIZE];

		bdLobbyErrorCode m_errorCode;

		void *m_sendBuffer;
		bdUWord m_sendBufSize;
		void *m_receiveBuffer;
		bdUWord m_recvBufSize;

	public:

		bdNChar8 m_cdKey[MAX_RSA_DATA_SIZE];
		bdInt32 m_cdKeyTimeToLiveSecs;

    private:
		bdRSAKey m_RSAKey;
		
};
#endif //BD_AUTH_SERVICE_H
