#include "stdafx.h"

#include <system/debug/I_DebugRender.h>
#include <Dynamic/Cloth/Cloth.h>

#include <algorithm>

//lint -e662 -e661 -e415

namespace DynamicEffect {

Debug::Bool g_debugClothEnable("Graphics.Dynamic.Cloth.Enable", true);

namespace Cloth {
//-----------------------------------------------------------------------------------

Debug::Bool g_DebugClothEnableUpdate("Graphics.Dynamic.Cloth.Physics.EnableUpdate", true);

Debug::Float g_DebugWindAverageVelocityZ("Graphics.Dynamic.Cloth.Physics.WindAverageVelocityZ", 0.0f, Debug::Range(0.0f, 5.0f));
Debug::Float g_DebugWindAverageVelocityY("Graphics.Dynamic.Cloth.Physics.WindAverageVelocityY", 0.0f, Debug::Range(0.0f, 5.0f));
Debug::Float g_DebugWindAverageVelocityX("Graphics.Dynamic.Cloth.Physics.WindAverageVelocityX", 1.5f, Debug::Range(0.0f, 5.0f));
Debug::Float g_DebugWindNoiseVelocityScale("Graphics.Dynamic.Cloth.Physics.WindNoiseVelocityScale", 0.0f, Debug::Range(0.0f, 5.0f));
Debug::Float g_DebugWindNoiseTimeOffsetZ("Graphics.Dynamic.Cloth.Physics.WindNoiseTimeOffsetZ", 2.0f, Debug::Range(0.0f, 5.0f));
Debug::Float g_DebugWindNoiseTimeOffsetY("Graphics.Dynamic.Cloth.Physics.WindNoiseTimeOffsetY", 1.0f, Debug::Range(0.0f, 5.0f));
Debug::Float g_DebugWindNoiseTimeOffsetX("Graphics.Dynamic.Cloth.Physics.WindNoiseTimeOffsetX", 0.0f, Debug::Range(0.0f, 5.0f));
Debug::Float g_DebugWindNoiseTimeScale("Graphics.Dynamic.Cloth.Physics.WindNoiseTimeScale", 1.0f, Debug::Range(0.0f, 10.0f));

Debug::Bool g_DebugDebugRender("Graphics.Dynamic.Cloth.Physics.DebugRender", false);

Debug::Int g_DebugInverseConstraintIterations("Graphics.Dynamic.Cloth.Physics.InverseConstraintIterations", 0, Debug::Range(0, 5));

Debug::Float g_DebugMaxMinorTimeStep("Graphics.Dynamic.Cloth.Physics.MaxMinorTimeStep", 0.032f, Debug::Range(0.001f, 0.1f));
	
Debug::Float g_DebugShearCriticalDeformationRate("Graphics.Dynamic.Cloth.Physics.ShearCriticalDeformationRate",
	0.3f, Debug::Range(0.01f, 1.0f));

Debug::Float g_DebugStretchCriticalDeformationRate("Graphics.Dynamic.Cloth.Physics.StretchCriticalDeformationRate",
	0.1f, Debug::Range(0.01f, 1.0f));

#ifdef CLOTH_USE_SCALING_FACTORS
Debug::Bool g_DebugUseValues("Graphics.Dynamic.Cloth.Physics.UseDebugVales", false);

Debug::Float g_DebugGravity("Graphics.Dynamic.Cloth.Physics.Gravity",
	9.8f, Debug::Range(0.0f, 20.0f));

Debug::Float g_DebugWindResistanceCoeffScalingFactor("Graphics.Dynamic.Cloth.Physics.WindResistanceCoeffScalingFactor",
	10.0f, Debug::Range(0.0f, 5.0f));

Debug::Float g_DebugNaturalLengthScalingFactor("Graphics.Dynamic.Cloth.Physics.NaturalLengthScalingFactor",
	0.1f, Debug::Range(0.0f, 5.0f));

Debug::Float g_DebugBendDampCoeffScalingFactor("Graphics.Dynamic.Cloth.Physics.BendDampCoeffScalingFactor",
	0.2f, Debug::Range(0.0f, 5.0f));

Debug::Float g_DebugShearDampCoeffScalingFactor("Graphics.Dynamic.Cloth.Physics.ShearDampCoeffScalingFactor",
	4.0f, Debug::Range(0.0f, 5.0f));

Debug::Float g_DebugStretchDampCoeffScalingFactor("Graphics.Dynamic.Cloth.Physics.StretchDampCoeffScalingFactor",
	10.0f, Debug::Range(0.0f, 5.0f));

Debug::Float g_DebugBendSpringConstScalingFactor("Graphics.Dynamic.Cloth.Physics.BendSpringConstScalingFactor",
	10.0f, Debug::Range(0.0f, 5.0f));

Debug::Float g_DebugShearSpringConstScalingFactor("Graphics.Dynamic.Cloth.Physics.ShearSpringConstScalingFactor",
	2000.0f, Debug::Range(0.0f, 5.0f));

Debug::Float g_DebugStretchSpringConstScalingFactor("Graphics.Dynamic.Cloth.Physics.StretchSpringConstScalingFactor",
	5000.0f, Debug::Range(0.0f, 5.0f));

Debug::Float g_DebugParticleMassScalingFactor("Graphics.Dynamic.Cloth.Physics.ParticleMassScalingFactor",
	1.0f, Debug::Range(0.1f, 5.0f));
#endif


//---------------------------------


void C_Sheet::Initialise(C_Particle & p, const hv & position, bool isPinned)
{
	p.position = position;
	p.prevPosition = position;
	p.velocity = hv();
	p.force = hv();
	p.acceleration = hv();
	p.invMass = isPinned ? 0.f : 1.f / (F32)m_particleMass;
	p.isPinned = isPinned;
}


void C_Sheet::Initialise( const SheetParameters& parameters )
{
	m_nX = parameters.size.x;
	m_nY = parameters.size.y;
	m_particleArray.Resize(m_nX * m_nY);

	m_particleMass = parameters.particleMass;

	m_stretchSpringConst = 2500.0f;
	m_shearSpringConst = 1000.0f;
	m_bendSpringConst = 10.0f;

	m_stretchDampCoeff = 10.0f;
	m_shearDampCoeff = 4.0f;
	m_bendDampCoeff = 0.2f;

	F32 stretchNaturalLengthScaled = parameters.stretchNaturalLength * (1.f/parameters.stretchFactor);

	m_stretchNaturalLength = stretchNaturalLengthScaled;
	m_shearNaturalLength = stretchNaturalLengthScaled * sqrtf(2.f);
	m_bendNaturalLength = stretchNaturalLengthScaled * 2.f;

	m_windResistanceCoeff = parameters.windResistanceCoeff;

	m_gravity = parameters.gravity;


	CLOTH_FORGRID(m_nX, m_nY)
	{
		hv offsetx = parameters.worldMatrix.AX() * F32(x);
		hv offsety = parameters.worldMatrix.AY() * F32(y);
		hv position = parameters.worldMatrix.AW() + (offsetx + offsety) * parameters.stretchNaturalLength;
		Initialise( Particle(x, y), position, false );
	}

	for(S32 i = 0; i < parameters.pinnedPositions.Size(); ++i)
	{
		const MathsStorage::Int2& position = parameters.pinnedPositions[i];
		assert(position.x >= 0 && position.x < m_nX);
		assert(position.y >= 0 && position.y < m_nY);

		Particle(position.x, position.y).isPinned = true;
		Particle(position.x, position.y).invMass = 0.f;
	}

	CreateConstraints_Verlet();

	UpdateNormals();
}

void C_Sheet::Shutdown()
{
	m_particleArray.Destroy();
	m_constraints.Destroy();
}

Debug::Bool  g_DebugEnableVerletCloth("Graphics.Dynamic.Cloth.Physics.EnableVerletCloth", true);
Debug::Bool  g_DebugRecalcVerletConstraints("Graphics.Dynamic.Cloth.Physics.RecalcVerletConstraints", false);
Debug::Float g_DebugVerletClothDamping("Graphics.Dynamic.Cloth.Physics.VerletClothDamping", 1.f, Debug::Range(0.f, 1.f));
Debug::Int   g_DebugVerletConstraintIterations("Graphics.Dynamic.Cloth.Physics.VerletConstraintIterations", 1, Debug::Range(0, 100));
Debug::Bool  g_DebugSortVerletConstraints("Graphics.Dynamic.Cloth.Physics.SortVerletConstraints", true);
Debug::Float g_DebugVerletStretchStrength("Graphics.Dynamic.Cloth.Physics.VerletStretchStrength", 0.7f, Debug::Range(0.f, 1.f));
Debug::Float g_DebugVerletShearStrength("Graphics.Dynamic.Cloth.Physics.VerletShearStrength", 0.4f, Debug::Range(0.f, 1.f));
Debug::Float g_DebugVerletBendStrength("Graphics.Dynamic.Cloth.Physics.VerletBendStrength", 0.f, Debug::Range(0.f, 1.f));


void C_Sheet::ApplyForces_Verlet()
{
	Maths::HVec3 windVelocity;
	Maths::HF32 windResistance;

	windVelocity = GetWindVelocity();
	windResistance = GetWindResistanceCoeff();
	
	const Maths::HF32 zero;
	const Maths::HVec3 gravity(zero, -GetGravity(), zero);

	// Apply wind resistance and gravity.
	for (U32 i = 0, count = GetNumParticles(); i < count; ++i)
	{
		C_Particle& particle(m_particleArray[i]);

		Maths::HVec3 normal;
		Maths::HVec3 velocity;
		Maths::HF32 invMass(particle.invMass);
		const Maths::HF32 zero;

		normal = particle.normal;
		velocity = particle.velocity;
		
		Maths::HVec3 force = windResistance * (windVelocity - velocity);
		force = (force | normal) * normal;

		Maths::HVec3 accel;
		accel = Maths::Select(invMass.GT(zero), gravity, Maths::HVec3());	// Pinned particles have invMass of zero.
		accel += force * invMass;

		particle.acceleration = accel;
	}
}

void C_Sheet::Integrate_Verlet(F32 timeStep)
{
	const Maths::HF32 k1(1.f + g_DebugVerletClothDamping.Value());
	const Maths::HF32 k2(g_DebugVerletClothDamping.Value());
	Maths::HF32 timeStepSquared(timeStep * timeStep);
	Maths::HF32 invTimeStep(1.f / timeStep);

	Maths::HVec3 prevPos;
	Maths::HVec3 currPos;
	Maths::HVec3 currAccel;
	Maths::HVec3 newPos;

	for (U32 i = 0, count = GetNumParticles(); i < count; ++i)
	{
		C_Particle& particle(m_particleArray[i]);
		
		// 2 * x(t) - x(t - dt) + a * dt^2

		prevPos = particle.prevPosition;
		currPos = particle.position;
		currAccel = particle.acceleration;

		newPos = currPos * k1 - prevPos * k2 + currAccel * timeStepSquared;

		particle.prevPosition = currPos;
		particle.position = newPos;
		particle.velocity = invTimeStep * (newPos - currPos);

		assert((bool) particle.position.Finite());
	}
}

template <typename TPair>
bool LessThanSecond(const TPair& pair1, const TPair& pair2)
{
	return pair1.second < pair2.second;
}

void C_Sheet::CreateConstraints_Verlet()
{
	m_constraints.Clear();

	// Stretch.
	CLOTH_FORGRID(m_nX-1, m_nY)
	{
		Constraint c;
		c.particle1 = ParticleIndex(x, y);
		c.particle2 = ParticleIndex(x + 1, y);
		c.distance  = GetStretchNaturalLength();
		c.strength  = g_DebugVerletStretchStrength;

		m_constraints.PushBack(c);
	}

	CLOTH_FORGRID(m_nX, m_nY - 1)
	{
		Constraint c;
		c.particle1 = ParticleIndex(x, y);
		c.particle2 = ParticleIndex(x, y + 1);
		c.distance  = GetStretchNaturalLength();
		c.strength  = g_DebugVerletStretchStrength;

		m_constraints.PushBack(c);
	}

	// Shear.
	CLOTH_FORGRID(m_nX-1, m_nY-1)
	{
		Constraint c1;
		c1.particle1 = ParticleIndex(x, y);
		c1.particle2 = ParticleIndex(x + 1, y + 1);
		c1.distance  = GetShearNaturalLength();
		c1.strength  = g_DebugVerletShearStrength;
		m_constraints.PushBack(c1);

		Constraint c2;
		c2.particle1 = ParticleIndex(x + 1, y);
		c2.particle2 = ParticleIndex(x, y + 1);
		c2.distance  = GetShearNaturalLength();
		c2.strength  = g_DebugVerletShearStrength;
		m_constraints.PushBack(c2);
	}

	// Bend.
	if (g_DebugVerletBendStrength > 0.f)
	{
		CLOTH_FORGRID(m_nX-2, m_nY)
		{
			Constraint c;
			c.particle1 = ParticleIndex(x, y);
			c.particle2 = ParticleIndex(x + 2, y);
			c.distance  = GetBendNaturalLength();
			c.strength  = g_DebugVerletBendStrength;
			m_constraints.PushBack(c);
		}

		CLOTH_FORGRID(m_nX, m_nY - 2)
		{
			Constraint c;
			c.particle1 = ParticleIndex(x, y);
			c.particle2 = ParticleIndex(x, y + 2);
			c.distance  = GetBendNaturalLength();
			c.strength  = g_DebugVerletBendStrength;
			m_constraints.PushBack(c);
		}
	}

	// Sort constraints by distance to nearest pin point to make the relaxation pass
	// as efficient as possible.

	if (g_DebugSortVerletConstraints)
	{
		Ut::Vector<F32> particleDistances;

		for (U32 i = 0, count = m_particleArray.Size(); i < count; ++i)
		{
			F32 d = 0.f;
			if (!m_particleArray[i].isPinned)
			{
				F32 distToNearestPinPoint = FLT_MAX;

				for (U32 j = 0; j < count; ++j)
				{
					if (m_particleArray[j].isPinned && j != i)
					{
						F32 dist = Maths::Length(m_particleArray[j].position - m_particleArray[i].position);
						distToNearestPinPoint = Maths::Min(dist, distToNearestPinPoint);
					}
				}

				d = distToNearestPinPoint;
			}
			particleDistances.PushBack(d);
		}

		Ut::Vector<Ut::Pair<U32, F32> > nearestPinDistances;

		for (U32 i = 0, count = m_constraints.Size(); i < count; ++i)
		{
			Ut::Pair<U32, F32> result;
			result.first = i;
			result.second = Maths::Min(particleDistances[m_constraints[i].particle1],
									   particleDistances[m_constraints[i].particle2]);
			nearestPinDistances.PushBack(result);
		}

		std::sort(nearestPinDistances.Begin(), nearestPinDistances.End(), &LessThanSecond<Ut::Pair<U32, F32> >);
		
		// Create new constraints array.
		ConstraintVector sortedConstraints;

		for (U32 i = 0, count = nearestPinDistances.Size(); i < count; ++i)
		{
			sortedConstraints.PushBack(m_constraints[nearestPinDistances[i].first]);
		}
		
		m_constraints = sortedConstraints;
	}
}

void C_Sheet::ApplyConstraints_Verlet(U32 numIterations)
{
	for (U32 iter = 0; iter < numIterations; ++iter)
	{
		for (U32 i = 0, count = m_constraints.Size(); i < count; ++i)
		{
			const Constraint& constraint(m_constraints[i]);

			C_Particle& p1(m_particleArray[constraint.particle1]);
			C_Particle& p2(m_particleArray[constraint.particle2]);

			Maths::HVec3 pos1;
			Maths::HVec3 pos2;
			Maths::HF32 p1InvMass;
			Maths::HF32 p2InvMass;

			pos1 = p1.position;
			
			pos2 = p2.position;

			p1InvMass = p1.invMass;
			p2InvMass = p2.invMass;

			const Maths::HF32 invTotalInvMass = Maths::Reciprocal(p1InvMass + p2InvMass);

			Maths::HVec3 dir(pos2 - pos1);
			Maths::HF32 dist = Maths::Length(dir);

			Maths::HF32 distError = (dist - constraint.distance);

			Maths::HF32 correctionFactor = Maths::HF32(constraint.strength) * distError * Maths::Reciprocal(dist);

			pos1 += dir * correctionFactor * p1InvMass * invTotalInvMass;
			pos2 -= dir * correctionFactor * p2InvMass * invTotalInvMass;

			p1.position = pos1;
			p2.position = pos2;
		}
	}
}

// #define CLOTH_FORGRID(xMax, yMax)  for ( S32 y = 0; y < yMax; ++y ) for ( S32 x = 0; x < xMax; ++x )

void C_Sheet::ApplyForces()
{
	if (m_particleArray.Empty())
	{
		return;
	}

	//CLOTH_FORGRID(m_nX-1, m_nY)
	//	ApplyStretchForce( Particle(x, y), Particle(x+1, y) );
	{
		const Maths::HF32 kStretchSpringConst(GetStretchSpringConst());
		const Maths::HF32 kStretchDampCoeff(GetStretchDampCoeff());
		const Maths::HF32 kStretchNaturalLength(GetStretchNaturalLength());

		C_Particle* pRowStart = &m_particleArray[0];
		const U32 rowSize = m_nX;
		for (S32 y = 0, yMax = m_nY; y < yMax; ++y)
		{
			C_Particle* pRow = pRowStart;
			for (S32 x = 0, xMax = m_nX - 1; x < xMax; ++x)
			{
				//ApplyStretchForce( pRow[x], pRow[x + 1]);
				ApplySpringForce( pRow[x], pRow[x + 1], kStretchSpringConst, kStretchDampCoeff, kStretchNaturalLength );
			}
			pRowStart += rowSize;
		}
	}
	
	//CLOTH_FORGRID(m_nX, m_nY-1)
	//	ApplyStretchForce( Particle(x, y), Particle(x, y+1) );
	{
		const Maths::HF32 kStretchSpringConst(GetStretchSpringConst());
		const Maths::HF32 kStretchDampCoeff(GetStretchDampCoeff());
		const Maths::HF32 kStretchNaturalLength(GetStretchNaturalLength());

		C_Particle* pRowStart = &m_particleArray[0];
		const U32 rowSize = m_nX;
		for (S32 y = 0, yMax = m_nY - 1; y < yMax; ++y)
		{
			C_Particle* pRow = pRowStart;
			for (S32 x = 0, xMax = m_nX; x < xMax; ++x)
			{
				//ApplyStretchForce( pRow[x], pRow[x + rowSize]);
				ApplySpringForce( pRow[x], pRow[x + rowSize], kStretchSpringConst, kStretchDampCoeff, kStretchNaturalLength );
			}
			pRowStart += rowSize;
		}
	}

	//CLOTH_FORGRID(m_nX-1, m_nY-1)
	//{
	//	ApplyShearForce( Particle(x, y), Particle(x+1, y+1) );
	//	ApplyShearForce( Particle(x+1, y), Particle(x, y+1) );
	//}
	{
		C_Particle* pRowStart = &m_particleArray[0];
		const U32 rowSize = m_nX;
		for (S32 y = 0, yMax = m_nY - 1; y < yMax; ++y)
		{
			C_Particle* pRow = pRowStart;
			for (S32 x = 0, xMax = m_nX - 1; x < xMax; ++x)
			{
				ApplyShearForce( pRow[x], pRow[x + 1 + rowSize]);
				ApplyShearForce( pRow[x + 1], pRow[x + rowSize]);
			}
			pRowStart += rowSize;
		}
	}

	//CLOTH_FORGRID(m_nX-2, m_nY)
	//	ApplyBendForce( Particle(x, y), Particle(x+2, y) );
	{
		const Maths::HF32 kBendSpringConst(GetBendSpringConst());
		const Maths::HF32 kBendDampCoeff(GetBendDampCoeff());
		const Maths::HF32 kBendNaturalLength(GetBendNaturalLength());

		C_Particle* pRowStart = &m_particleArray[0];
		const U32 rowSize = m_nX;
		for (S32 y = 0, yMax = m_nY; y < yMax; ++y)
		{
			C_Particle* pRow = pRowStart;
			for (S32 x = 0, xMax = m_nX - 2; x < xMax; ++x)
			{
				//ApplyBendForce( pRow[x], pRow[x + 2]);
				ApplySpringForce( pRow[x], pRow[x + 2], kBendSpringConst, kBendDampCoeff, kBendNaturalLength);
			}
			pRowStart += rowSize;
		}
	}

	//CLOTH_FORGRID(m_nX, m_nY-2)
	//	ApplyBendForce( Particle(x, y), Particle(x, y+2) );
	{
		const Maths::HF32 kBendSpringConst(GetBendSpringConst());
		const Maths::HF32 kBendDampCoeff(GetBendDampCoeff());
		const Maths::HF32 kBendNaturalLength(GetBendNaturalLength());

		C_Particle* pRowStart = &m_particleArray[0];
		const U32 rowSize = m_nX;
		for (S32 y = 0, yMax = m_nY - 2; y < yMax; ++y)
		{
			C_Particle* pRow = pRowStart;
			for (S32 x = 0, xMax = m_nX; x < xMax; ++x)
			{
				//ApplyBendForce( pRow[x], pRow[x + 2 * rowSize]);
				ApplySpringForce( pRow[x], pRow[x + 2 * rowSize], kBendSpringConst, kBendDampCoeff, kBendNaturalLength);
			}
			pRowStart += rowSize;
		}
	}

	//CLOTH_FORGRID(m_nX, m_nY)
	//{
	//	ApplyGravity( Particle(x, y) );
	//	ApplyWindResistance(x, y);
	//}
	{
		C_Particle* pRowStart = &m_particleArray[0];
		const U32 rowSize = m_nX;
		for (S32 y = 0, yMax = m_nY; y < yMax; ++y)
		{
			C_Particle* pRow = pRowStart;
			for (S32 x = 0, xMax = m_nX; x < xMax; ++x)
			{
				ApplyGravity( pRow[x] );
				ApplyWindResistance( pRow[x] );
			}
			pRowStart += rowSize;
		}
	}
}


void C_Sheet::ApplyInverseConstraints()
{
	CLOTH_FORGRID(m_nX-1, m_nY)
		ApplyStretchInverseConstraint( Particle(x, y), Particle(x+1, y) );

	CLOTH_FORGRID(m_nX, m_nY-1)
		ApplyStretchInverseConstraint( Particle(x, y), Particle(x, y+1) );

	CLOTH_FORGRID(m_nX-1, m_nY-1)
	{
		ApplyShearInverseConstraint( Particle(x, y), Particle(x+1, y+1) );
		ApplyShearInverseConstraint( Particle(x+1, y), Particle(x, y+1) );
	}
}


void C_Sheet::Update( const hf & dt )
{
	if (!g_DebugClothEnableUpdate)
		return;

	if (m_particleArray.Empty())
	{
		return;
	}

	if (g_DebugEnableVerletCloth)
	{
		if (g_DebugRecalcVerletConstraints)
		{
			CreateConstraints_Verlet();
		}
		ApplyForces_Verlet();
		Integrate_Verlet(dt);
		ApplyConstraints_Verlet(g_DebugVerletConstraintIterations);

	}
	else
	{
		
		//CLOTH_FORGRID(m_nX, m_nY)
		//	ResetForce( Particle(x, y) );
		{
			C_Particle* pParticles = &m_particleArray[0];
			for (U32 i = 0, count = m_particleArray.Size(); i < count; ++i)
			{
				ResetForce( pParticles[i] );
			}
		}

		ApplyForces();

		//CLOTH_FORGRID(m_nX, m_nY)
		//	Step( Particle(x, y), dt );
		{
			C_Particle* pRowStart = &m_particleArray[0];
			const U32 rowSize = m_nX;
			for (S32 y = 0, yMax = m_nY; y < yMax; ++y)
			{
				C_Particle* pRow = pRowStart;
				for (S32 x = 0, xMax = m_nX; x < xMax; ++x)
				{
					Step( pRow[x], dt );
				}
				pRowStart += rowSize;
			}
		}

		for (S32 i=0; i < g_DebugInverseConstraintIterations; ++i)
			ApplyInverseConstraints();
	}

	UpdateNormals();
}


Debug::Bool  g_DebugEnableFastNormalUpdate("Graphics.Dynamic.Cloth.Physics.EnableFastNormalUpdate", true);

void C_Sheet::UpdateNormals()
{
	if (g_DebugEnableFastNormalUpdate)
	{
		UpdateNormals_Fast();
	}
	else
	{
		CLOTH_FORGRID(m_nX, m_nY)
			UpdateNormal(x, y);
	}
}

template <bool SampleLeft, bool SampleRight, bool SampleTop, bool SampleBottom>
FORCE_INLINE void CalculateNormal(C_Particle* pParticle, U32 rowSize)
{
	Maths::HVec3 right;
	Maths::HVec3 left;
	Maths::HVec3 top;
	Maths::HVec3 bottom;

	if (SampleRight)
		right   = (pParticle + 1)->position;
	if (SampleLeft)
		left	= (pParticle - 1)->position;
	if (SampleTop)
		top		= (pParticle + rowSize)->position;
	if (SampleBottom)
		bottom  = (pParticle - rowSize)->position;

	// [ChrisC] For some reason using the HVec3 constructor causes "centre" to spill onto the stack.
	// Using the assignment operator seems to generate better code.
	Maths::HVec3 centre;
	centre = (pParticle->position);

	Maths::HVec3 n1;
	Maths::HVec3 n2;
	Maths::HVec3 n3;
	Maths::HVec3 n4;

	if (SampleLeft && SampleBottom)
		n1 = (left - centre) ^ (bottom - centre);
	if (SampleBottom && SampleRight)
		n2 = (bottom - centre) ^ (right - centre);
	if (SampleTop && SampleLeft)
		n3 = (top - centre) ^ (left - centre);
	if (SampleRight && SampleTop)
		n4 = (right - centre) ^ (top - centre);

	Maths::HVec3 normal(Maths::Normalise(n1 + n2 + n3 + n4));

	pParticle->normal = normal;
}

void C_Sheet::UpdateNormals_Fast()
{
	// Interior pass.
	for (U32 y = 1, yCount = m_nY - 1; y < yCount; ++y)
	{
		const U32 rowIndex = y * m_nX;

		for (U32 x = 1, xCount = m_nX - 1; x < xCount; ++x)
		{
			CalculateNormal<true, true, true, true>(&m_particleArray[rowIndex + x], m_nX);
		}
	}

	// Corners.
	CalculateNormal<false, true, true, false>(&m_particleArray[0], m_nX);
	CalculateNormal<false, true, false, true>(&m_particleArray[(m_nY - 1) * m_nX], m_nX);
	CalculateNormal<true, false, true, false>(&m_particleArray[m_nX - 1], m_nX);
	CalculateNormal<true, false, false, true>(&m_particleArray[(m_nY * m_nX) - 1], m_nX);

	// Top and bottom side passes.
	for (U32 x = 1, xCount = m_nX - 1; x < xCount; ++x)
	{
		CalculateNormal<true, true, true, false>(&m_particleArray[x], m_nX);
		CalculateNormal<true, true, false, true>(&m_particleArray[(m_nY - 1) * m_nX + x], m_nX);
	}

	// Left and right side passes.
	for (U32 y = 1, yCount = m_nY - 1; y < yCount; ++y)
	{
		CalculateNormal<false, true, true, true>(&m_particleArray[(y * m_nX)], m_nX);
		CalculateNormal<true, false, true, true>(&m_particleArray[(y * m_nX) + m_nX - 1], m_nX);
	}
}


void C_Sheet::SetWind( const hv & wind)
{
	m_windVelocity = wind;
}


void C_Sheet::DebugRenderLink( DbUtil::I_DebugRenderPtr debugRenderPtr, const C_Particle & p1,
	const C_Particle & p2 ) const
{
	debugRenderPtr->Draw3dLine( p1.position, p2.position, c_DebugRenderLinkColour );
}


void C_Sheet::DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr ) const
{
	CLOTH_FORGRID(m_nX, m_nY)
		debugRenderPtr->Draw3dLine( Particle(x, y).position, Particle(x, y).position + Particle(x, y).normal * m_stretchNaturalLength, c_DebugRenderNormalColour );

	CLOTH_FORGRID(m_nX-1, m_nY)
		DebugRenderLink( debugRenderPtr, Particle(x, y), Particle(x+1, y) );

	CLOTH_FORGRID(m_nX, m_nY-1)
		DebugRenderLink( debugRenderPtr, Particle(x, y), Particle(x, y+1) );
}




//-----------------------------------------------------------------------------------
};
};
