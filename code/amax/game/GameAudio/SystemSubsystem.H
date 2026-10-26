
#ifndef SYSTEMSUBSYSTEM_H
#define SYSTEMSUBSYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "AppInterfaceLayer.h"
//#include <misc_shared/UID/FourCC.h>

#define OSCILLOSCOPE_BANDS 48 

typedef Btl::Vector< F32, Btl::VectorParameters_Array< OSCILLOSCOPE_BANDS > > AudioOscilloscopeValues;

class SystemSubsystem : public Audio::Subsystem, public Messaging::C_MessageHandler
{
public:
	enum message
	{
		eConnectToSystem = 0,
		eInitSystem,
		eSuspendCallback,
		eSetUIOptions,
		eDuckMusicLevel,
		eRaiseMusicLevel,
		eFocusCarHeight,
		eFadeOutMusic,
		eFadeOutSfx,
		eFadeInMusic,
		eFadeInSfx,
		eEnterSloMo,
		eExitSloMo,
		eRGBMedium,
		eRGBHard,
		eNearestCarDistance,
		eCarsInRadius,
		eGamePausedQuit,
		eEndofRaceFadeOutSfx,
		eEndofRaceFadeInSfx,
	};

	enum UIVolumeOption
	{
		E_UIVolumeOption_SFX,
		E_UIVolumeOption_Music,
		E_UIVolumeOption_Voice,
		E_UIVolumeOption_MenuMusic,

		E_UIVolumeOption_Count



	};

	SystemSubsystem();
	~SystemSubsystem() { }

	#define AUDIO_SUBSYSTEM_COMPONENT_LOOKUPS \
	AUDIO_COMPONENT_LOOKUP( MusicVolumeOverride ), \
	AUDIO_COMPONENT_LOOKUP( FocusCarHeight ), \
	AUDIO_COMPONENT_LOOKUP( StartMusicFadeOut ), \
	AUDIO_COMPONENT_LOOKUP( StartSfxFadeOut ), \
	AUDIO_COMPONENT_LOOKUP( StartMusicFadeIn ), \
	AUDIO_COMPONENT_LOOKUP( StartSfxFadeIn ), \
	AUDIO_COMPONENT_LOOKUP( DuckMusic ), \
	AUDIO_COMPONENT_LOOKUP( WaterJump ),\
	AUDIO_COMPONENT_LOOKUP( RGBHard ), \
	AUDIO_COMPONENT_LOOKUP( RGBMedium ), \
	AUDIO_COMPONENT_LOOKUP( NearestCarDistance ), \
	AUDIO_COMPONENT_LOOKUP( CarsInRadius ), \
	AUDIO_COMPONENT_LOOKUP( GamePausedQuitActionIn )
	
	#undef AUDIO_COMPONENT_LOOKUP
	#define AUDIO_COMPONENT_LOOKUP( x ) x
	enum ComponentIds
	{
		AUDIO_SUBSYSTEM_COMPONENT_LOOKUPS,
		NumMessageIds
	};

	char* GetString( U32 uId )
	{
		#undef AUDIO_COMPONENT_LOOKUP
		#define AUDIO_COMPONENT_LOOKUP( x ) #x
		static char* strMessages[] =
		{
			AUDIO_SUBSYSTEM_COMPONENT_LOOKUPS
		}; 
			
		StaticAssert( sizeof( strMessages ) / sizeof( char* ) == NumMessageIds, NUM_DECL_STRINGS_DONT_MATCH_NUM_ENUMS );

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

	void		UIMenuVolumeChange(UIVolumeOption, F32 value);
	
	bool		MessageHandler(const Messaging::C_MessageBase *pMessage );

	AudioOscilloscopeValues&		GetOscValues()			{ return m_OscValues; }

	void		LookupComponentHandles();

	Audio::EntityDataMemberHandle GetHandle( U32 uId ) { /*BIZ_ASSERT(m_aHandle[ uId ] != -1);*/  return m_aHandle[ uId ]; }

	AudioEntityHandle m_hSystemEntity;
	U32 m_uSystemEntityDefId;				// unique id of the system entity def. used as the parameter of parameterised connection functions

private:

	AudioOscilloscopeValues m_OscValues;

	Audio::EntityDataMemberHandle m_OscParamOutHandles[OSCILLOSCOPE_BANDS];
	Audio::ReturnDataHandle m_OscReturnDataHandles[OSCILLOSCOPE_BANDS];

	

	Btl::Vector< Audio::EntityDataMemberHandle > m_aHandle;

	bool	m_showingOverlay;
	bool	m_disableSfxFades;
#ifdef PLATFORM_PC
	// [Blur2PC] T144: retail PC +0xa8c / +0xa90, the audio quality asked for and the one last applied
	U32		m_uAudioQuality;
	U32		m_uAppliedAudioQuality;
#endif
};

#endif // MASTERSUBSYSTEM_H
