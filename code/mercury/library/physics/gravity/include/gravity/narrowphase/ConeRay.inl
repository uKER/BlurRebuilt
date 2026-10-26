#ifndef CONE_INL
#define CONE_INL

#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/Primitive.h"
#include "gravity/narrowphase/PrimitiveTest.h"

namespace Collision
{

NOALIAS FORCE_INLINE bool CollideConeRay(const C_Cone* const __restrict cone, const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, Maths::HVec3& contact)
{
	using namespace Maths;
	Spu::MemBaseMarker marker;
	//C_ConeShape* pConeShape = (C_ConeShape*)Spu::FetchAlloc(shape, sizeof(C_ConeShape));
	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one(Constants<HF32>::One());
	const HF32 _max(Constants<HF32>::Max());
	//C_Cone& cone=pConeShape->m_Cone;
	const HVec3 v(end - start);
	const HF32 cosTheta2 = one - (cone->SinTheta() * cone->SinTheta());
	const HVec3 p(cone->Tip());
	const HVec3 g(start - p);
	const C_Normal n(cone->MidPointAxis());
	const C_Normal d(-n, C_Normal::UNIT_LENGTH_VECTOR);
	const HF32 vd(v|d);
	const HF32 gd(g|d);
	const HF32 vv(v|v);
	const HF32 gv(g|v);
	const HF32 gg(g|g);

	const HF32 a(vd*vd - vv*cosTheta2);
	const HF32 b(gd*vd - gv*cosTheta2);
	const HF32 c(gd*gd - gg*cosTheta2);

	const HF32 discriminant(b*b - a*c);

	if(discriminant.LT(zero))
		return false;

	const HF32 sqrt(discriminant.Sqrt());
	const HF32 aRecip(a.Reciprocal());
	const HF32 tt1((-b-sqrt)*aRecip);
	const HF32 tt2((-b+sqrt)*aRecip);

	const HF32 t1 = Min(tt1, tt2);
	const HF32 t2 = Max(tt1, tt2);

	HF32 tCone(Select(t1.LT(zero), t2, t1));

	const HVec3 contactCone(start + tCone*v);
	const HVec3 vp(contactCone - p);
	const HF32 la(vp|d);

	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contactCone, 0.5f, RGBA(0, 255, 0));


	//the contact point is outside of the cone
	tCone = Select(la.GT(cone->Height()), _max, tCone);
	
	//Check ray and plane intersection
	const HVec3 q(cone->Bottom());
	const HF32 dist(q|d);
	const HF32 temp1(d|start);
	const HF32 temp2(d|v);
	HF32 tPlane((dist - temp1)/temp2);
	//const HVec3 cont = start + tPlane*v;
	//const HF32 signDist(d|(cont - q));

	//if(signDist.LE(zero))
	//{
	//	/*contact = start + tCone*v;
	//	return true;*/
	//	tPlane = zero;
	//}

	//if(tPlane.LE(zero) )
	//{
	//	contact = start + tCone*v;
	//	return true;
	//}
	/*const HVec3 cont = ray.m_start + tPlane*v;
	const HVec3 contP(cont - p);*/
	/*
		contP is the ray and plane intersect point(ray.m_start + tPlane*v) - the tip of the cone
	*/
	const HVec3 contP(g + tPlane*v);
	const HF32 distCont2 = (contP|contP) - (cone->Height() * cone->Height());
	const Scalar_Bool condi(distCont2.GE(cone->BaseRadius() * cone->BaseRadius()));
	tPlane = Select(condi, _max, tPlane);


	const HF32 tValue =Select(tCone.GT(tPlane), tPlane, tCone);

	if(tValue.GT(zero) && tValue.LT(one))
	{	
		contact = start + tValue*v;
		return true;
	}

	return false;
}


NOALIAS inline bool CollideConeRay(const C_Shape* const __restrict shape, C_Ray& ray)
{
	using namespace Maths;
	Spu::MemBaseMarker marker;
	C_ConeShape* pConeShape = (C_ConeShape*)shape;
	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one(Constants<HF32>::One());
	const HF32 _max(Constants<HF32>::Max());
	C_Cone& cone=pConeShape->m_Cone;
	const HVec3 v(ray.m_end - ray.m_start);
	const HF32 cosTheta2 = one - (cone.SinTheta() * cone.SinTheta());
	const HVec3 p(cone.Tip());
	const HVec3 g(ray.m_start - p);
	const C_Normal n(cone.MidPointAxis());
	const C_Normal d(-n, C_Normal::UNIT_LENGTH_VECTOR);
	const HF32 vd(v|d);
	const HF32 gd(g|d);
	const HF32 vv(v|v);
	const HF32 gv(g|v);
	const HF32 gg(g|g);

	const HF32 a(vd*vd - vv*cosTheta2);
	const HF32 b(gd*vd - gv*cosTheta2);
	const HF32 c(gd*gd - gg*cosTheta2);

	const HF32 discriminant(b*b - a*c);

	if(discriminant.LT(zero))
		return false;

	const HF32 sqrt(discriminant.Sqrt());
	const HF32 aRecip(a.Reciprocal());
	const HF32 tt1((-b-sqrt)*aRecip);
	const HF32 tt2((-b+sqrt)*aRecip);

	const HF32 t1 = Min(tt1, tt2);
	const HF32 t2 = Max(tt1, tt2);

	HF32 tCone(Select(t1.LT(zero), t2, t1));

	const HVec3 contactCone(ray.m_start + tCone*v);
	const HVec3 vp(contactCone - p);
	const HF32 la(vp|d);

	//the contact point is outside of the cone
	tCone = Select(la.GT(cone.Height()), _max, tCone);
	/*if(la.GT(cone.m_MidPointLength))
		return false;*/

	//Check ray and plane intersection
	const HVec3 q(cone.Bottom());
	const HF32 dist(q|d);
	const HF32 temp1(d|ray.m_start);
	const HF32 temp2(d|v);
	HF32 tPlane((dist - temp1)/temp2);

	/*const HVec3 cont = ray.m_start + tPlane*v;
	const HVec3 contP(cont - p);*/
	const HVec3 contP(g + tPlane*v);
	const HF32 distCont2 = (contP|contP) - (cone.Height() * cone.Height());
	const Scalar_Bool condi(distCont2.GE(cone.BaseRadius() * cone.BaseRadius()));
	tPlane = Select(condi, _max, tPlane);

	const HF32 tValue =Select(tCone.GE(tPlane), tPlane, tCone);

	if(tValue.GT(zero) && tValue.LT(one))
	{
		const HF32 lc = (vp|vp)/la;
		const HVec3 proj = p + lc*d;
		//const C_Normal dir(contactCone - proj);
		const C_Normal dir(contactCone - proj);

		const C_Normal normal = Select(tCone.GE(tPlane), d, dir);
		const HVec3 contact = ray.m_start + tValue*v;

		C_RayTracingResult& result=ray.m_result;
		result.m_contact=contact;
		
		result.m_normal=normal;
		result.m_TriangleNormal = normal;
		result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
		result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
		result.m_distance=(result.m_contact - ray.m_start).Length();
		result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
		result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
		result.m_UserData = shape->m_UserData;
		ray.m_end=contact;
		
		return true;
	}

	return false;
}

NOALIAS inline bool CollideConeRay(const C_Shape* const __restrict shape, C_TransparentRay& ray, SharedPhysics::C_GraphNode* pGraphNode)
{
	using namespace Maths;
	Spu::MemBaseMarker marker;
	C_ConeShape* pConeShape = (C_ConeShape*)shape;
	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one(Constants<HF32>::One());
	C_Cone& cone=pConeShape->m_Cone;
	const HVec3 v(ray.m_end - ray.m_start);
	const HF32 cosTheta2 = one - (cone.SinTheta() * cone.SinTheta());
	const HVec3 p(cone.Tip());
	const HVec3 g(ray.m_start - p);
	const C_Normal n(cone.MidPointAxis());
	const C_Normal d(-n, C_Normal::UNIT_LENGTH_VECTOR);
	const HF32 vd(v|d);
	const HF32 gd(g|d);
	const HF32 vv(v|v);
	const HF32 gv(g|v);
	const HF32 gg(g|g);

	const HF32 a(vd*vd - vv*cosTheta2);
	const HF32 b(gd*vd - gv*cosTheta2);
	const HF32 c(gd*gd - gg*cosTheta2);

	const HF32 discriminant(b*b - a*c);

	if(discriminant.LT(zero))
		return false;

	const HF32 sqrt(discriminant.Sqrt());
	const HF32 aRecip(a.Reciprocal());
	const HF32 tt1((-b-sqrt)*aRecip);
	const HF32 tt2((-b+sqrt)*aRecip);

	const HF32 t1 = Min(tt1, tt2);
	const HF32 t2 = Max(tt1, tt2);

	const HF32 tCone(Select(t1.LT(zero), t2, t1));

	const HVec3 contactCone = ray.m_start + tCone*v;
	const HVec3 vp(contactCone - p);
	const HF32 la(vp|d);

	//the contact point is outside of the cone
	if(la.GT(cone.Height()))
		return false;

	//Check ray and plane intersection
	const HVec3 q(cone.Bottom());
	const HF32 dist(q|d);
	const HF32 temp1(d|ray.m_start);
	const HF32 temp2(d|v);
	HF32 tPlane((dist - temp1)/temp2);

	/*const HVec3 cont = ray.m_start + tPlane*v;
	const HVec3 contP(cont - p);*/
	const HVec3 contP(g + tPlane*v);
	const HF32 distCont2 = (contP|contP) - (cone.Height() * cone.Height());
	const Scalar_Bool condi(distCont2.GE(cone.BaseRadius() * cone.BaseRadius()));
	tPlane = Select(condi, Constants<HF32>::Max(), tPlane);

	const HF32 tValue =Select(tCone.GE(tPlane), tPlane, tCone);

	if(tValue.GT(zero) && tValue.LT(one))
	{
		const HF32 lc = (vp|vp)/la;
		const HVec3 proj = p + lc*d;
		//const C_Normal dir(contactCone - proj);
		const C_Normal dir(contactCone - proj);

		const C_Normal normal = Select(tCone.GE(tPlane), d, dir);
		const HVec3 contact = ray.m_start + tValue*v;

		C_RayTracingResult result;
		result.m_contact=contact;
		
		result.m_normal=normal;
		result.m_TriangleNormal = normal;
		result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
		result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
		result.m_distance=(result.m_contact - ray.m_start).Length();
		result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
		result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
		result.m_UserData = shape->m_UserData;
		result.m_pNode = pGraphNode;
		ray.m_end=contact;

		ray.StoreResult(result);
		
		return true;
	}
	return false;
}
//
//
//NOALIAS FORCE_INLINE bool CollideCapsuleRay(const C_Capsule* const __restrict capsule, const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, Maths::HVec3& contact)
//{
//	using namespace Maths;
//	Spu::MemBaseMarker marker;
//	C_Capsule* pCapsule = (C_Capsule*)Spu::FetchAlloc(capsule, sizeof(C_Capsule));
//	const HF32 zero(Constants<HF32>::Zero());
//	const HVec3 m(start - capsule->m_Start);
//	const HVec3 n(end - start);
//	const HVec3 d(capsule->m_End - capsule->m_Start);
//
//	const HVec3 dn(d^n);
//	const HVec3 dm(d^m);
//	const HF32 dd(d|d);
//	const HF32 rr(capsule->m_Radius*capsule->m_Radius);
//
//	const HF32 a(dn|dn);
//	const HF32 b(dm|dn);
//	const HF32 c((dm|dm) - rr*dd);
//
//	const HF32 discriminant(b*b - a*c);
//
//	if(discriminant.LT(zero))
//		return false;
//
//	const HF32 sqrt(discriminant.Sqrt());
//	const HF32 aRecip(a.Reciprocal());
//	const HF32 t1((-b-sqrt)*aRecip);
//	const HF32 t2((-b+sqrt)*aRecip);
//
//	const Scalar_Bool con(t1.LT(zero));
//	const HF32 t(Select(con, t2, t1));
//
//	contact=start + n*t;
//
//	HVec3 closest(Maths::UNDEFINED);
//	Collision::ClosestPtPointSegment(contact, capsule->Start(), capsule->End(), closest);
//
//	const HVec3 dir(contact-closest);
//	const HF32 length(dir.Length());
//	//the ray hit in one of the end hemishpere fo the capsule
//	if(length.GT(capsule->m_Radius))
//	{
//		//Calculate the contact between the ray and the sphere if they are intersect
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
//		/*const C_Normal dir(start - closest);
//		contact = closest + dir * capsule->m_Radius;*/
//	}
//	return true;
//}

};

#endif