#include "stdafx.h"

#include "Impact\ContactUID.h"
#include "Impact\CollisionUtil.h"
#include "Impact\Shape.h"
#include "Impact\ConvexHull.h"
#include "Impact\CollisionPair.h"
#include "Impact\Body.h"
#include "Impact\I_TriMesh.h"
#include "Impact\Sphere.h"
#include "Impact\Event.h"

namespace Impact
{

Debug::Bool g_bDeactivation("Impact.Deactivation.Enable", true);
Debug::Float g_fDeactivationTime("Impact.Deactivation.Time", 1.f, Debug::Range(0.f, 10.f));
Debug::Float g_fDeactivationLinearThreshold("Impact.Deactivation.LinearThreshold", 0.01f, Debug::Range(0.f, 0.1f));
Debug::Float g_fDeactivationAngularThreshold("Impact.Deactivation.AngularThreshold", 0.001f, Debug::Range(0.f, 0.1f));
Debug::Int g_iDeactivationSolverThreshold("Impact.Deactivation.SolverThreshold", 30, Debug::Range(0, 100));

using namespace Impact;
using namespace Maths;

Body::Body() :
	m_vPos(0,0,0),
	m_rotation(1,0,0,0),
	m_angularVelocity(0,0,0),
	m_velocity(0,0,0),
	m_iTag(-1),
	m_iIsland(-1),
	m_bStatic(false),
	m_iBroadPhaseID(-1),
	m_iCollisionMask(0xffffffff),

	m_fFriction(0.5f),
	m_pShape(NULL),
	m_fDeactivationTime(0),
	m_bActive(true),
	m_EventFilter(IMPACT_EVENTFILTER_NONE),
	m_bContinuousCollision(false)
{
	m_invMass = 0.0f;
	m_invI = 0.0f;
}

Body::~Body()
{
	if (m_pShape)
		delete m_pShape;
}

/// Set friction coefficient
void Body::SetFriction(float f)
{
	m_fFriction = f;
}

/// set which notification events will be sent for this body
void Body::SetEventFilter(int iFlags)
{
	m_EventFilter = iFlags;
}

/// set the collison shape
void Body::AttachShape(BaseShape *pShape)
{
	m_pShape = pShape->Instance();
	Dirty();
}

/// set the collision mask used for collision filtering
void Body::SetCollisionMask(U32 iMask)
{
	m_iCollisionMask = iMask;
}	

/// force the body to be activated or deactivated
const Maths::HMat43	Body::GetOrientation()
{
	// get cone orientation + position from the attached physics body
	Maths::HMat33 mtx(m_rotation);
	HVec3 vOffset;
	Shape *pShape = GetShape();
	if (pShape->m_type == E_ShapeType_ConvexHull)
		vOffset = ((ConvexHullInstance*)pShape)->GetParent()->GetCenterOfMass();

	return HMat43(mtx, m_vPos + vOffset * mtx);
};

/// returns true if the body is meant to use continuous collision detection
bool Body::ContinuousCollision()
{
	return m_bContinuousCollision;
}

/// switch continuous collision on and off
void Body::SetContinuousCollision(bool bOn)
{
	m_bContinuousCollision = bOn;
}

/// force the body to be activated or deactivated
void Body::SetActive(bool bActive)
{
	if (bActive)
	{
		m_bActive = true;
		m_fDeactivationTime = 0;
	}
	else
	{
		m_bActive = false;
	}
}

void Body::SetMass(float mass)
{
	if (mass == FLT_MAX)
		return SetStatic(true);

	HVec3 dimensions = GetAABB().Diagonal() * HVec3(0.5f);
	m_invMass = 1.0f / mass;
	float I = mass * (dimensions.X() * dimensions.X() + dimensions.Y() * dimensions.Y() + dimensions.Z() * dimensions.Z()) / 12.0f;
	m_invI = 1.0f / I;
}

C_AxisBox Body::GetAABB()
{
	return GetShape()->GetAABB(this);
}

/// get the collision shape for colliding against
Shape *Body::GetShape()
{
	return m_pShape;
}

/// recalculate the activation status
void Body::UpdateActivationStatus(float fTimeDelta)
{
	if (g_bDeactivation &&
		m_velocity.Length2() < (HF32)g_fDeactivationLinearThreshold &&
		m_angularVelocity.Length2() < (HF32)g_fDeactivationAngularThreshold)
	{
		if (IsActive())
		{
			m_fDeactivationTime += fTimeDelta;
			if (m_fDeactivationTime > g_fDeactivationTime)
				m_bActive = false;
		}
	}
	else
	{
		if (!IsActive())
		{
			m_bActive = true;
			m_fDeactivationTime = 0.f;

			// also reactivate all connected collision pairs
			for (int i=0; i<m_Pairs.Size(); i++)
				m_Pairs[i]->m_bActive = true;
		}
	}
}

/// flag this body as 'dirty', meaning that it's position and/or rotation has changed
void Body::Dirty()
{
	m_pShape->UpdateOrientation( m_vPos, m_rotation );
}

/// returns true if the body is currently active
bool Body::IsActive()
{
	return m_bActive;
}

/// set a body to be static or not
void Body::SetStatic(bool bStatic)
{
	m_bStatic = bStatic;
	m_invMass = 0.0f;
	m_invI = 0.0f;
}

/// add a collision pair to the list (called by broadphase)
void Body::AddPair(BodyPair *p)
{
	if (IsStatic())
		return;
	m_Pairs.PushBack(p);
}

/// remove a collision pair from the list (called by broadphase)
void Body::RemovePair(BodyPair *p)
{
	if (IsStatic())
		return;
	for (Ut::Vector<BodyPair*>::Iterator iter=m_Pairs.Begin(); iter!=m_Pairs.End(); ++iter)
	{
		if (*iter == p)
		{
			m_Pairs.Erase(iter);
			return;
		}
	}
	assert(0); // here when we didn't find the given pair
}

}; // namespace Impact
