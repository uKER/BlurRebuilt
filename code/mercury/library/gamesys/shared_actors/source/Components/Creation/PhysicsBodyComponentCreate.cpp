
#include "SharedActorsPch.h"
#include <misc_shared/Context/ContextualStream.h>
#include <shared_actors/Components/Creation/PhysicsBodyComponentCreate.h>


namespace Components
{
;
C_PhysicsBodyCreateInfo::C_PhysicsBodyCreateInfo()
	: I_GameComponentCreator()
	, m_Matrix()
	, m_CentreOfMassOffset()
	, m_CollisionFilter()
	, m_Deactivator()
	, m_BuoyancyCoeffs()
	, m_BodyType(ActorPhysics::E_BodyType_Invalid)
	, m_OverrideCollisionFilterIndex(-1)
	, m_CollisionInfiniteMassRatio(-1.0f)
	, m_OverrideBodyFlagsAndMask(false)
	, m_ContinuousCollision(false)
	, m_IsConveyorBelt(false)
	, m_bFluidEvents(false)
	, m_MinCollisionImpulse(0.0f)
	, m_ContactEventLevel(E_ContactEventLevel_All)
{
	// Set default deactivator vals:
	m_Deactivator.m_sSleepCountdown = 15;
	m_Deactivator.m_ActivateThresholdSq = 0.01f;
	m_Deactivator.m_StaticActivateThresholdSq = -1.0f;
	m_Deactivator.m_SleepThresholdSq = 0.001f;
}

C_PhysicsBodyCreateInfo::~C_PhysicsBodyCreateInfo()
{
}

void C_PhysicsBodyCreateInfo::Serialise(Sys::C_ContextualStream& stream) const
{
	I_GameComponentCreator::Serialise(stream);
	stream << m_Matrix;
	stream << m_CentreOfMassOffset;
	stream << m_CollisionFilter;	
	stream << m_Deactivator.m_sSleepCountdown << m_Deactivator.m_ActivateThresholdSq << m_Deactivator.m_StaticActivateThresholdSq << m_Deactivator.m_SleepThresholdSq;
	stream << m_BuoyancyCoeffs;
	stream.WriteAs<S32>(m_BodyType);
	stream << m_OverrideCollisionFilterIndex;
	stream << m_CollisionInfiniteMassRatio;
	stream << m_OverrideBodyFlagsAndMask;
	stream << m_ContinuousCollision;
	stream << m_IsConveyorBelt;
	stream << m_bFluidEvents;
	stream << m_MinCollisionImpulse;
	stream.WriteAs<U8>(m_ContactEventLevel);
}


void C_PhysicsBodyCreateInfo::Deserialise(Sys::C_ContextualStream& stream)
{
	I_GameComponentCreator::Deserialise(stream);
	stream >> m_Matrix;
	stream >> m_CentreOfMassOffset;
	stream >> m_CollisionFilter;
	stream >> m_Deactivator.m_sSleepCountdown >> m_Deactivator.m_ActivateThresholdSq >> m_Deactivator.m_StaticActivateThresholdSq >> m_Deactivator.m_SleepThresholdSq;
	stream >> m_BuoyancyCoeffs;
	stream.ReadAs<S32>(m_BodyType);
	stream >> m_OverrideCollisionFilterIndex;
	stream >> m_CollisionInfiniteMassRatio;
	stream >> m_OverrideBodyFlagsAndMask;
	stream >> m_ContinuousCollision;
	stream >> m_IsConveyorBelt;
	stream >> m_bFluidEvents;
	stream >> m_MinCollisionImpulse;
	stream.ReadAs<U8>(m_ContactEventLevel);
}

}//namespace Components
