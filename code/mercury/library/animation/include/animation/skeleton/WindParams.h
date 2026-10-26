#ifndef ANIMATION_WINDPARAMS_H
#define ANIMATION_WINDPARAMS_H

#include <core/maths/hvec/new/Vec4F32.h>
#include <core/maths/hvec/new/Vector3.h>


namespace Animation
{

/**
Parameters controlling the wind simulation in C_SkeletonInstance.

@see C_SkeletonInstance::ApplyWindForces
*/
class ALIGN16 C_WindParams
{
public:
	friend class C_WindGenerator;

	C_WindParams();
	
	Maths::Vector3	GetWindDirection() const					{ return m_windDirection; }
	void			SetWindDirection(const Maths::Vector3& dir) { m_windDirection = dir; }
	void			SetWindDirection(F32 x, F32 y, F32 z)		{ m_windDirection = Maths::MakeVector3(x, y, z); }

	F32				GetWindDirectionVariation() const			{ return m_windConstants.x; }
	void			SetWindDirectionVariation(F32 variation)	{ m_windConstants.x = variation; }
	
	F32				GetWindStrength() const						{ return m_windConstants.y; }
	void			SetWindStrength(F32 strength)				{ m_windConstants.y = strength; }

	F32				GetWindStrengthVariation() const			{ return m_windConstants.z; }
	void			SetWindStrengthVariation(F32 variation)		{ m_windConstants.z = variation; }

	F32				GetWindVariationRate() const				{ return m_windConstants.w; }
	void			SetWindVariationRate(F32 rate)				{ m_windConstants.w = rate; }

	Maths::Vec4F32	GetWindConstants() const					{ return Maths::Load<Maths::Vec4F32>(&m_windConstants); }

private:
	Maths::Vector3			m_windDirection;
	MathsStorage::Float4A16	m_windConstants;	// directionVariation, strength, strengthVariation, variationRate
};

} // end Animation

#endif
