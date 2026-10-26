#ifndef GAMEDEF_RESOURCETYPES_H
#define GAMEDEF_RESOURCETYPES_H



#include <xt/XtGame.h>
#include <../../mercury/library/audio/include/XtAudio.h>
#include <resource_utils/ResourceUtilsDeclare.h>

#include <shared_actors/ActorDef/BasePhysicalObjectDef.h>

namespace DynamicEffect 
{
	namespace Particle 
	{ 
		namespace XtInterface 
		{
			struct Event;
		}
	}
}

namespace Game
{	


//========================  GameModel definition  ========================
//						Matches Schema\ResourceTypes.txt

	struct ModelAnimation
	{
		XtSz8							m_Filename;
	};


	struct ReplayRecord
	{
		XtSz8							m_Filename;
	};


//========================  Dynamic Object definition  ========================
//						Matches Schema\DynamicObjectDef.txt


	enum E_PHYSICS_BEHAVIOUR
	{
		E_PHYSICS_BEHAVIOUR_NONE,
		E_PHYSICS_BEHAVIOUR_FIXED,
		E_PHYSICS_BEHAVIOUR_DYNAMIC,
		E_PHYSICS_BEHAVIOUR_KINEMATIC,
		E_PHYSICS_BEHAVIOUR_POST,
		E_PHYSICS_BEHAVIOUR_VEHICLE,
	};

	enum E_ALIGNMENT_TYPE
	{
		E_ALIGNMENT_TYPE_NONE,
		E_ALIGNMENT_TYPE_VERTICAL,
		E_ALIGNMENT_TYPE_LEFTRIGHT,
		E_ALIGNMENT_TYPE_FORWARDBACK,
	};

	enum E_ANIM_EVENT_TYPE
	{
		E_ANIM_EVENT_TYPE_ONCREATION,
		E_ANIM_EVENT_TYPE_ONDELAYEDSTART,
		E_ANIM_EVENT_TYPE_ONCOLLISION,
		E_ANIM_EVENT_TYPE_ONEVENT1,
		E_ANIM_EVENT_TYPE_ONEVENT2,
		E_ANIM_EVENT_TYPE_ONEVENT3,
		NUM_ANIMATION_EVENT_TYPES
	};

	enum E_AUDIO_MASS_TYPE
	{
		E_AUDIO_MASS_TYPE_NONE,
		E_AUDIO_MASS_TYPE_LIGHT,
		E_AUDIO_MASS_TYPE_MEDIUM,
		E_AUDIO_MASS_TYPE_HEAVY,
		E_AUDIO_MASS_TYPE_VERY_HEAVY,
		E_AUDIO_MASS_TYPE_IMMOVABLE

	};


	struct AnimationEvent
	{
		E_ANIM_EVENT_TYPE				m_EventType;
		bool							m_StopAnimation;
		XtHandle<Game::ModelAnimation>	m_Animation;
//		bool							m_DamagedVersion;
	};


	struct GlobalTime
	{
		F32								m_Time;
	};


	struct DynamicObjectDef
	{
		XtHandle<ResourceUtils::C_GameModel>	m_ModelType;
		bool						m_DisplayShadow;
		XtF32						m_ShadowIntensity;

		XtF32						m_DrawDistance;
		bool						m_DisplayInEnvironment;
		bool						m_DisplayInReflection;
		bool						m_DisplayInMirror;
		bool						m_Bouncy;

		E_PHYSICS_BEHAVIOUR			m_PhysType;
		XtF32						m_Weight;
		XtF32						m_Breakaway;
		XtF32						m_SleepThreshold;
		XtF32						m_LinearDamping;
		XtF32						m_AngularDamping;
		XtF32						m_RangeOfMovement;
		XtF32						m_LifespanAfterDamage;
		XtF32						m_RespawnTimer;
		
		XtHandle<Audio::XtEntityProxy> m_DynamicObjectAudioDef;
		E_AUDIO_MASS_TYPE			m_AudioMassType;

		E_ALIGNMENT_TYPE			m_Alignment;

		XtF32						m_VehicleDamage;

		XtHandle<DynamicObjectDef>	m_DebrisType;

		// Animation events
		bool						m_SwapElementsOnCollision;
		XtArray< AnimationEvent >	m_AnimEvents;
	};


	struct DynamicObjectDefGroup
	{
		XtArray< XtHandle<DynamicObjectDef> > ObjDefList;
	};

	struct PhysicalObjectDefGroup
	{
		XtArray< XtHandle<BasePhysicalObject::C_ElementObjectRecord> > ObjDefList;
	};




//========================  Dynamic Camera definitions  ========================
//						Matches Schema\DynamicCameraDef.txt


	enum E_DYCAM_MOVEMENT
	{
		E_DYCAM_MOVEMENT_NONE,
		E_DYCAM_MOVEMENT_ATTACHED,
		E_DYCAM_MOVEMENT_FLYCAM,
		E_DYCAM_MOVEMENT_SPLINEFOLLOWER,
		E_DYCAM_MOVEMENT_LOCALSPLINEFOLLOWER,
		E_DYCAM_MOVEMENT_VEHICLEFEED,
		E_DYCAM_MOVEMENT_LASTCAMERA,

//		E_DYCAM_MOVEMENT_SIMPLECAMERA					// Always Last - Special behaviour for Simple camera positioning
	};

	enum E_DYCAM_TARGET
	{
		E_DYCAM_TARGET_NONE,
		E_DYCAM_TARGET_SIMPLE,
		E_DYCAM_TARGET_SPLINEFOLLOWER,
		E_DYCAM_TARGET_LOCALSPLINEFOLLOWER,
	};

	enum E_BLUR_TYPE
	{
		E_BLUR_TYPE_MOTIONBLUR,
		E_BLUR_TYPE_DEPTHOFFIELD,
	};

//=============================================================================

	struct DyCamPropertyDef
	{
		XtF32						m_FOV;

		E_BLUR_TYPE					m_BlurType;

		XtF32						m_DepthOfField;
		XtF32						m_Aperture;

		XtF32						m_Blur;

		XtF32						m_DOFNearBlurBegin;
		XtF32						m_DOFNearBlurEnd;
		XtF32						m_DOFFarBlurBegin;
		XtF32						m_DOFFarBlurEnd;
		XtF32						m_DOFFarBlurScale;

		XtF32						m_Colour;
		XtF32						m_Grain;
		XtF32						m_Vignette;
		XtF32						m_Tunnel;

		XtF32						m_RedSaturation;
		XtF32						m_GreenSaturation;
		XtF32						m_BlueSaturation;
	};

	struct DyCamMovementDef
	{
		E_DYCAM_MOVEMENT			m_MovementType;

		XtF32						m_Shake;
		XtF32						m_Lag;
		XtF32						m_TurnTilt;
		XtF32						m_DynPitch;
		bool						m_DoNotLoop;
	};

	struct DyCamTargetDef
	{
		E_DYCAM_TARGET				m_TargetType;

		XtF32						m_Shake;
		XtF32						m_ProximityShake;
		XtF32						m_Lag;
		bool						m_DoNotLoop;
		bool						m_CheckLineOfSight;
	};

	struct DyCamShakeDef
	{
		XtF32						m_X;
		XtF32						m_Y;
		XtF32						m_Z;
		XtF32						m_Freq;
	};

//=============================================================================

	struct RightStickCameraParams
	{
		XtF32						m_PivotDistance;
		XtF32						m_DollyDistance;
		XtF32						m_MaxHorizontalAngle;
		XtF32						m_MaxVerticalAngle;
		XtF32						m_BlendValue;
	};

//=============================================================================

	struct CutSceneLightDef
	{
		XtF32						m_MasterHeading;

		XtRgba						m_Colour0;
		XtF32						m_Wrap0;
		XtF32						m_Heading0;
		XtF32						m_Elevation0;

		XtRgba						m_Colour1;
		XtF32						m_Wrap1;
		XtF32						m_Heading1;
		XtF32						m_Elevation1;

		XtRgba						m_Colour2;
		XtF32						m_Wrap2;
		XtF32						m_Heading2;
		XtF32						m_Elevation2;

		XtRgba						m_Colour3;
		XtF32						m_Wrap3;
		XtF32						m_Heading3;
		XtF32						m_Elevation3;
	};


//========================  Trigger Object Definitions  ========================
//						Matches Schema\Actors\DynamicObjectMiscInfo.txt

	enum E_TRIGGER_TYPE
	{
		E_TRIGGER_TYPE_NORMAL,
		E_TRIGGER_TYPE_GAMELOGIC,
		E_TRIGGER_TYPE_RACERULES,
		E_TRIGGER_TYPE_HANDOFGOD_ENTRY,
		E_TRIGGER_TYPE_HANDOFGOD_EXIT,
		E_TRIGGER_TYPE_HANDOFGOD_TWOWAY,
		E_TRIGGER_TYPE_HANDOFGOD_CAMERA,
		E_TRIGGER_TYPE_ICONLEFT,
		E_TRIGGER_TYPE_ICONRIGHT,
		E_TRIGGER_TYPE_ICONFORK,
		E_TRIGGER_TYPE_TUNNEL_ENTER,
		E_TRIGGER_TYPE_TUNNEL_EXIT,
	};




//========================  Physical Object Definitions  ========================
//						Matches Schema\Actors\??.txt

struct C_ParticleEventSpawn
{
	XtVec3	m_Position;
	XtVec3	m_Direction;
	float	m_Intensity;
	bool	m_LeaveAtSpawn;
	bool	m_SpawnAtHitPosition;
	bool	m_DontUseCarVelocity;
	XtHandle<DynamicEffect::Particle::XtInterface::Event>	m_ParticleEvent;
};

// Element damage level
struct C_PhysicalObjectElementDamageLevelDef : public BasePhysicalObject::C_ElementDamageLevelDef
{	
};

// object element
struct C_PhysicalObjectElementDef : public BasePhysicalObject::C_ElementDef
{
      // Non-Xt stuff
      const C_PhysicalObjectElementDamageLevelDef& DamageLevel(S32 index) const {return *static_cast<C_PhysicalObjectElementDamageLevelDef*>(m_DamageLevels[index]);}
};


// Physical object def.
//
struct C_PhysicalObjectDef : public BasePhysicalObject::C_Def
{
	XtHandle<Audio::XtEntityProxy>		m_AudioEntity;
	XtArray<Game::C_ParticleEventSpawn>	m_BreakParticleEvents;
	float								m_LifeTimeAfterDamage;
	bool								m_DisplayInEnvironment;
	bool								m_DisplayInReflection;
	bool								m_DisplayInMirror;
	bool								m_Bouncy;
	bool								m_UseStaticActivate;
	E_AUDIO_MASS_TYPE					m_AudioMassType;
};






};


#endif
