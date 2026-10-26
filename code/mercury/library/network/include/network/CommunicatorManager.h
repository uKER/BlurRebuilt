// ***************************************************************
//  CommunicatorManager   version:  1.0   ·  date: 02/08/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Voice communication encoding / decoding in network games. 
// ***************************************************************

#ifndef COMMUNICATOR_MANAGER_H
#define COMMUNICATOR_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/SystemHeaders.h>
#include <Core/Threads/Thread.h>

// forward declerations
namespace SysProfile { struct FullProfile; struct UserIdentifier; }


namespace Communicator
{
	// forward declerations
	class C_RemoteTalker;
	class C_VoiceBuffer;

#if defined (PLATFORM_PS3)
	/// Thread to do any async voice communicator prrocessing
	/** Not all platforms reqire a thread to process voice communication,
		game code does not need to create one of these classes.
	 */
	class C_VoiceThread : private MT::Thread
	{
	public:

		// used as a bit field to define what states are active 
		typedef enum
		{
			eStatesMicIndex = 0,			// bit 0 + userindex to test if user has mic.
			eStatesServiceAttached = 31,	// bit 31 tells us if the voice service is currently attached or detached.
		}eStates;

		C_VoiceThread();
		virtual ~C_VoiceThread();

		void Enter(const sys_event_queue_t& queue);
		void Leave(void);
		void ThreadMain();

	public:
		volatile bool m_Quit;

	private:
		sys_event_queue_t m_Queue;
		bool m_Active;
		U32 m_State;
	};
#endif

	/// Static object that handles encoding / decoding of communicator packets for network voice communications.
	/** game uses this object to encode and decode network voice communication.
	 */
	class C_Core
	{
	public:

		/// Different modes a players communicator can be in.
		typedef enum
		{
			eModeNotActive = 0,		/**< not active */
			eModeNormal,			/**< normal operation */
			eModeLoopback,			/**< headset is in loopback */
		}eCommunicatorMode; 

#if defined ( PLATFORM_PS3 )
		/// Used on PS3 to track the input and output voice ports of local players
		struct LocalPlayerPortData
		{
			LocalPlayerPortData() : m_MicPort(0), m_VoiceOutPort(0), m_HeadsetPort(0)
									
			{}
			U32	m_MicPort;
			U32 m_VoiceOutPort;
			U32 m_HeadsetPort;
		};

		// allow for maximum pads on PS3.
		typedef Btl::Vector<LocalPlayerPortData, Btl::VectorParameters_Array<7> > SystemLocalPortData;
#endif

		/// Get a reference to the static class instance
		static C_Core& Instance(void)	{return s_CommunicatorCore;}

		/// call once to initialise all dynamic resources. Audio hardward must have been setup and configured before calling this function.
		/** @param maxLocalPlayers how many local talkers will we support at the same time.
		 *	@param maxRemotePlayers how many remote talkers will the game support.
		 *	@return true if system was successfully initialised.
		 */
		bool Initialise(const U32 maxLocalPlayers, const U32 maxRemotePlayers);

		/// Call to release any dynamic resources and release the communicator engine.
		void Shutdown(void);

		/// How many bytes is a single frame of voice communicator data.
		/** The audio stream is split up into individual frames, at no point should these frames be modified. if you want to
		 *	loose some of the voice data then the whole frame must be lost.
		 */
		U32 AudioFrameSize(void) const;

		/// Attempt to register a local player for voice communication encoding, once registered a players communicator starts up in eCommunicatorMode::eModeNotActive mode. 
		/**	@param localTalker the local players profile who will be registered for voice communications.
		 *	@param enforcePrivileges if true then the player will only be registered for voice communications their gamerprofile ssys thet can. 
		 *	@return true if player was registered for voice communications, or false if not
		 */
		bool RegisterLocalTalker(const SysProfile::FullProfile& localTalker, const bool enforcePrivileges = true);
			
		/// Attempt to change the communicators mode for a local player.
		/** @param userIndex user index for an already registered local talker.
		 *	@param newMode the new mode that the players communicator will be put into.
		 *	&return true if successfull or false if an error occurred.
		 */
		bool SetLocalMode(const U8 userIndex, const eCommunicatorMode newMode);

		/// Test to see if a specific local player is registered for voice communications.
		/** @param localPlayerIndex user index of a specific local player to test.
		 *	@return true if specified local player has been registered for voice communications.
		 */
		bool IsLocalTalkerRegistered(const U32 localPlayerIndex) const {return (m_Initialised) ? (m_LocalRegisteredIndexes&(1<<localPlayerIndex)) ? true : false : false;}	

		/// Attempt to unregister a local player from voice communication encoding.
		/**	@param userIndex the local players user index who will be unregistered from voice communications.
		 *	@return true if player was unregistered or false if not.
		 */
		bool UnRegisterLocalTalker(const U8 userIndex);

		/// Attempt to register a remote talker for voice decoding, once talker is registered you must call SetDecodingPriority() to actually start decoding incomming voice.
		/** @param remoteTalker remote talkers unique identifier.
		 *	@return true if successfull or false if an error occurred.
		 */
		bool RegisterRemoteTalker(const SysProfile::UserIdentifier& remoteTalker);

		/// Attempt to unregister a remote talker so voice will no longer be decoded for this player.
		/** @param remoteTalker remote talkers unique identifier.
		 *	@return true if successfull or false if an error occurred.
		 */
		bool UnRegisterRemoteTalker(const SysProfile::UserIdentifier& remoteTalker);

		/// Test to see if a specific remote player is already registered for voice.
		/**	@param remoteTalker the user identifier of the remote talker to test.
		 *	@return true if the specified remote player is already registered for voice, otherwise false.
		 */
		bool IsRemoteTalkerRegistered(const SysProfile::UserIdentifier& remoteTalker);

		/// Unregister all players both local and remote.
		/** @return true if successfull or false if not.
		 */
		bool UnRegisterAllPlayers(void);

		/// Attempt to change the decoding priority for an already registered remote talker.
		/** @param userIndex an already registered local talker who will act on the change of prioity.
		 *	@param remoteTalker an already registered remote talker who will get the change of priority.
		 *	@param decodePriority new priority for decoding of the remote players voice. 0 = high priority, 255 = low priority.
		 *	@param muteRemoteTalker are we changing the priority because this player needs to be muted?.
		 *	@return true if successfull or false if an error occurred.
		 */
		bool SetDecodingPriority(const U8 userIndex, const SysProfile::UserIdentifier& remoteTalker, const U8 decodePriority, const bool muteRemoteTalker);

		/// Has a specifc local player got a communicator, local player must be registered with RegisterLocalTalker() before using this function.  
		/** @return true if communicator connected or false if not.
		 */
		bool CommunicatorConnected(const U8 userIndex);

		/// Is a local registered player currently talking.
		/** @param userIndex user index for an already registered local talker.
		 *	@return true if the specified player is curently talking.
		 */
		bool IsLocalTalking(const U8 userIndex);

		/// Is a remote registered player currently talking.
		/** @param remoteTalker user identifier for an already registered remote talker.
		 *	@return true if the specified player is curently talking.
		 */
		bool IsRemoteTalking(const SysProfile::UserIdentifier& remoteTalker);

		/// Attempt to retrieve some encoded voice packets for the specified registered local player.
		/** @param userIndex user index for an already registered local talker.
		 *	@param pOutputBuffer pointer to a valid buffer to hold the encoded voice data.
		 *	@param outputBufferSize size in bytes of pOutputBuffer.
		 *	@param numBytesEncoded upon return this will hold how many bytes of voice data got encoded into the buffer.
		 *	@return true if successfull or false if an error occurred.
		 */
		bool GetLocalVoiceData(const U8 userIndex, U8* pOutputBuffer, U32 outputBufferSize, U32& numBytesEncoded);
		
		/// Attempt to retrieve some encoded voice packets for the specified registered local player.
		/** @param userIndex user index for an already registered local talker.
		 *	@param outputBuffer a previously initialised buffer for voice storage.
	 	 *	@param numBytesEncoded upon return this will hold how many bytes of voice data got encoded into the buffer.
		 *	@return true if successfull or false if an error occurred.
		 */
		bool GetLocalVoiceData(const U8 userIndex, C_VoiceBuffer& outputBuffer, U32& numBytesEncoded);

		/// Attepmt to submit some voice data from a remote talker for decoding and playback.
		/** @param remoteTalker user identifier for an already registered remote talker.
		 *	@param pData pointer to the incoming data to decode.
		 *	@param numPacketsToSubmit how many bytes pData points to.
		 *	@return true if successfull otherwise false.
		 */
		bool SubmitRemoteVoiceData(const SysProfile::UserIdentifier& remoteTalker, U8* pData, const U32 dataSize, U32* pDataSizeDecoded=NULL);

		/// Has a local talker curently muted the specific remote talker?.
		/** @param userIndex local player index to chack against.
		 *	@param remoteTalker have we muted this remote player?.
		 *	@return true if muted or false if not.
		 */
		bool LocalMuted(const U8 userIndex, const SysProfile::UserIdentifier& remoteTalker);

		/// Has the remote talker muted me?.
		/** @param userIndex local user index of the player who wants to be tested for being remotley muted.
		 *	@param remoteTalker remote player to test against.
		 *	@return true if local player has been muted by remote talker otherwise false.
		 */
		bool RemoteMuted(const U8 userIndex, const SysProfile::UserIdentifier& remoteTalker);

		/// Once we receive a message that we have been muted/un muted by a local player this is called to update our voice manager so it knows if it should decode any incoming vocie packets from this remote player.
		/** @param remoteTalker remote player who has changed his mute info.
		 *	@param localMutedUserIndex user index of the local player who is affected by the remote players mute change.
		 *	@param mute true if we should mute or false if we should unmute.
		 *	@param forceUpdate if true then the mute info will take effect straight away otherwise the mute wil only take effect on the next call to MuteUpdate().
		 */
		void SetMutedRemotely(const SysProfile::UserIdentifier& remoteTalker, const U8 localMutedUserIndex, const bool mute, const bool forceUpdate);

		/// call when you want to update any mute information to take effect on remote players. 
		/** This needs to called once per frame to enable mute functionality.
		 */
		void MuteUpdate(void);

	private:
		explicit C_Core();
		~C_Core();
		C_Core(const C_Core& original);				// private copy constructor
		C_Core& operator = (const C_Core rhs);		// private un coded assignment operator

		bool CreateEngine(void);
		void Cleanup(void);

	private:
		static C_Core					s_CommunicatorCore;
		U32								m_MaxLocalTalkers;				// how many local talkers can we allow
		U32								m_MaxRemoteTalkers;				// how many remote talkers can we allow
		U32								m_LocalRegisteredIndexes;		// which local players are curently registered for voice

		C_RemoteTalker*					m_pRemoteTalkerStorage;			// dynamically created memory used for tracking remote talkers
		void*							m_pRemoteTalkerList;			// list of current remote talkers
		void*							m_pRemoteTalkerPool;			// pool of available remote talker objects

		bool							m_Initialised;

#if defined (PLATFORM_PC)
#elif defined(PLATFORM_PS3)
		C_VoiceThread					m_VoiceThread;
		sys_ipc_key_t					m_VoiceEventKey;
		sys_event_queue_t				m_VoiceQueue;
		U8								m_Systems;
		SystemLocalPortData				m_LocalPortData;
		bool							m_StartAsap;
		bool							m_Running;
#endif
	};
}

#endif


