#ifndef __PHYSICS_BODY_H__
#define __PHYSICS_BODY_H__

namespace Impact
{

class Island;
class World;
class BodyPair;
class CollisionPair;
class Shape;
class BaseShape;

/// physics body class
class Body
{
	friend class Island;
	friend class World;
	friend class CollisionPair;
public:
							/// default constructor
							Body();

							/// destructor
							~Body();

							/// get world-space AABB
	Maths::C_AxisBox		GetAABB();

							/// set the body's mass
	void					SetMass(float mass);

							/// add a body pair to the list (called by broadphase)
	void					AddPair(BodyPair *p);

							/// remove a body pair from the list (called by broadphase)
	void					RemovePair(BodyPair *p);

							/// get the collision shape for colliding against
	Shape					*GetShape();

							/// set the collison shape
							/// T must have a function called _Instantiate_ returning a Shape*
	void					AttachShape(BaseShape *pHull);

							/// recalculate the activation status
	void					UpdateActivationStatus(float fTimeDelta);

							/// returns true if the body is currently active
	bool					IsActive();

							/// flag this body as 'dirty', meaning that it's position and/or rotation has changed
	void					Dirty();

							/// force the body to be activated or deactivated
	void					SetActive(bool bActive);

							/// get body orientation and position taking into account the shape's center of gravity offset
	const Maths::HMat43		GetOrientation();

							/// set which notification events will be sent for this body
	void					SetEventFilter(int iFlags);

							/// returns true if the body is meant to use continuous collision detection
	bool					ContinuousCollision();

							/// switch continuous collision on and off
	void					SetContinuousCollision(bool bOn);

							/// returns true if this body is static (e.g. world mesh)
	bool					IsStatic() { return m_bStatic; }

							/// set a body to be static or not
	void					SetStatic(bool bStatic);

							/// set the collision mask used for collision filtering
	void					SetCollisionMask(U32 iMask);

							/// Set friction coefficient
	void					SetFriction(float f);

	// location
	Maths::HVec3			m_vPos;
	Maths::HVec3			m_angularVelocity;
	Maths::HVec3			m_velocity;
	float					m_invMass;
	float					m_invI;
	Maths::C_Quaternion		m_rotation;

	Maths::HVec3			m_vPrevPos;
	Maths::C_Quaternion		m_qPrevRot;

private:
	Shape					*m_pShape;				// collision shape
	int						m_iIsland;				// index of the island this body is in
	bool					m_bActive;
	bool					m_bStatic;
	bool					m_bContinuousCollision;
	float					m_fDeactivationTime;	// time left to deactivation
	float					m_fFriction;			// friction coefficient
	int						m_iBroadPhaseID;		// handle to broadphase
	int						m_iTag;
	int						m_EventFilter;			// bitfield specifying which events we're interested in
	U32						m_iCollisionMask;		// mask for collision filtering

	// used by _Island_
	Ut::Vector<BodyPair*>	m_Pairs;
};

};

#endif

