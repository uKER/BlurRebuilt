 #include "StdAfx.h"
#include "gravity/narrowphase/GJKCollision.h"
#include "gravity/narrowphase/GJK.inl"
#include "gravity/narrowphase/primitive.inl"
#include "gravity/narrowphase/CollisionManager.h"
#include "gravity/narrowphase/CollisionManager.inl"

namespace Collision
{
						//Sphere vs...

	bool GJKSphereCylinder(C_Sphere& sphere, C_Cylinder& cylinder, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa)
	{
		return GJKCPairMargin<C_Sphere, C_Cylinder>(sphere, cylinder, 
			contact, normal.m_V, penetration, epa);
	}

	bool GJKSphereConvex(C_Sphere& sphere, C_ConvexHull& hull, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa)
	{
		Spu::MemBaseMarker marker;
		C_SpuConvexHull spuConvex(hull);
		return GJKCPairMargin<C_Sphere, C_SpuConvexHull>(sphere, spuConvex, 
			contact, normal.m_V, penetration, epa);
		/*return GJKCPairMargin<C_Sphere, C_ConvexHull>(sphere, hull, 
			contact, normal.m_V, penetration, epa);*/
	}

	bool GJKSphereCone(C_Sphere& sphere, C_Cone& cone, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa)
	{
		return GJKCPairMargin<C_Sphere, C_Cone>(sphere, cone, 
			contact, normal.m_V, penetration, epa);
	}

						//Capsule vs...

	bool GJKCapsuleCapsule(C_Capsule& capsule, C_Capsule& capsule2, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa)
	{
		return GJKCPairMargin<C_Capsule, C_Capsule>(capsule, capsule2, 
			contact, normal.m_V, penetration, epa);
	}

	bool GJKCapsuleCylinder(C_Capsule& capsule, C_Cylinder& cylinder, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa)
	{
		return GJKCPairMargin<C_Capsule, C_Cylinder>(capsule, cylinder, 
			contact, normal.m_V, penetration, epa);
	}

	bool GJKCapsuleConvex(C_Capsule& capsule, C_ConvexHull& hull, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa)
	{
		Spu::MemBaseMarker marker;
		C_SpuConvexHull spuConvex(hull);
		return GJKCPairMargin<C_Capsule, C_SpuConvexHull>(capsule, spuConvex, 
			contact, normal.m_V, penetration, epa);
		/*return GJKCPairMargin<C_Capsule, C_ConvexHull>(capsule, hull, 
			contact, normal.m_V, penetration, epa);*/
	}

	bool GJKCapsuleCone(C_Capsule& capsule, C_Cone& cone, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa)
	{
		return GJKCPairMargin<C_Capsule, C_Cone>(capsule, cone, 
			contact, normal.m_V, penetration, epa);
	}

						//Cylinder vs...

	bool GJKCylinderCylinder(C_Cylinder& cylinderA, C_Cylinder& cylinderB, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa)
	{
		return GJKCPairMargin<C_Cylinder, C_Cylinder>(cylinderA, cylinderB, 
			contact, normal.m_V, penetration, epa);
	}

	bool GJKCylinderConvex(C_Cylinder& cylinder, C_ConvexHull& hull, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa)
	{
		Spu::MemBaseMarker marker;
		C_SpuConvexHull spuConvex(hull);
		//spuConvex.Initialise(hull);
		return GJKCPairMargin<C_Cylinder, C_SpuConvexHull>(cylinder, spuConvex, 
			contact, normal.m_V, penetration, epa);
		/*return GJKCPairMargin<C_Cylinder, C_ConvexHull>(cylinder, hull, 
			contact, normal.m_V, penetration, epa);*/
	}

	bool GJKCylinderCone(C_Cylinder& cylinder, C_Cone& cone, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa)
	{
		return GJKCPairMargin<C_Cylinder, C_Cone>(cylinder, cone, 
			contact, normal.m_V, penetration, epa);
	}

						//Convex vs...

	bool GJKConvexConvex(C_ConvexHull& hullA, C_ConvexHull& hullB, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa)
	{
	
		Spu::MemBaseMarker marker;
		//DebugBreak();
		C_SpuConvexHull spuConvexA(hullA);
		C_SpuConvexHull spuConvexB(hullB);
		
		return GJKCPairMargin<C_SpuConvexHull, C_SpuConvexHull>(spuConvexA, spuConvexB, 
			contact, normal.m_V, penetration, epa);

		/*return GJKCPairMargin<C_ConvexHull, C_ConvexHull>(hullA, hullB, 
			contact, normal.m_V, penetration, epa);*/
	}


	//bool GJKConvexConvex(const C_SpuConvexHull& hullA, const C_SpuConvexHull& hullB, Maths::HVec3& contact, 
	//	Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa)
	//{
	//	
	//	return GJKCPairMargin<C_SpuConvexHull, C_SpuConvexHull>(spuConvexA, spuConvexB, 
	//		contact, normal.m_V, penetration, epa);
	//	/*return GJKCPairMargin<C_ConvexHull, C_ConvexHull>(hull, hullB, 
	//		contact, normal.m_V, penetration, epa);*/
	//}

	bool GJKConvexCone(C_ConvexHull& hull, C_Cone& cone, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa)
	{
		Spu::MemBaseMarker marker;
		Collision::C_SpuConvexHull spuhull(hull);
		return GJKCPairMargin<C_SpuConvexHull, C_Cone>(spuhull, cone, 
			contact, normal.m_V, penetration, epa);
	}

					//Cone vs Cone

	bool GJKConeCone(C_Cone& cone1, C_Cone& cone2, Maths::HVec3& contact, 
		Maths::C_Normal& normal, Maths::HF32& penetration, Collision::EPA& epa)
	{
		return GJKCPairMargin<C_Cone, C_Cone>(cone1, cone2, contact, normal.m_V, penetration, epa);
	}


}

