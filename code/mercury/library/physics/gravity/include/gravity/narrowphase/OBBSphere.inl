#ifndef OBBSPHERE_INL
#define OBBSPHERE_INL

//using namespace Maths;

namespace Collision
{


inline bool CollideOBBSphere(const C_OBB& obb, const C_Sphere& sphere, Maths::HVec3& contact, Maths::HF32& pDepth, Maths::C_Normal& normal)
{
	/*
		P is a point in the world space, Q is the corresponding point represented in the local space.
		Therefore, 
		P = M*Q (M is the translation matrix)
		Q=Inver(M)*P;
	*/

	using namespace Maths;
	const HF32 zero(Constants<HF32>::Zero());
	const HVec3 centre(sphere.Centre());
	const HF32 radius(sphere.Radius());
	//const Maths::HVec3 lc(obb.u * (sphere.Centre()) ;
	const HVec3 d(centre - obb.Centre());
	const HMat33& u = obb.Orientation();

	//cL is the point representing sphere.c in the local space
	const HVec3 cL(u * d); //M*d(Math) = d*M(Implementation), d*Inver(M) = M*d 
	const HVec3 e(obb.Extent());

	//Get the projection of cL of box obb
	const HVec3 proj(Max(-e, Min(cL, e)));

	const HVec3 w(proj - cL);
	if((w|w).EQ(zero))
	{
		//The center of the sphere is inside the obb 
		
		const HF32 x(cL.X());
		const HF32 y(cL.Y());
		const HF32 z(cL.Z());

		/*const HF32 distX = Min( e.X() - x, x - ne.X());
		const HF32 distY = Min( e.Y() - y, y - ne.Y());
		const HF32 distZ = Min( e.Z() - z, z - ne.Z());*/

		const HVec3 dist = Min(e - cL, cL + e);

		
		const HVec3 projX (x + Sign(x)*dist.X(),	y,			z		 );
		const HVec3 projY (x,			y + Sign(y)*dist.Y(),	z		 );
		const HVec3 projZ (x,			y,			z + Sign(z)*dist.Z());


		const Scalar_Bool con1(dist.X().LT(dist.Y()) & dist.X().LT(dist.Z()));
		const Scalar_Bool con2(dist.Y().LT(dist.Z()));
		
		const HVec3 contactPoint = Select(con1, projX, Select(con2, projY, projZ));

		const HMat33& u = obb.Orientation();

		contact = (obb.Centre() + contactPoint*u);

		const HVec3 v(contact - centre);

		const Maths::HF32 vLen(v.Length());
		pDepth = vLen + radius;
		normal = Maths::C_Normal(v/vLen, Maths::C_Normal::UNIT_LENGTH_VECTOR);

		return true;

	}

	contact =(obb.Centre() + proj*obb.Orientation());

	const HVec3 v(centre- contact);
	//const Maths::HF32 length(v.Length());
	const HF32 l2(v | v);
	const HF32 r2(radius * radius);

	if(l2.GT(r2))
		return false;

	const HF32 length(l2.Sqrt());
	normal= C_Normal(v/length, C_Normal::UNIT_LENGTH_VECTOR);
	pDepth=radius - length;

	return true;
	
}

template <int MaxContacts>
bool CollideOBBSphere(const C_OBB& obb, const C_Sphere& sphere, C_ContactPatch<MaxContacts>& patch, const U32 physicsMaterial, const U32 effectMaterial)
{
	/*
		P is a point in the world space, Q is the corresponding point represented in the local space.
		Therefore, 
		P = M*Q (M is the translation matrix)
		Q=Inver(M)*P;
	*/

	using namespace Maths;
	const HF32 zero(Constants<HF32>::Zero());
	//const Maths::HVec3 lc(obb.u * (sphere.Centre()) ;
	const HVec3 d(sphere.Centre() - obb.Centre());

	//cL is the point representing sphere.c in the local space
	const HVec3 cL(obb.Orientation() * d); //M*d(Math) = d*M(Implementation), d*Inver(M) = M*d 
	//const HVec3 ne(-obb.e);

	//Get the projection of cL of box obb
	const HVec3 proj(Max(-obb.Extent(), Min(cL, obb.Extent())));
	const HVec3 w(proj - cL);
	if((w|w).EQ(zero))
	{
		//The center of the sphere is inside the obb 
		
		const HF32 x(cL.X());
		const HF32 y(cL.Y());
		const HF32 z(cL.Z());

		/*const HF32 distX = Min( obb.e.X() - x, x - ne.X());
		const HF32 distY = Min( obb.e.Y() - y, y - ne.Y());
		const HF32 distZ = Min( obb.e.Z() - z, z - ne.Z());*/

		const HVec3 dist = Min(obb.Extent() - cL, obb.Extent() + cL);

		
		const HVec3 projX (x + Sign(x)*dist.X(),	y,			z		 );
		const HVec3 projY (x,			y + Sign(y)*dist.Y(),	z		 );
		const HVec3 projZ (x,			y,			z + Sign(z)*dist.Z());


		const Scalar_Bool con1(dist.X().LT(dist.Y()) & dist.X().LT(dist.Z()));
		const Scalar_Bool con2(dist.Y().LT(dist.Z()));
		
		const HVec3 contact = Select(con1, projX, Select(con2, projY, projZ));

		/*const HVec3 sign(Sign(cL.X()), Sign(cL.Y()), Sign(cL.Z()));
		const HVec3 contact(sign*obb.e);*/

		const HVec3 contactPoint(obb.Centre() + contact*obb.Orientation());

		const HVec3 v(contactPoint - sphere.Centre());

		patch.m_NumContactPoints = 1;
		C_ContactPoint& point=patch.m_ContactPoints[0];
		point.m_HashValue = 0;
		point.m_PhysicsMaterialIndices = physicsMaterial;
		point.m_EffectMaterialIndices = effectMaterial;
		patch.m_Normal = C_Normal(v);
		point.m_ContactPoint = contactPoint;
		//const HF32 pDepth(v.Length() + sphere.Radius());
		const HF32 pDepth(sphere.Radius());
		Store(pDepth, &point.m_PenetrationDepth);
		//point.m_PenetrationDepth=v.Length() + sphere.Radius();
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contactPoint, 0.05f, Maths::RGBA(0, 0 ,255));
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(contactPoint, patch.m_Normal, point.m_PenetrationDepth, 0.01f, Maths::RGBA(0, 255 ,255));
		return true;

	}

	const HVec3 contactPoint(obb.Centre() + proj*obb.Orientation()); 

	
	const HVec3 v(sphere.Centre() - contactPoint);
	//const Maths::HF32 length(v.Length());
	const HF32 l2(v | v);
	const HF32 r2(sphere.Radius() * sphere.Radius());

	if(l2.GT(r2))
		return false;

	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contactPoint, 0.02f, Maths::RGBA(255, 255 ,0));

	
	const HF32 length(l2.Sqrt());
	const C_Normal normal(v/length, C_Normal::UNIT_LENGTH_VECTOR);
	const HF32 penetrationDepth(sphere.Radius() - length);
	patch.m_Normal = normal;
	patch.m_NumContactPoints = 1;
	C_ContactPoint& point=patch.m_ContactPoints[0];
	point.m_HashValue = 0;
	point.m_PhysicsMaterialIndices = physicsMaterial;
	point.m_EffectMaterialIndices = effectMaterial;
		
	point.m_ContactPoint = contactPoint;

	BIZ_ASSERT(penetrationDepth.LT(sphere.Radius()));
	Store(penetrationDepth, &point.m_PenetrationDepth);

	return true;
	
}

//template <int MaxContacts>
//bool CollideOBBSphere(const C_OBB& obb, const C_Sphere& sphere, C_ContactPatch<MaxContacts>& patch, const U32 physicsMaterial, const U32 effectMaterial)
//{
//	using namespace Maths;
//
//	HVec3 minContacts(UNDEFINED);
//	HVec3 maxContacts(UNDEFINED);
//	//const Maths::HMat33& u =obb.u.Transposed();
//	const HVec3 p(obb.u*(sphere.Centre()-obb.c));
//	const HVec3 q(Constants<HVec3>::Zero());
//
//	HVec3 normal(UNDEFINED);
//
//	//Because the ray end point is the center of the obb, therefore, the ray should always intersect
//	if(IntersectSegmentBox(p, q, obb.e, minContacts, maxContacts, normal))
//	{
//		
//		const HVec3 contact(minContacts*obb.u+obb.c);
//		const HVec3 v(sphere.Centre() - contact);
//		const HF32 sqDist(v|v);
//		const HVec3 norm(normal * obb.u);
//		if(sqDist.GT(sphere.Radius()*sphere.Radius()))
//			return false;
//
//		if(sqDist.EQ(Constants<HF32>::Zero()))
//		{
//			//if the ray is inside the box, the contact will be the same as the sphere centre
//			int bob=0;
//		}
//
//		
//
//		patch.m_Normal = C_Normal(sphere.Centre() - obb.c);//norm;
//		patch.m_NumContactPoints = 1;
//		C_ContactPoint& point=patch.m_ContactPoints[0];
//		point.m_HashValue = 0;
//		point.m_PhysicsMaterialIndices = physicsMaterial;
//		point.m_EffectMaterialIndices = effectMaterial;
//			
//		point.m_ContactPoint = contact;
//
//		const HF32 penetrationDepth = sphere.Radius() - v.Length();
//		//BIZ_ASSERT(penetrationDepth.LE(sphere.Radius() - v.Length()));
//		Store(penetrationDepth, &point.m_PenetrationDepth);
//
//				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.05f, Maths::RGBA(0, 0 ,255));
//		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(contact, patch.m_Normal, penetrationDepth, 0.01f, Maths::RGBA(0, 255 ,255));
//	}
//	else
//	{
//		int bob=0;
//		return false;
//	}
//	
//
//}

};

#endif
