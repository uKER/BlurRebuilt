#include "stdafx.h"
#include "gravity/narrowphase/GJK.h"

//using namespace Maths;

namespace Collision
{
//GJK Primitives Test, which include the simplex update
Maths::HVec3 GJKIntersectClosestPtPointSegment(Maths::HVec3* Q, U32& size)
{
	using namespace Maths;

	const HVec3 ab=Q[1]-Q[0];
	const HF32 t=((-Q[0])|ab ) / (ab|ab);//Dot(origin-Q[0], ab)/Dot(ab, ab);
	
	if(t < 0.f)
	{
		size=1;//the closest point is Q[0], therefore the simplex should be just one element a
		return Q[0];
	}
	if(t > 1.f)
	{
		Q[0]=Q[1];//the closest point is Q[1], therefore the simplex should be just one element b
		size=1;
		return Q[0];
	}
	return Q[0]+ ab*t; // teh closest point lie in line ab, therefore the simplex should not be changed
}

Maths::HVec3 GJKIntersectClosestPtPointTriangle(Maths::HVec3* Q, U32& size)
{
	using namespace Maths;

	const HF32 zero(Maths::Constants<HF32>::Zero());
	const HVec3 ab(Q[1]-Q[0]);
	const HVec3 ac(Q[2]-Q[0]);
	const HVec3 ap(-Q[0]); // origin - Q[0]

	const HF32 d1(ab|ap);//Dot(ab, ap);
	const HF32 d2(ac|ap);//Dot(ac, ap);

	//p is in the vertex region a, a is the closest point to p
	
	if(d1.LE(zero)&& d2.LE(zero))
	{
		size=1;
		return Q[0];
	}
	

	const HVec3 bp(-Q[1]);//origin-Q[1];
	const HF32 d3(ab|bp);//Dot(ab, bp);
	const HF32 d4(ac|bp);//Dot(ac, bp);

	//p is in the vertex region b, b is the closest point to p
	if(d3.GE(zero) && d4.LE(d3)) 
	{
		size=1;
		Q[0]=Q[1];
		return Q[0];
	}

	const HF32 vc((d1*d4) - (d3*d2));
	//P is in the edge region of AB, return projection of P onto AB
	if(vc.LE(zero) && d1.GE(zero) && d3.LE(zero))
	{
		size=2;
		return Q[0] + ab*(d1/(d1-d3));
	}

	const HVec3 cp(-Q[2]);//origin-Q[2];
	const HF32 d5(ab|cp);//Dot(ab, cp);
	const HF32 d6(ac|cp);//Dot(ac, cp);
	//P is in the vertex region c, c is the closest point to p
	if(d6.GE(zero) && d5.LE(d6)) 
	{
		size=1;
		Q[0]=Q[2];
		return Q[0];
	}

	const HF32 vb((d5*d2) - (d1*d6));
	//P is in the edge region of AC, return projection of P onto AC
	if(vb.LE(zero) && d2.GE(zero) && d6.LE(zero))
	{
		size=2;
		Q[1]=Q[2];//swap
		return Q[0]+ ac*(d2/(d2-d6));
	}

	const HF32 va((d3*d6) - (d5*d4));
	const HF32 d43(d4-d3);
	const HF32 d56(d5-d6);
	//P is in the edge region of BC, return the projection of P onto BC
	if(va.LE(zero) && d43.GE(zero) && d56.GE(zero))
	{	
		const HF32 w(d43/(d43+d56));
		size=2;
		Q[0]=Q[2];
		return Q[1]+(Q[0]-Q[1])*w;//b+ (c-b)*w;
	}

	size=3;
	//P is inside the face region, return the projection of P onto abc
	HF32 denom=1.f/(va+vb+vc);
	HF32 v=vb*denom;
	HF32 w=vc*denom;
	return Q[0] + ab*v + ac*w;
}


Maths::HVec3 GJKIntersectClosestPtPointTetrahedron(Maths::HVec3* Q, U32& size)
{
	using namespace Maths;
	//const HVec3 origin(Maths::Constants<HVec3>::Zero()); 
	HVec3 closestPt(Maths::Constants<HVec3>::Zero());//set the closestPt as origin first
	HF32 bestSqDist=FLT_MAX;

	const HVec3 Qt[4]={Q[0], Q[1], Q[2], Q[3]};
	U32 _size(size);
	if(GJKPointOutsideOfPlane(Qt[0], Qt[1], Qt[2], Qt[3]))
	{
		HVec3 _Q[3]={Qt[0], Qt[1],Qt[2]};
		const HVec3 q=GJKIntersectClosestPtPointTriangle(_Q, _size);
		//const HVec3 t(q-p);
		//const HF32 sqDist(t|t);//Dot(q-p, q-p);
		const HF32 sqDist(q | q);
		if(sqDist.LT(bestSqDist)) 
		{
			bestSqDist=sqDist;
			closestPt=q;
			size=_size;
			for(U32 i=0; i<size; ++i)
			{
				Q[i]=_Q[i];
			}
			
			//Q[0]=_Q[0]; Q[1]=_Q[1]; _Q[2]=_Q[2];

		}
	}

	if(GJKPointOutsideOfPlane(Qt[0], Qt[2], Qt[3], Qt[1]))
	{
		HVec3 _Q[3]={Qt[0], Qt[2], Qt[3]};
		const HVec3 q=GJKIntersectClosestPtPointTriangle(_Q, _size);
		//const HVec3 t(q-p);
		//const HF32 sqDist(t|t);//Dot(q-p, q-p);
		const HF32 sqDist(q | q);
		if(sqDist.LT(bestSqDist))
		{
			bestSqDist=sqDist;
			closestPt=q;
			size=_size;
			for(U32 i=0; i<size; ++i)
			{
				Q[i]=_Q[i];
			}
			
			//Q[0]=_Q[0]; Q[1]=_Q[1]; _Q[2]=_Q[2];
		}
	}

	if(GJKPointOutsideOfPlane(Qt[0], Qt[3], Qt[1], Qt[2]))
	{
		HVec3 _Q[3]={Qt[0], Qt[3], Qt[1]};
		const HVec3 q=GJKIntersectClosestPtPointTriangle(_Q, _size);
		//const HVec3 t(q-p);
		//const HF32 sqDist(t|t);//(q-p)|(q-p);//Dot(q-p, q-p);
		const HF32 sqDist(q | q);
		if(sqDist.LT(bestSqDist))
		{
			bestSqDist=sqDist;
			closestPt=q;
			size=_size;
			for(U32 i=0; i<size; ++i)
			{
				Q[i]=_Q[i];
			}
			
			//Q[0]=_Q[0]; Q[1]=_Q[1]; _Q[2]=_Q[2];
		}
	}

	if(GJKPointOutsideOfPlane(Qt[1], Qt[3], Qt[2],Qt[0]))
	{
		HVec3 _Q[3]={Qt[1], Qt[3], Qt[2]};
		const HVec3 q=GJKIntersectClosestPtPointTriangle(_Q, _size);
		//const HVec3 t(q-p);
		//const HF32 sqDist(t|t);//Dot(q-p, q-p);
		const HF32 sqDist(q | q);
		if(sqDist.LT(bestSqDist)) 
		{
			bestSqDist=sqDist;
			closestPt=q;
			size=_size;
			for(U32 i=0; i<size; ++i)
			{
				Q[i]=_Q[i];
			}
			
			//Q[0]=_Q[0]; Q[1]=_Q[1]; _Q[2]=_Q[2];
		}
	}

	return closestPt;
}

//void GJKIntersectSupport(const ConvexHull& a, const ConvexHull& b, HVec3 PCREF d, HVec3& support)
//{
//	//support=a.Support(d)-b.Support(-d);
//	/*const HF32 aNum=a.NumVertex();
//	const HF32 bNum=b.NumVertex();
//	const Maths::HVec3 dneg(-d);
//	
//	Maths::HF32 extreamA(a[0]|d);
//	Maths::HVec3 supportA(a[0]);
//	for(U32 i=1; i< aNum; ++i)
//	{
//		const HF32 asup(a[i]|d);//Dot(a[i], d);
//		Maths::Scalar_Bool con(extreamA.LT(asup));
//		extreamA=Select(con, asup, extreamA);
//		supportA=Select(con, a[i], supportA);
//	}
//
//	Maths::HF32 extreamB(b[0]|dneg);
//	Maths::HVec3 supportB(b[0]);
//	for(U32 i=1; i< bNum; ++i)
//	{
//		const HF32 bsup(b[i]|dneg);//Dot(a[i], d);
//		Maths::Scalar_Bool con(extreamB.LT(bsup));
//		extreamB=Select(con, bsup, extreamB);
//		supportB=Select(con, b[i], supportB);
//	}
//
//	HVec3 support1=supportA-supportB;
//	*/
//
//	
//	//OBB *oba= (OBB*)&a;
//	//OBB *obb= (OBB*)&b;
//
//	//const HVec3 supA(Support(a, d));
//	//const HVec3 supB(Support(b, -d));
//
//	//support=supA - supB;
//	//assert(support1.All_EQ(support));
//
//
//}



Maths::HVec3 GJKIntersectDoSimplex(Maths::HVec3* Q, U32& size)
{
	switch(size)
	{
	case 1: return Q[0];
	case 2: return GJKIntersectClosestPtPointSegment( Q, size);
	case 3: return GJKIntersectClosestPtPointTriangle(Q, size);
	case 4: return GJKIntersectClosestPtPointTetrahedron(Q, size);
	default:
		BIZ_ASSERT(0 && "Undefined Simplex!");
		return Maths::Constants<Maths::HVec3>::Zero();//Maths::HVec3(0,0,0);
	}
}

//bool GJKIntersect(const ConvexHull& a, const ConvexHull& b, HF32& dis, SupportFun aSup, SupportFun bSup)
//{
//	U32 size=0;
//	HVec3 origin(Maths::Constants<HVec3>::Zero());				//(0.0f, 0.0f, 0.0f)
//	HVec3 Q[4];				//simplex set
//	Q[0]=a[0]-b[0];				//pick a random point int eh minkowski sum to start
//	//HVec3 d=-Q[0];//Q[0].Neg();		//direction
//	HVec3 support=Q[size++];	//if the size of Q is one, the support point is the point in the simplex 
//	HVec3 closestPt=support;
//	HVec3 d=-closestPt;  //direction
//	//these variables are designed for ealier exist
//	HF32 delta(ZERO_INIT);
//	HF32 mu(ZERO_INIT);
//	const HF32 epsilon(1e-3f);
//	dis=closestPt.Length();
//	
//	//OBB *oba= (OBB*)&a;
//	//OBB *obb= (OBB*)&b;
//
//	while(!closestPt.All_EQ(origin))//if the origin is the closest point, which means a and b intersect
//	{
//
//		//d=-closestPt;//closestPt.Neg();
//		//calculate the support point of the minkowski sum of a and d in direction of d
//		//GJKIntersectSupport(a, b, d, support);
//		//const HVec3 support(a.Support(d)-b.Support(-d));
//		const HVec3 support(aSup(a, d)-bSup(b, -d));
//		
//		if((support|d).LE(epsilon)) 
//			return false;// no vertex is closer to the origin in direction d than support point itself, no intersection
//
//		Q[size++]=support;
//		
//		//delta=(closestPt|support)/dis;//Dot(closestPt, support) / dis;
//
//		//if(mu.GT(delta))
//		//	mu=delta;
//		
//		//if( (dis-mu) <= GJK_EPSILON )
//		//	return true;// if the object is non polytope object, the gjk algorithm need to terminates as soon as possible
//
//		closestPt=GJKIntersectDoSimplex(Q, size);
//
//		dis=closestPt.Length();
//
//		d=-closestPt;
//		
//	}
//
//	return true;
//}

}
