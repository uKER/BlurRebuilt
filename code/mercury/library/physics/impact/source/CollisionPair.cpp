#include "stdafx.h"

#include "Impact\Body.h"
#include "Impact\ContactUID.h"
#include "Impact\PersistentData.h"
#include "Impact\Collision.h"
#include "Impact\PersistentData.h"
#include "Impact\CollisionPair.h"

using namespace Maths;

namespace Impact
{

/// construct using two bodies
CollisionPair::CollisionPair(Body *p1, Body *p2) :
	BodyPair(p1, p2),
	m_pPersistentData(NULL)
{
	m_type = E_PairType_Collision;
	m_fFriction = sqrtf(p1->m_fFriction * p2->m_fFriction);
#ifdef SINGLE_FRICTION_MANIFOLD
	m_Friction.m_fAccTangentImpulseX = 0;
	m_Friction.m_fAccTangentImpulseY = 0;
	m_Friction.m_fAccTwistImpulse = 0;
#endif	
}

/// destructor
CollisionPair::~CollisionPair()
{
	FreePersistentData();
}

/// free this collision pair's persistent collision data
void CollisionPair::FreePersistentData()
{
	if (m_pPersistentData)
	{
		delete m_pPersistentData;
		m_pPersistentData = NULL;
	}
}

// set contacts
void CollisionPair::UpdateContacts(Contact *newContacts, int nNewContacts)
{
	Ut::UtStackVector<Contact,MAX_CONTACTS> mergedContacts;
	mergedContacts.Resize(nNewContacts);

	int nContacts = m_Contacts.Size();

	// for each new contact
	for (int i=0; i<nNewContacts; i++)
	{
		Contact *pMerged = &mergedContacts[i];

		// see if it already exists
		for (int j=0; true; j++)
		{
			// this is a new contact, add it to the end of the list
			if (j>=nContacts)
			{
				*pMerged = newContacts[i];
				pMerged->m_fAccNormalImpulse = 0;
#ifdef TWO_FRICTION_AXIS
				pMerged->m_fAccTangentImpulseX = 0;
				pMerged->m_fAccTangentImpulseY = 0;
#endif
				break;
			}

			// merge the new contact with the existing contact's accumulated impulses
			Contact *pNew = &newContacts[i];
			if (pNew->m_uid == m_Contacts[j].m_uid)
			{
				Contact *pOld = &m_Contacts[j];
				*pMerged = *pNew;
				pMerged->m_fAccNormalImpulse = pOld->m_fAccNormalImpulse;
#ifdef TWO_FRICTION_AXIS
				pMerged->m_fAccTangentImpulseX = pOld->m_fAccTangentImpulseX;
				pMerged->m_fAccTangentImpulseY = pOld->m_fAccTangentImpulseY;
#endif
				break;
			}
		}
	}

	// copy merged contacts into the CollisionPair
	m_Contacts.Resize(nNewContacts);
	for (int i = 0; i < nNewContacts; ++i)
	{
		m_Contacts[i] = mergedContacts[i];
		m_Contacts[i].m_fAccNormalImpulse = mergedContacts[i].m_fAccNormalImpulse;
#ifdef TWO_FRICTION_AXIS
		m_Contacts[i].m_fAccTangentImpulseX = mergedContacts[i].m_fAccTangentImpulseX;
		m_Contacts[i].m_fAccTangentImpulseY = mergedContacts[i].m_fAccTangentImpulseY;
#endif
	}
}

#if defined(TWO_FRICTION_AXIS)

const static HVec3 g_TangentVec[2] = { HVec3(1,0,0), HVec3(0,0,1) };

//
void CollisionPair::PreStep(float inv_dt)
{
	const HF32 k_allowedPenetration = 0.01f;

	int n = m_Contacts.Size();
	for (int i = 0; i < n; i++)
	{
		Contact *c = &m_Contacts[i];
		HVec3 r1 = c->m_vPos - m_pBody[0]->m_vPos;
		HVec3 r2 = c->m_vPos - m_pBody[1]->m_vPos;

		// Precompute normal mass, tangent mass, and bias.
		HF32 rn1 = r1 | c->m_vNormal;
		HF32 rn2 = r2 | c->m_vNormal;
		HF32 kNormal = m_pBody[0]->m_invMass + m_pBody[1]->m_invMass;
		kNormal += m_pBody[0]->m_invI * ((r1 | r1) - rn1 * rn1) + m_pBody[1]->m_invI * ((r2 | r2) - rn2 * rn2);
		c->m_invkNormal = 1.0f / kNormal;

		// Relative velocity at contact
		HVec3 dv = m_pBody[1]->m_velocity + (m_pBody[1]->m_angularVelocity ^ r2) - m_pBody[0]->m_velocity - (m_pBody[0]->m_angularVelocity ^ r1);

		HVec3 tangent[2] = {
			(c->m_vNormal ^ g_TangentVec[0]),
			(c->m_vNormal ^ g_TangentVec[1])
		};
		HF32 rt1[2] = {	(r1 | tangent[0]), (r1 | tangent[1]) };
		HF32 rt2[2] = { (r2 | tangent[0]), (r2 | tangent[1]) };

		HF32 kTangent[2]={Maths::UNDEFINED,Maths::UNDEFINED};
		kTangent[0] = kTangent[1] = m_pBody[0]->m_invMass + m_pBody[1]->m_invMass;
		kTangent[0] += m_pBody[0]->m_invI * ((r1 | r1) - rt1[0] * rt1[0]) + m_pBody[1]->m_invI * ((r2 | r2) - rt2[0] * rt2[0]);
		kTangent[1] += m_pBody[0]->m_invI * ((r1 | r1) - rt1[1] * rt1[1]) + m_pBody[1]->m_invI * ((r2 | r2) - rt2[1] * rt2[1]);
		c->m_invkTangent[0] = 1.0f /  kTangent[0];
		c->m_invkTangent[1] = 1.0f /  kTangent[1];

		c->m_fBias = -0.1f * inv_dt * Min(0.0f, c->m_fSeparation + k_allowedPenetration);

		// Apply normal + friction impulse
		HVec3 impulse = c->m_fAccNormalImpulse * c->m_vNormal
			+ (HF32)c->m_fAccTangentImpulseX * tangent[0]
			+ (HF32)c->m_fAccTangentImpulseY * tangent[1];

		m_pBody[0]->m_velocity -= (HF32)m_pBody[0]->m_invMass * impulse;
		m_pBody[0]->m_angularVelocity -= (HF32)m_pBody[0]->m_invI * (r1 ^ impulse);

		m_pBody[1]->m_velocity += (HF32)m_pBody[1]->m_invMass * impulse;
		m_pBody[1]->m_angularVelocity += (HF32)m_pBody[1]->m_invI * (r2 ^ impulse);
	}
}

//
void CollisionPair::Step()
{
	Body *pBody1 = m_pBody[0];
	Body *pBody2 = m_pBody[1];

	int n = m_Contacts.Size();
	for (int i=0; i<n; i++)
	{
		Contact *c = &m_Contacts[i];
		// position of the current contact relative to each body's center of mass
		HVec3 r1 = c->m_vPos - pBody1->m_vPos;
		HVec3 r2 = c->m_vPos - pBody2->m_vPos;

		// Relative velocity at contact
		HVec3 dv = pBody2->m_velocity + (pBody2->m_angularVelocity ^ r2) - pBody1->m_velocity - (pBody1->m_angularVelocity ^ r1);

		// Compute normal impulse with bias.
		HF32 vn = (dv | c->m_vNormal);
		HF32 fImpulse = c->m_invkNormal * (-vn + c->m_fBias);

		// Clamp the accumulated impulse
		HF32 fOld = c->m_fAccNormalImpulse;
		c->m_fAccNormalImpulse = Max(fOld + fImpulse, 0.0f);
		fImpulse = c->m_fAccNormalImpulse - fOld;

		// Apply contact impulse
		HVec3 impulse = fImpulse * c->m_vNormal;

		pBody1->m_velocity -= (HF32)pBody1->m_invMass * impulse;
		pBody1->m_angularVelocity -= (HF32)pBody1->m_invI * (r1 ^ impulse);

		pBody2->m_velocity += (HF32)pBody2->m_invMass * impulse;
		pBody2->m_angularVelocity += (HF32)pBody2->m_invI * (r2 ^ impulse);

		// Relative velocity at contact
		dv = pBody2->m_velocity + (pBody2->m_angularVelocity ^ r2) - pBody1->m_velocity - (pBody1->m_angularVelocity ^ r1);

		// Compute friction impulse
		HF32 maxTangentImpulse = m_fFriction * c->m_fAccNormalImpulse;

		HVec3 tangent[2] = {
			(c->m_vNormal ^ g_TangentVec[0]),
			(c->m_vNormal ^ g_TangentVec[1])
		};
		HF32 vt[2] = {
			(dv | tangent[0]),
			(dv | tangent[1])
		};
		HF32 tangentImpulse[2] = {
			c->m_invkTangent[0] * (-vt[0]),
			c->m_invkTangent[1] * (-vt[1])
		};

		// Clamp friction
		HF32 oldTangentImpulse = c->m_fAccTangentImpulseX;
		c->m_fAccTangentImpulseX = Min(Max(oldTangentImpulse + tangentImpulse[0], -maxTangentImpulse), maxTangentImpulse);
		tangentImpulse[0] = c->m_fAccTangentImpulseX - oldTangentImpulse;

		oldTangentImpulse = c->m_fAccTangentImpulseY;
		c->m_fAccTangentImpulseY = Min(Max(oldTangentImpulse + tangentImpulse[1], -maxTangentImpulse), maxTangentImpulse);
		tangentImpulse[1] = c->m_fAccTangentImpulseY - oldTangentImpulse;

		// Apply contact impulse
		impulse = tangentImpulse[0] * tangent[0] + tangentImpulse[1] * tangent[1];

		pBody1->m_velocity -= (HF32)pBody1->m_invMass * impulse;
		pBody1->m_angularVelocity -= (HF32)pBody1->m_invI * (r1 ^ impulse);

		pBody2->m_velocity += (HF32)pBody2->m_invMass * impulse;
		pBody2->m_angularVelocity += (HF32)pBody2->m_invI * (r2 ^ impulse);
	}
}

#elif defined(SINGLE_FRICTION_MANIFOLD)

const static HVec3 g_TangentVec[2] = { HVec3(1,0,0), HVec3(0,0,1) };

//
void CollisionPair::PreStep(float inv_dt)
{
	const float k_allowedPenetration = 0.04f;

	HVec3 vAvgNormal(0,0,0);
	HVec3 vAvgPos(0,0,0);
	float fSumNormal = 0;

	for (int i = 0; i < m_Contacts.Size(); i++)
	{
		Contact *c = &m_Contacts[i];
		HVec3 r1 = c->m_vPos - m_pBody[0]->m_vPos;
		HVec3 r2 = c->m_vPos - m_pBody[1]->m_vPos;

		vAvgPos += c->m_vPos;
		vAvgNormal += c->m_vNormal;
		fSumNormal += fabs(c->m_fAccNormalImpulse);

		// precalculate 1/_kt_
		float rn1 = r1 | c->m_vNormal;
		float rn2 = r2 | c->m_vNormal;
		float kNormal = m_pBody[0]->m_invMass + m_pBody[1]->m_invMass;
		kNormal += m_pBody[0]->m_invI * ((r1 | r1) - rn1 * rn1) + m_pBody[1]->m_invI * ((r2 | r2) - rn2 * rn2);
		c->m_invkNormal = 1.0f / kNormal;

		// precalculate bias
		c->m_fBias = -0.1f * inv_dt * Min(0.0f, c->m_fSeparation + k_allowedPenetration);

		// Apply accumulated normal impulse
		HVec3 impulse = c->m_fAccNormalImpulse * c->m_vNormal;

		m_pBody[0]->m_velocity -= m_pBody[0]->m_invMass * impulse;
		m_pBody[0]->m_angularVelocity -= m_pBody[0]->m_invI * (r1 ^ impulse);

		m_pBody[1]->m_velocity += m_pBody[1]->m_invMass * impulse;
		m_pBody[1]->m_angularVelocity += m_pBody[1]->m_invI * (r2 ^ impulse);
	}

	if (m_Contacts.Size() <= 0)
		return;

	// calculate friction coefficients - 
	// friction manifold center approximation (average of positions)
	// TODO: properly calculate the center of the manifold
	float f = 1.f/(float)m_Contacts.Size();
	vAvgPos *= f;
	vAvgNormal *= f;

	HVec3 r1 = vAvgPos - m_pBody[0]->m_vPos;
	HVec3 r2 = vAvgPos - m_pBody[1]->m_vPos;

	HVec3 tangent[2];
	// tangent axis
	tangent[0] = (vAvgNormal ^ g_TangentVec[0]);
	tangent[1] = (vAvgNormal ^ g_TangentVec[1]);
	float rt1[2], rt2[2];

	// precalculate 1/_kt_ to be used by _ApplyImpulse_
	rt1[0] = (r1 | tangent[0]); rt2[0] = (r2 | tangent[0]);
	rt1[1] = (r1 | tangent[1]); rt2[1] = (r2 | tangent[1]);
	float kTangent[2];
	kTangent[0] = kTangent[1] = m_pBody[0]->m_invMass + m_pBody[1]->m_invMass;
	kTangent[0] += m_pBody[0]->m_invI * ((r1 | r1) - rt1[0] * rt1[0]) + m_pBody[1]->m_invI * ((r2 | r2) - rt2[0] * rt2[0]);
	kTangent[1] += m_pBody[0]->m_invI * ((r1 | r1) - rt1[1] * rt1[1]) + m_pBody[1]->m_invI * ((r2 | r2) - rt2[1] * rt2[1]);
	m_Friction.m_invkTangent[0] = 1.0f / kTangent[0];
	m_Friction.m_invkTangent[1] = 1.0f / kTangent[1];

	m_Friction.m_vTangent[0] = tangent[0];
	m_Friction.m_vTangent[1] = tangent[1];
	m_Friction.m_vPosition = vAvgPos;
	m_Friction.m_fLimit = m_fFriction * fSumNormal;
	m_Friction.m_vNormal = vAvgNormal;

	// Apply accumulated friction impulse
	HVec3 impulse = m_Friction.m_fAccTangentImpulseX * tangent[0] +
					m_Friction.m_fAccTangentImpulseY * tangent[1];

	m_pBody[0]->m_velocity -= m_pBody[0]->m_invMass * impulse;
	m_pBody[1]->m_velocity += m_pBody[1]->m_invMass * impulse;

	// apply accumulated twist friction impulse
	impulse = m_Friction.m_vNormal * m_Friction.m_fAccTwistImpulse;

	m_pBody[0]->m_angularVelocity -= m_pBody[0]->m_invI * impulse;
	m_pBody[1]->m_angularVelocity += m_pBody[1]->m_invI * impulse;
}

//
void CollisionPair::Step()
{
	Body *pBody1 = m_pBody[0];
	Body *pBody2 = m_pBody[1];

	HVec3	nbody1_pos(-pBody1->m_vPos),
			nbody2_pos(-pBody2->m_vPos),
			body1_vel(pBody1->m_velocity),
			body2_vel(pBody2->m_velocity),
			body1_avel(pBody1->m_angularVelocity),
			body2_avel(pBody2->m_angularVelocity);

	for (int i = 0; i < m_Contacts.Size(); ++i)
	{
		Contact *c = &m_Contacts[i];
		HVec3 normal = c->m_vNormal;

		// position of the current contact relative to each body's center of mass
		HVec3 r1 = c->m_vPos + nbody1_pos;
		HVec3 r2 = c->m_vPos + nbody2_pos;

		// negative relative velocity at contact
		HVec3 nrv = body1_vel - body2_vel + (body1_avel ^ r1) + (r2 ^ body2_avel);

		// calculate normal impulse + bias
		HF32 fImpulse = c->m_invkNormal * ((nrv | normal) + c->m_fBias);

		// clamp accumulated normal impulse
		HF32 fOld = c->m_fAccNormalImpulse;
		c->m_fAccNormalImpulse = Max(fOld + fImpulse, 0.0f);
		fImpulse = c->m_fAccNormalImpulse - fOld;

		// apply normal impulse
		HVec3 impulse = fImpulse * normal;

		body1_vel -= pBody1->m_invMass * impulse;
		body1_avel += pBody1->m_invI * (impulse ^ r1);

		body2_vel += pBody2->m_invMass * impulse;
		body2_avel += pBody2->m_invI * (r2 ^ impulse);
	}

	if (m_Contacts.Size() <= 0)
		return;

	HVec3 r1 = m_Friction.m_vPosition + nbody1_pos;
	HVec3 r2 = m_Friction.m_vPosition + nbody2_pos;

	// Relative velocity at friction center
	HVec3 dv = body2_vel + (body2_avel ^ r2) - body1_vel - (body1_avel ^ r1);

	// Compute friction impulse
	float maxTangentImpulse = m_Friction.m_fLimit;

	HVec3 tangent[2];
	tangent[0] = m_Friction.m_vTangent[0];
	tangent[1] = m_Friction.m_vTangent[1];
	float vt[2];
	vt[0] = (dv | tangent[0]);
	vt[1] = (dv | tangent[1]);
	float tangentImpulse[2];
	tangentImpulse[0] = m_Friction.m_invkTangent[0] * (-vt[0]);
	tangentImpulse[1] = m_Friction.m_invkTangent[1] * (-vt[1]);		

	// Clamp friction
	float oldTangentImpulse = m_Friction.m_fAccTangentImpulseX;
	m_Friction.m_fAccTangentImpulseX = Min(Max(oldTangentImpulse + tangentImpulse[0], -maxTangentImpulse), maxTangentImpulse);
	tangentImpulse[0] = m_Friction.m_fAccTangentImpulseX - oldTangentImpulse;

	oldTangentImpulse = m_Friction.m_fAccTangentImpulseY;
	m_Friction.m_fAccTangentImpulseY = Min(Max(oldTangentImpulse + tangentImpulse[1], -maxTangentImpulse), maxTangentImpulse);
	tangentImpulse[1] = m_Friction.m_fAccTangentImpulseY - oldTangentImpulse;

	// Apply friction impulse
	HVec3 impulse = tangentImpulse[0] * tangent[0] + tangentImpulse[1] * tangent[1];

	body1_vel -= pBody1->m_invMass * impulse;
	body2_vel += pBody2->m_invMass * impulse;

	// compute twist friction impulse
	float fTwistImpulse = (body1_avel | m_Friction.m_vNormal) - (body2_avel | m_Friction.m_vNormal);
	fTwistImpulse = fTwistImpulse * 0.01f;
	// TODO: need to work out the scale of the twist friction impulse here. but don't know how

	float old = m_Friction.m_fAccTwistImpulse;
	m_Friction.m_fAccTwistImpulse = Min(Max(old + fTwistImpulse, -maxTangentImpulse), maxTangentImpulse);
	fTwistImpulse = m_Friction.m_fAccTwistImpulse - old;

	// apply twist friction impulse
	impulse = m_Friction.m_vNormal * fTwistImpulse;

	body1_avel -= pBody1->m_invI * impulse;
	body2_avel += pBody2->m_invI * impulse;

	// update body's velocities
	pBody1->m_velocity = body1_vel;
	pBody2->m_velocity = body2_vel;
	pBody1->m_angularVelocity = body1_avel;
	pBody2->m_angularVelocity = body2_avel;
}

#endif

};
