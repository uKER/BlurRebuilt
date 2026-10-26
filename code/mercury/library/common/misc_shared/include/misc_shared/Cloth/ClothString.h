#ifndef CLOTHSTRING_H
#define CLOTHSTRING_H

namespace Cloth {
//-------------------------------------------------------------------------------


#define CLOTH_FORSTRING for ( iv loc(0); loc.x < m_size.x; ++loc.x )

#define CLOTH_FORSTRINGCROP(cropX) for ( iv loc(0); loc.x < m_size.x - cropX; ++loc.x )


//-------------------------------------


struct C_StringInitParams : public C_BaseInitParams
{
	hv startPos;
	hv endPos;
};


class C_String : public C_Base<C_String>
{
public:

	typedef C_StringInitParams InitParams;


	// Returns number of particles in the string.
	S32 GetSize() const {return m_size.x;}

	const Maths::HVec3 & GetParticlePosition(S32 x) const { return Particle( iv(x) ).position; }
	const Maths::HVec3 GetParticleTangent(S32 x) const { return CalculateTangentX( iv(x) ); }

	const Maths::C_Quaternion GetParticleOrientation(S32 x) const { return Orientation( iv(x) ); }

	void ApplyImpulse(S32 x, const Maths::C_ImpulseKgMpS & impulse) { Base::ApplyImpulse( iv(x), impulse ); }

	void SetPin(S32 x, const Maths::HVec3 & pinPosition) { Base::SetPin( iv(x), pinPosition ); }
	void ClearPin(S32 x) { Base::ClearPin( iv(x) ); }


//***************************************************************************************//
private:

	template <class T>
	friend class C_WorldBase;


	void ApplyWindResistance(const iv & loc)
	{
		C_Particle & p = Particle(loc);

		hv force = GetWindResistanceCoeff() * (GetWindVelocity() - p.velocity);
		hv tangent = CalculateTangentX(loc);
		force -= (force | tangent) * tangent;

		ApplyForce( p, force );
	}


	void Init(const InitParams & initParams);
	void DoMinorUpdate( const hf & dt );
	void DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr ) const;


	hq & Orientation(const iv & loc)
	{
		return m_orientationArray[ loc.GetArrayIndex(m_size) ];
	}


	const hq & Orientation(const iv & loc) const
	{
		return m_orientationArray[ loc.GetArrayIndex(m_size) ];
	}


	void UpdateOrientation(const iv & loc)
	{
		hf zero;
		hf half(c_ScalarHalf);
		hf one(c_ScalarOne);

		hq & orientation = Orientation(loc);
		hv zVector = Maths::C_Normal::AxisPositiveZ() * orientation;
		hv tangent = CalculateTangentX(loc);

		hv crossProduct = tangent ^ zVector;
		hf crossProductLength = Length( crossProduct );
		if (crossProductLength > zero)
		{
			hf cosTheta = zVector | tangent;
			hf sinHalfTheta = ( (one - cosTheta) * half ).Sqrt();
			hf cosHalfTheta = ( (one + cosTheta) * half ).Sqrt();

			hv rotationNormal = crossProduct / crossProductLength;
			hq rotation( rotationNormal * sinHalfTheta, Maths::HVecW(cosHalfTheta) );

			orientation = (rotation * orientation).Normalised();
		}


		if ( loc.x != 0 )
		{
			hq rotation = Orientation(loc.PlusX(-1)) * !orientation;
			rotation = hq( (rotation.Imag() | tangent) * tangent, Maths::HVecW(one) ).Normalised();
			orientation = rotation * orientation;
		}
	}


	void UpdateOrientations()
	{
		CLOTH_FORSTRING
			UpdateOrientation(loc);
	}


	void DebugRenderOrientation( DbUtil::I_DebugRenderPtr debugRenderPtr, const iv & loc ) const;

		
//************//

	Ut::Vector<hq> m_orientationArray;
};


//-------------------------------------------------------------------------------
};

#endif
