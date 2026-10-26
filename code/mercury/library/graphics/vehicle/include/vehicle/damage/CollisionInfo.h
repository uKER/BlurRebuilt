/////////////////////////////////////////////////////////////
// CollisionInfo
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_COLLISION_INFO__
#define __VEHICLE_COLLISION_INFO__

//CA - Sorry I know this is a god awful way of getting at the E_IMPACT_LEVEL enum if there's a better way give me a shout
//#include "../../../../../../../amax/Game/GameLogic/RaceLogicCore/Data/RLEnums.h" 

namespace Vehicle
{

/* 
===============================================================
					class C_VehicleInfo

NOTES: 
===============================================================
*/
class C_VehicleInfo
{
public:
	
	C_VehicleInfo()
	{
		m_Matrix.SetIdentity();

		m_Centre				= Maths::HVec3(0.0f, 0.0f, 0.0f);
		m_Extents				= Maths::HVec3(0.0f, 0.0f, 0.0f);
	}

public:
	
	Maths::HMat43			m_Matrix;							//!< Current position/orientation of the vehicle
	
	Maths::HVec3			m_Centre;							//!< Vehicle centre
	Maths::HVec3			m_Extents;							//!< Vehicle extents

	Maths::HVec3			m_Velocity;
};

/* 
===============================================================
					class C_DamageFilter

NOTES: 
===============================================================
*/
class C_DamageFilter
{
public:

	C_DamageFilter()
	{
		Reset();
	}

	//! Reset to defaults
	void Reset()
	{
		m_FatalDamage			= false;
		m_ScratchDamage			= false;
		m_DeformParts			= false;
		m_DislodgeParts			= false;
		m_DetachParts			= false;
		m_BurstTyres			= false;
		m_DetachWheels			= false;
		m_LightBreak			= false;
		m_LightKnockOut			= false;
		m_RadiatorSmoke			= false;
		m_EngineSmokeLight		= false;
		m_EngineSmokeHeavy		= false;
		m_EngineFire			= false;

		m_CodeDeformAmount		= 1.0f;
		m_ArtDeformAmount		= 1.0f;
	}

public:

	bool				m_FatalDamage;
	bool				m_ScratchDamage;
	bool				m_DeformParts;
	bool				m_DislodgeParts;
	bool				m_DetachParts;
	bool				m_BurstTyres;
	bool				m_DetachWheels;
	bool				m_LightBreak;
	bool				m_LightKnockOut;
	bool				m_RadiatorSmoke;
	bool				m_EngineSmokeLight;
	bool				m_EngineSmokeHeavy;
	bool				m_EngineFire;

	float				m_CodeDeformAmount;
	float				m_ArtDeformAmount;
};

/* 
===============================================================
					class C_ImpactEvent

NOTES: 
===============================================================
*/

class CollisionInfo
{
public:

	CollisionInfo()
	{
		Clear();
	}

	void Clear()
	{
		m_ContactNormal		= Maths::Constants<Maths::HVec3>::Zero();
		m_ImpactVelocity	= Maths::Constants<Maths::HVec3>::Zero();

		m_MaxImpulse			= 0.0f;
		m_PenerationDepth		= 0.0f;
		m_GameplayImpulse		= 0.0f;
		m_GameplayFriction		= 0.0f;
		m_DamageImpulse			= 0.0f;
		m_EffectImpulse			= 0.0f;
		m_AudioImpulse			= 0.0f;
		m_AudioFriction			= 0.0f;
		m_ImpactRadiusOverride	= 0.0f;
		
		m_PhysicsMaterial		= 0;
		m_PhysicsMaterialA		= 0;
		m_PhysicsMaterialB		= 0;
		m_EffectMaterial		= 0;
		m_EffectMaterialA		= 0;
		m_EffectMaterialB		= 0;

		m_DamageStage			= 0;

		m_FatalDamage			= false;
		m_actorIsVisible		= false;
		m_InvalidatePhysicsReaction	= false;
		m_PrimaryVehicle		= false;

		m_ContactPoints.Clear();
	}

public:

	struct ContactPoint
	{
		ContactPoint() : m_ContactPoint(Maths::UNDEFINED), m_LocalContactPoint(Maths::UNDEFINED), m_Friction(Maths::UNDEFINED),
			m_Impulse(Maths::UNDEFINED), m_PtVelA(Maths::UNDEFINED), m_PtVelB(Maths::UNDEFINED)
		{
			/*const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
			m_ContactPoint			= zero;
			m_LocalContactPoint		= zero;
			m_Friction				= zero;
			m_Impulse				= zero;
			m_PtVelA				= zero;
			m_PtVelB				= zero;*/
			m_MassA					= 0.0f;
			m_MassB					= 0.0f;
			m_PenetrationDepth		= 0.0f;
			m_ContactType			= 0;
		}

		Maths::HVec3					m_ContactPoint;
		Maths::HVec3					m_LocalContactPoint;
		Maths::HVec3					m_Friction;
		Maths::HVec3					m_Impulse;
		Maths::HVec3					m_PtVelA;
		Maths::HVec3					m_PtVelB;
		float							m_MassA;
		float							m_MassB;
		float							m_PenetrationDepth;
		U8								m_ContactType;
	};

	Btl::Vector<ContactPoint, Btl::VectorParameters_Static<16> >		m_ContactPoints;
	
	Maths::HVec3					m_ContactNormal;
	Maths::HVec3					m_ImpactVelocity;	
	
	float							m_MaxImpulse;
	float							m_PenerationDepth;
	float							m_ImpactRadiusOverride;

	U16								m_PhysicsMaterial;
	U16								m_PhysicsMaterialA;
	U16								m_PhysicsMaterialB;
	U16								m_EffectMaterial;
	U16								m_EffectMaterialA;
	U16								m_EffectMaterialB;

	float							m_GameplayImpulse; //A value between 0.f and 1.f where 1.f is the heaviest impulse we'll allow and 0.0001f is the merest touch
	float							m_DamageImpulse;
	float							m_EffectImpulse;
	U32								m_DamageStage;

	bool							m_FatalDamage;
	bool							m_PrimaryVehicle;

	float							m_GameplayFriction;
	float							m_AudioImpulse;
	float							m_AudioFriction;
	bool							m_actorIsVisible;
	bool							m_InvalidatePhysicsReaction;
};

class C_ImpactEvent
{
public:

	C_ImpactEvent();
	
	//! Set impact information
	void SetImpactInfo(Maths::HMat43 p_Matrix, Maths::HVec3 p_Position, Maths::HVec3 p_Vector, float p_ImpactSize);

	//! Constructs a collision plane form the contact point info
	void ConstructCollisionPlane(Maths::HVec3 p_Bounds);

	//! Constructs a collision point form the contact point info
	void ConstructCollisionPoint(Maths::HVec3 p_Point, Maths::HVec3 p_Friction, float p_PenetrationDepth);

public:

	CollisionInfo		m_CollisionInfo;

	C_DamageFilter		m_Filter;

	Maths::HMat43		m_Matrix;

	Maths::HVec3		m_LocalPosition;
	Maths::HVec3		m_GlobalPosition;
	Maths::HVec3		m_LocalNormal;
	Maths::HVec3		m_GlobalNormal;
	Maths::HVec3		m_LocalFriction;
	Maths::HVec3		m_GlobalFriction;

	Maths::C_Plane		m_ImpactPlane;

	float				m_ImpactSize;
	float				m_ImpactRadius;
	float				m_FrictionSize;

	bool				m_bProcessParts;
	bool				m_bDeform;
	bool				m_bScrape;
};

} //namespace Vehicle

#endif

