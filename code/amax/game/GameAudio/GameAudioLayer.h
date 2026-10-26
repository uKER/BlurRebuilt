#ifndef _GAME_AUDIO_LAYER_H
#define _GAME_AUDIO_LAYER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Framework/Layer/Layer.h>


#include <Core/Threads/Thread.h>

#include <Game/GameWorld/GameWorld.h>
#include <Resource/Resource.h>
#include <game_resource/LevelResource.h>
#include "XtAudio.h"
#include <Game/GameActors/AudioActors.h>

#ifndef CFG_GOLD
#include <Render/font/Font.h>
#include <Render/debug/DebugRender.h>
#endif

#define MAX_ENTITIES 2

#define AUDIO_CACHE_COMMON_RESOURCES

#if defined (USER_NBYGRAVE) || defined (USER_JCUTTER) || defined (USER_COFLANAGAN) || defined (USER_SHALL)
#define GAME_AUDIO_BUILD
#endif

struct AudioEntityDebug
{
	Util::C_String					strName;
	Debug::Function*				pDebugFunc;
	Audio::EntityDefinitionHandle	hEntityDef;
};

namespace Audio
{
	extern Debug::Logger &g_Log;

	namespace Core
	{
		extern Debug::Logger &g_Log;
	}

	class C_ErrorHandler : public Debug::SubscriberCommon
	{
	public:
					C_ErrorHandler() {} 
					~C_ErrorHandler() {}
		void		Log(Debug::LogEvent *pEvent); 
		void        LogString(const Util::C_String &string) {}
	};
};



namespace Game
{

	void CacheEntityResources(Audio::XtEntityProxy& hEntity, const char* pchName, bool bInRaceWarning = true, bool bForceLoad = false);

	class C_GameAudioLayer : public Framework::C_Layer,public Messaging::C_MessageHandler
	{
		


	public:

		enum SubsystemsIds
		{
			eDefaultSubsystem = 0,
			eVehicleSubsystem,
			eUISubsystem,
			eMusicSubsystem,
			eDialogueSubsystem,
			eDynamicObjSubsystem,
			eSystemSubsystem,
			eWheelSubsystem,
			eListenerSubsystem,
			ePickupSubsystem,
			eCollisionSubsystem,
			//eEffectSubsystem,
			//eMusicSubsystem,

			TotalNumSubsystems
		};

		C_GameAudioLayer(Messaging::C_MessageSubscriberSystem* pSubscriberSystem);
		virtual ~C_GameAudioLayer();

		// For layer messages
		HRESULT					ProcessMessage(const Framework::E_LayerMessage message);

		//For game messages
		bool					MessageHandler(const Messaging::C_MessageBase* pMessage);

		void					InitSystem();
		HRESULT					Create();
		HRESULT					Destroy();
		HRESULT					Update();
		HRESULT					GlobalUpdate();
		HRESULT					RenderFullscreen();
		HRESULT					RenderSplitscreen();

		#if defined AUDIO_CACHE_COMMON_RESOURCES
		void					CacheCommonResources();
		#endif

		#ifdef AUDIO_DEFERRED_LOADING
		void					ResourcePreProcessVehicle(S32 p_Slot);
		void					VehicleResourcePrePass();
		#endif

		void					SetCarEntityHnd(AudioEntityHandle& hnd) { m_hCarEntity = hnd; } 
		void					SetupPath();
		void					SetActorEntityHnd(SubsystemsIds ID,AudioEntityHandle& hnd){m_hActorEntities[ID] = hnd;}
		void					SetAudioActorHnd(SubsystemsIds ID,Actor::C_ActorHandle& hnd){m_hAudioActorHandles[ID] = hnd;}
		AudioEntityHandle		GetActorEntityHnd(SubsystemsIds ID){return m_hActorEntities[ID];}
		Actor::C_ActorHandle	GetAudioActorHnd(SubsystemsIds ID){return m_hAudioActorHandles[ID];}
		Messaging::C_MessageSubscriberSystem* GetSubscriberSystem() {return m_pSubscriberSystem;}
		bool					TitleHasBGMusicControl();
		bool					AreWeInRace() { return m_bInRace; }
		bool					FineToCreateEntityInstances() { return m_bFineToCreateEntityInstances; }
		Audio::ISystem*			GetSystem() { return m_pSystem;}

		MT::Lock&				GetAsyncLoadLock() { return m_AsyncLoadLock; }

		static Btl::String		WbMapFilenameFunc(const Btl::String& str);

		void					SetSFXVolume(S32 volume){m_profileSFXVolume = volume;}
		F32						FormatBinkMovieVolume(S32 movieVol);

	private:
		static void				GetDatPathFn(const char* pchBuf, U32 uBufLen);
		Audio::ISystem*			m_pSystem;
		Audio::IWaveManager*	m_pWaveManager;
		static                  Sys::C_FilePath m_strPath;
		AudioEntityDebug*		pEntDb[MAX_ENTITIES];
		AudioEntityHandle		m_hCarEntity;
		AudioEntityHandle		m_hActorEntities[TotalNumSubsystems];
		Actor::C_ActorHandle	m_hAudioActorHandles[TotalNumSubsystems];
		//Game::Actors::C_UIAudioActor m_UIActor;
		Messaging::C_MessageSubscriberSystem* m_pSubscriberSystem;
		Audio::C_ErrorHandler*	m_pErrorHandler;
		bool					m_videoPresent;
		U32						m_peakXmaDecoderCount;
		bool					m_bInRace;			// are we in the race yet?
		bool					m_bFineToCreateEntityInstances; // to fix bug where instances can be created after we receive quit level message, which causes a memory leak.
		MT::Lock				m_AsyncLoadLock;

		S32						m_profileSFXVolume; 

		S32				       m_AudioLevelTransitionState;
	};

} // namespace Game

class C_AudioEntityTypeList
{
public:
	void Add(Audio::XtEntityProxy*);
	void Clear() { entityList.Clear(); }

	void IncrementRefCounts();
	void DecrementRefCounts();

private:
	Btl::Vector<Audio::XtEntityProxy*> entityList;
};

extern C_AudioEntityTypeList gAudioDynObjLevelEntities;

#endif // _GAME_AUDIO_LAYER_H
