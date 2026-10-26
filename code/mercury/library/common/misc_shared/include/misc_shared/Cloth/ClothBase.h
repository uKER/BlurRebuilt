#ifndef CLOTHBASE_H
#define CLOTHBASE_H

namespace Cloth {
//-------------------------------------------------------------------------------


#ifndef CFG_SPEED
#define CLOTH_USE_SCALING_FACTORS
#endif


//-------------------------------------


class C_String;
class C_Sheet;


typedef Maths::HF32 hf;
typedef Maths::HVec3 hv;
typedef Maths::C_Quaternion hq;


const Maths::RGBA c_DebugRenderLinkColour( 0xff, 0xff, 0xff );
const Maths::RGBA c_DebugRenderNormalColour( 0xff, 0, 0 );
const Maths::RGBA c_DebugRenderTangentColour( 0, 0xff, 0 );


extern Debug::Logger & g_Log;


extern Debug::Bool g_DebugDebugRender;
extern Debug::Float g_DebugDebugRenderNormalLength;
extern Debug::Float g_DebugDebugRenderTangentLength;
extern Debug::Float g_DebugDebugRenderOrientationMatrixSize;

#ifdef CLOTH_USE_SCALING_FACTORS
extern Debug::Float g_DebugMaxMinorTimeStepScalingFactor;
extern Debug::Float g_DebugFrictionCoeffScalingFactor;
extern Debug::Float g_DebugWindVelocityScalingFactor;
extern Debug::Float g_DebugWindResistanceCoeffScalingFactor;
extern Debug::Float g_DebugBendDampCoeffScalingFactor;
extern Debug::Float g_DebugBendSpringConstScalingFactor;
extern Debug::Float g_DebugShearDampCoeffScalingFactor;
extern Debug::Float g_DebugShearSpringConstScalingFactor;
extern Debug::Float g_DebugStretchDampCoeffScalingFactor;
extern Debug::Float g_DebugStretchSpringConstScalingFactor;
extern Debug::Float g_DebugParticleMassScalingFactor;
#endif

extern Debug::Float g_DebugGravitationalFieldStrength;


//-------------------------------------


class C_Particle
{
public:
	hv position;
	hv velocity;
	hv force;
	bool isPinned;
};


template <class T>
class C_IntVector
{
private:
	C_IntVector();
};


template <>
class C_IntVector<C_String>
{
public:
	S32 x;

	C_IntVector() {}

	C_IntVector(S32 _x): x(_x) {}

	S32 GetArrayIndex(const C_IntVector<C_String> & arraySize) const
	{
		assert(x >= 0 && x < arraySize.x);
		return x;
	}

	const C_IntVector<C_String> PlusX(S32 rhs) const {return C_IntVector<C_String>(x + rhs);}
};


template <>
class C_IntVector<C_Sheet>
{
public:
	S32 x;
	S32 y;

	C_IntVector() {}

	C_IntVector(S32 _x, S32 _y): x(_x), y(_y) {}

	S32 GetArrayIndex(const C_IntVector<C_Sheet> & arraySize) const
	{
		assert(x >= 0 && x < arraySize.x);
		assert(y >= 0 && y < arraySize.y);
		return x + y * arraySize.x;
	}

	const C_IntVector<C_Sheet> PlusX(S32 rhs) const {return C_IntVector<C_Sheet>(x + rhs, y);}
	const C_IntVector<C_Sheet> PlusY(S32 rhs) const {return C_IntVector<C_Sheet>(x, y + rhs);}
	const C_IntVector<C_Sheet> Plus(S32 _x, S32 _y) const {return C_IntVector<C_Sheet>(x + _x, y + _y);}
};


struct C_WindInitParams
{
	hv direction;	// must be normalised
	F32 averageSpeed;
	F32 maxSpeedDeviation;
	F32 noiseTimeScale;
};


class C_WindGenerator
{
public:
	void Init(const C_WindInitParams & initParams);
	void Update(const hf & dt);
	const hv & GetVelocity() const { return m_velocity; }

private:

	// Parameters
	hv m_direction;
	hf m_averageSpeed;	
	hf m_maxSpeedDeviation;
	hf m_noiseTimeScale;

	// Variables
	hf m_time;
	hv m_velocity;
};


// You should derive your external-data structures from this base class.
struct C_ExternalDataBase {};


struct C_BaseInitParams
{
	F32 particleSeparation;	// natural distance between adjacent particles
	F32 particleMass;

	F32 stretchSpringConst;
	F32 stretchDampCoeff;

	F32 bendSpringConst;
	F32 bendDampCoeff;

	F32 windResistanceCoeff;
	C_WindInitParams windInitParams;

	F32 rumpleFactor;	// distance across string/sheet divided by distance across pins

	S32 pinSeparation;
	// No. particles between successive pins (may be zero, in which case only the particle at loc = 0 is pinned).

	F32 maxMinorTimeStep;
	F32 forceLimit;	// max force per particle (excluding collision forces)

	Ut::Vector<Maths::C_Plane> collisionPlaneArray;
	F32 frictionCoeff;	// used for friction against collision planes

	C_ExternalDataBase* pExternalData;


	C_BaseInitParams(): pExternalData(NULL), forceLimit(FLT_MAX) {}
};


template <class T>
class C_Base
{
public:

	typedef Ut::UtList<T, Ut::DoubleEndedList, Ut::MaintainedCounter, Ut::ListElementDouble> List;


	C_ExternalDataBase* GetExternalData() const {return m_pExternalData;}

	bool IsActive() const { return m_isActive; }
	void Activate(bool flag) {m_isActive = flag;}
	// Note - cloth objects are active by default.

	const hf GetParticleSeparation() const { return GetStretchNaturalLength(); }

	// Returns the previous value of the wind-resistance coeff.
	F32 SetWindResistanceCoeff(F32 val) {
		F32 ret = m_windResistanceCoeff; m_windResistanceCoeff = val; return ret; }


protected:

	typedef C_Base<T> Base;

	typedef C_IntVector<T> iv;


	C_Particle & Particle(const iv & loc)
	{
		return m_particleArray[ loc.GetArrayIndex(m_size) ];
	}


	const C_Particle & Particle(const iv & loc) const
	{
		return m_particleArray[ loc.GetArrayIndex(m_size) ];
	}


#ifdef CLOTH_USE_SCALING_FACTORS
	const hf GetParticleMass() const { return m_particleMass * g_DebugParticleMassScalingFactor; }
	const hf GetStretchSpringConst() const { return m_stretchSpringConst * g_DebugStretchSpringConstScalingFactor; }
	const hf GetStretchDampCoeff() const { return m_stretchDampCoeff * g_DebugStretchDampCoeffScalingFactor; }
	const hf GetBendSpringConst() const { return m_bendSpringConst * g_DebugBendSpringConstScalingFactor; }
	const hf GetBendDampCoeff() const { return m_bendDampCoeff * g_DebugBendDampCoeffScalingFactor; }
	const hf GetWindResistanceCoeff() const { return m_windResistanceCoeff * g_DebugWindResistanceCoeffScalingFactor; }
	const hv GetWindVelocity() const { return m_windGenerator.GetVelocity() * g_DebugWindVelocityScalingFactor; }
	const hf GetFrictionCoeff() const { return m_frictionCoeff * g_DebugFrictionCoeffScalingFactor; }
	const hf GetMaxMinorTimeStep() const { return m_maxMinorTimeStep * g_DebugMaxMinorTimeStepScalingFactor; }
#else
	const hf GetParticleMass() const { return m_particleMass; }
	const hf GetStretchSpringConst() const { return m_stretchSpringConst; }
	const hf GetStretchDampCoeff() const { return m_stretchDampCoeff; }
	const hf GetBendSpringConst() const { return m_bendSpringConst; }
	const hf GetBendDampCoeff() const { return m_bendDampCoeff; }
	const hf GetWindResistanceCoeff() const { return m_windResistanceCoeff; }
	const hv GetWindVelocity() const { return m_windGenerator.GetVelocity(); }
	const hf GetFrictionCoeff() const { return m_frictionCoeff; }
	const hf GetMaxMinorTimeStep() const { return m_maxMinorTimeStep; }
#endif

	const hf GetStretchNaturalLength() const { return m_stretchNaturalLength; }
	const hf GetBendNaturalLength() const { return m_bendNaturalLength; }


	void Init(C_Particle & p, const hv & position, bool isPinned)
	{
		p.position = position;
		p.velocity = hv();
		p.force = hv();
		p.isPinned = isPinned;
	}


	void Init(const C_BaseInitParams & initParams)
	{
		m_particleMass = initParams.particleMass;

		m_stretchSpringConst = initParams.stretchSpringConst;
		m_stretchDampCoeff = initParams.stretchDampCoeff;

		m_bendSpringConst = initParams.bendSpringConst;
		m_bendDampCoeff = initParams.bendDampCoeff;

		m_windResistanceCoeff = initParams.windResistanceCoeff;
		m_windGenerator.Init(initParams.windInitParams);

		m_maxMinorTimeStep = initParams.maxMinorTimeStep;
		if (m_maxMinorTimeStep <= 0.0f)
		{
			m_maxMinorTimeStep = 0.02f;
			LOGWARN( g_Log, "Max minor time-step was zero or negative when initialising a string/sheet of "
				"cloth - defaulting to 0.02f." );
		}

		m_forceLimit = initParams.forceLimit;
		m_isActive = true;

		m_collisionPlaneArray = initParams.collisionPlaneArray;
		m_frictionCoeff = initParams.frictionCoeff;

		m_pExternalData = initParams.pExternalData;
	}


	void ResetForce( C_Particle & p )
	{
		p.force = hv();
	}


	// Applies a force to a single particle.
	void ApplyForce( C_Particle & p, const hv & force )
	{
		p.force += force;
	}


	// Applies a force between two particles, with positive sign for p1.
	void ApplyForce( C_Particle & p1, C_Particle & p2, const hv & force )
	{
		ApplyForce(p1, force);
		ApplyForce(p2, -force);
	}


	// Hooke's Law plus damping.
	void ApplySpringForce( C_Particle & p1, C_Particle & p2, const hf & springConst, const hf & dampCoeff,
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


	void ApplyStretchForce( C_Particle & p1, C_Particle & p2 )
	{
		ApplySpringForce( p1, p2, GetStretchSpringConst(), GetStretchDampCoeff(), GetStretchNaturalLength() );
	}


	void ApplyBendForce( C_Particle & p1, C_Particle & p2 )
	{
		ApplySpringForce( p1, p2, GetBendSpringConst(), GetBendDampCoeff(), GetBendNaturalLength() );
	}


	void ApplyGravity( C_Particle & p )
	{
		ApplyForce( p, GetParticleMass() * g_DebugGravitationalFieldStrength * Maths::C_Normal::AxisNegativeY() );
	}


	void LimitForce( C_Particle & p )
	{
		p.force.LimitLength( m_forceLimit );
	}


	void Step( C_Particle & p, const hf & dt )
	{
		if ( !p.isPinned )
		{
			p.velocity += p.force / GetParticleMass() * dt;
			p.position += p.velocity * dt;
		}
	}


	const hv CalculateLinkNormal(const iv & start, const iv & end) const
	{
		hv vector = Particle(start).position - Particle(end).position;
		//return vector.Normalised().Normal();
		return Normalise( vector );
	}


	// Calculates a tangent to the string/sheet at a point, in the 'x' grid-direction.
	const hv CalculateTangentX(const iv & loc) const
	{
		hv sum;

		if ( loc.x != 0 )
			sum += CalculateLinkNormal(loc.PlusX(-1), loc);

		if ( loc.x != m_size.x-1 )
			sum += CalculateLinkNormal(loc, loc.PlusX(1));

		//return sum.Normalised().Normal();
		return Normalise( sum );
	}


	void UpdateWind(const hf & dt)
	{
		m_windGenerator.Update(dt);
	}


	void DebugRenderLink( DbUtil::I_DebugRenderPtr debugRenderPtr, const C_Particle & p1,
		const C_Particle & p2 ) const
	{
		debugRenderPtr->Draw3dLine( p1.position, p2.position, c_DebugRenderLinkColour );
	}


	void DebugRenderTangent( DbUtil::I_DebugRenderPtr debugRenderPtr, const iv & loc ) const
	{
		hv start = Particle(loc).position;
		hv end = start + CalculateTangentX(loc) * g_DebugDebugRenderTangentLength;
		debugRenderPtr->Draw3dLine( start, end, c_DebugRenderTangentColour );
	}


	void ApplyImpulse( const iv & loc, const Maths::C_ImpulseKgMpS & impulse )
	{
		C_Particle & p = Particle(loc);
		if (!p.isPinned)
			p.velocity += impulse.Get() / m_particleMass;
	}


	void DoCollisions( C_Particle & p )
	{
		if (p.isPinned)
			return;

		hf zero;
		hv zeroVector;
		for (int i=0; i < m_collisionPlaneArray.Size(); ++i)
		{
			const Maths::C_Plane & plane = m_collisionPlaneArray[i];

			if (plane.Distance(p.position) < zero)
			{
				p.position = plane.OnPlane(p.position);

				hv planeNormal = plane.Normal();
				hf perpVelocity = p.velocity | planeNormal;
				if (perpVelocity < zero)
				{
					p.velocity -= perpVelocity * planeNormal;

					// Friction.
					hf speedReduction = GetFrictionCoeff() * -perpVelocity;
					hf velocityMagnitude = p.velocity.Length();
					if (velocityMagnitude > speedReduction)
					{
						hv velocityDirection = p.velocity / velocityMagnitude;
						p.velocity -= velocityDirection * speedReduction;
					}
					else
						p.velocity = zeroVector;
				}
			}
		}
	}


	// Pins the specified particle at the specified position.
	void SetPin( const iv & loc, const hv & position )
	{
		C_Particle & p = Particle(loc);
		p.position = position;
		p.velocity = hv();
		p.isPinned = true;
	}


	// Un-pins the specified particle.
	void ClearPin( const iv & loc )
	{
		C_Particle & p = Particle(loc);
		p.isPinned = false;
	}


//************//

	iv m_size;
	Ut::Vector<C_Particle> m_particleArray;

	hf m_particleMass;

	hf m_stretchSpringConst;
	hf m_stretchDampCoeff;
	hf m_stretchNaturalLength;

	hf m_bendSpringConst;
	hf m_bendDampCoeff;
	hf m_bendNaturalLength;

	hf m_windResistanceCoeff;
	C_WindGenerator m_windGenerator;

	hf m_maxMinorTimeStep;
	hf m_forceLimit;
	bool m_isActive;

	Ut::Vector<Maths::C_Plane> m_collisionPlaneArray;
	hf m_frictionCoeff;

	C_ExternalDataBase* m_pExternalData;
};


//-------------------------------------------------------------------------------
};

#endif
