// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdLobby/bdLobbyEventHandler.h>


bdLobbyEventHandler::bdLobbyEventHandler()
{
}

bdLobbyEventHandler::~bdLobbyEventHandler()
{
}

void bdLobbyEventHandler::onPlayerStatusUpdate(bdOnlineUserID /*userID*/,
											   bdNChar8* /*userName*/,
											   bdUByte8 /*status*/)
{
}


void bdLobbyEventHandler::onTeamMemberStatusUpdate(bdOnlineUserID /*userID*/,
												   bdNChar8* /*userName*/,
												   bdTeamID /*teamID*/,
												   bdNChar8* /*teamName*/,
												   bdUByte8 /*status*/)
{
}

void bdLobbyEventHandler::onRichPresenceUpdate(bdOnlineUserID /*userID*/,
											   bdNChar8* /*userName*/,
											   void* /*data*/,
											   bdUInt /*dataSize*/,
											   bdTeamID /*teamID*/)
{
}

void bdLobbyEventHandler::onChatChannelUpdate(bdUInt64 /*channelID*/,
											  bdOnlineUserID /*userID*/,
											  bdNChar8* /*userName*/,
											  bdUByte8 /*status*/)
{
}

void bdLobbyEventHandler::onChatChannelMessage(bdUInt64 /*channelID*/,
											   bdOnlineUserID /*senderID*/,
											   bdNChar8* /*senderName*/,
											   void* /*message*/,
											   bdUInt /*messageSize*/)
{
}

void bdLobbyEventHandler::onInstantMessage(bdOnlineUserID /*senderID*/,
										   bdNChar8* /*senderName*/,
										   void* /*message*/,
										   bdUInt /*messageSize*/,
										   bdTeamID /*teamID*/)
{
}

void bdLobbyEventHandler::onSessionInvite(bdSessionInvite& /*invite*/)
{
}

void bdLobbyEventHandler::onFriendshipProposal()
{
}

void bdLobbyEventHandler::onTeamProposal()
{
}

void bdLobbyEventHandler::onNewNotification()
{
}

void bdLobbyEventHandler::onNewMail()
{
}

void bdLobbyEventHandler::onNotifyLeave(bdOnlineUserID /*userID*/,
										bdSessionID /*sessionID*/)
{
}

void bdLobbyEventHandler::onMultipleLogon(bdOnlineUserID /*userID*/)
{
}

