#ifndef CAPSULERAY_INL
#define CAPSULERAY_INL

#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/Primitive.h"
#include "gravity/narrowphase/PrimitiveTest.h"

namespace Collision
{
NOALIAS inline bool CollideCapsuleRay(const C_Shape* const __restrict shape, C_Ray& ray)
{
	using namespace Maths;
	Spu::MemBaseMarker marker;
	C_CapsuleShape* pCapsuleShape = (C_CapsuleShape*)shape;
	const HF32 zero(Constants<HF32>::Zero());
	C_Capsule& capsule=pCapsuleShape->m_Capsule;
	const HVec3 m(ray.m_start - capsule.Start());
	const HVec3 n(ray.m_end - ray.m_start);
	const HVec3 d(capsule.End() - capsule.Start());
	const HF32 r(capsule.Radius());

	const HVec3 dn(d^n);
	const HVec3 dm(d^m);
	const HF32 dd(d|d);
	const HF32 rr(r*r);

	const HF32 a(dn|dn);
	const HF32 b(dm|dn);
	const HF32 c((dm|dm) - rr*dd);
	C_Normal normal(UNDEFINED);

	/*const Maths::HF32 dd(d|d);
	const Maths::HF32 nn(n|n);
	const Maths::HF32 mn(m|n);
	const Maths::HF32 mm(m|m);
	const Maths::HF32 nd(n|d);
	const Maths::HF32 md(m|d);
	const Maths::HF32 rr(capsule.r*capsule.r);

	const Maths::HF32 a(dd*nn - nd*nd);
	const Maths::HF32 b(dd*mn - nd*md);
	const Maths::HF32 c(dd*(mm - rr) - md*md);*/

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
		if(length2.GT(rr))//the start point outside the capsule
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
			result.m_normal=C_Normal(Normalise(v-w));
			ray.m_result.m_TriangleNormal = result.m_normal;
			ray.m_result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
			ray.m_result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
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
	if(length2.GT(rr))
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

	/*C_RayTracingResult& result=ray.m_result;
	const Maths::HVec3 contact(ray.m_start + t*n);
	result.m_contact=contact;
	
	const Maths::HVec3 v(result.m_contact - capsule.m_Start);
	const Maths::HF32  vd(v|d);
	const Maths::HF32  scale(vd/dd);
	const Maths::HVec3 w(d*scale);
	result.m_normal=C_Normal(Normalise(v-w));
	result.m_distance=(result.m_contact - ray.m_start).Length();
	ray.m_end=contact;
	result.m_pShape = const_cast<C_Shape*>(shape);
	return true;
	*/

	C_RayTracingResult& result=ray.m_result;
	result.m_contact=contact;
	
	/*const HVec3 v(result.m_contact - capsule.m_Start);
	const HF32  vd(v|d);
	const HF32  scale(vd/dd);
	const HVec3 w(d*scale);
	result.m_normal=C_Normal(Normalise(v-w));*/
	result.m_normal =normal;
	ray.m_result.m_TriangleNormal = result.m_normal;
	ray.m_result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
	ray.m_result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
	result.m_distance=(result.m_contact - ray.m_start).Length();
	result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
	result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
	result.m_UserData = shape->m_UserData;
	ray.m_end=contact;
	
	return true;
}

NOALIAS inline bool CollideCapsuleRay(const C_Shape* const __restrict shape, C_TransparentRay& ray,
									  SharedPhysics::C_GraphNode* pGraphNode)
{
	using namespace Maths;
	Spu::MemBaseMarker marker;
	C_CapsuleShape* pCapsuleShape = (C_CapsuleShape*)shape;
	const HF32 zero(Constants<HF32>::Zero());
	C_Capsule& capsule=pCapsuleShape->m_Capsule;
	const HVec3 m(ray.m_start - capsule.Start());
	const HVec3 n(ray.m_end - ray.m_start);
	const HVec3 d(capsule.End() - capsule.Start());
	const HF32 r(capsule.Radius());

	const HVec3 dn(d^n);
	const HVec3 dm(d^m);
	const HF32 dd(d|d);
	const HF32 rr(r*r);

	const HF32 a(dn|dn);
	const HF32 b(dm|dn);
	const HF32 c((dm|dm) - rr*dd);

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
		if(length2.GT(rr))//the start point outside the capsule
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
			HVec3 contact(ray.m_start + t*n);

			C_RayTracingResult result;
			result.m_contact = contact;

			const HVec3 v(result.m_contact - capsule.Start());
			const HF32  vd(v|d);
			const HF32  scale(vd/dd);
			const HVec3 w(d*scale);
			result.m_normal=C_Normal(v-w);
			result.m_TriangleNormal = result.m_normal;
			result.m_PhysicsMaterialIndex =shape->m_PhysicsMaterialIndex;
			result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;

			result.m_distance=(result.m_contact - ray.m_start).Length();
			result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
			result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
			result.m_UserData = shape->m_UserData;
			result.m_pNode = pGraphNode;
			ray.StoreResult(result);

			return true;
		}
		
	}

	HVec3 contact(ray.m_start + n*t1);

	HVec3 closest(Maths::UNDEFINED);
	Collision::ClosestPtPointSegment(contact, capsule.Start(), capsule.End(), closest);

	const HVec3 dir(contact-closest);
	const HF32 length2(dir.Length2());
	if(length2.GT(rr))
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
		
	}

	
	C_RayTracingResult result;
	result.m_contact = contact;

	const HVec3 v(result.m_contact - capsule.Start());
	const HF32  vd(v|d);
	const HF32  scale(vd/dd);
	const HVec3 w(d*scale);
	result.m_normal=C_Normal(v-w);
	result.m_TriangleNormal = result.m_normal;
	result.m_PhysicsMaterialIndex =shape->m_PhysicsMaterialIndex;
	result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;

	/*ray.m_result.m_TriangleNormal = result.m_normal;
	ray.m_result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
	ray.m_result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;*/
	result.m_distance=(result.m_contact - ray.m_start).Length();
	result.m_pShape = const_cast<C_Shape*>(shape);
	result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
	result.m_UserData = shape->m_UserData;
	//ray.m_end=contact;
	result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);

	ray.StoreResult(result);
	return true;
}


NOALIAS FORCE_INLINE bool CollideCapsuleRay(const C_Capsule* const __restrict capsule, const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, Maths::HVec3& contact)
{
	using namespace Maths;
	Spu::MemBaseMarker marker;
	C_Capsule* pCapsule = (C_Capsule*)capsule;//Spu::FetchAlloc(capsule, sizeof(C_Capsule));
	const HF32 zero(Constants<HF32>::Zero());
	const HVec3 m(start - capsule->Start());
	const HVec3 n(end - start);
	const HVec3 d(capsule->End() - capsule->Start());
	const HF32 r(capsule->Radius());

	const HVec3 dn(d^n);
	const HVec3 dm(d^m);
	const HF32 dd(d|d);
	const HF32 rr(r*r);

	const HF32 a(dn|dn);
	const HF32 b(dm|dn);
	const HF32 c((dm|dm) - rr*dd);


	const HF32 discriminant(b*b - a*c);

	if(discriminant.LT(zero))
		return false;

	const HF32 sqrt(discriminant.Sqrt());
	const HF32 aRecip(a.Reciprocal());
	const HF32 t1((-b-sqrt)*aRecip);
	const HF32 t2((-b+sqrt)*aRecip);


	//if(t1.LT(zero)) //the start point inside the infinite cylinder
	//{
	//	HVec3 closest(Maths::UNDEFINED);
	//	Collision::ClosestPtPointSegment(start, capsule->Start(), capsule->End(), closest);
	//	const HVec3 dir(start-closest);
	//	const HF32 length(dir.Length());
	//	if(length.GT(r))//the start point outside the capsule
	//	{
	//
	//		const HVec3 sc(start - closest);
	//		const HF32 a(n|n);
	//		const HF32 b(sc|n);
	//		const HF32 c((sc|sc)-rr);
	//		const HF32 discriminant(b*b-a*c);
	//		//Check whether the ray and the sphere intersect
	//		if(discriminant.LT(zero))
	//			return false;
	//		const HF32 t((-b-discriminant.Sqrt())/a);
	//		contact=start + t*n;
	//		return true;
	//	}
	//	
	//}

	contact = start + n*t1;


	HVec3 closest(Maths::UNDEFINED);
	Collision::ClosestPtPointSegment(contact, capsule->Start(), capsule->End(), closest);

	const HVec3 dir(contact-closest);
	const HF32 length2(dir.Length2());
	//the ray hit in one of the end hemishpere fo the capsule
	if(length2.GT(rr))
	{
		//Calculate the contact between the ray and the sphere if they are intersect
		const HVec3 sc(start - closest);
		const HF32 a(n|n);
		const HF32 b(sc|n);
		const HF32 c((sc|sc)-rr);
		const HF32 discriminant(b*b-a*c);
		//Check whether the ray and the sphere intersect
		if(discriminant.LT(zero))
			return false;
		const HF32 t((-b-discriminant.Sqrt())/a);
		contact=start + t*n;
		/*const C_Normal dir(start - closest);
		contact = closest + dir * capsule->m_Radius;*/
	}
	return true;
}

NOALIAS FORCE_INLINE bool CollideCapsuleRay(const C_Capsule* const __restrict capsule, const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, Maths::HVec3& contact1, Maths::HVec3& contact2)
{
	using namespace Maths;
	Spu::MemBaseMarker marker;
	C_Capsule* pCapsule = (C_Capsule*)capsule;//Spu::FetchAlloc(capsule, sizeof(C_Capsule));
	const HF32 zero(Constants<HF32>::Zero());
	const HVec3 m(start - capsule->Start());
	const HVec3 n(end - start);
	const HVec3 d(capsule->End() - capsule->Start());
	const HF32 r(capsule->Radius());

	const HVec3 dn(d^n);
	const HVec3 dm(d^m);
	const HF32 dd(d|d);
	const HF32 rr(r*r);

	const HF32 a(dn|dn);
	const HF32 b(dm|dn);
	const HF32 c((dm|dm) - rr*dd);


	const HF32 discriminant(b*b - a*c);

	if(discriminant.LT(zero))
		return false;

	const HF32 sqrt(discriminant.Sqrt());
	const HF32 aRecip(a.Reciprocal());
	const HF32 t1((-b-sqrt)*aRecip);
	const HF32 t2((-b+sqrt)*aRecip);


	//if(t1.LT(zero)) //the start point inside the infinite cylinder
	//{
	//	HVec3 closest(Maths::UNDEFINED);
	//	Collision::ClosestPtPointSegment(start, capsule->Start(), capsule->End(), closest);
	//	const HVec3 dir(start-closest);
	//	const HF32 length(dir.Length());
	//	if(length.GT(r))//the start point outside the capsule
	//	{
	//
	//		const HVec3 sc(start - closest);
	//		const HF32 a(n|n);
	//		const HF32 b(sc|n);
	//		const HF32 c((sc|sc)-rr);
	//		const HF32 discriminant(b*b-a*c);
	//		//Check whether the ray and the sphere intersect
	//		if(discriminant.LT(zero))
	//			return false;
	//		const HF32 t((-b-discriminant.Sqrt())/a);
	//		contact1=start;
	//		contact2=start + t*n;
	//		return true;
	//	}
	//	
	//}

	contact1 = start + n*t1;
	contact2 = start + n*t2;


	HVec3 closest(Maths::UNDEFINED);
	Collision::ClosestPtPointSegment(contact1, capsule->Start(), capsule->End(), closest);

	const HVec3 dir1(contact1-closest);
	//const HF32 length1(dir1.Length());
	//the ray hit in one of the end hemishpere fo the capsule
	if((dir1|dir1).GT(rr))
	{
		//Calculate the contact between the ray and the sphere if they are intersect
		const HVec3 sc(start - closest);
		const HF32 a(n|n);
		const HF32 b(sc|n);
		const HF32 c((sc|sc)-rr);
		const HF32 discriminant(b*b-a*c);
		//Check whether the ray and the sphere intersect
		if(discriminant.LT(zero))
			return false;
		const HF32 t((-b-discriminant.Sqrt())/a);
		contact1=start + t*n;
		/*const C_Normal dir(start - closest);
		contact = closest + dir * capsule->m_Radius;*/
	}

	Collision::ClosestPtPointSegment(contact2, capsule->Start(), capsule->End(), closest);
	const HVec3 dir2(contact2 - closest);
	if((dir2|dir2).GT(rr))
	{
		//Calculate the contact between the ray and the sphere if they are intersect
		const HVec3 v(-n);
		const HVec3 sc(end - closest);
		const HF32 a(v|v);
		const HF32 b(sc|v);
		const HF32 c((sc|sc)-rr);
		const HF32 discriminant(b*b-a*c);
		//Check whether the ray and the sphere intersect
		if(discriminant.LT(zero))
			return false;
		const HF32 t((-b-discriminant.Sqrt())/a);
		contact2=end + t*v;
	}
	
	return true;
}

NOALIAS FORCE_INLINE bool CollideCapsuleRay(const Maths::HVec3 PCREF capsuleA, const Maths::HVec3 PCREF capsuleB, const Maths::HF32 PCREF radius, const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, Maths::HVec3& hitPoint, Maths::C_Normal& normal)
{
	using namespace Maths;
	Spu::MemBaseMarker marker;
	const HF32 zero(Constants<HF32>::Zero());
	const HVec3 m(start - capsuleA);
	const HVec3 n(end - start);
	const HVec3 d(capsuleB - capsuleA);
	const HF32 r(radius);

	const HVec3 dn(d^n);
	const HVec3 dm(d^m);
	const HF32 dd(d|d);
	const HF32 rr(r*r);

	const HF32 a(dn|dn);
	const HF32 b(dm|dn);
	const HF32 c((dm|dm) - rr*dd);

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
		Collision::ClosestPtPointSegment(start, capsuleA, capsuleB, closest);
		const HVec3 dir(start-closest);
		const HF32 length2(dir.Length2());
		if(length2.GT(rr))//the start point outside the capsule
		{
	
			const HVec3 sc(start - closest);
			const HF32 a(n|n);
			const HF32 b(sc|n);
			const HF32 c((sc|sc)-rr);
			const HF32 discriminant(b*b-a*c);
			//Check whether the ray and the sphere intersect
			if(discriminant.LT(zero))
				return false;
			const HF32 t((-b-discriminant.Sqrt())/a);
			const HVec3 contact(start + t*n);

			hitPoint = contact;
			const HVec3 v(contact - capsuleA);
			const HF32  vd(v|d);
			const HF32  scale(vd/dd);
			const HVec3 w(d*scale);
			normal = v-w;
			return true;
		}
		
	}

	HVec3 contact(start + n*t1);

	HVec3 closest(Maths::UNDEFINED);
	Collision::ClosestPtPointSegment(contact, capsuleA, capsuleB, closest);

	const HVec3 dir(contact-closest);
	const HF32 length2(dir.Length2());
	if(length2.GT(rr))
	{
		const HVec3 sc(start - closest);
		const HF32 a(n|n);
		const HF32 b(sc|n);
		const HF32 c((sc|sc)-rr);
		const HF32 discriminant(b*b-a*c);
		if(discriminant.LT(zero))
			return false;
		const HF32 t((-b-discriminant.Sqrt())/a);
		contact=start + t*n;
		normal = dir;
	}
	else
	{
		
		const HVec3 v(contact - capsuleA);
		const HF32  vd(v|d);
		const HF32  scale(vd/dd);
		const HVec3 w(d*scale);
		normal=v-w;
	}
	hitPoint = contact;
	return true;
}


};

#endif
