/////////////////////////////////////////////////////////////
// CollisionInfo
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/Damage/CollisionInfo.h"
#include "Vehicle/Component.h"

#include <System/Debug/I_DebugRender.h>

#include <gravity/sdk/Body.h>

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

namespace Vehicle
{

/* 
===============================================================
					class C_ImpactEvent

NOTES: 
===============================================================
*/

/*
===============
NAME: C_ImpactEvent

NOTE: Constructor
===============
*/
	C_ImpactEvent::C_ImpactEvent() : m_LocalPosition(Maths::ZERO_INIT), m_GlobalPosition(Maths::ZERO_INIT),
		m_LocalNormal(Maths::ZERO_INIT), m_GlobalNormal(Maths::ZERO_INIT), m_LocalFriction(Maths::ZERO_INIT),
		m_GlobalFriction(Maths::ZERO_INIT)
{
	m_Matrix.SetIdentity();

	m_ImpactSize		= 0.0f;

	m_CollisionInfo.Clear();

	m_bProcessParts		= false;
	m_bDeform			= false;
	m_bScrape			= false;

	m_ImpactRadius		= 0.0f;
	m_FrictionSize		= 0.0f;
}

/*
===============
NAME: SetImpactInfo

NOTE: Set impact information
===============
*/
void C_ImpactEvent::SetImpactInfo(Maths::HMat43 p_Matrix, Maths::HVec3 p_Position, Maths::HVec3 p_Vector, float p_ImpactSize)
{
	m_GlobalNormal		= p_Vector;
	m_GlobalNormal.Normalise();

	m_GlobalPosition	= p_Position;

	m_LocalPosition		= m_GlobalPosition;
	m_LocalNormal		= m_GlobalNormal;

	m_ImpactSize		= p_ImpactSize;
}

Debug::Bool g_debugClipImpactPosition("Vehicle.Damage.ClipImpactPosition", false);

/*
===============
NAME: ConstructCollisionPlane

NOTE: Constructs a collision plane form the contact point info
===============
*/
void C_ImpactEvent::ConstructCollisionPlane(Maths::HVec3 p_Bounds)
{
	MathsStorage::Float3 position(0.0f, 0.0f, 0.0f);
	for (int iPoint = 0; iPoint < m_CollisionInfo.m_ContactPoints.Size(); iPoint++)
	{
		m_CollisionInfo.m_ContactPoints[iPoint].m_ContactPoint += m_CollisionInfo.m_ContactNormal * m_CollisionInfo.m_ContactPoints[iPoint].m_PenetrationDepth;
		position.x += m_CollisionInfo.m_ContactPoints[iPoint].m_ContactPoint.X();
		position.y += m_CollisionInfo.m_ContactPoints[iPoint].m_ContactPoint.Y();
		position.z += m_CollisionInfo.m_ContactPoints[iPoint].m_ContactPoint.Z();

		m_CollisionInfo.m_ContactPoints[iPoint].m_LocalContactPoint = m_CollisionInfo.m_ContactPoints[iPoint].m_ContactPoint * m_Matrix.InverseRotTrans();
	}
	if (m_CollisionInfo.m_ContactPoints.Size() > 0)
	{
		position.x = position.x / (float)m_CollisionInfo.m_ContactPoints.Size();
		position.y = position.y / (float)m_CollisionInfo.m_ContactPoints.Size();
		position.z = position.z / (float)m_CollisionInfo.m_ContactPoints.Size();
	}
	
	MathsStorage::Float3 normal(m_CollisionInfo.m_ContactNormal.X(),
								m_CollisionInfo.m_ContactNormal.Y(),
								m_CollisionInfo.m_ContactNormal.Z());
	
	m_GlobalPosition	= Maths::Load<Maths::HVec3>(&position);
	m_GlobalNormal		= Maths::Load<Maths::HVec3>(&normal);

	m_LocalPosition		= m_GlobalPosition * m_Matrix.InverseRotTrans();

	if (g_debugClipImpactPosition)
	{
		float px = m_LocalPosition.X() * 10.0f;
		float py = m_LocalPosition.Y() * 10.0f;
		float pz = m_LocalPosition.Z() * 10.0f;

		Maths::C_Vertex impactVertex(px, py, pz);
		Maths::C_AxisBox axisBox(	Maths::HVec3(-p_Bounds.X(), -p_Bounds.Y(), -p_Bounds.Z()),
									Maths::HVec3(p_Bounds.X(), p_Bounds.Y(), p_Bounds.Z()));
		impactVertex = Maths::Clip(impactVertex, axisBox);
		m_LocalPosition = Maths::HVec3(impactVertex.X(), impactVertex.Y(), impactVertex.Z());
		m_GlobalPosition = m_LocalPosition * m_Matrix;
	}

	m_LocalNormal		= (m_GlobalPosition + m_GlobalNormal) * m_Matrix.InverseRotTrans();
	m_LocalNormal		= m_LocalNormal - m_LocalPosition;
	m_LocalNormal.Normalise();

	normal = MathsStorage::Float3(	m_LocalNormal.X(),
									m_LocalNormal.Y(),
									m_LocalNormal.Z());
	position = MathsStorage::Float3(m_LocalPosition.X(),
									m_LocalPosition.Y(),
									m_LocalPosition.Z());

	m_ImpactPlane = Maths::C_Plane(	Maths::Load<Maths::C_Normal>(&normal),
									Maths::Load<Maths::C_Vertex>(&position));
}

/*
===============
NAME: ConstructCollisionPoint

NOTE: Constructs a collision point form the contact point info
===============
*/
void C_ImpactEvent::ConstructCollisionPoint(Maths::HVec3 p_Point, Maths::HVec3 p_Friction, float p_PenetrationDepth)
{
	m_GlobalPosition	= p_Point;
	m_GlobalPosition	+= m_CollisionInfo.m_ContactNormal * p_PenetrationDepth;
	m_GlobalNormal		= m_CollisionInfo.m_ContactNormal;


	m_LocalPosition		= m_GlobalPosition * m_Matrix.InverseRotTrans();

	m_CollisionInfo.m_ContactPoints.Clear();
	m_CollisionInfo.m_ContactPoints.PushBack();
	m_CollisionInfo.m_ContactPoints.Back().m_LocalContactPoint = m_LocalPosition;

	m_LocalNormal		= (m_GlobalPosition + m_GlobalNormal) * m_Matrix.InverseRotTrans();
	m_LocalNormal		= m_LocalNormal - m_LocalPosition;
	m_LocalNormal.Normalise();

	m_FrictionSize		= p_Friction.Length();
	m_GlobalFriction	= p_Friction;
	m_GlobalFriction.Normalise();
	m_LocalFriction		= m_GlobalFriction * m_Matrix.Rotation().Inverse();
}

} //namespace Vehicle

