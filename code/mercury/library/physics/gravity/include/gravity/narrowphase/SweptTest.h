#ifndef SWEPTTEST_H
#define SWEPTTEST_H
#include "gravity/narrowphase/primitive.h"

namespace Collision
{
	inline Maths::Scalar_Bool SweptSphereTriangleTest(const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, const Maths::HF32 PCREF radius, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::C_Normal PCREF normal, Maths::HVec3& contact, Maths::C_Normal& conNormal, Maths::HF32& dis)
	{

		using namespace Maths;
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());
		const Scalar_Bool bFalse(false);
		

		/*
			Assume the sphere is in front of the plane of triangle and moving toward it
		*/
		
		HVec3 dir(end-start);
		const HF32 dirLength(dir.Length());


		if((dir|normal).Abs().EQ(zero))//the dir is moving parallel to the plane of triangle
		{
			dir=- dirLength*normal;
		}
		
		const HVec3 D(start - radius*normal);
		
		const HF32 d(normal|a);
		const HF32 temp1(normal|D);

		
		const HF32 temp2(normal|dir);
		const HF32 t((d - temp1)/temp2);

		const HVec3 p(D + (dir*t));
		HVec3 q(UNDEFINED);


		if(t.LE(zero) | t.GE(one))
		{
			return bFalse; //no intersection
		}
		else
		{
			if(Collision::PointInsideTriangle(p, a, b, c))
			{
				contact=p;
				conNormal=normal;
				//conNormal=Maths::C_Normal(start-p);
				dis=(start-p).Length() - radius;
				assert(dis<=dirLength);
				return !bFalse;
			}
			else
			{
				Collision::ClosestPtPointTriangle(p, a, b, c,q);
		
				const HVec3 qs(q-start);
				const HF32 sQs(qs|qs);
				const HF32 equationC(sQs-radius*radius);
				const HF32 equationA(dir|dir);
				const HF32 equationB(-(qs|dir));

				const HF32 discriminant(equationB*equationB - equationA*equationC);
				if(discriminant.LT(zero))
					return bFalse; // no intersection
				else
				{
					const HF32 t((-equationB - discriminant.Sqrt())/equationA);
					contact=q;
					HVec3 R(q - t*dir);
					dis=(R-contact).Length();
				//	conNormal=Maths::C_Normal(start-R);
					conNormal=normal;
					return t.GE(zero) & t.LE(one);
				}
			}

		}
	}


	inline bool SweptSphereSphereTest(const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, const Maths::HF32 PCREF radius, const Collision::C_Sphere& sphere, Maths::HVec3& contact, Maths::C_Normal& conNormal, Maths::HF32& dis)
	{
		
		using namespace Maths;
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());
		const HVec3 v(end - start);
		const HVec3 u(start - sphere.Centre() );
		//inflate the sphere with radius
		const HF32 r(radius + sphere.Radius());
		const HF32 sR(r*r);
		const HF32 sU(u|u);

		const HF32 a(v|v);
		const HF32 b(u|v);
		const HF32 c(sU - sR);

		const HF32 discriminant(b*b - a*c);
		if(discriminant.LT(zero))
			return false; //no intersection

		const HF32 t=((-b-discriminant.Sqrt())/a);
		if(t.GT(one) | t.LT(zero))
			return false;

		const HVec3 s(start + v*t);
		const C_Normal dir(s - sphere.Centre());
		contact=sphere.Centre()+dir*sphere.Radius();
		conNormal=dir;
		dis=(start - s).Length();
		return true;
	}

	inline bool SweptSphereOBBTest(const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, const Maths::HF32 PCREF radius, const Collision::C_OBB& obb, Maths::HVec3& contact, Maths::C_Normal& conNormal, Maths::HF32& dis)
	{

		using namespace Maths;
		HVec3 minContacts(UNDEFINED);
		HVec3 maxContacts(UNDEFINED);
		//const Maths::HMat33& u =obb.u.Transposed();
		const HMat33& u = obb.Orientation();
		const HVec3 p(u*(start-obb.Centre()));
		const HVec3 q(u*(end-obb.Centre()));
		const HVec3 v(q-p);
		const HF32 sr(radius*radius);


		//inflate the obb with radius
		Collision::C_OBB box(obb);
		const Maths::HVec3 r(radius, radius, radius);
		//box.e+=r;
		box.Extent(box.Extent() + r);
		//The ray intersect with the inflated obb
		if(IntersectSegmentBox(p, q, box.Extent(), minContacts, maxContacts))
		{
			const HF32 zero(Constants<HF32>::Zero());
			const HF32 one(Constants<HF32>::One());

			//const HF32 ex(obb.e.X());
			//const HF32 ey(obb.e.Y());
			//const HF32 ez(obb.e.Z());

			//HF32 x(minContacts.X());
			//HF32 y(minContacts.Y());
			//HF32 z(minContacts.Z());


			//if(x > ex) x=ex;
			//if(x <-ex) x=-ex;

			//if(y > ey) y=ey;
			//if(y <-ey) y=-ey;

			//if(z > ez) z=ez;
			//if(z <-ez) z=-ez;

			////get the closest point from minContacts to the original obb in the local space
			//const HVec3 closest(x, y, z);

			const HVec3 e(obb.Extent());
			const HVec3 closest = Min(e, Max(minContacts, -e));

			//In local space
			const HVec3 pc(p-closest);
			const HF32 a(v|v);
			const HF32 b(pc|v);
			const HF32 c((pc|pc) - sr);

			const HF32 discriminant(b*b - a*c);
			if(discriminant.LT(zero))
				return false; //no intersection

			const HF32 t=((-b-discriminant.Sqrt())/a);
			if(t.GT(one) | t.LT(zero))
				return false;

			const HVec3 center(p + t*v);
			const HVec3 worldCenter(center*u + obb.Centre());
			const HVec3 worldClosest(closest*u+obb.Centre());


		/*	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(minContacts*obb.u + obb.c, 0.2f, RGBA(0, 0, 255, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(minContacts*obb.u + obb.c, "M");

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(worldClosest, 0.2f, RGBA(0, 0, 255, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(worldClosest, "C");*/

			//const Maths::HVec3 v(minContacts - closest);
			//const Maths::HF32 sDist(v|v);
			//if(sDist > (radius*radius))
			//	return false; //no intersection			
			contact=worldClosest;
			conNormal=Maths::C_Normal(worldCenter - worldClosest);
			dis=(start-worldCenter).Length();
			return true;
		}
		return false;
		
	}
	
	inline bool SweptSphereCapsuleTest(const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, const Maths::HF32 PCREF radius, const Collision::C_Capsule& capsule, Maths::HVec3& contact, Maths::C_Normal& conNormal, Maths::HF32& dis)
	{
		using namespace Maths;
		//inflate the capsule with the sphere's radius

		const HF32 r(capsule.Radius() + radius);

		const HVec3 m(start - capsule.Start());
		const HVec3 n(end - start);
		const HVec3 d(capsule.End() - capsule.Start());

		/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_Start, r, RGBA(255, 255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(capsule.m_End, r, RGBA(255, 255, 0, 255));
		
		HF32 base(10.f);
		for(int i=0; i<10; i++)
		{
			HF32 ratio(((HF32)i)/base);
			HVec3 e(capsule.m_Start+(capsule.m_End-capsule.m_Start)*ratio);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, d, r, RGBA(255, 255, 0));
		}*/

		const HVec3 dn(d^n);
		const HVec3 dm(d^m);
		const HF32 dd(d|d);
		const HF32 rr(r*r);

		const HF32 a(dn|dn);
		const HF32 b(dm|dn);
		const HF32 c((dm|dm) - rr*dd);


		const HF32 discriminant(b*b - a*c);

		const HF32 zero(Maths::Constants<HF32>::Zero());
		const HF32 one(Maths::Constants<HF32>::One());
		if(discriminant.LT(zero))
			return false; //no intersection

		const HF32 sqrt(discriminant.Sqrt());
		const HF32 aRecip(a.Reciprocal());
		const HF32 t1((-b-sqrt)*aRecip);
		const HF32 t2((-b+sqrt)*aRecip);

		/*const Maths::Scalar_Bool con(t1.LT(zero));
		const Maths::HF32 t(Select(con, t2, t1));*/

	
		if(t1.LT(zero) & t2.GT(zero))
		{

			HVec3 closest(Maths::UNDEFINED);
			Collision::ClosestPtPointSegment(start, capsule.Start(), capsule.End(), closest);
			const Maths::C_Normal nor(start-closest);
			contact = closest + nor*capsule.Radius();
			conNormal=nor;
			dis=zero;
			return true;
		}

		if(t1.GT(one))
			return false;

		HVec3 s(start + n*t1);

		HVec3 closest(Maths::UNDEFINED);
		Collision::ClosestPtPointSegment(s, capsule.Start(), capsule.End(), closest);

		const Maths::HVec3 dir(s-closest);
		const Maths::HF32 length(dir.Length());
		if(length.GT(r))
		{
			const HVec3 sc(start - closest);
			//const Maths::HVec3 d(end - start);
			const HF32 a(n|n);
			const HF32 b(sc|n);
			const HF32 c((sc|sc)-rr);
			const HF32 discriminant(b*b-a*c);
			if(discriminant.LT(zero))
				return false;
			const HF32 t((-b-discriminant.Sqrt())/a);
			s=start + t*n;
		}

		/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(s, 0.2f, RGBA(0, 0, 255, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(s, "S");

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closest, 0.2f, RGBA(255, 0, 255, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(closest, "C");*/

		const C_Normal nor(s-closest);
		//contact = closest + nor*capsule.Radius();
		contact = s - nor * radius;
		conNormal=nor;
		dis=(start - s).Length();
		


		return true;
	}

	inline bool SweptSphereCylinderTest(const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, const Maths::HF32 PCREF radius, const Collision::C_Cylinder& cylinder, Maths::HVec3& contact, Maths::C_Normal& conNormal, Maths::HF32& dis)
	{
		using namespace Maths;
		//inflate the cylinder
		const HVec3 center(cylinder.Centre());
		const HVec3 u(cylinder.CentreAxis());
		const HF32 r(cylinder.Radius() + radius);
		const HF32 halfHeight(cylinder.HalfHeight() + radius );

		const Maths::HVec3 s(center - u*halfHeight);
		const Maths::HVec3 e(center + u*halfHeight);

	/*	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(s, 0.2f, RGBA(0, 255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(e, 0.2f, RGBA(0, 255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(s, "s");
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(e, "e");

		HF32 base(20.f);
		for(int i=0; i<20; i++)
		{
			HF32 ratio(((HF32)i)/base);
			HVec3 e(s+(e-s)*ratio);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, u, r, RGBA(255, 0, 0));
		}*/

		const HVec3 d(end-start);
	
		const HF32 cu(center|u);
		const HF32 su(start|u);
		const HF32 du(d|u);
		const HVec3 n(u*cu);
		const HVec3 m(u*su);
		const HVec3 o(u*du);


		const HVec3 cs(start-center);
		const HF32 csu(cs|u);

		const HVec3 z(cs -m +n);
		const HVec3 g(d-o);

		const HF32 a(g|g);
		const HF32 b(z|g);
		const HF32 sqR(r*r);
		const HF32 c((z|z) - sqR);

		const HF32 discriminant(b*b - a*c);

		const HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		
		if(discriminant.LE(zero)) // no intersection with the infinite cylinder
			return false;
		
		const HF32 comp(discriminant.Sqrt());
		const HF32 t0((-b+comp)/a);
		const HF32 t1((-b-comp)/a);

		const HF32 _min(Min(t0, t1)); //the point when the ray enter the cyliner
		const HF32 _max(Max(t0, t1)); //the point when the ray exit the cylinder

		//const Maths::HVec3 p(i+min*j); //p is the projection point of x on the local axis

		//Now need to determine whether the point is in the valid range, which is the two endcaps

		const HVec3 x0(start + _min*d);
		const HVec3 x1(start + _max*d);

		/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(x0, 0.5f, RGBA(0, 255, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(x1, 0.5f, RGBA(0, 255, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(x0, "x0");
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(x1, "x1");*/
		//Calculate the sign distance from the point to the endcap planes
		const HF32 dist0(u|(x0-center));
		const HF32 dist1(u|(x1-center));

		const HF32 dist(dist0*dist1);

		const Scalar_Bool con0(dist0.Abs().LE(halfHeight));
		const Scalar_Bool con1(dist1.Abs().LE(halfHeight));
		//the enter point and exit point are on the same side of a endcap plane
		const Scalar_Bool con2(dist.GT(zero));

		if((!con0) & (!con1) & (con2))
			return false;	//no intersection with the finite cylinder

		//The start point is inside the inflated cylinder and the end point is outside the inflated cylinder
		if(_min.LT(zero))
		{
			HVec3 closest(Maths::UNDEFINED);
			Collision::ClosestPtPointCylinder(start, cylinder, closest);
			contact = closest;
			conNormal = C_Normal(start - closest);
			dis = zero;
			return true;
		}

		/*
			calculate the point hit the inflated endcap(either the plane(ux=halfHeight or ux=-halfHeight))
		*/
		
		const Scalar_Bool cont(dist0.LE(zero) && con0);
		const HVec3 nor=Select(cont, -u, u);
		const HVec3 endCap(center + nor*halfHeight);
		const HVec3 v(start - endCap);
		const HF32 k(-(v|u)/(d|u));
		const HVec3 planeIntersectPoint(start + k*d);

		const HVec3 v1(planeIntersectPoint - endCap);
		const Scalar_Bool cont1((v1|v1).LT(sqR));

		const HVec3 point=Select(cont1, x0, (start+k*d));

		/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(x0, 0.2f, RGBA(0, 255, 255, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(x0, "x0");*/

		/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point, 0.2f, RGBA(0, 255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(point, "p");

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(endCap, 0.2f, RGBA(0, 255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(endCap, "endCap");
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(endCap, u, 1.f, 0.2f, RGBA(255, 0, 255));*/
		
		
		//Get the closest from point to the original cylinder
		HVec3 closest(Maths::UNDEFINED);
		Collision::ClosestPtPointCylinder(point, cylinder, closest);
		//calcuate the center, which lie in the ray, from the closest point
		const HVec3 sc2(start - closest);
		const HF32 a2(d|d);
		const HF32 b2(sc2|d);
		const HF32 c2((sc2|sc2) - (radius*radius));

		const HF32 discriminant2(b2*b2 - a2*c2);
		if(discriminant2.LT(zero))
			return false;

		const HF32 t2((-b2- discriminant2.Sqrt())/ a2);
		const HVec3 x(start + t2*d);
		contact= closest;
		conNormal=Maths::C_Normal(x - closest);
		dis=(start - x).Length();

	/*	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(x, 0.2f, RGBA(0, 0, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(x, "x");*/


		return true;
	}

	inline bool SweptSphereCompoundTest(const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, const Maths::HF32 PCREF radius, const Collision::C_CompositeShape* shape, Maths::HVec3& contact, Maths::C_Normal& conNormal, Maths::HF32& dis)
	{

		Collision::C_CompositeShapeIterator iter(shape);
		//for(U32 i=0; i<shape->m_NumShapes; ++i)
		while(iter.HasNextShape())
		{
			iter.NextShape();
			//Collision::C_ShapeWrapper& wrapper=shape->m_pShapes[i];
			//Collision::C_Shape* subShape=wrapper.m_pShape;
			Collision::C_Shape* subShape=iter.GetCurrentShape<Collision::C_Shape>();
			switch(subShape->GetShape())
			{
			case Collision::E_Box_Shape:
				break;
			case Collision::E_ConvexHull_Shape:
				break;
			case Collision::E_Capsule_Shape:
				break;
			case Collision::E_Cylinder_Shape:
				break;
			case Collision::E_Composite_Shape:
				break;
			default:
				return false;
			}
		}
	}

	//inline bool SweptSphereConvexHullTest(const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, const Maths::HF32 PCREF radius, const Collision::C_ConvexHull& convex, Maths::HVec3& contact, Maths::C_Normal& conNormal, Maths::HF32& dis)
	//{
	//	using namespace Maths;
	//	const HF32 zero(Constants<HF32>::Zero());
	//	const HF32 one(Constants<HF32>::One());
	//	const HVec3 d(end - start);
	//	const HF32 rr(radius*radius);

	//	HVec3 curContact(UNDEFINED);
	//	C_Normal curNormal(UNDEFINED);
	//	HF32 curDist(Constants<HF32>::m_Max);

	//	bool collided=false;

	//	for(U32 i=0; i<convex.m_NumTriangles; ++i)
	//	{
	//		Collision::C_ConvexTriangle& tri=convex.m_pTriangles[i];
	//		const HVec3 a=Maths::Load<HVec3, MathsStorage::Float3>(&convex.m_pVertices[tri.m_VertexA]);
	//		const HVec3 b=Maths::Load<HVec3, MathsStorage::Float3>(&convex.m_pVertices[tri.m_VertexB]);
	//		const HVec3 c=Maths::Load<HVec3, MathsStorage::Float3>(&convex.m_pVertices[tri.m_VertexC]);
	//		const HVec3 n = Load<HVec3, MathsStorage::HenDN3>(&tri.m_Normal);
	//		//const HVec3 n=Maths::Load<HVec3, MathsStorage::Float3>(&tri.m_Normal);
	//		const C_Normal normal(n, C_Normal::UNIT_LENGTH_VECTOR);
	//		const HF32 dist(n|a);
	//		const HF32 sDist((n|start) - dist);
	//		const Scalar_Bool con(sDist.GT(zero));
	//		const C_Normal v=Select(con, -normal, normal);
	//		//Get the projection point of start to the triangle plane
	//		const HVec3 D(start + v*radius);

	//		const HF32 nD(n|D);
	//		const HF32 nd(n|d);

	//		const HF32 t((dist - nD)/nd);

	//		if(t.LT(zero)|t.GT(one))
	//			continue;

	//		const HVec3 p(D + d*t);

	//		if(Collision::PointInsideTriangle(p, a, b, c))
	//		{
	//			const HF32 dist=(D-p).Length();
	//			if(curDist.GT(dist))
	//			{
	//				curDist=dist;
	//				curContact=p;
	//				curNormal=normal;
	//				collided=true;
	//			}
	//		}
	//		else
	//		{
	//			HVec3 closest(Maths::UNDEFINED);
	//			Collision::ClosestPtPointTriangle(p, a, b, c, closest);
	//			const HVec3 cs(closest - start);
	//			const HVec3 nd(-d);
	//			const HF32 aE(nd|nd);
	//			const HF32 bE(cs|nd);
	//			const HF32 cE((cs|cs) - rr);
	//		
	//			const HF32 discriminant(bE*bE - aE*cE);

	//			if(discriminant.LT(zero))
	//				continue;

	//			const HF32 t((-bE - discriminant.Sqrt())/aE);

	//			if(t.LT(zero) | t.GT(one))
	//				continue;
	//			
	//			const HVec3 q(closest + nd*t);

	//			const HF32 dist((closest - q).Length());

	//			if(curDist.GT(dist))
	//			{
	//				curDist=dist;
	//				curContact=closest;
	//				curNormal=normal;
	//				collided=true;
	//			}
	//		}
	//	}
	//	return collided;
	//	
	//}

	//inline bool SweptSphereConvexHullTest(const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, const Maths::HF32 PCREF radius, const Collision::C_SpuConvexHull& convex, Maths::HVec3& contact, Maths::C_Normal& conNormal, Maths::HF32& dis)
	//{
	//	using namespace Maths;
	//	const HF32 zero(Constants<HF32>::Zero());
	//	const HF32 one(Constants<HF32>::One());
	//	const HVec3 d(end - start);
	//	const HF32 rr(radius*radius);

	//	HVec3 curContact(UNDEFINED);
	//	C_Normal curNormal(UNDEFINED);
	//	HF32 curDist(Constants<HF32>::Max());

	//	bool collided=false;

	//	//const Maths::HVec3 range(convex.m_High - convex.m_Low);
	//	//const Maths::HVec3 range(convex.Range());

	//	U32 numTriangles = convex.GetNumTriangles();

	//	for(U32 i=0; i<numTriangles; ++i)
	//	{
	//		Collision::C_ConvexTriangle& tri=convex.GetTriangle(i);
	//		const HVec3 a(convex.GetWorldVertex(tri.m_VertexA));
	//		const HVec3 b(convex.GetWorldVertex(tri.m_VertexB));
	//		const HVec3 c(convex.GetWorldVertex(tri.m_VertexC));
	//		

	//		const HVec3 n = Load<HVec3, MathsStorage::HenDN3>(&tri.m_Normal);
	//		//const HVec3 n=Maths::Load<HVec3, MathsStorage::Float3>(&tri.m_Normal);
	//		const C_Normal normal(n, C_Normal::UNIT_LENGTH_VECTOR);
	//		
	//		const HF32 dist(n|a);
	//		const HF32 sDist((n|start) - dist);
	//		const Scalar_Bool con(sDist.GT(zero));
	//		const C_Normal v=Select(con, -normal, normal);
	//		//Get the projection point of start to the triangle plane
	//		const HVec3 D(start + v*radius);

	//		const HF32 nD(n|D);
	//		const HF32 nd(n|d);

	//		const HF32 t((dist - nD)/nd);

	//		if(t.LT(zero)|t.GT(one))
	//			continue;

	//		const HVec3 p(D + d*t);

	//	/*	const HVec3 v(start - a);
	//		const HF32 t(-(v|n)/(d|n));
	//		const HVec3 p(start + t*d);*/
	//		
	//		if(Collision::PointInsideTriangle(p, a, b, c))
	//		{
	//			const HF32 dist=(D-p).Length();

	//			if(curDist.GT(dist))
	//			{
	//				curDist=dist;
	//				curContact=p;
	//				contact = p;
	//				curNormal=normal;
	//				collided=true;
	//			}
	//		}
	//		else
	//		{
	//			HVec3 closest(Maths::UNDEFINED);
	//			Collision::ClosestPtPointTriangle(p, a, b, c, closest);
	//			const HVec3 cs(closest - start);
	//			const HVec3 nd(-d);
	//			const HF32 aE(nd|nd);
	//			const HF32 bE(cs|nd);
	//			const HF32 cE((cs|cs) - rr);
	//		
	//			const HF32 discriminant(bE*bE - aE*cE);

	//			if(discriminant.LT(zero))
	//				continue;

	//			const HF32 t((-bE - discriminant.Sqrt())/aE);

	//			if(t.LT(zero) | t.GT(one))
	//				continue;
	//			
	//			const HVec3 q(closest + nd*t);

	//			const HF32 dist((closest - q).Length());

	//			if(curDist.GT(dist))
	//			{
	//				curDist=dist;
	//				curContact=closest;
	//				contact = closest;
	//				curNormal=normal;
	//				collided=true;
	//			}
	//		}
	//	}

	//	return collided;
	//}

	inline bool SweptSphereConvexHullTest(const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, const Maths::HF32 PCREF radius, const Collision::C_SpuConvexHull& convex, Maths::HVec3& contact, Maths::C_Normal& conNormal, Maths::HF32& dis)
	{
		using namespace Maths;
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());
		const HVec3 d(end - start);
		const HF32 rr(radius*radius);

		HVec3 curContact(UNDEFINED);
		C_Normal curNormal(UNDEFINED);
		HF32 curDist(Constants<HF32>::Max());

		bool collided=false;

		//const Maths::HVec3 range(convex.m_High - convex.m_Low);
		//const Maths::HVec3 range(convex.Range());

		U32 numTriangles = convex.GetNumTriangles();

		//U32 i=14;
		for(U32 i=14; i<15; ++i)
		{
			Collision::C_ConvexTriangle& tri=convex.GetTriangle(i);
			const HVec3 a(convex.GetWorldVertex(tri.m_VertexA));
			const HVec3 b(convex.GetWorldVertex(tri.m_VertexB));
			const HVec3 c(convex.GetWorldVertex(tri.m_VertexC));
			

			const HVec3 n = Load<HVec3, MathsStorage::HenDN3>(&tri.m_Normal);
			//const HVec3 n=Maths::Load<HVec3, MathsStorage::Float3>(&tri.m_Normal);
			const C_Normal normal(n, C_Normal::UNIT_LENGTH_VECTOR);
			const HF32 signDist0((start - a)|normal);
			const HF32 signDist1((end - a)|normal);
			if((signDist0*signDist1).GT(zero) && (signDist0.Abs().LT(radius) || signDist1.Abs().LT(radius)))
			{
				//the fat ray doesn't intersect the plane
				continue;
			}

			//move the triangle plane up on the direction of normal about radius height
			const HVec3 newA(a + normal*radius);
			const HVec3 newB(b + normal*radius);
			const HVec3 newC(c + normal*radius);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(newA, newB, newC, RGBA(15, 60, 90));
			HVec3 intersectPoint(Maths::UNDEFINED);
			if(Collision::IntersectTriangleSegmentSB(newA, newB, newC, normal, start, end, intersectPoint))
			{
				//if(curDist.GT(dis))
				{
					contact = intersectPoint - normal*radius;
					dis = (intersectPoint - start).Length();
					curDist=radius;
				}
				collided = true;
			}
			else
			{
				HVec3 c1(Maths::UNDEFINED);
				HVec3 c2(Maths::UNDEFINED);
				HF32 sqDist(Maths::UNDEFINED);
				Collision::ClosestPtSegmentTriangle(start, end, a, b, c, c1, c2, sqDist);
				const HF32 sqR(radius*radius);
				if(sqDist.GT(sqR))
					continue;

				

				const HF32 sqn(normal|normal);
				const HF32 san((start - a)|normal);
				const HF32 dn(d|normal);
				const HF32 t((sqn*radius - san)/dn);
				
				const HVec3 x(start + t*d);
				const HVec3 projX = x - normal*radius;
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(projX, 0.5f, RGBA(0, 0, 255));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(projX, "projX");
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(x, 0.5f, RGBA(0, 255, 0));
				if(PointInsideTriangle(projX, a, b, c))
				{
					contact = projX;
					dis = (x-start).Length();
				//	if(curDist.GT(dis))
					{
						curDist = radius;
					}
					collided = true;
				}
				else
				{
					const HVec3 closest = Collision::ClosestPtPointTriangle(projX, a, b, c);
					const HVec3 cs(start - projX);
					const HF32 a(d|d);
					const HF32 b(cs|d);
					const HF32 c((cs|cs)-radius*radius);

					const HF32 discriminant(b*b - a*c);

					if(discriminant.LT(zero)) // not intersect
						continue;
					
					const HF32 sqrt(discriminant.Sqrt());
					const HF32 aRecip(a.Reciprocal());
					const HF32 t1((-b-sqrt)*aRecip);
					const HF32 t2((-b+sqrt)*aRecip);
					const HF32 t = Min(t1, t2);
					const HVec3 x(start + t*d);

					contact = closest;
					dis = (x - start).Length();
					curDist = (projX - closest).Length();
					collided = true;

				}
				
			}
		
			//C_Normal n = Select(signDist0.GT(zero), -normal, normal);
			//const HF32 sqn(n|n);

			//HVec3 intersectPoint(Maths::UNDEFINED);
			//if(Collision::IntersectTriangleSegmentSB(a, b, c, normal, start, end, intersectPoint))
			//{
			//	//calculate the project point of start point to the plane
			//	const HF32 san((start - a)|n);
			//	const HF32 dn(d|n);
			//	const HF32 t((-sqn*radius - san)/dn);
			//	//calcuate the point which has a radius distance to the plane
			//	const HVec3 x(start + t*d);
			//	//calculate the project point
			//	const HVec3 proj(x + n*radius);
			//	//check to see whether the project point is inside the triangle
			//	if(Collision::PointInsideTriangle(proj, a, b, c))
			//	{
			//		contact = proj;
			//		dis = (x - start).Length();
			//		curDist = dis;
			//	}
			//	else
			//	{
			//		contact = Collision::ClosestPtPointTriangle(proj, a, b, c);
			//		const HVec3 cs(start - contact);
			//		const HF32 a(d|d);
			//		const HF32 b(cs|d);
			//		const HF32 c((cs|cs)-radius*radius);

			//		const HF32 discriminant(b*b - a*c);

			//		if(discriminant.LT(zero)) // not intersect
			//			continue;
			//		
			//		const HF32 sqrt(discriminant.Sqrt());
			//		const HF32 aRecip(a.Reciprocal());
			//		const HF32 t1((-b-sqrt)*aRecip);
			//		const HF32 t2((-b+sqrt)*aRecip);
			//		const HF32 t = Min(t1, t2);
			//		const HVec3 x(start + t*d);
			//		dis = (x -start).Length();
			//		curDist = dis;
			//	}
			//	collided = true;
			//}
			//else
			//{
			//	
			//	//HVec3 c1(Maths::UNDEFINED);
			//	//HVec3 c2(Maths::UNDEFINED);
			//	//HF32 sqDist(Maths::UNDEFINED);
			//	//Collision::ClosestPtSegmentTriangle(start, end, a, b, c, c1, c2, sqDist);
			//	//const HF32 sqR(radius*radius);
			//	//if(sqDist.GT(sqR))
			//	//	continue;

			//	//contact = c2;
			//	//const HVec3 cs(start - contact);
			//	//const HF32 a(d|d);
			//	//const HF32 b(cs|d);
			//	//const HF32 c((cs|cs)-radius*radius);

			//	//const HF32 discriminant(b*b - a*c);

			//	//if(discriminant.LT(zero)) // not intersect
			//	//	continue;
			//	//
			//	//const HF32 sqrt(discriminant.Sqrt());
			//	//const HF32 aRecip(a.Reciprocal());
			//	//const HF32 t1((-b-sqrt)*aRecip);
			//	//const HF32 t2((-b+sqrt)*aRecip);
			//	//const HF32 t = Min(t1, t2);
			//	//const HVec3 x(start + t*d);
			//	//
			//	////calculate the x projection point to the plane
			//	//const HVec3 ax(x-a);
			//	//const HF32 k(-(ax|n)/sqn);
			//	//const HVec3 projX(x + n*k);
			//	//if(Collision::PointInsideTriangle(projX))
			//	//{
			//	//	if((k*k*sqn).LT(sqR))
			//	//	{
			//	//		//calculate the 
			//	//	}
			//	//}
			//	//collided = true;
			//}
		}

		return collided;
	}
};

#endif
