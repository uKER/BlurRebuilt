#ifndef RENDER_CLOTH_H
#define RENDER_CLOTH_H

namespace DynamicEffect {
namespace Cloth {
//-----------------------------------------------------------------------------------


#ifdef CFG_DEBUG
#define CLOTH_USE_SCALING_FACTORS
#endif


//-------------------------------------

	
typedef Maths::HF32 hf;
typedef Maths::HVec3 hv;


#define CLOTH_FORGRID(xMax, yMax)  for ( S32 y = 0; y < (yMax); ++y ) for ( S32 x = 0; x < (xMax); ++x )


const Maths::RGBA c_DebugRenderLinkColour( 0xff, 0xff, 0xff );
const Maths::RGBA c_DebugRenderNormalColour( 0xff, 0x00, 0x00 );

extern Debug::Float g_DebugShearCriticalDeformationRate;
extern Debug::Float g_DebugStretchCriticalDeformationRate;
extern Debug::Float g_DebugMaxMinorTimeStep;

#ifdef CLOTH_USE_SCALING_FACTORS
extern Debug::Bool	g_DebugUseValues;
extern Debug::Float g_DebugGravity;
extern Debug::Float g_DebugWindResistanceCoeffScalingFactor;
extern Debug::Float g_DebugNaturalLengthScalingFactor;
extern Debug::Float g_DebugBendDampCoeffScalingFactor;
extern Debug::Float g_DebugShearDampCoeffScalingFactor;
extern Debug::Float g_DebugStretchDampCoeffScalingFactor;
extern Debug::Float g_DebugBendSpringConstScalingFactor;
extern Debug::Float g_DebugShearSpringConstScalingFactor;
extern Debug::Float g_DebugStretchSpringConstScalingFactor;
extern Debug::Float g_DebugParticleMassScalingFactor;
#endif


//-------------------------------------

struct SheetParameters
{
	SheetParameters() : size(0, 0), gravity(9.8f), stretchNaturalLength(.1f), stretchFactor(1.f), particleMass(1.f), windResistanceCoeff(10.f) {}
	~SheetParameters() {}

	MathsStorage::Int2				size;
	Ut::Vector<MathsStorage::Int2>	pinnedPositions;

	Maths::HMat43					worldMatrix;
	F32								gravity;
	F32								stretchNaturalLength;
	F32								stretchFactor;
	F32								particleMass;
	F32								windResistanceCoeff;
};

class C_Particle
{
public:
	hv position;
	hv prevPosition;
	hv normal;
	hv velocity;
	hv force;
	hv acceleration;
	F32 invMass;
	bool isPinned;
};


class C_Sheet
{
public:

	// Returns size of sheet-grid.
	S32 GetSizeX() const {return m_nX;}
	S32 GetSizeY() const {return m_nY;}

	// Particle positions and normals in world space.
	const Maths::HVec3 & GetParticlePosition(S32 x, S32 y) const { return Particle(x, y).position; }
	const Maths::HVec3 & GetParticleNormal(S32 x, S32 y) const { return Particle(x, y).normal; }

	void SetPinnedParticlePosition(S32 x, S32 y, const Maths::HVec3& position)
	{
		C_Particle& p(Particle(x, y));
		p.position = position;
		p.prevPosition = position;
	}

	// Calculated on-the-fly; in the 'x' direction on the sheet-grid.
	const Maths::HVec3 GetParticleTangent(S32 x, S32 y) const { return CalculateTangentX(x, y); };


//***************************************************************************************//


	void Initialise( const SheetParameters& paramaters );
	void Shutdown();
	void Update( const hf & dt );
	void SetWind( const hv & wind );
	void DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr ) const;


//***************************************************************************************//


	C_Particle & Particle(S32 x, S32 y)
	{
		return m_particleArray[x + y * m_nX];
	}


	const C_Particle & Particle(S32 x, S32 y) const
	{
		return m_particleArray[x + y * m_nX];
	}

	U32 ParticleIndex(S32 x, S32 y) const
	{
		return x + y * m_nX;
	}

	U32		GetNumParticles() const
	{
		return m_particleArray.Size();
	}

private:

#ifdef CLOTH_USE_SCALING_FACTORS
	const hf GetParticleMass() const { return g_DebugUseValues ? hf(g_DebugParticleMassScalingFactor) : m_particleMass; }

	const hf GetStretchSpringConst() const { return g_DebugUseValues ? hf(g_DebugStretchSpringConstScalingFactor) : m_stretchSpringConst; }
	const hf GetShearSpringConst() const { return g_DebugUseValues ? hf(g_DebugShearSpringConstScalingFactor) : m_shearSpringConst; }
	const hf GetBendSpringConst() const { return g_DebugUseValues ? hf(g_DebugBendSpringConstScalingFactor) : m_bendSpringConst; }

	const hf GetStretchDampCoeff() const { return g_DebugUseValues ? hf(g_DebugStretchDampCoeffScalingFactor) : m_stretchDampCoeff; }
	const hf GetShearDampCoeff() const { return g_DebugUseValues ? hf(g_DebugShearDampCoeffScalingFactor) : m_shearDampCoeff; }
	const hf GetBendDampCoeff() const { return g_DebugUseValues ? hf(g_DebugBendDampCoeffScalingFactor) : m_bendDampCoeff; }

	const hf GetStretchNaturalLength() const { return g_DebugUseValues ? hf(g_DebugNaturalLengthScalingFactor) : m_stretchNaturalLength; }
	const hf GetShearNaturalLength() const { return g_DebugUseValues ? hf(g_DebugNaturalLengthScalingFactor * 2.0f) : m_shearNaturalLength; }
	const hf GetBendNaturalLength() const { return g_DebugUseValues ? hf(g_DebugNaturalLengthScalingFactor * 2.0f) : m_bendNaturalLength; }

	const hf GetWindResistanceCoeff() const { return g_DebugUseValues ? hf(g_DebugWindResistanceCoeffScalingFactor) : m_windResistanceCoeff; }
	const hf GetGravity() const { return g_DebugUseValues ? hf(g_DebugGravity) : m_gravity; }
#else
	const hf GetParticleMass() const { return m_particleMass; }

	const hf GetStretchSpringConst() const { return m_stretchSpringConst; }
	const hf GetShearSpringConst() const { return m_shearSpringConst; }
	const hf GetBendSpringConst() const { return m_bendSpringConst; }

	const hf GetStretchDampCoeff() const { return m_stretchDampCoeff; }
	const hf GetShearDampCoeff() const { return m_shearDampCoeff; }
	const hf GetBendDampCoeff() const { return m_bendDampCoeff; }

	const hf GetStretchNaturalLength() const { return m_stretchNaturalLength; }
	const hf GetShearNaturalLength() const { return m_shearNaturalLength; }
	const hf GetBendNaturalLength() const { return m_bendNaturalLength; }

	const hf GetWindResistanceCoeff() const { return m_windResistanceCoeff; }
	const hf GetGravity() const { return m_gravity; }
#endif

	const hf GetStretchCriticalDeformationRate() const { return F32( g_DebugStretchCriticalDeformationRate ); }
	const hf GetShearCriticalDeformationRate() const { return F32( g_DebugShearCriticalDeformationRate ); }

	const hv & GetWindVelocity() const { return m_windVelocity; }


	void Initialise(C_Particle & p, const hv & position, bool isPinned);


	__forceinline void ResetForce( C_Particle & p )
	{
		p.force = hv();
	}


	// Applies a force to a single particle.
	__forceinline void ApplyForce( C_Particle & p, const hv & force )
	{
		p.force += force;
	}


	// Applies a force between two particles, with positive sign for p1.
	__forceinline void ApplyForce( C_Particle & p1, C_Particle & p2, const hv & force )
	{
		ApplyForce(p1, force);
		ApplyForce(p2, -force);
	}


	// Hooke's Law plus damping.
	__forceinline void ApplySpringForce( C_Particle & p1, C_Particle & p2, const hf & springConst, const hf & dampCoeff,
		const hf & naturalLength )
	{
		hv dr = p2.position - p1.position;
		hf drMod = Length(dr);
		hv drHat = dr / drMod;

		hv dv = p2.velocity - p1.velocity;

		hv force =
			springConst * (drMod - naturalLength) * drHat +
			dampCoeff * (dv | drHat) * drHat;

		ApplyForce( p1, p2, force );
	}


	__forceinline void ApplyStretchForce( C_Particle & p1, C_Particle & p2 )
	{
		ApplySpringForce( p1, p2, GetStretchSpringConst(), GetStretchDampCoeff(), GetStretchNaturalLength() );
	}


	__forceinline void ApplyShearForce( C_Particle & p1, C_Particle & p2 )
	{
		ApplySpringForce( p1, p2, GetShearSpringConst(), GetShearDampCoeff(), GetShearNaturalLength() );
	}


	__forceinline void ApplyBendForce( C_Particle & p1, C_Particle & p2 )
	{
		ApplySpringForce( p1, p2, GetBendSpringConst(), GetBendDampCoeff(), GetBendNaturalLength() );
	}


	__forceinline void ApplyGravity( C_Particle & p )
	{
		ApplyForce( p, GetParticleMass() * GetGravity() * Maths::C_Normal::AxisNegativeY() );
	}


	__forceinline void ApplyWindResistance(C_Particle& p)
	{
		hv force = GetWindResistanceCoeff() * (GetWindVelocity() - p.velocity);
		hv normal = p.normal;
		force = (force | normal) * normal;

		ApplyForce( p, force );
	}

	void ApplyForces_Verlet();
	void Integrate_Verlet(F32 timeStep);
	void CreateConstraints_Verlet();
	void ApplyConstraints_Verlet(U32 numIterations);

	__forceinline void Step( C_Particle & p, const hf & dt )
	{
		if ( !p.isPinned )
		{
			p.velocity += p.force / GetParticleMass() * dt;
			p.position += p.velocity * dt;
		}
	}

	
	void ApplyForces();


	__forceinline void ApplyInverseConstraint( C_Particle & p1, C_Particle & p2, const hf & criticalDeformationRate,
		const hf & naturalLength )
	{
		hv dr = p2.position - p1.position;
		hf drMod = Length(dr);
		hf deformationRate = drMod / naturalLength - hf(c_ScalarOne);

		if (deformationRate > criticalDeformationRate)
		{
			hv displacement = naturalLength * (deformationRate - criticalDeformationRate) * dr / drMod;
			if (p1.isPinned)
			{
				if (!p2.isPinned)
					p2.position -= displacement;
			}
			else
			{
				if (p2.isPinned)
					p1.position += displacement;
				else
				{
					displacement *= hf(c_ScalarHalf);
					p1.position += displacement;
					p2.position -= displacement;
				}
			}
		}
	}


	__forceinline void ApplyStretchInverseConstraint( C_Particle & p1, C_Particle & p2 )
	{
		ApplyInverseConstraint( p1, p2, GetStretchCriticalDeformationRate(), GetStretchNaturalLength() );
	}


	__forceinline void ApplyShearInverseConstraint( C_Particle & p1, C_Particle & p2 )
	{
		ApplyInverseConstraint( p1, p2, GetShearCriticalDeformationRate(), GetShearNaturalLength() );
	}


	void ApplyInverseConstraints();


	__forceinline const hv CalculateLinkNormal(S32 xStart, S32 yStart, S32 xEnd, S32 yEnd) const
	{
		hv vector = Particle(xEnd, yEnd).position - Particle(xStart, yStart).position;
		return vector.Normalised().Normal();
	}


	// Calculates a tangent to the sheet at a point, in the 'x' grid-direction.
	__forceinline const hv CalculateTangentX(S32 x, S32 y) const
	{
		hv sum;

		if ( x != 0 )
			sum += CalculateLinkNormal(x-1, y, x, y);

		if ( x != m_nX-1 )
			sum += CalculateLinkNormal(x, y, x+1, y);

		sum.Normalise();
		return sum;
	}


	__forceinline const hv CalculateTriangleNormal(S32 xOrigin, S32 yOrigin, S32 xOffset0, S32 yOffset0, S32 xOffset1, S32 yOffset1) const
	{
		hv a = Particle(xOrigin, yOrigin).position;
		hv b = Particle(xOrigin + xOffset0, yOrigin + yOffset0).position;
		hv c = Particle(xOrigin + xOffset1, yOrigin + yOffset1).position;

		hv vector = (b-a) ^ (c-a);
		vector.Normalise();
		return vector;
	}


	__forceinline void UpdateNormal( S32 x, S32 y )
	{
		hv sum;

		if ( x != 0 && y != 0 )
			sum += CalculateTriangleNormal(x, y, -1, 0, 0, -1);

		if ( x != m_nX-1 && y != 0 )
			sum += CalculateTriangleNormal(x, y, 0, -1, 1, 0);

		if ( x != 0 && y != m_nY-1 )
			sum += CalculateTriangleNormal(x, y, 0, 1, -1, 0);

		if ( x != m_nX-1 && y != m_nY-1 )
			sum += CalculateTriangleNormal(x, y, 1, 0, 0, 1);

		sum.Normalise();
		Particle(x, y).normal = sum;
	}


	void UpdateNormals();
	void UpdateNormals_Fast();
	void UpdateWind( const hf & dt );


	void DebugRenderLink( DbUtil::I_DebugRenderPtr debugRenderPtr, const C_Particle & p1,
		const C_Particle & p2 ) const;


//************//


	S32 m_nX;
	S32 m_nY;
	Ut::Vector<C_Particle> m_particleArray;

	hf m_particleMass;

	hf m_stretchSpringConst;
	hf m_shearSpringConst;
	hf m_bendSpringConst;

	hf m_stretchDampCoeff;
	hf m_shearDampCoeff;
	hf m_bendDampCoeff;

	hf m_stretchNaturalLength;
	hf m_shearNaturalLength;
	hf m_bendNaturalLength;

	hf m_windResistanceCoeff;

	hf m_gravity;

	hv m_windVelocity;

	//
	// Verlet cloth.
	//
	struct Constraint
	{
		U16		particle1;
		U16		particle2;
		F32		distance;
		F32		strength;
		// stretch, bend, shear
	};

	typedef Ut::Vector<Constraint>				ConstraintVector;

	ConstraintVector		m_constraints;
	U32						m_numStretchConstraints;
	U32						m_numShearConstraints;
	U32						m_numBendConstraints;


};

//-----------------------------------------------------------------------------------
};
};

#endif
