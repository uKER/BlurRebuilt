// ***************************************************************
//  NetRouterBase   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Handles sending / receiving network datagrams. Also
//			responsible for connectong to the platform specific servers.
// ***************************************************************

#ifndef NETWORK_ROUTER_BASE_H
#define NETWORK_ROUTER_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/core.h>
#include <bdNet/bdNet.h>
#include <btl/Containers/vector.h>
#include "NetRouterDefines.h"
#include "../Message/NetMessage.h"

//#include <SchedulerConfig.h>

// mercury forward declerations.
class bdContentStreaming;
class bdKeyArchive;
class bdTags;
class bdStorage;
class bdVoteRank;
class bdMessaging;
class bdTitleUtilities;
class bdFacebook;
class bdTwitter;
class bdLinkCode;
namespace NetPlay { class C_BandwidthEvaluator; }
namespace SysProfile { struct GameInvite; }

namespace RacerNet
{
	// forward declerations.
	class C_NetMessageListener;
	class C_NetAccountCreationListener;

	/// Used for connecting to the platform holders servers and any game specific servers.
	/** This also controlls the demonware socket router so all incomming / outgoing datagrams
	 *	will pass through this.
	 *	Each platform will derive from this as it's connection logic may be different. 
	 */
	class C_NetRouterBase
	{
	public:
		C_NetRouterBase();
		virtual ~C_NetRouterBase();

		/// Initialise dynamic resources.
		/** @param maxSendMessageSize the maximum size of a network message that the game will send.
			@return true if resources were alloacted. 
		 */
		virtual bool Initialise(const U32 maxSendMessageSize);

		/// Shutdown router and release all dynamic resources.
		/** This will also disconnect from all game servers.
		 */
		virtual void Shutdown(void);

		/// Attempt to create a new account on the servers.
		/** This is only valid on PC since PS3 and Xbox360 all have their platfor account management handle by the OS.
		 *	Once account creation has finished we will be disconnected from the platform servers. and have to reconnect via PlatformConnect().
		 *	Don't call PlatformConnect() while trying to create an account as that will totally kill the account creation process.
		 *	@param username the name that the user wants to be reference by.
		 *	@param password the password that the user requires.
		 *	@param licenseCode the license code ffrom the users copy of the game.
		 *	@param pListener object that will be used to fire callback when the account creation process completes. NOTE : this object must stay resident for the complete
		 *	account creation process or it will stomp memory.
		 *	@return true if the account process was started or false if not.
		 */
		virtual bool CreateOnlineAccount(char* userName, char *password, char* licenseCode, C_NetAccountCreationListener* pListener);
		
		/// Attempt to connect to our service provider for online functionality.
		/** @param onlineMode if true then if using demonware we will go through NAT negotiation otherwise LAN games will only be possible.
		*	@param username name of player who will be authenticated (only relavant on PC).
		*	@param username password for the player who will be authenticated (only relavant on PC).
		*	@return true if the connection logic was started.
		*/
		virtual bool PlatformConnect(const bool onlineMode, char* userName, char* password) = 0;

		/// Attempt to disconnect from our platform service provider.
		/** Network multiplayer won't be available if disconnected.
		 */
		virtual void PlatformDisconnect(void) = 0;

		/// Call once per frame to pump any active tasks.
		/** This will also pump the socket to receive incomming data and then pass this data off to any registered message listeners.
		 *	@see RegisterListener()
		 */
		virtual void Poll(const F32 frameTimeInSeconds);

		/// Attempt to disconnect from the address reference.
		/** @param addr address to disconect from
		 *	@return true if disconnection was attempted.
		 */
		bool Disconnect(bdAddrHandleRef& addr);

		/// Primarilly for PS3 so the game can hook into to the router so we can listen for NP/demonware ticket replies.
		virtual void PlatformManagerCallback(S32 ev, S32 result, void *arg) {};

		/// Get the current state of our connection to the platform service provider.
		ePlatformConnectionState GetPlatformConnectionState(void) const {return m_PlatformConnectionState;}

#if defined (PLATFORM_PS3)
		/// this should be called when we get the notification from the os that the net control dialog has finished.
		/** the dialog is started up whenever the router tries to connect in online or offline mode.
		 */
		virtual void NetStartDialogFinished(void) {};

		/// On PS3 we show the net start dialog each time we try and connect to our service provider. TCR requirement.
		/** This function needs to be called when the sysUtilCallback() returns CELL_SYSUTIL_NET_CTL_NETSTART_UNLOADED
		 *	so we can continue with the connection attempt once control is returned to the game.
		 */
		virtual void NetStartDialogUnloaded(void) {};
#endif

		/// What was the reason the last connection attempt failed.
		/** Only use this if the connection has actually failed, once a new connection is started then this will be 
		 *	flushed.
		 */
		ePlatformConnectionResult GetLastConnectionErrorCode(void) const {return m_LastConnectionErrorCode;}

		/// have we connected to the service provider in online mode?.
		/** This will only be valid once we have started a connection attempt.
		 *	@see PlatformConnect()
		 *	@see GetPlatformConnectionState()
		 */
		bool IsOnlineMode(void) const {return m_OnlineMode;}

		/// Enable or disable content streaming functionality.
		/** content streaming MUST be enabled prior to calling GetContentStreamingServer() to allocate resources.
		 *	once content streaming is no longer required we can disable it to release resources.
		 *	@param enabled true is we wish to enable content streaming or false if we wish to disable it.
		 *	@return true if succeeded.
		 */
		virtual bool EnableContentStreaming(const bool enabled) { return false; }

		/// Gets access to the LSP content streaming service wrapper.
		/** @return NULL if the router is not conected or it's not possible to
		 *	stream data to the server yet.
		 */
		bdContentStreaming* GetContentStreamingServer(void);

		/// Access to the demonware key archive system.
		/** @return NULL if the router is not connected to demonware or if it's not possible to read / write key data yet.
		 */
		bdKeyArchive* GetKeyArchive(void);

		/// Access to the demonware tags system.
		/** @return NULL if the router is not connected to demonware.
		 */
		bdTags* GetTags(void);

		/// Access to the demonware user storage.
		/** @return NULL if the router is not connected to demonware.
		 */
		bdStorage* GetStorage(void);

		/// Access to the demonware voting system.
		/** @return NULL if the router is not connected to demonware.
		 */
		bdVoteRank* GetVoteRank(void);

		/// Access to the demonware messaging system.
		/** @return NULL if the router is not connected to demonware.
		 */
		bdMessaging* GetMessaging(void);

		/// Access to the demonware facebook system.
		/** @return NULL if the router is not connected to demonware.
		 */
		bdFacebook* GetFacebook(void);

		/// Access to the demonware twitter system.
		/** @return NULL if the router is not connected to demonware.
		 */
		bdTwitter* GetTwitter(void);

		/// Access to the demonware / agora linking system.
		/** @return NULL if the router is not connected to demonware.
		 */
		bdLinkCode* GetAgoraLink(void);

		/// Access to the demonware title utilities.
		/** This includes the profanity string checker and the event record writer.
			@return NULL if the router is not connected to demonware.
		 */
		bdTitleUtilities* GetTitleUtilities(void);

		/// registers a listener to process incomming network packets.
		/** @param pListener class derrived from C_NetMessageListener that will be called when messages are receved from clients.
		 *	@return true if listener was registered.
		 */
		bool RegisterListener(C_NetMessageListener* pListener);

		/// removes a listener from the registered listener list.
		/** @see RegisterListener();
		 *	@param pListener object to be removed from the list.
		 *	@return true if listener was unregistered.
		 */
		bool UnRegisterListener(C_NetMessageListener* pListener);

		/// Get an interface to an outgoing reliable network message.
		/** There is only one allocated outgoing message so calling this multiple times will actually give you an interface to the same object.
		 *	@return interface to the outgoing reliable message.
		 */
		C_NetMessage& GetOutgoingReliable(void) {return m_SendMessage;}

		/// Get an interface to an outgoing un reliable network message.
		/** There is only one allocated outgoing message so calling this multiple times will actually give you an interface to the same object.
		 *	@return interface to the outgoing unreliable message.
		 */
		C_NetMessage& GetOutgoingUnReliable(void) {return m_SendMessage;}

		/// Get an interface to a the message used to receive incoming data..
		/** There is only one allocated incoming message so calling this multiple times will actually give you an interface to the same object.
		 *	Don't use this message at the same time as calling Poll() as that will write into this message with data from the socket.
		 *	@return interface to the message.
		 */
		C_NetMessage& GetReceiveMessage(void) {return m_ReceiveMessage;}

		/// Attemp to send a previously configured unreliable network message to a remote peer.
		/** function will fail if the socket is not in a state where data can be sent.
		 *	@param mag interface to the unreliable message that needs to be sent. @see GetOutgoingUnReliable()
		*	@param addr the address handle of the peer who the data is going to be sent to.
		 *	@return true if the data was sent or false if not.
		*/
		bool SendMessage(C_NetMessage& msg, bdAddrHandleRef addr);

		/// Attempt to send a preconfigured data buffer to the remote peer.
		/** @param pData pointer to a valid data buffer.
		 *	@param dataSize number of bytes to send.
		 *	addr address of the peer who will receive the data.
		 *	@return true if the data was sent or false if not.
		 */
		bool SendMessage(U8* pData, const U32 dataSize, bdAddrHandleRef addr);

		/// Access the internal game invite structure.
		SysProfile::GameInvite& GetGameInvite(void) {return *m_pGameInvite;}

		/// Are we currently evaluating the speed of internet connection.
		bool BandwidthEvaluationInProgress(void) const {return m_pBandwidthEvaluator != NULL ? true : false;}

		/// Get the upstream bandwith available to the game. 
		U16 EvaluatedUpstreamBandwidthKbps(void) const {return m_UpstreamBandwithKbps;}

		/// Get the downstream bandwith available to the game.
		U16 EvaluatedDownstreamBandwidthKbps(void) const {return m_DownstreamBandwithKbps;}

		/// Debug functionality for tracking bandwidth usage.
		void DebugGetBandwidthUsage(F32& in, F32& out) {in = m_UsageIn; out = m_UsageOut;}

		/// Attempt to start the process to evaluate the players bandwidth.
		/** This takes quite a while to complete.
		*/
		void StartBandwidthEvaluation(void);

		/// Attempt to verify a character string.
		/** Only used when creating groups so if no connection to the LSP exists this functionality will fail.
		 *	if the function returns true then continue to call PollVerifyString() until completion
		 *	@param pString string to be verified. This needs to stay in memory for the complete duration of the verification process.
		 *	@return true if the async string verification was begun.
		 */
		virtual bool VerifyString(const char* pString) { return false; }

		/// Continues the asynchronous process of validating a string.
		/** @param validString once the function returns true then this value determins if the string has passed the validation
		 *	@return true once the validation process hhas completed, false if its still in progress..
		 */
		virtual bool PollVerifyString(bool& validString) { validString = false; return true; }

		/// What type of Nat does this player have.
		virtual eNatType GetNatType(void) { return eNatOpen; }

		/// Used on the PS3 to verify the age restriction parental control settings for online play.
		virtual bool CheckOnlineAgeRestriction(void) { return true; }

		/// Access the matching region of the main player.
		/** This is only valid once the router is filly connected in online mode.
			@return the region that the main player resides within for matchmaking purposes.
		 */
		virtual eMatchingRegion GetMatchingRegion(void) { return m_MatchingRegion; }

		/// should the demonware router be enabled for routing data through peers.
		/** this is only valid once the router is connected to the online servers.
		 *	peer routing will never be enabled in offline mode.
		 */
		bool PeerRoutingEnabled(void);

protected:
		void StopBandwidthEvaluation(void);
		void SetMatchingRegion(const eMatchingRegion r) { m_MatchingRegion = r; }

private:
		void TrackIncomingBytes(const U32 size);
		void TrackOutgoingBytes(const U32 size);

	protected:
		ePlatformConnectionState		m_PlatformConnectionState;
		ePlatformConnectionResult		m_LastConnectionErrorCode;
		bool							m_OnlineMode;
		bool							m_ContentStreamingEnabled;
		bool							m_UseRelaySocketRouter;
		NetPlay::C_BandwidthEvaluator*	m_pBandwidthEvaluator;
		bool							m_BandwidthEvaluated;
		U16								m_UpstreamBandwithKbps;
		U16								m_DownstreamBandwithKbps;

		SysProfile::GameInvite*			m_pGameInvite;

	private:
		bool				m_BaseInitialised;
		C_NetMessage		m_SendMessage;
		C_NetMessage		m_ReceiveMessage;
		Btl::Vector<C_NetMessageListener*, Btl::VectorParameters_Static<10> > m_ReceiveListeners;

		F32					m_UsageOut;
		F32					m_UsageIn;
		F32					m_UsageSent;
		F32					m_UsageReceived;
		F32					m_UsageTimer;

		eMatchingRegion		m_MatchingRegion;
	};

//#if PLATFORM_XENON
//void DeferredInitContentStreaming(Scheduler::Task* pTask);
//#endif

}// end namespace RacerNet

#endif // NETWORK_ROUTER_BASE_H
