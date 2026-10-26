#ifndef __GRAVITY_CYLINDER_TRIANGLE_INL__
#define __GRAVITY_CYLINDER_TRIANGLE_INL__

#include "TriangleTriangle.inl"
#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/GJK.inl"
namespace Collision
{

/*
	Suppose cylinder and triangle intersect already
*/
template < int MaxContacts>
bool CollideCylinderTriangle2(const C_Cylinder& cylinder, const Maths::HVec3 PCREF _a, const Maths::HVec3 PCREF _b, const Maths::HVec3 PCREF _c, C_ContactPatch< MaxContacts>& patch,
							 const Maths::HMat33 PCREF orientation)
{
	const Maths::HVec3 _center(cylinder.Centre());
	const Maths::HVec3 center;
	const Maths::HVec3 a(orientation * (_a - _center));
	const Maths::HVec3 b(orientation * (_b - _center));
	const Maths::HVec3 c(orientation * (_c - _center));
	const Maths::HVec3 ab(b-a);
	const Maths::HVec3 ac(c-a);
	const Maths::C_Normal normal(ab^ac);
	const Maths::HF32 d(normal|a); //the plane sign distance

	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(_a, _b, _c, RGBA(0, 255, 0, 255));
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(_a, _c, _b, RGBA(0, 255, 0, 255));
	
	const Maths::C_Normal cAxis(cylinder.LocalCentreAxis());


	//const Maths::HVec3 temp();
	const Maths::C_Normal dir(normal^cAxis); //construct a plane which include the center of cylinder and perpendicular to the triangle's normal
	//The triangle plane and the retangle plane intersect into one line

	const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
	/*if(temp.All_EQ(zero))
	{
		printf("This is the problem");
	}*/
	//if((temp | temp) < Maths::Constants<Maths::HF32>::Epsilon())
	//{
	//	printf("this is a problem");
	//}
	

	const Maths::HF32 t(normal|cAxis);
	const Maths::C_Normal w(normal - cAxis.m_V*t); //the component of normal orthogonal to cAxis

	const Maths::HVec3 dir1(cAxis.m_V * cylinder.m_halfHeight);
	const Maths::HVec3 dir2(-dir1);


	const Maths::HVec3 comp1(w*cylinder.m_Radius);
	const Maths::HVec3 comp2(-comp1);

	//if(comp1.All_EQ(zero))
	//{
	//	printf("This is the problem");
	//}

	//construct a rectange
	const Maths::HVec3 point1(center + dir1 + comp1);
	const Maths::HVec3 point2(center + dir1 + comp2);
	const Maths::HVec3 point3(center + dir2 + comp2);
	const Maths::HVec3 point4(center + dir2 + comp1);

	const Maths::HF32 dist1((normal|point1) - d);
	const Maths::HF32 dist2((normal|point2) - d);
	const Maths::HF32 dist3((normal|point3) - d);
	const Maths::HF32 dist4((normal|point4) - d);

	const Maths::HF32 zerof(Maths::Constants<Maths::HF32>::Zero());
	Maths::Scalar_Bool con1(dist1.GT(zerof) & dist2.GT(zerof) & dist3.GT(zerof) & dist4.GT(zerof));
	Maths::Scalar_Bool con2(dist1.LT(zerof) & dist2.LT(zerof) & dist3.LT(zerof) & dist4.LT(zerof));

	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point1, 0.05f, RGBA(0, 255, 0, 255));
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point2, 0.05f, RGBA(0, 255, 0, 255));
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point3, 0.05f, RGBA(0, 255, 0, 255));
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point4, 0.05f, RGBA(0, 255, 0, 255));
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(point1, "1");
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(point2, "2");
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(point3, "3");
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(point4, "4");
	

	if(con1|con2)
		return false; //no intersection

	const U32 numContacts = patch.m_NumContactPoints;

	bool collide1=Collision::OrenCollideTriangleTriangle(a, b, c, point1, point2, point3, normal, dir, patch, 0, 0);
	bool collide2=Collision::OrenCollideTriangleTriangle(a, b, c, point1, point3, point4, normal, dir, patch, 0, 0);
	bool collide3=Collision::OrenCollideTriangleTriangle(a, b, c, point2, point4, point1, normal, dir, patch, 0, 0);
	bool collide4=Collision::OrenCollideTriangleTriangle(a, b, c, point2, point3, point4, normal, dir, patch, 0, 0);

	for(U32 a = numContacts; a < patch.m_NumContactPoints; ++a)
	{
		patch.m_ContactPoints[a].m_ContactPoint = (patch.m_ContactPoints[a].m_ContactPoint * orientation) + _center;
	}

	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(point2, point3, point4, RGBA(0, 0, 255, 255));


	//return (collide1|collide2|collide3|collide4);
	return patch.m_NumContactPoints > numContacts;
}

template < int MaxContacts>
bool CollideCylinderTriangle(const C_Cylinder& cylinder, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, C_ContactPatch< MaxContacts>& patch, const U32 hashValue, const U32 physicsMaterial, const U32 effectMaterial)
{
	using namespace Maths;
	const Maths::HVec3 ab(b-a);
	const Maths::HVec3 ac(c-a);
	const Maths::C_Normal normal(ab^ac);
	const Maths::HF32 d(normal|a); //the plane sign distance

	const Maths::HVec3 center(cylinder.Centre());
	const Maths::C_Normal cAxis(cylinder.CentreAxis(), Maths::C_Normal::UNIT_LENGTH_VECTOR);

	const Maths::C_Normal dir(Normalise(normal^cAxis)); //construct a plane which include the center of cylinder and perpendicular to the triangle's normal
	//The triangle plane and the retangle plane intersect into one line

	const Maths::HF32 t(normal|cAxis);
	const Maths::C_Normal w(Normalise(C_Vector(normal) - cAxis*t)); //the component of normal orthogonal to cAxis

	const Maths::HVec3 dir1(cAxis * cylinder.HalfHeight());
	const Maths::HVec3 comp1(w*cylinder.Radius());
	

	//construct a rectange
	const Maths::HVec3 point1(center + dir1 + comp1);
	const Maths::HVec3 point2(center + dir1 - comp1);
	const Maths::HVec3 point3(center - dir1 - comp1);
	const Maths::HVec3 point4(center - dir1 + comp1);


	const Maths::HF32 dist1((normal|point1) - d);
	const Maths::HF32 dist2((normal|point2) - d);
	const Maths::HF32 dist3((normal|point3) - d);
	const Maths::HF32 dist4((normal|point4) - d);

	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	Maths::Scalar_Bool con1(dist1.GT(zero) & dist2.GT(zero) & dist3.GT(zero) & dist4.GT(zero));
	Maths::Scalar_Bool con2(dist1.LT(zero) & dist2.LT(zero) & dist3.LT(zero) & dist4.LT(zero));

	if(con1|con2)
		return false; //no intersection

#if 0 //ndef PLATFORM_SPU
	bool collide1=Collision::OrenCollideTriangleTriangle(a, b, c, point1, point2, point3, normal, dir, patch, hashValue, physicsMaterial, effectMaterial);
	bool collide2=Collision::OrenCollideTriangleTriangle(a, b, c, point1, point3, point4, normal, dir, patch, hashValue << 1, physicsMaterial, effectMaterial);
	bool collide3=Collision::OrenCollideTriangleTriangle(a, b, c, point2, point4, point1, normal, dir, patch, hashValue << 2, physicsMaterial, effectMaterial);
	bool collide4=Collision::OrenCollideTriangleTriangle(a, b, c, point2, point3, point4, normal, dir, patch, hashValue << 4, physicsMaterial, effectMaterial);
#else
	bool collide1=Collision::CollideTriangleTriangle(a, b, c, point1, point2, point3, normal, dir, patch, hashValue, physicsMaterial, effectMaterial);
	bool collide2=Collision::CollideTriangleTriangle(a, b, c, point1, point3, point4, normal, dir, patch, hashValue << 1, physicsMaterial, effectMaterial);
	//const U32 numFirstContacts(patch.m_NumContactPoints);
	//bool collide3=Collision::CollideTriangleTriangle(a, b, c, point2, point4, point1, normal, dir, patch, hashValue << 2, physicsMaterial, effectMaterial);
	//bool collide4=Collision::CollideTriangleTriangle(a, b, c, point2, point3, point4, normal, dir, patch, hashValue << 4, physicsMaterial, effectMaterial);

	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(point1, point2, point3, Maths::RGBA(0, 0, 255, 0));
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(point1, point3, point4, Maths::RGBA(0, 0, 255, 0));

#endif

	//return (collide1|collide2|collide3|collide4);
	return (collide1|collide2);
}

/*template <int MaxContacts>
bool CollideCylinderTriangle2(const Cylinder& cylinder, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, C_ContactPatch<MaxContacts>& results,
							  const Maths::HMat33 PCREF cylinderMatrix, const Maths::HVec3 PCREF clylinderPosition)
{
	const Maths::HVec3 localA(cylinderMatrix * (a-cylinderPosition));
	const Maths::HVec3 localB(cylinderMatrix * (b-cylinderPosition));
	const Maths::HVec3 localC(cylinderMatrix * (c-cylinderPosition));

	const Maths::HVec3 localEnd(cylinder.LocalCenterAxis() * cylinder.HalfHeight());
	const Maths::HVec3 localStart(-localEnd);

	Maths::HVec3 closestSegment(Maths::UNDEFINED), closestTri(Maths::UNDEFINED);
	Maths::HF32 dis(Maths::UNDEFINED);
	Collision::ClosestPtSegmentTriangle(localStart, localEnd, localA, localB, localC, closestSegment, closestTri, dist);

	if(dist > (cylinder.Radius() * cylinder.Radius())
	{
		return false; //No collision because capsule doesn't collide with triangle!
	}

	//Now we have closest points, we need to use the vector between the two closest points
	//to find the support point on the cylinder providing deepest point of intersection...
	const Maths::HVec3 dif(closestTriangle - closestSegment);
	
	const Maths::HF32 zero(Maths::Constants<HF32>::Zero());
	const Maths::HF32 one(Maths::Constants<HF32>::One());

	const HF32 d(dif.Y());
	const HVec3 w(dif - cylinder.LocalCenterAxis()*d ); // the component of dir orthogonal to u
	const C_Normal normal(w);
	const Maths::Scalar_Bool b(d.GT(zero));
	const HF32 t=Select(b, one, -one );
	
	Maths::HVec3 support(cylinder.Center()+cylinder.LocalCenterAxis()*t*cylinder.HalfHeight()+normal*cylinder.Radius();


	Maths::HVec3 contact(Maths::UNDEFINED);
	if(Collision::IntersectTriangleSegment(localA, localB, localC, closestSegment, support, contact))
	{
		//We have a point of contact...

	}




}*/

//template <int MaxPatches, int MaxContacts, class Triangle>
//bool CollideCylinderTriangleMesh(const C_Shape* const __restrict shape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::Float3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, 
//								 const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& results, const U32 materialIndex, const Maths::HVec3& low, const Maths::HVec3& high)
//{
//	using namespace Maths;
//	const HF32 disEps(SharedPhysics::Constants::CloseNormal());
//	Collision::C_Cylinder cylinder=((Collision::C_CylinderShape*)shape)->m_Cylinder;
//
//	const HVec3 range(high - low);
//	for(U32 i=0; i<numTris; ++i)
//	{
//		const Triangle& temp=tris[i];
//		const HVec3 localA(&verts[temp->m_IndexA ]);
//		const HVec3 localB(&verts[temp->m_IndexB ]);
//		const HVec3 localC(&verts[temp->m_IndexC ]);
//		const HVec3 a = localA*range + low;
//		const HVec3 b = localB*range + low;
//		const HVec3 c = localC*range + low;
//		/*const HVec3 a=Load<HVec3, MathsStorage::Float3>(&verts[temp.m_IndexA]);
//		const HVec3 b=Load<HVec3, MathsStorage::Float3>(&verts[temp.m_IndexB]);
//		const HVec3 c=Load<HVec3, MathsStorage::Float3>(&verts[temp.m_IndexC]);*/
//
//		const HVec3& norm(-Maths::Load<Maths::HVec3>(&normals[temp.m_NormalIndex]));
//
//		//if(((capsuleMiddle - a) | norm) > zero)
//		//{
//		//	//normal = -normal;
//		//	continue;	
//		//}
//
//		C_ContactPatch<MaxContacts>* pPatch = &results.m_ContactPatches[results.m_NumContactPatches];
//		pPatch->m_Normal = Maths::C_Normal(norm, Maths::C_Normal::UNIT_LENGTH_VECTOR);
//		pPatch->m_NumContactPoints = 0;
//		S32 incr = 1;
//		for(U32 j = 0; j < results.m_NumContactPatches; ++j)
//		{
//			Maths::HF32 dis = results.m_ContactPatches[j].m_Normal.m_V | norm;
//			if(dis >= disEps)
//			{
//				pPatch = &results.m_ContactPatches[j];
//				incr = 0;
//				break;
//			}
//		}
//		
//		CollideCylinderTriangle<MaxContacts>(cylinder, a, b, c, *pPatch);
//
//		results.m_NumContactPatches += incr;
//	
//	}
//	return false;
//}

template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideCylinderTriangleMesh(C_Shape* shape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
		const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
		const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high)
{
		using namespace Maths;
		//C_CylinderShape* pShape((C_CylinderShape*)Spu::FetchAlloc(shape, sizeof(C_CylinderShape)));
		C_CylinderShape* pShape = (C_CylinderShape*)shape;
		C_Cylinder& convex(pShape->m_Cylinder);
		//Loop through all triangles, make a C_Triangle around it, then do GJK/EPA collision detection!
		const HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		//const Maths::HF32 third(Maths::Constants<Maths::HF32>::Third());
		const HF32 twenty(SharedPhysics::Constants::Twenty());
		C_Triangle triangle;
		triangle.Margin(zero);//convex.Margin());
		const HF32 closeNormal(SharedPhysics::Constants::CloseNormal());
		bool bResult(false);
		HVec3 contact(Maths::UNDEFINED);
		HVec3 normal(Maths::UNDEFINED);
		HF32 pen(Maths::UNDEFINED);
		const HF32 margin(convex.Margin());
		const HF32 disEps(SharedPhysics::Constants::CloseNormal());

		MathsStorage::HenDN3 dNorm;
		//const Maths::HVec3 centre(convex.Centre());
		const HVec3 range(high - low);
		for(U32 a = 0; a < numTris; ++a)
		{
			//Spu::MemBaseMarker marker;
			//const Triangle* tri((Triangle*)Spu::FetchAllocUnaligned(tris+a, sizeof(Triangle)));
			const Triangle* tri = &tris[a];

			const HVec3 localA(&verts[tri->m_IndexA ]);
			const HVec3 localB(&verts[tri->m_IndexB ]);
			const HVec3 localC(&verts[tri->m_IndexC ]);
			triangle.m_VertexA = localA*range + low;
			triangle.m_VertexB = localB*range + low;
			triangle.m_VertexC = localC*range + low;
			
			dNorm = tri->m_SurfaceNormal;
			triangle.m_Normal = (-Maths::Load<Maths::HVec3, MathsStorage::HenDN3>(&dNorm));
			const Maths::HF32 dotA(triangle.m_VertexA | triangle.m_Normal);

			const Maths::HF32 dotCentre((convex.Centre() | triangle.m_Normal) - dotA);
			if(dotCentre >= zero)
				continue;

			//triangle.m_Extrusion = Maths::Max(zero, dotNow * twenty);

			C_ContactPatch<MaxContacts>* pPatch(NULL);
			bool incr = false;
			if(res.m_NumContactPatches < MaxPatches)
			{
				pPatch = &res.m_ContactPatches[res.m_NumContactPatches];
				pPatch->m_NumContactPoints = 0;
				pPatch->m_Normal = Maths::C_Normal(triangle.m_Normal, Maths::C_Normal::UNIT_LENGTH_VECTOR);
				incr = true;
			}
			
			for(U32 j = 0; j < res.m_NumContactPatches; ++j)
			{
				Maths::HF32 dis(res.m_ContactPatches[j].m_Normal.m_V| triangle.m_Normal);
				if(dis >= disEps)
				{
					pPatch = &res.m_ContactPatches[j];
					incr = false;
					break;
				}
			}

			if(!pPatch)
				continue;

			
			
			const U32 numContacts = pPatch->m_NumContactPoints;

			 
			
			bool bContact = CollideCylinderTriangle(convex, triangle.m_VertexA, triangle.m_VertexB, triangle.m_VertexC, *pPatch, triangleStartIndex + a, physicsMaterial, effectMaterial);
			bResult = bResult || bContact;
			res.m_NumContactPatches += (incr && bContact);			
		}
		return bResult;

}

template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideCylinderTriangleMesh(C_CylinderProxy& proxy, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
		const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
		const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high)
{
		using namespace Maths;

		const C_Cylinder& convex(proxy.m_Cylinder);
		//Loop through all triangles, make a C_Triangle around it, then do GJK/EPA collision detection!
		const Maths::HF32 zero(Maths::Constants<HF32>::Zero());
		C_Triangle triangle;
		triangle.Margin(zero);//convex.Margin());
		const HF32 closeNormal(SharedPhysics::Constants::CloseNormal());
		bool bResult(false);
		Maths::HVec3 contact(UNDEFINED);
		Maths::HVec3 normal(UNDEFINED);
		Maths::HF32 pen(UNDEFINED);
		const HF32 margin(convex.Margin());
		const HF32 disEps(SharedPhysics::Constants::CloseNormal());

		//const Maths::HVec3 centre(convex.Centre());
		const HMat33 orientation(proxy.m_PosRot.Rotation());
		const HVec3 position(proxy.m_PosRot.Translation());
		const HF32 twenty(SharedPhysics::Constants::Twenty());

		MathsStorage::HenDN3 dNorm;
		const HVec3 range(high - low);
		for(U32 a = 0; a < numTris; ++a)
		{
			Spu::MemBaseMarker marker;
			//const Triangle* tempTri=(const Triangle*)Spu::FetchAllocUnaligned(tris + a, sizeof(Triangle));
			const Triangle* tempTri = &tris[a];
			//((U32&)dNorm) = Spu::Get( (U32*)normals + tempTri->m_NormalIndex);
			dNorm = tempTri->m_SurfaceNormal;

			const HVec3 localA(&verts[tempTri->m_IndexA ]);
			const HVec3 localB(&verts[tempTri->m_IndexB ]);
			const HVec3 localC(&verts[tempTri->m_IndexC ]);
			const HVec3 vertexA = localA*range + low;
			const HVec3 vertexB = localB*range + low;
			const HVec3 vertexC = localC*range + low;
			HVec3 normal = -Load<HVec3, MathsStorage::HenDN3>(&dNorm);

			const HF32 dotA(vertexA | normal);
			

			const Maths::HF32 dotCentre((convex.Centre() | normal) - dotA);
			if(dotCentre >= zero)
				continue;


			C_ContactPatch<MaxContacts>* pPatch(NULL);
			bool incr = false;
			if(res.m_NumContactPatches < MaxPatches)
			{
				pPatch = &res.m_ContactPatches[res.m_NumContactPatches];
				pPatch->m_NumContactPoints = 0;
				pPatch->m_Normal = C_Normal(normal * orientation, C_Normal::UNIT_LENGTH_VECTOR);
				incr = true;
			}
			
			for(U32 j = 0; j < res.m_NumContactPatches; ++j)
			{
				HF32 dis(res.m_ContactPatches[j].m_Normal.m_V| normal);
				if(dis >= disEps)
				{
					pPatch = &res.m_ContactPatches[j];
					incr = false;
					break;
				}
			}

			if(!pPatch)
				continue;

			
			
			const U32 numContacts = pPatch->m_NumContactPoints;
			bool bContact = CollideCylinderTriangle(convex, vertexA, vertexB, vertexC, *pPatch, triangleStartIndex + a, physicsMaterial, effectMaterial);
			for(U32 c = numContacts; c < pPatch->m_NumContactPoints; ++c)
			{
				pPatch->m_ContactPoints[c].m_ContactPoint = pPatch->m_ContactPoints[c].m_ContactPoint * orientation + position;
			}
			bResult = bResult || bContact;
			res.m_NumContactPatches += (incr && bContact);		
		}
		return bResult;

}

template <class Triangle>
bool ContinuousCylinderTriangleMesh(C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, 
									const Maths::HVec3 PCREF r, Maths::HF32& maxLambda, Maths::HF32& lambda, Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB, const Maths::HVec3& low, const Maths::HVec3& high)
{
	using namespace Maths;

	Collision::C_ContinuousShape* pContinuous = (Collision::C_ContinuousShape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_ContinuousShape));
	Collision::C_CylinderShape* pCylinderShape = (Collision::C_CylinderShape*)Spu::FetchAlloc(pContinuous->m_pContinuousPrimitive, sizeof(C_CylinderShape));
	Collision::C_Cylinder cylinder= pCylinderShape->m_Cylinder;
	Collision::C_Triangle triangle;
	const HF32 zero(Constants<HF32>::Zero());

	bool collide = false;

	const HVec3 range(high - low);

	for(U32 i=0; i<numTris; ++i)
	{
		const Triangle& temp=tris[i];
		
		const HVec3 localA(&verts[temp.m_IndexA ]);
		const HVec3 localB(&verts[temp.m_IndexB ]);
		const HVec3 localC(&verts[temp.m_IndexC ]);
		triangle.m_VertexA = localA*range + low;
		triangle.m_VertexB = localB*range + low;
		triangle.m_VertexC = localC*range + low;
		/*triangle.m_VertexA=Load<HVec3, MathsStorage::Float3>(&verts[temp.m_IndexA]);
		triangle.m_VertexB=Load<HVec3, MathsStorage::Float3>(&verts[temp.m_IndexB]);
		triangle.m_VertexC=Load<HVec3, MathsStorage::Float3>(&verts[temp.m_IndexC]);*/

		const HVec3& norm(-Maths::Load<Maths::HVec3>(&temp.m_SurfaceNormal));
		//lambda = zero;

		if(Collision::GJKRayCast<C_Cylinder, C_Triangle>(cylinder, triangle, r, maxLambda, lambda, nor, closestA, closestB))
		{
			nor = norm;
			maxLambda = lambda;
			collide = true;
		}
	}

	return collide;
}

};

#endif
