#include "stdafx.h"

#include "ui/Game/UIMaths.h"

namespace UI
{

// other stuff

void CatmullRom::Init( )
{
	m_Closed = true;
}

void CatmullRom::AddControlPoint(const U32 controlPoint, const Maths::HVec3& in)
{
	assert(controlPoint < m_TotalControlPoints);

	m_ControlPoints[controlPoint] = in;
}

void CatmullRom::Solve(const F32 t)
{
	F32 s = 0;
	Maths::HVec3 v[4];

	if(m_Closed)
	{
		F32 x = t * (F32)m_TotalControlPoints;
		s = floor(x-1.0f);
		S32 idx = (S32)s;
		s = fmod(x,1.0f);
		if(idx<0)
			idx += m_TotalControlPoints;

		v[0] = m_ControlPoints[idx];

		for(U32 i = 0; i < 4-1; i++)
		{
			idx++;
			if(idx >= (S32)m_TotalControlPoints)
				idx -= (S32)m_TotalControlPoints;

			v[i+1] = m_ControlPoints[idx];
		}
		
	}
	else
	{
		LOGWARN( g_Log, "Not yet implemented");
		return;
	}

	Solve(v[0],v[1],v[2],v[3],s);
}

void CatmullRom::Solve(const Maths::HVec3& v0,const Maths::HVec3& v1,const Maths::HVec3& v2,const Maths::HVec3& v3,const F32 s)
{
	m_Result=v1*2.f;
	m_Result+=(-v0+v2)*s;
	m_Result+=((v0*2.f)-(v1*5.f)+(v2*4.f)-v3) * (s*s);
	m_Result+=(-v0+(v1*3.f)-(v2*3.f)+v3) * (s*s*s);
	m_Result=m_Result*0.5f;
}

void UISpline::Solve(float iter)
{
	float mum1,mum13,mu3;

	mum1 = 1 - iter;
	mum13 = mum1 * mum1 * mum1;
	mu3 = iter * iter * iter;

	x = mum13*x1 + 3*iter*mum1*mum1*cx1 + 3*iter*iter*mum1*cx2 + mu3*x2;
	y = mum13*y1 + 3*iter*mum1*mum1*cy1 + 3*iter*iter*mum1*cy2 + mu3*y2;
	z = mum13*z1 + 3*iter*mum1*mum1*cz1 + 3*iter*iter*mum1*cz2 + mu3*z2;
}

} // namespace UI
