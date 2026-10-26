#ifndef CAPSULESPHERE_INL
#define CAPSULESPHERE_INL

//using namespace Maths;

namespace Collision
{

	inline bool CollideCapsuleSphere(const C_Capsule& capsule, const C_Sphere& sphere, Maths::HVec3& contact, Maths::HF32& pDepth, Maths::C_Normal& normal)
	{
		using namespace Maths;
		const HVec3 c_start(capsule.Start());
		const HVec3 c_end(capsule.End());
		const HVec3 center(sphere.Centre());
		const HF32 radius(sphere.Radius());
		const HF32 r(radius+capsule.Radius());
		HVec3 closest(Maths::UNDEFINED);
		ClosestPtPointSegment(center, c_start, c_end, closest);
		const HVec3 temp(center - closest);

		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closest, 0.25f, RGBA(0, 0, 255));

		const HF32 sqDist(temp|temp);
		if(sqDist > r*r)
			return false;		//non collide

		
		const HF32 len(sqDist.Sqrt());
		//pDepth=(r-len);

		normal=C_Normal(temp/len, C_Normal::UNIT_LENGTH_VECTOR) ;
		//contact = (center+(normal*radius));
		const HVec3 sphereContact = center - normal * radius;
		contact = closest + normal*capsule.Radius();
		pDepth = (contact - sphereContact).Length();

		return true;

	}

	template <int MaxContacts>
	bool CollideCapsuleSphere(const C_Capsule& capsule, const C_Sphere& sphere, C_ContactPatch<MaxContacts>& patch, const U32 physicsMaterial, const U32 effectMaterial)
	{
		using namespace Maths;
		const HVec3 c_start(capsule.Start());
		const HVec3 c_end(capsule.End());
		const HVec3 center(sphere.Centre());
		const HF32 radius(sphere.Radius());
		const HF32 r(radius+capsule.Radius());
		HVec3 closest(Maths::UNDEFINED);
		ClosestPtPointSegment(center, c_start, c_end, closest);
		const HVec3 temp(closest-center);
		const HF32 sqDist(temp|temp);
		if(sqDist > r*r)
			return false;		//non collide

		const HF32 len(sqDist.Sqrt());
		const HF32 pDepth(r-len);

		const C_Normal normal(temp/len, C_Normal::UNIT_LENGTH_VECTOR);
		const HVec3 contactPoint(center+(normal*radius));

		patch.m_Normal= normal;
		patch.m_NumContactPoints=1;

		C_ContactPoint& point=patch.m_ContactPoints[0];
		point.m_ContactPoint = contactPoint;
		//point.m_PenetrationDepth=pDepth;
		Maths::Store(pDepth, &point.m_PenetrationDepth);
		point.m_HashValue = 0;
		point.m_PhysicsMaterialIndices = physicsMaterial;
		point.m_EffectMaterialIndices = effectMaterial;

		return true;
	}

};

#endif
