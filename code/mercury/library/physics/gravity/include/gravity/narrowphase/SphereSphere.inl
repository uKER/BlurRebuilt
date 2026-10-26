#ifndef SPHERESPHERE_INL
#define SPHERESPHERE_INL

namespace Collision
{

inline bool CollideSphereSphere(const C_Sphere& sphereA, const C_Sphere& sphereB, Maths::HVec3& contact, Maths::HF32& pDepth, Maths::C_Normal& normal)
{
	/*using namespace Maths;
	const HVec3 d(sphereB.Centre() - sphereA.Centre());
	const HF32 rad(sphereA.Radius() + sphereB.Radius());
	const HF32 d2(d | d);
	if(d2.GT(rad * rad))
	{
		return false;
	} 

	const HF32 len(d.Length());
	normal = C_Normal(d/len, C_Normal::UNIT_LENGTH_VECTOR);
	contact = sphereA.Centre() + (normal * sphereA.Radius());
	pDepth = (rad - len);
	
	return true;*/

	using namespace Maths;
	const HVec3 d(sphereB.Centre() - sphereA.Centre());
	const HF32 rad(sphereA.Radius() + sphereB.Radius());
	const HF32 d2(d | d);
	if(d2.GT(rad * rad))
		return false;
	
	const HF32 len(d.Length());
	normal = C_Normal(d/len, C_Normal::UNIT_LENGTH_VECTOR);
	contact = sphereA.Centre() + (normal * sphereA.Radius());
	const Scalar_Bool con(rad > len);
	const HVec3 contactB = sphereB.Centre() - normal*sphereB.Radius();
	pDepth = Select(con, rad - len, (contact - contactB).Length());

	return true;
}

template <int MaxContacts>
bool CollideSphereSphere(const C_Sphere& sphereA, const C_Sphere& sphereB, C_ContactPatch<MaxContacts>& patch,
						 const U32 physicsMaterial, const U32 effectMaterial)
{
	const Maths::HVec3 d(sphereB.Centre() - sphereA.Centre());
	const Maths::HF32 rad(sphereA.Radius() + sphereB.Radius());
	const Maths::HF32 d2(d | d);
	if(d2.GT(rad * rad))
	{
		return false;
	}

	patch.m_NumContactPoints = 1;
	C_ContactPoint& point(patch.m_ContactPoints[0]);

	const Maths::HF32 len(d.Length());
	const Maths::C_Normal normal(d/len, Maths::C_Normal::UNIT_LENGTH_VECTOR);
	patch.m_Normal = normal;
	point.m_ContactPoint = sphereA.Centre() + (normal * sphereA.Radius());
	const Maths::HF32 pDepth(rad - len);
	Maths::Store(pDepth, &point.m_PenetrationDepth);
	//point.m_PenetrationDepth = (rad - len);
	point.m_PhysicsMaterialIndices = physicsMaterial;
	point.m_EffectMaterialIndices = effectMaterial;
	point.m_HashValue = 0;
	return true;
}

};
#endif

