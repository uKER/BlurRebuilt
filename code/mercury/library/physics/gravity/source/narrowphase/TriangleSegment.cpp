#include "stdafx.h"

namespace Collision
{
	//using namespace Maths;

	bool IntersectTriangleBoxSegment(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q,   const Maths::C_Normal PCREF normal, Maths::HVec3& res, F32& pDepth)
	{
		using namespace Maths;
		const HF32 epsilon(Maths::Constants<HF32>::Epsilon());
		const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
		const HVec3 ab(b-a);
		const HVec3 ac(c-a);
		const HVec3 n(ab^ac);
		const HVec3 qp(p-q);
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());

		const HF32 d(qp|n);

		if(d.Abs().LE(epsilon))
			return false;
		/*if(d.GT(Maths::Constants<Maths::HF32>::Epsilon2()))
			return false;*/

		const HF32 recipD(d.Reciprocal());

		const HVec3 ap(p-a);

		//t is the parameter of a point in the segment
		const HF32 ttemp(ap|n);
		const HF32 t(ttemp*recipD);

		//v, w, u(1-v-w) is the barycentric coordinate of a point in the triangle
		const HVec3 e(qp^ap);
		const HF32 vtemp(ac|e);
		const HF32 v(vtemp*recipD);

		const HF32 wtemp(-(ab|e));
		const HF32 w(wtemp*recipD);

		res=p - qp*t;
		/*const Maths::HVec3 dir1(q - res);
		const Maths::HVec3 dir2(p - res);
		const HF32 sign1(dir1|normal);
		const HF32 sign2(dir2|normal);
		const Maths::Scalar_Bool sign(sign1.LT(zero));
		pDepth=Select(sign, sign1.Abs(), sign2.Abs());
		pDepth = Maths::Abs( (qp * t) | normal);*/

		//Calculate pen depht = Abs(n | q - d)
		const Maths::HF32 dd(a | normal);

		const Maths::HF32 pDepthP( ((p | normal) - dd));
		const Maths::HF32 pDepthQ( ((q | normal) - dd));

		const Maths::HF32 pFinalDepth(Maths::Abs( Maths::Min(pDepthP, pDepthQ)));

		const Maths::Scalar_Bool con1(t.GE(zero) & t.LE(one));
		const Maths::Scalar_Bool con2(v.GE(zero) & v.LE(one));
		const Maths::Scalar_Bool con3(w.GE(zero) & (v+w).LE(one));
		Maths::Store(pFinalDepth, &pDepth);
		return (con1 & con2 & con3);

	}
};
