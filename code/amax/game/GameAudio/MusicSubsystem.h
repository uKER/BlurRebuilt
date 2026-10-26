
#ifndef MUSICSUBSYSTEM_H
#define MUSICSUBSYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "AppInterfaceLayer.h"
#include <GameLogic/RaceModes/RaceModeDefines.h>	// [Blur2PC]
#include <misc_shared/UID/FourCC.h>

#ifdef PLATFORM_PS3
#include <PS3Installer/PS3InstallData.h>
#endif

class MusicPlaylist
{
	enum TransportControlMessage
	{
		Stop,
		Play,
		Next,
		Previous,
		Shuffle,
		Repeat,
	};

public:

	MusicPlaylist(){m_hEntity = NULL;m_pXtEntity = NULL;}
	~MusicPlaylist(){Destroy();}
	void Create(Util::C_String& string);
	bool HasValidComponentHandle(U32 handleID){ if (handleID >= m_aHandle.Size()) return false; return m_aHandle[handleID] != -1; };
	inline Audio::EntityDataMemberHandle GetComponentHandle(U32 handleID){ return m_aHandle[handleID]; };
	inline AudioEntityHandle GetEntityHandle(){return m_hEntity;}
	inline Util::C_String	 GetEntityName(){return m_sEntityName;}
	void SendVector(S32 u32MessageId,F32* vector);
	void SendParameter(S32 u32MessageId,F32 value);
	void SendAction(S32 u32MessageId);
	//Only called by internal async loading thread.
	void SetMusicAsyncPostLoadPending();

private:
	
	void Destroy();	
	void TransportControl(TransportControlMessage msg);	
	Btl::Vector< Audio::EntityDataMemberHandle >* GetHandleArray(){return &m_aHandle;}

private:
	AudioEntityHandle m_hEntity;
	Audio::XtEntityProxy* m_pXtEntity;
	Btl::Vector< Audio::EntityDataMemberHandle > m_aHandle;
	Util::C_String m_sEntityName;
	
};

class MusicSubsystem : public Audio::Subsystem
{
public:

	enum E_PLAYLIST_TYPE
	{
		E_PLAYLIST_TYPE_RESIDENT,
		E_PLAYLIST_TYPE_DYNAMIC,
		E_PLAYLIST_TYPE_MAX
	};

	MusicSubsystem();
	~MusicSubsystem();

	enum E_GAMEFLOW_STATE
	{
		E_GAMEFLOW_STATE_UNKNOWN,
		E_GAMEFLOW_STATE_FRONTEND_PENDING,
		E_GAMEFLOW_STATE_FRONTEND_LOADED_FROM_START,
		E_GAMEFLOW_STATE_FRONTEND_LOADED_FROM_LEVEL,
		E_GAMEFLOW_STATE_LEVEL_LOADED_FROM_FRONTEND
		
	};

	enum MusicSubsystemLoadingState
	{
		eMusicLoadingFrontend,
		eMusicLoadingLevel,
		eMusicLoadingBossLevel,
		eMusicNotLoaded,
		eMusicLoaded
	};

	#define AUDIO_MUSIC_SUBSYSTEM_COMPONENT_LOOKUPS \
	AUDIO_MUSIC_COMPONENT_LOOKUP( PressStartActionIn ), \
	AUDIO_MUSIC_COMPONENT_LOOKUP( MenuStartActionIn ), \
	AUDIO_MUSIC_COMPONENT_LOOKUP( LoadingStartActionIn ), \
	AUDIO_MUSIC_COMPONENT_LOOKUP( LevelStartActionIn ), \
	AUDIO_MUSIC_COMPONENT_LOOKUP( RaceStartActionIn ), \
	AUDIO_MUSIC_COMPONENT_LOOKUP( RaceFinishPassActionIn ), \
	AUDIO_MUSIC_COMPONENT_LOOKUP( RaceFinishFailActionIn ), \
	AUDIO_MUSIC_COMPONENT_LOOKUP( LevelFinishActionIn ), \
	AUDIO_MUSIC_COMPONENT_LOOKUP( DestructionTimeRemaining ), \
	AUDIO_MUSIC_COMPONENT_LOOKUP( MusicType ), \
	AUDIO_MUSIC_COMPONENT_LOOKUP( BossID ), \
	AUDIO_MUSIC_COMPONENT_LOOKUP( BossDistance ), \
	AUDIO_MUSIC_COMPONENT_LOOKUP( BossRelativeSpeed ), \
	AUDIO_MUSIC_COMPONENT_LOOKUP( RacePosition ), \
	AUDIO_MUSIC_COMPONENT_LOOKUP( CheckpointTimeRemaining ), \
	AUDIO_MUSIC_COMPONENT_LOOKUP( LastCheckpointActionIn ), \
	AUDIO_MUSIC_COMPONENT_LOOKUP( VistaID ), \
	AUDIO_MUSIC_COMPONENT_LOOKUP( SetMusicType)

	#undef AUDIO_MUSIC_COMPONENT_LOOKUP
	#define AUDIO_MUSIC_COMPONENT_LOOKUP( x ) x
	enum Message
	{
		AUDIO_MUSIC_SUBSYSTEM_COMPONENT_LOOKUPS,
		NumMessageIds
	};

	static char* GetString( U32 uId )
	{
		#undef AUDIO_MUSIC_COMPONENT_LOOKUP
		#define AUDIO_MUSIC_COMPONENT_LOOKUP( x ) #x
		static char* strMessages[] =
		{
			AUDIO_MUSIC_SUBSYSTEM_COMPONENT_LOOKUPS
		}; 
			
		//StaticAssert( sizeof( strMessages ) / sizeof( char* ) == NumMessageIds, NUM_DECL_STRINGS_DONT_MATCH_NUM_ENUMS );

		assert( uId < NumMessageIds );

		return strMessages[ uId ];
	}

	bool		ProcessSystemMessage(const U32 u32MessageId);
	bool		ProcessCustomMessage(const U32 u32MessageId, U32 u32Data = 0, void* pData = 0);
	bool		SendStateTransition(const U32 u32OutgoingState, const U32 u32IncomingState)
	{
		return true;
	}
	void		Update();
	void		ProcessPostLoad();

	static void		LookupComponentHandles(AudioEntityHandle hEntity, Btl::Vector< Audio::EntityDataMemberHandle > *m_aHandle);
	void			SetMusicAsyncPostLoadPending() { m_MusicAsyncPostLoadPending = true; }

#ifdef PLATFORM_PS3
	bool		CanInstallUI() { return C_PS3InstallData::Instance().SafeToOpenFile(); }
#else
	bool		CanInstallUI() { return true; }
#endif
	
private:
	void LoadAsync(MusicSubsystemLoadingState eState, const Util::C_String& playlistName, MusicPlaylist* PLObj);
	void CacheMessage(const U32 u32MessageId, U32 u32Data , void* pData);

	MusicPlaylist* m_playlist[E_PLAYLIST_TYPE_MAX];
	bool m_inRace;
	bool m_isSingleplayer;
	bool m_isSplitScreen;	
	S8	m_prevPosition;
	E_GAMEFLOW_STATE m_currentState;
	F32	m_MusicType;
	bool m_MusicAsyncPostLoadPending;
	MusicSubsystemLoadingState m_eMusicLoadingState;
	Game::E_RaceMode m_eRaceMode;
	U32 m_u32CachedMessageId;
	U32 m_u32CachedData;
	void* m_pCachedData;
	bool m_bMessageCached;
};

#endif // MUSICSUBSYSTEM_H
