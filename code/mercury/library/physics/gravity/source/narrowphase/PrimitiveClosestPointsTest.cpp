#include <stdafx.h>
#include "gravity/narrowphase/PrimitiveTest.h"
#include "gravity/narrowphase/CylinderTriangle.h"

//using namespace Maths;

//Debug::Bool g_dbUseMathsFunction("Gravity.UseMathsRaytrace", true);
//Debug::Bool g_dbUseMathsFunctionV2("Gravity.UseMathsRaytraceV2", false);

namespace Collision
{
//calculate the closest point between point q and line ab


//	void CEClosestPtSegmentSegment(const Maths::HVec3 PCREF p1, const Maths::HVec3 PCREF q1, const Maths::HVec3 PCREF p2, const Maths::HVec3 PCREF q2, Maths::HVec3& c1, Maths::HVec3& c2)
//{
//	using namespace Maths;
//	const HVec3 d1=q1-p1;
//	const HVec3 d2=q2-p2;
//	const HVec3 r=p1-p2;
//	const HF32 a=d1|d1;
//	const HF32 e=d2|d2;
//	const HF32 f=d2|r;
//
//	const HF32 zero(Maths::Constants<HF32>::Zero());
//	const HF32 one(Maths::Constants<HF32>::One());
//	HF32 s(Maths::ZERO_INIT);
//	HF32 t(Maths::ZERO_INIT);
//	const HF32 eps(Maths::Constants<HF32>::Epsilon());
//	//const HF32 eps(EPSILON);
//	
//
//	//either or both segments degenerate into points
//	if(a.LE(eps) && e.LE(eps))
//	{
//		c1 = p1;
//		c2 = p2;
//		return;
//	}
//
//	if(a.LE(eps))//First segement degenerate into a point
//	{
//		const HF32 _t= f/e;
//		//Clamp t into [0.f, 1.f]
//		t = Clamp(_t, zero, one);
//	}
//	else
//	{
//		const HF32 c=d1|r;
//		if(e.LE(eps))//second segement degenerate into a point
//		{
//			const HF32 _s=(-c)/a;
//			//Clamp s into [0.f, 1.f]
//			s = Clamp(_s, zero, one);
//		}
//		else //the general non degenerate case
//		{
//			const HF32 b= d1|d2;
//			const HF32 denom((a*e) - (b*b));
//
//			//if the segment is not parallel, compute closest point on L1 and L2 and clamp to segment S1, otherwise, s=0.f;
//			const HF32 w(Clamp((b*f - c*e)/denom, zero, one));
//			s = Select(denom.NE(zero), w, s);
//			/*if(denom.NE(zero))
//			{
//				const HF32 w= (b*f - c*e)/denom;
//				s = Clamp(w, zero, one);
//			}*/
//
//			const HF32 _t((b*s + f)/e);
//			t=Clamp(_t, zero, one);
//			if(_t.LT(zero))
//			{
//				//t=zero;
//				const HF32 _s=(-c)/a;
//				s=Clamp(_s, zero, one);
//			}
//			else if(_t.GT(one))
//			{
//				//t=one;
//				const HF32 _s=(b-c)/a;
//				s=Clamp(_s, zero, one);
//			}
//			//s = Clamp(s, zero, one);
//
//		}
//	}
//
//	//assert(s.GE(zero) && s.LE(one));
//	//assert(t.GE(zero) && t.LE(one));
//	c1 = p1 + d1*s;
//	c2 = p2 + d2*t;
//
//}

void CEClosestPtSegmentSegment(const Maths::HVec3 PCREF p1, const Maths::HVec3 PCREF q1, const Maths::HVec3 PCREF p2, const Maths::HVec3 PCREF q2, Maths::HVec3& c1, Maths::HVec3& c2)
{
	using namespace Maths;

	const HF32 zero(Maths::Constants<HF32>::Zero());
	const HF32 one(Maths::Constants<HF32>::One());
	const HF32 eps(Maths::Constants<HF32>::Epsilon());

	HF32 s(zero);
	HF32 t(zero);

	const HVec3 d1=q1-p1;
	const HVec3 d2=q2-p2;
	const HVec3 r=p1-p2;
	const HF32 a=d1|d1;
	const HF32 e=d2|d2;
	const HF32 b= d1|d2;
	const HF32 f=d2|r;
	const HF32 c=d1|r;
	
	const HF32 denom((a*e) - (b*b));
	//const HF32 recipA(a.Reciprocal());
	//const HF32 recipE(e.Reciprocal());

	const HVec3 numerator(a, e, denom);
	const HVec3 recipNum(Maths::Reciprocal(numerator));

	const HF32 recipA(recipNum.X());
	const HF32 recipE(recipNum.Y());
	const HF32 recipDenom(recipNum.Z());

	const Scalar_Bool aGtEps(a.GT(eps));
	const Scalar_Bool eLeEps(e.LE(eps));
	
	const HF32 w(Clamp((b*f - c*e)*recipDenom, zero, one));
	s = Select(denom.NE(zero) & aGtEps, w, s);

	const HF32 _t((b*s + f)*recipE);
	t=Clamp(Select(eLeEps, t, _t), zero, one);
	const HF32 _s1=(-c)*recipA;
	const HF32 _s2=(b-c)*recipA;
	s = Clamp(Select(_t.LT(zero) | eLeEps, _s1, Select(_t.GT(one), _s2, s)), zero, one);
	c1 = p1 + d1*s;
	c2 = p2 + d2*t;
}


void ClosestPtPointTriangle2(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, Maths::HVec3& closest)
{
	using namespace Maths;
	
	const HVec3 ab(b-a);
	const HVec3 ac(c-a);
	const HVec3 ap(p-a);

	const HF32 zero(Maths::Constants<Maths::HF32>::Zero());

	const HF32 d1(ab|ap);//Dot(ab, ap);
	const HF32 d2(ac|ap);//Dot(ac, ap);

	//p is in the vertex region a, a is the closest point to p
	if(d1.LE(zero) & d2.LE(zero)) 
	{
		closest=a;
		return; 
	}

	const HVec3 bp(p-b);
	const HF32 d3(ab|bp);//Dot(ab, bp);
	const HF32 d4(ac|bp);//Dot(ac, bp);

	//p is in the vertex region b, b is the closest point to p
	if(d3.GE(zero) & d4.LE(d3))
	{
		closest=b;
		return;
	}

	const HF32 vc((d1*d4) - (d3*d2));
	//P is in the edge region of AB, return projection of P onto AB
	if(vc.LE(zero)  & d1.GE(zero) & d3.LE(zero))
	{
		closest= a + (d1/(d1-d3))*ab;
		return;
	}

	const HVec3 cp(p-c);
	const HF32 d5(ab|cp);//Dot(ab, cp);
	const HF32 d6(ac|cp);//Dot(ac, cp);
	//P is in the vertex region c, c is the closest point to p
	if(d6.GE(zero) & d5.LE(d6))
	{
		closest=c;
		return;
	}

	const HF32 vb((d5*d2) - (d1*d6));
	//P is in the edge region of AC, return projection of P onto AC
	if(vb.LE(zero) & d2.GE(zero) & d6.LE(zero))
	{
		closest=a+ (d2/(d2-d6))*ac;
		return;
	}

	const HF32 va((d3*d6) - (d5*d4));
	const HF32 d43(d4-d3);
	const HF32 d56(d5-d6);
	//P is in the edge region of BC, return the projection of P onto BC
	if(va.LE(zero) & d43.GE(zero) & d56.GE(zero))
	{	
		const HF32 w(d43/(d43+d56));
		closest= b+ w*(c-b);
		return;
	}

	//P is inside the face region, return the projection of P onto abc
	const HF32 denom((va+vb+vc).Reciprocal());
	const HF32 v(vb*denom);
	const HF32 w(vc*denom);
	closest= a + ab*v + ac*w;
	
}


void ClosestPtPointTriangle(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, Maths::HF32& u, Maths::HF32& v, Maths::HF32& w)
{
	using namespace Maths;
	const HVec3 ab(b-a);
	const HVec3 ac(c-a);
	const HVec3 ap(p-a);

	//const HF32 zero;//(-Maths::Constants<HF32>::Epsilon());

	const HF32 d1(ab|ap);//Dot(ab, ap);
	const HF32 d2(ac|ap);//Dot(ac, ap);

	const HF32 one(Maths::Constants<Maths::HF32>::One());
	const HF32 zero(Maths::Constants<Maths::HF32>::Zero());

	//p is in the vertex region a, a is the closest point to p
	if(d1.LE(zero) & d2.LE(zero)) 
	{
		u=one;//a
		v=zero;//b
		w=zero;//c
		//closest=a;
		return; 
	}

	const HVec3 bp(p-b);
	const HF32 d3(ab|bp);//Dot(ab, bp);
	const HF32 d4(ac|bp);//Dot(ac, bp);

	//p is in the vertex region b, b is the closest point to p
	if(d3.GE(zero) & d4.LE(d3))
	{
		u=zero;//a
		v=one;//b
		w=zero;//c
		//closest=b;
		return;
	}

	const HF32 vc((d1*d4) - (d3*d2));
	//P is in the edge region of AB, return projection of P onto AB
	if(vc.LE(zero)  & d1.GE(zero) & d3.LE(zero))
	{
		v=d1/(d1-d3);//b
		w=zero;//c
		u=one - v;//a
		//closest= a + (d1/(d1-d3))*ab;
		return;
	}

	const HVec3 cp(p-c);
	const HF32 d5(ab|cp);//Dot(ab, cp);
	const HF32 d6(ac|cp);//Dot(ac, cp);
	//P is in the vertex region c, c is the closest point to p
	if(d6.GE(zero) & d5.LE(d6))
	{
		u=zero;//a
		v=zero;//b
		w=one;//c
		//closest=c;
		return;
	}

	const HF32 vb((d5*d2) - (d1*d6));
	//P is in the edge region of AC, return projection of P onto AC
	if(vb.LE(zero) & d2.GE(zero) & d6.LE(zero))
	{
		w=d2/(d2-d6);//c
		u=one-w;//a
		v=zero;//b
		//closest=a+ (d2/(d2-d6))*ac;
		return;
	}

	const HF32 va((d3*d6) - (d5*d4));
	const HF32 d43(d4-d3);
	const HF32 d56(d5-d6);
	//P is in the edge region of BC, return the projection of P onto BC
	if(va.LE(zero) & d43.GE(zero) & d56.GE(zero))
	{	
		const HF32 g(d43/(d43+d56));
		//closest= b+ g*(c-b);
		u=zero;//a
		v=one-g;//b
		w=g;//c
		return;
	}

	//P is inside the face region, return the projection of P onto abc
	const HF32 denom((va+vb+vc).Reciprocal());
	const HF32 v1(vb*denom);
	const HF32 w1(vc*denom);
	//closest= a + ab*v1 + ac*w1;
	u=one-v1-w1;
	v=v1;
	w=w1;
}

void ClosestPtPointPlane(const C_Plane& plane, const Maths::HVec3 PCREF p, Maths::HVec3& closest)
{
	using namespace Maths;
	//const HF32 t = (plane.m_n|p) - plane.m_d;
	//closest = p - t*plane.m_n;
	closest = ClosestPointOnPlane( p, MakePlane( (Normal3)plane.m_n, -(VF32)plane.m_d ) );
}

//void ClosestPtPointTriangle1(const HVec3 PCREF p, const HVec3 PCREF a, const HVec3 PCREF b, const HVec3 PCREF c, HVec3& closest)
//{
//	const HVec3 ab(b-a);
//	const HVec3 ac(c-a);
//	const HVec3 ap(p-a);
//
//	const HF32 zero(Maths::Constants<HF32>::Zero());
//
//	const HF32 d1(ab|ap);//Dot(ab, ap);
//	const HF32 d2(ac|ap);//Dot(ac, ap);
//
//	//p is in the vertex region a, a is the closest point to p
//	HVec3 _closest(a);
//	const Scalar_Bool b1(d1.LE(zero) & d2.LE(zero));
//	//if(d1.LE(zero) && d2.LE(zero)) 
//	/*{
//		closest=a;
//		return; 
//	}*/
//
//	const HVec3 bp(p-b);
//	const HF32 d3(ab|bp);//Dot(ab, bp);
//	const HF32 d4(ac|bp);//Dot(ac, bp);
//
//	//p is in the vertex region b, b is the closest point to p
//	const Scalar_Bool b2(d3.GE(zero) & d4.LE(d3));
//	_closest = Select(b2, Select(b1, _closest, b), _closest);
//	const Scalar_Bool b11 = b1 | b2;
//	
//	/*{
//		closest=b;
//		return;
//	}*/
//
//	const HF32 vc((d1*d4) - (d3*d2));
//	//P is in the edge region of AB, return projection of P onto AB
//	const Scalar_Bool b3(vc.LE(zero) & d1.GE(zero) & d3.LE(zero));
//	_closest = Select(b3, Select(b1, _closest, a + (d1/(d1-d3))*ab), _closest);
//	const Scalar_Bool b12 = b11 | b3;
//	/*{
//		closest= a + (d1/(d1-d3))*ab;
//		return;
//	}*/
//
//	const HVec3 cp(p-c);
//	const HF32 d5(ab|cp);//Dot(ab, cp);
//	const HF32 d6(ac|cp);//Dot(ac, cp);
//	//P is in the vertex region c, c is the closest point to p
//	const Scalar_Bool b4(d6.GE(zero) & d5.LE(d6));
//	
//	_closest = Select(b4, Select(b1, _closest, c), _closest);
//	const Scalar_Bool b13 = b12 | b4;
//	/*{
//		closest=c;
//		return;
//	}*/
//
//	const HF32 vb((d5*d2) - (d1*d6));
//	//P is in the edge region of AC, return projection of P onto AC
//	const Scalar_Bool b5(vb.LE(zero) & d2.GE(zero) & d6.LE(zero));
//	
//	_closest = Select(b5, Select(b1, _closest, a+ (d2/(d2-d6))*ac), _closest);
//	const Scalar_Bool b14 = b13 | b5;
//	/*{
//		closest=a+ (d2/(d2-d6))*ac;
//		return;
//	}*/
//
//	const HF32 va((d3*d6) - (d5*d4));
//	const HF32 d43(d4-d3);
//	const HF32 d56(d5-d6);
//	//P is in the edge region of BC, return the projection of P onto BC
//	const Scalar_Bool b6(va.LE(zero) & d43.GE(zero) & d56.GE(zero));
//	
//	//{	
//		const HF32 w(d43/(d43+d56));
//		_closest = Select(b6, Select(b1, _closest, b+ w*(c-b)), _closest);
//		//return;
//	//}
//		const Scalar_Bool b15 = b14 | b6;
//
//	//P is inside the face region, return the projection of P onto abc
//	const HF32 denom((va+vb+vc).Reciprocal());
//	const HF32 v(vb*denom);
//	const HF32 _w(vc*denom);
//	closest= Select(b15, _closest, a + ab*v + ac*_w);
//}

Maths::HVec3 ClosestPtPointTriangle(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, 
							const Maths::C_Normal PCREF normal)
{
	using namespace Maths;
	{
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());
		const HF32 epsilon(Constants<HF32>::Epsilon());
		const HVec3 ab(b-a);
		const HVec3 ac(c-a);
		const HVec3 ap(p-a);
		const HVec3 bc(c-b);

		const C_Normal n(normal);
		const HF32 t(n|ap);
		const HVec3 closestABC(p - t*n);

		//Check whether closestABC is interial point
		const HF32 d00(ab|ab);
		const HF32 d01(ab|ac);
		const HF32 d11(ac|ac);
		const HF32 d20(ap|ab);
		const HF32 d21(ap|ac);
		const HF32 demo(d00*d11 - d01*d01);
		const Maths::HVec4 numerator(d00, d11, demo, (bc|bc));
		/*const HF32 recipD00(d00.Reciprocal());
		const HF32 recipD11(d11.Reciprocal());
		const HF32 recipDemo(demo.Reciprocal());*/
		const Maths::HVec4 recipNum(Maths::Reciprocal(numerator));
		const HF32 recipD00(recipNum.X());
		const HF32 recipD11(recipNum.Y());
		const HF32 recipDemo(recipNum.Z());
		const HF32 recipDotBC(recipNum.W());
		
		//const HF32 demo(Maths::Select(_demo.Abs().LE(epsilon), epsilon, _demo));
		
		const HF32 v((d11*d20 - d01*d21)*recipDemo);
		const HF32 w((d00*d21 - d01*d20)*recipDemo);
		const HF32 u(one - v - w);		//demo - v - w?

		const Scalar_Bool bClosestABC(v.GT(zero) & v.LT(one) & w.GT(zero) & w.LT(one) & u.GT(zero) & u.LT(one));
		//if( v.GT(zero) & v.LT(one) & w.GT(zero) & w.LT(one) & u.GT(zero) & u.LT(one))
		//{
		//	closest = closestABC;
		//	return;
		//}

		//Calculate the closest point between edge ab
		const HF32 _t0(d20*recipD00);//Dot(p-a, ab)/Dot(ab, ab);
		const HF32 t0=Clamp(_t0, zero, one);
		const HVec3 closestAB(a+(ab*t0));

		//Calculate the closest point between edge ac
		const HF32 _t1(d21*recipD11);//Dot(p-a, ac)/Dot(ac, ac);
		const HF32 t1=Clamp(_t1, zero, one);
		const HVec3 closestAC(a+(ac*t1));

		//Calculate the closest point between edge bc
		
		const HF32 _t2=((p-b)| bc)*recipDotBC;//Dot(p-a, bc)/Dot(bc, bc);
		const HF32 t2=Clamp(_t2, zero, one);
		const HVec3 closestBC(b+(bc*t2));

		const HVec3 d1(p-closestAB);
		const HVec3 d2(p-closestAC);
		const HVec3 d3(p-closestBC);
		
		const HF32 sqDist1(d1|d1);
		const HF32 sqDist2(d2|d2);
		const HF32 sqDist3(d3|d3);

		const Scalar_Bool b11(sqDist1.LT(sqDist2) & sqDist1.LT(sqDist3));
		const Scalar_Bool b21(sqDist2.LT(sqDist3));
		
		return Select(bClosestABC, closestABC, Select(b11, closestAB, Select(b21, closestAC, closestBC)));
	}
}


void ClosestPtPointTriangle(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, Maths::HVec3& closest)
{
	using namespace Maths;
#if 1
	//if ( 1 || !g_dbUseMathsFunction )
	{
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());
		const HF32 epsilon(Constants<HF32>::Epsilon());
		const HVec3 ab(b-a);
		const HVec3 ac(c-a);
		const HVec3 ap(p-a);

		const C_Normal n(ab^ac);
		const HF32 t(n|ap);
		const HVec3 closestABC(p - t*n);

		//Check whether closestABC is interial point
		const HF32 d00(ab|ab);
		const HF32 d01(ab|ac);
		const HF32 d11(ac|ac);
		const HF32 d20(ap|ab);
		const HF32 d21(ap|ac);
		const HF32 demo(d00*d11 - d01*d01);
		//const HF32 demo(Maths::Select(_demo.Abs().LE(epsilon), epsilon, _demo));
		const HF32 recipDemo(demo.Reciprocal());
		const HF32 v((d11*d20 - d01*d21)*recipDemo);
		const HF32 w((d00*d21 - d01*d20)*recipDemo);
		const HF32 u(one - v - w);

		const Scalar_Bool bClosestABC(v.GT(zero) & v.LT(one) & w.GT(zero) & w.LT(one) & u.GT(zero) & u.LT(one));
		//if( v.GT(zero) & v.LT(one) & w.GT(zero) & w.LT(one) & u.GT(zero) & u.LT(one))
		//{
		//	closest = closestABC;
		//	return;
		//}

		//Calculate the closest point between edge ab
		const HF32 _t0(d20/d00);//Dot(p-a, ab)/Dot(ab, ab);
		const HF32 t0=Clamp(_t0, zero, one);
		const HVec3 closestAB(a+(ab*t0));

		//Calculate the closest point between edge ac
		const HF32 _t1(d21/d11);//Dot(p-a, ac)/Dot(ac, ac);
		const HF32 t1=Clamp(_t1, zero, one);
		const HVec3 closestAC(a+(ac*t1));

		//Calculate the closest point between edge bc
		const HVec3 bc(c-b);
		const HF32 _t2=(p-b| bc)/(bc|bc);//Dot(p-a, bc)/Dot(bc, bc);
		const HF32 t2=Clamp(_t2, zero, one);
		const HVec3 closestBC(b+(bc*t2));

		const HVec3 d1(p-closestAB);
		const HVec3 d2(p-closestAC);
		const HVec3 d3(p-closestBC);
		
		const HF32 sqDist1(d1|d1);
		const HF32 sqDist2(d2|d2);
		const HF32 sqDist3(d3|d3);

		const Scalar_Bool b11(sqDist1.LT(sqDist2) & sqDist1.LT(sqDist3));
		const Scalar_Bool b21(sqDist2.LT(sqDist3));
		
		closest=Select(bClosestABC, closestABC, Select(b11, closestAB, Select(b21, closestAC, closestBC)));
	}
	/*else
	{
	    if ( g_dbUseMathsFunctionV2 )
            closest = Maths::ClosestPointOnTriangleV2( p, a, b, c );
	    else
            closest = Maths::ClosestPointOnTriangle( p, a, b, c );
    }*/
#else
	closest = Maths::ClosestPointOnTriangle( p, a, b, c );
#endif
}

Maths::HVec3 ClosestPtPointTriangle(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c)
{
	using namespace Maths;
	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one(Constants<HF32>::One());
	const HF32 epsilon(Constants<HF32>::Epsilon());
	const HVec3 ab(b-a);
	const HVec3 ac(c-a);
	const HVec3 ap(p-a);
	const HVec3 bc(c-b);

	const C_Normal n(ab ^ ac);
	const HF32 t(n|ap);
	const HVec3 closestABC(p - t*n);

	//Check whether closestABC is interial point
	const HF32 d00(ab|ab);
	const HF32 d01(ab|ac);
	const HF32 d11(ac|ac);
	const HF32 d20(ap|ab);
	const HF32 d21(ap|ac);
	const HF32 demo(d00*d11 - d01*d01);
	const Maths::HVec4 numerator(d00, d11, demo, (bc|bc));
	/*const HF32 recipD00(d00.Reciprocal());
	const HF32 recipD11(d11.Reciprocal());
	const HF32 recipDemo(demo.Reciprocal());*/
	const Maths::HVec4 recipNum(Maths::Reciprocal(numerator));
	const HF32 recipD00(recipNum.X());
	const HF32 recipD11(recipNum.Y());
	const HF32 recipDemo(recipNum.Z());
	const HF32 recipDotBC(recipNum.W());
	
	//const HF32 demo(Maths::Select(_demo.Abs().LE(epsilon), epsilon, _demo));
	
	const HF32 v((d11*d20 - d01*d21)*recipDemo);
	const HF32 w((d00*d21 - d01*d20)*recipDemo);
	const HF32 u(one - v - w);		//demo - v - w?

	const Scalar_Bool bClosestABC(v.GT(zero) & v.LT(one) & w.GT(zero) & w.LT(one) & u.GT(zero) & u.LT(one));
	//if( v.GT(zero) & v.LT(one) & w.GT(zero) & w.LT(one) & u.GT(zero) & u.LT(one))
	//{
	//	closest = closestABC;
	//	return;
	//}

	//Calculate the closest point between edge ab
	const HF32 _t0(d20*recipD00);//Dot(p-a, ab)/Dot(ab, ab);
	const HF32 t0=Clamp(_t0, zero, one);
	const HVec3 closestAB(a+(ab*t0));

	//Calculate the closest point between edge ac
	const HF32 _t1(d21*recipD11);//Dot(p-a, ac)/Dot(ac, ac);
	const HF32 t1=Clamp(_t1, zero, one);
	const HVec3 closestAC(a+(ac*t1));

	//Calculate the closest point between edge bc
	
	const HF32 _t2=((p-b)| bc)*recipDotBC;//Dot(p-a, bc)/Dot(bc, bc);
	const HF32 t2=Clamp(_t2, zero, one);
	const HVec3 closestBC(b+(bc*t2));

	const HVec3 d1(p-closestAB);
	const HVec3 d2(p-closestAC);
	const HVec3 d3(p-closestBC);
	
	const HF32 sqDist1(d1|d1);
	const HF32 sqDist2(d2|d2);
	const HF32 sqDist3(d3|d3);

	const Scalar_Bool b11(sqDist1.LT(sqDist2) & sqDist1.LT(sqDist3));
	const Scalar_Bool b21(sqDist2.LT(sqDist3));
	
	return Select(bClosestABC, closestABC, Select(b11, closestAB, Select(b21, closestAC, closestBC)));

}

//void ClosestPtSegmentTriangle(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, Maths::HVec3& c1, Maths::HVec3& c2, Maths::HF32& sqDist)
//{
//	using namespace Maths;
//	//HF32 sqDist(Maths::UNDEFINED);
//	HVec3 t00(Maths::UNDEFINED);
//	HVec3 t01(Maths::UNDEFINED);
//	//compute the closest point between segment pq and ab	
//	ClosestPtSegmentSegment(p, q, a, b, t00, t01);
//	const HVec3 temp1(t00-t01);
//	const HF32 sqDist0(temp1|temp1);
//
//	HVec3 t10(Maths::UNDEFINED);
//	HVec3 t11(Maths::UNDEFINED);
//	//compute the closest point between segment pq and ac
//	ClosestPtSegmentSegment(p, q, a, c, t10, t11);
//	const HVec3 temp2(t10-t11);
//	const HF32 sqDist1(temp2|temp2);
//
//
//	const Maths::Scalar_Bool b1(sqDist0.LE(sqDist1));
//	c1=Select(b1, t00, t10);
//	c2=Select(b1, t01, t11);
//	sqDist=Select(b1, sqDist0, sqDist1);
//	
//
//	//compute the closest point between segment pq and bc
//	HVec3 t20(Maths::UNDEFINED);
//	HVec3 t21(Maths::UNDEFINED);
//	ClosestPtSegmentSegment(p, q, b, c, t20, t21);
//	const HVec3 temp3(t20-t21);
//	const HF32 sqDist2(temp3|temp3);
//
//	const Maths::Scalar_Bool b2(sqDist.LE(sqDist2));
//	c1=Select(b2, c1, t20);
//	c2=Select(b2, c2, t21);
//	sqDist=Select(b2, sqDist, sqDist2);
//
//	//compute the closest point between point p and plane of triangle abc
//	HVec3 t31(Maths::UNDEFINED);
//	ClosestPtPointTriangle(p, a, b, c, t31);
//	const HVec3 temp4(p-t31);
//	const HF32 sqDist3(temp4|temp4);
//
//	const Maths::Scalar_Bool b3(sqDist.LE(sqDist3));
//	c1=Select(b3, c1, p);
//	c2=Select(b3, c2, t31);
//	sqDist=Select(b3, sqDist, sqDist3);
//
//	//compute the closest point between point q and plane of triangle abc
//
//	HVec3 t41(Maths::UNDEFINED);
//	ClosestPtPointTriangle(q, a, b, c, t41);
//	const HVec3 temp5(q-t41);
//	const HF32 sqDist4(temp5|temp5);
//	
//	const Maths::Scalar_Bool b4(sqDist.LE(sqDist4));
//	c1=Select(b4, c1, q);
//	c2=Select(b4, c2, t41);
//	sqDist=Select(b4, sqDist, sqDist4);
//}


/*
	Non optimised version
*/
//void ClosestPtSegmentTriangle(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, 
//							  const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, 
//							  Maths::C_Normal PCREF normal, Maths::HVec3& c1, Maths::HVec3& c2, Maths::HF32& sqDistc)
//{
//	using namespace Maths;
//	//compute the closest point between segment pq and ab	
//	CEClosestPtSegmentSegment(p, q, a, b, c1, c2);
//	const HVec3 temp1(c1-c2);
//	sqDistc=temp1|temp1;
//	HVec3 t10(Maths::UNDEFINED);
//	HVec3 t11(Maths::UNDEFINED);
//	//compute the closest point between segment pq and ac
//	CEClosestPtSegmentSegment(p, q, a, c, t10, t11);
//	const HVec3 temp2(t10-t11);
//	const HF32 sqDist1(temp2|temp2);
//	if(sqDistc > sqDist1)
//	{
//		c1 = t10;
//		c2 = t11;
//		sqDistc=sqDist1;
//	}
//
//	//compute the closest point between segment pq and bc
//	HVec3 t20(Maths::UNDEFINED);
//	HVec3 t21(Maths::UNDEFINED);
//	CEClosestPtSegmentSegment(p, q, b, c, t20, t21);
//	const HVec3 temp3(t20-t21);
//	const HF32 sqDist2(temp3|temp3);
//	if(sqDistc> sqDist2)
//	{
//		c1 = t20;
//		c2 = t21;
//		sqDistc=sqDist2;
//	}
//
//	//compute the closest point between point p and plane of triangle abc
//	const HVec3 t31(ClosestPtPointTriangle(p, a, b, c, normal));
//	const HVec3 temp4(p-t31);
//	const HF32 sqDist3(temp4|temp4);
//	if(sqDistc > sqDist3)
//	{
//		c1 = p;
//		c2 = t31;
//		sqDistc=sqDist3;
//	}
//	//compute the closest point between point q and plane of triangle abc
//
//	const HVec3 t41(ClosestPtPointTriangle(q, a, b, c, normal));
//	const HVec3 temp5(q-t41);
//	const HF32 sqDist4(temp5|temp5);
//	if(sqDistc> sqDist4)
//	{
//		c1 = q;
//		c2 = t41;
//		sqDistc=sqDist4;
//	}
//}

void ClosestPtSegmentTriangle(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, 
							  const Maths::C_Normal PCREF normal, Maths::HVec3& c1, Maths::HVec3& c2, Maths::HF32& sqDistc)
{
	using namespace Maths;
	//compute the closest point between segment pq and ab	

	HVec3 _c1(Maths::UNDEFINED), _c2(Maths::UNDEFINED);
	CEClosestPtSegmentSegment(p, q, a, b, _c1, _c2);
	const HVec3 temp1(_c1-_c2);
	Maths::HF32 dist(temp1|temp1);
	HVec3 t10(Maths::UNDEFINED);
	HVec3 t11(Maths::UNDEFINED);
	//compute the closest point between segment pq and ac
	CEClosestPtSegmentSegment(p, q, a, c, t10, t11);
	const HVec3 temp2(t10-t11);
	const HF32 sqDist1(temp2|temp2);
	const Scalar_Bool bEarlier1(sqDist1.LT(dist));
	_c1 = Select(bEarlier1, t10, _c1);
	_c2 = Select(bEarlier1, t11, _c2);
	dist = Select(bEarlier1,sqDist1, dist);

	//compute the closest point between segment pq and bc
	HVec3 t20(Maths::UNDEFINED);
	HVec3 t21(Maths::UNDEFINED);
	CEClosestPtSegmentSegment(p, q, b, c, t20, t21);
	const HVec3 temp3(t20-t21);
	const HF32 sqDist2(temp3|temp3);
	const Scalar_Bool bEarlier2(sqDist2.LT(dist));
	_c1 = Select(bEarlier2, t20, _c1);
	_c2 = Select(bEarlier2, t21, _c2);
	dist = Select(bEarlier2,sqDist2, dist);

	//compute the closest point between point p and plane of triangle abc
	const HVec3 t31(ClosestPtPointTriangle(p, a, b, c, normal));
	const HVec3 temp4(p-t31);
	const HF32 sqDist3(temp4|temp4);
	const Scalar_Bool bEarlier3(sqDist3.LT(dist));
	_c1 = Select(bEarlier3, p, _c1);
	_c2 = Select(bEarlier3, t31, _c2);
	dist = Select(bEarlier3,sqDist3, dist);
	const HVec3 t41(ClosestPtPointTriangle(q, a, b, c, normal));
	const HVec3 temp5(q-t41);
	const HF32 sqDist4(temp5|temp5);
	const Scalar_Bool bEarlier4(sqDist4.LT(dist));
	c1 = Select(bEarlier4, q, _c1);
	c2 = Select(bEarlier4, t41, _c2);
	sqDistc = Select(bEarlier4,sqDist4, dist);
}


void ClosestPtSegmentTriangle(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, Maths::HVec3& c1, Maths::HVec3& c2, Maths::HF32& sqDistc)
{
	using namespace Maths;

	//compute the closest point between segment pq and ab	
	HVec3 _c1(Maths::UNDEFINED), _c2(Maths::UNDEFINED);
	CEClosestPtSegmentSegment(p, q, a, b, _c1, _c2);
	const HVec3 temp1(_c1-_c2);
	Maths::HF32 dist(temp1|temp1);
	HVec3 t10(Maths::UNDEFINED);
	HVec3 t11(Maths::UNDEFINED);
	//compute the closest point between segment pq and ac
	CEClosestPtSegmentSegment(p, q, a, c, t10, t11);
	const HVec3 temp2(t10-t11);
	const HF32 sqDist1(temp2|temp2);
	const Scalar_Bool bEarlier1(sqDist1.LT(dist));
	_c1 = Select(bEarlier1, t10, _c1);
	_c2 = Select(bEarlier1, t11, _c2);
	dist = Select(bEarlier1,sqDist1, dist);

	//compute the closest point between segment pq and bc
	HVec3 t20(Maths::UNDEFINED);
	HVec3 t21(Maths::UNDEFINED);
	CEClosestPtSegmentSegment(p, q, b, c, t20, t21);
	const HVec3 temp3(t20-t21);
	const HF32 sqDist2(temp3|temp3);
	const Scalar_Bool bEarlier2(sqDist2.LT(dist));
	_c1 = Select(bEarlier2, t20, _c1);
	_c2 = Select(bEarlier2, t21, _c2);
	dist = Select(bEarlier2,sqDist2, dist);

	//compute the closest point between point p and plane of triangle abc
	const HVec3 t31(ClosestPtPointTriangle(p, a, b, c));
	const HVec3 temp4(p-t31);
	const HF32 sqDist3(temp4|temp4);
	const Scalar_Bool bEarlier3(sqDist3.LT(dist));
	_c1 = Select(bEarlier3, p, _c1);
	_c2 = Select(bEarlier3, t31, _c2);
	dist = Select(bEarlier3,sqDist3, dist);

	const HVec3 t41(ClosestPtPointTriangle(q, a, b, c));
	
	const HVec3 temp5(q-t41);
	const HF32 sqDist4(temp5|temp5);
	const Scalar_Bool bEarlier4(sqDist4.LT(dist));
	c1 = Select(bEarlier4, q, _c1);
	c2 = Select(bEarlier4, t41, _c2);
	sqDistc = Select(bEarlier4,sqDist4, dist);
}

void ClosestPtSegmentTriangleSegment(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, Maths::HVec3& closest)
{
	using namespace Maths;

	HVec3 c1(UNDEFINED);
	HVec3 c2(UNDEFINED);
	HVec3 c3(UNDEFINED);
	HVec3 c4(UNDEFINED);
	HVec3 c5(UNDEFINED);
	HVec3 c6(UNDEFINED);

	Collision::ClosestPtSegmentSegment(p, q, a, b, c1, c2);
	Collision::ClosestPtSegmentSegment(p, q, a, c, c3, c4);
	Collision::ClosestPtSegmentSegment(p, q, b, c, c5, c6);

	const HVec3 av(c1 - c2);
	const HVec3 bv(c3 - c4);
	const HVec3 cv(c5 - c6);

	const HF32 sqDist1( av|av );
	const HF32 sqDist2( bv|bv );
	const HF32 sqDist3( cv|cv );

	const Maths::Scalar_Bool con1(sqDist1.LT(sqDist2) && sqDist1.LT(sqDist3));
	const Maths::Scalar_Bool con2(sqDist2.LT(sqDist3));

	closest = Select(con1, c2, Select(con2, c4, c6));
}

void ClosestPtTriangleTriangle(const Maths::HVec3 PCREF a1, const Maths::HVec3 PCREF b1, const Maths::HVec3 PCREF c1, const Maths::HVec3 PCREF a2, const Maths::HVec3 PCREF b2, const Maths::HVec3 PCREF c2, Maths::HVec3& closestP1, Maths::HVec3& closestP2)
{
	using namespace Maths;

	HVec3 v1[15];
	HVec3 v2[15];
	v1[0]=a1;
	v1[1]=b1;
	v1[2]=c1;
	v1[3]=a2;
	v1[4]=b2;
	v1[5]=c2;

	ClosestPtPointTriangle(a1, a2, b2, c2, v2[0]);
	ClosestPtPointTriangle(b1, a2, b2, c2, v2[1]);
	ClosestPtPointTriangle(c1, a2, b2, c2, v2[2]);

	ClosestPtPointTriangle(a2, a1, b1, c1, v2[3]);
	ClosestPtPointTriangle(b2, a1, b1, c1, v2[4]);
	ClosestPtPointTriangle(c2, a1, b1, c1, v2[5]);

	ClosestPtSegmentSegment(a1,b1, a2, b2,v1[6], v2[6]);
	ClosestPtSegmentSegment(a1,b1, a2, c2,v1[7], v2[7]);
	ClosestPtSegmentSegment(a1,b1, b2, c2,v1[8], v2[8]);

	ClosestPtSegmentSegment(a1, c1, a2, b2, v1[9], v2[9]);
	ClosestPtSegmentSegment(a1,c1, a2, c2, v1[10], v2[10]);
	ClosestPtSegmentSegment(a1,c1, b2, c2, v1[11], v2[11]);

	ClosestPtSegmentSegment(b1, c1, a2, b2, v1[12], v2[12]);
	ClosestPtSegmentSegment(b1,c1, a2, c2, v1[13], v2[13]);
	ClosestPtSegmentSegment(b1,c1, b2, c2, v1[14], v2[14]);

	
	HF32 dist=(v2[0]-v1[0])|(v2[0]-v1[0]);
	closestP1=v1[0];
	closestP2=v2[0];
	for(U32 i=1; i<15; ++i)
	{
		const HF32 temp=(v2[i]-v1[i])|(v2[i]-v1[i]);
		const Maths::Scalar_Bool b(temp.LT(dist));
		dist = Select(b, temp, dist);
		closestP1=Select(b, v1[i], closestP1);
		closestP2=Select(b, v2[i], closestP2);	
	}

}

Maths::HVec3 ClosestPtPointTetrahedron(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF d)
{
	using namespace Maths;
	HVec3 closestPt(p);
	HF32 bestSqDist(Maths::Constants<HF32>::Max());

	if(PointOutsideOfPlane(p, a, b, c, d))
	{
		HVec3 q(Maths::UNDEFINED);
		ClosestPtPointTriangle(p, a, b, c, q);
		const HVec3 qp(q-p);
		const HF32 sqDist=(qp)|(qp);//Dot(q-p, q-p);
		//if(sqDist < bestSqDist) bestSqDist=sqDist, closestPt=q;
		const Scalar_Bool comp(sqDist.LT(bestSqDist));
		closestPt = Select(comp, q, closestPt);
		bestSqDist = Min(sqDist, bestSqDist);
	}

	if(PointOutsideOfPlane(p, a, c, d, b))
	{
		HVec3 q(Maths::UNDEFINED);
		ClosestPtPointTriangle(p, a, c, d, q);
		const HVec3 qp(q-p);
		HF32 sqDist=qp |qp;//Dot(q-p, q-p);
		//if(sqDist < bestSqDist) bestSqDist=sqDist, closestPt=q;
		const Scalar_Bool comp(sqDist.LT(bestSqDist));
		closestPt = Select(comp, q, closestPt);
		bestSqDist = Min(sqDist, bestSqDist);
	}

	if(PointOutsideOfPlane(p, a, d, b, c))
	{
		HVec3 q(Maths::UNDEFINED);
		ClosestPtPointTriangle(p, a, d, b,q);
		const HVec3 qp(q-p);
		HF32 sqDist=qp|qp;//Dot(q-p, q-p);
		//if(sqDist < bestSqDist) bestSqDist=sqDist, closestPt=q;
		const Scalar_Bool comp(sqDist.LT(bestSqDist));
		closestPt = Select(comp, q, closestPt);
		bestSqDist = Min(sqDist, bestSqDist);
	}

	if(PointOutsideOfPlane(p, b, d, c, a))
	{
		HVec3 q(Maths::UNDEFINED);
		ClosestPtPointTriangle(p, b, d, c,q);
		const HVec3 qp(q-p);
		HF32 sqDist=qp | qp;//Dot(q-p, q-p);
		//if(sqDist < bestSqDist) bestSqDist=sqDist, closestPt=q;
		const Scalar_Bool comp(sqDist.LT(bestSqDist));
		closestPt = Select(comp, q, closestPt);
		bestSqDist = Min(sqDist, bestSqDist);
	}

	return closestPt;
}

bool PointOutsideOfPlane(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF d)
{
	using namespace Maths;

	const HF32 zero(Constants<HF32>::Zero());
	HF32 signp=(p-a) | ((b-a)^(c-a));//Dot(p-a, Cross(b-a, c-a));
	HF32 signd=(d-a) | ((b-a)^(c-a));//Dot(d-a, Cross(b-a, c-a));
	return (signp*signd).LT(zero);
}


void ClosestPtPointAABB(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF _min, const Maths::HVec3 PCREF _max, Maths::HVec3& closest)
{
	
	/*Maths::HF32 x(p.X());
	Maths::HF32 y(p.Y());
	Maths::HF32 z(p.Z());


	if(x > _max.X()) x=_max.X();
	if(x < _min.X()) x=_min.X();

	if(y > _max.Y()) y=_max.Y();
	if(y < _min.Y()) y=_min.Y();

	if(z > _max.Z()) z=_max.Z();
	if(z < _min.Z()) z=_min.Z(); 

	closest = Maths::HVec3(x, y, z);*/

	
	closest = Maths::Max( _min, Maths::Min( _max, p ));

}

/*
	p in world space, and closest in world space as well
*/
void ClosestPtPointOBB(const Maths::HVec3 PCREF p, const C_OBB& obb, Maths::HVec3& closest)
{
	//Transform p into obb's local space
	//const Maths::HVec3 q(obb.u*(p-obb.c));
	//Maths::HF32 x(q.X());
	//Maths::HF32 y(q.Y());
	//Maths::HF32 z(q.Z());

	//const Maths::HF32 ex(obb.e.X());
	//const Maths::HF32 ey(obb.e.Y());
	//const Maths::HF32 ez(obb.e.Z());

	//if(x > ex) x=ex;
	//if(x <-ex) x=-ex;

	//if(y > ey) y=ey;
	//if(y <-ey) y=-ey;

	//if(z > ez) z=ez;
	//if(z <-ez) z=-ez;

	//const Maths::HVec3 project(x, y, z);
	//closest=project*obb.u + obb.c;

	using namespace Maths;
	const HMat33& u = obb.Orientation();
	HVec3 q(u*(p-obb.Centre()));
	const HVec3 e(obb.Extent());
	q = Max( -e, Min( e, q ));

	closest=q*u + obb.Centre();
}

void ClosestPtPointCylinder(const Maths::HVec3 PCREF p, const Collision::C_Cylinder& cylinder, Maths::HVec3& closest)
{
	const Maths::HF32 radius(cylinder.Radius());
	const Maths::HVec3 center(cylinder.Centre());
	const Maths::C_Normal u(cylinder.CentreAxis(), Maths::C_Normal::UNIT_LENGTH_VECTOR);
	const Maths::HF32 halfHeight(cylinder.HalfHeight());

	const Maths::HVec3 v(p-center);
	const Maths::HF32 proDist(v|u);//v*cos(A)
	const Maths::HF32 aProDist(proDist.Abs());
	
	const Maths::HVec3 w(u*proDist);
	const Maths::HVec3 vw(v-w);
	const Maths::C_Normal normal(vw);

	if(aProDist.LE(halfHeight))
	{
		const Maths::HVec3 proj(w+center);
		closest=proj + normal*radius;
	}
	else
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::Scalar_Bool con(proDist.GT(zero));
		const Maths::HF32 h=Select(con, halfHeight, -halfHeight);
		const Maths::HF32 dif=proDist-h;
		const Maths::HVec3 a(p - u*dif);

		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(a, 0.2f, RGBA(255, 255, 0, 255));
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(a, "a");

		/*const Maths::HVec3 proj(w+center);
		const Maths::HVec3 vw(proj - p);
		const Maths::C_Normal normal(vw);
		const Maths::HF32 length(vw.Length() - radius);
		const Maths::Scalar_Bool con1(length.GT(zero));
		closest = Select(con1, a + normal*length, a);*/

		
		const Maths::HF32 length(vw.Length() - radius);
		const Maths::Scalar_Bool con1(length.GT(zero));
		closest = Select(con1, a - normal*length, a);
	
	}
}

}; // namespace NewAlgorithms

