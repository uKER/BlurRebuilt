 #ifndef GJK_H
#define GJK_H

#include "gravity/narrowphase/EPA.h"

namespace Collision
{

NOALIAS __forceinline  bool GJKPointOutsideOfPlane(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF  b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF d)
{
	using namespace Maths;
	const HF32 zero(Constants<HF32>::Zero());
	const HVec3 temp=(b-a)^(c-a);
	const HF32 signp=(-a) |temp;//Dot(origin-a, Cross(b-a, c-a));
	const HF32 signd=(d-a)|temp;//Dot(d-a, Cross(b-a, c-a));
	return (signp*signd).LT(zero);
};

Maths::HVec3 GJKIntersectClosestPtPointSegment(Maths::HVec3* Q, U32& size);
Maths::HVec3 GJKIntersectClosestPtPointTriangle(Maths::HVec3* Q, U32& size);
Maths::HVec3 GJKIntersectClosestPtPointTetrahedron(Maths::HVec3* Q, U32& size);



Maths::HVec3 GJKIntersectDoSimplex(Maths::HVec3* Q, U32& size);


NOALIAS inline bool GJKCPairClosestPtPointSegment(Maths::HVec3* __restrict Q, U32& size, Maths::HVec3* __restrict A, Maths::HVec3* __restrict B, Maths::HVec3& closestA, Maths::HVec3& closestB);

NOALIAS inline bool GJKCPairClosestPtPointSegment(Maths::HVec3* __restrict Q, Maths::HVec3* __restrict Q2, U32& size, Maths::HVec3* __restrict A, Maths::HVec3* __restrict A2, Maths::HVec3* __restrict B, Maths::HVec3* __restrict B2, Maths::HVec3& closestA, Maths::HVec3& closestB);

NOALIAS inline bool GJKCPairClosestPtPointTriangle(Maths::HVec3* __restrict Q, U32& size, Maths::HVec3* __restrict A, Maths::HVec3* __restrict B, Maths::HVec3& closestA, Maths::HVec3& closestB);

NOALIAS inline bool GJKCPairClosestPtPointTriangle(Maths::HVec3* __restrict Q, Maths::HVec3* __restrict Q2, U32& size, Maths::HVec3* __restrict A, Maths::HVec3* __restrict A2, Maths::HVec3* __restrict B, Maths::HVec3* __restrict B2, Maths::HVec3& closestA, Maths::HVec3& closestB);


//void GJKCPairClosestPtPointTetrahedron(Maths::HVec3* __restrict Q, U32& size, Maths::HVec3* __restrict A, Maths::HVec3* __restrict B, Maths::HVec3& closestA, Maths::HVec3& closestB);

NOALIAS  inline bool GJKCPairClosestPtPointTetrahedron(Maths::HVec3* __restrict Q, U32& size, Maths::HVec3* __restrict A, Maths::HVec3* __restrict B, Maths::HVec3& closestA, Maths::HVec3& closestB);

NOALIAS  inline bool GJKCPairClosestPtPointTetrahedron(Maths::HVec3* __restrict Q, Maths::HVec3* __restrict Q2, U32& size, Maths::HVec3* __restrict A, Maths::HVec3* __restrict A2, Maths::HVec3* __restrict B, Maths::HVec3* __restrict B2, Maths::HVec3& closestA, Maths::HVec3& closestB);


NOALIAS inline bool GJKCPairClosestPtPointSegment(const Maths::HVec3 PCREF origin, Maths::HVec3* __restrict Q, U32& size, Maths::HVec3* __restrict A, Maths::HVec3* __restrict B, Maths::HVec3& closestA, Maths::HVec3& closestB);
NOALIAS inline bool GJKCPairClosestPtPointTriangle(const Maths::HVec3 PCREF origin, Maths::HVec3* __restrict Q, U32& size, Maths::HVec3* __restrict A, Maths::HVec3* __restrict B, Maths::HVec3& closestA, Maths::HVec3& closestB);
NOALIAS  inline bool GJKCPairClosestPtPointTetrahedron(const Maths::HVec3 PCREF origin, Maths::HVec3* __restrict Q, U32& size, Maths::HVec3* __restrict A, Maths::HVec3* __restrict B, Maths::HVec3& closestA, Maths::HVec3& closestB);
bool GJKCPairDoSimplex(const Maths::HVec3 PCREF origin, Maths::HVec3* __restrict Q, Maths::HVec3* __restrict A, Maths::HVec3* __restrict B, U32& size, Maths::HVec3& closestA,Maths::HVec3& closestB);

bool GJKCPairDoSimplex(Maths::HVec3* __restrict Q, Maths::HVec3* __restrict A, Maths::HVec3* __restrict B, U32& size, Maths::HVec3& closestA,Maths::HVec3& closestB);

bool GJKCPairDoSimplex(Maths::HVec3* __restrict Q, Maths::HVec3* __restrict A, Maths::HVec3* __restrict B, Maths::HVec3* __restrict Q2, Maths::HVec3* __restrict A2, Maths::HVec3* __restrict B2, U32& size, Maths::HVec3& closestA,Maths::HVec3& closestB);

//Maths::HVec3 GJKCPairDoSimplex1(Maths::HVec3* __restrict Q, Maths::HVec3* __restrict A, Maths::HVec3* __restrict B, U32& size, Maths::HVec3& closestA,Maths::HVec3& closestB);


template <class ConvexA, class ConvexB>
bool GJKCPair(ConvexA& a, ConvexB& b);

template <class ConvexA, class ConvexB>
bool GJKCPair(ConvexA& a, ConvexB& b, Maths::HF32& dis, Maths::HVec3& closestA, Maths::HVec3& closestB);

//This one get out the penetration depth but without margin. The implementation is in GJK
template <class ConvexA, class ConvexB>
bool GJKCPair( ConvexA& a, ConvexB& b, Maths::HVec3& contact, Maths::HVec3& normal, Maths::HF32& penetration, EPA& epa);

template <class ConvexA, class ConvexB>
bool GJKCPairMargin(ConvexA& a, ConvexB& b, Maths::HVec3 & contact, Maths::HVec3& norm, Maths::HF32& penetration, EPA& epa);



template <class ConvexA, class ConvexB>
bool GJKRayCast(ConvexA& a, ConvexB& b, const Maths::HVec3 PCREF r, const Maths::HF32 PCREF maxLambda, Maths::HF32& lambda,  Maths::HVec3& norm, Maths::HVec3& closestA, Maths::HVec3& closestB);



template <class ConvexA, class ConvexB>
bool GJKRayCastPenteraction(const ConvexA& a, const ConvexB& b, const Maths::HVec3 PCREF r, const Maths::HF32 PCREF maxLambda, Maths::HF32& lambda, Maths::HVec3& normal,  
				 Maths::HVec3& closestA, Maths::HVec3& closestB, EPA& epa);

#ifdef PLATFORM_SPU

bool _GJKCPair(C_Convex* a, C_Convex* b);
bool _GJKCPair(C_Convex* a, C_Convex* b, Maths::HF32& dis, Maths::HVec3& closestA, Maths::HVec3& closestB);

//This one get out the penetration depth but without margin. The impleementation is in GJK
bool _GJKCPair(C_Convex* a, C_Convex* b, Maths::HVec3& contact, Maths::HVec3& normal, Maths::HF32& penetration, EPA& epa);

bool _GJKCPairMargin(C_Convex* a, C_Convex* b, Maths::HVec3 & contact, Maths::HVec3& norm, Maths::HF32& penetration, EPA& epa);

bool _GJKRayCast(C_Convex*a, C_Convex* b, const Maths::HVec3 PCREF r, const Maths::HF32 PCREF maxLambda, Maths::HF32& lambda, Maths::HVec3& norm, Maths::HVec3& closestA, Maths::HVec3& closestB);

#endif

};


#endif

