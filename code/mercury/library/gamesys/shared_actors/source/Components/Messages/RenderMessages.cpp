#include "SharedActorsPch.h"
#include <shared_actors/components/messages/RenderMessages.h>




C_RenderNotifyResourceUsage::C_RenderNotifyResourceUsage()
	: m_WorldCentre()
	, m_WorldRadius(0.0f)
	, m_bHasDetails(false)
	, m_ActorHandle()
	, m_ComponentInterface(-1) 
{
}

C_RenderNotifyResourceUsage::C_RenderNotifyResourceUsage(const Maths::C_Sphere& worldBoundingSphere, const bool bHasDetails, const Actor::C_ActorHandle& actorHandle, const Component::C_ComponentInterfaceId componentInterface)
	: m_WorldCentre()
	, m_WorldRadius(0.0f)
	, m_bHasDetails(bHasDetails)
	, m_ActorHandle(actorHandle)
	, m_ComponentInterface(componentInterface) 
{
	SetWorldBoundingSphere(worldBoundingSphere);
}

C_RenderNotifyResourceUsage::~C_RenderNotifyResourceUsage()
{
}

Maths::C_Sphere C_RenderNotifyResourceUsage::GetWorldBoundingSphere() const
{
	return Maths::C_Sphere(Maths::Load<Maths::C_Vertex>(&m_WorldCentre), m_WorldRadius);
}

void C_RenderNotifyResourceUsage::SetWorldBoundingSphere(const Maths::C_Sphere& worldBoundingSphere)
{
	Maths::Store(worldBoundingSphere.Centre(), &m_WorldCentre);
	m_WorldRadius = worldBoundingSphere.Radius();
}




C_RenderNotifyResourceTotalMsg::C_RenderNotifyResourceTotalMsg()
	: m_WorldPosition()
	, m_uGraphicsBytes(0)
	, m_uPhysicsBytes(0)
{
}

C_RenderNotifyResourceTotalMsg::C_RenderNotifyResourceTotalMsg(const Maths::HVec3& worldPosition, const U32 uGraphicsBytes, const U32 uPhysicsBytes)
	: m_WorldPosition()
	, m_uGraphicsBytes(uGraphicsBytes)
	, m_uPhysicsBytes(uPhysicsBytes)
{
	SetWorldPosition(worldPosition);
}

C_RenderNotifyResourceTotalMsg::~C_RenderNotifyResourceTotalMsg()
{
}

Maths::HVec3 C_RenderNotifyResourceTotalMsg::GetWorldPosition() const
{
	return Maths::Load<Maths::HVec3>(&m_WorldPosition);
}

void C_RenderNotifyResourceTotalMsg::SetWorldPosition(const Maths::HVec3& worldPosition)
{
	Maths::Store(worldPosition, &m_WorldPosition);
}
