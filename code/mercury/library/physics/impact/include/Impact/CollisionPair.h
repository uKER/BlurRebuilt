#ifndef __PHYSICS_COLLISIONPAIR_H__
#define __PHYSICS_COLLISIONPAIR_H__

#include "Impact\BodyPair.h"

namespace Impact
{

class Contact;
class Body;

// coefficients used for contact manifold friction
struct FrictionCoefficients
{
	Maths::HVec3	m_vPosition;
	float			m_fLimit;
	Maths::HVec3	m_vTangent[2];
	Maths::HVec3	m_vNormal;
	float			m_invkTangent[2];
	float			m_fAccTangentImpulseX;
	float			m_fAccTangentImpulseY;
	float			m_fAccTwistImpulse;
};

// contain a list of contact points between two bodies
class CollisionPair : public BodyPair
{
public:
	/// default constructor
			CollisionPair() : BodyPair(), m_pPersistentData(NULL) { m_type = E_PairType_Collision; };

	/// construct using two bodies
			CollisionPair(Body *p1, Body *p2);

	/// destructor
			~CollisionPair();

	/// update the CollisionPair with a new set of contacts
	void	UpdateContacts(Contact *contacts, int nContacts);

	/// do the first iteration of the solver + some precalculation
	void	PreStep(float inv_dt);

	/// do subsequent iterations of the solver
	void	Step();

	/// free this collision pair's persistent collision data
	void	FreePersistentData();

	// variables are arranged in the order they are accessed
	float	m_fFriction;
	I_PersistentData *m_pPersistentData;

	// contacts
	Ut::Vector<Contact> m_Contacts;

#ifdef SINGLE_FRICTION_MANIFOLD
	// friction coefficients
	FrictionCoefficients m_Friction;
#endif
};

/// operator for stl sorting
inline bool operator < (const CollisionPair &a1, const CollisionPair &a2)
{
	return (a1.m_pBody[0] < a2.m_pBody[0]) ||
		(a1.m_pBody[0] == a2.m_pBody[0] && a1.m_pBody[1] < a2.m_pBody[1]);
}

};

#endif
