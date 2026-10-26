#ifndef XENOCOLLIDE_H
#define XENOCOLLIDE_H

#include "gravity/narrowphase/Primitive.h"
#include "gravity/narrowphase/Primitive.inl"
#include "gravity/sharedphysics/BinaryHeap.h"


namespace Collision
{

	bool IsPointsOnTheDifferentSide(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF p1, const Maths::HVec3 PCREF p2, Maths::C_Normal& normal)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HVec3 ab(b-a);
		const Maths::HVec3 ac(c-a);
		const Maths::C_Normal n(ab^ac);
		const Maths::HVec3 v1(p1-a);
		const Maths::HVec3 v2(p2-a);
		const Maths::HF32 signDist1(n|v1);
		const Maths::HF32 signDist2(n|v2);


		const Maths::HF32 mul(signDist1*signDist2);

		const Maths::Scalar_Bool con(mul.LT(zero));
		
		const Maths::Scalar_Bool con1(signDist1.GE(zero));
		/*
			If signDist1>0, normal=n, vice verce
		*/
		normal=Maths::Select(con1, n, -n);
		return con;
	}

template <class ConvexA, class ConvexB>
bool XenoCollide(ConvexA& a, ConvexB& b, Maths::HVec3 & result, Maths::HVec3& nor)
{
	const Maths::HVec3 origin(Maths::Constants<Maths::HVec3>::Zero());
	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
	const Maths::HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon());

	const Maths::HVec3 centerMassA(a.Centre());
	const Maths::HVec3 centerMassB(b.Centre());
	const Maths::HVec3 v0(centerMassB - centerMassA);
	/*
		original Ray=origin - (centerMassB - centerMassA);
	*/
	const Maths::C_Normal d0(-v0);

	Maths::HVec3 aQ[4];
	Maths::HVec3 bQ[4];
	Maths::HF32 barycentric[3];

	aQ[0]=a.Support(-d0);
	bQ[0]=b.Support(d0);
	Maths::HVec3 v1(bQ[0] - aQ[0]);
	const Maths::C_Normal d1(v0^v1);
	aQ[1]=a.Support(-d1);
	bQ[1]=b.Support(d1);
	Maths::HVec3 v2(bQ[1] - aQ[1]);
	const Maths::HVec3 v2v0(v2 - v0);
	const Maths::HVec3 v1v0(v1 - v0);
	const Maths::C_Normal d2(v1v0^v2v0);
	aQ[2]=a.Support(-d2);
	bQ[2]=b.Support(d2);
	Maths::HVec3 v3(bQ[2] - aQ[2]);

	Maths::HVec3 contact;
	Maths::C_Normal normal;
	

	while(!Collision::IntersectTriangleRay(v1, v2, v3, v0, origin, contact))
	{
		
		if(IsPointsOnTheDifferentSide(v0, v1, v2, origin, v3, normal))
		{
			//v3=b.Support(normal) - a.Support(-normal);
			aQ[2]=a.Support(-normal);
			bQ[2]=b.Support(normal);
			v3=bQ[2]-aQ[2];
		}
		else if(IsPointsOnTheDifferentSide(v0, v2, v3, origin, v1, normal))
		{
			aQ[0]=a.Support(-normal);
			bQ[0]=b.Support(normal);
			v1=bQ[0]-aQ[0];

			//v1=b.Support(normal) - a.Support(-normal);	
		}
		else if(IsPointsOnTheDifferentSide(v0, v3, v1, origin, v2, normal))
		{
			aQ[1]=a.Support(-normal);
			bQ[1]=b.Support(normal);
			v2=bQ[1]-aQ[1];

			//v2=b.Support(normal) - a.Support(-normal);
		}
	}

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(origin, 0.2f, Maths::RGBA(0, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(origin, "o");

	/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(v1, v2, v3, RGBA(255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(v1, 0.2f, RGBA(0, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(v2, 0.2f, RGBA(0, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(v3, 0.2f, RGBA(0, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(v0, 0.2f, RGBA(0, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(v0, "v0");
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(v1, "v1");
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(v2, "v2");
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(v3, "v3");

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(v0, v1, RGBA(255, 0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(v0, v2, RGBA(255, 0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(v0, v3, RGBA(255, 0, 0, 255));*/
	

	int count=0;
	while(true)
	{
		if(!IsPointsOnTheDifferentSide(v1, v2, v3, origin, v0, normal))
		{
			//Maths::HF32 v(Maths::UNDEFINED);
			//Maths::HF32 w(Maths::UNDEFINED);
			////origin is inside the tetrahedron, both convex objects intersect with each other
			//
			//Collision::IntersectTriangleRay(v1, v2, v3, v0, origin, v, w);
			//barycentric[0]=one - v-w;
			//barycentric[1]=v;
			//barycentric[2]=w;
			//normal=-normal;

			//int count=0;
			//
			//while(true)
			//{
			//	aQ[3]=a.Support(-normal);
			//	bQ[3]=b.Support(normal);
			//	const Maths::HVec3 support(bQ[3] - aQ[3]);

			//	const Maths::HF32 signDist(normal|(contact - support));
			//	
			//	if(signDist.Abs().LT(Maths::Constants<Maths::HF32>::Epsilon2()))
			//		break;

			//	count++;
			//	if(count>10)
			//		break;

			//	if(Collision::IntersectTriangleRay(support, v1, v2, v0, origin, v, w))
			//	{
			//		/*Maths::HVec3 c;
			//		Collision::IntersectTriangleRay(support, v1, v2, v0, origin, c);
			//		const Maths::HF32 u(one -v-w);
			//		contact=support*u + v1*v + v2*w;

			//		assert(c.All_EQ(contact));*/
			//		aQ[2]=aQ[3];
			//		bQ[2]=bQ[3];
			//		v3=support;
			//		barycentric[0]=v;
			//		barycentric[1]=w;
			//		barycentric[2]=one - v -w;

			//	}
			//	else if(Collision::IntersectTriangleRay(support, v1, v3, v0, origin, v, w))
			//	{
			//		/*Maths::HVec3 c;
			//		Collision::IntersectTriangleRay(support, v1, v3, v0, origin, c);
			//		const Maths::HF32 u(one -v-w);
			//		contact=support*u + v1*v + v2*w;

			//		assert(c.All_EQ(contact));*/
			//		aQ[1]=aQ[3];
			//		aQ[2]=bQ[3];
			//		v2=support;
			//		barycentric[0]=v;
			//		barycentric[1]=one - v - w;
			//		barycentric[2]=w;
			//	}
			//	else if(Collision::IntersectTriangleRay(support, v3, v2, v0, origin, v, w))
			//	{
			//	/*	Maths::HVec3 c;
			//		Collision::IntersectTriangleRay(support, v3, v2, v0, origin, c);
			//		const Maths::HF32 u(one -v-w);
			//		contact=support*u + v1*v + v2*w;

			//		assert(c.All_EQ(contact));*/
			//		aQ[0]=aQ[3];
			//		bQ[0]=bQ[3];
			//		v1=support;
			//		barycentric[0]=one - v-w;
			//		barycentric[1]=w;
			//		barycentric[2]=v;
			//	}


			//	const Maths::HVec3 v1v2(v2 - v1);
			//	const Maths::HVec3 v1v3(v3 - v1);
			//	const Maths::C_Normal n(v1v2^v1v3);

			//	const Maths::HF32 dist(n|(-v1));

			//	const Maths::Scalar_Bool con1(dist.LT(zero));

			//	normal=Select(con1, n, -n);

			//	
			//}

			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(v0, origin, RGBA(155, 155, 155, 255));
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(v1, normal, 2.f, 0.2f, RGBA(255, 0,0, 0));
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(v1, v2, v3, RGBA(255, 0, 255));
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(v1, 0.2f, RGBA(155+count, 255, 0, 255));
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(v2, 0.2f, RGBA(155+count, 255, 0, 255));
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(v3, 0.2f, RGBA(155+count, 255, 0, 255));
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(v0, 0.2f, RGBA(155+count, 255, 0, 255));
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(v0, "v0");
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(v1, "v1");
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(v2, "v2");
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(v3, "v3");

			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(v0, v1, RGBA(155+count, 0, 255, 255));
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(v0, v2, RGBA(155+count, 0, 255, 255));
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(v0, v3, RGBA(155+count, 0, 255, 255));

			//
			//contact=aQ[0]*barycentric[0] + aQ[1]*barycentric[1] + aQ[2]*barycentric[2];

			//const Maths::HVec3 contact2=bQ[0]*barycentric[0] + bQ[1]*barycentric[1] + bQ[2]*barycentric[2];
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.5f, RGBA(255, 255, 0, 255));
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact2, 0.5f, RGBA(255, 0, 0, 255));

			return true;
		}

		const Maths::HVec3 v4(b.Support(normal) - a.Support(-normal));
		


		/*if(count == 30)
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(v4, normal, 3.f, 0.2f, RGBA(0, 255,0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(v4, 0.2f, RGBA(0, 255, 255, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(v4, "v4");

		}*/

		const Maths::HF32 signDist(normal|(-v4));

		//if the origin lies outside of the new support plane formed by the support normal and the
		//new support point, the origin lies outside the minkowski difference, therefore, a and b are
		//non-overlap
		if(signDist.GT(zero))
			return false;

		//const Maths::HF32 signDist2(normal|(-v1));
		//assert(signDist2.GT(zero));

	
		if(Collision::IntersectTriangleRay(v4, v1, v2, v0, origin, contact))
		{
			v3=v4;
		}
		else if(Collision::IntersectTriangleRay(v4, v1, v3, v0, origin, contact))
		{
			v2=v4;
		}
		else if(Collision::IntersectTriangleRay(v4, v3, v2, v0, origin, contact))
		{
			v1=v4;
		}



	
		const Maths::HVec3 v1v2(v2 - v1);
		const Maths::HVec3 v1v3(v3 - v1);
		const Maths::C_Normal n(v1v2^v1v3);
		const Maths::HF32 dist(n|(-v1));

		const Maths::Scalar_Bool con1(dist.GT(zero));
		/*
			If dist>0, normal=n, vice verce
		*/
		normal=Maths::Select(con1, n, -n);


		const Maths::HVec3 sPoint=b.Support(normal) - a.Support(-normal);
		const Maths::HF32 signDist0(normal|(sPoint - v1));


		if(signDist0.LT(Maths::Constants<Maths::HF32>::Epsilon2()))
			return false;

		count++;

		

		if(count > 10)
		{
			
			return true;
		}
	}





}

#ifndef MaxFacets
#define MaxFacets 128
#endif
#ifndef MaxSupportPoints
#define MaxSupportPoints 128
#endif

class XenoFacet
{
	public:

	inline XenoFacet(): m_closest( Maths::UNDEFINED )
	{
	}

	inline XenoFacet(const U32 _i0, const U32 _i1, const U32 _i2)
		: m_closest( Maths::UNDEFINED )
	{
		m_indices[0]=_i0;
		m_indices[1]=_i1;
		m_indices[2]=_i2;

	}

	NOALIAS inline XenoFacet& operator = (const XenoFacet& other)
	{
		m_closest = other.m_closest;
		m_lambda1 = other.m_lambda1;			//barycentric coordinate
		m_lambda2 = other.m_lambda2;
		m_det = other.m_det;			//determinate
		m_dist = other.m_dist;		//the distance of v to the origin
		m_indices[0] = other.m_indices[0];			//the index of vertices of the triangle
		m_indices[1] = other.m_indices[1];			//the index of vertices of the triangle
		m_indices[2] = other.m_indices[2];			//the index of vertices of the triangle

		return *this;
	}

	inline U32 operator[](const U32 i) const 
	{ 
		return m_indices[i]; 
	} 

	inline const Maths::HVec3& getClosest() const 
	{ 
		return m_closest; 
	} 

	inline void isValid(const U32 i0, const U32 i1, const U32 i2, const Maths::HVec3* __restrict a_verts, const Maths::HVec3* __restrict  b_verts,
		Maths::HF32& sDist, S32& b1, Maths::Scalar_Bool& b2)
	{
		
		using namespace Maths;
		
		HF32 zero(-Maths::Constants<Maths::HF32>::Epsilon());
		const HVec3 p0(a_verts[i0]-b_verts[i0]);
		const HVec3 v1((a_verts[i1]-b_verts[i1]) - p0);
		const HVec3 v2((a_verts[i2]-b_verts[i2]) - p0);

		const HF32 v1dv1 = v1 | v1;
		const HF32 v1dv2 = v1 | v2;
		const HF32 v2dv2 = v2 | v2;
		const HF32 p0dv1 = p0 | v1; 
		const HF32 p0dv2 = p0 | v2;
	    
		const HF32 det( v1dv1 * v2dv2 - v1dv2 * v1dv2 ); // non-negative
		const HF32 lambda1(p0dv2 * v1dv2 - p0dv1 * v2dv2);
		const HF32 lambda2(p0dv1 * v1dv2 - p0dv2 * v1dv1);
		const HVec3 closest(p0 + (lambda1 * v1 + lambda2 * v2) / det);

		Store( det, &m_det );
		Store(lambda1, &m_lambda1);
		Store(lambda2, &m_lambda2);
		m_closest=closest;
		sDist=closest | closest;
		Store( sDist, &m_dist);
		
		b1= det >= zero;
		b2=((lambda1.GT(zero)) & (lambda2.GT(zero)) & ((lambda1+lambda2).LE(det)));
		//return ((lambda1.GT(zero)) & (lambda2.GT(zero)) & ((lambda1+lambda2).LE(det)));
		//return (m_lambda1 > 0.f) & (m_lambda2 > 0.f) & ((m_lambda1+m_lambda2) <= m_det);
	}

	inline Maths::HF32 getDist() const 
	{ 
		return Maths::Load<Maths::HF32>( &m_dist ); 
	}
	
	inline void getClosestPoint(const Maths::HVec3 *aBuf, const Maths::HVec3 *bBuf, Maths::HVec3& closestA, Maths::HVec3& closestB) const 
	{
		using namespace Maths;

		const HVec3& pa = aBuf[m_indices[0]];
		const HVec3& pb = bBuf[m_indices[0]];
		const HF32 lambda1(Load<Maths::HF32>( &m_lambda1 ));
		const HF32 lambda2(Load<Maths::HF32>( &m_lambda2 ));
		const HF32 det(Load<Maths::HF32>( &m_det ));

		closestA=pa + (lambda1 * (aBuf[m_indices[1]] - pa) + lambda2  * (aBuf[m_indices[2]] - pa)) / det ;
		closestB=pb + (lambda1 * (bBuf[m_indices[1]] - pb) + lambda2  * (bBuf[m_indices[2]] - pb)) / det ;
    }


	inline bool operator <(const XenoFacet& b) const
	{
		return m_dist < b.m_dist;
	}

	inline bool operator > (const XenoFacet& b) const
	{
		return m_dist > b.m_dist;
	}

	inline bool operator <=(const XenoFacet& b) const
	{
		return m_dist <= b.m_dist;
	}


private:
	
	Maths::HVec3 m_closest;		//closest point to the origin
	F32 m_lambda1;			//barycentric coordinate
	F32 m_lambda2;
	F32 m_det;			//determinate
	F32 m_dist;		//the distance of v to the origin
	S32 m_indices[3];			//the index of vertices of the triangle

};

class Xeno
{
public:

	Xeno(): num_verts(0), num_facets(0), freeFacet(0), heap(MaxFacets)
	{
	}

	template <class ConvexA, class ConvexB>
	bool XenoCollide(ConvexA& a, ConvexB& b, Maths::HVec3 & pa, Maths::HVec3& pb);

	//bool ConstructTetrahedron(const ConvexA& a, const ConvexB& b, const Maths::HF32 PCREF marginA, const Maths::HF32 PCREF marginB);

	//template <class ConvexA, class ConvexB>
	//bool PenetrationDepth(const ConvexA& a, const ConvexB& b, const Maths::HVec3* __restrict Q, const Maths::HVec3* __restrict A, const Maths::HVec3* __restrict B, const U8 PCREF size,  Maths::HVec3& pa, Maths::HVec3& pb, bool fail);
	
	inline XenoFacet* addXenoFacet(U32 i0, U32 i1, U32 i2, const Maths::HF32 PCREF lower2, const Maths::HF32 PCREF upper2);

	inline bool originInTetrahedron(const Maths::HVec3 PCREF p1, const Maths::HVec3 PCREF p2, const Maths::HVec3 PCREF p3, const Maths::HVec3 PCREF p4);
	int num_verts;
	int	num_facets;
	int freeFacet;

	Maths::HVec3  aBuf[MaxSupportPoints];
	Maths::HVec3  bBuf[MaxSupportPoints];

	XenoFacet facetBuf[MaxFacets];
	SharedPhysics::BinaryHeap<XenoFacet> heap;
};

inline XenoFacet* Xeno::addXenoFacet(U32 i0, U32 i1, U32 i2, const Maths::HF32 PCREF lower2, const Maths::HF32 PCREF upper2)
{
	using namespace Maths;

	assert(i0 != i1 && i0 != i2 && i1 != i2);
    if (freeFacet < MaxFacets)
	{
		XenoFacet *facet = new(&facetBuf[freeFacet])XenoFacet(i0, i1, i2);

		HF32 sDist(Maths::UNDEFINED);
		S32 b1;
		Scalar_Bool b2(Maths::UNDEFINED);
		facet->isValid(i0, i1, i2, aBuf, bBuf, sDist, b1, b2);
		if(b2 & lower2.LE(sDist) & sDist.LE(upper2))
		{
			heap.insert(*facet);
			++num_facets;
		}
		freeFacet+=b1;
		return SharedPhysics::Fast_Select<XenoFacet>(b1 != 0, facet, (XenoFacet*)0);
    }
    return 0;
}

inline bool Xeno::originInTetrahedron(const Maths::HVec3 PCREF p1, const Maths::HVec3 PCREF p2, 
								const Maths::HVec3 PCREF p3, const Maths::HVec3 PCREF p4)
{

	using namespace Maths;
	//const HF32 epsilon(Maths::Constants<HF32>::Epsilon());
	const HF32 zero(Maths::Constants<HF32>::Zero());
    HVec3 normal1((p2 - p1)^(p3 - p1));
    HVec3 normal2((p3 - p2)^(p4 - p2));
    HVec3 normal3((p4 - p3)^(p1 - p3));
    HVec3 normal4((p1 - p4)^(p2 - p4));

	const HF32 n1p1(normal1|p1);
	const HF32 n1p4(normal1|p4);
	const HF32 n2p2(normal2|p2);
	const HF32 n2p1(normal2|p1);
	const HF32 n3p3(normal3|p3);
	const HF32 n3p2(normal3|p2);
	const HF32 n4p4(normal4|p4);
	const HF32 n4p3(normal4|p3);

    
	return (n1p1 * n1p4).LE(zero) & 
		(n2p2 * n2p1).LE(zero) & 
		(n3p3 * n3p2).LE(zero) & 
		(n4p4 * n4p3).LE(zero);
    /*return	(((n1p1.GT(zero)) ^ (n1p4.GT(zero))) | ((n1p1.GT(epsilon)) ^ (n1p4.GT(epsilon))))&
			(((n2p2.GT(zero)) ^ (n2p1.GT(zero))) | ((n2p2.GT(epsilon)) ^ (n2p1.GT(epsilon)))) &
			(((n3p3.GT(zero)) ^ (n3p2.GT(zero))) | ((n3p3.GT(epsilon)) ^ (n3p2.GT(epsilon))))&
			(((n4p4.GT(zero)) ^ (n4p3.GT(zero))) | ((n4p4.GT(epsilon)) ^ (n4p3.GT(epsilon))));*/
}






template <class ConvexA, class ConvexB>
bool Xeno::XenoCollide( ConvexA& a, ConvexB& b, Maths::HVec3 & pa, Maths::HVec3& pb)
{
	using namespace Maths;

	const HVec3 origin(Maths::Constants<Maths::HVec3>::Zero());
	const HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const HF32 one(Maths::Constants<Maths::HF32>::One());
	const HF32 _max(Maths::Constants<HF32>::Max());
	const HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon());

	const HVec3 centerMassA(a.Centre());
	const HVec3 centerMassB(b.Centre());
	const HVec3 v0(centerMassB - centerMassA);
	/*
		original Ray=origin - (centerMassB - centerMassA);
	*/
	const C_Normal d0(-v0);


	aBuf[0]=centerMassA;
	bBuf[0]=centerMassB;

	aBuf[1]=a.Support(-d0);
	bBuf[1]=b.Support(d0);
	Maths::HVec3 v1(bBuf[1] - aBuf[1]);
	const Maths::C_Normal d1(v0^v1);
	aBuf[2]=a.Support(-d1);
	bBuf[2]=b.Support(d1);
	Maths::HVec3 v2(bBuf[2] - aBuf[2]);
	const Maths::HVec3 v2v0(v2 - v0);
	const Maths::HVec3 v1v0(v1 - v0);
	const Maths::C_Normal d2(v1v0^v2v0);
	aBuf[3]=a.Support(-d2);
	bBuf[3]=b.Support(d2);
	Maths::HVec3 v3(bBuf[3] - aBuf[3]);

	Maths::HVec3 q0(v0);
	Maths::HVec3 q1(v1);
	Maths::HVec3 q2(v2);
	Maths::HVec3 q3(v3);

	Maths::HVec3 contact;
	Maths::C_Normal normal;
	

	while(!Collision::IntersectTriangleRay(v1, v2, v3, v0, origin, contact))
	{
		
		if(IsPointsOnTheDifferentSide(v0, v1, v2, origin, v3, normal))
		{
			//v3=b.Support(normal) - a.Support(-normal);
			aBuf[3]=a.Support(-normal);
			bBuf[3]=b.Support(normal);
			v3=bBuf[3]-aBuf[3];
		}
		else if(IsPointsOnTheDifferentSide(v0, v2, v3, origin, v1, normal))
		{
			aBuf[1]=a.Support(-normal);
			bBuf[1]=b.Support(normal);
			v1=bBuf[1]-aBuf[1];

			//v1=b.Support(normal) - a.Support(-normal);	
		}
		else if(IsPointsOnTheDifferentSide(v0, v3, v1, origin, v2, normal))
		{
			aBuf[2]=a.Support(-normal);
			bBuf[2]=b.Support(normal);
			v2=bBuf[2]-aBuf[2];

			//v2=b.Support(normal) - a.Support(-normal);
		}
	}

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(origin, 0.2f, RGBA(0, 255, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(origin, "o");

	

	int count=0;
	while(true)
	{
		if(!IsPointsOnTheDifferentSide(v1, v2, v3, origin, v0, normal))
		{
			
			const Maths::HF32 dist(normal|(-v1));
			Maths::HF32 u, v, w;
			Collision::ClosestPtPointTriangle(origin, v1, v2, v3, u, v, w);

			if(dist.LT(0.02f))
			{
				const Maths::HVec3 pa(aBuf[1]*u + aBuf[2]*v + aBuf[3]*w);
				const Maths::HVec3 pb(bBuf[1]*u + bBuf[2]*v + bBuf[3]*w);
				const Maths::HVec3 closest(v1*u + v2*v + v3*w);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closest, 0.5f, RGBA(255, 0, 0, 255));
				const Maths::HF32 penetration(dist);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(pa, 0.5f, RGBA(255, 0, 0, 255));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(pb, 0.5f, RGBA(0, 255, 0, 255));
				return true;
			}

			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closest, 0.5f, RGBA(255, 0, 0, 255));

		/*	normal=-normal;

			aBuf[4]=a.Support(-normal);
			bBuf[4]=b.Support(normal);
			const Maths::HVec3 v4(bBuf[4] - aBuf[4]);

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(v4, 0.2f, RGBA(255, 0, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(v4, "v4");*/

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(v1, normal, 3.f, 0.2f, RGBA(0, 255, 0, 255));

			

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(v1, v2, v3, RGBA(255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(v1, 0.2f, RGBA(0, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(v2, 0.2f, RGBA(0, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(v3, 0.2f, RGBA(0, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(v0, 0.2f, RGBA(0, 255, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(v0, "v0");
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(v1, "v1");
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(v2, "v2");
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(v3, "v3");

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(v0, v1, RGBA(255, 0, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(v0, v2, RGBA(255, 0, 0, 255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(v0, v3, RGBA(255, 0, 0, 255));
	
	
			heap.makeEmpty();
			XenoFacet* f0=addXenoFacet(0, 1, 2, zero, _max);
			XenoFacet* f1=addXenoFacet(0, 3, 1, zero, _max);
			XenoFacet* f2=addXenoFacet(0, 2, 3, zero, _max);
			XenoFacet* f3=addXenoFacet(1, 3, 2, zero, _max);
			/*XenoFacet* f4=addXenoFacet(4, 2, 1, zero, max);
			XenoFacet* f5=addXenoFacet(4, 3, 2, zero, max);
			XenoFacet* f6=addXenoFacet(4, 1, 3, zero, max);*/
	
			num_verts=4;

			if(heap.isEmpty())
				return false;

			XenoFacet f;
			XenoFacet* facet = &f;

			
			HF32 upper_bound2(_max);
			const HF32 ee(one + epsilon);
			const HF32 e(ee * ee);

			do 
			{
				heap.deleteTop(f); //get the shortest distance triangle of origin from the list

				assert(facet->getDist() > zero);
				
				if (num_verts == MaxSupportPoints)	
					break;

				const HVec3 closest(facet->getClosest());

				const HVec3 tempa(a.Support(closest));
				const HVec3 tempb(b.Support(-closest));

				aBuf[num_verts]=tempa;
				bBuf[num_verts]=tempb;
				const HVec3 q(tempa - tempb);
				
				U32 index =num_verts++;
				
				//if the support point and closest point close enough, found the contact point, break
				//HF32 dist(qBuf[index]|closest);
				HF32 sDist(q|closest);
				//assert(sDist > zero);
				HF32 sfDist(facet->getDist());
			
				const HF32 er(e * sfDist);
				upper_bound2=Min(upper_bound2, (sDist * sDist) / sfDist);
				if(upper_bound2 <= er )
				{
					
					break;
				}

				addXenoFacet((*facet)[0], (*facet)[1], index, sfDist, upper_bound2);
				addXenoFacet((*facet)[1], (*facet)[2], index, sfDist, upper_bound2);
				addXenoFacet((*facet)[2], index, (*facet)[0], sfDist, upper_bound2);

			}
			while ((--num_facets) && U32(heap.getTop().getDist() <= upper_bound2));


			facet->getClosestPoint(aBuf, bBuf, pa, pb);

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(pa, 0.5f, RGBA(255, 0, 0, 255));
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(pb, 0.5f, RGBA(0, 255, 0, 255));
			return true;
		}

		/*aBuf[4]=a.Support(-normal);
		bBuf[4]=b.Support(normal);
		const Maths::HVec3 v4(bBuf[4] - aBuf[4]);*/

		const Maths::HVec3 v4(b.Support(normal) - a.Support(-normal));

		const Maths::HF32 signDist(normal|(-v4));

		//if the origin lies outside of the new support plane formed by the support normal and the
		//new support point, the origin lies outside the minkowski difference, therefore, a and b are
		//non-overlap
		if(signDist.GT(zero))
			return false;

		//const Maths::HF32 signDist2(normal|(-v1));
		//assert(signDist2.GT(zero));

	
		if(Collision::IntersectTriangleRay(v4, v1, v2, v0, origin, contact))
		{
			v3=v4;
		/*	aBuf[3]=aBuf[4];
			bBuf[3]=bBuf[4];*/
		}
		else if(Collision::IntersectTriangleRay(v4, v1, v3, v0, origin, contact))
		{
			v2=v4;
			/*aBuf[2]=aBuf[4];
			bBuf[2]=bBuf[4];*/
		}
		else if(Collision::IntersectTriangleRay(v4, v3, v2, v0, origin, contact))
		{
			v1=v4;
		/*	aBuf[1]=aBuf[4];
			bBuf[1]=aBuf[4];*/
		}
		const Maths::HVec3 v1v2(v2 - v1);
		const Maths::HVec3 v1v3(v3 - v1);
		const Maths::C_Normal n(v1v2^v1v3);
		const Maths::HF32 dist(n|(-v1));

		const Maths::Scalar_Bool con1(dist.GT(zero));
		/*
			If dist>0, normal=n, vice verce
		*/
		normal=Maths::Select(con1, n, -n);


		const Maths::HVec3 sPoint=b.Support(normal) - a.Support(-normal);
		const Maths::HF32 signDist0(normal|(sPoint - v1));


		if(signDist0.LT(Maths::Constants<Maths::HF32>::Epsilon2()))
			return false;

		count++;

		

		if(count > 10)
		{
			
			return true;
		}
	}
}

};


#endif
