#include "StdAfx.h"
#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/GJKRayCastCollision.h"
#include "gravity/narrowphase/GJK.h"
#include "gravity/narrowphase/GJK.inl"
#include "gravity/narrowphase/raytriangle.h"


#ifdef PLATFORM_SPU
#include "gravity/narrowphase/SPUManager.h"
#endif


namespace Collision
{
	bool GJKRayCastCapsule( const C_Shape* const __restrict shape, C_Ray& ray)
	{
		using namespace Maths;
		/*C_CapsuleShape* pCapsuleShape = (C_CapsuleShape*)shape;
		C_Capsule& capsule=pCapsuleShape->m_Capsule;

		const HVec3 r(ray.m_end - ray.m_start);
		const HF32 height = r.Length();
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());

		C_Sphere sphere;
		sphere.Centre(ray.m_start);
		sphere.Radius(ray.m_Radius);
		
		HF32 lambda(UNDEFINED);
		HVec3 normal(UNDEFINED), closestA(UNDEFINED), closestB(UNDEFINED);
		if(GJKRayCast<C_Sphere, C_Capsule>(sphere, capsule, -r, one, lambda,  normal, closestA, closestB))
		{
			const HVec3 contact(sphere.Centre() + lambda*r);
			const C_Normal norm(closestA - sphere.Centre());

			ray.m_result.m_contact=contact;
			ray.m_result.m_normal = C_Normal(norm, C_Normal::UNIT_LENGTH_VECTOR);
			ray.m_result.m_TriangleNormal = ray.m_result.m_normal;
			ray.m_result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
			ray.m_result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
			ray.m_result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
			ray.m_result.m_distance=(contact-ray.m_start)|ray.m_Direction;
			ray.m_result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
			ray.m_result.m_TriangleIndex = -1;
			ray.m_result.m_UserData = pCapsuleShape->m_UserData;
			ray.m_end=contact;
			return true;
		}
		return false;*/

		using namespace Maths;
		Spu::MemBaseMarker marker;
		C_CapsuleShape* pCapsuleShape = (C_CapsuleShape*)shape;
		const HF32 zero(Constants<HF32>::Zero());
		C_Capsule& capsule=pCapsuleShape->m_Capsule;
		const HVec3 m(ray.m_start - capsule.Start());
		const HVec3 n(ray.m_end - ray.m_start);
		const HVec3 d(capsule.End() - capsule.Start());
		const HF32 r(capsule.Radius() + ray.m_Radius);
		const HF32 r2(r * r);

		const HVec3 dn(d^n);
		const HVec3 dm(d^m);
		const HF32 dd(d|d);
		const HF32 rr(r*r);

		const HF32 a(dn|dn);
		const HF32 b(dm|dn);
		const HF32 c((dm|dm) - rr*dd);
		C_Normal normal(UNDEFINED);

		const HF32 discriminant(b*b - a*c);

		if(discriminant.LT(zero))
			return false;

		const HF32 sqrt(discriminant.Sqrt());
		const HF32 aRecip(a.Reciprocal());
		const HF32 t1((-b-sqrt)*aRecip);
		const HF32 t2((-b+sqrt)*aRecip);

		/*const Scalar_Bool con(t1.LT(zero));
		const HF32 t(Select(con, t2, t1));

		HVec3 contact(ray.m_start + n*t);*/

		if(t1.LT(zero)) //the start point inside the infinite cylinder
		{
			HVec3 closest(Maths::UNDEFINED);
			Collision::ClosestPtPointSegment(ray.m_start, capsule.Start(), capsule.End(), closest);
			const HVec3 dir(ray.m_start-closest);
			const HF32 length2(dir.Length2());
			if(length2.GT(r2))//the start point outside the capsule
			{
		
				const HVec3 sc(ray.m_start - closest);
				const HF32 a(n|n);
				const HF32 b(sc|n);
				const HF32 c((sc|sc)-rr);
				const HF32 discriminant(b*b-a*c);
				//Check whether the ray and the sphere intersect
				if(discriminant.LT(zero))
					return false;
				const HF32 t((-b-discriminant.Sqrt())/a);
				const HVec3 contact(ray.m_start + t*n);

				C_RayTracingResult& result=ray.m_result;
				result.m_contact=contact;
				
				const HVec3 v(result.m_contact - capsule.Start());
				const HF32  vd(v|d);
				const HF32  scale(vd/dd);
				const HVec3 w(d*scale);
				result.m_normal=C_Normal(v-w);
				result.m_TriangleNormal = result.m_normal;
				result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
				result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
				result.m_distance=(result.m_contact - ray.m_start).Length();
				result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
				result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
				ray.m_end=contact;
				result.m_UserData = shape->m_UserData;

				return true;
			}
			
		}

		HVec3 contact(ray.m_start + n*t1);

		HVec3 closest(Maths::UNDEFINED);
		Collision::ClosestPtPointSegment(contact, capsule.Start(), capsule.End(), closest);

		const HVec3 dir(contact-closest);
		const HF32 length2(dir.Length2());
		if(length2.GT(r2))
		{
			const HVec3 sc(ray.m_start - closest);
			const HF32 a(n|n);
			const HF32 b(sc|n);
			const HF32 c((sc|sc)-rr);
			const HF32 discriminant(b*b-a*c);
			if(discriminant.LT(zero))
				return false;
			const HF32 t((-b-discriminant.Sqrt())/a);
			contact=ray.m_start + t*n;
			normal = C_Normal(dir);
		}
		else
		{
			
			const HVec3 v(contact - capsule.Start());
			const HF32  vd(v|d);
			const HF32  scale(vd/dd);
			const HVec3 w(d*scale);
			normal=C_Normal(v-w);
		}

		C_RayTracingResult& result=ray.m_result;
		result.m_contact=contact;

		result.m_normal =normal;
		result.m_TriangleNormal = result.m_normal;
		result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
		result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
		result.m_distance=(result.m_contact - ray.m_start).Length();
		result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
		result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
		result.m_UserData = shape->m_UserData;
		ray.m_end=contact;
		
		return true;
	}

	bool GJKRayCastCylinder(const C_Shape* const __restrict shape, C_Ray& ray)
	{
		using namespace Maths;
		C_CylinderShape* pCylinderShape = (C_CylinderShape*)shape;
		C_Cylinder& cylinder=pCylinderShape->m_Cylinder;

		const HVec3 r(ray.m_end - ray.m_start);
		const HF32 height = r.Length();
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());

		C_Sphere sphere;
		sphere.Centre(ray.m_start);
		sphere.Radius(ray.m_Radius);
		
		HF32 lambda(UNDEFINED);
		HVec3 normal(UNDEFINED), closestA(UNDEFINED), closestB(UNDEFINED);
		if(GJKRayCast<C_Sphere, C_Cylinder>(sphere, cylinder, -r, one, lambda,  normal, closestA, closestB))
		{
			const HVec3 contact(ray.m_start + lambda*r);
			const C_Normal norm(closestA - ray.m_start);

			ray.m_result.m_contact=contact;
			ray.m_result.m_normal = C_Normal(norm, C_Normal::UNIT_LENGTH_VECTOR);
			ray.m_result.m_TriangleNormal = ray.m_result.m_normal;
			ray.m_result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
			ray.m_result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
			ray.m_result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
			ray.m_result.m_distance=(contact-ray.m_start)|ray.m_Direction;
			ray.m_result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
			ray.m_result.m_TriangleIndex = -1;
			ray.m_result.m_UserData = pCylinderShape->m_UserData;
			ray.m_end=contact;
			return true;
		}
		return false;
	}

	bool GJKRayCastConvex(const C_Shape* const __restrict shape, C_Ray& ray)
	{
		using namespace Maths;
		Spu::MemBaseMarker marker;
		C_ConvexHullShape* pConvexShape = (C_ConvexHullShape*)shape;
		C_ConvexHull& convexHull=pConvexShape->m_Hull;
		Collision::C_SpuConvexHull spuConvex(convexHull);

		const HVec3 r(ray.m_end - ray.m_start);
		const HF32 height = r.Length();
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());

		C_Sphere sphere;
		sphere.Centre(ray.m_start);
		sphere.Radius(ray.m_Radius);
		
		HF32 lambda(UNDEFINED);
		HVec3 normal(UNDEFINED), closestA(UNDEFINED), closestB(UNDEFINED);
		if(GJKRayCast<C_Sphere, Collision::C_SpuConvexHull>(sphere, spuConvex, -r, one, lambda,  normal, closestA, closestB))
		{
			const HVec3 contact(sphere.Centre() + lambda*r);
			const C_Normal norm(closestA - sphere.Centre());

			ray.m_result.m_contact=contact;
			ray.m_result.m_normal = C_Normal(norm, C_Normal::UNIT_LENGTH_VECTOR);
			ray.m_result.m_TriangleNormal = ray.m_result.m_normal;
			ray.m_result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
			ray.m_result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
			ray.m_result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
			ray.m_result.m_distance=(contact-ray.m_start)|ray.m_Direction;
			ray.m_result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
			ray.m_result.m_TriangleIndex = -1;
			ray.m_result.m_UserData = pConvexShape->m_UserData;
			ray.m_end=contact;
			return true;
		}
		return false;
	}

	bool GJKRayCastCone(const C_Shape* const __restrict shape, C_Ray& ray)
	{
		using namespace Maths;
		C_ConeShape* pConeShape = (C_ConeShape*)shape;
		C_Cone& cone=pConeShape->m_Cone;

		const HVec3 r(ray.m_end - ray.m_start);
		const HF32 height = r.Length();
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());

		C_Sphere sphere;
		sphere.Centre(ray.m_start);
		sphere.Radius(ray.m_Radius);
		
		HF32 lambda(UNDEFINED);
		HVec3 normal(UNDEFINED), closestA(UNDEFINED), closestB(UNDEFINED);
		if(GJKRayCast<C_Sphere, C_Cone>(sphere, cone, -r, one, lambda,  normal, closestA, closestB))
		{
			const HVec3 contact(sphere.Centre() + lambda*r);
			const C_Normal norm(closestA - sphere.Centre());

			ray.m_result.m_contact=contact;
			ray.m_result.m_normal = C_Normal(norm, C_Normal::UNIT_LENGTH_VECTOR);
			ray.m_result.m_TriangleNormal = ray.m_result.m_normal;
			ray.m_result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
			ray.m_result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
			ray.m_result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
			ray.m_result.m_distance=(contact-ray.m_start)|ray.m_Direction;
			ray.m_result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
			ray.m_result.m_TriangleIndex = -1;
			ray.m_result.m_UserData = pConeShape->m_UserData;
			ray.m_end=contact;
			return true;
		}
		return false;
	}

	bool GJKRayCastBox(const C_Shape* const __restrict shape, C_Ray& ray)
	{
		using namespace Maths;
		C_BoxShape* pBoxShape = (C_BoxShape*)shape;
		C_OBB& obb=pBoxShape->m_Box;

		const HVec3 r(ray.m_end - ray.m_start);
		const HF32 height = r.Length();
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());

		C_Sphere sphere;
		sphere.Centre(ray.m_start);
		sphere.Radius(ray.m_Radius);
		
		HF32 lambda(UNDEFINED);
		HVec3 normal(UNDEFINED), closestA(UNDEFINED), closestB(UNDEFINED);
		if(GJKRayCast<C_Sphere, C_OBB>(sphere, obb, -r, one, lambda,  normal, closestA, closestB))
		{
			const HVec3 contact(sphere.Centre() + lambda*r);
			const C_Normal norm(closestA - sphere.Centre());

			ray.m_result.m_contact=contact;
			ray.m_result.m_normal = C_Normal(norm, C_Normal::UNIT_LENGTH_VECTOR);
			ray.m_result.m_TriangleNormal = ray.m_result.m_normal;
			ray.m_result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
			ray.m_result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
			ray.m_result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
			ray.m_result.m_distance=(contact-ray.m_start)|ray.m_Direction;
			ray.m_result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
			ray.m_result.m_TriangleIndex = -1;
			ray.m_result.m_UserData = pBoxShape->m_UserData;
			ray.m_end=contact;
			return true;
		}
		return false;
	}

	bool GJKRayCastSphere(const C_Shape* const __restrict shape, C_Ray& ray)
	{
		using namespace Maths;

		/*	C_SphereShape* pSphereShape = (C_SphereShape*)shape;
		C_Sphere& sphere1=pSphereShape->m_Sphere;

		const HVec3 r(ray.m_end - ray.m_start);
		const HF32 height = r.Length();
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());

		C_Sphere sphere0;
		sphere0.Centre(ray.m_start);
		sphere0.Radius(ray.m_Radius);
		
		HF32 lambda(UNDEFINED);
		HVec3 normal(UNDEFINED), closestA(UNDEFINED), closestB(UNDEFINED);
		if(GJKRayCast<C_Sphere, C_Sphere>(sphere0, sphere1, -r, one, lambda,  normal, closestA, closestB))
		{
			const HVec3 centre = sphere0.Centre();
			const HVec3 contact(centre + lambda*r);
			const C_Normal norm(closestA - centre);

			ray.m_result.m_contact=contact;
			ray.m_result.m_normal = C_Normal(norm, C_Normal::UNIT_LENGTH_VECTOR);
			ray.m_result.m_TriangleNormal = ray.m_result.m_normal;
			ray.m_result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
			ray.m_result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
			ray.m_result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
			ray.m_result.m_distance=(contact-ray.m_start)|ray.m_Direction;
			ray.m_result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
			ray.m_result.m_TriangleIndex = -1;
			ray.m_result.m_UserData = pSphereShape->m_UserData;
			ray.m_end=contact;
			return true;
		}
		return false;*/

		C_SphereShape* pSphereShape = (C_SphereShape*)shape;
		const HF32 zero(Constants<HF32>::Zero());
		C_Sphere& sphere=pSphereShape->m_Sphere;
		const HF32 radius(sphere.Radius() + ray.m_Radius);
		const HVec3 v(ray.m_end - ray.m_start);
		const HVec3 m(ray.m_start - sphere.Centre());
		const HF32 a(v|v);
		const HF32 b(m|v);
		const HF32 c((m|m)-(radius * radius));
		const HF32 discriminant(b*b - a*c);
		if(discriminant.LT(zero)) // not intersect
			return false;
		const HF32 sqrt(discriminant.Sqrt());
		const HF32 aRecip(a.Reciprocal());
		const HF32 t1((-b-sqrt)*aRecip);
		const HF32 t2((-b+sqrt)*aRecip);
		const Scalar_Bool con(t1.LT(zero));
		const HF32 t(Select(con, t2, t1));
		const HVec3 w(v*t);
		const HVec3 contact(ray.m_start + w);
		ray.m_result.m_distance=w.Length();
		ray.m_result.m_contact=contact;
		ray.m_result.m_normal=C_Normal(contact-sphere.Centre());
		ray.m_result.m_TriangleNormal = ray.m_result.m_normal;
		ray.m_result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
		ray.m_result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
		ray.m_result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
		ray.m_result.m_distance=(contact-ray.m_start)|ray.m_Direction;
		ray.m_result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
		ray.m_result.m_TriangleIndex = -1;
		ray.m_result.m_UserData = pSphereShape->m_UserData;
		ray.m_end=contact;
		return true;
	}

	bool GJKRayCastComposite(const C_Shape* const shape, C_Ray& ray, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache)
	{
		Spu::MemBaseMarker marker;
		C_CollisionFilter* pRayFilter = &ray.m_Filter;//(C_CollisionFilter*)Spu::FetchAlloc(ray.m_pFilter, sizeof(C_CollisionFilter));
		Collision::C_CompositeShape* composite=(C_CompositeShape*)shape;
		bool collide=false;

		Collision::C_CompositeShapeIterator iter(composite);
		while(iter.HasNextShape())
		{
			Spu::MemBaseMarker innerMarker;
			iter.NextShape();
			Collision::C_Shape* subShape=iter.GetCurrentShape<C_Shape>();
		
			if(pRayFilter->CanCollide(&subShape->m_Filter))
			{
				switch(subShape->GetShape())
				{
				case E_Box_Shape:
					collide= collide | GJKRayCastBox(subShape, ray);
					break;
				case E_ConvexHull_Shape:
					collide=collide | GJKRayCastConvex(subShape, ray);
					break;
				case E_Sphere_Shape:
					collide=collide | GJKRayCastSphere(subShape, ray);
					break;
				case E_Capsule_Shape:
					collide=collide | GJKRayCastCapsule(subShape, ray);
					break;
				case E_Cylinder_Shape:
					collide=collide | GJKRayCastCylinder(subShape, ray);
					break;
				case E_Cone_Shape:
					collide = collide | GJKRayCastCone(subShape, ray);
					break;
				case E_Composite_Shape:
					collide = collide | GJKRayCastComposite(subShape, ray, stack, spuCache);
					break;
				case E_InstancedTriangleMesh_Shape:
					{
						C_InstancedTriangleMeshShape* pShape = (C_InstancedTriangleMeshShape*)subShape;//Spu::FetchAlloc(pShape, sizeof(C_InstancedTriangleMeshShape));
						if(pShape->m_AABBTree.RayTracing<CollideFatRayTriangleMesh>(ray, stack, spuCache))
						{
							collide = true;
							ray.m_result.m_pShape = pShape->m_pPpuPtr;
						}
						break;
					}
				
				default:
					BIZ_ASSERT_MSG(0, "Undefined collision primitive for raytrace!");
					break;
				}
			}
		}
		return collide;
	}

	bool GJKRayCastCapsule(const C_Shape* const __restrict shape, C_TransparentRay& ray, SharedPhysics::C_GraphNode* pGraphNode)
	{
		/*using namespace Maths;
		C_CapsuleShape* pCapsuleShape = (C_CapsuleShape*)shape;
		C_Capsule& capsule=pCapsuleShape->m_Capsule;

		const HVec3 r(ray.m_end - ray.m_start);
		const HF32 height = r.Length();
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());

		C_Sphere sphere;
		sphere.Centre(ray.m_start);
		sphere.Radius(ray.m_Radius);
		
		HF32 lambda(UNDEFINED);
		HVec3 normal(UNDEFINED), closestA(UNDEFINED), closestB(UNDEFINED);
		if(GJKRayCast<C_Sphere, C_Capsule>(sphere, capsule, -r, one, lambda,  normal, closestA, closestB))
		{
			const HVec3 contact(ray.m_start + lambda*r);
			const C_Normal norm(closestA - ray.m_start);

			Collision::C_RayTracingResult result;

			result.m_contact=contact;
			result.m_normal = C_Normal(norm, C_Normal::UNIT_LENGTH_VECTOR);
			result.m_TriangleNormal = result.m_normal;
			result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
			result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
			result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
			result.m_distance=(contact-ray.m_start)|ray.m_Direction;
			result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
			result.m_TriangleIndex = -1;
			result.m_UserData = pCapsuleShape->m_UserData;
			result.m_pNode = pGraphNode;
			ray.StoreResult(result);

			return true;
		}
		return false;*/

		using namespace Maths;
		Spu::MemBaseMarker marker;
		C_CapsuleShape* pCapsuleShape = (C_CapsuleShape*)shape;
		const HF32 zero(Constants<HF32>::Zero());
		C_Capsule& capsule=pCapsuleShape->m_Capsule;
		const HVec3 m(ray.m_start - capsule.Start());
		const HVec3 n(ray.m_end - ray.m_start);
		const HVec3 d(capsule.End() - capsule.Start());
		const HF32 r(capsule.Radius() + ray.m_Radius);
		const HF32 r2(r * r);

		const HVec3 dn(d^n);
		const HVec3 dm(d^m);
		const HF32 dd(d|d);
		const HF32 rr(r*r);

		const HF32 a(dn|dn);
		const HF32 b(dm|dn);
		const HF32 c((dm|dm) - rr*dd);
		C_Normal normal(UNDEFINED);

		const HF32 discriminant(b*b - a*c);

		if(discriminant.LT(zero))
			return false;

		const HF32 sqrt(discriminant.Sqrt());
		const HF32 aRecip(a.Reciprocal());
		const HF32 t1((-b-sqrt)*aRecip);
		const HF32 t2((-b+sqrt)*aRecip);

		/*const Scalar_Bool con(t1.LT(zero));
		const HF32 t(Select(con, t2, t1));

		HVec3 contact(ray.m_start + n*t);*/

		if(t1.LT(zero)) //the start point inside the infinite cylinder
		{
			HVec3 closest(Maths::UNDEFINED);
			Collision::ClosestPtPointSegment(ray.m_start, capsule.Start(), capsule.End(), closest);
			const HVec3 dir(ray.m_start-closest);
			const HF32 length2(dir.Length2());
			if(length2.GT(r2))//the start point outside the capsule
			{
		
				const HVec3 sc(ray.m_start - closest);
				const HF32 a(n|n);
				const HF32 b(sc|n);
				const HF32 c((sc|sc)-rr);
				const HF32 discriminant(b*b-a*c);
				//Check whether the ray and the sphere intersect
				if(discriminant.LT(zero))
					return false;
				const HF32 t((-b-discriminant.Sqrt())/a);
				const HVec3 contact(ray.m_start + t*n);

				Collision::C_RayTracingResult result;
				result.m_contact=contact;
				
				const HVec3 v(result.m_contact - capsule.Start());
				const HF32  vd(v|d);
				const HF32  scale(vd/dd);
				const HVec3 w(d*scale);
				result.m_normal=C_Normal(v-w);
				result.m_TriangleNormal = result.m_normal;
				result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
				result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
				result.m_distance=(result.m_contact - ray.m_start).Length();
				result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
				result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
				ray.m_end=contact;
				result.m_UserData = shape->m_UserData;
				ray.StoreResult(result);

				return true;
			}
			
		}

		HVec3 contact(ray.m_start + n*t1);

		HVec3 closest(Maths::UNDEFINED);
		Collision::ClosestPtPointSegment(contact, capsule.Start(), capsule.End(), closest);

		const HVec3 dir(contact-closest);
		const HF32 length2(dir.Length2());
		if(length2.GT(r2))
		{
			const HVec3 sc(ray.m_start - closest);
			const HF32 a(n|n);
			const HF32 b(sc|n);
			const HF32 c((sc|sc)-rr);
			const HF32 discriminant(b*b-a*c);
			if(discriminant.LT(zero))
				return false;
			const HF32 t((-b-discriminant.Sqrt())/a);
			contact=ray.m_start + t*n;
			normal = C_Normal(dir);
		}
		else
		{
			
			const HVec3 v(contact - capsule.Start());
			const HF32  vd(v|d);
			const HF32  scale(vd/dd);
			const HVec3 w(d*scale);
			normal=C_Normal(v-w);
		}

		Collision::C_RayTracingResult result;
		result.m_contact=contact;

		result.m_normal =normal;
		result.m_TriangleNormal = result.m_normal;
		result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
		result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
		result.m_distance=(result.m_contact - ray.m_start).Length();
		result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
		result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
		result.m_UserData = shape->m_UserData;
		ray.m_end=contact;
		ray.StoreResult(result);
		
		return true;

	}

	bool GJKRayCastCylinder(const C_Shape* const __restrict shape, C_TransparentRay& ray, SharedPhysics::C_GraphNode* pGraphNode)
	{
		using namespace Maths;
		C_CylinderShape* pCylinderShape = (C_CylinderShape*)shape;
		C_Cylinder& cylinder=pCylinderShape->m_Cylinder;

		const HVec3 r(ray.m_end - ray.m_start);
		const HF32 height = r.Length();
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());

		C_Sphere sphere;
		sphere.Centre(ray.m_start);
		sphere.Radius(ray.m_Radius);
		
		HF32 lambda(UNDEFINED);
		HVec3 normal(UNDEFINED), closestA(UNDEFINED), closestB(UNDEFINED);
		if(GJKRayCast<C_Sphere, C_Cylinder>(sphere, cylinder, -r, one, lambda,  normal, closestA, closestB))
		{
			const HVec3 contact(ray.m_start + lambda*r);
			const C_Normal norm(closestA - ray.m_start);

			Collision::C_RayTracingResult result;

			result.m_contact=contact;
			result.m_normal = C_Normal(norm, C_Normal::UNIT_LENGTH_VECTOR);
			result.m_TriangleNormal = result.m_normal;
			result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
			result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
			result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
			result.m_distance=(contact-ray.m_start)|ray.m_Direction;
			result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
			result.m_TriangleIndex = -1;
			result.m_UserData = pCylinderShape->m_UserData;
			result.m_pNode = pGraphNode;
			ray.StoreResult(result);

			return true;
		}
		return false;
	}

	bool GJKRayCastConvex(const C_Shape* const __restrict shape, C_TransparentRay& ray, SharedPhysics::C_GraphNode* pGraphNode)
	{
		using namespace Maths;
		C_ConvexHullShape* pConvexShape = (C_ConvexHullShape*)shape;
		C_ConvexHull& convexHull=pConvexShape->m_Hull;
		C_SpuConvexHull spuHull(convexHull);

		const HVec3 r(ray.m_end - ray.m_start);
		const HF32 height = r.Length();
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());

		C_Sphere sphere;
		sphere.Centre(ray.m_start);
		sphere.Radius(ray.m_Radius);
		
		HF32 lambda(UNDEFINED);
		HVec3 normal(UNDEFINED), closestA(UNDEFINED), closestB(UNDEFINED);
		if(GJKRayCast<C_Sphere, C_SpuConvexHull>(sphere, spuHull, -r, one, lambda,  normal, closestA, closestB))
		{
			const HVec3 contact(ray.m_start + lambda*r);
			const C_Normal norm(closestA - ray.m_start);

			Collision::C_RayTracingResult result;

			result.m_contact=contact;
			result.m_normal = C_Normal(norm, C_Normal::UNIT_LENGTH_VECTOR);
			result.m_TriangleNormal = result.m_normal;
			result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
			result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
			result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
			result.m_distance=(contact-ray.m_start)|ray.m_Direction;
			result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
			result.m_TriangleIndex = -1;
			result.m_UserData = pConvexShape->m_UserData;
			result.m_pNode = pGraphNode;
			ray.StoreResult(result);

			return true;
		}
		return false;
	}

	bool GJKRayCastCone(const C_Shape* const __restrict shape, C_TransparentRay& ray, SharedPhysics::C_GraphNode* pGraphNode)
	{
		using namespace Maths;
		C_ConeShape* pConeShape = (C_ConeShape*)shape;
		C_Cone& cone=pConeShape->m_Cone;

		const HVec3 r(ray.m_end - ray.m_start);
		const HF32 height = r.Length();
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());

		C_Sphere sphere;
		sphere.Centre(ray.m_start);
		sphere.Radius(ray.m_Radius);
		
		HF32 lambda(UNDEFINED);
		HVec3 normal(UNDEFINED), closestA(UNDEFINED), closestB(UNDEFINED);
		if(GJKRayCast<C_Sphere, C_Cone>(sphere, cone, -r, one, lambda,  normal, closestA, closestB))
		{
			const HVec3 contact(ray.m_start + lambda*r);
			const C_Normal norm(closestA - ray.m_start);

			Collision::C_RayTracingResult result;

			result.m_contact=contact;
			result.m_normal = C_Normal(norm, C_Normal::UNIT_LENGTH_VECTOR);
			result.m_TriangleNormal = result.m_normal;
			result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
			result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
			result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
			result.m_distance=(contact-ray.m_start)|ray.m_Direction;
			result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
			result.m_TriangleIndex = -1;
			result.m_UserData = pConeShape->m_UserData;
			result.m_pNode = pGraphNode;
			ray.StoreResult(result);

			return true;
		}
		return false;
	}

	bool GJKRayCastBox(const C_Shape* const __restrict shape, C_TransparentRay& ray, SharedPhysics::C_GraphNode* pGraphNode)
	{
		using namespace Maths;
		C_BoxShape* pBoxShape = (C_BoxShape*)shape;
		C_OBB& obb=pBoxShape->m_Box;

		const HVec3 r(ray.m_end - ray.m_start);
		const HF32 height = r.Length();
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());

		C_Sphere sphere;
		sphere.Centre(ray.m_start);
		sphere.Radius(ray.m_Radius);
		
		HF32 lambda(UNDEFINED);
		HVec3 normal(UNDEFINED), closestA(UNDEFINED), closestB(UNDEFINED);
		if(GJKRayCast<C_Sphere, C_OBB>(sphere, obb, -r, one, lambda,  normal, closestA, closestB))
		{
			const HVec3 contact(ray.m_start + lambda*r);
			const C_Normal norm(closestA - ray.m_start);

			Collision::C_RayTracingResult result;

			result.m_contact=contact;
			result.m_normal = C_Normal(norm, C_Normal::UNIT_LENGTH_VECTOR);
			result.m_TriangleNormal = result.m_normal;
			result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
			result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
			result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
			result.m_distance=(contact-ray.m_start)|ray.m_Direction;
			result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
			result.m_TriangleIndex = -1;
			result.m_UserData = pBoxShape->m_UserData;
			result.m_pNode = pGraphNode;
			ray.StoreResult(result);

			return true;
		}
		return false;
	}

	bool GJKRayCastSphere(const C_Shape* const __restrict shape, C_TransparentRay& ray, SharedPhysics::C_GraphNode* pGraphNode)
	{
		using namespace Maths;
		/*C_SphereShape* pSphereShape = (C_SphereShape*)shape;
		C_Sphere& sphere1=pSphereShape->m_Sphere;

		const HVec3 r(ray.m_end - ray.m_start);
		const HF32 height = r.Length();
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());

		C_Sphere sphere0;
		sphere0.Centre(ray.m_start);
		sphere0.Radius(ray.m_Radius);
		
		HF32 lambda(UNDEFINED);
		HVec3 normal(UNDEFINED), closestA(UNDEFINED), closestB(UNDEFINED);
		if(GJKRayCast<C_Sphere, C_Sphere>(sphere0, sphere1, -r, one, lambda,  normal, closestA, closestB))
		{
			const HVec3 contact(ray.m_start + lambda*r);
			const C_Normal norm(closestA - ray.m_start);

			Collision::C_RayTracingResult result;

			result.m_contact=contact;
			result.m_normal = C_Normal(norm, C_Normal::UNIT_LENGTH_VECTOR);
			result.m_TriangleNormal = result.m_normal;
			result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
			result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
			result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
			result.m_distance=(contact-ray.m_start)|ray.m_Direction;
			result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
			result.m_TriangleIndex = -1;
			result.m_UserData = pSphereShape->m_UserData;
			result.m_pNode = pGraphNode;
			ray.StoreResult(result);

			return true;
		}
		return false;*/

		C_SphereShape* pSphereShape = (C_SphereShape*)shape;
		const HF32 zero(Constants<HF32>::Zero());
		C_Sphere& sphere=pSphereShape->m_Sphere;
		const HF32 radius(sphere.Radius() + ray.m_Radius);
		const HVec3 v(ray.m_end - ray.m_start);
		const HVec3 m(ray.m_start - sphere.Centre());
		const HF32 a(v|v);
		const HF32 b(m|v);
		const HF32 c((m|m)-(radius * radius));
		const HF32 discriminant(b*b - a*c);
		if(discriminant.LT(zero)) // not intersect
			return false;
		const HF32 sqrt(discriminant.Sqrt());
		const HF32 aRecip(a.Reciprocal());
		const HF32 t1((-b-sqrt)*aRecip);
		const HF32 t2((-b+sqrt)*aRecip);
		const Scalar_Bool con(t1.LT(zero));
		const HF32 t(Select(con, t2, t1));
		const HVec3 w(v*t);
		const HVec3 contact(ray.m_start + w);

		Collision::C_RayTracingResult result;

		result.m_distance=w.Length();
		result.m_contact=contact;
		result.m_normal=C_Normal(contact-sphere.Centre());
		result.m_TriangleNormal = result.m_normal;
		result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
		result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
		result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
		result.m_distance=(contact-ray.m_start)|ray.m_Direction;
		result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
		result.m_TriangleIndex = -1;
		result.m_UserData = pSphereShape->m_UserData;
		ray.StoreResult(result);
		ray.m_end=contact;
		return true;

	}

	bool GJKRayCastComposite(const C_Shape* const shape, C_TransparentRay& ray, SharedPhysics::C_SafeStack<true>& stack, C_CollisionFilter* pRayFilter, SharedPhysics::C_SmallSPUCache& spuCache,
		SharedPhysics::C_GraphNode* pGraphNode)
	{
		Spu::MemBaseMarker marker;
		Collision::C_CompositeShape* composite=(C_CompositeShape*)shape;
		bool collide=false;

		Collision::C_CompositeShapeIterator iter(composite);
		while(iter.HasNextShape())
		{
			Spu::MemBaseMarker innerMarker;
			iter.NextShape();
			Collision::C_Shape* subShape= iter.GetCurrentShape<C_Shape>();
		
			if(pRayFilter->CanCollide(&subShape->m_Filter))
			{
				switch(subShape->GetShape())
				{
				case E_Box_Shape:
					collide= collide | GJKRayCastBox(subShape, ray, pGraphNode);
					break;
				case E_ConvexHull_Shape:
						collide = collide | GJKRayCastConvex(subShape, ray, pGraphNode);
					break;
				case E_Sphere_Shape:
					collide=collide | GJKRayCastSphere(subShape, ray, pGraphNode);
					break;
				case E_Capsule_Shape:
					collide=collide | GJKRayCastCapsule(subShape, ray, pGraphNode);
					break;
				case E_Cylinder_Shape:
					collide=collide | GJKRayCastCylinder(subShape, ray, pGraphNode);
					break;
				case E_Cone_Shape:
					collide = collide | GJKRayCastCone(subShape, ray, pGraphNode);
					break;
				case E_Composite_Shape:
					collide = collide | GJKRayCastComposite(subShape, ray, stack, pRayFilter, spuCache, pGraphNode);
					break;
				case E_InstancedTriangleMesh_Shape:
					{
						C_InstancedTriangleMeshShape* pShape = (C_InstancedTriangleMeshShape*)subShape;//Spu::FetchAlloc(wrapper->m_pShape, sizeof(C_InstancedTriangleMeshShape));
						if(pShape->m_AABBTree.RayTracing<CollideFatRayTriangleMesh>(ray, pRayFilter, pShape->m_pPpuPtr, pGraphNode,  stack, spuCache))
						{
							collide = true;
						}
						break;
					}
				default:
					BIZ_ASSERT_MSG(0, "Undefined collision primitive for raytrace!");
					break;
				}
			}
		}
		return collide;
	}
}
