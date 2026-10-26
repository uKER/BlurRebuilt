#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#ifdef _MSC_VER
#pragma once
#endif

#include <gravity/sdk/PhysicsSDK.h>
#include <actor_system/ActorSys.h>
#include <Game/ActorDef/GameActorList.h>
#include <Game/GameDef/Polyflags.h>
#include "gravity/narrowphase/CollisionFilter.h"
#include "gravity/narrowphase/ray.h"
#include "gravity/sdk/EventCallback.h"
#include <Render/Texture/GaussianBlur.h>
#include "GameWorldTasks.h"
#include <SceneGraph/RenderStates.h>
#include <Game/GameWorld/EnvironmentSettings.h>
#include <Lod/LodInterface.h>
#include <Game/GameWorld/PostProcessLayer.h>

#include <misc_shared\Messaging\MessageMultiHandler.h>

class C_DriverManager;

namespace SceneGraph
{
	class C_SceneGraph;
	class C_BackFrontSorter;
	class C_FrontBackSorter;
	class C_DepthSorter;
}

namespace WorldRender
{
	class C_LightList;
}

namespace BlobShadows
{
	class C_BlobShadows;
}

namespace DropShadows
{
	class C_DropShadows;
}

namespace SplashCamera
{
	class C_SplashCamera;
}

namespace LightTrails
{
	class C_LightTrails;
}

namespace FX
{
	class I_EnvironmentSettings;
}

namespace Effect
{
namespace Sky
{
	class C_TexturedSkyDome;
}
}

namespace Actors
{
class C_VisibilityDetector;
}

namespace DynamicEffect
{
	class C_Renderer;

	namespace Light
	{
		class C_Manager;
	}

	namespace Particle
	{
		class C_ParticleManager;
	}

	namespace Track
	{
		class C_TrackRenderer;
	}
}

namespace Render
{

namespace Reduced
{
	class C_Manager;
}

}

namespace AuxFlares
{
	class C_AuxFlares;
}

namespace Gravity
{
	class C_Scene;
	class C_TriggerLayer;
}

namespace GameResource
{
	class C_LevelSceneManager;
}

namespace AnimatedCrowd
{
	class C_CrowdSystem;
	class C_CrowdFlags;
}

namespace Sys
{
	class I_ContextProvider;
}

namespace RenderingData
{
namespace Target
{
struct SharedTextureData;
}
}

class C_LightingPrepass;


namespace Game
{

enum E_TriggerLayer
{
	E_TriggerLayer_Pickup,
	E_TriggerLayer_TriggerRegion,
	E_TriggerLayer_Audio_DynamicObjects,

	E_TriggerLayer_Max,
};

struct GameWorldFxParams
{
	U32		luminanceParameter;
	U32		luminanceTexture;
	U32		colourBuffer;
	U32		depthBuffer;
	U32		depthScale;
	U32		screenReflectionTexture;
	U32		blurScreenReflectionTexture;
	U32		normalBuffer;
	U32		dynamicLights;
	U32		bloomBuffer;

	U32		cutSceneLightColour0;
	U32		cutSceneLightColour1;
	U32		cutSceneLightColour2;
	U32		cutSceneLightColour3;
	U32		cutSceneLightDir0;
	U32		cutSceneLightDir1;
	U32		cutSceneLightDir2;
	U32		cutSceneLightDir3;

	U32		vehicleEnvTex;
	U32		time;
	U32		sunDirection;
	U32		dynamicScale;
	U32		shockTexture;
	U32		screenSettingsVS;
	U32		screenSettingsPS;
	U32		globalLightFlareSize;
	U32		globalLightFlareColour;
	U32		carGlobalLightFlareSize;
	U32		carGlobalLightFlareColour;
	U32		planeReflection;
	U32		halfPixelOffset;
	U32		reflectionViewMatrix0;
	U32		reflectionViewMatrix1;
	U32		reflectionViewMatrix3;
	U32		viewPositionPS;
	U32		viewMatrix0;
	U32		viewMatrix1;
	U32		viewMatrix2;
	U32		viewMatrix3;
	U32		viewProjMatrix0;
	U32		viewProjMatrix1;
	U32		viewProjMatrix2;
	U32		viewProjMatrix3;
	U32		nearFar;
	U32		windMatrices;
	U32		leafMatrices;
	U32		nearFarLightFlare;
	U32		heroCarPos;

	int		m_ClampLinear, m_WrapLinear, m_ClampPoint, m_MirrorPoint;
};

class C_GameWorldContextProvider;
class C_GameWorld;
class C_GameActorList;
class C_ViewManager;
class C_Wind;
class C_CrowdGameManager;

struct C_LightFlareSettings
{
	F32											m_GlobalSize;
	F32											m_GlobalSizeX;
	F32											m_GlobalSizeY;
	Maths::C_RGBA_F32							m_GlobalColour;
	F32											m_ReflectionMinSizeX;
	F32											m_ReflectionMinSizeY;
	F32											m_ReflectionMaxSizeX;
	F32											m_ReflectionMaxSizeY;
	Maths::C_RGBA_F32							m_ReflectionColour;
	F32											m_EnvironmentSizeX;
	F32											m_EnvironmentSizeY;
	Maths::C_RGBA_F32							m_EnvironmentColour;
	F32											m_NearFadeDistant;
	F32											m_FarFadeDistant;
	F32											m_EnvironmentNearFadeDistant;
	F32											m_EnvironmentFarFadeDistant;
	F32											m_SizeDistantFactor;
	F32											m_IntensityDistantFactor;
};

struct C_CarLightFlareSettings : public C_LightFlareSettings
{
	Maths::C_RGBA_F32							m_CarWhiteFlaresColour;
	Maths::C_RGBA_F32							m_CarRedFlaresColour;
};

struct C_CarLightSettings
{
	F32		m_SHLightFactor;
	F32		m_SHLightMax;
	F32		m_StaticLightCubemapFactor;
	F32		m_DynamicLightCubemapFactor;
	F32		m_LightCubemapMax;
	F32		m_SunBlobShadowIntensity;
	Maths::HVec3 m_SunBlobShadowPosition;
	Maths::C_RGBA_F32							m_SunFakeLightColour;
	Maths::HVec3								m_SunFakeLightPosition;
	F32											m_SunFakeLightSize;
	C_CarLightSettings()
	{
		m_SHLightFactor = 1.f;
		m_SHLightMax = 1.f;
		m_StaticLightCubemapFactor = 1.f;
		m_DynamicLightCubemapFactor = 1.f;
		m_LightCubemapMax = 1.f;
		m_SunBlobShadowIntensity = 0.f;
		
		m_SunFakeLightColour = Maths::C_RGBA_F32(0.f, 0.f, 0.f, 0.f);
		m_SunFakeLightSize = 0.f;
	}
};

class C_GameWorld  : public C_GameWorldPostProcess, public Messaging::C_MessageMultiHandler, public Lod::I_LodInterface
{
public:
	C_GameWorld();
	~C_GameWorld();

	bool										Create( Actor::C_ActorFactory* pActorFactory, const S32 views );
	bool										Reset();
	bool										CreateViews(const S32 views);
	bool										Destroy();

	void										AddSubscribers();
	void										RemoveSubscribers();

	void										DebugRender( DbUtil::I_DebugRender* pDebugRender );
	void										BeginFrame();
	void										BeginSplitscreen();
	void										ScheduleOnce();
	void										ScheduleSplitscreen();
	void										RenderOnce();
	void										PreRenderSplitscreen();
	void										RenderSplitscreenWithAA();
	void										RenderSplitscreenWithoutAA();
	void										EndRenderSplitscreenWithoutAA();
	void										RenderFullscreen();
	void										PostRender();
	void										Resize();

	void										SetNumViews(const S32 numViews);

	void										Update();
	void										GlobalUpdate();

	bool										LevelSceneManagerUpdate();

	Gravity::C_ExternalConstants&				GetPhysicsExternalConstants();

	void										SetSunDirection( Maths::HVec3 PCREF sunDir ) { m_SunDirection = sunDir; }// temporary, this needs to be replaced with better scene settings

	void										SetReflectionBlur( F32 blurX, F32 blurY, F32 desviation);
	void										SetLevelLightFlareParams(const C_LightFlareSettings& lightFlareSettings);
	const C_LightFlareSettings&					GetLevelLightFlareSettings() const { return m_LevelLightFlareSettings; }
	void										SetCarLightFlareParams(const C_CarLightFlareSettings& lightFlareSettings);
	const C_CarLightFlareSettings&				GetCarLightFlareSettings() const { return m_CarLightFlareSettings; }
	void										SetCarLightSettings(const C_CarLightSettings& carLightSettings);
	const C_CarLightSettings&					GetCarLightSettings() const {return m_CarLightSettings;};

	void										SetMaxBlobShadows(U32 maxBlobShadows);
	const Display::C_Texture3D*					GetHeadlightTexture() const { return &m_HeadlightTexture; }
	const Display::C_RenderTexture2D*			GetMirrorTexture(S32 view = 0) const;
	const Display::C_RenderTexture2D*			GetReflectionTexture(S32 view = 0) const;

	void										SetDynamicDetail(const F32 cpuDetail, const F32 gpuDetail, const bool immediate);
#if PLATFORM_PC
	// [Blur2PC] Retail PC graphical detail preset 1-4 (Blur.exe 1.2 0x0041b9e0; name ours).
	void										SetGraphicalDetail(const U32 graphicalDetail);
	U32											GetGraphicalDetail() const { return m_GraphicalDetail; }
#endif

	Scheduler::GroupHandle						GetRaytraceGroup() {return m_RaytraceGroup;}
public:
	Game::C_GameActorList						*GetActorList();
	Gravity::C_Scene							*GetPhysicsScene();
	GameResource::C_LevelSceneManager			*GetLevelSceneManager();
	SceneGraph::C_SceneGraph					*GetSceneGraph();
	DynamicEffect::C_Renderer					*GetDynamicEffectRenderer();
	DynamicEffect::Light::C_Manager				*GetLightEffectManager();
	DynamicEffect::Particle::C_ParticleManager	*GetParticleManager();
	DynamicEffect::Track::C_TrackRenderer		*GetTrackRenderer();
	BlobShadows::C_BlobShadows					*GetBlobShadows();
	DropShadows::C_DropShadows					*GetDropShadows();
	SplashCamera::C_SplashCamera				*GetSplashCamera();
	Render::Reduced::C_Manager					*GetReducedRender();
	AuxFlares::C_AuxFlares						*GetAuxFlares();
	LightTrails::C_LightTrails					*GetLightTrails();
	C_LightingPrepass*							GetLightingPrepass();
	Gravity::C_TriggerSystem					*GetTriggerSystem();
	Gravity::C_TriggerLayer						*GetTriggerLayer(E_TriggerLayer p_Layer);

	C_ViewManager*								GetViewManager() const { return m_pViewManager; }
	F32											GetVehicleParticleEmitScale() const { return m_VehicleParticleEmitScale; }
	F32											GetVehicleParticleLifeScale() const { return m_VehicleParticleLifeScale; }

	void										SetGlobalFade(F32 fade) { m_GlobalFade = fade; }

	U32											GetElectricSubTechniqueHandle() const;

//	void										CreateEnvironmentSettingsFromSorcery(bool do_flashback=false);
	void										CreateEnvironmentSettingsFromSorcery(bool do_flashback=false, Game::EnvironmentSettings *override_settings = NULL);
	void										ResetEnvironmentSettings();
	FX::I_EnvironmentSettings*					GetEnvironmentSettings(S32 viewport);

	AnimatedCrowd::C_CrowdSystem				*GetCrowdSystem();

	Sys::I_ContextProvider						*GetContextProvider();

	bool										WaitForStreamer() { return m_waitForStreamer; }
	S32											GameStep() const { return m_GameStep; }
	
	void										Exit();

	Display::C_TextureCube*						GetVehicleLightCubeMap(U32 vehicleIndex, U32 doubleBufferIndex);

	void										SetHandleUISceneReady() { m_HandleUISceneReady = true; }
	void										HandleUISceneReady();

private:
	bool										CreatePhysics();
	bool										CreateActorList( Actor::C_ActorFactory* pActorFactory );
	bool										CreateSceneGraph();
	bool										CreateRenderers();

	bool										CreateLevelSceneManager();
	bool										CreateAnimationTaskManager();
	bool										CreateAnimatedModelTaskManager();
	bool										CreateDriverManager();

	bool										CreateCrowdSystem();

	bool										CreateVehicleLightCubeMaps();

	bool										CreateTextureHeadlight();
	void										DestroyTextureHeadlight();

	void										OncePerFrameFXUpdate();
	void										ViewDependentFXUpdate(S32 viewIndex);

	void										CalcNumberOfSteps();
	void										UpdatePhysics();
	void										UpdateGroundPlane();
#ifndef CFG_GOLD
	void										UpdateCubemapOverride();
#endif
	void										UpdateVehicleParticleScale();

	//Message
	virtual bool MessageHandler(const Messaging::C_MessageBase* pMessage);
	
	//Car Effects Tasks
	void BeginCarEffectTasks();
	void EndCarEffectTasks();

	friend void CarDynamicEffectsTask(Scheduler::Task* pTask);
	friend void CarUpdateLightCubeMapTask(Scheduler::Task* pTask);
private:
	Game::C_GameActorList						*m_pActorList;
	C_GameWorldContextProvider					*m_pContextProvider;
	FX::I_EnvironmentSettings					*m_pSorceryEnvironmentSettings;
	F32											m_GlobalFade;

	// renderers
	Effect::Sky::C_TexturedSkyDome				*m_pSky;
	DynamicEffect::C_Renderer					*m_pDynamicEffectRenderer;
	DynamicEffect::Light::C_Manager				*m_pLightEffectManager;
	DynamicEffect::Particle::C_ParticleManager	*m_pParticleManager;
	DynamicEffect::Track::C_TrackRenderer		*m_pTrackRenderer;
	F32											m_VehicleParticleLifeScale;
	F32											m_VehicleParticleEmitScale;
	BlobShadows::C_BlobShadows*					m_pBlobShadows;
	DropShadows::C_DropShadows*					m_pDropShadows;
	SplashCamera::C_SplashCamera*				m_pSplashCamera;
	Render::Reduced::C_Manager*					m_pReducedRender;
	C_LightingPrepass*							m_pLightingPrepass;
	C_Wind*										m_pWind;
	AuxFlares::C_AuxFlares*						m_pAuxFlares;
	LightTrails::C_LightTrails*					m_pLightTrails;
	
	SceneGraph::C_SceneGraph*					m_pSceneGraph;
	SceneGraph::C_BackFrontSorter*				m_pBackFront;
	SceneGraph::C_DepthSorter*					m_pVehicleSorter;
	SceneGraph::C_DepthSorter*					m_pPunchDepthSorter;
	SceneGraph::RenderStates					m_NoMotionBlurStates;
	SceneGraph::RenderStates					m_NoMotionBlurSolidStates;
	SceneGraph::RenderStates					m_MotionBlurStates;
	SceneGraph::RenderStates					m_MotionBlurColourWriteAllStates;
	SceneGraph::RenderStates					m_ColourWriteAllStates;
	SceneGraph::RenderStates					m_RefractAlphaStates;
	SceneGraph::RenderStates					m_VehicleAlphaStates;

	Gravity::C_ExternalConstants				m_PhysicsExternalConstants;
	Gravity::C_Scene							*m_pPhysicsScene;

	GameResource::C_LevelSceneManager			*m_pLevelSceneManager;
	Maths::HVec3								m_SunDirection;

	Display::C_Texture3D						m_HeadlightTexture;

	AnimatedCrowd::C_CrowdSystem				*m_pCrowdSystem;
	AnimatedCrowd::C_CrowdFlags					*m_pCrowdFlagRenderer;
	C_CrowdGameManager							*m_pCrowdGameManager;

	C_DriverManager*							m_pDriverManager;

	C_ViewManager*								m_pViewManager;
	


	bool										m_bRunningCarEffectTasks;
#if PLATFORM_PC
	U32											m_GraphicalDetail;	// [Blur2PC] retail C_GameWorld +0x3ec
#endif

	// Car Dynamic Effects Task
	C_CarDynamicEffectsTaskInit					m_CarDynamicEffectsTaskInit;
	Scheduler::GroupHandle						m_CarDynamicEffectsTaskGroup;
	C_CarUpdateLightCubeMapTaskInit				m_CarUpdateLightCubeMapTaskInit[4]; //One per view
	Scheduler::GroupHandle						m_CarUpdateLightCubeMapTaskGroup;
	Scheduler::GroupHandle						m_LogicActorUpdateGroup;
	Scheduler::GroupHandle						m_ImpactProcessGroup;
	Scheduler::GroupHandle						m_RaytraceGroup;
	Scheduler::GroupHandle						m_PickupSubsystemGroup;
	Scheduler::GroupHandle						m_VehicleAudioGroup;
	Btl::Vector<C_DynamicPointLight>			m_DynamicPointLightVector[4];
	Btl::Vector<C_DynamicSpotLight>				m_DynamicSpotLightVector[4];

	//Lightflare params
	C_LightFlareSettings						m_LevelLightFlareSettings;

	//Car lightflare params
	C_CarLightFlareSettings						m_CarLightFlareSettings;

	//Car Light Settings
	C_CarLightSettings							m_CarLightSettings;

	//Max blobshadows setting
	U32											m_MaxBlobShadowCasters;

	::Actors::C_VisibilityDetector				*m_pVisibilityDetector;

#if PLATFORM_PC
	// [Blur2PC] retail Blur PC (Blur.exe 1.2 game world +0x1ec8): a 1x1 R32F texture holding 0, the display
	// windows' depth texture while Amax.Views.Main.PrepassEnable is off (BeginFrame)
	Display::C_Texture2D						m_WindowDepthTexture;
#endif

	//Shock effect
	Display::C_Texture2D						m_ShockTexture;
	Display::C_Float4							m_ShockIntensity;
	U32											m_ShockCurrentTime;
	S32											m_GameStep; // Number of game steps to do
	F32											m_GameFraction; // Fractional time left over after game steps

	// Timers
	S32											m_GameFrame;

	public:
	Collision::C_CollisionFilter				m_TempBodyCollisionFilter;

	bool										m_RaceLoaded;
	bool										m_cutsceneVideoPlaying;
	bool										m_allowedToStream;
	bool										m_waitForStreamer;
	bool										m_streamerStartedLoading;
	bool										m_streamerFinishedLoading;
	bool										m_startStreamer;
	bool										m_letsGoRacing;

	bool										m_bCubeMapOverridden;

	bool										m_HandleUISceneReady;
	bool										m_paksSwitched;

	Display::C_TextureCube						m_VehicleLightCubeMapTexture[20][2];
	U32											m_NumVehicleLightCubeMapTexture;

	RenderingData::Target::SharedTextureData*	m_pSceneEnvMapTexture;
	const void*									m_pSceneEnvMapGPUTexture;
};

INLINE Game::C_GameActorList* C_GameWorld::GetActorList()
{
	return m_pActorList;
}

INLINE Gravity::C_Scene* C_GameWorld::GetPhysicsScene()
{
	return m_pPhysicsScene;
}

INLINE GameResource::C_LevelSceneManager* C_GameWorld::GetLevelSceneManager()
{
	return m_pLevelSceneManager;
}

INLINE SceneGraph::C_SceneGraph* C_GameWorld::GetSceneGraph()
{
	return m_pSceneGraph;
}

INLINE DynamicEffect::C_Renderer* C_GameWorld::GetDynamicEffectRenderer()
{
	return m_pDynamicEffectRenderer;
}

INLINE DynamicEffect::Light::C_Manager* C_GameWorld::GetLightEffectManager()
{
	return m_pLightEffectManager;
}

INLINE DynamicEffect::Particle::C_ParticleManager* C_GameWorld::GetParticleManager()
{
	return m_pParticleManager;
}

INLINE DynamicEffect::Track::C_TrackRenderer* C_GameWorld::GetTrackRenderer()
{
	return m_pTrackRenderer;
}

INLINE BlobShadows::C_BlobShadows* C_GameWorld::GetBlobShadows()
{
	return m_pBlobShadows;
}

INLINE DropShadows::C_DropShadows* C_GameWorld::GetDropShadows()
{
	return m_pDropShadows;
}

INLINE SplashCamera::C_SplashCamera* C_GameWorld::GetSplashCamera()
{
	return m_pSplashCamera;
}

INLINE Render::Reduced::C_Manager* C_GameWorld::GetReducedRender()
{
	return m_pReducedRender;
}

INLINE AuxFlares::C_AuxFlares* C_GameWorld::GetAuxFlares()
{
	return m_pAuxFlares;
}

INLINE LightTrails::C_LightTrails* C_GameWorld::GetLightTrails()
{
	return m_pLightTrails;
}

INLINE C_LightingPrepass* C_GameWorld::GetLightingPrepass()
{
	return m_pLightingPrepass;
}

INLINE AnimatedCrowd::C_CrowdSystem* C_GameWorld::GetCrowdSystem()
{
	return m_pCrowdSystem;
}

} // namespace Game

#endif // GAME_WORLD_H
