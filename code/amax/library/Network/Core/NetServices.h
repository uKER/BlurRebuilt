// ***************************************************************
//  NetServices   version:  1.0   ·  date: 04/11/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	handles the platform specific network management,
//			also manages all structures required for network play,
//			sessions, scoreboards etc.
// ***************************************************************

#ifndef __NET_SERVICES_H__
#define __NET_SERVICES_H__

#ifdef _MSC_VER
#pragma once
#endif

extern "C" 
{
	#include <lua.h>
}

#include <Core/Core.h>
#include <core/threads/mutex.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h> // for demonware logging
//#include "../Message/NetMessageListener.h"
#include <Network/Message/NetMessageListener.h>
#include <Network/PublisherServerStorageReader.h>
#include <btl/Containers/vector.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdGroup/bdGroup.h>
#include <bdCore/bdTiming/bdStopwatch.h>
#include <bdLobby/bdTitleUtilities/bdTimeStamp.h>
#include "../Router/NetRouterDefines.h"

#define DW_MAX_NUM_COUNTVALUES					11
#define DW_DEFAULT_GROUP_ID						1983

// WARNING: The refresh value should stay above 60 seconds!
//          - Group data on the DemonWare servers are refreshed once a minute
//          - If this value is lowered it runs the risk of polling the server, if
//          this happens you run the risk of hitting task limits on the servers
//          - Keep it at 60 or your toenails will fall out, I'm serious
#define DW_NUM_SECONDS_BETWEEN_GROUP_REFRESH	300 //update every 5 minutes


#if defined ( PLATFORM_PS3 )
#include <np.h>
#endif

// mercury forward declerations.
class bdContentStreaming;
class bdVoteRank;

namespace Scoreboard { class C_ReaderByRank; class C_ReaderByUsers; class C_Writer; }
namespace NetPlay { class C_Session; class C_SessionEnumerator; class C_PlayerSkillWriter; }
namespace SysProfile { struct GameInvite; }

namespace RacerNet
{
	// forward declerations.
	class C_NetRouterBase;
	class C_NetPlayerManager;
	class C_NetRace;
	class C_NetParty;
	class C_NetTimer;
	class C_NetCreationHelper;

	// maximun number of sessions we return from a matching server query.
	#define NET_MAX_MATCHING_SESSIONS_TO_ENUMERATE (10)

	/// Single place to access all network services from. 
	class C_NetServices : public C_NetMessageListener
	{
	private:
		//enums for title storage
		typedef enum
		{
			eTitleStorageWaitingForInit = 0,
			eTitleStorageInit,
			eTitleStorageSetDownload,
			eTitleStorageDownloading,
			eTitleStorageError,
			eTitleStorageCompleted
		}eTitleStorage;			// title storage download states

		typedef enum
		{
			eMultiplayer_Pack = 0,
			eMultiplayerTS_Loc,
			eTitleStorageFilesMax //don't use this enum
		}eTitleStorageFiles;	// title storage files that are downloaded


	public:

		/// Defines the different types of matchmaking sessions we create on the server.
		typedef enum
		{
			eMatchmakingSessionTypeParty = 0,	/**< controls the marty matchmaking session. */	
			eMatchmakingSessionTypeRace,		/**< controls the race matchmaking session. */	
			eMatchmakingSessionMax,
		}eMatchmakingSessionType;

		typedef Btl::Vector<NetPlay::C_Session*, Btl::VectorParameters_Array<eMatchmakingSessionMax> > NetMatchmakingSessionsArray;
		typedef Btl::Vector<U8, Btl::VectorParameters_Static<NET_MAX_MATCHING_SESSIONS_TO_ENUMERATE> > NetMatchingRemapList;

		/// get a reference to the static class instance
		static C_NetServices& Instance(void)	{return s_NetServices;}

		/// Initialise dynamic resources required for network services.
		/**	@return true if resources were alloacted.
		 */
		bool Initialise();
		void InitialiseTitleStorage( Btl::Vector<Btl::String> &filesToDownLoad );
		/// has the netwoek services been correctly initialised.
		/** @return true if all dynamic services have been allocated and initialised.
		 */
		bool IsInitialised(void) const {return m_Initialised;}

		/// Shutdown network services and release all dynamic resources.
		/** This will also disconnect from all game servers.
		 */
		void Shutdown(void);

		/// Poll network services.
		/** Need to call this each frame.
		 */
		void Poll(void);

		// try and enter the net services critical section.
		void EnterLock(void);

		// try and leave the net services critical section.
		void Exitlock(void);

		/// Sync enum defines with lua.
		void CreateLuaEnums(lua_State *pState, char *tableName);

		/// Access to the network router.
		/** Only call this function if the net services have been initialised. 
		 *	@see Initialise()
		 *	@see IsInitialised()
		 *	@return reference to the object for sending receiving .
		 */
		C_NetRouterBase& Router(void) {return *m_pRouter;}

		/// Access to the network player manager.
		/** Only call this function if the net services have been initialised. 
		 *	@see Initialise()
		 *	@see IsInitialised()
		 *	@return reference to the object for tracking the local and remote network players.
		 */
		C_NetPlayerManager&	PlayerManager(void) {return *m_pPlayerManager;}

		/// Access to the network race manager.
		/** Only call this function if the net services have been initialised. 
		 *	@see Initialise()
		 *	@see IsInitialised()
		 *	@return reference to the object for controlling the network race.
		 */
		C_NetRace& Race(void) {return *m_pNetRace;}

		/// Access to the network party manager.
		/** Only call this function if the net services have been initialised. 
		 *	@see Initialise()
		 *	@see IsInitialised()
		 *	@return reference to the object for controlling the network party.
		 */
		C_NetParty& Party(void) {return *m_pNetParty;}

		/// Access a specific matchmaking sssion manager.
		/** Only call this function if the net services have been initialised. 
		 *	@see Initialise()
	 	 *	@see IsInitialised()
		 *	@param type the matchmaking session manager to returm, @see eMatchmakingSessionType.
		 *	@return reference to the object for controlling the specified matchmaking session.
		 */
		NetPlay::C_Session& MatchmakingSession(const eMatchmakingSessionType type) {assert(type < eMatchmakingSessionMax); return *(m_pMatchmakingSessions[type]);}

		/// Access to the matchmaking session enumerator object.
		/** Only call this function if the net services have been initialised. 
		 *	@see Initialise()
		 *	@see IsInitialised()
		 *	@return reference to the object for finding available matchmaking sessions in progress.
		*/
		NetPlay::C_SessionEnumerator& MatchmakingSessionEnumerator(void) {return *m_pSessionEnumerator;}

		/// can be configured to hold indexes into a NetPlay::C_SessionEnumeraor object since not all sessions may be joinable.
		/** gets setup after we have enumerated matching sessions, not all sessions may be joinable so we store the indexes
		 *	of the joinable sesion in this remap table so the ui can access the correct ones.
		 */
		NetMatchingRemapList& MatchingRemapList(void) {return m_MatchingRemapList;}

		/// Access to the object for storing the data used to create / join network sessions.
		/** Only call this function if the net services have been initialised. 
		 *	@see Initialise()
		 *	@see IsInitialised()
		 *	@return reference to the object for storing settings / player details required for the session.
		*/
		C_NetCreationHelper& SessionCreationHelper(void) {return *m_pNetCreationHelper;}

		/// Access to the internal scoreboard reader by rank.
		/** Only call this function if the net services have been initialised. 
		 *	@see Initialise()
		 *	@see IsInitialised()
		 *	@return reference to the object for reading scoreboards based on ranks.
		 */
		Scoreboard::C_ReaderByRank& ScoreboardReaderByRank(void) {return *m_pScoreboardReaderByRank;}

		/// Access to the internal scoreboard reader by users.
		/** Only call this function if the net services have been initialised. 
		 *	@see Initialise()
		 *	@see IsInitialised()
		 *	@return reference to the object for reading specific player(s) data from a scoreboard.
		 */
		Scoreboard::C_ReaderByUsers& ScoreboardReaderByUsers(void) {return *m_pScoreboardReaderByUsers;}

		/// Access to the internal scoreboard writer.
		/** Only call this function if the net services have been initialised. 
		 *	@see Initialise()
		 *	@see IsInitialised()
		 *	@return reference to the object for writing data to a scoreboard for a specific player.
		 */
		Scoreboard::C_Writer& ScoreboardWriter(void) {return *m_pScoreboardWriter;}

		/// Access to the internal player skill writer
		/** Only call this function if the net services have been initialised. 
		*	@see Initialise()
		*	@see IsInitialised()
		*	@return reference to the object for writing player skill results at the end of races.
		*/
		NetPlay::C_PlayerSkillWriter& PlayerSkillWriter(void) {return *m_pPlayerSkillWriter;}

		/// Stops any scoreboard read / write operations that are currently in progress.
		void StopScoreboardRequests(void);

		/// When the game gets a notification from the system that mute information has changed this function should be called.
		/** The purpose of this function is to send updated mute information to either the party session or the race session so
		 *	the mute actually happens.
		 */
		void CommunicatorMuteInformationChanged(void);

		/// Stores a game invite for that will be processed and acepted asap.
		/** Game invites come through the froends list on px3/pc and via the guide on xbox360, the game will only start
		 *	acecpting an invite once its passed to via this finction.
		 *	@param gi correctly configired gameInvite structure that has come from the Operating Sysyem.
		 */
		void AcceptGameInvite(const SysProfile::GameInvite& gi);

		/// Gets a crude representation of our outgoing / incoming bandwidth usage.
		/** Should only be used whilst debugging.
		 */
		void DebugGetBandwidthUsage(F32& in, F32& out);

		/// Gets access to the LSP content streaming service wrapper.
		/** @return NULL if the router is not conected or it's not possible to
		  *	stream data to the server at this point in time.
		*/
		bdContentStreaming* GetContentStreamingServer(void);

		/// [PK] Gets access to the LSP vote rank service wrapper.
		/** @return NULL if the router is not conected or it's not possible to
		  *	vote on data to the server at this point in time.
		*/
		bdVoteRank*	GetVoteRank(void);

		/// On PS3 this will take the player data fromplayer manager and attempt to update the XMB met players list
		void AddToRecentPlayers(void);

		/// on PS3 this will be called when we get the notification back from the np manager callback.
		/** We add players to the met list in batches so it may take more than 1 server call.
		 */
		void FinishedAddingRecentPlayers(void) { m_CanAddMoreRecentPlayers = true; }

		/// [Dave B DW] Sets a user group
		/** Player is added to a ranked event groups as they move around, when not in a
			Ranked Event they are added to the default group
			*/
		void SetUserGroup(const S32 &groupID);	

		/// [J BUNNER] starts the get server time task
		//	returns true if complete false if not
		bool StartServerTimeTask();

		/// [J BUNNER] return stored server time stamp
		U64  GetServerTimeStamp() {return m_ServerTimeStampAtSessionStart.m_timeStamp;}

		/// [J BUNNER] return Server Time Task Failure flag
		bool GetServerTimeTaskFailed(){return m_ServerTimeTaskFailed;}

		/// [TK] Show player count data in UI
		bool ShowPlayerCountData();

		/// [Dave B DW] Returns the total number of players online or just in ranked events
		U32 GetOnlinePlayerCount(const bool rankedEventsOnly = true);

		/// [Dave B DW] Returns the total number of players in a single ranked event
		U32 GetRankedEventPlayerCount(const U32 &groupID);
				
		/// [Dave B DW] Flags the "m_GroupCountRetrieved" value as dirty
		/** This sets the "m_GroupCountRetrieved" value to false so the next time
			"UpdatPlayerCount" it gets an updated count from the DemonWare servers */
		void AllowPlayerCountUpdate(void);
		void StopPlayerCountUpdate(void);

		// Starts the title storage download sequence and pumps the file download
		void TitleStorageUpdate(void);

		// Checks the status of the download
		int TitleStorageStatus(void);

		// Handles time-out for waiting to init
		void ResetTimeOutTimer(void);

		//reset the downloaded flag, used when an invite fails due to the fact there are two different playlists in use
		void TitleStorageStatusReset(void);

		/// Stores the UI settings for matchmaking mode.
		/**
			@param matchByRequiredRegion true if we wish to match by a specific region or false if we prefer a specific region.
			@param region specific region to match on based on preferred/required mode.
		 */
		void SetMatchingRegionInfo(const bool matchByRequiredRegion, const eMatchingRegion region) { m_MatchingByRequiredRegion = matchByRequiredRegion; m_MatchingRegion = region; }

		/// Returns the current matching region information.
		void GetMatchingRegionInfo(bool& matchByRequiredRegion, eMatchingRegion& region) { matchByRequiredRegion = m_MatchingByRequiredRegion; region = m_MatchingRegion; }

		void SetJoinSessionsInRace( const bool joinSessionsInRace ) { m_JoinSessionsInRace = joinSessionsInRace; }
		bool GetJoinSessionsInRace() const { return m_JoinSessionsInRace; }

		void RefreshLocalPlayerName( const WCHAR* pName );

	protected:
		// platform socket router passes incoming messages to these functions when we call C_NetServices::Poll()
		bool ReceiveNetMessage(C_NetMessage& msg, const DatagramUnReliablePacketHeader& hdr, const U32 numUnEncryptedBytes, bdAddrHandleRef& addr);
		bool ReceiveNetMessage(C_NetMessage& msg, const DatagramReliablePacketHeader& hdr,  bdAddrHandleRef& addr);
		bool ReceiveNetMessage(const DatagramAcknowledgementHeader& hdr,  bdAddrHandleRef& addr);

		/// [Dave B DW] Pump any active group tasks
		/** Any active group related task will be pumped here */
		void PumpGroupTasks(void);

		/// [Dave B DW] Terminate a group task when it is complete
		void KillGroupCountTask(void);
		void KillSetGroupTask(void);

		/// [J Bunner] Terminate a Server time task when it is complete
		void KillServerTimeStampTask(void);

		/// [Dave B DW] This will update the group counts when needed
		void RetrieveGroupsPlayerCount(void);

	private:
		explicit C_NetServices();
		virtual ~C_NetServices();
		C_NetServices(const C_NetServices& original);				// private copy constructor
		C_NetServices& operator = (const C_NetServices rhs);		// private un coded assignment operator
		
		bool CreateRouter(void);
		bool CreatePlayerManager(void);
		bool CreateScoreboardReaders(void);
		bool CreateScoreboardWriter(void);
		bool CreateRace(void);
		bool CreateParty(void);
		bool CreateMisc(void);
		bool CreateOnlineSessions(void);

		void UpdateAddToRecentPlayers(void);
		void UpdateCommunicatorInserted(F32 frameTimeInSeconds);
		void UpdatePeerRouting(void);

		//title storage methods
		
		//Cleans up after are download successfully or there has been an error
		void TitleStorageStopAndCleanup(void);

		//Decompresses and loads the file downloaded from title storage
		bool TitleStorageLoad(U8 CurrFileDownload, U32 numBytesDownloaded);
		
	private:
		static C_NetServices			s_NetServices;
		MT::Lock*						m_pLock;
		bool							m_Initialised;
		C_NetRouterBase*				m_pRouter;
		C_NetPlayerManager*				m_pPlayerManager;
		Scoreboard::C_ReaderByRank* 	m_pScoreboardReaderByRank;
		Scoreboard::C_ReaderByUsers*	m_pScoreboardReaderByUsers;
		Scoreboard::C_Writer*			m_pScoreboardWriter;
		NetPlay::C_PlayerSkillWriter*	m_pPlayerSkillWriter;
		NetPlay::C_Session*				m_pMatchmakingSessions[eMatchmakingSessionMax];	
		NetPlay::C_SessionEnumerator*	m_pSessionEnumerator;
		C_NetRace*						m_pNetRace;
		C_NetParty*						m_pNetParty;
		C_NetTimer*						m_pFrameTimer;
		C_NetCreationHelper*			m_pNetCreationHelper;
		F32								m_MuteUpdateDelay;
		F32								m_CommunicatorInsertedDelay;
		NetMatchingRemapList			m_MatchingRemapList;
		U16								m_TotalRecentPlayersToAdd;
		U16								m_RemainingRecentPlayersToAdd;
		U16								m_RecentPlayersToAddStartOffset;
		bool							m_CanAddMoreRecentPlayers;

		bool							m_SetGroupTaskActive;
		bdRemoteTaskRef					m_SetGroupTask;

		bool							m_GroupCountTaskActive;
		bdRemoteTaskRef					m_GroupCountTask;

		bool							m_ServerTimeStampTaskActive;
		bdRemoteTaskRef					m_ServerTimeStampTask;
		bdTimeStamp						m_ServerTimeStampAtSessionStart;
		bool							m_ServerTimeTaskFailed; 

		bool							m_HaveGroupCounts;
		bdGroupCount					m_GroupCounts[DW_MAX_NUM_COUNTVALUES];
		bdGroupCount					m_GroupCountsBuffer[DW_MAX_NUM_COUNTVALUES];
		bool							m_GroupCountRetrieved;
		bdStopwatch						m_StopWatch;

		//Title storage
		bool							m_TitleStorageFilesInit;
		bdStopwatch						m_TitleStorageRefreshTimer;							// title storage download interval timer
		bdStopwatch						m_TitleStorageTimeoutTimer;							// title storage download interval timer
		bdStopwatch						m_TitleStorageStatusTimeoutTimer;					// title storage download interval timer

		eTitleStorage					m_TitleStorage;										// title storage state
		NetPlay::C_PublisherServerStorageReader *m_pReader;									// title storage file reader
		U8*								m_pTSBuffer;										// title storage download buffer
		U8								m_CurrFileDownload;									// current file being downloaded
		char							m_TitleStorageFiles[eTitleStorageFilesMax][128];	// list of files to download
		
#if defined ( PLATFORM_PS3 )
		Btl::Vector<SceNpId, Btl::VectorParameters_Array<19> > m_RecentPlayers;
#endif

#if !defined (CFG_GOLD)
		bdLogSubscriber					m_dwLog;
#endif

		bool							m_MatchingByRequiredRegion;
		eMatchingRegion					m_MatchingRegion;
		bool							m_JoinSessionsInRace;
	};

}// end namespace RacerNet

#endif //__NET_SERVICES_H__
