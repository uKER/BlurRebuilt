#ifndef __PHYSICS_CONSTRAINT_H__
#define __PHYSICS_CONSTRAINT_H__

#include "Impact\BodyPair.h"

namespace Impact
{

class Contact;
class Body;

// constraint between two bodies
class Constraint : public BodyPair
{
public:
	/// default constructor
			Constraint() : BodyPair() { m_type = E_PairType_Constraint; };

	/// construct using two bodies
			Constraint(Body *p1, Body *p2);

	/// destructor
			~Constraint();

	/// do the first iteration of the solver + some precalculation
	void	PreStep(float inv_dt);

	/// do subsequent iterations of the solver
	void	Step();

	Maths::C_Quaternion m_invOrgOrientation;
};

};

#endif
