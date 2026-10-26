#ifndef GJK_INL
#define GJK_INL

#include "gravity/narrowphase/Primitive.h"
#include "gravity/narrowphase/GJK.h"
#include "gravity/narrowphase/EPA.inl"

namespace Collision
{

NOALIAS inline bool GJKCPairClosestPtPointSegment(Maths::HVec3*  Q, Maths::HVec3*  Q2, U32& size, Maths::HVec3*  A, Maths::HVec3*  A2, Maths::HVec3*  B, Maths::HVec3*  B2, Maths::HVec3& closestA, Maths::HVec3& closestB)
{
	using namespace Maths;

	const HVec3 Q0(Q[0]);
	const HVec3 Q1(Q[1]);
	const HVec3 ab(Q1-Q0);

	const HF32 zero(Maths::Constants<HF32>::Zero());
	const HF32 one(Maths::Constants<HF32>::One());
	const HVec3 a_ab=A[1] -A[0];
	const HVec3 b_ab =B[1] - B[0];
	const HF32 _t=((-Q0)|ab ) / (ab|ab);
	const HF32 t=Clamp(_t, zero, one);
	const Scalar_Bool b(_t.LE(zero));
	const Scalar_Bool b2(_t.GE(one));
	const Scalar_Bool ored( b | b2 );
	
	Q[0] = Select(b2, Q1, Q0);
	Q2[0] = Select(b2, Q2[1], Q2[0]);
	A[0] = Select(b2, A[1], A[0]);
	A2[0] = Select(b2, A2[1], A2[0]);
	B[0] = Select(b2, B[1], B[0]);
	B2[0] = Select(b2, B2[1], B2[0]);
	closestA = Select(ored, A[0], A[0] + a_ab * t);
	closestB = Select(ored, B[0], B[0] + b_ab * t);

	size = Select(bool(ored), 1, 2);
	return true;
	//closest=a+(ab*t);
}

NOALIAS inline bool GJKCPairClosestPtPointSegment(Maths::HVec3*  Q, U32& size, Maths::HVec3*  A, Maths::HVec3* B, Maths::HVec3& closestA, Maths::HVec3& closestB)
{
	using namespace Maths;

	const HVec3 Q0(Q[0]);
	const HVec3 Q1(Q[1]);
	const HVec3 ab(Q1-Q0);

	//Deal with the degeneration case
	const Maths::HF32 epsilon(Constants<HF32>::Epsilon());
	const Maths::Scalar_Bool bDegenerate((ab | ab).LT(epsilon));
	/*if(ab.Length().LT(Constants<HF32>::Epsilon()))
	{
		size = 1;
		closestA = A[0];
		closestB = B[0];
		return true;
	}*/

	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one(Constants<HF32>::One());
	
	const HF32 _t=((-Q0)|ab ) / (ab|ab);
	//const HF32 t=Clamp(_t, zero, one);
	const HF32 t=_t.Clamp(zero, one);
	const Scalar_Bool b(_t.LE(zero));
	const Scalar_Bool _b2(_t.GE(one));
	const Scalar_Bool b2(_b2 & (!bDegenerate));
	const Scalar_Bool ored( b | _b2 | bDegenerate );
	const bool bOred = (bool)ored;

	Q[0] = Select(b2, Q1, Q0);
	A[0] = Select(b2, A[1], A[0]);
	B[0] = Select(b2, B[1], B[0]);

	const HVec3 a_ab = A[1] - A[0];
	const HVec3 b_ab = B[1] - B[0];

	closestA = Select(ored, A[0], A[0] + a_ab * t);
	closestB = Select(ored, B[0], B[0] + b_ab * t);

	size = Select(bOred, 1, 2);

	return true;
}

NOALIAS inline Maths::HVec3 GJKCPairClosestPtPointSegment1(Maths::HVec3*  Q, U32& size,  Maths::HVec3* A, Maths::HVec3* B, Maths::HVec3& closestA, Maths::HVec3& closestB)
{
	using namespace Maths;

	const HVec3 Q0(Q[0]);
	const HVec3 Q1(Q[1]);
	const HVec3 ab(Q1-Q0);

	const HF32 zero(Maths::Constants<HF32>::Zero());
	const HF32 one(Maths::Constants<HF32>::One());
	const HVec3 a_ab=A[1] -A[0];
	const HVec3 b_ab =B[1] - B[0];
	const HF32 _t=((-Q0)|ab ) / (ab|ab);
	const HF32 t=Clamp(_t, zero, one);
	const Scalar_Bool b(_t.LE(zero));
	const Scalar_Bool b2(_t.GE(one));
	const Scalar_Bool ored( b | b2 );
	
	Q[0] = Select(b2, Q1, Q0);
	A[0] = Select(b2, A[1], A[0]);
	B[0] = Select(b2, B[1], B[0]);
	
	size = Select(bool(ored), 1, 2);

	closestA = Select(ored, A[0], A[0] + a_ab * t);
	closestB = Select(ored, B[0], B[0] + b_ab * t);

	return Select(ored, A[0], A[0] + a_ab * t) - Select(ored, B[0], B[0] + b_ab * t);
}


//GJK Primitives Test, which include the simplex update

NOALIAS inline bool GJKCPairClosestPtPointTriangle(Maths::HVec3*  Q, U32& size, Maths::HVec3*  A, Maths::HVec3*  B, Maths::HVec3& closestA, Maths::HVec3& closestB)
{
	using namespace Maths;

	const HF32 eps(Maths::Constants<HF32>::Epsilon());

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
		if(u.Length2().LT(eps))
		{
			Q[1] = Q[2];
			A[1] = A[2];
			B[1] = B[2];
		}
		U32 _size = 2;
		GJKCPairClosestPtPointSegment(Q, _size, A, B, closestA, closestB);
		size = _size;
		return true;
	}
	const HVec3 a_ab(A[1]-A[0]);
	const HVec3 a_ac(A[2]-A[0]);
	const HVec3 b_ab(B[1]-B[0]);
	const HVec3 b_ac(B[2]-B[0]);
	//const HVec3 _ap(Q[0]); // origin - Q[0]
	//const HVec3 ap(-_ap);
	const HVec3 ap(-Q[0]);

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
	

	const HVec3 bp(-Q[1]);//origin-Q[1];
	const HF32 d3(ab|bp);//Dot(ab, bp);
	const HF32 d4(ac|bp);//Dot(ac, bp);

	//p is in the vertex region b, b is the closest point to p
	if(d3.GE(eps) & d4.LE(d3)) 
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
	if(vc.LE(zero) & d1.GE(eps) & d3.LE(zero))
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

	const HVec3 cp(-Q[2]);//origin-Q[2];
	const HF32 d5(ab|cp);//Dot(ab, cp);
	const HF32 d6(ac|cp);//Dot(ac, cp);
	//P is in the vertex region c, c is the closest point to p
	if(d6.GE(eps) & d5.LE(d6)) 
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
	if(vb.LE(zero) & d2.GE(eps) & d6.LE(zero))
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
	if(va.LE(zero) & d43.GE(eps) & d56.GE(eps))
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

NOALIAS inline bool GJKCPairClosestPtPointTriangle(Maths::HVec3*  Q, Maths::HVec3*  Q2, U32& size, Maths::HVec3*  A, Maths::HVec3*  A2, Maths::HVec3*  B, Maths::HVec3*  B2, Maths::HVec3& closestA, Maths::HVec3& closestB)
{
	using namespace Maths;

	const HF32 zero(Maths::Constants<HF32>::Zero());
	const HVec3 ab(Q[1]-Q[0]);
	const HVec3 ac(Q[2]-Q[0]);
	const HVec3 a_ab(A[1]-A[0]);
	const HVec3 a_ac(A[2]-A[0]);
	const HVec3 b_ab(B[1]-B[0]);
	const HVec3 b_ac(B[2]-B[0]);
	//const HVec3 _ap(Q[0]); // origin - Q[0]
	//const HVec3 ap(-_ap);
	const HVec3 ap(-Q[0]);

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
	

	const HVec3 bp(-Q[1]);//origin-Q[1];
	const HF32 d3(ab|bp);//Dot(ab, bp);
	const HF32 d4(ac|bp);//Dot(ac, bp);

	//p is in the vertex region b, b is the closest point to p
	if(d3.GE(zero) & d4.LE(d3)) 
	{
		size=1;
		Q[0]=Q[1];
		Q2[0] = Q2[1];
		A[0]=A[1];
		A2[0] = A2[1];
		B[0]=B[1];
		B2[0] = B2[1];
		closestA=A[0];
		closestB=B[0];
		return true;
		//return Q[0];
	}

	const HF32 vc((d1*d4) - (d3*d2));
	//P is in the edge region of AB, return projection of P onto AB
	if(vc.LE(zero) & d1.GE(zero) & d3.LE(zero))
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

	const HVec3 cp(-Q[2]);//origin-Q[2];
	const HF32 d5(ab|cp);//Dot(ab, cp);
	const HF32 d6(ac|cp);//Dot(ac, cp);
	//P is in the vertex region c, c is the closest point to p
	if(d6.GE(zero) & d5.LE(d6)) 
	{
		size=1;
		Q[0]=Q[2];
		A[0]=A[2];
		B[0]=B[2];
		Q2[0]=Q2[2];
		A2[0]=A2[2];
		B2[0]=B2[2];
		closestA=A[2];
		closestB=B[2];
		return true;
		//return Q[0];
	}

	const HF32 vb((d5*d2) - (d1*d6));
	//P is in the edge region of AC, return projection of P onto AC
	if(vb.LE(zero) & d2.GE(zero) & d6.LE(zero))
	{
		size=2;
		Q[1]=Q[2];//swap
		A[1]=A[2];
		B[1]=B[2];
		Q2[1]=Q2[2];//swap
		A2[1]=A2[2];
		B2[1]=B2[2];
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
	if(va.LE(zero) & d43.GE(zero) & d56.GE(zero))
	{	
		const HF32 w(d43/(d43+d56));
		size=2;
		Q[0]=Q[2];
		A[0]=A[2];
		B[0]=B[2];
		Q2[0]=Q2[2];
		A2[0]=A2[2];
		B2[0]=B2[2];
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


NOALIAS inline Maths::HVec3 GJKCPairClosestPtPointTriangle1(Maths::HVec3*  Q, U32& size, Maths::HVec3*  A, Maths::HVec3*  B, Maths::HVec3& closestA, Maths::HVec3& closestB)
{
	using namespace Maths;

	const HF32 zero(Maths::Constants<HF32>::Zero());
	const HVec3 ab(Q[1]-Q[0]);
	const HVec3 ac(Q[2]-Q[0]);
	const HVec3 a_ab(A[1]-A[0]);
	const HVec3 a_ac(A[2]-A[0]);
	const HVec3 b_ab(B[1]-B[0]);
	const HVec3 b_ac(B[2]-B[0]);
	//const HVec3 _ap(Q[0]); // origin - Q[0]
	//const HVec3 ap(-_ap);
	const HVec3 ap(-Q[0]);

	const HF32 d1(ab|ap);//Dot(ab, ap);
	const HF32 d2(ac|ap);//Dot(ac, ap);

	//p is in the vertex region a, a is the closest point to p
	
	if(d1.LE(zero)& d2.LE(zero))
	{
		size=1;
		closestA=A[0];
		closestB=B[0];
		//closest=A[0]-B[0];
		return A[0]-B[0];
		//return Q[0];
	}
	

	const HVec3 bp(-Q[1]);//origin-Q[1];
	const HF32 d3(ab|bp);//Dot(ab, bp);
	const HF32 d4(ac|bp);//Dot(ac, bp);

	//p is in the vertex region b, b is the closest point to p
	if(d3.GE(zero) & d4.LE(d3)) 
	{
		size=1;
		Q[0]=Q[1];
		A[0]=A[1];
		B[0]=B[1];
		closestA=A[1];
		closestB=B[1];
		//closest=A[1]-B[1];
		return Q[1];
	}

	const HF32 vc((d1*d4) - (d3*d2));
	//P is in the edge region of AB, return projection of P onto AB
	if(vc.LE(zero) & d1.GE(zero) & d3.LE(zero))
	{
		size=2;
		//const HVec3 a_ab(A[1]-A[0]);
		//const HVec3 b_ab(B[1]-B[0]);
		const HF32 t(d1/(d1-d3));
		closestA=A[0]+a_ab*t;
		closestB=B[0]+b_ab*t;
		//const HVec3 closA(A[0]+a_ab*t);
		//const HVec3 closB(B[0]+b_ab*t);
		//closestA=closA;
		//closestB=closB;
		//closest=closA-closB;
		//return;
		return Q[0] + ab*t;
	}

	const HVec3 cp(-Q[2]);//origin-Q[2];
	const HF32 d5(ab|cp);//Dot(ab, cp);
	const HF32 d6(ac|cp);//Dot(ac, cp);
	//P is in the vertex region c, c is the closest point to p
	if(d6.GE(zero) & d5.LE(d6)) 
	{
		size=1;
		Q[0]=Q[2];
		A[0]=A[2];
		B[0]=B[2];
		closestA=A[2];
		closestB=B[2];
		//closest=A[2]-B[2];
		//return;
		return Q[2];
	}

	const HF32 vb((d5*d2) - (d1*d6));
	//P is in the edge region of AC, return projection of P onto AC
	if(vb.LE(zero) & d2.GE(zero) & d6.LE(zero))
	{
		size=2;
		Q[1]=Q[2];//swap
		A[1]=A[2];
		B[1]=B[2];
		const HF32 t(d2/(d2-d6));
		closestA=A[0]+a_ac*t;
		closestB=B[0]+b_ac*t;

		//const HVec3 closA(A[0]+a_ac*t);
		//const HVec3 closB(B[0]+b_ac*t);
		//closestA=closA;
		//closestB=closB;
		//closest=closA-closB;
		//return;
		return Q[0]+ ac*t;
	}

	const HF32 va((d3*d6) - (d5*d4));
	const HF32 d43(d4-d3);
	const HF32 d56(d5-d6);
	//P is in the edge region of BC, return the projection of P onto BC
	if(va.LE(zero) & d43.GE(zero) & d56.GE(zero))
	{	
		const HF32 w(d43/(d43+d56));
		size=2;
		Q[0]=Q[2];
		A[0]=A[2];
		B[0]=B[2];
		closestA=A[1]+(A[2]-A[1])*w;
		closestB=B[1]+(B[2]-B[1])*w;

		//const HVec3 closA(A[1]+(A[2]-A[1])*w);
		//const HVec3 closB(B[1]+(B[2]-B[1])*w);
		//closestA=closA;
		//closestB=closB;
		//closest=closA-closB;
		//return;
		return Q[1]+(Q[2]-Q[1])*w;//b+ (c-b)*w;
	}

	size=3;
	//P is inside the face region, return the projection of P onto abc
	//const HF32 denom(1.f/(va+vb+vc));
	const HF32 denom(Reciprocal(va+vb+vc));
	const HF32 v(vb*denom);
	const HF32 w(vc*denom);
	closestA=A[0]+a_ab*v+a_ac*w;
	closestB=B[0]+b_ab*v+b_ac*w;

	//const HVec3 closA(A[0]+a_ab*v+a_ac*w);
	//const HVec3 closB(B[0]+b_ab*v+b_ac*w);
	//closestA=closA;
	//closestB=closB;
	//closest=closA-closB;
	//return;
	return Q[0] + ab*v + ac*w;
}


NOALIAS inline bool GJKCPairClosestPtPointTetrahedron(Maths::HVec3* Q, Maths::HVec3*  Q2, U32& size, Maths::HVec3*  A, Maths::HVec3* A2, Maths::HVec3*  B, Maths::HVec3*  B2, Maths::HVec3& closestA, Maths::HVec3& closestB)
{
	using namespace Maths;

	const HVec3 origin(Maths::Constants<HVec3>::Zero());
	//const HVec3 closestPt(Maths::Constants<HVec3>::Zero());
	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Epsilon());
	HF32 bestSqDist(Maths::Constants<HF32>::Max());
	closestA=origin;//set the closestPt as origin first
	closestB=origin;//=closestPt;
	
	HVec3 tempA(Maths::Constants<HVec3>::Zero());
	HVec3 tempB(Maths::Constants<HVec3>::Zero());
	
	
	const HVec3 ao(-Q[0]);
	const HVec3 bo(-Q[1]);
	const HVec3 ab(Q[1]-Q[0]);
	const HVec3 ac(Q[2]-Q[0]);
	const HVec3 ad(Q[3]-Q[0]);

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

	const HVec3 Qt2[4]={Q2[0], Q2[1], Q2[2], Q2[3]};
	const HVec3 At2[4]={A2[0], A2[1], A2[2], A2[3]};
	const HVec3 Bt2[4]={B2[0], B2[1], B2[2], B2[3]};

	bool bResult = false;

	U32 _size(size);

	
	if((o_abc*d_abc).LE(zero))
	{
		HVec3 _Q1[3]={Qt[0],Qt[1],Qt[2]};
		HVec3 _A1[3]={At[0],At[1],At[2]};
		HVec3 _B1[3]={Bt[0],Bt[1],Bt[2]};

		HVec3 _Q2[3]={Qt2[0],Qt2[1],Qt2[2]};
		HVec3 _A2[3]={At2[0],At2[1],At2[2]};
		HVec3 _B2[3]={Bt2[0],Bt2[1],Bt2[2]};

		GJKCPairClosestPtPointTriangle(_Q1,_Q2,_size, _A1, _A2, _B1, _B2, tempA, tempB);
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

			Q2[0] = _Q2[0]; Q2[1] = _Q2[1]; Q2[2] = _Q2[2];
			A2[0] = _A2[0]; A2[1] = _A2[1]; A2[2] = _A2[2];
			B2[0] = _B2[0]; B2[1] = _B2[1]; B2[2] = _B2[2];
			
		//}
			bResult = true;
	}
	

	

	if((o_acd*b_acd).LE(zero))
	{
		//_size=s;
		HVec3 _Q1[3] = {Qt[0],Qt[2],Qt[3]};
		HVec3 _A1[3] = {At[0],At[2],At[3]};
		HVec3 _B1[3] = {Bt[0],Bt[2],Bt[3]};

		HVec3 _Q2[3] = {Qt2[0],Qt2[2],Qt2[3]};
		HVec3 _A2[3] = {At2[0],At2[2],At2[3]};
		HVec3 _B2[3] = {Bt2[0],Bt2[2],Bt2[3]};

		GJKCPairClosestPtPointTriangle(_Q1, _Q2, _size, _A1, _A2, _B1, _B2, tempA, tempB);
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

			Q2[0] = _Q2[0]; Q2[1] = _Q2[1]; Q2[2] = _Q2[2];
			A2[0] = _A2[0]; A2[1] = _A2[1]; A2[2] = _A2[2];
			B2[0] = _B2[0]; B2[1] = _B2[1]; B2[2] = _B2[2];
			
		}
		bResult = true;
	}

	
	if((o_adb*c_adb).LE(zero))
	{
		//_size=s;
		HVec3 _Q1[3]={Qt[0],Qt[3],Qt[1]};
		HVec3 _A1[3]={At[0],At[3],At[1]};
		HVec3 _B1[3]={Bt[0],Bt[3],Bt[1]};

		HVec3 _Q2[3]={Qt2[0],Qt2[3],Qt2[1]};
		HVec3 _A2[3]={At2[0],At2[3],At2[1]};
		HVec3 _B2[3]={Bt2[0],Bt2[3],Bt2[1]};
		GJKCPairClosestPtPointTriangle(_Q1, _Q2, _size, _A1, _A2, _B1, _B2, tempA, tempB);
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

			Q2[0] = _Q2[0]; Q2[1] = _Q2[1]; Q2[2] = _Q2[2];
			A2[0] = _A2[0]; A2[1] = _A2[1]; A2[2] = _A2[2];
			B2[0] = _B2[0]; B2[1] = _B2[1]; B2[2] = _B2[2];
		}
		bResult = true;
	}

	if((o_bdc*a_bdc).LE(zero))
	{
		//_size=s;
		HVec3 _Q1[3]={Qt[1],Qt[3],Qt[2]};
		HVec3 _A1[3]={At[1],At[3],At[2]};
		HVec3 _B1[3]={Bt[1],Bt[3],Bt[2]};

		HVec3 _Q2[3]={Qt2[1],Qt2[3],Qt2[2]};
		HVec3 _A2[3]={At2[1],At2[3],At2[2]};
		HVec3 _B2[3]={Bt2[1],Bt2[3],Bt2[2]};

		GJKCPairClosestPtPointTriangle(_Q1, _Q2, _size, _A1, _A2, _B1, _B2, tempA, tempB);
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

			Q2[0] = _Q2[0]; Q2[1] = _Q2[1]; Q2[2] = _Q2[2];
			A2[0] = _A2[0]; A2[1] = _A2[1]; A2[2] = _A2[2];
			B2[0] = _B2[0]; B2[1] = _B2[1]; B2[2] = _B2[2];
		}
		bResult = true;
	}
	return bResult;
}

NOALIAS inline bool GJKCPairClosestPtPointTetrahedron(Maths::HVec3*  Q, U32& size, Maths::HVec3*  A, Maths::HVec3*  B, Maths::HVec3& closestA, Maths::HVec3& closestB)
{
	using namespace Maths;

	const HVec3 origin(Constants<HVec3>::Zero());
	//const HVec3 closestPt(Maths::Constants<HVec3>::Zero());
	const HF32 zero(Constants<HF32>::Zero());
	HF32 bestSqDist(Constants<HF32>::Max());
	closestA=origin;//set the closestPt as origin first
	closestB=origin;//=closestPt;
	
	HVec3 tempA(Constants<HVec3>::Zero());
	HVec3 tempB(Constants<HVec3>::Zero());

	
	
	const HVec3 ao(-Q[0]);
	const HVec3 bo(-Q[1]);
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
		GJKCPairClosestPtPointTriangle(_Q1,_size, _A1, _B1, tempA, tempB);
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

		GJKCPairClosestPtPointTriangle(_Q1,_size, _A1, _B1, tempA, tempB);
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

		GJKCPairClosestPtPointTriangle(_Q1, _size, _A1, _B1, tempA, tempB);
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
		GJKCPairClosestPtPointTriangle(_Q1, _size, _A1, _B1, tempA, tempB);
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

		GJKCPairClosestPtPointTriangle(_Q1, _size, _A1, _B1, tempA, tempB);
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

#ifndef PLATFORM_SPU

template <class ConvexA, class ConvexB>
bool GJKCPair(ConvexA& a, ConvexB& b)
{
	using namespace Maths;

	U32 size=1;
	//HVec3 origin(Constants<HVec3>::Zero());				//(0.0f, 0.0f, 0.0f)

	const HVec3 initialSearchDir(a.Centre() - b.Centre());
	const HVec3 initialSupportA(a.Support(-initialSearchDir));
	const HVec3 initialSupportB(b.Support(initialSearchDir));


	HVec3 Q[4] = {initialSupportA - initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//simplex set
	HVec3 A[4] = {initialSupportA, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull a simplex set
	HVec3 B[4] = {initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull b simplex set
	

	//HVec3 upper(Maths::Constants<HVec3>::Max());
	HVec3 closestPt(Q[0]);
	HVec3 supportA(initialSupportA);
	HVec3 supportB(initialSupportB);
	const HF32 epsilon(Maths::Min(a.Epsilon(), b.Epsilon()));
	HVec3 closA(initialSupportA), closB(initialSupportB);
	HF32 sDist(closestPt|closestPt);

	S32 maxIterations = 10;

	//while(!(closestPt.All_EQ(origin)) && (maxIterations --))
	while( sDist.GT(epsilon) && (maxIterations --))
	{
		
		supportA=a.Support(-closestPt);//aSup(a, -closestPt);
		supportB=b.Support(closestPt);//bSup(b, closestPt);
		
		//calculate the support point
		const HVec3 support(supportA-supportB);
		
		const HF32 sup_val(sDist -(support|closestPt));
		
		//const HVec3 supDelta(oldSupport - support);
		//const HF32 dif(supDelta | supDelta);
		//const Scalar_Bool b(sup_val.LE(epsilon) | dif.LE(epsilon));
		if(sup_val.LE(epsilon))
			return false;
	
		A[size]=supportA;
		B[size]=supportB;
		Q[size++]=support;

		//calculate the closest point between two convex hull
		GJKCPairDoSimplex(Q, A, B, size, closA, closB);

		closestPt = (closA - closB);
		sDist = closestPt|closestPt;
	}

	return maxIterations != -1;
	//if(maxIterations == -1)
	//	return false;
	//
	//return true; //intersect, closest point is origin

}

template <class ConvexA, class ConvexB>
bool GJKCPair(ConvexA& a, ConvexB& b, Maths::HF32& dis, Maths::HVec3& closestA, Maths::HVec3& closestB)
{
	using namespace Maths;

	U32 size=1;
	HVec3 origin(Constants<HVec3>::Zero());				//(0.0f, 0.0f, 0.0f)

	const HVec3 initialSearchDir(a.Centre() - b.Centre());
	const HVec3 initialSupportA(a.Support(-initialSearchDir));
	const HVec3 initialSupportB(b.Support(initialSearchDir));


	HVec3 Q[4] = {initialSupportA - initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//simplex set
	HVec3 A[4] = {initialSupportA, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull a simplex set
	HVec3 B[4] = {initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull b simplex set
	

	//HVec3 upper(Maths::Constants<HVec3>::Max());
	HVec3 closestPt(Q[0]);
	HVec3 supportA(initialSupportA);
	HVec3 supportB(initialSupportB);
	const HF32 epsilon(Maths::Constants<HF32>::Epsilon());
	HVec3 closA(initialSupportA), closB(initialSupportB);
	HF32 sDist(closestPt|closestPt);

	S32 maxIterations = 10;

	bool result = true;
	//while(!(closestPt.All_EQ(origin)) && (maxIterations --))
	while( sDist.GT(epsilon) && (maxIterations --))
	{
		
		supportA=a.Support(-closestPt);//aSup(a, -closestPt);
		supportB=b.Support(closestPt);//bSup(b, closestPt);
		
		//calculate the support point
		const HVec3 support(supportA-supportB);
		
		const HF32 sup_val((closestPt|closestPt)-(support|closestPt));
		
		//const HVec3 supDelta(oldSupport - support);
		//const HF32 dif(supDelta | supDelta);
		//const Scalar_Bool b(sup_val.LE(epsilon) | dif.LE(epsilon));
		if(sup_val.LE(epsilon))
		{
			result = false;
			break;
		}
	
		A[size]=supportA;
		B[size]=supportB;
		Q[size++]=support;

		//calculate the closest point between two convex hull
		GJKCPairDoSimplex(Q, A, B, size, closA, closB);

		closestPt = (closA - closB);
		sDist = closestPt|closestPt;
	}

	if(maxIterations == -1)
		result = false;
	dis = closestPt.Length();
	closestA = closA;
	closestB = closB;
	a.WarmStart(supportA);
	b.WarmStart(supportB);
	
	return result; //intersect, closest point is origin
	
}

template <class ConvexA, class ConvexB>
bool GJKCPair(ConvexA& a, ConvexB& b, Maths::HVec3& contact, Maths::HVec3& normal, Maths::HF32& penetration, EPA& epa)
{
	using namespace Maths;
	//const HF32 zero(Maths::Constants<HF32>::Epsilon());//*Maths::Constants<Maths::HF32>::Epsilon2());
	U32 size=1;
	//HVec3 origin(Maths::Constants<HVec3>::Zero());				//(0.0f, 0.0f, 0.0f)
	const HF32 _zero(Constants<HF32>::Zero());

	const HVec3 assumedNormDir(a.Centre() - b.Centre());
	const HF32 half(Constants<HF32>::Half());
	
	const HVec3 initialSearchDir(assumedNormDir);
	const HVec3 initialSupportA(a.Support(-initialSearchDir));
	const HVec3 initialSupportB(b.Support(initialSearchDir));


	HVec3 Q[4] = {initialSupportA - initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//simplex set
	HVec3 A[4] = {initialSupportA, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull a simplex set
	HVec3 B[4] = {initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull b simplex set
	

	//HF32 upper(Maths::Constants<HF32>::Epsilon());
	HVec3 closestPt(Q[0]);
	
	const HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon());
	const HF32 zero(epsilon*epsilon);

	HF32 sDist(closestPt|closestPt);
	
	GJKResultType result = E_DEEP_INTERSECTION;

	Maths::HVec3 closestA(initialSupportA), closestB(initialSupportB);

	S32 maxIterations = 10;
	
	while(U32(sDist > zero) && (maxIterations--))//(!(closestPt.All_EQ(origin)))
	{	
		
		const HVec3 supportA=a.Support(-closestPt);//aSup(a, -closestPt);
		const HVec3 supportB=b.Support(closestPt);//bSup(b, closestPt);
		
		//calculate the support point
		const HVec3 support(supportA-supportB);

		const HF32 sup_val(sDist-(support|closestPt));
		
		if(sup_val.LE(epsilon))// | dif.LE(zero)) //the objects don't intersect with each other, but they might intersect in the margin
			return false;	

		A[size]=supportA;
		B[size]=supportB;
		Q[size++]=support;

		//calculate the closest point between two convex hull, the size of simplex will be reseted in this method
		//Maths::Scalar_Bool bResult(GJKCPairDoSimplex(Q, A, B, Q2, A2, B2, size,closA, closB));
		GJKCPairDoSimplex(Q, A, B, size,closestA, closestB);
		
		closestPt = (closestA - closestB);
		sDist=closestPt|closestPt;
	}

	//Which mean the support point is jump between two points, we just simple return no intersection
	if(maxIterations == -1)
	{
		return false;
	}

//#ifdef PLATFORM_SPU
//	epa.PenetrationDepth(a, b, Q, A, B, size, closestA, closestB);
//#else
 	epa.PenetrationDepth<ConvexA, ConvexB>(a, b, Q, A, B, size, closestA, closestB); //The original objects intersect with each other
//#endif	
	const HVec3 dir = closestA - closestB;
	normal = C_Normal(dir);
	contact = closestA;
	penetration = dir.Length();
	/*a.m_wStart = closestA;
	b.m_wStart = closestB;*/
	a.WarmStart(closestA);
	b.WarmStart(closestB);
	return true;
	
}


template <class ConvexA, class ConvexB>
bool GJKCPairMargin(ConvexA& a, ConvexB& b, Maths::HVec3& contact, Maths::HVec3& normal, Maths::HF32& penetration, EPA& epa)
{
	using namespace Maths;
	U32 size=1;
	
	
	bool bQuadratic = a.QuadraticShape() | b.QuadraticShape();
	const HF32 minMargin(Min(a.Margin(), b.Margin()));

	const HF32 marginA(bQuadratic ? a.Margin() : minMargin);
	const HF32 marginB(bQuadratic ? b.Margin() : minMargin);
	const HF32 margin(marginA + marginB);
	const HF32 sqMargin(margin*margin);
	//const HF32 marginExtrusionA(marginA);//a.QuadraticShape() ? _zero : minMargin);
	//const HF32 marginExtrusionB(marginB);//b.QuadraticShape() ? _zero : minMargin);
	//const HF32 marginExtrusion(marginExtrusionA + marginExtrusionB);
	//const HF32 sqMarginExtrusion(marginExtrusion * marginExtrusion);

	//const HF32 epsilon(Min(Min(a.Epsilon(), b.Epsilon()), minMargin));//Maths::Constants<HF32>::Epsilon());
	const HF32 epsilon(Min(a.Epsilon(), b.Epsilon()));//Maths::Constants<HF32>::Epsilon());
	//const HF32 zero(epsilon*epsilon);
	const HF32 epsilonSq(epsilon*epsilon);

	const HF32 _zero(Constants<HF32>::Zero());
	const HVec3 _initialSearchDir(a.WarmStart() - b.WarmStart());
	//BIZ_ASSERT((_initialSearchDir.X().EQ(_zero) & _initialSearchDir.Y().EQ(_zero) & _initialSearchDir.Z().EQ(_zero))!=1);
	const HVec3 initialSearchDir(Maths::Select(_initialSearchDir.Length2().GT(epsilon), _initialSearchDir, Maths::Constants<Maths::HVec3>::UnitAxisPositiveY()));
	//const HVec3 initialSearchDir(a.Centre() - b.Centre());
	
	const HVec3 initialSupportA(a.SupportMargin(-initialSearchDir, marginA));
	const HVec3 initialSupportB(b.SupportMargin(initialSearchDir, marginB));
	
	
	HVec3 Q[4] = {initialSupportA - initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//simplex set
	HVec3 A[4] = {initialSupportA, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull a simplex set
	HVec3 B[4] = {initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull b simplex set
	
	HVec3 closestPt(Q[0]);
	HF32 sqDist(closestPt|closestPt);
	
	GJKResultType result = E_DEEP_INTERSECTION;

	HVec3 closestA(initialSupportA), closestB(initialSupportB);

	S32 maxIterations = 10;

	HVec3 supportA(initialSupportA);
	HVec3 supportB(initialSupportB); 
	
	//while(U32(sqDist > zero) && (maxIterations--))//(!(closestPt.All_EQ(origin)))
	do
	{	
		supportA=a.SupportMargin(-closestPt, marginA);//aSup(a, -closestPt);
		supportB=b.SupportMargin(closestPt, marginB);//bSup(b, closestPt);
		
		//calculate the support point
		const HVec3 support(supportA-supportB);
		const HF32 temp(support|closestPt);
		const HF32 con2(sqDist-temp);
		const HF32 sTemp(temp*temp);
		const HF32 con1(sTemp/sqDist);

		//The search direction is -closestPt, therefore, temp should be large than zero to guarantee it is the closest point 
		if(temp.GT(epsilonSq) & con1.GT(sqMargin))
		{
			/*a.WarmStart(closestA);
			b.WarmStart(closestB); */
			a.WarmStart(supportA);
			b.WarmStart(supportB);
			//we don't need the exact closest point
			return false;
		}

		if(con2.LT(epsilon) & sqDist.LE(sqMargin))
		{
			result = E_MARGIN_INTERSECTION;
			break;
		}

		BIZ_ASSERT(size < 4);
		A[size]=supportA;
		B[size]=supportB;
		Q[size++]=support;

		//calculate the closest point between two convex hull, the size of simplex will be reseted in this method
		//Maths::Scalar_Bool bResult(GJKCPairDoSimplex(Q, A, B, Q2, A2, B2, size,closA, closB));
		GJKCPairDoSimplex(Q, A, B, size,closestA, closestB);
		
		closestPt = closestA - closestB;
		sqDist=closestPt|closestPt;
	}
	while(U32(sqDist > epsilonSq) && (maxIterations--));//(!(closestPt.All_EQ(origin)))

	//Which mean the support point is jump between two points, we just simple return no intersection
	if(maxIterations == -1)
	{
		return false;
	}

	
	if(result == E_MARGIN_INTERSECTION)
	{
		const C_Normalised normalised(closestPt.Normalised());
		const HF32 dis(normalised.Length());
		normal = -normalised.Normal();
		contact = closestA + normal*marginA;
		penetration = margin - dis;

		a.WarmStart(supportA);
		b.WarmStart(supportB);

		return true;
	}
	else if(result == E_DEEP_INTERSECTION)
	{
 		bool bResult = epa.PenetrationDepthMargin<ConvexA, ConvexB>(a, b, Q, A, B, size, closestA, closestB, marginA, marginB); //The original objects intersect with each other
		const HVec3 cAcB(closestA - closestB);
		const C_Normalised normalised(cAcB.Normalised());
		const HF32 len(normalised.Length());
		normal = normalised.Normal();
		contact = closestA + normal * marginA;
		penetration = len  + margin;
		a.WarmStart(supportA);
		b.WarmStart(supportB);
		return bResult;
	
	}
	return false;
}
 
template <class ConvexA, class ConvexB>
bool GJKRayCastPenteraction(const ConvexA& a, const ConvexB& b, const Maths::HVec3 PCREF r, const Maths::HF32 PCREF maxLambda, Maths::HF32& lambda, Maths::HVec3& normal,  
				 Maths::HVec3& closestA, Maths::HVec3& closestB, EPA& epa)
{
	using namespace Maths;

	U32 size=1;
	
	//const HVec3 assumedNormDir(a.Centre() - b.Centre());
	const HVec3 initialSearchDir(a.Centre() - b.Centre());
	const HVec3 initialSupportA(a.Support(-initialSearchDir));
	const HVec3 initialSupportB(b.Support(initialSearchDir));


	HVec3 Q[4] = {initialSupportA - initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//simplex set
	HVec3 A[4] = {initialSupportA, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull a simplex set
	HVec3 B[4] = {initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull b simplex set
	
	const HF32 _zero(Constants<HF32>::Zero());
	const HF32 _one(Constants<HF32>::One());
	const HVec3 zeroV(Constants<HVec3>::Zero());

	//Initialize the ray
	HVec3 x(zeroV);//the clipping point of the ray
	HF32 _lambda(_zero);
	HVec3 v(x - Q[0]);

	HVec3 supportA(initialSupportA);
	HVec3 supportB(initialSupportB);
	
	const HF32 epsilon(Min(a.Epsilon(), b.Epsilon()));
	HVec3 closA(initialSupportA), closB(initialSupportB);
	HF32 sDist(v|v);

	S32 maxIterations = 10;

	while(sDist.GT(epsilon) && (maxIterations --))
	{
		
		supportA=a.Support(v);//aSup(a, -closestPt);
		supportB=b.Support(-v);//bSup(b, closestPt);
		
		//calculate the support point
		const HVec3 support(supportA-supportB);

		const HVec3 w(x - support);

		const HF32 vw(v|w);
		const HF32 vr(v|r);
		
		if(vw.GT(_zero))
		{
			//The ray point away from the objects. no interception , reject
			if(vr.GE(_zero))
				return false;
			else
			{
				_lambda = _lambda - (vw/vr);

				if(_lambda.GT(maxLambda) || _lambda.LT(_zero))
					return false; // the ray hit C but not in next frame
				x = _lambda*r;
				size = 0;
				maxIterations = 10;
			}
		}
		
		A[size]=supportA;
		B[size]=supportB;
		Q[size++]=support;

		//calculate the closest point between two convex hull
		GJKCPairDoSimplex(x, Q, A, B, size, closA, closB);

		v = x - (closA - closB);
		sDist = v|v;

	}

	if(maxIterations == -1)
		return false;
	//Already intersect, go to EPA
	if(_lambda == _zero)
	{
//#ifdef PLATFORM_SPU
//		epa.PenetrationDepth(a, b, Q, A, B, size, closestA, closestB);
//#else
 		epa.PenetrationDepth<ConvexA, ConvexB>(a, b, Q, A, B, size, closestA, closestB); //The original objects intersect with each other
//#endif
	}

	//Continuous collision detection 
	lambda = _lambda;
	normal = Maths::Normalise(v);
	closestA = closA;
	closestB = closB;
	//a.m_wStart=supportA;
	//b.m_wStart=supportB;
	a.WarmStart(closestA);
	b.WarmStart(closestB);
	return true; 
	
}

template <class ConvexA, class ConvexB>
bool GJKRayCast(ConvexA& a,ConvexB& b, const Maths::HVec3 PCREF r, const Maths::HF32 PCREF maxLambda, Maths::HF32& lambda, Maths::HVec3& normal,  Maths::HVec3& closestA, Maths::HVec3& closestB)
{
	using namespace Maths;

	U32 size=1;
	
	//const HVec3 assumedNormDir(a.Centre() - b.Centre());
	const HVec3 initialSearchDir(a.Centre() - b.Centre());
	const HVec3 initialSupportA(a.Support(-initialSearchDir));
	const HVec3 initialSupportB(b.Support(initialSearchDir));


	HVec3 Q[4] = {initialSupportA - initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//simplex set
	HVec3 A[4] = {initialSupportA, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull a simplex set
	HVec3 B[4] = {initialSupportB, HVec3(UNDEFINED), HVec3(UNDEFINED), HVec3(UNDEFINED)};				//ConvexHull b simplex set
	
	const HF32 _zero(Constants<HF32>::Zero());
	const HF32 _one(Constants<HF32>::One());
	const HVec3 zeroV(Constants<HVec3>::Zero());

	//Initialize the ray
	HVec3 x(zeroV);//the clipping point of the ray
	HF32 _lambda(_zero);
	HVec3 v(x - Q[0]);

	HVec3 supportA(initialSupportA);
	HVec3 supportB(initialSupportB);
	
	const HF32 epsilon(Min(a.Epsilon(), b.Epsilon()));
	HVec3 closA(initialSupportA), closB(initialSupportB);
	HF32 sDist(v|v);

	S32 maxIterations = 10;

	//
	do
	{
		
		supportA=a.Support(v);//aSup(a, -closestPt);
		supportB=b.Support(-v);//bSup(b, closestPt);
		
		//calculate the support point
		const HVec3 support(supportA-supportB);

		const HVec3 w(x - support);

		const HF32 vw(v|w);
		const HF32 vr(v|r);
		
		if(vw.GT(_zero))
		{
			//The ray point away from the objects. no interception , reject
			if(vr.GE(_zero))
				return false;
			else
			{
				_lambda = _lambda - (vw/vr);

				if(_lambda.GT(maxLambda) | _lambda.LT(_zero))
					return false; // the ray hit C but not in next frame
				x = _lambda*r;
				size = 0;
				maxIterations = 10;
			}
		}
		
		A[size]=supportA;
		B[size]=supportB;
		Q[size++]=support;

		//calculate the closest point between two convex hull
		GJKCPairDoSimplex(x, Q, A, B, size, closA, closB);

		if(size == 4)
			break;

		v = x - (closA - closB);
		sDist = v|v;

	}
	while(sDist.GT(epsilon) && (maxIterations --));

	lambda = _lambda;
	normal = Maths::Normalise(v);
	//normal = Maths::Normalise(closB - closA);
	closestA = closA;
	closestB = closB;
	/*a.m_wStart=supportA;
	b.m_wStart=supportB;*/
	/*a.WarmStart(closA);
	b.WarmStart(closB);*/
	return (maxIterations != -1); 
	
}

#else

template <class ConvexA, class ConvexB>
bool GJKCPair(ConvexA& a, ConvexB& b)
{
	return _GJKCPair(&a, &b);
}

template <class ConvexA, class ConvexB>
bool GJKCPair(ConvexA& a, ConvexB& b, Maths::HF32& dis, Maths::HVec3& closestA, Maths::HVec3& closestB)
{
	return _GJKCPair(&a, &b, dis, closestA, closestB);
}

//This one get out the penetration depth but without margin. The impleementation is in GJK
template <class ConvexA, class ConvexB>
bool GJKCPair(ConvexA& a, ConvexB& b, Maths::HVec3& contact, Maths::HVec3& normal, Maths::HF32& penetration, EPA& epa)
{
	return _GJKCPair(&a, &b, contact, normal, penetration, epa);
}

template <class ConvexA, class ConvexB>
bool GJKCPairMargin(ConvexA& a, ConvexB& b, Maths::HVec3 & contact, Maths::HVec3& norm, Maths::HF32& penetration, EPA& epa)
{
	return _GJKCPairMargin(&a, &b, contact, norm, penetration, epa);
}

template <class ConvexA, class ConvexB>
bool GJKRayCast(ConvexA& a, ConvexB& b, const Maths::HVec3 PCREF r, const Maths::HF32 PCREF maxLambda, Maths::HF32& lambda, Maths::HVec3& normal,  Maths::HVec3& closestA, Maths::HVec3& closestB)
{
	return _GJKRayCast(&a, &b, r, maxLambda, lambda, normal, closestA, closestB);
}
#endif



};

#endif

