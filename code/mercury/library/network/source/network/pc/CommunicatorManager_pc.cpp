#include <StdAfx.h>
#include <Network/CommunicatorManager.h>
#include <SystemProfile/SystemProfile.h>
#include <Network/CommunicatorVoiceBuffer.h>
#include "../Shared/Remotetalker.h" 

namespace Communicator
{
	// static instance..
	C_Core	C_Core::s_CommunicatorCore;


	// constructor
	C_Core::C_Core() :	m_MaxLocalTalkers(0), m_MaxRemoteTalkers(0),
						m_LocalRegisteredIndexes(0), m_pRemoteTalkerStorage(NULL), m_pRemoteTalkerList(NULL),
						m_pRemoteTalkerPool(NULL), m_Initialised(false)
	{
	}


	// destructor 
	C_Core::~C_Core()
	{
		Shutdown();
	}


	/// call once to initialise all dynamic resources and startup voice engine.
	bool C_Core::Initialise(const U32 maxLocalPlayers, const U32 maxRemotePlayers)
	{
		if (m_Initialised)
			return true; // already initialised once.

		if (0 == maxLocalPlayers)
			return false; // must have local players, and some storage for the voice data..

		m_Initialised = true;

		return m_Initialised;
	}


	// release all resources
	void C_Core::Shutdown(void)
	{
		UnRegisterAllPlayers();
		Cleanup();
	}

	U32 C_Core::AudioFrameSize(void) const
	{
		return 1;
	}

	// register local player for voice communications.
	bool C_Core::RegisterLocalTalker(const SysProfile::FullProfile& localTalker, const bool enforcePrivileges)
	{
		if (!m_Initialised)
			return false;

		return false;
	}


	// change mode for players communicator
	bool C_Core::SetLocalMode(const U8 userIndex, const eCommunicatorMode newMode)
	{
		if (!m_Initialised)
			return false;

		bool ok = true;
		return ok;
	}

	
	// Attempt to unregister a local player from voice communication encoding
	bool C_Core::UnRegisterLocalTalker(const U8 userIndex)
	{
		if (!m_Initialised)
			return false;

		return false;
	}


	// register a remote talker so we can decode any voice packets they send.
	bool C_Core::RegisterRemoteTalker(const SysProfile::UserIdentifier& remoteTalker)
	{
		if (!m_Initialised)
			return false;

		return false;
	}


	// unregister a remote talker to stop voice being decoded for this player.
	bool C_Core::UnRegisterRemoteTalker(const SysProfile::UserIdentifier& remoteTalker)
	{
		if (!m_Initialised)
			return false;

		return false;
	}


	bool C_Core::IsRemoteTalkerRegistered(const SysProfile::UserIdentifier& remoteTalker)
	{
		if (!m_Initialised)
			return false;
		return false;
	}


	// Unregister all players both local and remote.
	bool C_Core::UnRegisterAllPlayers(void)
	{
		if (!m_Initialised)
			return false;

		return false;
	}


	// change a remote players decoding priority
	bool C_Core::SetDecodingPriority(const U8 userIndex, const SysProfile::UserIdentifier& remoteTalker, const U8 decodePriority, const bool muteRemoteTalker)
	{
		if (!m_Initialised)
			return false;

		return false;
	}


	// does a specific player have a communicator connected.
	bool C_Core::CommunicatorConnected(const U8 userIndex)
	{
		if (!m_Initialised)
			return false;

		return false;
	}

	
	// is a registered local player currently talking..
	bool C_Core::IsLocalTalking(const U8 userIndex)
	{
		if (!m_Initialised)
			return false;

		return false;
	}


	// is a registered remote player talking..
	bool C_Core::IsRemoteTalking(const SysProfile::UserIdentifier& remoteTalker)
	{
		if (!m_Initialised)
			return false;
		return false;
	}


	// encode voice for a local player
	bool C_Core::GetLocalVoiceData(const U8 userIndex, U8* pOutputBuffer, U32 outputBufferSize, U32& numBytesEncoded)
	{
		// default this. 
		numBytesEncoded = 0;

		if (!m_Initialised)
			return false;

		return false;
	}

	bool C_Core::GetLocalVoiceData(const U8 userIndex, C_VoiceBuffer& outputBuffer, U32& numBytesEncoded)
	{
		numBytesEncoded = 0;

		if (!m_Initialised)
			return false;

		return true;
	}

	// submit remote voice data for decoding and playback.
	bool C_Core::SubmitRemoteVoiceData(const SysProfile::UserIdentifier& remoteTalker, U8* pData, const U32 dataSize, U32* pDataSizeDecoded)
	{
		if(pDataSizeDecoded)
			*pDataSizeDecoded = dataSize;

		if (!m_Initialised)
			return false;

		return false;
	}


	// Has a local talker curently muted the specific remote talker ?
	// returns true if muted or false if not
	bool C_Core::LocalMuted(const U8 userIndex, const SysProfile::UserIdentifier& remoteTalker)
	{
		if (!m_Initialised)
			return false;

		return false;
	}


	// has the remote talker muted me
	// returns true if local player has been muted by remote talker otherwise false
	bool C_Core::RemoteMuted(const U8 userIndex, const SysProfile::UserIdentifier& remoteTalker)
	{
		if (!m_Initialised)
			return false;

		return false;
	}


	// once we receive a message that we have been muted/un muted by a local player this is called to update our voice manager so it knows if it
	// should decode any incoming vocie packets from this remote player.
	// remoteTalker		:	remote player who has changed his mute info
	// localMutedUserIndex	:	user index of the local player who is affected by the remote players mute change
	// mute				:	true if we should mute or false if we should unmute
	// forceUpdate			:	if true then the mute info will take effect straight away otherwise the mute wil only take effect on the next call to MuteUpdate()
	void C_Core::SetMutedRemotely(const SysProfile::UserIdentifier& remoteTalker, const U8 localMutedUserIndex, const bool mute, const bool forceUpdate)
	{
		if (!m_Initialised)
			return;
	}


	/// call when you want to update the any mute information to take effect on remote players. 
	void C_Core::MuteUpdate(void)
	{
		if (!m_Initialised)
			return;
	}


	// private function to create XHV engine....
	bool C_Core::CreateEngine(void)
	{
		return false;
   	}


	// private cleanup function.. Releases all dynamic resources and shuts down voice engine
	void C_Core::Cleanup(void)
	{
		m_Initialised					=	false;
		m_MaxLocalTalkers				=	0;
		m_MaxRemoteTalkers				=	0;
		m_LocalRegisteredIndexes		=	0;
	}
}