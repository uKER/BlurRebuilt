// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_APPLE_PUSH_NOTIFICATION_H
#define BD_APPLE_PUSH_NOTIFICATION_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdLobbyErrorCode.h>

BD_REFERENCE(bdRemoteTask);
class bdRemoteTaskManager;

#define BD_APPLE_PUSH_NOTIFICATION_SET_DEVICE_TOKEN			(1u)
#define BD_APPLE_PUSH_NOTIFICATION_SEND_PUSH_NOTIFICATION	(2u)

#define BD_APPLE_PUSH_NOTIFICATION_MAX_DEVICE_TOKEN_LENGTH	(255u)
#define BD_APPLE_PUSH_NOTIFICATION_MAX_PAYLOAD_LENGTH		(255u)

/// Apple Push Notification Service
class bdApplePushNotification
{
public:
	
	BD_DECLARE_NEW_AND_DELETE_OPERATORS
	
	/// Constructor
	bdApplePushNotification(bdRemoteTaskManager *const remoteTaskManager);
	
	/// Destructor
	~bdApplePushNotification();
	
	/// Set the group memberships for the logged in user.
	/// \param deviceToken[in]:	The device token to be set for the current user (Obtained from APNs).
	///							This must be formatted as a string of characters, each containing 8 bits of the token.
	///							Must not be longer than	BD_APPLE_PUSH_NOTIFICATION_MAX_DEVICE_TOKEN_LENGTH.
	/// \param numBytes[in]:	The number of bytes in the device token.
	/// \return	A reference to the task handle that should be pumped to completion,
	///			or BD_NULL if the operation could not be performed (message 
	///			could not be sent, or maximum number of tasks was exceeded).
	/// \note	When complete, the returned bdRemoteTask will specify:
	///			BD_NO_ERROR - on success.
	bdRemoteTaskRef setDeviceToken(const bdNChar8 *const deviceToken, const bdUInt numBytes);
	
	/// Sends a push notification to the specfied user.
	/// \param payload[in]:		The json-encoded payload of the push notification.
	///							This is defined in Apple's APNs documentation.
	///							Must not be longer than	BD_APPLE_PUSH_NOTIFICATION_MAX_PAYLOAD_LENGTH.
	/// \param userIDs[in]:		A list of userIDs of the users to send the notification to.
	/// \param numEntries[in]:	The number of userIDs in the array.
	/// \return	A reference to the task handle that should be pumped to
	///			completion, or BD_NULL if the operation could not be 
	///			performed (message could not be sent, or maximum number of
	///			tasks was exceeded).
	/// \note	When complete, the returned bdRemoteTask will specify:
	///			BD_APPLE_PUSH_NOTIFICATION_USER_NOT_REGISTERED - if one of the users hasn't set their device token.
	bdRemoteTaskRef sendPushNotification(const bdNChar8 *const payload,
										 const bdEntityID *const userIDs,
										 const bdUInt numEntries);
	
protected:
	
	/// remote task manager
	bdRemoteTaskManager *m_remoteTaskManager;
	
};

#endif //BD_APPLE_PUSH_NOTIFICATION_H

