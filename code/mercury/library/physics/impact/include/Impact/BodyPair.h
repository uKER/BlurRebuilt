#ifndef __IMPACT_BODYPAIR_H__
#define __IMPACT_BODYPAIR_H__

namespace Impact
{

class Body;

enum E_PairType
{
	E_PairType_Collision,
	E_PairType_Constraint,
};

/// a parent class for CollisionPair and Constraint
/// this class encapsulates functionality that constrains two bodies together either because of collision or joints
/// this class is mainly used to generate islands between bodies. bodies that are in a BodyPair are always in the same island
class BodyPair
{
public:
	/// default constructor
			BodyPair() : m_iTag(0), m_bActive(true) {};

	/// construct using two bodies
			BodyPair(Body *p1, Body *p2);

	/// destructor
			~BodyPair();

	/// get the type of this pair (e.g. collisionpair or constraint)
	E_PairType	GetType() { return m_type; }

	// variables are arranged in the order they are accessed
	int			m_iTag;
	Body		*m_pBody[2];
	bool		m_bActive;
	E_PairType	m_type;
};

};

#endif
