
#include "stdafx.h"
#include "camera2/CameraType.h"
#include "camera2/stock/ShakeCamera.h"

namespace Cam2
{

C_ShakeCamera::C_ShakeCamera()
{
	SetType(E_ShakeCamera);
	SetName("Shake");
	m_shakePos = 0;
	SetParam(0.f, 0.f);
}

void C_ShakeCamera::SetParam(F32 amount, F32 freq)
{
	SetParamXYZ(amount, amount, amount, freq);
}

void C_ShakeCamera::SetParamXYZ(F32 ax, F32 ay, F32 az, F32 freq)
{
	m_shakeAmountX = ax;
	m_shakeAmountY = ay;
	m_shakeAmountZ = az;
	m_shakeFreq = freq;
}

// Predictable random number generator for input 'x' (output is -1 to +1)
static F32 PseudoNoise(int x)
{
	x = (x << 13) ^ x;
	x = x * (x * x * 19379 + 819233) + 1266122899;
	return ((x & 0x7fffffff) / 1073741823.5f) - 1.f;
}

// Interpolate between a and b where t is the amount (0 to 1)
static F32 Interp(F32 a, F32 b, F32 t)
{
	t = (1.f - cos(t * Maths::Pi)) * 0.5f; // Change t to a curve
	return a * (1.f - t) + b * t;
}

// Soft Perlin noise
F32 SoftNoise(int x)
{
	F32 a = PseudoNoise( x >> 16     );
	F32 b = PseudoNoise((x >> 16) + 1);
	return Interp(a, b, (x & 0xffff) / 65536.f);
}

void C_ShakeCamera::OnUpdate()
{
	F32 ax = Maths::DegToRad((F32)m_shakeAmountX) * 0.5f;
	F32 ay = Maths::DegToRad((F32)m_shakeAmountY) * 0.5f;
	F32 az = Maths::DegToRad((F32)m_shakeAmountZ) * 0.5f;

	F32 ra = SoftNoise(m_shakePos           ) * ax;
	F32 rb = SoftNoise(m_shakePos + 0x234567) * ay;
	F32 rc = SoftNoise(m_shakePos + 0x566757) * az;

	Maths::C_Quaternion orient(sin(ra), 0.f, 0.f, cos(ra));
	orient = orient * Maths::C_Quaternion(0.f, sin(rb), 0.f, cos(rb));
	orient = orient * Maths::C_Quaternion(0.f, 0.f, sin(rc), cos(rc));
	SetOrient(orient);

	m_shakePos += (S32)(Period() * 65536.f * m_shakeFreq);
}


} // End of namespace Cam2
