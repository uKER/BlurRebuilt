#ifndef DYNAMICSCONTEXT_H
#define DYNAMICSCONTEXT_H

#include <animation/skeleton/WindParams.h>

namespace Animation
{
	class C_SkeletonInstance;
}

class C_DynamicsContext
{
public:
	C_DynamicsContext();
	
	void SetDefaults();
	
	void SetGravity(const Maths::Vector3& g)			{ m_gravity = g; }
	const Maths::Vector3& GetGravity() const			{ return m_gravity; }

	void SetWindEnabled(bool enabled)					{ m_windEnabled = enabled; }
	bool IsWindEnabled() const							{ return m_windEnabled; }

	void SetWindDirection(const Maths::Vector3& dir)	{ m_windParams.SetWindDirection(dir); }
	Maths::Vector3 GetWindDirection() const				{ return m_windParams.GetWindDirection(); }
	void SetWindDirectionVariation(F32 v)				{ m_windParams.SetWindDirectionVariation(v); }
	F32 GetWindDirectionVariation() const				{ return m_windParams.GetWindDirectionVariation(); }
	void SetWindStrength(F32 s)							{ m_windParams.SetWindStrength(s); }
	F32 GetWindStrength() const							{ return m_windParams.GetWindStrength(); }
	void SetWindStrengthVariation(F32 v)				{ m_windParams.SetWindStrengthVariation(v); }
	F32 GetWindStrengthVariation() const				{ return m_windParams.GetWindStrengthVariation(); }
	void SetWindVariationRate(F32 rate)					{ m_windParams.SetWindVariationRate(rate); }
	F32 GetWindVariationRate() const					{ return m_windParams.GetWindVariationRate(); }
	void ShowWindVector(bool show)						{ m_showWindVector = show; }
	bool ShowWindVector() const							{ return m_showWindVector; }

	void SetBuoyancyEnabled(bool enabled)				{ m_buoyancyEnabled = enabled; }
	bool IsBuoyancyEnabled() const						{ return m_buoyancyEnabled; }
	void SetWaterPlaneHeight(F32 h)						{ m_waterPlaneHeight = h; }
	F32 GetWaterPlaneHeight() const						{ return m_waterPlaneHeight; }
	
	void SetDynamicJointsEnabled(bool enabled)			{ m_dynamicJointsEnabled = enabled; }
	bool AreDynamicJointsEnabled() const				{ return m_dynamicJointsEnabled; }
	
	
	void Reset();
	void Update(Animation::C_SkeletonInstance& skeletonInstance, const Maths::Mat43& modelWorldMatrix, F32 deltaTime);
	
private:

	Maths::Vector3 GetWindForce(U32 seed) const;

	Maths::Vector3			m_gravity;
	Animation::C_WindParams	m_windParams;
	F32						m_waterPlaneHeight;
	bool					m_dynamicJointsEnabled;
	bool					m_buoyancyEnabled;
	bool					m_windEnabled;
	bool					m_showWindVector;
	bool					m_reset;
};


#endif
