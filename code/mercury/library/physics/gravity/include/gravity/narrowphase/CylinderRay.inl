#ifndef CYLINDERRAY_INL
#define CYLINDERRAY_INL

#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/PrimitiveTest.h"

namespace Collision
{

NOALIAS inline bool CollideCylinderRay(const C_Shape* const __restrict shape, C_Ray& ray)
{
	C_CylinderShape* pCylShape = (C_CylinderShape*)shape;
	Collision::C_Cylinder& cylinder=pCylShape->m_Cylinder;
	const Maths::HF32 r(cylinder.Radius());
	const Maths::HVec3 center(cylinder.Centre());
	const Maths::HF32 halfHeight(cylinder.HalfHeight());
	//const Maths::HVec3 u(cylinder.CenterAxis());
	const Maths::HVec3 u(cylinder.LocalCentreAxis());

	const Maths::HMat33 orientation(shape->Orientation());

	//Translate ray into cylinder's local space
	const Maths::HVec3 s(orientation*(ray.m_start - center));
	const Maths::HVec3 e(orientation*(ray.m_end - center));
	const Maths::HVec3 v= e-s;
	const Maths::C_Normal d(Normalise(v));


	
	const Maths::HF32 su(s|u);
	const Maths::HF32 du(d|u);
	const Maths::HVec3 i(u*su);
	const Maths::HVec3 j(u*du);
	const Maths::HVec3 m(s-i);
	const Maths::HVec3 n(d-j);
	/*
		p=i+t*j, x=s+t*d, (x-p)(x-p)=r*r, x-p=(s-i)+t(d-j)=m+tn
		(x-p)(x-p)=(m|m)+2t*(m|n)+t*t*(n|n)=r*r
		According to the quadratic equations in t on the form a(t*t)+2b*t + c*c=0
		a=(n|n), b=m|n , c=(m|m)-r*r, t=(-b(+/-)Sqrt(b*b-ac))/a
	*/

	const Maths::HF32 a(n|n);
	const Maths::HF32 b(m|n);
	const Maths::HF32 c((m|m) - r*r);

	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const Maths::HF32 discriminant(b*b - a*c);
	if(discriminant.LE(zero)) // no intersection with the infinite cylinder
		return false;
	const Maths::HF32 comp(discriminant.Sqrt());
	const Maths::HF32 t0((-b+comp)/a);
	const Maths::HF32 t1((-b-comp)/a);

	if(t0.LT(zero) && t1.LT(zero))
		return false;

	const Maths::HF32 _min(Min(t0, t1)); //the point when the ray enter the cyliner
	const Maths::HF32 _max(Max(t0, t1)); //the point when the ray exit the cylinder

	const Maths::HF32 vLength = v.Length();
	if(_min.GT(vLength))
		return false;

	const Maths::HVec3 p(i+_min*j); //p is the projection point of x on the local axis

	//Now need to determine whether the point is in the valid range, which is the two endcaps

	const Maths::HVec3 x0(s + _min*d);
	const Maths::HVec3 x1(s + _max*d);
	//Calculate the sign distance from the point to the endcap planes
	const Maths::HF32 dist0(u|x0);
	const Maths::HF32 dist1(u|x1);


	const Maths::HF32 dist(dist0*dist1);

	const Maths::Scalar_Bool con0(dist0.Abs().LE(halfHeight));
	const Maths::Scalar_Bool con1(dist1.Abs().LE(halfHeight));
	//the enter point and exit point are on the same side of a endcap plane
	const Maths::Scalar_Bool con2(dist.GT(zero));

	if((!con0) & (!con1) & (con2))
		return false;	//no intersection with the finite cylinder

	/*
		calculate the point hit the endcap(either the plane(ux=halfHeight or ux=-halfHeight))
	*/
	const Maths::Scalar_Bool cont(dist0.LE(zero));
	const Maths::HF32 h=Select(cont, -halfHeight, halfHeight);
	const Maths::HVec3 nor=Select(cont, -u, u);
	const Maths::HF32 k((h-su)/du);

	const Maths::HVec3 point=Select(con0, x0, (s+k*d));
	const Maths::HVec3 normal0=Select(con0, x0-p, nor);
	const Maths::HVec3 normal1(normal0*orientation);
	const Maths::HVec3 contact(point*orientation + center);
	ray.m_result.m_contact=contact;
	ray.m_end=contact;
	ray.m_result.m_distance=(x0-s).Length();
	ray.m_result.m_normal=Maths::C_Normal(normal1);
	ray.m_result.m_TriangleNormal = ray.m_result.m_normal;
	ray.m_result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
	ray.m_result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
	ray.m_result.m_TriangleIndex = -1;
	ray.m_result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
	ray.m_result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
	ray.m_result.m_UserData = shape->m_UserData;
	return true;
}

NOALIAS inline bool CollideCylinderRay(const C_Shape* const __restrict shape, C_TransparentRay& ray,
									   SharedPhysics::C_GraphNode* pGraphNode)
{
	C_CylinderShape* pCylShape = (C_CylinderShape*)shape;
	Collision::C_Cylinder& cylinder=pCylShape->m_Cylinder;
	const Maths::HF32 r(cylinder.Radius());
	const Maths::HVec3 center(cylinder.Centre());
	const Maths::HF32 halfHeight(cylinder.HalfHeight());
	//const Maths::HVec3 u(cylinder.CenterAxis());
	const Maths::HVec3 u(cylinder.LocalCentreAxis());

	const Maths::HMat33 orientation(shape->Orientation());

	//Translate ray into cylinder's local space
	const Maths::HVec3 s(orientation*(ray.m_start - center));
	const Maths::HVec3 e(orientation*(ray.m_end - center));
	const Maths::HVec3 v= e-s;
	const Maths::C_Normal d(Normalise(v));


	
	const Maths::HF32 su(s|u);
	const Maths::HF32 du(d|u);
	const Maths::HVec3 i(u*su);
	const Maths::HVec3 j(u*du);
	const Maths::HVec3 m(s-i);
	const Maths::HVec3 n(d-j);
	/*
		p=i+t*j, x=s+t*d, (x-p)(x-p)=r*r, x-p=(s-i)+t(d-j)=m+tn
		(x-p)(x-p)=(m|m)+2t*(m|n)+t*t*(n|n)=r*r
		According to the quadratic equations in t on the form a(t*t)+2b*t + c*c=0
		a=(n|n), b=m|n , c=(m|m)-r*r, t=(-b(+/-)Sqrt(b*b-ac))/a
	*/

	const Maths::HF32 a(n|n);
	const Maths::HF32 b(m|n);
	const Maths::HF32 c((m|m) - r*r);

	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const Maths::HF32 discriminant(b*b - a*c);
	if(discriminant.LE(zero)) // no intersection with the infinite cylinder
		return false;
	const Maths::HF32 comp(discriminant.Sqrt());
	const Maths::HF32 t0((-b+comp)/a);
	const Maths::HF32 t1((-b-comp)/a);

	if(t0.LT(zero) && t1.LT(zero))
		return false;

	const Maths::HF32 _min(Min(t0, t1)); //the point when the ray enter the cyliner
	const Maths::HF32 _max(Max(t0, t1)); //the point when the ray exit the cylinder

	const Maths::HF32 vLength = v.Length();
	if(_min.GT(vLength))
		return false;

	const Maths::HVec3 p(i+_min*j); //p is the projection point of x on the local axis

	//Now need to determine whether the point is in the valid range, which is the two endcaps

	const Maths::HVec3 x0(s + _min*d);
	const Maths::HVec3 x1(s + _max*d);
	//Calculate the sign distance from the point to the endcap planes
	const Maths::HF32 dist0(u|x0);
	const Maths::HF32 dist1(u|x1);


	const Maths::HF32 dist(dist0*dist1);

	const Maths::Scalar_Bool con0(dist0.Abs().LE(halfHeight));
	const Maths::Scalar_Bool con1(dist1.Abs().LE(halfHeight));
	//the enter point and exit point are on the same side of a endcap plane
	const Maths::Scalar_Bool con2(dist.GT(zero));

	if((!con0) & (!con1) & (con2))
		return false;	//no intersection with the finite cylinder

	/*
		calculate the point hit the endcap(either the plane(ux=halfHeight or ux=-halfHeight))
	*/
	const Maths::Scalar_Bool cont(dist0.LE(zero));
	const Maths::HF32 h=Select(cont, -halfHeight, halfHeight);
	const Maths::HVec3 nor=Select(cont, -u, u);
	const Maths::HF32 k((h-su)/du);

	const Maths::HVec3 point=Select(con0, x0, (s+k*d));
	const Maths::HVec3 normal0=Select(con0, x0-p, nor);
	const Maths::HVec3 normal1(normal0*orientation);
	const Maths::HVec3 contact(point*orientation + center);

	Collision::C_RayTracingResult result;

	result.m_contact=contact;
	result.m_distance=(x0-s).Length();
	result.m_normal=Maths::C_Normal(normal1);
	result.m_TriangleNormal = result.m_normal;
	result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
	result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
	result.m_TriangleIndex = -1;
	result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
	result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
	result.m_UserData = shape->m_UserData;
	result.m_pNode = pGraphNode;

	ray.StoreResult(result);
	return true;
}
};

#endif
