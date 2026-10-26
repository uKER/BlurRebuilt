 /////////////////////////////////////////////////////////////
// VehicleActor
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_ACTOR_H__
#define __VEHICLE_ACTOR_H__

#include "VehicleActorEnums.h"
#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <Game/GameDef/UserData.h>
#include <shared_actors/XtActorBase.h>

#include <Resource/Resource.h>
#include <game_resource/ModelResource.h>

#include "VehicleControlType.h"
#include "VehiclePickupEffect.h"

#include <RacingPhysics/Vehicles/Vehicle.h>
#include "VehiclePerformanceModifiers.h"
#include "VehiclePickupEffect.h"

#include <Graphics/Effects/VehicleWheel.h>
#include <Graphics/Effects/ExhaustEffect.h>
#include <Graphics/Effects/ParticleTrail.h>
#include <Dynamic/Node.h>

#include <GameLogic/Messaging/GameLogicMessages.h>

#include <misc_shared/Types/FP32.h>

#include <vehicle/effects/Lights.h>

#include "game/AmaxCam/VFCamera.h"

#include <game/GameUtil/ResourceBagDeclare.h>
#include <Game/GameUtil/ResourceBagListener.h>
 
#include <Vehicle/Component.h>

#include <Game/GameWorld/RenderPass.h> //Used by ChangeModelRenderPass();

#include <GameLogic/GameData/GameVehicleData.h>
#include <GameLogic\Structures\VehicleAlphaComponentType.h>

//#define _WHEEL_LOD

#ifdef _WHEEL_LOD
#include <Vehicle/Lod/LodSwitchNode.h>
#endif

class C_DriverManager;
namespace Actors { class C_VisibleDetect; }
namespace Vehicle { class C_ModelInput; }
namespace Vehicle { class C_VehicleModel; }
namespace Vehicle { class C_Part; }
namespace Vehicle { class C_DamageState; }
namespace Game
{
	class C_GameWorld;
	class C_ViewManager;
	class C_SendControlsToVehicle;
	class C_VehicleImpactMsg;
	class C_LevelState;
	struct LevelStateCar;
	class C_LevelStateMultiplayerVehicleAbilities;
}
namespace Gravity 
{ 
	class C_KinematicController; 
	class C_TriggerActor;
}
namespace SpecialFX { class I_SpecialFXInstance; class C_SpecialFX; }
namespace GameResource
{
	class C_Model;
	class C_ModelElement;
}
namespace WorldRender { class C_ModelInstance; }
namespace RacingPhysics 
{	
	class Car; 
	class Vehicle;
	class CarControls;
}

namespace AmaxCam
{
	class C_PhysicsChecks;
	class C_VehicleFeedParam;
}

namespace BlobShadows
{
	class C_DynamicBlobShadowInstance;
}
namespace DynamicEffect
{
	namespace Light
	{
		class C_Manager;
	}

	namespace Particle { namespace XtInterface { struct Event; } }
}
namespace Kanzen
{
	struct HollywoodVehicleParams;
}

namespace LightTrails
{
	class C_LightTrailInstance;
}

enum E_ResetType
{
	E_ResetType_None = -1,

	E_ResetType_Position,
	E_ResetType_Spawn,
	E_ResetType_Camera,
};

namespace Cam2
{
	class C_Camera;
}

namespace Game
{

class C_VehicleParticleEmitter
{
public:

	C_VehicleParticleEmitter();

	void Create(DynamicEffect::C_Node* p_pLocalSpace, float p_Intensity);

	void Setup(Maths::HVec3 p_Position, Maths::HVec3 p_Direction, XtHandle<DynamicEffect::Particle::XtInterface::Event> p_EventIdle, XtHandle<DynamicEffect::Particle::XtInterface::Event> p_EventSpeed);

	void Destroy();

	void Shutdown();

	void Update(DynamicEffect::C_Node* p_pLocalSpace, const Maths::C_RGBA_F32& p_Lighting, float p_Quality, float p_Speed, Maths::HVec3 p_Velocity, U32 visibilityFlags);

public:

	struct Emitter
	{
		Maths::HVec3									m_Position;
		Maths::C_Quaternion								m_Orientation;
		DynamicEffect::Particle::C_EventInstanceHandle	m_ParticleEventIdle;
		DynamicEffect::Particle::C_EventInstanceHandle	m_ParticleEventSpeed;
		XtHandle<DynamicEffect::Particle::XtInterface::Event>	m_XtEventIdle;
		XtHandle<DynamicEffect::Particle::XtInterface::Event>	m_XtEventSpeed;
	};

	float											m_Intensity;

	bool											m_Activate;
	bool											m_Deactivate;
	bool											m_Created;
	
	Btl::Vector<Emitter>							m_Emitters;	
};

namespace Actors
{
class RaceLogicCore;

class C_BodyShake
{
public:

	C_BodyShake();

	void Update(Maths::HMat43& p_Matrix, float p_fTimeStep);

	void Start();

	void Extend(float p_fDuration);

public:

	bool							m_bShaking;
	
	MathsStorage::Float3			m_vRange;
	float							m_fTime;
	float							m_fDuration;
	float							m_fFrequency;
	float							m_fScale;
	FP32							m_fNoise;
	
};

class LandingDetection
{
public:
							LandingDetection();
				void		Reset();
				void		Update(const C_VehicleActor& vehicleActor, F32 timeElapsed);
				F32			GetLandedImpact() const;
				void		HandleImpact(const C_VehicleActor& vehicleActor);
				bool		IsAirbourne() const;
private:

	static const int		LANDING_MAX_WHEELS = 4;
	static const F32		MIN_TIME_OFF_GROUND_BEFORE_AIRBOURNE;
	static const F32		YVELOCITY_FOR_MAX_IMPACT;
	static const F32		YDISTANCE_FOR_MAX_IMPACT;
	static const F32		XZDISTANCE_FOR_MAX_IMPACT;
	static const F32		AIRBOURNE_TIME_FOR_MAX_IMPACT;
	static const F32		LANDING_IMPACT_LOWER_DOT;
	static const F32		LANDING_IMPACT_UPPER_DOT;
	static const F32		IMPACT_SCALE_FACTOR;


				Maths::HVec3 m_prevVelocity;
				Maths::HVec3 m_PositionAtTakeOff;
				F32			m_WheelOffGroundTime[LANDING_MAX_WHEELS];
				F32			m_AirbourneTime;
				F32			m_LandedImpact;
				F32			m_MaxYDistance;
				F32			m_MaxYVelocity;
				bool		m_Airbourne;
				bool		m_Impact;
};

class TumblingDetection
{
public:
							TumblingDetection();
				void		Reset();
				void		Update(const C_VehicleActor& vehicleActor, F32 timeElapsed);
				bool		GetTumbling() const;

	static const int		TUMBLING_MAX_WHEELS = 4;
	static const F32		MIN_TIME_OFF_GROUND_BEFORE_TUMBLING;
	static const F32		UPSIDE_DOWN_DOT;
	static const F32		TUMBLING_REGAINED_CONTROL_ANG_VELOCITY_MAG;
private:
		
				F32			m_WheelOffGroundTime[TUMBLING_MAX_WHEELS];
				bool		m_tumbling;
				
};

/* 
===============================================================
					class C_VehicleActor

NOTES: 
===============================================================
*/
class C_VehicleActorCreate : public ::Actors::C_PosActorCreate
{
public:

	C_VehicleActorCreate()
	{
		m_ModelFilename		= "";
		m_WheelFilename		= "";
		m_DynamicsFilename	= "";
		m_pGameWorld		= NULL;
		m_XtIndex			= 0;
		m_XtPaintJobId		= 0;
		m_XtResprayJobId    = 0;
		m_ResprayColour		= 0;
		m_VehicleIndex		= 0;
		m_ControlType		= ::E_VEHICLE_CONTROL_TYPE_PLAYER;
		m_pRLC				= 0;
		m_Dormant			= false;
		m_HasLogicActor		= false;
	}

	static Actor::C_ActorTypeId					ClassType() { return E_ActorTypeId_Vehicle; }
	const Actor::C_ActorTypeId					GetType() const { return E_ActorTypeId_Vehicle; }

	Util::C_String								m_ModelFilename;
	Util::C_String								m_WheelFilename;
	Util::C_String								m_DynamicsFilename;

	Game::C_GameWorld*							m_pGameWorld;
	
	U32											m_XtIndex;

	XtId										m_XtPaintJobId;
	U32											m_XtResprayJobId;
	U32											m_ResprayColour;

	U8											m_VehicleIndex;
	bool										m_Dormant;
	bool										m_HasLogicActor;

	enum ::E_VEHICLE_CONTROL_TYPE				m_ControlType;
	Game::Actors::RaceLogicCore					*m_pRLC;
};

/* 
===============================================================
					class C_VehicleActor

NOTES: 
===============================================================
*/


enum E_FakeVehicleSpeedEffect
{
	E_FAKE_SPEED_NONE,
	E_FAKE_SPEED_ACCELERATE,
	E_FAKE_SPEED_BRAKE
};

struct FakeVehiclePhysicsState
{
	E_FakeVehiclePhysicsEffect	_nextEffect;

public:
	E_FakeVehiclePhysicsEffect	_effect;
	float						_value;
	float						_recoveryTime;
	int							_numConsecutiveHits;

public:
	float						_stickToGround;

public:
	FakeVehiclePhysicsState();

public:
	void Start(E_FakeVehiclePhysicsEffect effect);
	void Stop();

public:
	void Update(class C_VehicleActor* self, float dt, SharedPhysics::C_RigidBody& rb);

public:
	bool IsActive() const;
	bool IsInterruptable() const;

public:
	void IncrementConsecutiveHits();

public:
	void ApplyForce(SharedPhysics::C_RigidBody& rb, const Debug::VectorStruct3& kickForce, bool flip);
};

struct FakeVehicleSpeedState
{
	E_FakeVehicleSpeedEffect	_nextEffect;
	float						_nextTarget;

public:
	E_FakeVehicleSpeedEffect	_effect;
	float						_target;
	float						_value;
	float						_slowdownRate;

public:
	FakeVehicleSpeedState();

public:
	void Start(E_FakeVehicleSpeedEffect effect, float target, float rate = 0.f);
	void Stop();

public:
	void Update(class C_VehicleActor* self, float dt, SharedPhysics::C_RigidBody& rb);

public:
	bool IsActive() const;
};

class C_VehicleActor : public C_GameActor, public C_ResourceBagListener
{
public:

	C_VehicleActor();
	~C_VehicleActor();

	const Maths::HMat43 &						GetMatrix() const {return m_Matrix;}
#if PLATFORM_PC
	// [Blur2PC] render interpolation: m_Matrix blended between the last two update ticks while a frame
	// renders (the pose the car's model is drawn at), m_Matrix otherwise. For render-time readers such
	// as the HUD gadgets drawn in the world around the car.
	Maths::HMat43								Blur2PCRenderMatrix() const;
private:
	struct S_Blur2PCTickMatrix { Maths::HMat43 prev, cur; U32 tick; S_Blur2PCTickMatrix() : tick(0) {} } m_Blur2PCMatrix;
	// [Blur2PC] light flares, aux flares and the rear point light hold world positions computed from m_Matrix
	// during the update; this render hook moves them to the render pose for each frame and back afterwards
	static void								Blur2PCRenderHook(void* ctx, bool begin);
	void									Blur2PCPlaceLights(const Maths::HMat43& matrix, bool begin);
	Btl::Vector<MathsStorage::Float4A16>		m_Blur2PCSavedFlarePos;
	bool									m_Blur2PCHookAdded;
	bool									m_Blur2PCLightsMoved;
public:
#endif
	const Maths::HMat43 &						GetRawMatrix() const {return m_RawMatrix;}
	const Maths::HVec3	&						GetVelocity() const {return m_Velocity;}
	void										SetMatrix(Maths::HMat43);

	static const Actor::C_ActorTypeId			TypeId()	{ return E_ActorTypeId_Vehicle; }
	static const Btl::String32					TypeName()	{ return "Vehicle"; }

	//! Update
	void										Update();
	void										PausedUpdate();
	void										PostUpdate();
	void										UpdateBlobShadow(const Cam2::C_Camera& camera);
	void										UpdateBlobShadow();
	void										SanityCheckPosition();
	void										DoRayTraces();
	void										Update_Debug();
	void										BodyShake();
	void										UpdateGripLoss();
	void										AddSubscribers();
	void										CheckCamera(bool force = false);
	void										UpdateScreenSFX();
	void										HandleConstraintFreezing();

	Game::Effects::E_MiscParticleEventType		GetLandingEffectType(S32 p_SurfaceType);
	void										CheckForLandingEffects();
	void										UpdateSmallImpactEffects(F32 elapsedTime);
	void										UpdateRoughTerrainEffects(F32 elapsedTime);
	void										UpdateHighSpeedEffects(F32 elapsedTime);
	void										HandleDefferedEvents();
	void										HandleCollisionStates();

	//! Spawns a particle effect
	void										SpawnParticleEffect(Game::Effects::E_MiscParticleEventType p_EventType, 
																	Maths::HVec3 p_Position, Maths::HVec3 p_Direction, Maths::HVec3 p_Velocity, 																				
																	float p_Scale, 
																	DynamicEffect::C_Node* p_pLocalSpace,
																	Vehicle::C_Part* p_pPart = NULL, 
																	DynamicEffect::Particle::C_EventInstanceHandle* p_pHandle = NULL);

	virtual bool								OnMessage(const Messaging::C_MessageBase *pMessage);

	virtual const char*							GetActorName() const;

	//! Get pointer to vehicle model
	Vehicle::C_VehicleModel*					GetVehicleModel() const { return m_pVehicleModel; }
	void										SetVehicleModel(Vehicle::C_VehicleModel* p_pVehicleModel) { m_pVehicleModel = p_pVehicleModel; }

	//! Get pointer to vehicle physics in the racing physics
	RacingPhysics::Car*							GetVehiclePhysics() const { return m_pVehiclePhysics; }
	void										SetVehiclePhysics(RacingPhysics::Car* p_pVehiclePhysics) { m_pVehiclePhysics = p_pVehiclePhysics; }

	//! Get the group ID
	const S32									GetPhysicalGroupID() const { return m_PhysicsGroupId; };

	
	//! Return vehicle input
	Vehicle::C_ModelInput*						GetVehicleInput() const;

	const bool									GetVehicleControlsEnabled() const;
	const int									GetVehicleIndex() const;
 
	void										SetVisibleHealthDamageEnabled( bool enabled )  { m_VisibleHealthDamageEnabled = enabled; }
	//! Syncs racing physics data with model data - done in a thread-safe happy place
	void										SynchData(Maths::HMat43* p_pMatrix, const Maths::HVec3& photoOffset);

	void										ResourceNotify(Resource::C_ResourceBase* p_pResource, const Resource::E_Notify p_Result, void* p_pData);

	void										SetDrawDistance(WorldRender::C_ModelInstance* p_pModelInstance, const Util::C_String p_Name);

	void										SetupDirtLayer();

	bool										Create();

	//Shadow Mesh
	void										FindShadowMesh();
	void										CreateShadowMesh( const Maths::HVec3*	shadowPositionVector, U32 shadowPositionSize, const U32* shadowIndexVector, U32 shadowIndexSize);
	BlobShadows::C_DynamicBlobShadowInstance*	GetBlobShadowInstance() { return m_pBlobShadowInstance;};

	bool										IsVisible();
	bool										IsEnabled() const { return m_Enabled; }
	bool										IsCollisionEnabled() const;
	//Light CubeMaps
		void									UpdateLightCubeMap(const Cam2::C_Camera& camera);
	Display::C_TextureCube*						GetCurrentLightCubeMap();
	bool										NeedUpdateLightCubeMap();

	//Dynamic SH light data
	void										UpdateSHLightData(const F32 additiveSHLightData[16]);

	//! Create vehicle physics body
	int											CreateCollisionMesh(const GameResource::C_Model* p_pPhysicsModel, const LevelStateCar& levelStateCar);
	
	//! Create a wheel instance
	void										CreateWheelInstance(const GameResource::C_ModelElement* p_pModelElement);

	//! Create bone
	void										CreateBones(const GameResource::C_Model* p_pModel);

	void										Destroy();

	//! Shutsdown vehicle, but doesn't destroy everything
	void										Shutdown();

	void										KillImpl();

	//! Sets vehicle matrix and updates parts
	void										SetMatrices(const Maths::HMat43 newWorldMatrix);

	//! Draw debug hud
	void										DrawDebugHud();

	void										DisplayDebug();

	//! Handle vehicle collisions
	void										HandleCollision(const Game::C_ImpactMsg* p_pMsg);

	//Test for tunneling and recover from it!
	void										HandleTunneling();
	
	//Update the constraint on the car!
	void										UpdateConstraint(F32 time);
	//change the air firction coefficient for the car when it is on the air
	void										UpdateVehicleAirFrictionCoefficient(const LevelStateCar& levelStateCar);

	AmaxCam::C_PhysicsChecks*					GetPhysicsChecks() const { return m_pPhysicsChecks; }
	void										GetNetworkControlsAndState(RacingPhysics::Vehicle::CompressedNetworkControls* controls, RacingPhysics::Vehicle::CompressedNetworkState* state, const C_VehicleActor* pTarget = 0);
	Gravity::C_Body*							GetCollisionBody() const { return m_pCollisionBody; }
	Game::Actors::RaceLogicCore					*GetRLC() const;

	void										SetFlightControl(float p_fRoll, float p_fPitch) { m_fRoll = p_fRoll; m_fPitch = p_fPitch; }
	void										UpdateFlightControl();
	void										UpdateDownForce();

	void										SetLandedEffect(const SpecialFX::C_SpecialFX* p_pScreenEffect);
	void										SetImpactEffect(const SpecialFX::C_SpecialFX* p_pScreenEffect,const Maths::HMat33& transform, bool immediate = false);
	void										SetHealthEffect(const SpecialFX::C_SpecialFX* p_pHealthEffect);

	Maths::HVec3								GetGroundPosition() const { return m_GroundPosition; }
	Maths::C_Normal								GetGroundNormal() const { return m_GroundNormal; }

	//! Spawns impact effects
	void										SpawnImpactEffects(	U32 p_SurfaceType, 
																	Maths::HVec3 p_Position, 
																	Maths::HVec3 p_Direction, 
																	Maths::HVec3 p_Velocity, 
																	float p_Intensity);

	//! Sets the rendering/physics status of the vehicle
	void SetVehicleStatus(bool p_Status);

	void AIApplyTorquesForces();

	bool										RenderShadowInMirror();

	const AmaxCam::C_VehicleFeedParam*			GetCameraParam() const;

	AmaxVehicleWeight							GetWeightClass() const { return m_WeightClass; }

	//!	Replay stuff
	float										GetReplayTimestep() const { return m_ReplayUpdateTimeStep; };

	void										SetCollisions(bool p_Status);
	void										SetState(bool p_Status);

	void										CheckForPotentialCollisions(Btl::Vector<int>& vehiclesUnderneath);

	static void									SetDeferredRaytracePolicy(int iPolicy);

	void										AudioUpdate() const;
	void										DeferredUpdate();
	void										GetDamageState(Vehicle::C_DamageState& p_DamageState) const;
	void										SetDamageState(const Vehicle::C_DamageState& p_DamageState);

	Scheduler::GroupHandle						GetSchedulerGroup() { return m_hUpdateSchedulerGroup; }

	static void									SetAudioSchedulerGroup(Scheduler::GroupHandle handle);
	static Scheduler::GroupHandle				GetAudioSchedulerGroup() { return m_hAudioSchedulerGroup; }

	static void									UpdateAudio();

	static void									SetRaytraceGroup(Scheduler::GroupHandle handle);
	static Scheduler::GroupHandle				GetRaytraceGroup() { return m_hRaytraceGroup; }

//	void										SetProxyActor(C_GameActor *proxy)	{ m_pProxyActor = proxy; };						// Set ptr to vehicle proxy used in cutscenes
	void										SetProxyActor(Actor::C_ActorHandle proxy)	{ m_pProxyActor = proxy; };						// Set ptr to vehicle proxy used in cutscenes

	const Btl::Vector<Maths::HVec3>&			GetCollisionCacheVertices() const { return m_CollisionCacheVertices; }

	::Actors::C_VisibleDetect*					GetVisibleDetect() const { return m_pVisDetect; }
	F32											GetScreenSize() const { return m_ScreenSize; }
	bool										IsVisibleInMainView() const;
	F32											GetAudioRPMLimit() const { return m_AudioRPMLimit;}
	F32											GetAudioGarageID() const { return m_AudioGarageID;}
	bool										GetAudioForcedInduction() const { return m_AudioForcedInduction;}
	F32											GetAudioEngineType() const { return m_AudioEngineType;}
	F32											GetAudioExhaustType() const { return m_AudioExhaustType;}
	void										PhotoMoveCar(const Maths::HVec3& offset);
	enum ::E_VEHICLE_CONTROL_TYPE				GetControlType()const { return m_ControlType;}

	void										HaltAndRemoveFromRaceLogic();	//	Called when vehicle is wrecked/eliminated/leaves a MP race. Clears/disables controls and excludes from position calculations.
	void										RemoveFromRace();				//	Shuts off updates and stops rendering vehicle/driver etc.
	void										RemoveModelFromSceneGraph();
	void										DisableHeadlights();

	void										RepairForFinish();

//	Vehicle Proxy Utility Functions
	void										Damage_EnableDisable(bool recieveDamage);
	void										Damage_Apply(S32 damagestate);
	void										Damage_Repair();
/*
	void										SetVehicleHandbrake(bool applyHandbrake);
	void										SetVehicleSpeed(F32 speed);
	void										SetVehicleRevs(F32 revs);
	void										SetVehicleBrake(F32 brakeValue);
	void										SetVehicleSteering(F32 steerValue);
*/
	void										ResetInitialStates();
	void										SetDoorPosition(F32 leftDoorState, F32 rightDoorState);
	void										SetWindowPosition(F32 leftWindowState, F32 rightWindowState);
	void										SetBonnetPosition(F32 bonnetState);
	void										SetLightState(bool leftIndicator, bool rightIndicator, bool headLights, bool brakeLights, bool reverseLights);
	void										DisplayVehicle(bool displayVehicle, bool fromMessage);
	void										DisplayDriver(bool displayDriver, S32 viewportIndex = -1, bool fromMessage = false);
	void										CreateVehicleEffects();
	void										SwapVehicleModel();

	float										GetChaseCamHeightBase() const;

	void										Freeze();
	void										Resume();

	Kanzen::HollywoodVehicleParams				GetHollywoodCameraParams(S32 debugIndex = 0) const;

	Maths::HVec3								GetExtents() const;
//	Maths::HVec3								GetCentre() { return m_pVehicleModel->GetModelInput()->m_vCentre; }

	void										SetOverideControls(RacingPhysics::CarControls& controls);

	bool										CanIgnoreCollision() const { return m_CanPhysicsIgnoreCollision; }
	// Fake physics
	FakeVehiclePhysicsState*					GetFakePhysics() { return &m_FakeVehiclePhysics; }
	const FakeVehiclePhysicsState*				GetFakePhysics() const { return &m_FakeVehiclePhysics; }
	FakeVehicleSpeedState*						GetFakeSpeed() { return &m_FakeVehicleSpeed; }
	const FakeVehicleSpeedState*				GetFakeSpeed() const { return &m_FakeVehicleSpeed; }

	bool							m_corneringAssist;
	bool							m_brakeAssist;

	void										SetLinearVelocityFromSpeed(float speed);
	const bool									LostGrip() const { return m_bLostGrip; }

	bool										IsFocusVehicle() const { return m_IsFocusVehicle; }
	S32											GetViewportID() const { return m_ViewportID; } 

	void										SetVisibility(bool p_Visible, S32 viewportIndex = -1, bool force = false, bool forceTransition = true);
	bool										GetVisibility() const { return m_Visible != 0; }

	void										PlayWreckedEffect();
	void										SetLostControl(bool LostControl) {m_LostControl=LostControl;}
	void										SetFatallyDamaged(bool fatallyDamaged) {m_ConstraintFatallyDamaged=fatallyDamaged;}
	void										SetHeavyCollision(bool heavyCollision) {m_HeavyCollision=heavyCollision;}
	bool										GetLostControl() const { return m_LostControl; }
	U32											GetXtIndex() const { return m_XtIndex; }

	S32											GetFramesInWater() const {return m_FramesInWater;};
	bool										InWater() const { return (m_FramesInWater > 0); }
	bool										IsTumbling() const { return m_tumbling.GetTumbling(); }
	bool										IsAirbourne() const { return m_landing.IsAirbourne(); }
	AmaxCam::E_VFCamera							GetCameraType(int viewport = 0) const;

	bool										IsEliminated() const { return m_Eliminated; };
	
	bool										Created() const { return m_Created; }

	void										RegisterWaterCollision(const MathsStorage::Float3& p_Position);

	void										OnlyUnloadResources() { m_OnlyUnloadResources = true; }

	bool										HasLogicActor() { return m_HasLogicActor; }

	void										SetEvictDelay(S32 p_EvictDelay);

	Maths::HMat43								LastKnownSafeMatrix() const { return m_LastKnownSafeMatrix; }

	bool										HasGoneOffTrack() const { return m_hasGoneOffTrack; }
	void										SetGoneOffTrack(bool val) { m_hasGoneOffTrack = val; }
	void										ApplyWreckedImpulse(F32 Scale = 1.0f);
		
	// Record the actual controller input for the player as they get overidden at various times...ie AI.
	float										m_accelControllerInput;
	float										m_brakeControllerInput;
	float										m_handbrakeControllerInput;

	Collision::C_CollisionFilter				m_BodyCollisionFilter;
	Collision::C_CollisionFilter				m_ObjectCollisionFilter;
	Collision::C_CollisionFilter				m_VehicleCollisionFilter;

#ifndef CFG_GOLD
	Maths::HVec3								m_LastKnownGroundPosition;
#endif

	void										SetAudioOcclusionValue(U32 value){ m_AudioOcclusionValue = value;}
	U32											GetAudioOcclusionValue() { return m_AudioOcclusionValue;}

	const float									GetAlpha(	const enum E_VAC_TYPE eVehicleAlphaComponentType, 
															const enum E_VAC_TARGET_TYPE eVehicleAlphaComponentTargetType = E_VAC_TARGET_TYPE_CURRENT) const;
	
	void										SetAlpha(	const enum E_VAC_TYPE eVehicleAlphaComponentType, 
															const float alpha,
															const enum E_VAC_TARGET_TYPE eVehicleAlphaComponentTargetType = E_VAC_TARGET_TYPE_CURRENT);

	const bool									Ghosted() const;

	void										Ghost(const float duration = 0.0f, const float targetAlpha = 0.f);
	void										Deghost(const float duration = 0.0f, const float targetAlpha = 1.f);

	void										SetAlphaTransitionCurve(const Curve::C_HermiteCurve *pAlphaTransitionCurve = NULL);

	void										ChangeModelRenderPass(const enum Game::E_RenderPass eRenderPass);

	void										SetVisibilityOverride(bool p_OverrideVisibility) { m_VisibilityOverride = p_OverrideVisibility; }

	void										TriggerExhaustPop(Game::Actors::C_VehicleExhaustPopMsg* p_pMsg);

	void										EnableVehicle();
	void										DisableVehicle();

	void										SetWheelParticleLod(bool p_Wheel0Enable, bool p_Wheel1Enable, bool p_Wheel2Enable, bool p_Wheel3Enable, 
																	float p_Wheel0Quality, float p_Wheel1Quality, float p_Wheel2Quality, float p_Wheel3Quality);

	void										ForceAIControl(bool bChangeDynamics = true);
	void										ForcePlayerControl(bool bChangeDynamics = true);

	void										SetSpawnMatrices(Maths::HMat43& p_Matrix) { m_Matrix = m_RawMatrix = m_BirthPlace = m_LastKnownResetMatrix = m_LastKnownSafeMatrix = p_Matrix; }

protected:

	virtual bool								InitialiseImpl();
	virtual bool								CreateImpl();
	bool										PrimeImpl() OVERRIDE;

	void										OnResourceBagLoaded(const C_ResourceBag& bag) OVERRIDE;

private:

	//! Resets vehicle actor	
	void										Reset(const enum E_ResetType eResetType = E_ResetType_Position, const Maths::HMat43 newWorldMatrix = Maths::HMat43(), Maths::HVec3 velocity = Maths::HVec3(0.0f,0.0f,0.0f), bool networkJump = false);
	void										ResetVehicleAlphaComponents();

	bool										HandleHoGForceResetArea(const Messaging::C_MessageBase *pMessage);

	void										CreateAIDriver();
	void										ApplyPerformanceModifiers(RacingPhysics::CarControls* pControls);
	

	void										UpdatePickupSFX(const float frameTime);
	void										AddPickupSFX(const VehiclePickupEffect newPickupSFX);

	void										UpdateAlphaTransitions(const float frameTime);
	void										UpdateAlpha(const float alpha);
	void										UpdateComponentAlpha(const float alpha, const enum E_VAC_TYPE eVehicleAlphaComponentType);

	void										AddAuxFlare(Vehicle::C_Part* pNode, Maths::HMat33 rotation, Maths::HVec3 offset, const F32 size, F32 intensity, Vehicle::E_AuxFlareColour flareColour);
	
	//void										CreateRadiatorSmoke();
	void										CreateEngineSmokeLight();
	void										CreateEngineSmokeHeavy();
	//void										CreateEngineFire();
	void										UpdateDamageEffects(const Maths::C_RGBA_F32& lighting);
	void										UpdateHeadlights();
	void										UpdateLightTrails(const Maths::C_RGBA_F32& lighting);
	void										UpdateLightFlares();
	void										UpdateDynamicLights();
	void										UpdateAuxFlares();
	void										UpdateSizeOnScreen();

	F32											CheckRoadSweeperMultiplier(Game::C_LevelState& levelState) const;

	void										CheckOverideControls(RacingPhysics::CarControls& controls);

	void										NetworkTests(); // debug

	void										FireDamageEvent(Maths::HVec3 p_ImpactPosition, Maths::HVec3 p_ImpactVector);

	const float									CalculateAlphaValue(const float lerpValue, const enum E_VAC_TYPE eVehicleAlphaComponentType = E_VAC_TYPE_VEHICLE) const;

	Maths::HMat43								m_Matrix;
	Maths::C_Quaternion							m_RotationQuaternion;
	Maths::HMat43								m_RawMatrix;
	Maths::HVec3								m_Velocity;
public:
	Maths::HMat43								m_BirthPlace;
private:
	Maths::HMat43								m_LastKnownResetMatrix;
	Maths::HMat43								m_LastKnownSafeMatrix;
	Maths::HVec3								m_GroundPosition;
	Maths::C_Normal								m_GroundNormal;
	Maths::HVec3								m_AITorque;
	Maths::HVec3								m_AILocalForce;
	Maths::HVec3								m_Centre;
public:
	Maths::HVec3								m_PrevPosition;						//Previous position for tunneling checks!
private:
	SharedPhysics::C_Constraint*				m_pConstraint;
	F32											m_GroundedTimer;

	Vehicle::C_VehicleModel*					m_pVehicleModel;					//!< Render/game model
	Btl::Vector<MathsStorage::MatrixRow44*>		m_WorldMatrices;
#if PLATFORM_PC
	Btl::Vector<const void*>					m_Blur2PCWorldMatrixOwners;			//!< [Blur2PC] model instance per m_WorldMatrices entry (render interpolation)
#endif
	
	RacingPhysics::Car*							m_pVehiclePhysics;					//!< Physics model
	AmaxCam::C_PhysicsChecks*					m_pPhysicsChecks;

	Gravity::C_Body*							m_pCollisionBody;
	Gravity::C_TriggerActor*					m_pPickupTriggerActor;
	Gravity::C_TriggerActor*					m_pTriggerRegionTriggerActor;

	C_DriverManager*							m_pDriverManager;

	S32											m_ObjectColliderIndex;
	S32											m_CarColliderIndex;
	S32											m_MainElementIndex;

	C_ResourceBagHandle							m_hResourceBag;
	GameResource::C_ModelHandle					m_ModelHandle;
	GameResource::C_ModelResource*				m_pModelResource;
	GameResource::C_ModelResource*				m_pVehicleModelResource;
	GameResource::C_ModelResource*				m_pWheelModelResource;
#ifdef _WHEEL_LOD
	GameResource::C_ModelResource*				m_pWheelLodModelResource;	
#endif
	GameResource::C_ModelResource*				m_pAuxFlareModelResource;

	static Scheduler::GroupHandle				m_hAudioSchedulerGroup;
	static Scheduler::GroupHandle				m_hRaytraceGroup;
	Scheduler::GroupHandle						m_hUpdateSchedulerGroup;

	Game::C_GameWorld*							m_pGameWorld;
	DynamicEffect::Light::C_Manager*			m_pLightEffectManager;
	const Display::C_Texture3D*					m_pHeadlightTexture;
	C_ViewManager*								m_pViewManager;
	
	Util::C_String								m_ModelFileName;
	Util::C_String								m_WheelFileName;
	Util::C_String								m_DynamicsFileName;

	S32											m_VehicleModelResourceIndex;
	S32											m_WheelModelResourceIndex;
#ifdef _WHEEL_LOD
	S32											m_WheelLodModelResourceIndex;
#endif
	S32											m_AuxFlareResourceIndex;
	S32											m_VehicleIndex;
	S32											m_PhysicsGroupId;
	S32											m_FramesInWater;

	U32											m_XtIndex;
	U32											m_HealthPoints;

	F32											m_fRoll;
	F32											m_fPitch;

	F32											m_ReplayUpdateTimeStep;			//	How often (in seconds) the replay system will record the vehicle's state

	bool										m_LoadSeparateWheelModel, 
												m_RaceStarted,
												m_IsFocusVehicle,
												m_IsSplitscreen,
												m_Created,
												m_Enabled,
												m_CollisionsEnabled,
												m_ControlsEnabled,
												m_OnlyUnloadResources,
												m_ResourcesUnloaded,
												m_HasLogicActor,
												m_IsBumperCamera,
												m_EnableParticles,
												m_EnableDeferredEventParticles,
												m_hasGoneOffTrack;
	S32											m_ViewportID; //-1 if not in focus
	U32											m_EffectsVisiblityFlags;

	enum ::E_VEHICLE_CONTROL_TYPE				m_ControlType;
//#ifdef NOT_USING_AUDIO_COMPONENETS
//	//faux audio component for now
//	Game::VehicleAudioComponent					m_vehicleAudio;
//#endif

	VehiclePerformanceModifiers					m_PerformanceModifiers;

//	C_VehicleParticleEmitter					m_RadiatorSmokeEmitter;
	C_VehicleParticleEmitter					m_EngineSmokeLightEmitter;
	C_VehicleParticleEmitter					m_EngineSmokeHeavyEmitter;
	//C_VehicleParticleEmitter					m_EngineFireEmitter;

	bool										m_LostControl;
	bool										m_FatallyDamaged;
	bool										m_ConstraintFatallyDamaged;
	bool										m_HeavyCollision;
	bool										m_ResetifyDriver;
	
	float										m_ParticleEmitScale;
	float										m_ParticleLifeScale;
	float										m_ParticleReducedEmitScaleTimer;

	float										m_fSmallImpactEffectValue;
	float										m_fSmallImpactTimeSinceImpact;
	float										m_fSmallImpactAccumulator;	
	float										m_fRoughTerrainPercentage;
	SpecialFX::I_SpecialFXInstance*				m_pRoughTerrainSpecialFX;
	float										m_fHighSpeedPercentage;
	SpecialFX::I_SpecialFXInstance*				m_pHighSpeedSpecialFX;
	float										m_fLandedEffectTime;
	SpecialFX::I_SpecialFXInstance*				m_pLandedSpecialFX;
	float										m_fImpactEffectTime;
	SpecialFX::I_SpecialFXInstance*				m_pImpactSpecialFX;
	float										m_fHealthEffectTime;
	SpecialFX::I_SpecialFXInstance*				m_pHealthSpecialFX;

	const SpecialFX::C_SpecialFX*				m_pPendingImpactEffect;
	Maths::HMat33								m_PendingImpactTransform;

	bool										m_Eliminated;
	U32											m_Visible;
	bool										m_VisibilityOverride;
	//bool										m_VisuallyGhosted;
	bool										m_VisibleHealthDamageEnabled;
	
	Btl::Vector<VehiclePickupEffect>			mPickupSFXArray;

	Maths::HMat33								m_OriginalIBodyMatrix;

	Btl::Vector<Effects::C_VehicleWheel>		m_WheelEffects;
	F32											m_ScreenSize;
	Effects::C_ExhaustEffect					m_ExhaustEffects;
	Vehicle::C_VehicleWindow					m_Window;

	SceneGraph::C_SpotLightHandle				m_HeadlightHandle;
	SceneGraph::C_PointLightHandle				m_HeroLightHandle;
	SceneGraph::C_PointLightHandle				m_RearLightHandle;
	::Actors::C_VisibleDetect*					m_pVisDetect;
	::Actors::C_VisibleDetect*					m_pReflectionVisDetect;
	

	DynamicEffect::C_Node						m_LocalSpace;
	DynamicEffect::C_Node						m_LocalSpacePosition;
	DynamicEffect::C_Node						m_LocalSpacePositionWater;
	float										m_FrameTime;
	float										m_fAirTime;
	float										m_fLostGripTime;
	float										m_fCantLoseGripTime;
	float										m_KillPlaneHeight;

	float										m_fMaxWheelTravelFront;
	float										m_fMaxWheelTravelRear;
	float										m_fRearAxlePivotAngle;

	bool										m_bLostGrip;
	bool										m_bCanLoseGrip;

	Collision::C_Ray							*m_pGroundRay;
	Collision::C_Ray							*m_pCollisionRay;
	static int									m_iDeferredRaytracePolicy;

	LandingDetection							m_landing;
	TumblingDetection							m_tumbling;

	//Btl::Vector<Game::Effects::C_ParticleTrail>	m_ParticleTrails;

	struct C_VehicleLightTrail
	{
		Maths::HVec3						m_Position;
		Vehicle::E_PartCode					m_PartCode;
		LightTrails::C_LightTrailInstance*	m_pLightTrailInstance;
	};
	Btl::Vector<C_VehicleLightTrail>		m_LightTrails;

	C_LevelStateMultiplayerVehicleAbilities*	m_pAbilities;
	const Curve::C_HermiteCurve					*m_pAlphaTransitionCurve;

#ifdef _WHEEL_LOD
	Btl::Vector<Vehicle::C_LodSwitchNode>		m_LodNodes;
#endif

	struct EmpState
	{
		float	m_fEmpMaxTime;
		float	m_fEmpTime;
		float	m_fBrake;
		bool	m_bEmp;
		bool	m_bHeadLights;
		bool	m_bRearLights;
		bool	m_bFogLights;
	};
	EmpState m_EmpState;

	//Shadow
	BlobShadows::C_DynamicBlobShadowInstance*	m_pBlobShadowInstance;

	Display::C_VertexBuffer m_ShadowVertexBuffer;
	U32						m_ShadowVertexCount;
	Display::C_IndexBuffer  m_ShadowIndexBuffer;
	U32						m_ShadowIndexCount;

//	C_GameActor									*m_pProxyActor;
	Actor::C_ActorHandle						m_pProxyActor;
	F32											m_AudioRPMLimit;
	F32											m_AudioGarageID;
	bool										m_AudioForcedInduction;
	F32											m_AudioEngineType;
	F32											m_AudioExhaustType;

	Btl::Vector<Maths::HVec3> m_CollisionCacheVertices;
	Btl::Vector<WorldRender::C_ModelInstance*> m_ModelInstances;

	//Light cubemap
	U32						m_CurrentLightCubeMapIndex;
	Display::C_TextureCube*	m_LightCubeMapTexture[2];
	bool					m_NeedUpdateLightCubeMap;
	U32						m_UpdateLightCubeMapFrecuency;
	U32						m_UpdateLightCubeMapCount;

	//Need to defer HoG reset messages due to threaded update
	Game::Actors::C_VehicleResetAtWorldPositionMsg		m_ResetOnTrackMsg;
	E_ResetType											m_DeferredResetType;

	friend class C_VehicleLogicActor;
	bool m_bCatchingAir;
	bool m_bInAirDownForceEngaged;
	float m_fGroundedDownForceTime;
	float m_fAirDownForceTime;

	float							m_GhostTransitionTime;
	float							m_GhostTransitionDuration;

	bool							m_tempHaveNewVehicleToVehicleMesh;

	// Cutscene controller overrides
	bool							m_overide_controls;
	RacingPhysics::CarControls		m_new_controls;

	Gravity::C_ExternalConstants	m_GravityConstants;

	bool							m_FreezeConstrainedParts;
	bool							m_CanPhysicsIgnoreCollision;
	bool							m_CollisionThisFrame;
	int								m_FramesSinceCollision;
	FakeVehiclePhysicsState m_FakeVehiclePhysics;
	FakeVehicleSpeedState m_FakeVehicleSpeed;


	Game::C_BodyUserDataActor		m_UserData;
	
	C_BodyShake						m_BodyShake;
	C_BodyShake						m_BodyShakeDamage;

	Vehicle::C_Part*				m_pDashBoardNode;

	DynamicEffect::Particle::C_EventInstanceHandle	m_WaterBobParticleEffect;

	U32								m_AudioOcclusionValue;

	AmaxVehicleWeight				m_WeightClass;

	//Used to store alpha value for each type of alpha component as part of the fix 
	//for the mess that is ghost/secret ghost/silent running/network silent running
	float							m_ComponentAlpha[E_VAC_TARGET_TYPE_MAX][E_VAC_TYPE_MAX];

	Btl::Vector<Btl::Pair<bool, float>, Btl::VectorParameters_Array<4> > m_WheelParticleState;
};

void DeferredVehicleUpdate(Scheduler::Task* pTask);
void DeferredVehicleAudioUpdate(Scheduler::Task* pTask);

} //namespace Actors

} //namespace Game

#endif
