#ifndef CLOTHSHEET_H
#define CLOTHSHEET_H

namespace Cloth {
//-------------------------------------------------------------------------------


#define CLOTH_FORSHEET for ( iv loc(0, 0); loc.y < m_size.y; ++loc.y ) for ( loc.x = 0; loc.x < m_size.x; ++loc.x )

#define CLOTH_FORSHEETCROP(cropX, cropY) for ( iv loc(0, 0); loc.y < m_size.y - cropY; ++loc.y ) \
	for ( loc.x = 0; loc.x < m_size.x - cropX; ++loc.x )


//-------------------------------------


struct C_SheetInitParams : public C_BaseInitParams
{
	Maths::HMat43 worldMatrix;
	F32 length;
	F32 width;

	F32 shearSpringConst;
	F32 shearDampCoeff;

	bool pinOnSide;	// if true, pins the sheet along its side rather than along its top
};


class C_Sheet : public C_Base<C_Sheet>
{
public:

	typedef C_SheetInitParams InitParams;


	// Returns size of sheet-grid.
	S32 GetSizeX() const {return m_size.x;}
	S32 GetSizeY() const {return m_size.y;}

	// Particle positions and normals.
	const Maths::HVec3 & GetParticlePosition(S32 x, S32 y) const { return Particle( iv(x, y) ).position; }
	const Maths::HVec3 GetParticleNormal(S32 x, S32 y) const { return CalculateNormal( iv(x, y) ); }

	// Tangent in the 'x' direction on the sheet-grid.
	const Maths::HVec3 GetParticleTangent(S32 x, S32 y) const { return CalculateTangentX( iv(x, y) ); }

	void ApplyImpulse(S32 x, S32 y, const Maths::C_ImpulseKgMpS & impulse) { Base::ApplyImpulse( iv(x, y), impulse ); } 


//***************************************************************************************//
private:

	template <class T>
	friend class C_WorldBase;


#ifdef CLOTH_USE_SCALING_FACTORS
	const hf GetShearSpringConst() const { return m_shearSpringConst * g_DebugShearSpringConstScalingFactor; }
	const hf GetShearDampCoeff() const { return m_shearDampCoeff * g_DebugShearDampCoeffScalingFactor; }
#else
	const hf GetShearSpringConst() const { return m_shearSpringConst; }
	const hf GetShearDampCoeff() const { return m_shearDampCoeff; }
#endif

	const hf GetShearNaturalLength() const { return m_shearNaturalLength; }
	const hf GetStretchNaturalLengthY() const { return m_stretchNaturalLengthY; }
	const hf GetBendNaturalLengthY() const { return m_bendNaturalLengthY; }


	void ApplyShearForce( C_Particle & p1, C_Particle & p2 )
	{
		ApplySpringForce( p1, p2, GetShearSpringConst(), GetShearDampCoeff(), GetShearNaturalLength() );
	}


	void ApplyStretchForceY( C_Particle & p1, C_Particle & p2 )
	{
		ApplySpringForce( p1, p2, GetStretchSpringConst(), GetStretchDampCoeff(), GetStretchNaturalLengthY() );
	}


	void ApplyBendForceY( C_Particle & p1, C_Particle & p2 )
	{
		ApplySpringForce( p1, p2, GetBendSpringConst(), GetBendDampCoeff(), GetBendNaturalLengthY() );
	}


	const hv CalculateTriangleNormal(const iv & origin, S32 xOffset, S32 yOffset ) const
	{
		hv a = Particle(origin).position;
		hv b = Particle( origin.PlusX(xOffset) ).position;
		hv c = Particle( origin.PlusY(yOffset) ).position;

		hv vector = (b-a) ^ (c-a);
		vector *= F32(xOffset * yOffset);
		//return vector.Normalised().Normal();
		return Normalise( vector );
	}


	const hv CalculateNormal(const iv & loc) const
	{
		hv sum;

		if ( loc.x != 0 && loc.y != 0 )
			sum += CalculateTriangleNormal(loc, -1, -1);

		if ( loc.x != m_size.x-1 && loc.y != 0 )
			sum += CalculateTriangleNormal(loc, 1, -1);

		if ( loc.x != 0 && loc.y != m_size.y-1 )
			sum += CalculateTriangleNormal(loc, -1, 1);

		if ( loc.x != m_size.x-1 && loc.y != m_size.y-1 )
			sum += CalculateTriangleNormal(loc, 1, 1);

		//return sum.Normalised().Normal();
		return Normalise( sum );
	}


	void ApplyWindResistance(const iv & loc)
	{
		C_Particle & p = Particle(loc);

		hv force = GetWindResistanceCoeff() * (GetWindVelocity() - p.velocity);
		hv normal = CalculateNormal(loc);
		force = (force | normal) * normal;

		ApplyForce( p, force );
	}


	void DebugRenderNormal( DbUtil::I_DebugRenderPtr debugRenderPtr, const iv & loc ) const
	{
		hv start = Particle(loc).position;
		hv end = start + CalculateNormal(loc) * g_DebugDebugRenderNormalLength;
		debugRenderPtr->Draw3dLine( start, end, c_DebugRenderNormalColour );
	}

		
	void Init(const InitParams & initParams);
	void DoMinorUpdate( const hf & dt );
	void DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr ) const;

	void UpdateOrientations() {}
	// Empty inline function.


//************//

	hf m_shearSpringConst;
	hf m_shearDampCoeff;
	hf m_shearNaturalLength;

	// The natural lengths in the x and y directions may be slightly different.
	hf m_stretchNaturalLengthY;
	hf m_bendNaturalLengthY;
};


//-------------------------------------------------------------------------------
};

#endif
