#include "stdafx.h"

#include "Impact\Body.h"
#include "Impact\Constraint.h"

using namespace Maths;

namespace Impact
{

/// constructor from two bodies
Constraint::Constraint(Body *p1, Body *p2)
	: BodyPair(p1, p2)
{
	m_type = E_PairType_Constraint;
}

/// defstructor
Constraint::~Constraint()
{
}

// currently we've got a really simple constraint that is used for the bdoingy cones in PGR4

Debug::Float g_ConstraintStrength("Impact.Constraint.Strength", 100.f, Debug::Range(0.f,100.f));
Debug::Float g_ConstraintDamping("Impact.Constraint.Damping", 10.f, Debug::Range(0.f,100.f));

void Constraint::PreStep(float inv_dt)
{
	// apply spring to make cone point up
	const C_Quaternion rotation = m_pBody[0]->m_rotation * m_invOrgOrientation;

	// Initialise the rotation variables.
	HVec3 rvRotationVector = rotation.Imag() * HF32(2.f);
	HVec3 v = rvRotationVector * g_ConstraintStrength - m_pBody[0]->m_angularVelocity * g_ConstraintDamping;
	m_pBody[0]->m_angularVelocity += v / inv_dt;
}

/// apply the constraint
void Constraint::Step()
{
	// fix position
	m_pBody[0]->m_velocity = HVec3(0,0,0);
}

};
