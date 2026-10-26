
#ifndef UISUBSYSTEM_H
#define UISUBSYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef PLATFORM_PS3
#include <PS3Installer/PS3InstallData.h>
#endif

#include "AppInterfaceLayer.h"
#include "Game/GameActors/Components/Messages/AudioMessages.h"

typedef Btl::Vector< F32, Btl::VectorParameters_Array< 8 > > AudioEqValues;
		
	//must be formatted this way . ie stopped/resumed/stopped/resumed
#define AUDIO_UI_ACTIONOUTS_STOPPED \
	AUDIO_SMESSAGE( UI_AudioStopped), \
	AUDIO_SMESSAGE( MusicController_AudioStopped), \
	AUDIO_SMESSAGE( Voicemail_AudioStopped), \
	AUDIO_SMESSAGE( Theme_AudioStopped), \
	AUDIO_SMESSAGE( Track00_AudioStopped), \
	AUDIO_SMESSAGE( Track01_AudioStopped), \
	AUDIO_SMESSAGE( Track02_AudioStopped), \
	AUDIO_SMESSAGE( Track03_AudioStopped), \
	AUDIO_SMESSAGE( Track04_AudioStopped), \
	AUDIO_SMESSAGE( Track05_AudioStopped), \
	AUDIO_SMESSAGE( Track06_AudioStopped), \
	AUDIO_SMESSAGE( Track07_AudioStopped), \
	AUDIO_SMESSAGE( Track08_AudioStopped), \
	AUDIO_SMESSAGE( Track09_AudioStopped), \
	AUDIO_SMESSAGE( Track10_AudioStopped), \
	AUDIO_SMESSAGE( Track11_AudioStopped), \
	AUDIO_SMESSAGE( Track12_AudioStopped), \
	AUDIO_SMESSAGE( Track13_AudioStopped), \
	AUDIO_SMESSAGE( Track14_AudioStopped), \
	AUDIO_SMESSAGE( Track15_AudioStopped), \
	AUDIO_SMESSAGE( Track16_AudioStopped), \
	AUDIO_SMESSAGE( Track17_AudioStopped), \
	AUDIO_SMESSAGE( Track18_AudioStopped), \
	AUDIO_SMESSAGE( Track19_AudioStopped), \
	AUDIO_SMESSAGE( Track20_AudioStopped), \
	AUDIO_SMESSAGE( Track21_AudioStopped), \
	AUDIO_SMESSAGE( Track22_AudioStopped), \
	AUDIO_SMESSAGE( Track23_AudioStopped), \
	AUDIO_SMESSAGE( Track24_AudioStopped)


#define AUDIO_UI_ACTIONOUTS_RESUMED \
	AUDIO_RMESSAGE( UI_AudioResumed), \
	AUDIO_RMESSAGE( MusicController_AudioResumed), \
	AUDIO_RMESSAGE( Voicemail_AudioResumed), \
	AUDIO_RMESSAGE( Theme_AudioResumed), \
	AUDIO_RMESSAGE( Track00_AudioResumed), \
	AUDIO_RMESSAGE( Track01_AudioResumed), \
	AUDIO_RMESSAGE( Track02_AudioResumed), \
	AUDIO_RMESSAGE( Track03_AudioResumed), \
	AUDIO_RMESSAGE( Track04_AudioResumed), \
	AUDIO_RMESSAGE( Track05_AudioResumed), \
	AUDIO_RMESSAGE( Track06_AudioResumed), \
	AUDIO_RMESSAGE( Track07_AudioResumed), \
	AUDIO_RMESSAGE( Track08_AudioResumed), \
	AUDIO_RMESSAGE( Track09_AudioResumed), \
	AUDIO_RMESSAGE( Track10_AudioResumed), \
	AUDIO_RMESSAGE( Track11_AudioResumed), \
	AUDIO_RMESSAGE( Track12_AudioResumed), \
	AUDIO_RMESSAGE( Track13_AudioResumed), \
	AUDIO_RMESSAGE( Track14_AudioResumed), \
	AUDIO_RMESSAGE( Track15_AudioResumed), \
	AUDIO_RMESSAGE( Track16_AudioResumed), \
	AUDIO_RMESSAGE( Track17_AudioResumed), \
	AUDIO_RMESSAGE( Track18_AudioResumed), \
	AUDIO_RMESSAGE( Track19_AudioResumed), \
	AUDIO_RMESSAGE( Track20_AudioResumed), \
	AUDIO_RMESSAGE( Track21_AudioResumed), \
	AUDIO_RMESSAGE( Track22_AudioResumed), \
	AUDIO_RMESSAGE( Track23_AudioResumed), \
	AUDIO_RMESSAGE( Track24_AudioResumed)


#define AUDIO_UI_INSTANCES \
	AUDIO_IMESSAGE( UI				), \
	AUDIO_IMESSAGE( MusicController ), \
	AUDIO_IMESSAGE( Voicemail		), \
	AUDIO_IMESSAGE( Theme			), \
	AUDIO_IMESSAGE( Track00			), \
	AUDIO_IMESSAGE( Track01			), \
	AUDIO_IMESSAGE( Track02			 ), \
	AUDIO_IMESSAGE( Track03			 ), \
	AUDIO_IMESSAGE( Track04			 ), \
	AUDIO_IMESSAGE( Track05			 ), \
	AUDIO_IMESSAGE( Track06			 ), \
	AUDIO_IMESSAGE( Track07			 ), \
	AUDIO_IMESSAGE( Track08			 ), \
	AUDIO_IMESSAGE( Track09			 ), \
	AUDIO_IMESSAGE( Track10			 ), \
	AUDIO_IMESSAGE( Track11			 ), \
	AUDIO_IMESSAGE( Track12			 ), \
	AUDIO_IMESSAGE( Track13			 ), \
	AUDIO_IMESSAGE( Track14			 ), \
	AUDIO_IMESSAGE( Track15			 ), \
	AUDIO_IMESSAGE( Track16			 ), \
	AUDIO_IMESSAGE( Track17			 ), \
	AUDIO_IMESSAGE( Track18			 ), \
	AUDIO_IMESSAGE( Track19			 ), \
	AUDIO_IMESSAGE( Track20			 ), \
	AUDIO_IMESSAGE( Track21			 ), \
	AUDIO_IMESSAGE( Track22			 ), \
	AUDIO_IMESSAGE( Track23			 ), \
	AUDIO_IMESSAGE( Track24			 )


	  
class UISubsystem : public Audio::Subsystem, public Messaging::C_MessageHandler
{
public:

		enum Messages
		{
			eCurrentHealth,
			eCurrentHealthLevel,
		};



		#undef AUDIO_IMESSAGE
		#define AUDIO_IMESSAGE( x ) x
		//enum MessageIds
		//{
		//	Previous = - 1,
		//	AUDIO_UI_INSTANCES,
		//	NumMessageIds
		//};
		#undef AUDIO_SMESSAGE
		#define AUDIO_SMESSAGE( x ) x

		//#undef AUDIO_RMESSAGE
		//#define AUDIO_RMESSAGE( x ) x

		enum ActionOutId
		{
			AUDIO_UI_ACTIONOUTS_STOPPED,
			NumActionOutIds
		};

		#undef AUDIO_RMESSAGE
		#define AUDIO_RMESSAGE( x ) x

		enum ActionOutResumedId
		{
			FirstResumed = NumActionOutIds - 1,
			AUDIO_UI_ACTIONOUTS_RESUMED,
			NumActionOutResumedIds
		};

		enum InstanceId
		{
			AUDIO_UI_INSTANCES,
			NumInstanceIds
		};

		//#undef AUDIO_IMESSAGE
	//	#define AUDIO_AOMESSAGE( x ) x

		//enum ActionOut_Stopped
		//{
		//	#define AUDIO_IMESSAGE(x) x##_AudioStopped
		//			AUDIO_UI_INSTANCES,
		//	#undef AUDIO_IMESSAGE
		//	Num_ActionOut_Stopped
		//};

		//enum ActionOut_Resumed
		//{
		//	#define AUDIO_IMESSAGE(x) x##_AudioResumed
		//			AUDIO_UI_INSTANCES,
		//	#undef AUDIO_IMESSAGE
		//	Num_ActionOut_Resumed
		//};

		char* GetInstanceString( U32 uId ) const
		{
			#undef AUDIO_IMESSAGE
			#define AUDIO_IMESSAGE( x ) #x
			static char* strMessages[] =
			{
				AUDIO_UI_INSTANCES
			}; 
			#undef AUDIO_IMESSAGE

			StaticAssert( sizeof( strMessages ) / sizeof( char* ) == NumInstanceIds, NUM_DECL_STRINGS_DONT_MATCH_NUM_ENUMS );

			assert( uId < NumInstanceIds );

			return strMessages[ uId ];
		}

		char* GetActionOutStringStopped( U32 uId ) const
		{
			#undef AUDIO_SMESSAGE
			#define AUDIO_SMESSAGE(x) #x
			static char* strMessages[] =
			{
				AUDIO_UI_ACTIONOUTS_STOPPED
			}; 

			StaticAssert( sizeof( strMessages ) / sizeof( char* ) == NumActionOutIds, NUM_DECL_STRINGS_DONT_MATCH_NUM_ENUMS );

			assert( uId < NumActionOutIds );

			return strMessages[ uId ];

		}

		char* GetActionOutStringResumed( U32 uId ) const
		{
			#undef AUDIO_RMESSAGE
			#define AUDIO_RMESSAGE(x) #x
			static char* strMessages[] =
			{
				AUDIO_UI_ACTIONOUTS_RESUMED
			}; 

			StaticAssert( sizeof( strMessages ) / sizeof( char* ) == NumActionOutResumedIds - NumActionOutIds, NUM_DECL_STRINGS_DONT_MATCH_NUM_ENUMS );

			assert( uId < NumActionOutResumedIds );

			return strMessages[ uId - NumActionOutIds ];

		}

		//char* GetActionOutStringResumed( U32 uId ) const
		//{
		//	#undef AUDIO_RMESSAGE
		//	#define AUDIO_RMESSAGE(x) #x
		//	static char* strMessages[] =
		//	{
		//		AUDIO_UI_ACTIONOUTS_RESUMED
		//	}; 

		//	StaticAssert( sizeof( strMessages ) / sizeof( char* ) == NumInstanceIds, NUM_DECL_STRINGS_DONT_MATCH_NUM_ENUMS );

		//	assert( uId < NumInstanceIds );

		//	return strMessages[ uId ];

		//}

		//char* GetString( U32 UId )
		//{
		//	static char* strMessages[] =
		//	{
		//		"StartActionIn",
		//	};

		//	//StaticAssert( sizeof( strMessages ) / sizeof( char* ) == NumMessageIds - WorldObjectMessageHandler::NumMessageIds, NUM_DECL_STRINGS_DONT_MATCH_NUM_ENUMS );

		//	assert( UId < NumMessageIds );

		//	return strMessages[ UId - NumMessageIds ];
		//}


		enum Message
	{
		E_Msg_HiBits = FOURCC_HI20('UISS'),

		eMoveActionIn,
		eToggleActionIn,
		eForwardsActionIn,
		eBackActionIn,
		eIn_horizontalActionIn,
		eIn_verticalActionIn,
		eOutActionIn,
		eLinkActionIn,
		eStartActionIn,
		eStopActionIn,
		ePressStartActionIn,
		eTabsOpenActionIn,
		eErrorActionIn,

	};

	enum UiSubsystemMessage
	{
		eAudioStopped = 0,
		eAudioResumed,
	

		TotalNumUiMessages
	};

	enum UiSubsystemLoadingState
	{
		eUiAudioNotLoaded,
		eUiAudioLoading,
		eUiAudioLoaded
	};

	enum E_GAMEFLOW_STATE
	{
		E_GAMEFLOW_STATE_UNKNOWN,
		E_GAMEFLOW_STATE_FRONTEND_LOADED_FROM_INIT,
		E_GAMEFLOW_STATE_FRONTEND_LOADED_FROM_LEAVING_LEVEL,
		E_GAMEFLOW_STATE_INGAME_LOADED_FROM_LEAVING_UI	
	};


	enum ActiveUIEntity
	{
		eActiveUIEntity_None,
		eActiveUIEntity_FrontEnd,
		eActiveUIEntity_InGame
	};


	UISubsystem();
	~UISubsystem();

	bool		ProcessSystemMessage(const U32 u32MessageId);
	bool		ProcessCustomMessage(const U32 u32MessageId, U32 u32Data = 0, void* pData = 0);

	bool		SendStateTransition(const U32 u32OutgoingState, const U32 u32IncomingState)
	{
		return true;
	}
	void		Update();

	bool		MessageHandler(const Messaging::C_MessageBase *pMessage );

//	void		LoadUI();

	AudioEqValues&		GetEqValues()			{ return m_BinValues; }
	//ActorMsg::C_AudioGraphicalEQMsg		GetEQValues(){  return EQmsg;}

	void		SetupInstanceHandles();
	void		SuspendInstances();

	void		ResumeInstance(U32 uId);
	void		SuspendInstance(U32 uId);

	void		RegisterActionOuts();

#ifdef PLATFORM_PS3
	bool		CanInstallUI() { return C_PS3InstallData::Instance().SafeToOpenFile(); }
#else
	bool		CanInstallUI() { return true; }
#endif


	F32			GetVOPosition(){return m_VOPosition;}

	void		StartLoadUIEntity(Util::C_String string);
	bool		LoadUIEntityPreLoad();
	void		LoadUIEntityAsync(const Util::C_String& entityName);
	bool		LoadUIEntityAsyncPostLoad();
	void		LoadUIEntity(Util::C_String string);
	void		LoadUIEntityPostLoad();

	void		ReleaseUIEntity();

	void		CreateLoadingUI();
	void		ReleaseLoadingUI();

	UiSubsystemLoadingState		GetUiAudioLoadedState() const;
	//Only called by internal async loading thread.
	void		SetUiAudioAsyncPostLoadPending();

private:

	void		SetSubSystemState(E_GAMEFLOW_STATE state){m_currentSubsystemState = state;}
	E_GAMEFLOW_STATE	GetSubSystemState(){return m_currentSubsystemState;}
	
	void		SetUiAudioLoadedState(UiSubsystemLoadingState state);

	AudioEntityHandle m_hUIAudioEntity;
	Audio::EntityDefinitionHandle m_pDefHandle;

	Audio::EntityDataMemberHandle m_Bin1;
	Audio::EntityDataMemberHandle m_Bin2;
	Audio::EntityDataMemberHandle m_Bin3;
	Audio::EntityDataMemberHandle m_Bin4;
	Audio::EntityDataMemberHandle m_Bin5;
	Audio::EntityDataMemberHandle m_Bin6;
	Audio::EntityDataMemberHandle m_Bin7;
	Audio::EntityDataMemberHandle m_Bin8;

	Audio::ReturnDataHandle g_hBin1;
	Audio::ReturnDataHandle g_hBin2;
	Audio::ReturnDataHandle g_hBin3;
	Audio::ReturnDataHandle g_hBin4;
	Audio::ReturnDataHandle g_hBin5;
	Audio::ReturnDataHandle g_hBin6;
	Audio::ReturnDataHandle g_hBin7;
	Audio::ReturnDataHandle g_hBin8;

	AudioEqValues			m_BinValues;
	//ActorMsg::C_AudioGraphicalEQMsg EQmsg;

	Btl::Vector< Audio::EntityDataMemberHandle > m_aActionOutHandles;

	F32						m_MinHealthLevelThisFrame; 

	bool					m_LevelMusicPlaying;

	Audio::EntityDataMemberHandle m_VOPosParamOutHandle;
	Audio::ReturnDataHandle m_VOPosReturnDataHandle;
	
	F32 m_VOPosition;
	F32 m_VOPositionPrev;

	bool firstTimeLoad;
	bool m_bHaveInstalledUI;
	bool m_bShouldInstallUI;

	bool m_RaceInProgress;
	bool m_VoicemailStarted;
	bool m_GamePaused;
	bool m_trackingVoicemal;
	bool m_startTracking;
	bool m_MPRaceInProgress;

	Audio::XtEntityProxy*	m_hProxy;

	bool m_goingToFrontEnd;
	UI::E_MultiplayerVehicleAbilities abilitySlots[UI::E_MULTIPLAYER_LOADOUT_SLOT_MAX];
	bool m_InGarage;
	Scheduler::GroupHandle			m_hSchedulerGroup;

	E_GAMEFLOW_STATE m_currentSubsystemState;
	UiSubsystemLoadingState			m_uiAudioLoadedState;
	bool							m_uiAudioAsyncPostLoadPending;
	ActiveUIEntity					m_activeUIEntity;
	bool							m_intoUIForFirstTime;
	bool							m_syncLoadInitialUpdateNeeded;
};

#endif // UISUBSYSTEM_H
