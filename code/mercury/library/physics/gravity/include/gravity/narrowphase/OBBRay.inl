#ifndef OBBRAY_INL
#define OBBRAY_INL

#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/PrimitiveTest.h"

namespace Collision
{

inline Maths::Scalar_Bool IntersectSegmentBox(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF extents, Maths::HVec3& minContacts, Maths::HVec3& maxContacts, Maths::HVec3& normal)
{
	using namespace Maths;

	const HVec3 frontNormal(-Constants<HVec3>::One());
	const HVec3 backNormal(Constants<HVec3>::One());
	const HF32 zero(Constants<HF32>::Zero());
	const HVec3 d(q - p);
	const HVec3 ood(d.Reciprocal());
	const HVec3 epsilon(Constants<HVec3>::Epsilon2());
	const HVec3 _max(Constants<HVec3>::One());
	const HVec3 _min(Constants<HVec3>::Zero());
	
	//Check to see if any of the extents are outside of the box...
	const HVec3 absD(Abs(d));
	const Vector3_Bool bSmall(absD < epsilon);
	const Vector3_Bool bOutside((bSmall) & ((p < -extents) | (p > extents)));
	
	const HVec3 _t1(((-extents) - p) * ood);
	const HVec3 _t2((extents - p) * ood);
	const Vector3_Bool vb(_t1 < _t2);
	const HVec3 normals(Maths::Select(vb, frontNormal, backNormal));
	const HVec3 t11(Select(vb, _t1, _t2));
	const HVec3 t21(Select(vb, _t2, _t1));

	const HVec3 t1(Select(bSmall, _min, Max(t11, _min)));
	const HVec3 t2(Select(bSmall, _max, Min(t21, _max)));

	const HF32 t1x(t1.X());
	const HF32 t1y(t1.Y());
	const HF32 t1z(t1.Z());
	const HF32 t2x(t2.X());
	const HF32 t2y(t2.Y());
	const HF32 t2z(t2.Z());

	const Scalar_Bool sb1(t1y.GT(t1z));
	const HF32 tmin_tmp(Select(sb1, t1y, t1z));
	const Scalar_Bool sb2(tmin_tmp.GT(t1x));
	const HF32 tmin(Select(sb2, tmin_tmp, t1x));

	normal = HVec3(Select(!sb2, normals.X(), zero), Select(sb1 & sb2, normals.Y(), zero), 
		Select(!sb1 & sb2, normals.Z(), zero));

	const Scalar_Bool sb3(t2y.LT(t2z));
	const HF32 tmax_tmp=Select(sb3, t2y, t2z);
	const Scalar_Bool sb4(tmax_tmp.LT(t2x));
	const HF32 tmax=Select(sb4, tmax_tmp, t2x);
	
	//Work out what the contact point is...
	minContacts = p + d * tmin;
	maxContacts = p + d * tmax;
	const Scalar_Bool sb(X(bOutside) | Y(bOutside) | Z(bOutside));
	return ((tmin.LT(tmax)) & (!sb)) != 0;
}

NOALIAS inline bool CollideOBBRay(const C_Shape* const shape, C_Ray& ray)
{
	using namespace Maths;
	C_BoxShape* pBoxShape = (C_BoxShape*)shape;
	C_OBB& obb=pBoxShape->m_Box;
	HVec3 minContacts(UNDEFINED);
	HVec3 maxContacts(UNDEFINED);
	//const Maths::HMat33& u =obb.u.Transposed(); 
	const HMat33&u = obb.Orientation();
	const HVec3 e(obb.Extent());
	const HVec3 c(obb.Centre());
	const HVec3 p(u*(ray.m_start-c));
	const HVec3 q(u*(ray.m_result.m_contact-c));
	HVec3 normal(UNDEFINED);

	if(IntersectSegmentBox(p, q, e, minContacts, maxContacts, normal))
	{
		const HVec3 contact(minContacts*u+c);
		const HVec3 norm(normal * u);

		ray.m_result.m_contact=contact;
		ray.m_result.m_normal = C_Normal(norm, C_Normal::UNIT_LENGTH_VECTOR);
		ray.m_result.m_TriangleNormal = ray.m_result.m_normal;
		ray.m_result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
		ray.m_result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
		ray.m_result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
		ray.m_result.m_distance=(contact-ray.m_start).Length();
		ray.m_result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
		ray.m_result.m_TriangleIndex = -1;
		ray.m_result.m_UserData = pBoxShape->m_UserData;
		ray.m_end=contact;
		return true;
	}
	return false;
}


NOALIAS inline bool CollideOBBRay(const C_Shape* const shape, C_TransparentRay& ray,
								  SharedPhysics::C_GraphNode* pGraphNode)
{
	using namespace Maths;
	C_BoxShape* pBoxShape = (C_BoxShape*)shape;
	C_OBB& obb=pBoxShape->m_Box;
	HVec3 minContacts(UNDEFINED);
	HVec3 maxContacts(UNDEFINED);
	HVec3 normal(UNDEFINED);
	const HMat33& u = obb.Orientation();
	const HVec3 e(obb.Extent());
	const HVec3 c(obb.Centre());
	//const Maths::HMat33& u =obb.u.Transposed();
	const HVec3 p(u*(ray.m_start -c));
	const HVec3 q(u*(ray.m_end - c));
	

	if(IntersectSegmentBox(p, q, e, minContacts, maxContacts, normal))
	{
		const HVec3 contact(minContacts*u+c);
		const HVec3 norm(normal * u);

		Collision::C_RayTracingResult result;

		result.m_contact=contact;
		result.m_normal = C_Normal(norm, C_Normal::UNIT_LENGTH_VECTOR);
		result.m_TriangleNormal = result.m_normal;
		result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
		result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
		result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
		result.m_distance=(contact-ray.m_start).Length();
		result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
		result.m_TriangleIndex = -1;
		result.m_UserData = pBoxShape->m_UserData;
		result.m_pNode = pGraphNode;

		ray.StoreResult(result);
		
		return true;
	}
	return false;
}

NOALIAS inline bool CollideAABBSweptSphere(const Maths::HVec3 PCREF max, const Maths::HVec3 PCREF min, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q,
							const Maths::HF32 PCREF radius)
{
	using namespace Maths;
	//Low-detail version of the function - may give false positives, but only going to be used for traversal code
	const HF32 one(Constants<HF32>::One());
	const HVec3 r(radius);
	const HVec3 nMin(min - r);
	const HVec3 nMax(max + r);
	HF32 t(UNDEFINED);
	return (IntersectRayAABBMinMax(p, q, nMin, nMax, t));
}

};

#endif
