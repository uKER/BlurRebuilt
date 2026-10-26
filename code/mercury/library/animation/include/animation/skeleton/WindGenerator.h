#ifndef ANIMATION_WINDGENERATOR_H
#define ANIMATION_WINDGENERATOR_H

namespace Animation
{

class C_WindParams;

/**
Generates wind forces that change over time.

Wind forces are deterministic so that each combination of WindParams, time
and seed values will always generate the same force.
*/

class C_WindGenerator
{
public:
	C_WindGenerator();

	void ResetTime();
	void SetTime(U32 time);
	U32 GetTime() const;

	void Update(const C_WindParams& windParams, F32 deltaTime);

	Maths::Vector3 GenerateWindForce(U32 seed) const;

private:
	static F32				SmoothNoise1d(U32 x);
	static Maths::Vector3	SmoothNoise3d(U32 x, U32 y, U32 z);

	static U32	Noise(U32 x)
	{
		x -= (x << 6);
		x ^= (x >> 17);
		x -= (x << 9);
		x ^= (x << 4);
		x -= (x << 3);
		x ^= (x << 10);
		x ^= (x >> 15);
		return x;
	}
	
	Maths::Normal3	m_windDirection;
	Maths::Vec4F32	m_windConstants;
	U32				m_time;
};

} // end Animation

#endif
