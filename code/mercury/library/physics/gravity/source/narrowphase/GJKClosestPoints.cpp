#include <stdafx.h>
#include "gravity/narrowphase/GJK.h"
#include "gravity/narrowphase/GJK.inl"

//using namespace Maths;

namespace Collision
{

/*NOALIAS __forceinline HVec3 GJKCPairClosestPtPointTetrahedron1(HVec3* __restrict Q, U32& size, HVec3* __restrict A, HVec3* __restrict B, HVec3& closestA, HVec3& closestB)
{
	const HVec3 origin(Maths::Constants<HVec3>::Zero());
	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	HF32 bestSqDist(Maths::Constants<HF32>::Max());
	closestA=origin;//set the closestPt as origin first
	closestB=origin;//=closestPt;
	
	HVec3 tempA(Maths::Constants<HVec3>::Zero());
	HVec3 tempB(Maths::Constants<HVec3>::Zero());
	HVec3 closestPt(Maths::Constants<HVec3>::Zero());
	
	
	const HVec3 ao(-Q[0]);
	const HVec3 bo(-Q[1]);
	const HVec3 ab(Q[1]-Q[0]);
	const HVec3 ac(Q[2]-Q[0]);
	const HVec3 ad(Q[3]-Q[0]);

	const HVec3 bd(Q[3]-Q[1]);
	const HVec3 bc(Q[2]-Q[1]);
	const HVec3 ba(Q[0]-Q[1]);

	const Maths::HVec3 abc=ab^ac;
	const Maths::HF32 o_abc(ao|abc);
	const Maths::HF32 d_abc(ad|abc);

	const Maths::HVec3 acd=ac^ad;
	const Maths::HF32 o_acd(ao|acd);
	const Maths::HF32 b_acd(ab|acd);

	const Maths::HVec3 adb=ad^ab;
	const Maths::HF32 o_adb(ao|adb);
	const Maths::HF32 c_adb(ac|adb);

	const Maths::HVec3 bdc=bd^bc;
	const HF32 o_bdc(bo|bdc);
	const HF32 a_bdc(ba|bdc);

	const HVec3 Qt[4]={Q[0], Q[1], Q[2], Q[3]};
	const HVec3 At[4]={A[0], A[1], A[2], A[3]};
	const HVec3 Bt[4]={B[0], B[1], B[2], B[3]};

	U32 _size;

	
	if((o_abc*d_abc).LT(zero))
	{
		HVec3 _Q1[3]={Qt[0],Qt[1],Qt[2]};
		HVec3 _A1[3]={At[0],At[1],At[2]};
		HVec3 _B1[3]={Bt[0],Bt[1],Bt[2]};

		GJKCPairClosestPtPointTriangle1(_Q1, _size, _A1, _B1, tempA, tempB);
		const HVec3 q(tempA-tempB);
		const HF32 sqDist(q | q);
		//if(sqDist.LT(bestSqDist)) 
		//{
			bestSqDist=sqDist;
			closestA=tempA;
			closestB=tempB;
			closestPt=q;
			size=_size;
			Q[0] = _Q1[0]; Q[1] = _Q1[1]; Q[2] = _Q1[2];
			A[0] = _A1[0]; A[1] = _A1[1]; A[2] = _A1[2];
			B[0] = _B1[0]; B[1] = _B1[1]; B[2] = _B1[2];
			
		//}
	}
	

	

	if((o_acd*b_acd).LE(zero))
	{
		//_size=s;
		HVec3 _Q1[3] = {Qt[0],Qt[2],Qt[3]};
		HVec3 _A1[3] = {At[0],At[2],At[3]};
		HVec3 _B1[3] = {Bt[0],Bt[2],Bt[3]};

		GJKCPairClosestPtPointTriangle1(_Q1, _size, _A1, _B1, tempA, tempB);
		const HVec3 q(tempA-tempB);
		const HF32 sqDist(q | q);
		if(sqDist.LT(bestSqDist))
		{
			bestSqDist=sqDist;
			closestA=tempA;
			closestB=tempB;
			closestPt=q;
			size=_size;

			Q[0] = _Q1[0]; Q[1] = _Q1[1]; Q[2] = _Q1[2];
			A[0] = _A1[0]; A[1] = _A1[1]; A[2] = _A1[2];
			B[0] = _B1[0]; B[1] = _B1[1]; B[2] = _B1[2];
		}
	}

	
	if((o_adb*c_adb).LE(zero))
	{
		//_size=s;
		HVec3 _Q1[3]={Qt[0],Qt[3],Qt[1]};
		HVec3 _A1[3]={At[0],At[3],At[1]};
		HVec3 _B1[3]={Bt[0],Bt[3],Bt[1]};
		GJKCPairClosestPtPointTriangle1(_Q1, _size, _A1, _B1, tempA, tempB);
		const HVec3 q(tempA-tempB);
		const HF32 sqDist(q | q);
		if(sqDist.LT(bestSqDist))
		{
			bestSqDist=sqDist;
			closestA=tempA;
			closestB=tempB;
			closestPt=q;
			size=_size;

			Q[0] = _Q1[0]; Q[1] = _Q1[1]; Q[2] = _Q1[2];
			A[0] = _A1[0]; A[1] = _A1[1]; A[2] = _A1[2];
			B[0] = _B1[0]; B[1] = _B1[1]; B[2] = _B1[2];
		}
	}

	if((o_bdc*a_bdc).LE(zero))
	{
		//_size=s;
		HVec3 _Q1[3]={Qt[1],Qt[3],Qt[2]};
		HVec3 _A1[3]={At[1],At[3],At[2]};
		HVec3 _B1[3]={Bt[1],Bt[3],Bt[2]};

		GJKCPairClosestPtPointTriangle1(_Q1, _size, _A1, _B1, tempA, tempB);
		const HVec3 q(tempA-tempB);
		//const HVec3 t(q-p);
		//const HF32 sqDist(t|t);//Dot(q-p, q-p);
		const HF32 sqDist(q | q);
		if(sqDist.LT(bestSqDist)) 
		{
			bestSqDist=sqDist;
			closestA=tempA;
			closestB=tempB;
			closestPt=q;
			size=_size;
			Q[0] = _Q1[0]; Q[1] = _Q1[1]; Q[2] = _Q1[2];
			A[0] = _A1[0]; A[1] = _A1[1]; A[2] = _A1[2];
			B[0] = _B1[0]; B[1] = _B1[1]; B[2] = _B1[2];
		}
	}
	return closestPt;
}*/

bool GJKCPairDoSimplex(Maths::HVec3* __restrict Q, Maths::HVec3* __restrict A, Maths::HVec3* __restrict B, U32& size, Maths::HVec3& closestA, Maths::HVec3& closestB)
{
	switch(size)
	{
	case 1:
		{
			closestA = A[0];
			closestB = B[0];
			return true;
		}
	case 2:
		return GJKCPairClosestPtPointSegment(Q, size, A, B, closestA, closestB);
	case 3: 
		return GJKCPairClosestPtPointTriangle(Q, size, A, B, closestA, closestB);
	case 4: 
		return GJKCPairClosestPtPointTetrahedron(Q, size, A, B, closestA, closestB);
	default:
		BIZ_ASSERT(0 && "Undefined Simplex!");
		return false;
	}
	//return _size;
}

NOALIAS inline bool GJKCPairClosestPtPointSegment(const Maths::HVec3 PCREF origin, Maths::HVec3*  Q, U32& size, Maths::HVec3*  A, Maths::HVec3* B, Maths::HVec3& closestA, Maths::HVec3& closestB)
{
	using namespace Maths;

	const HVec3 Q0(Q[0]);
	const HVec3 Q1(Q[1]);
	const HVec3 ab(Q1-Q0);

	//Deal with the degeneration case
	if(ab.Length().LT(Constants<HF32>::Epsilon()))
	{
		size = 1;
		closestA = A[0];
		closestB = B[0];
		return true;
	}

	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one(Constants<HF32>::One());
	
	const HF32 _t=((origin - Q0)|ab ) / (ab|ab);
	const HF32 t=Clamp(_t, zero, one);
	const Scalar_Bool b(_t.LE(zero));
	const Scalar_Bool b2(_t.GE(one));
	const Scalar_Bool ored( b | b2 );

	size = Select(bool(ored), 1, 2);

	Q[0] = Select(b2, Q1, Q0);
	A[0] = Select(b2, A[1], A[0]);
	B[0] = Select(b2, B[1], B[0]);

	const HVec3 a_ab = A[1] - A[0];
	const HVec3 b_ab = B[1] - B[0];

	closestA = Select(ored, A[0], A[0] + a_ab * t);
	closestB = Select(ored, B[0], B[0] + b_ab * t);

	return true;
}

NOALIAS inline bool GJKCPairClosestPtPointTriangle(const Maths::HVec3 PCREF origin, Maths::HVec3*  Q, U32& size, Maths::HVec3*  A, Maths::HVec3*  B, Maths::HVec3& closestA, Maths::HVec3& closestB)
{
	using namespace Maths;

	const HF32 eps = Maths::Constants<HF32>::Epsilon();

	const HF32 zero(-eps);
	const HVec3 ab(Q[1]-Q[0]);
	const HVec3 ac(Q[2]-Q[0]);
	const HVec3 bc(Q[1]-Q[2]);
	const HVec3 n1 = ab^ac;
	const HVec3 n2 = (-ab)^bc;
	const HF32 n=Min((n1|n1), (n2|n2));
	//Deal with the degenerate case
	if(n.LT(eps))
	{
		const HVec3 u(Q[0]-Q[1]);
		if(u.Length().LT(eps))
		{
			Q[1] = Q[2];
			A[1] = A[2];
			B[1] = B[2];
		}
		U32 _size = 2;
		GJKCPairClosestPtPointSegment(origin, Q, _size, A, B, closestA, closestB);
		size = _size;
		return true;
	}
	const HVec3 a_ab(A[1]-A[0]);
	const HVec3 a_ac(A[2]-A[0]);
	const HVec3 b_ab(B[1]-B[0]);
	const HVec3 b_ac(B[2]-B[0]);
	//const HVec3 _ap(Q[0]); // origin - Q[0]
	//const HVec3 ap(-_ap);
	const HVec3 ap(origin - Q[0]);

	const HF32 d1(ab|ap);//Dot(ab, ap);
	const HF32 d2(ac|ap);//Dot(ac, ap);

	//p is in the vertex region a, a is the closest point to p
	
	if(d1.LE(zero)& d2.LE(zero))
	{
		size=1;
		closestA=A[0];
		closestB=B[0];
		return true;
		//return Q[0];
	}
	

	const HVec3 bp(origin -Q[1]);//origin-Q[1];
	const HF32 d3(ab|bp);//Dot(ab, bp);
	const HF32 d4(ac|bp);//Dot(ac, bp);

	//p is in the vertex region b, b is the closest point to p
	if(d3.GE(-zero) & d4.LE(d3)) 
	{
		size=1;
		Q[0]=Q[1];
		A[0]=A[1];
		B[0]=B[1];
		closestA=A[0];
		closestB=B[0];
		return true;
		//return Q[0];
	}

	const HF32 vc((d1*d4) - (d3*d2));
	//P is in the edge region of AB, return projection of P onto AB
	if(vc.LE(zero) & d1.GE(-zero) & d3.LE(zero))
	{
		size=2;
		//const HVec3 a_ab(A[1]-A[0]);
		//const HVec3 b_ab(B[1]-B[0]);
		const HF32 t(d1/(d1-d3));
		closestA=A[0]+a_ab*t;
		closestB=B[0]+b_ab*t;
		return true;
		//return Q[0] + ab*t;
	}

	const HVec3 cp(origin -Q[2]);//origin-Q[2];
	const HF32 d5(ab|cp);//Dot(ab, cp);
	const HF32 d6(ac|cp);//Dot(ac, cp);
	//P is in the vertex region c, c is the closest point to p
	if(d6.GE(-zero) & d5.LE(d6)) 
	{
		size=1;
		Q[0]=Q[2];
		A[0]=A[2];
		B[0]=B[2];
		closestA=A[2];
		closestB=B[2];
		return true;
		//return Q[0];
	}

	const HF32 vb((d5*d2) - (d1*d6));
	//P is in the edge region of AC, return projection of P onto AC
	if(vb.LE(zero) & d2.GE(-zero) & d6.LE(zero))
	{
		size=2;
		Q[1]=Q[2];//swap
		A[1]=A[2];
		B[1]=B[2];
		const HF32 t(d2/(d2-d6));
		closestA=A[0]+a_ac*t;
		closestB=B[0]+b_ac*t;
		return true;
		//return Q[0]+ ac*t;
	}

	const HF32 va((d3*d6) - (d5*d4));
	const HF32 d43(d4-d3);
	const HF32 d56(d5-d6);
	//P is in the edge region of BC, return the projection of P onto BC
	if(va.LE(zero) & d43.GE(-zero) & d56.GE(-zero))
	{	
		const HF32 w(d43/(d43+d56));
		size=2;
		Q[0]=Q[2];
		A[0]=A[2];
		B[0]=B[2];
		const HVec3 a21((A[2]-A[1])*w);
		const HVec3 b21((B[2]-B[1])*w);
		//closestA=A[1]+(A[2]-A[1])*w;
		//closestB=B[1]+(B[2]-B[1])*w;
		closestA=A[1]+a21;
		closestB=B[1]+b21;
		return true;
		//return Q[1]+(Q[0]-Q[1])*w;//b+ (c-b)*w;
	}

	size=3;
	//P is inside the face region, return the projection of P onto abc
	//const HF32 denom(1.f/(va+vb+vc));
	const HF32 denom(Reciprocal(va+vb+vc));
	const HF32 v(vb*denom);
	const HF32 w(vc*denom);
	const HVec3 a_abv(a_ab*v);
	const HVec3 b_abv(b_ab*v);
	const HVec3 a_acw(a_ac*w);
	const HVec3 b_acw(b_ac*w);

	//closestA=A[0]+a_ab*v+a_ac*w;
	//closestB=B[0]+b_ab*v+b_ac*w;
	closestA=A[0]+a_abv+a_acw;
	closestB=B[0]+b_abv+b_acw;
	return true;
	//return Q[0] + ab*v + ac*w;
}

NOALIAS inline bool GJKCPairClosestPtPointTetrahedron(const Maths::HVec3 PCREF origin, Maths::HVec3*  Q, U32& size, Maths::HVec3*  A, Maths::HVec3*  B, Maths::HVec3& closestA, Maths::HVec3& closestB)
{
	using namespace Maths;

	//const HVec3 origin(Constants<HVec3>::Zero());
	//const HVec3 closestPt(Maths::Constants<HVec3>::Zero());
	const HF32 zero(Constants<HF32>::Zero());
	HF32 bestSqDist(Constants<HF32>::Max());
	closestA=origin;//set the closestPt as origin first
	closestB=origin;//=closestPt;
	
	HVec3 tempA(Constants<HVec3>::Zero());
	HVec3 tempB(Constants<HVec3>::Zero());

	
	
	const HVec3 ao(origin -Q[0]);
	const HVec3 bo(origin -Q[1]);
	const HVec3 ab(Q[1]-Q[0]);
	const HVec3 ac(Q[2]-Q[0]);
	const HVec3 ad(Q[3]-Q[0]);

	const HF32 recipSixth=Reciprocal(Constants<HF32>::Sixth());
	const HF32 volume1 = Abs(ab |(ac^ad))*recipSixth;
	const HF32 volume2 = Abs(ac |(ab^ad))*recipSixth;
	const HF32 volume3 = Abs(ad |(ab^ac))*recipSixth;
	const HF32 volume = Min(volume1, Min(volume2, volume3));


	//The tetrahedron is co-plane
	/*if(ab.All_EQ(ac)|ab.All_EQ(ad)|ac.All_EQ(ad))
	{
		bool coplane = true;
	}
	*/
	if(volume.LT(HF32(1e-4f)) )
	{
		U32 _size = 3;
		HVec3 _Q1[3]={Q[0],Q[1],Q[2]};
		HVec3 _A1[3]={A[0],A[1],A[2]};
		HVec3 _B1[3]={B[0],B[1],B[2]};
		GJKCPairClosestPtPointTriangle(origin, _Q1,_size, _A1, _B1, tempA, tempB);
		const HVec3 q(tempA-tempB);
		const HF32 sqDist(q | q);
		
		closestA=tempA;
		closestB=tempB;
		
		size=_size;
		
		Q[0] = _Q1[0]; Q[1] = _Q1[1]; Q[2] = _Q1[2];
		A[0] = _A1[0]; A[1] = _A1[1]; A[2] = _A1[2];
		B[0] = _B1[0]; B[1] = _B1[1]; B[2] = _B1[2];
			
		return true;
	

	}
	
	const HVec3 bd(Q[3]-Q[1]);
	const HVec3 bc(Q[2]-Q[1]);
	const HVec3 ba(Q[0]-Q[1]);

	const HVec3 abc=ab^ac;
	const HF32 o_abc(ao|abc);
	const HF32 d_abc(ad|abc);

	const HVec3 acd=ac^ad;
	const HF32 o_acd(ao|acd);
	const HF32 b_acd(ab|acd);

	const HVec3 adb=ad^ab;
	const HF32 o_adb(ao|adb);
	const HF32 c_adb(ac|adb);

	const HVec3 bdc=bd^bc;
	const HF32 o_bdc(bo|bdc);
	const HF32 a_bdc(ba|bdc);

	const HVec3 Qt[4]={Q[0], Q[1], Q[2], Q[3]};
	const HVec3 At[4]={A[0], A[1], A[2], A[3]};
	const HVec3 Bt[4]={B[0], B[1], B[2], B[3]};

	U32 _size(size);

	bool bResult = false;
	
	if((o_abc*d_abc).LE(zero))
	{
		HVec3 _Q1[3]={Qt[0],Qt[1],Qt[2]};
		HVec3 _A1[3]={At[0],At[1],At[2]};
		HVec3 _B1[3]={Bt[0],Bt[1],Bt[2]};

		GJKCPairClosestPtPointTriangle(origin, _Q1,_size, _A1, _B1, tempA, tempB);
		const HVec3 q(tempA-tempB);
		const HF32 sqDist(q | q);
		//if(sqDist.LT(bestSqDist)) 
		//{
			bestSqDist=sqDist;
			closestA=tempA;
			closestB=tempB;
			//closestPt=q;
			size=_size;
			Q[0] = _Q1[0]; Q[1] = _Q1[1]; Q[2] = _Q1[2];
			A[0] = _A1[0]; A[1] = _A1[1]; A[2] = _A1[2];
			B[0] = _B1[0]; B[1] = _B1[1]; B[2] = _B1[2];
			
		//}
		bResult = true;
	}
	
	if((o_acd*b_acd).LE(zero))
	{
		//_size=s;
		HVec3 _Q1[3] = {Qt[0],Qt[2],Qt[3]};
		HVec3 _A1[3] = {At[0],At[2],At[3]};
		HVec3 _B1[3] = {Bt[0],Bt[2],Bt[3]};

		GJKCPairClosestPtPointTriangle(origin, _Q1, _size, _A1, _B1, tempA, tempB);
		const HVec3 q(tempA-tempB);
		const HF32 sqDist(q | q);
		if(sqDist.LT(bestSqDist))
		{
			bestSqDist=sqDist;
			closestA=tempA;
			closestB=tempB;
			//closestPt=q;
			size=_size;

			Q[0] = _Q1[0]; Q[1] = _Q1[1]; Q[2] = _Q1[2];
			A[0] = _A1[0]; A[1] = _A1[1]; A[2] = _A1[2];
			B[0] = _B1[0]; B[1] = _B1[1]; B[2] = _B1[2];
		}
		bResult = true;
	}

	
	if((o_adb*c_adb).LE(zero))
	{
		//_size=s;
		HVec3 _Q1[3]={Qt[0],Qt[3],Qt[1]};
		HVec3 _A1[3]={At[0],At[3],At[1]};
		HVec3 _B1[3]={Bt[0],Bt[3],Bt[1]};
		GJKCPairClosestPtPointTriangle(origin, _Q1, _size, _A1, _B1, tempA, tempB);
		const HVec3 q(tempA-tempB);
		const HF32 sqDist(q | q);
		if(sqDist.LT(bestSqDist))
		{
			bestSqDist=sqDist;
			closestA=tempA;
			closestB=tempB;
			//closestPt=q;
			size=_size;

			Q[0] = _Q1[0]; Q[1] = _Q1[1]; Q[2] = _Q1[2];
			A[0] = _A1[0]; A[1] = _A1[1]; A[2] = _A1[2];
			B[0] = _B1[0]; B[1] = _B1[1]; B[2] = _B1[2];
		}
		bResult = true;
	}

	if((o_bdc*a_bdc).LE(zero))
	{
		//_size=s;
		HVec3 _Q1[3]={Qt[1],Qt[3],Qt[2]};
		HVec3 _A1[3]={At[1],At[3],At[2]};
		HVec3 _B1[3]={Bt[1],Bt[3],Bt[2]};

		GJKCPairClosestPtPointTriangle(origin, _Q1, _size, _A1, _B1, tempA, tempB);
		const HVec3 q(tempA-tempB);
		//const HVec3 t(q-p);
		//const HF32 sqDist(t|t);//Dot(q-p, q-p);
		const HF32 sqDist(q | q);
		if(sqDist.LT(bestSqDist)) 
		{
			bestSqDist=sqDist;
			closestA=tempA;
			closestB=tempB;
			//closestPt=q;
			size=_size;
			Q[0] = _Q1[0]; Q[1] = _Q1[1]; Q[2] = _Q1[2];
			A[0] = _A1[0]; A[1] = _A1[1]; A[2] = _A1[2];
			B[0] = _B1[0]; B[1] = _B1[1]; B[2] = _B1[2];
		}
		bResult = true;
	}

	return bResult;
}

bool GJKCPairDoSimplex(const Maths::HVec3 PCREF origin, Maths::HVec3* __restrict Q, Maths::HVec3* __restrict A, Maths::HVec3* __restrict B, U32& size, Maths::HVec3& closestA, Maths::HVec3& closestB)
{
	switch(size)
	{
	case 1:
		{
			closestA = A[0];
			closestB = B[0];
			return true;
		}
	case 2:
		return GJKCPairClosestPtPointSegment(origin, Q, size, A, B, closestA, closestB);
	case 3: 
		return GJKCPairClosestPtPointTriangle(origin, Q, size, A, B, closestA, closestB);
	case 4: 
		return GJKCPairClosestPtPointTetrahedron(origin, Q, size, A, B, closestA, closestB);
	default:
		BIZ_ASSERT(0 && "Undefined Simplex!");
		return false;
	}
	//return _size;
}


/*bool GJKCPairDoSimplex(HVec3* __restrict Q, HVec3* __restrict A, HVec3* __restrict B, HVec3* __restrict Q2, HVec3* __restrict A2, HVec3* __restrict B2, U32& size, HVec3& closestA, HVec3& closestB)
{
	switch(size)
	{

	case 2:
		return GJKCPairClosestPtPointSegment(Q, Q2, size, A, A2, B, B2, closestA, closestB);
	case 3: 
		return GJKCPairClosestPtPointTriangle(Q, Q2, size, A, A2, B, B2, closestA, closestB);
	case 4: 
		return GJKCPairClosestPtPointTetrahedron(Q, Q2, size, A, A2, B, B2, closestA, closestB);
	default:
		assert(0 && "Undefined Simplex!");
		return false;
	}
	//return _size;
}

HVec3 GJKCPairDoSimplex1(HVec3* __restrict Q, HVec3* __restrict A, HVec3* __restrict B, U32& size, HVec3& closestA, HVec3& closestB)
{
	U32 _size(size+1);
	switch(_size)
	{

	case 2:
		{
			return GJKCPairClosestPtPointSegment1(Q, size, A, B, closestA, closestB);
		}
	case 3:
		{
			return GJKCPairClosestPtPointTriangle1(Q, size, A, B, closestA, closestB);
			
		}
	case 4:
		{
			return GJKCPairClosestPtPointTetrahedron1(Q, size, A, B, closestA, closestB);
		}
	default:
		assert(0 && "Undefined Simplex!");
		return Maths::Constants<HVec3>::Zero();
	}
	//return _size;
}


void GJKSupport(GJKConvexHull& a, const HVec3 PCREF d, HVec3& support)
{
	U32 index=0;
	U32 aIndex=index;
	GJKConvexNode* aNode;
	HVec3* aVert;
	HVec3 temp;
	
	GJKConvexNode* startNode=a.getNode(index);
	support=*startNode->getVertex();
	float max=0.f;
	float dis=0.f;

	while(true)
	{
		for(U32 i=0; i< startNode->num; ++i)
		{
			aNode=a.getNode(startNode->index[i]);
			aVert=aNode->getVertex();
			//const HVec3 b(aNode->getVertex());
			temp=(*aVert) - support;
			dis= temp | d;
			if(max < dis)//aNode is more extrem than startNode and some of startNode's neighbour
			{
				max=dis;
				aIndex=i;
				support=*aVert;
			}
		}
		if(aIndex==index)//no neighbour node is more extrem than startNode, break;
			return;
		index=aIndex;
		max=dis=0.f;
		startNode=a.getNode(index);
		support=*startNode->getVertex();
	}	
}

void GJKHillClimbingSupport(GJKConvexHull& a, GJKConvexHull& b, HVec3 PCREF d, HVec3& support)
{
	HVec3 dneg=-d;
	HVec3 temp;
	GJKSupport(a, d, support);
	GJKSupport(a, dneg, temp );
	support-=temp;
}*/

};
