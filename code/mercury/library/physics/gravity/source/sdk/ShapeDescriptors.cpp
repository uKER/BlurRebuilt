#include "stdafx.h"
#include "gravity/narrowphase/Shapes.h"
#include "gravity/treebuilder/buildClothTree.h"
#include "gravity/treebuilder/AABBTree.h"
#include "gravity/sdk/ShapeDescriptors.h"
#include "gravity/sdk/InertiaTensorGenerator.h"
#include "gravity/narrowphase/primitive.inl"
#include "gravity/importer/InstanceMeshResourceManager.h"
#include "gravity/cloths/rope.h"

namespace Gravity
{
	/*
		C_BoxDesc
	*/

static Debug::Logger &g_ImportLog = Debug::GetLogger("mercury.physics.gravity.import");

#ifdef DEBUG_PHYSICS
#pragma optimize("", off)
#endif

	Collision::C_Shape* C_PlaneDesc::CreateShape(const Collision::C_CollisionFilter* pFilter) const
	{
		PERF_EVENT("C_PlaneDesc::CreateShape");
		Collision::C_PlaneShape* pPlaneShape = new Collision::C_PlaneShape(pFilter);
		pPlaneShape->m_Plane.m_n = m_Normal;
		pPlaneShape->m_Plane.m_d = m_Distance;
		return pPlaneShape;
	}

	Collision::C_Shape* C_PlaneDesc::CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const
	{
		BIZ_ASSERT(0);
		return NULL;
	}

	Collision::C_Shape* C_PlaneDesc::CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter)
	{
		Import(deserial);
		Collision::C_PlaneShape* pPlaneShape = new Collision::C_PlaneShape(pFilter);
		pPlaneShape->m_Plane.m_n = m_Normal;
		pPlaneShape->m_Plane.m_d = m_Distance;
		return pPlaneShape;
	}


	/*
		C_ConeDesc
	*/
	Collision::C_Shape* C_ConeDesc::CreateShape(const Collision::C_CollisionFilter* pFilter) const
	{
		PERF_EVENT("C_ConeDesc::CreateShape");
		using namespace Maths;
		Collision::C_ConeShape* pConeShape = new Collision::C_ConeShape(pFilter);
		const HVec3 center= Constants<HVec3>::Zero();
		const HVec3 offset(0.f, m_Height*Constants<HF32>::Quarter(), 0.f);
		const C_Normal axis = C_Normal(Constants<HVec3>::UnitAxisPositiveY(), C_Normal::UNIT_LENGTH_VECTOR);
		pConeShape->m_Cone.SetUpCone(center, offset, axis, m_BaseRadius, m_Height); 
		pConeShape->m_LocalBoundingBox = GetBoundingBox();
		pConeShape->m_ElementOffset = m_RenderOffset;
		pConeShape->m_Cone.Margin(m_Margin);
		pConeShape->m_Cone.Epsilon(m_Epsilon);
		pConeShape->m_Filter.MergeFlags(m_CollisionFlag);
		pConeShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
		pConeShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
		pConeShape->m_UserData = m_UserDataFlag;
		return pConeShape;
	}

	Collision::C_Shape* C_ConeDesc::CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const
	{
		PERF_EVENT("C_ConeDesc::CreateShape");
		using namespace Maths;
		Collision::C_ConeShape* pConeShape = new (pAddress) Collision::C_ConeShape(pFilter);
		const HVec3 center= Constants<HVec3>::Zero();
		const HVec3 offset(0.f, m_Height*Constants<HF32>::Quarter(), 0.f);
		const C_Normal axis = C_Normal(Constants<HVec3>::UnitAxisPositiveY(), C_Normal::UNIT_LENGTH_VECTOR);
		pConeShape->m_Cone.SetUpCone(center, offset, axis, m_BaseRadius, m_Height); 
		pConeShape->m_LocalBoundingBox = GetBoundingBox();
		pConeShape->m_ElementOffset = m_RenderOffset;
		pConeShape->m_Cone.Margin(m_Margin);
		pConeShape->m_Cone.Epsilon(m_Epsilon);
		pConeShape->m_Filter.MergeFlags(m_CollisionFlag);
		pConeShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
		pConeShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
		pConeShape->m_UserData = m_UserDataFlag;
		return pConeShape;
	}

	
	Collision::C_Shape* C_ConeDesc::CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter)
	{	
		using namespace Maths;
		Import(deserial);
		Collision::C_ConeShape* pConeShape = new Collision::C_ConeShape(pFilter);
		const HVec3 center= Constants<HVec3>::Zero();
		const HVec3 offset(0.f, m_Height*Constants<HF32>::Quarter(), 0.f);
		const C_Normal axis = C_Normal(Constants<HVec3>::UnitAxisPositiveY(), C_Normal::UNIT_LENGTH_VECTOR);
		pConeShape->m_Cone.SetUpCone(center, offset, axis, m_BaseRadius, m_Height); 
		pConeShape->m_LocalBoundingBox = GetBoundingBox();
		pConeShape->m_ElementOffset = m_RenderOffset;
		pConeShape->m_Cone.Margin(m_Margin);
		pConeShape->m_Cone.Epsilon(m_Epsilon);
		pConeShape->m_Filter.MergeFlags(m_CollisionFlag);
		pConeShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
		pConeShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
		pConeShape->m_UserData = m_UserDataFlag;
		return pConeShape;
	}

	Maths::HMat33 C_ConeDesc::InertiaTensor(const Maths::HF32 PCREF mass) const
	{
		//Caclualte the inertia tensor for a cone!
		return C_InertiaTensorGenerator::ConeInertiaTensor(mass, m_BaseRadius, m_Height);
	}

	
	Collision::C_CRAxisBox C_ConeDesc::GetBoundingBox() const
	{
		Collision::C_CRAxisBox box;
		box.centre = Maths::Constants<Maths::HVec3>::Zero();
		box.extents.X(m_BaseRadius + m_Margin);
		box.extents.Y(m_Height*Maths::Constants<Maths::HF32>::Half() + m_Margin);
		box.extents.Z(m_BaseRadius + m_Margin);
		return box;
	}


	Collision::C_Shape* C_BoxDesc::CreateShape(const Collision::C_CollisionFilter* pFilter) const
	{
		PERF_EVENT("C_BoxDesc::CreateShape");
		Collision::C_BoxShape* pBoxShape = new Collision::C_BoxShape(pFilter);//s_BoxAllocationManager.GetNextFreeItem();
		pBoxShape->m_Filter.MergeFlags(m_CollisionFlag);
		pBoxShape->m_ElementOffset = m_RenderOffset;
		pBoxShape->m_Box.Centre(Maths::Constants<Maths::HVec3>::Zero());
		pBoxShape->m_Box.Extent(m_HalfExtents);
		//pBoxShape->m_Box.u.SetIdentity();
		pBoxShape->m_Box.Orientation(Maths::Constants<Maths::HMat33>::Identity());
		pBoxShape->m_Box.Margin(m_Margin);
		pBoxShape->m_LocalBoundingBox = GetBoundingBox();
		pBoxShape->m_Box.Epsilon(m_Epsilon);
		pBoxShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
		pBoxShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
		pBoxShape->m_UserData = m_UserDataFlag;
		return pBoxShape;
	}

	Collision::C_Shape* C_BoxDesc::CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const
	{
		PERF_EVENT("C_BoxDesc::CreateShape");
		Collision::C_BoxShape* pBoxShape = new (pAddress)Collision::C_BoxShape(pFilter);//s_BoxAllocationManager.GetNextFreeItem();
		pBoxShape->m_Filter.MergeFlags(m_CollisionFlag);
		pBoxShape->m_ElementOffset = m_RenderOffset;
		pBoxShape->m_Box.Centre(Maths::Constants<Maths::HVec3>::Zero());
		pBoxShape->m_Box.Extent(m_HalfExtents);
		//pBoxShape->m_Box.u.SetIdentity();
		pBoxShape->m_Box.Orientation(Maths::Constants<Maths::HMat33>::Identity());
		pBoxShape->m_Box.Margin(m_Margin);
		pBoxShape->m_LocalBoundingBox = GetBoundingBox();
		pBoxShape->m_Box.Epsilon(m_Epsilon);
		pBoxShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
		pBoxShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
		pBoxShape->m_UserData = m_UserDataFlag;
		return pBoxShape;
	}

	
	Collision::C_Shape* C_BoxDesc::CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter)
	{	
		Import(deserial);
		Collision::C_BoxShape* pBoxShape = new Collision::C_BoxShape(pFilter);//s_BoxAllocationMaanger.GetNextFreeItem();
		pBoxShape->m_Box.Centre(Maths::Constants<Maths::HVec3>::Zero());
		pBoxShape->m_ElementOffset = m_RenderOffset;
		//pBoxShape->m_Box.u.SetIdentity();
		pBoxShape->m_Box.Orientation(Maths::Constants<Maths::HMat33>::Identity());
		pBoxShape->m_Box.Extent(m_HalfExtents);
		pBoxShape->m_LocalBoundingBox = GetBoundingBox();
		pBoxShape->m_Box.Margin(m_Margin);
		pBoxShape->m_Box.Epsilon(m_Epsilon);
		pBoxShape->m_Filter.MergeFlags(m_CollisionFlag);
		pBoxShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
		pBoxShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
		pBoxShape->m_UserData = m_UserDataFlag;
		return pBoxShape;
	}

	Maths::HMat33 C_BoxDesc::InertiaTensor(const Maths::HF32 PCREF mass) const
	{
		//Caclualte the inertia tensor for a box!
		return C_InertiaTensorGenerator::BoxInertiaTensor(mass, m_HalfExtents);
	}

	
	Collision::C_CRAxisBox C_BoxDesc::GetBoundingBox() const
	{
		Collision::C_CRAxisBox box;
		box.centre = Maths::Constants<Maths::HVec3>::Zero();
		box.extents = m_HalfExtents + Maths::HVec3(m_Margin);
		return box;
	}

	/*
		C_SphereDesc
	*/
	
	Collision::C_Shape* C_SphereDesc::CreateShape(const Collision::C_CollisionFilter* pFilter) const
	{
		PERF_EVENT("C_SphereDesc::CreateShape");
		Collision::C_SphereShape* pSphereShape = new Collision::C_SphereShape(pFilter);//s_BoxAllocationManager.GetNextFreeItem();
		pSphereShape->m_Sphere.Centre(Maths::Constants<Maths::HVec3>::Zero());
		pSphereShape->m_Sphere.Radius(m_Radius);
		pSphereShape->m_ElementOffset = m_RenderOffset;
		pSphereShape->m_LocalBoundingBox = GetBoundingBox();
		pSphereShape->m_Sphere.Epsilon(m_Epsilon);
		pSphereShape->m_Filter.MergeFlags(m_CollisionFlag);
		pSphereShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
		pSphereShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
		pSphereShape->m_UserData = m_UserDataFlag;
		return pSphereShape;
	}

	Collision::C_Shape* C_SphereDesc::CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const
	{
		PERF_EVENT("C_SphereDesc::CreateShape");
		Collision::C_SphereShape* pSphereShape = new (pAddress) Collision::C_SphereShape(pFilter);//s_BoxAllocationManager.GetNextFreeItem();
		pSphereShape->m_Sphere.Centre(Maths::Constants<Maths::HVec3>::Zero());
		pSphereShape->m_Sphere.Radius(m_Radius);
		pSphereShape->m_ElementOffset = m_RenderOffset;
		pSphereShape->m_LocalBoundingBox = GetBoundingBox();
		pSphereShape->m_Sphere.Epsilon(m_Epsilon);
		pSphereShape->m_Filter.MergeFlags(m_CollisionFlag);
		pSphereShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
		pSphereShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
		pSphereShape->m_UserData = m_UserDataFlag;
		return pSphereShape;
	}


	Collision::C_Shape* C_SphereDesc::CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter)
	{	

		Import(deserial);
		Collision::C_SphereShape* pSphereShape = new Collision::C_SphereShape(pFilter);//s_BoxAllocationManager.GetNextFreeItem();
		pSphereShape->m_Sphere.Centre(Maths::Constants<Maths::HVec3>::Zero());
		pSphereShape->m_Sphere.Radius(m_Radius);
		pSphereShape->m_ElementOffset = m_RenderOffset;
		pSphereShape->m_LocalBoundingBox = GetBoundingBox();
		pSphereShape->m_Sphere.Epsilon(m_Epsilon);
		pSphereShape->m_Filter.MergeFlags(m_CollisionFlag);
		pSphereShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
		pSphereShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
		pSphereShape->m_UserData = m_UserDataFlag;
		return pSphereShape;
	}

	
	Maths::HMat33 C_SphereDesc::InertiaTensor(const Maths::HF32 PCREF mass) const
	{
		//Caclualte the inertia tensor for a box!
		return C_InertiaTensorGenerator::SphereInertiaTensor(mass, m_Radius);
	}

	
	Collision::C_CRAxisBox C_SphereDesc::GetBoundingBox() const
	{
		Collision::C_CRAxisBox box;
		box.centre = Maths::Constants<Maths::HVec3>::Zero();
		box.extents = Maths::HVec3(m_Radius, m_Radius, m_Radius);
		return box;
	}

	/*
		C_CapsuleDesc
	*/

	
	Collision::C_Shape* C_CapsuleDesc::CreateShape(const Collision::C_CollisionFilter* pFilter) const
	{
		PERF_EVENT("C_CapsuleDesc::CreateShape");
		Collision::C_CapsuleShape* pCapsuleShape = new Collision::C_CapsuleShape(pFilter);
		const Maths::HF32 halfHeight(m_Height*Maths::Constants<Maths::HF32>::Half());
		pCapsuleShape->m_Capsule.LocalStart(Maths::HVec3(0.f, -halfHeight, 0.f));
		pCapsuleShape->m_Capsule.LocalEnd(Maths::HVec3(0.f, halfHeight, 0.f));
		pCapsuleShape->m_ElementOffset = m_RenderOffset;
		pCapsuleShape->m_Capsule.Radius(m_Radius);
		pCapsuleShape->m_Capsule.Epsilon(this->m_Epsilon);
		pCapsuleShape->m_LocalBoundingBox = GetBoundingBox();
		pCapsuleShape->m_Filter.MergeFlags(m_CollisionFlag);
		pCapsuleShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
		pCapsuleShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
		pCapsuleShape->m_UserData = m_UserDataFlag;
		return pCapsuleShape;
	}

	Collision::C_Shape* C_CapsuleDesc::CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const
	{
		PERF_EVENT("C_CapsuleDesc::CreateShape");
		Collision::C_CapsuleShape* pCapsuleShape = new (pAddress)Collision::C_CapsuleShape(pFilter);
		const Maths::HF32 halfHeight(m_Height*Maths::Constants<Maths::HF32>::Half());
		pCapsuleShape->m_Capsule.LocalStart(Maths::HVec3(0.f, -halfHeight, 0.f));
		pCapsuleShape->m_Capsule.LocalEnd(Maths::HVec3(0.f, halfHeight, 0.f));
		pCapsuleShape->m_ElementOffset = m_RenderOffset;
		pCapsuleShape->m_Capsule.Radius(m_Radius);
		pCapsuleShape->m_Capsule.Epsilon(this->m_Epsilon);
		pCapsuleShape->m_LocalBoundingBox = GetBoundingBox();
		pCapsuleShape->m_Filter.MergeFlags(m_CollisionFlag);
		pCapsuleShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
		pCapsuleShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
		pCapsuleShape->m_UserData = m_UserDataFlag;
		return pCapsuleShape;
	}

	
	Collision::C_Shape* C_CapsuleDesc::CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter)
	{	
		Import(deserial);
		const Maths::HF32 halfHeight(m_Height * Maths::Constants<Maths::HF32>::Half());
		Collision::C_CapsuleShape* pCapsuleShape = new Collision::C_CapsuleShape(pFilter);
		pCapsuleShape->m_Capsule.LocalStart(Maths::HVec3(0.f, -halfHeight, 0.f));
		pCapsuleShape->m_Capsule.LocalEnd(Maths::HVec3(0.f, halfHeight, 0.f));
		pCapsuleShape->m_Capsule.Radius(m_Radius);
		pCapsuleShape->m_ElementOffset = m_RenderOffset;
		pCapsuleShape->m_LocalBoundingBox = GetBoundingBox();
		pCapsuleShape->m_Capsule.Epsilon(this->m_Epsilon);
		pCapsuleShape->m_Filter.MergeFlags(m_CollisionFlag);
		pCapsuleShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
		pCapsuleShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
		pCapsuleShape->m_UserData = m_UserDataFlag;
		return pCapsuleShape;
	}

	
	Maths::HMat33 C_CapsuleDesc::InertiaTensor(const Maths::HF32 PCREF mass) const
	{
		//Caclualte the inertia tensor for a capsule!
		const Maths::HF32 twoThird(2.f/3.f);
		const Maths::HF32 oneThird(1.f/3.f);
		const Maths::HF32 massCylinder(mass*twoThird);
		const Maths::HF32 massHemisphere(mass*oneThird*Maths::Constants<Maths::HF32>::Half());
		return C_InertiaTensorGenerator::CapsuleInertiaTensor(massCylinder, m_Height, massHemisphere, m_Radius);
		//const Maths::HVec3 extents(m_Radius, m_Height * Maths::Constants<Maths::HF32>::Half() + m_Radius, m_Radius);
		//return C_InertiaTensorGenerator::BoxInertiaTensor(mass, extents);

		//return C_InertiaTensorGenerator::SphereInertiaTensor(mass, m_Radius);
	}

	
	Collision::C_CRAxisBox C_CapsuleDesc::GetBoundingBox() const
	{

		Collision::C_CRAxisBox box;
		box.centre = Maths::Constants<Maths::HVec3>::Zero();
		box.extents.X(m_Radius);
		box.extents.Y(m_Height*Maths::Constants<Maths::HF32>::Half() + m_Radius);
		box.extents.Z(m_Radius);
		return box;
	}


	/*
		C_CylinderDesc
	*/
	
	Collision::C_Shape* C_CylinderDesc::CreateShape(const Collision::C_CollisionFilter* pFilter) const
	{
		PERF_EVENT("C_CylinderDesc::CreateShape");
		Collision::C_CylinderShape* pCylinderShape = new Collision::C_CylinderShape(pFilter);
		//pCylinderShape->m_Cylinder.Center(m_Center);
		pCylinderShape->m_Cylinder.HalfHeight(m_Height*Maths::Constants<Maths::HF32>::Half());
		pCylinderShape->m_Cylinder.Radius(m_Radius);
		pCylinderShape->m_ElementOffset = m_RenderOffset;
		pCylinderShape->m_LocalBoundingBox = GetBoundingBox();
		pCylinderShape->m_Cylinder.Epsilon(m_Epsilon);
		pCylinderShape->m_Cylinder.Margin(m_Margin);
		pCylinderShape->m_Filter.MergeFlags(m_CollisionFlag);
		pCylinderShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
		pCylinderShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
		pCylinderShape->m_UserData = m_UserDataFlag;
		return pCylinderShape;
	}

	Collision::C_Shape* C_CylinderDesc::CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const
	{
		PERF_EVENT("C_CylinderDesc::CreateShape");
		Collision::C_CylinderShape* pCylinderShape = new (pAddress) Collision::C_CylinderShape(pFilter);
		//pCylinderShape->m_Cylinder.Center(m_Center);
		pCylinderShape->m_Cylinder.HalfHeight(m_Height*Maths::Constants<Maths::HF32>::Half());
		pCylinderShape->m_Cylinder.Radius(m_Radius);
		pCylinderShape->m_ElementOffset = m_RenderOffset;
		pCylinderShape->m_LocalBoundingBox = GetBoundingBox();
		pCylinderShape->m_Cylinder.Epsilon(m_Epsilon);
		pCylinderShape->m_Cylinder.Margin(m_Margin);
		pCylinderShape->m_Filter.MergeFlags(m_CollisionFlag);
		pCylinderShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
		pCylinderShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
		pCylinderShape->m_UserData = m_UserDataFlag;
		return pCylinderShape;
	}

	
	Collision::C_Shape* C_CylinderDesc::CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter)
	{	
		Import(deserial);

		Collision::C_CylinderShape* pCylinderShape = new Collision::C_CylinderShape(pFilter);
		//pCylinderShape->m_Cylinder.Center(m_Center);
		pCylinderShape->m_Cylinder.HalfHeight(m_Height*Maths::Constants<Maths::HF32>::Half());
		pCylinderShape->m_Cylinder.Radius(m_Radius);
		pCylinderShape->m_ElementOffset = m_RenderOffset;
		pCylinderShape->m_LocalBoundingBox = GetBoundingBox();
		pCylinderShape->m_Cylinder.Epsilon(m_Epsilon);
		pCylinderShape->m_Cylinder.Margin(m_Margin);
		pCylinderShape->m_Filter.MergeFlags(m_CollisionFlag);
		pCylinderShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
		pCylinderShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
		pCylinderShape->m_UserData = m_UserDataFlag;
		return pCylinderShape;
	}

	
	Maths::HMat33 C_CylinderDesc::InertiaTensor(const Maths::HF32 PCREF mass) const
	{
		const Maths::HF32 five(5.f);
		if( (m_Radius * five).LT(m_Height))
			return C_InertiaTensorGenerator::CylinderInertiaTensor(mass, m_Radius, m_Height);
		else if( (m_Height * five).LT(m_Radius))
			return C_InertiaTensorGenerator::ThinCircularDiskInertiaTensor(mass, m_Radius);
	
		return C_InertiaTensorGenerator::CylinderInertiaTensor(mass, m_Radius, m_Height);
	}

	
	Collision::C_CRAxisBox C_CylinderDesc::GetBoundingBox() const
	{
		Collision::C_CRAxisBox box;
		box.centre = Maths::Constants<Maths::HVec3>::Zero();
		//box.extents = Maths::HVec3(m_Radius, m_Radius, m_Radius);
		box.extents.Y(m_Height * Maths::Constants<Maths::HF32>::Half() + m_Margin);
		box.extents.X(m_Radius + m_Margin);
		box.extents.Z(m_Radius + m_Margin);
		return box;
	}

	/*
		C_ClothDesc
	*/

	C_ClothDesc::C_ClothDesc() : C_ShapeDesc(Collision::E_Cloth_Shape, Collision::E_ImportCloth_Shape), m_HasCollision(true)
	{
#if CFG_CONVERTER
		m_ExportClothTree = new Baking::C_BuildSoftBodyTree;
#else
		m_ImportClothTree = new Collision::C_SoftBodyTree(Collision::E_ClothSystem);
#endif
	}

	C_ClothDesc::~C_ClothDesc()
	{
#if CFG_CONVERTER
		delete m_ExportClothTree;
#else
		delete m_ImportClothTree;
#endif
	}

	Collision::C_Shape* C_ClothDesc::CreateShape(const Collision::C_CollisionFilter *pFilter) const
	{
		PERF_EVENT("C_ClothDesc::CreateShape");
#if !CFG_CONVERTER
		if(m_HasCollision)
		{
			Collision::C_ClothShape* pClothShape = new Collision::C_ClothShape(pFilter);
			Collision::C_Cloth& cloth = pClothShape->m_Cloth;
			cloth = *m_ImportClothTree;
			pClothShape->m_Filter.MergeFlags(m_CollisionFlag);
			pClothShape->m_UserData = m_UserDataFlag;
			cloth.CalculateParticleEdgeData();
			cloth.InitialiseEdgeTriangleIndices();
			return pClothShape;
		}
		else
		{
			Collision::C_NonCollidableClothShape* pClothShape = new Collision::C_NonCollidableClothShape(pFilter);
			Collision::C_Cloth& cloth = pClothShape->m_Cloth;
			cloth = *((Collision::C_Cloth*)m_ImportClothTree);
			pClothShape->m_Filter.MergeFlags(m_CollisionFlag);
			pClothShape->m_UserData = m_UserDataFlag;
			cloth.CalculateParticleEdgeData();
			cloth.InitialiseEdgeTriangleIndices();
			return pClothShape;
		}
#else
		return NULL;
#endif

	}

	Collision::C_Shape* C_ClothDesc::CreateShape(const Collision::C_CollisionFilter *pFilter, void* pAddress) const
	{
		BIZ_ASSERT(0);
		return NULL;
	}

	Collision::C_Shape* C_ClothDesc::CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter)
	{	
#if !CFG_CONVERTER
		Collision::C_ClothShape* pClothShape = new Collision::C_ClothShape(pFilter);
		Collision::C_Cloth& cloth = pClothShape->m_Cloth;
		cloth.Import(deserial);
		pClothShape->m_Filter.MergeFlags(m_CollisionFlag);
		pClothShape->m_UserData = m_UserDataFlag;
		cloth.CalculateParticleEdgeData();
		cloth.InitialiseEdgeTriangleIndices();
		return pClothShape;

#else
		return NULL;
#endif
	}

	Maths::HMat33 C_ClothDesc::InertiaTensor(const Maths::HF32 PCREF mass) const
	{
		//Mesh is immoveable, so it's inertia tensor is all zeros
		Maths::HMat33 temp(Maths::ZERO_MATRIX);
		return temp;
	}

	Collision::C_CRAxisBox C_ClothDesc::GetBoundingBox() const
	{
		Collision::C_CRAxisBox box;
		return box;
	}

	bool C_ClothDesc::ValidShapeDescriptor() const
	{
#if CFG_CONVERTER
		return m_ExportClothTree->m_SoftBodyTriangles.m_numElements > 0;
#else
		return true;
#endif
	}

	bool C_ClothDesc::Import(Deserialiser& deserial)
	{
#if !CFG_CONVERTER
		ImportBasic(deserial);

		m_ImportClothTree->Import(deserial);
#endif
		return true;
		
	}

	/*
		C_DeformableDes
	*/

	C_DeformableDesc::C_DeformableDesc() : C_ShapeDesc(Collision::E_Deformable_Shape, Collision::E_ImportDeformable_Shape), m_HasCollision(true)
	{
#if CFG_CONVERTER
		m_ExportDeformableTree = new Baking::C_BuildSoftBodyTree;
#else
		m_ImportDeformableTree = new Collision::C_SoftBodyTree(Collision::E_DeformableSystem);
#endif
	}

	C_DeformableDesc::~C_DeformableDesc()
	{
#if CFG_CONVERTER
		delete m_ExportDeformableTree;
#else
		delete m_ImportDeformableTree;
#endif
	}

	Collision::C_Shape* C_DeformableDesc::CreateShape(const Collision::C_CollisionFilter *pFilter) const
	{
		PERF_EVENT("C_DeformableDesc::CreateShape");
#if !CFG_CONVERTER
		
		Collision::C_DeformableShape* pShape = new Collision::C_DeformableShape(pFilter);
		Collision::C_DeformableBody& deformableBody = pShape->m_Deformable;
		deformableBody = *m_ImportDeformableTree;
		pShape->m_Filter.MergeFlags(m_CollisionFlag);
		pShape->m_UserData = m_UserDataFlag;
		deformableBody.CalculateParticleEdgeData();
		deformableBody.InitialiseEdgeTriangleIndices();
		return pShape;
	
		/*else
		{
			Collision::C_NonCollidableClothShape* pClothShape = new Collision::C_NonCollidableClothShape(pFilter);
			Collision::C_Cloth& cloth = pClothShape->m_Cloth;
			cloth = *((Collision::C_Cloth*)m_ImportClothTree);
			pClothShape->m_Filter.MergeFlags(m_CollisionFlag);
			pClothShape->m_UserData = m_UserDataFlag;
			cloth.CalculateParticleEdgeData();
			cloth.InitialiseEdgeTriangleIndices();
			return pClothShape;
		}*/
#else
		return NULL;
#endif

	}

	Collision::C_Shape* C_DeformableDesc::CreateShape(const Collision::C_CollisionFilter *pFilter, void* pAddress) const
	{
		BIZ_ASSERT(0);
		return NULL;
	}

	Collision::C_Shape* C_DeformableDesc::CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter)
	{	
#if !CFG_CONVERTER
		Collision::C_DeformableShape* pShape = new Collision::C_DeformableShape(pFilter);
		Collision::C_DeformableBody& deformableBody = pShape->m_Deformable;
		deformableBody.Import(deserial);
		pShape->m_Filter.MergeFlags(m_CollisionFlag);
		pShape->m_UserData = m_UserDataFlag;
		deformableBody.CalculateParticleEdgeData();
		deformableBody.InitialiseEdgeTriangleIndices();
		return pShape;

#else
		return NULL;
#endif
	}

	Maths::HMat33 C_DeformableDesc::InertiaTensor(const Maths::HF32 PCREF mass) const
	{
		//Mesh is immoveable, so it's inertia tensor is all zeros
		Maths::HMat33 temp(Maths::ZERO_MATRIX);
		return temp;
	}

	Collision::C_CRAxisBox C_DeformableDesc::GetBoundingBox() const
	{
		Collision::C_CRAxisBox box;
		return box;
	}

	bool C_DeformableDesc::ValidShapeDescriptor() const
	{
#if CFG_CONVERTER
		return m_ExportDeformableTree->m_SoftBodyTriangles.m_numElements > 0;
#else
		return true;
#endif
	}

	bool C_DeformableDesc::Import(Deserialiser& deserial)
	{
#if !CFG_CONVERTER
		ImportBasic(deserial);

		m_ImportDeformableTree->Import(deserial);
#endif
		return true;
		
	}

	/*
		C_FluidDesc
	*/

	Collision::C_Shape* C_FluidDesc::CreateShape(const Collision::C_CollisionFilter *pFilter) const
	{
		PERF_EVENT("C_FluidDesc::CreateShape");
		Collision::C_FluidShape* pFluidShape = new Collision::C_FluidShape(pFilter);
		Collision::C_Fluid& fluid = pFluidShape->m_Fluid;
		pFluidShape->m_Filter.MergeFlags(m_CollisionFlag);
		pFluidShape->m_UserData = m_UserDataFlag;
		return pFluidShape;
	

	}

	Collision::C_Shape* C_FluidDesc::CreateShape(const Collision::C_CollisionFilter *pFilter, void* pAddress) const
	{
		BIZ_ASSERT(0);
		return NULL;
	}

	Collision::C_Shape* C_FluidDesc::CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter)
	{
		
		Collision::C_FluidShape* pFluidShape = new Collision::C_FluidShape(pFilter);
		Collision::C_Fluid& fluid = pFluidShape->m_Fluid;

		fluid.SetGridDimension(10);
		fluid.SetWaveSpeed(Maths::HF32(2.f));
		fluid.SetWaveWidth(Maths::HF32(1.f));
		pFluidShape->m_Filter.MergeFlags(m_CollisionFlag);
		pFluidShape->m_UserData = m_UserDataFlag;
	
		return pFluidShape;
	}

	Maths::HMat33 C_FluidDesc::InertiaTensor(const Maths::HF32 PCREF mass) const
	{
		Maths::HMat33 temp(Maths::ZERO_MATRIX);
		return temp;
	}

	Collision::C_CRAxisBox C_FluidDesc::GetBoundingBox() const
	{
		Collision::C_CRAxisBox box;
		return box;
	}


	/*
		C_FluidMeshDesc
	*/

	C_FluidMeshDesc::C_FluidMeshDesc() : C_ShapeDesc(Collision::E_FluidMesh_Shape, Collision::E_ImportFluidMesh_Shape),
		m_Transform(Maths::Constants<Maths::HMat43>::Identity()), m_Normal(Maths::UNDEFINED), m_Distance(Maths::UNDEFINED),
		m_Vertex(Maths::UNDEFINED), m_Density(Maths::Constants<Maths::HF32>::Zero()), m_LinearDrag(Maths::Constants<Maths::HF32>::Zero()), 
		m_AngularDrag(Maths::Constants<Maths::HF32>::Zero()), m_Current(Maths::Constants<Maths::HVec3>::Zero()), 
		m_MaxBuoyancy(Maths::Constants<Maths::HF32>::Zero()), m_Gravity(Maths::Constants<Maths::HF32>::Zero())
	{
#if !CFG_CONVERTER
		m_OptimisedTree = new Collision::OptimisedAABBTree;
#else
	m_AABBTree = new Baking::AABBTree;
#endif
	}

	C_FluidMeshDesc::~C_FluidMeshDesc()
	{
#if !CFG_CONVERTER
		delete m_OptimisedTree;
#else
		delete m_AABBTree;
#endif
	}

	bool C_FluidMeshDesc::ValidShapeDescriptor() const
	{
#if CFG_CONVERTER
		return m_AABBTree->GetNumTriangles() > 0;
#else
		return m_OptimisedTree->m_NumTriangles > 0;
#endif
	}
	
	Collision::C_Shape* C_FluidMeshDesc::CreateShape(const Collision::C_CollisionFilter* pFilter)const
	{
		PERF_EVENT("C_FluidMeshDesc::CreateShape");
#if !CFG_CONVERTER
		//Kier - I think we can't have this anymore. Need to check and verify this works otherwise we're going to have a memory leak!
		//m_Instanced = true;
		this->m_NodeFlag = SharedPhysics::C_GraphNode::E_Type_Static;
		Collision::C_FluidMeshShape* pFluidMeshShape = new Collision::C_FluidMeshShape(pFilter);
		pFluidMeshShape->m_AABBTree.m_pTree = m_OptimisedTree;
		Collision::C_AxisBoxMinMax minMaxBox;
		m_OptimisedTree->m_pNodes[0].m_BoundingBox.LoadBox(minMaxBox, m_OptimisedTree->m_Low, m_OptimisedTree->m_High);
		minMaxBox.m_min.Y(minMaxBox.m_min.Y() - m_Depth);
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		pFluidMeshShape->m_LocalBoundingBox.centre = (minMaxBox.m_min + minMaxBox.m_max) * half;
		pFluidMeshShape->m_LocalBoundingBox.extents = (minMaxBox.m_max - minMaxBox.m_min) * half;
		pFluidMeshShape->m_Filter.MergeFlags(m_CollisionFlag);
		pFluidMeshShape->m_Normal = m_Normal;
		pFluidMeshShape->m_Distance = m_Distance;
		pFluidMeshShape->m_Vertex = m_Vertex;
		pFluidMeshShape->m_Gravity = m_Gravity;
		pFluidMeshShape->m_Current = m_Current;
		pFluidMeshShape->m_Density = m_Density;
		pFluidMeshShape->m_LinearDrag= m_LinearDrag;
		pFluidMeshShape->m_AngularDrag= m_AngularDrag;
		pFluidMeshShape->m_MaxBuoyancy= m_MaxBuoyancy;
		pFluidMeshShape->m_UserData = m_UserDataFlag;
		return pFluidMeshShape;
#else
		return NULL;
#endif
	}

	Collision::C_Shape* C_FluidMeshDesc::CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress)const
	{
		BIZ_ASSERT(0);
		return NULL;
	}

	U32 C_FluidMeshDesc::GetCombinedCollisionFlags() const
	{
#if CFG_CONVERTER
		U32 combinedFlags = 0;
		const SharedPhysics::SortedArray<Baking::AABBBuildTreeTriangle*>& tris = m_AABBTree->GetTriangles();
		for(S32 a = 0; a < tris.m_numElements; ++a)
		{
			Baking::AABBBuildTreeTriangle* tri = tris[a];
			combinedFlags |= tri->m_CollisionFlags;
		}
		return combinedFlags;
#else
		return 0;
#endif
	}

	bool C_FluidMeshDesc::Import(Deserialiser& deserial)
	{
#if !CFG_CONVERTER
		//deserial >> (U32&)C_ShapeDesc<Deserialiser>::m_Type >> C_ShapeDesc<Deserialiser>::m_ID;
		ImportBasic(deserial);
		deserial >> m_Depth;
		m_CollisionFlag = m_OptimisedTree->DeSerialise(deserial);
		if(m_OptimisedTree->m_NumNormals >= 1 && m_OptimisedTree->m_NumVertices >= 1)
		{
			Maths::C_Normal norm(Maths::Load<Maths::HVec3>(&m_OptimisedTree->m_pNormals[0]), Maths::C_Normal::UNIT_LENGTH_VECTOR);
			//Oops...this can't work.
			//m_Vertex = Maths::HVec3(&m_OptimisedTree->m_pVertices[0]);

			//Will need to load in a quantized set of vertices instead!
			Collision::OptimisedAABBNode* pNode = m_OptimisedTree->m_pNodes;
			while(pNode->m_TriangleSizeIndices == 0)
			{
				pNode = pNode + 1;
			}

			Collision::C_AxisBoxMinMax box;
			pNode->m_BoundingBox.LoadBox(box, m_OptimisedTree->m_Low, m_OptimisedTree->m_High);
			m_Vertex = Maths::HVec3(&m_OptimisedTree->m_pVertices[pNode->m_VerticeStartIndex]);
			m_Vertex = (m_Vertex * (box.m_max - box.m_min)) + box.m_min;
			
			Maths::HF32 dis = norm | m_Vertex;
			m_Normal = norm;
			m_Distance = dis;
		}

		if(deserial.m_stream.GetVersionNumber() >= 5)
		{
			//Load in fluid density/drag/current/buoyancy
			deserial >> m_Density >>m_LinearDrag >> m_AngularDrag >> m_Current >> m_MaxBuoyancy >> m_Gravity;
		}
#endif
		return true;
	}

	
	Collision::C_Shape* C_FluidMeshDesc::CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter)
	{	
#if !CFG_CONVERTER
		//Kier - I think we can't have this anymore. Need to check and verify this works otherwise we're going to have a memory leak!
		//m_Instanced = true;
		this->m_NodeFlag = SharedPhysics::C_GraphNode::E_Type_Static;
		Collision::C_FluidMeshShape* pFluidMeshShape = new Collision::C_FluidMeshShape(pFilter);
		pFluidMeshShape->m_Filter.SetCollisionFlags(0);
		pFluidMeshShape->m_AABBTree.m_pTree = m_OptimisedTree;
		Collision::C_AxisBoxMinMax minMaxBox;
		m_OptimisedTree->m_pNodes[0].m_BoundingBox.LoadBox(minMaxBox, m_OptimisedTree->m_Low, m_OptimisedTree->m_High);
		minMaxBox.m_min.Y(minMaxBox.m_min.Y() - m_Depth);
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		pFluidMeshShape->m_LocalBoundingBox.centre = (minMaxBox.m_min + minMaxBox.m_max) * half;
		pFluidMeshShape->m_LocalBoundingBox.extents = (minMaxBox.m_max - minMaxBox.m_min) * half;
		pFluidMeshShape->m_Filter.MergeFlags(m_CollisionFlag);
		pFluidMeshShape->m_Normal = m_Normal;
		pFluidMeshShape->m_Distance = m_Distance;
		pFluidMeshShape->m_Vertex = m_Vertex;
		pFluidMeshShape->m_Gravity = m_Gravity;
		pFluidMeshShape->m_Current = m_Current;
		pFluidMeshShape->m_Density = m_Density;
		pFluidMeshShape->m_LinearDrag= m_LinearDrag;
		pFluidMeshShape->m_AngularDrag= m_AngularDrag;
		pFluidMeshShape->m_MaxBuoyancy= m_MaxBuoyancy;
		pFluidMeshShape->m_UserData = m_UserDataFlag;
		return pFluidMeshShape;
#else
		return NULL;
#endif
	}


	
	Maths::HMat33 C_FluidMeshDesc::InertiaTensor(const Maths::HF32 PCREF mass) const
	{
		//Mesh is immoveable, so it's inertia tensor is all zeros
		Maths::HMat33 temp(Maths::ZERO_MATRIX);
		return temp;
	}

	
	Collision::C_CRAxisBox C_FluidMeshDesc::GetBoundingBox() const
	{
		Collision::C_CRAxisBox box;
		return box;
	}

	/*
		C_TriangleMeshDesc
	*/

	C_TriangleMeshDesc::C_TriangleMeshDesc() : C_ShapeDesc(Collision::E_TriangleMesh_Shape, Collision::E_ImportTriangleMesh_Shape), m_Instanced(false)
	{
#if !CFG_CONVERTER
		m_OptimisedTree = new Collision::OptimisedAABBTree;
#else
	m_AABBTree = new Baking::AABBTree;
#endif
	}

	C_TriangleMeshDesc::~C_TriangleMeshDesc()
	{
#if !CFG_CONVERTER
		delete m_OptimisedTree;
#else
		delete m_AABBTree;
#endif
	}

	bool C_TriangleMeshDesc::ValidShapeDescriptor() const
	{
#if CFG_CONVERTER
		return m_AABBTree->GetNumTriangles() > 0;
#else
		return m_OptimisedTree->m_NumTriangles > 0;
#endif
	}
	
	Collision::C_Shape* C_TriangleMeshDesc::CreateShape(const Collision::C_CollisionFilter* pFilter)const
	{
		PERF_EVENT("C_TriangleMeshDesc::CreateShape");
#if !CFG_CONVERTER
		//Kier - I think we can't have this anymore. Need to check and verify this works otherwise we're going to have a memory leak!
		//m_Instanced = true;
		this->m_NodeFlag = SharedPhysics::C_GraphNode::E_Type_Static;
		Collision::C_TriangleMeshShape* pMeshShape = new Collision::C_TriangleMeshShape(pFilter);
		pMeshShape->m_AABBTree = *m_OptimisedTree;
		Collision::C_AxisBoxMinMax minMaxBox;
		pMeshShape->m_AABBTree.m_pNodes[0].m_BoundingBox.LoadBox(minMaxBox, pMeshShape->m_AABBTree.m_Low, pMeshShape->m_AABBTree.m_High);
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		pMeshShape->m_LocalBoundingBox.centre = (minMaxBox.m_min + minMaxBox.m_max) * half;
		pMeshShape->m_LocalBoundingBox.extents = (minMaxBox.m_max - minMaxBox.m_min) * half;
		pMeshShape->m_Filter.MergeFlags(m_CollisionFlag);
		pMeshShape->m_UserData = m_UserDataFlag;
		return pMeshShape;
#else
		return NULL;
#endif
	}

	Collision::C_Shape* C_TriangleMeshDesc::CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress)const
	{
		BIZ_ASSERT(0);
		return NULL;
	}

	U32 C_TriangleMeshDesc::GetCombinedCollisionFlags() const
	{
#if CFG_CONVERTER
		U32 combinedFlags = 0;
		const SharedPhysics::SortedArray<Baking::AABBBuildTreeTriangle*>& tris = m_AABBTree->GetTriangles();
		for(S32 a = 0; a < tris.m_numElements; ++a)
		{
			Baking::AABBBuildTreeTriangle* tri = tris[a];
			combinedFlags |= tri->m_CollisionFlags;
		}
		return combinedFlags;
#else
		return 0;
#endif
	}

	bool C_TriangleMeshDesc::Import(Deserialiser& deserial)
	{
#if !CFG_CONVERTER
		//deserial >> (U32&)C_ShapeDesc<Deserialiser>::m_Type >> C_ShapeDesc<Deserialiser>::m_ID;
		ImportBasic(deserial);
		//deserial >> C_ShapeDesc::m_ID;
		m_CollisionFlag = m_OptimisedTree->DeSerialise(deserial);
		
#endif
		return true;
	}
	
	Collision::C_Shape* C_TriangleMeshDesc::CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter)
	{	
#if !CFG_CONVERTER
		this->m_NodeFlag = SharedPhysics::C_GraphNode::E_Type_Static;
		Collision::C_TriangleMeshShape* pMeshShape = new Collision::C_TriangleMeshShape(pFilter);
		//Mesh doesn't take flags! Just takes it from the polygons!!!
		pMeshShape->m_Filter.SetCollisionFlags(0);
		pMeshShape->m_AABBTree.DeSerialise(deserial);
		Collision::C_AxisBoxMinMax minMaxBox;
		pMeshShape->m_AABBTree.m_pNodes[0].m_BoundingBox.LoadBox(minMaxBox, pMeshShape->m_AABBTree.m_Low, pMeshShape->m_AABBTree.m_High);
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		pMeshShape->m_LocalBoundingBox.centre = (minMaxBox.m_min + minMaxBox.m_max) * half;
		pMeshShape->m_LocalBoundingBox.extents = (minMaxBox.m_max - minMaxBox.m_min) * half;
		pMeshShape->m_Filter.MergeFlags(m_CollisionFlag);
		pMeshShape->m_UserData = m_UserDataFlag;
		return pMeshShape;
#else
		return NULL;
#endif
	}


	
	Maths::HMat33 C_TriangleMeshDesc::InertiaTensor(const Maths::HF32 PCREF mass) const
	{
		//Mesh is immoveable, so it's inertia tensor is all zeros
		Maths::HMat33 temp(Maths::ZERO_MATRIX);
		return temp;
	}

	
	Collision::C_CRAxisBox C_TriangleMeshDesc::GetBoundingBox() const
	{
		Collision::C_CRAxisBox box;
		return box;
	}
	
	/*
		TriangleMeshBase Shape
	*/

	C_TriangleMeshBaseDesc::C_TriangleMeshBaseDesc() : C_ShapeDesc(Collision::E_TriangleMeshBase_Shape, Collision::E_ImportTriangleMeshBase_Shape)
	{
#if !CFG_CONVERTER
		m_OptimisedTree = new Collision::OptimisedAABBTree;
#else
		m_AABBTree = new Baking::AABBTree;
#endif
	}

	C_TriangleMeshBaseDesc::~C_TriangleMeshBaseDesc()
	{
#if !CFG_CONVERTER
		delete m_OptimisedTree;
#else
		delete m_AABBTree;
#endif
	}
	
	Collision::C_Shape* C_TriangleMeshBaseDesc::CreateShape(const Collision::C_CollisionFilter* pFilter)const
	{
		PERF_EVENT("C_TriangleMeshBaseDesc::CreateShape");
#if !CFG_CONVERTER
		Collision::C_TriangleMeshShape* pMeshShape = new Collision::C_TriangleMeshShape(pFilter);
		if(pMeshShape->m_AABBTree.m_NumTriangles > 0)
		{
			Collision::C_AxisBoxMinMax minMaxBox;
			pMeshShape->m_AABBTree.m_pNodes[0].m_BoundingBox.LoadBox(minMaxBox, pMeshShape->m_AABBTree.m_Low, pMeshShape->m_AABBTree.m_High);
			const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
			pMeshShape->m_LocalBoundingBox.centre = (minMaxBox.m_min + minMaxBox.m_max) * half;
			pMeshShape->m_LocalBoundingBox.extents = (minMaxBox.m_max - minMaxBox.m_min) * half;
			pMeshShape->m_Filter.MergeFlags(m_CollisionFlag);
			pMeshShape->m_UserData = m_UserDataFlag;
		}
		else
		{
			pMeshShape->m_LocalBoundingBox.centre = Maths::Constants<Maths::HVec3>::Zero();
			pMeshShape->m_LocalBoundingBox.extents = Maths::Constants<Maths::HVec3>::Zero();
		}
		return pMeshShape;
#else
		return NULL;
#endif
	}

	Collision::C_Shape* C_TriangleMeshBaseDesc::CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress)const
	{
		BIZ_ASSERT(0);
		return NULL;
	}

	
	Collision::C_Shape* C_TriangleMeshBaseDesc::CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter)
	{	
#if !CFG_CONVERTER
		Collision::C_TriangleMeshShape* pMeshShape = new Collision::C_TriangleMeshShape(pFilter);
		pMeshShape->m_AABBTree.DeSerialise(deserial);
		if(pMeshShape->m_AABBTree.m_NumTriangles > 0)
		{
			Collision::C_AxisBoxMinMax minMaxBox;
			pMeshShape->m_AABBTree.m_pNodes[0].m_BoundingBox.LoadBox(minMaxBox, pMeshShape->m_AABBTree.m_Low, pMeshShape->m_AABBTree.m_High);
			const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
			pMeshShape->m_LocalBoundingBox.centre = (minMaxBox.m_min + minMaxBox.m_max) * half;
			pMeshShape->m_LocalBoundingBox.extents = (minMaxBox.m_max - minMaxBox.m_min) * half;
			pMeshShape->m_Filter.MergeFlags(m_CollisionFlag);
			pMeshShape->m_UserData = m_UserDataFlag;
		}
		else
		{
			pMeshShape->m_LocalBoundingBox.centre = Maths::Constants<Maths::HVec3>::Zero();
			pMeshShape->m_LocalBoundingBox.extents = Maths::Constants<Maths::HVec3>::Zero();
		}
		return pMeshShape;
#else
		return NULL;
#endif
	}

	
	Maths::HMat33 C_TriangleMeshBaseDesc::InertiaTensor(const Maths::HF32 PCREF mass) const
	{
		//Mesh is immoveable, so it's inertia tensor is all zeros
		Maths::HMat33 temp(Maths::ZERO_MATRIX);
		return temp;
	}

	
	Collision::C_CRAxisBox C_TriangleMeshBaseDesc::GetBoundingBox() const
	{
		Collision::C_CRAxisBox box;
		return box;
	}

	U32 C_TriangleMeshBaseDesc::GetCombinedCollisionFlags() const
	{
#if CFG_CONVERTER
		U32 combinedFlags = 0;
		const SharedPhysics::SortedArray<Baking::AABBBuildTreeTriangle*>& tris = m_AABBTree->GetTriangles();
		for(S32 a = 0; a < tris.m_numElements; ++a)
		{
			Baking::AABBBuildTreeTriangle* tri = tris[a];
			combinedFlags |= tri->m_CollisionFlags;
		}
		return combinedFlags;
#else
		return 0;
#endif
	}

	bool C_TriangleMeshBaseDesc::Import(Deserialiser& deserial)
	{
#if !CFG_CONVERTER
		ImportBasic(deserial);
		//deserial >> (U32&)C_ShapeDesc::m_Type >> C_ShapeDesc::m_ID;
		m_CollisionFlag = m_OptimisedTree->DeSerialise(deserial);
#endif
		return true;
	}

	bool C_TriangleMeshBaseDesc::ValidShapeDescriptor() const
	{
#if CFG_CONVERTER
		return m_AABBTree->GetNumTriangles() > 0;
#else
		return m_OptimisedTree->m_NumTriangles > 0;
#endif
	}


	/*
		C_InstancedTriangleMeshShape
	 */

	
	bool C_InstancedTriangleMeshDesc::ValidShapeDescriptor() const
	{
		return (m_pBaseDesc != NULL && m_pBaseDesc->ValidShapeDescriptor()) || 
			(m_pShape != NULL && m_pShape->m_AABBTree.m_NumTriangles > 0);
	}


	Collision::C_Shape* C_InstancedTriangleMeshDesc::CreateShape(const Collision::C_CollisionFilter* pFilter) const
	{
		PERF_EVENT("C_InstancedTriangleMeshDesc::CreateShape");
		Collision::C_InstancedTriangleMeshShape* pMeshShape = new Collision::C_InstancedTriangleMeshShape(pFilter);
		pMeshShape->m_Filter.SetCollisionFlags(0);
		pMeshShape->m_AABBTree.m_pTree = &m_pShape->m_AABBTree;
		if(m_pShape->m_AABBTree.m_NumTriangles != 0)
		{
			Collision::C_AxisBoxMinMax minMaxBox;
			pMeshShape->m_AABBTree.m_pTree->m_pNodes[0].m_BoundingBox.LoadBox(minMaxBox, pMeshShape->m_AABBTree.m_pTree->m_Low, pMeshShape->m_AABBTree.m_pTree->m_High);
			const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
			pMeshShape->m_LocalBoundingBox.centre = (minMaxBox.m_min + minMaxBox.m_max) * half;
			pMeshShape->m_LocalBoundingBox.extents = (minMaxBox.m_max - minMaxBox.m_min) * half;
			pMeshShape->m_Filter.MergeFlags(m_CollisionFlag);
			pMeshShape->m_UserData = m_UserDataFlag;
		}
		else
		{
			pMeshShape->m_LocalBoundingBox.centre = Maths::Constants<Maths::HVec3>::Zero();
			pMeshShape->m_LocalBoundingBox.extents = pMeshShape->m_LocalBoundingBox.centre;
		}
		return pMeshShape;
	}

	Collision::C_Shape* C_InstancedTriangleMeshDesc::CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const
	{
		PERF_EVENT("C_InstancedTriangleMeshDesc::CreateShape");
		Collision::C_InstancedTriangleMeshShape* pMeshShape = new (pAddress) Collision::C_InstancedTriangleMeshShape(pFilter);
		pMeshShape->m_Filter.SetCollisionFlags(0);
		pMeshShape->m_AABBTree.m_pTree = &m_pShape->m_AABBTree;
		if(m_pShape->m_AABBTree.m_NumTriangles != 0)
		{
			Collision::C_AxisBoxMinMax minMaxBox;
			pMeshShape->m_AABBTree.m_pTree->m_pNodes[0].m_BoundingBox.LoadBox(minMaxBox, pMeshShape->m_AABBTree.m_pTree->m_Low, pMeshShape->m_AABBTree.m_pTree->m_High);
			const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
			pMeshShape->m_LocalBoundingBox.centre = (minMaxBox.m_min + minMaxBox.m_max) * half;
			pMeshShape->m_LocalBoundingBox.extents = (minMaxBox.m_max - minMaxBox.m_min) * half;
			pMeshShape->m_Filter.MergeFlags(m_CollisionFlag);
			pMeshShape->m_UserData = m_UserDataFlag;
		}
		else
		{
			pMeshShape->m_LocalBoundingBox.centre = Maths::Constants<Maths::HVec3>::Zero();
			pMeshShape->m_LocalBoundingBox.extents = pMeshShape->m_LocalBoundingBox.centre;
		}
		return pMeshShape;
	}

	
	Collision::C_Shape* C_InstancedTriangleMeshDesc::CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter)
	{	
		Collision::C_InstancedTriangleMeshShape* pMeshShape = new Collision::C_InstancedTriangleMeshShape(pFilter);
		pMeshShape->m_Filter.SetCollisionFlags(0);
		pMeshShape->m_AABBTree.m_pTree = &m_pShape->m_AABBTree;
		Collision::C_AxisBoxMinMax minMaxBox;
		if(m_pShape->m_AABBTree.m_NumTriangles != 0)
		{
			pMeshShape->m_AABBTree.m_pTree->m_pNodes[0].m_BoundingBox.LoadBox(minMaxBox, pMeshShape->m_AABBTree.m_pTree->m_Low, pMeshShape->m_AABBTree.m_pTree->m_High);
			const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
			pMeshShape->m_LocalBoundingBox.centre = (minMaxBox.m_min + minMaxBox.m_max) * half;
			pMeshShape->m_LocalBoundingBox.extents = (minMaxBox.m_max - minMaxBox.m_min) * half;
			pMeshShape->m_Filter.MergeFlags(m_CollisionFlag);
			pMeshShape->m_UserData = m_UserDataFlag;
		}
		else
		{
			pMeshShape->m_LocalBoundingBox.centre = Maths::Constants<Maths::HVec3>::Zero();
			pMeshShape->m_LocalBoundingBox.extents = pMeshShape->m_LocalBoundingBox.centre;
		}
		return pMeshShape;
	}

	
	Maths::HMat33 C_InstancedTriangleMeshDesc::InertiaTensor(const Maths::HF32 PCREF mass) const
	{
		//Mesh is immoveable, so it's inertia tensor is all zeros
		Maths::HMat33 temp(Maths::ZERO_MATRIX);
		return temp;
		//return C_InertiaTensorGenerator::BoxInertiaTensor(mass, m_pShape->m_AABBTree.m_High - m_pShape->m_AABBTree.m_Low);
	}

	
	Collision::C_CRAxisBox C_InstancedTriangleMeshDesc::GetBoundingBox() const
	{  
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		Collision::C_CRAxisBox box;
		box.centre = (m_pShape->m_AABBTree.m_High + m_pShape->m_AABBTree.m_Low) * half;
		box.extents = (m_pShape->m_AABBTree.m_High - m_pShape->m_AABBTree.m_Low) * half;
		return box;
	}


	/*
		C_ShapeDescWrapper
	*/

	Collision::C_ShapeWrapper C_ShapeDescWrapper::Serialise(Deserialiser& deserial, Collision::C_CollisionFilter* pFilter)
	{
		
		deserial >> m_RelativeTransformation;
		return Collision::C_ShapeWrapper(m_pShapeDesc->CreateShape(deserial, pFilter), m_RelativeTransformation);
	}
	

	
	Collision::C_Shape* C_ShapeDescWrapper::Shape(const Collision::C_CollisionFilter* pFilter, void* address)
	{
		//return Collision::C_ShapeWrapper(m_pShapeDesc->CreateShape(pFilter), m_RelativeTransformation);
		Collision::C_Shape* pShape = m_pShapeDesc->CreateShape(pFilter, address);
		pShape->m_RelativeTransformation = m_RelativeTransformation;
		return pShape;
	}

	/*
		C_ConvexHullDesc
	*/

	C_ConvexHullDesc::C_ConvexHullDesc(bool bShareData) : C_ShapeDesc(Collision::E_ConvexHull_Shape, Collision::E_ImportConvexHull_Shape),
		m_Points(32), m_Triangles(32), m_ShareData(bShareData), m_pBaseHull(NULL)
	{
		if(bShareData)
		{
			m_pBaseHull = new Collision::C_ConvexHull;
		}
	}

	C_ConvexHullDesc::~C_ConvexHullDesc()
	{
		delete m_pBaseHull;
		m_pBaseHull = NULL;
	}


	bool C_ConvexHullDesc::Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);
		//((C_ShapeDesc<Deserialiser>*)this)->Import(deserial);
		//C_ShapeDesc<Deserialiser>::Export<Serialiser>(serial);
		U32 numPoints, numTriangles;
		deserial >> numPoints >> numTriangles;
	/*	BIZ_ASSERT(numTriangles > 0 && numTriangles < 128);
		BIZ_ASSERT(numPoints > 0 && numPoints < 512);*/
		if(numTriangles > 128 || numPoints > 512)
		{
			LOGWARN(g_ImportLog, "Error - convex hull with %i polygons - more than 128 polygons! This must be reduced for SPU", numTriangles);
		}
		m_Points.Resize(numPoints);
		m_Triangles.Resize(numTriangles);
		m_Points.m_numElements = numPoints;
		m_Triangles.m_numElements = numTriangles;

		for(S32 i=0; i<m_Points.m_numElements; ++i)
		{
			deserial >> m_Points[i];
		}

		bool succeeded = true;
		for(S32 i=0; i<m_Triangles.m_numElements; ++i)
		{
			 m_Triangles[i].Deserialise(deserial);
			 bool con1 = m_Triangles[i].m_IndexA < (U32)m_Points.m_numElements;
			 bool con2 = m_Triangles[i].m_IndexB < (U32)m_Points.m_numElements;
			 bool con3 = m_Triangles[i].m_IndexC < (U32)m_Points.m_numElements;

			 if(!(con1&&con2&&con3))
			 {
				 succeeded = false;
			 }	 
			/* BIZ_ASSERT(m_Triangles[i].m_IndexA < m_Points.m_numElements);
			 BIZ_ASSERT(m_Triangles[i].m_IndexB < m_Points.m_numElements);
			 BIZ_ASSERT(m_Triangles[i].m_IndexC < m_Points.m_numElements);*/
		}

		if(m_ShareData)
		{
			CreateSharedData();
		}
		return succeeded;

	}


	void C_ConvexHullDesc::AddPoint(Maths::HVec3 PCREF point)
	{
		m_Points.AddToEnd(&point);
	}

	
	void C_ConvexHullDesc::AddTriangle(C_TriangleIndices tri)
	{
		BIZ_ASSERT((S32)tri.m_IndexA < m_Points.m_numElements);
		BIZ_ASSERT((S32)tri.m_IndexB < m_Points.m_numElements);
		BIZ_ASSERT((S32)tri.m_IndexC < m_Points.m_numElements);
		m_Triangles.AddToEnd(&tri);
	}

	void C_ConvexHullDesc::ApplyStaticRotation(const Maths::HMat33 PCREF rot)
	{
		for(S32 a = 0; a < m_Points.m_numElements; ++a)
		{
			m_Points[a] = m_Points[a] * rot;
		}
	}

	void C_ConvexHullDesc::RemapShape()
	{
		//Loop through and calculate the centre of the AABB around this mesh, then remap all vertices
		//to be around this local centre.
		if(!m_Remapped)
		{
			const Maths::HVec3 centre(CalculateCentreOfMass());

			for(S32 a = 0; a < m_Points.m_numElements; ++a)
			{
				m_Points[a] = m_Points[a] - centre;
			}
			m_RenderOffset = centre;
			//m_Remapped = true;
		}
	}

	Maths::HVec3 C_ConvexHullDesc::CalculateCentreOfMass()
	{
		Maths::HVec3 midPoint(Maths::Constants<Maths::HVec3>::Zero());
		if(m_Points.m_numElements > 0)
		{
			for(S32 a = 0; a < m_Points.m_numElements; ++a)
			{
				midPoint += m_Points[a];
			}
			midPoint /= Maths::HF32(F32(m_Points.m_numElements));
		}
		return midPoint;
	}

	/*void C_ConvexHullDesc::CreateSharedData()
	{
		m_pBaseHull->SetNumberVertices(m_Points.m_numElements);
		m_pBaseHull->SetNumberTriangles(m_Triangles.m_numElements);
		for(S32 a = 0; a < m_Points.m_numElements; ++a)
		{
			Maths::Store(m_Points[a], m_pBaseHull->m_pVertices + a);
		}
		for(S32 a = 0; a < m_Triangles.m_numElements; ++a)
		{
			m_pBaseHull->SetTriangles(m_Triangles[a].m_IndexA, m_Triangles[a].m_IndexB, m_Triangles[a].m_IndexC);
		}
		m_pBaseHull->FinaliseHull();
	}*/

	void C_ConvexHullDesc::CreateSharedData()
	{
		m_pBaseHull->SetNumberVertices(m_Points.m_numElements);
		m_pBaseHull->SetNumberTriangles(m_Triangles.m_numElements);
		
		Maths::HVec3 low(Maths::Constants<Maths::HVec3>::Max());
		Maths::HVec3 high(-Maths::Constants<Maths::HVec3>::Max());
		

		for(S32 a = 0; a < m_Points.m_numElements; ++a)
		{
			low = Maths::Min(m_Points[a], low);
			high = Maths::Max(m_Points[a], high);
		}
		m_pBaseHull->Low(low);
		m_pBaseHull->High(high);
		m_pBaseHull->Range(high - low);
		const Maths::HVec3 rangeRecip((high - low).Reciprocal());
		for(S32 a = 0; a < m_Points.m_numElements; ++a)
		{
			const Maths::HVec3 vert((m_Points[a] - low) * rangeRecip);
			//m_pBaseHull->m_pVertices[a] = (MathsStorage::Fixed3_S16<14>)vert;
			m_pBaseHull->SetVertex(vert, a);

			//Maths::Store(m_Points[a], m_pBaseHull->m_pVertices + a);
		}
		/*for(S32 a = 0; a < m_Points.m_numElements; ++a)
		{
			Maths::Store(m_Points[a], pHullShape->m_Hull.m_pVertices + a);
		}*/
		for(S32 a = 0; a < m_Triangles.m_numElements; ++a)
		{
			m_pBaseHull->SetTriangles(m_Triangles[a].m_IndexA, m_Triangles[a].m_IndexB, m_Triangles[a].m_IndexC);
		}
		m_pBaseHull->FinaliseHull();
	}

	
	//Collision::C_Shape* C_ConvexHullDesc::CreateShape(const Collision::C_CollisionFilter* pFilter) const
	//{
	//	Collision::C_ConvexHullShape* pHullShape = new Collision::C_ConvexHullShape(pFilter);//s_BoxAllocationManager.GetNextFreeItem();

	//	if(m_ShareData)
	//	{
	//		//CreateSharedData();
	//		pHullShape->m_Hull.SetNumberVertices(m_Points.m_numElements, m_pBaseHull->m_pVertices, m_pBaseHull->m_HillClimbingNodes);
	//		pHullShape->m_Hull.SetNumberTriangles(m_Triangles.m_numElements, m_pBaseHull->m_pTriangles);
	//		pHullShape->m_Hull.SetExternallyOwned(true);
	//	}
	//	else
	//	{
	//		pHullShape->m_Hull.SetNumberVertices(m_Points.m_numElements);
	//		pHullShape->m_Hull.SetNumberTriangles(m_Triangles.m_numElements);
	//		for(S32 a = 0; a < m_Points.m_numElements; ++a)
	//		{
	//			Maths::Store(m_Points[a], pHullShape->m_Hull.m_pVertices + a);
	//		}
	//		for(S32 a = 0; a < m_Triangles.m_numElements; ++a)
	//		{
	//			pHullShape->m_Hull.SetTriangles(m_Triangles[a].m_IndexA, m_Triangles[a].m_IndexB, m_Triangles[a].m_IndexC);
	//		}
	//		pHullShape->m_Hull.FinaliseHull();
	//	}
	//	pHullShape->m_LocalBoundingBox = GetBoundingBox();
	//	pHullShape->m_Hull.Margin(this->m_Margin);
	//	pHullShape->m_Hull.Epsilon(this->m_Epsilon);
	//	pHullShape->m_ElementOffset = m_RenderOffset;
	//	pHullShape->m_Filter.MergeFlags(m_CollisionFlag);
	//	pHullShape->m_Hull.CalculateExtents();
	//	pHullShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
	//	pHullShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
	//	pHullShape->m_UserData = m_UserDataFlag;
	//	return pHullShape;
	//}

	Collision::C_Shape* C_ConvexHullDesc::CreateShape(const Collision::C_CollisionFilter* pFilter) const
	{
		PERF_EVENT("C_ConvexHullDesc::CreateShape");
		Collision::C_ConvexHullShape* pHullShape = new Collision::C_ConvexHullShape(pFilter);//s_BoxAllocationManager.GetNextFreeItem();

		if(m_ShareData)
		{
			//CreateSharedData();
			/*pHullShape->m_Hull.SetNumberVertices(m_Points.m_numElements, m_pBaseHull->m_pVertices, m_pBaseHull->m_HillClimbingNodes, m_pBaseHull->m_pNeighbours,
				m_pBaseHull->m_NumNeighbours);*/
			pHullShape->m_Hull.SetNumberVertices(m_Points.m_numElements, m_pBaseHull->GetVertices(), m_pBaseHull->GetHillClimbingNodes(), m_pBaseHull->GetNeighbours(),
				m_pBaseHull->GetNumNeighbours());
			//pHullShape->m_Hull.SetNumberTriangles(m_Triangles.m_numElements, m_pBaseHull->m_pTriangles);

			pHullShape->m_Hull.SetNumberTriangles(m_Triangles.m_numElements, m_pBaseHull->GetTriangles());

			pHullShape->m_Hull.SetExternallyOwned(true);
			pHullShape->m_Hull.Low(m_pBaseHull->Low());
			pHullShape->m_Hull.High(m_pBaseHull->High());
			pHullShape->m_Hull.Range(m_pBaseHull->Range());
		}
		else
		{
			pHullShape->m_Hull.SetNumberVertices(m_Points.m_numElements);
			pHullShape->m_Hull.SetNumberTriangles(m_Triangles.m_numElements);
			
			Maths::HVec3 low(Maths::Constants<Maths::HVec3>::Max());
			Maths::HVec3 high(-Maths::Constants<Maths::HVec3>::Max());
			

			for(S32 a = 0; a < m_Points.m_numElements; ++a)
			{
				low = Maths::Min(m_Points[a], low);
				high = Maths::Max(m_Points[a], high);
			}
			pHullShape->m_Hull.Low(low);
			pHullShape->m_Hull.High(high);
			pHullShape->m_Hull.Range(high - low);
			const Maths::HVec3 rangeRecip((high - low).Reciprocal());
			for(S32 a = 0; a < m_Points.m_numElements; ++a)
			{
				const Maths::HVec3 vert((m_Points[a] - low) * rangeRecip);
				//pHullShape->m_Hull.m_pVertices[a] = (MathsStorage::Fixed3_S16<14>)vert;
				pHullShape->m_Hull.SetVertex(vert, a);

				//Maths::Store(m_Points[a], m_pBaseHull->m_pVertices + a);
			}
			/*for(S32 a = 0; a < m_Points.m_numElements; ++a)
			{
				Maths::Store(m_Points[a], pHullShape->m_Hull.m_pVertices + a);
			}*/
			for(S32 a = 0; a < m_Triangles.m_numElements; ++a)
			{
				pHullShape->m_Hull.SetTriangles(m_Triangles[a].m_IndexA, m_Triangles[a].m_IndexB, m_Triangles[a].m_IndexC);
			}
			pHullShape->m_Hull.FinaliseHull();
		}
		pHullShape->m_LocalBoundingBox = GetBoundingBox();
		pHullShape->m_Hull.Margin(this->m_Margin);
		pHullShape->m_Hull.Epsilon(this->m_Epsilon);
		pHullShape->m_ElementOffset = m_RenderOffset;
		pHullShape->m_Filter.MergeFlags(m_CollisionFlag);
		pHullShape->m_Hull.CalculateExtents();
		pHullShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
		pHullShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
		pHullShape->m_UserData = m_UserDataFlag;
		return pHullShape;
	}

	Collision::C_Shape* C_ConvexHullDesc::CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const
	{
		PERF_EVENT("C_ConvexHullDesc::CreateShape");
		Collision::C_ConvexHullShape* pHullShape = new (pAddress)Collision::C_ConvexHullShape(pFilter);//s_BoxAllocationManager.GetNextFreeItem();

		if(m_ShareData)
		{
			//CreateSharedData();
			/*pHullShape->m_Hull.SetNumberVertices(m_Points.m_numElements, m_pBaseHull->m_pVertices, m_pBaseHull->m_HillClimbingNodes, m_pBaseHull->m_pNeighbours,
				m_pBaseHull->m_NumNeighbours);*/
			pHullShape->m_Hull.SetNumberVertices(m_Points.m_numElements, m_pBaseHull->GetVertices(), m_pBaseHull->GetHillClimbingNodes(), m_pBaseHull->GetNeighbours(),
				m_pBaseHull->GetNumNeighbours());
			//pHullShape->m_Hull.SetNumberTriangles(m_Triangles.m_numElements, m_pBaseHull->m_pTriangles);

			pHullShape->m_Hull.SetNumberTriangles(m_Triangles.m_numElements, m_pBaseHull->GetTriangles());

			pHullShape->m_Hull.SetExternallyOwned(true);
			pHullShape->m_Hull.Low(m_pBaseHull->Low());
			pHullShape->m_Hull.High(m_pBaseHull->High());
			pHullShape->m_Hull.Range(m_pBaseHull->Range());
		}
		else
		{
			pHullShape->m_Hull.SetNumberVertices(m_Points.m_numElements);
			pHullShape->m_Hull.SetNumberTriangles(m_Triangles.m_numElements);
			
			Maths::HVec3 low(Maths::Constants<Maths::HVec3>::Max());
			Maths::HVec3 high(-Maths::Constants<Maths::HVec3>::Max());
			

			for(S32 a = 0; a < m_Points.m_numElements; ++a)
			{
				low = Maths::Min(m_Points[a], low);
				high = Maths::Max(m_Points[a], high);
			}
			pHullShape->m_Hull.Low(low);
			pHullShape->m_Hull.High(high);
			pHullShape->m_Hull.Range(high - low);
			const Maths::HVec3 rangeRecip((high - low).Reciprocal());
			for(S32 a = 0; a < m_Points.m_numElements; ++a)
			{
				const Maths::HVec3 vert((m_Points[a] - low) * rangeRecip);
				//pHullShape->m_Hull.m_pVertices[a] = (MathsStorage::Fixed3_S16<14>)vert;
				pHullShape->m_Hull.SetVertex(vert, a);

				//Maths::Store(m_Points[a], m_pBaseHull->m_pVertices + a);
			}
			/*for(S32 a = 0; a < m_Points.m_numElements; ++a)
			{
				Maths::Store(m_Points[a], pHullShape->m_Hull.m_pVertices + a);
			}*/
			for(S32 a = 0; a < m_Triangles.m_numElements; ++a)
			{
				pHullShape->m_Hull.SetTriangles(m_Triangles[a].m_IndexA, m_Triangles[a].m_IndexB, m_Triangles[a].m_IndexC);
			}
			pHullShape->m_Hull.FinaliseHull();
		}
		pHullShape->m_LocalBoundingBox = GetBoundingBox();
		pHullShape->m_Hull.Margin(this->m_Margin);
		pHullShape->m_Hull.Epsilon(this->m_Epsilon);
		pHullShape->m_ElementOffset = m_RenderOffset;
		pHullShape->m_Filter.MergeFlags(m_CollisionFlag);
		pHullShape->m_Hull.CalculateExtents();
		pHullShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
		pHullShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
		pHullShape->m_UserData = m_UserDataFlag;
		return pHullShape;
	}

	
	Collision::C_Shape* C_ConvexHullDesc::CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter)
	{	
		PERF_EVENT("C_ConvexHullDesc::CreateShape");
		C_ShapeDesc::ImportBasic(deserial);
		U32 numPoints;
		U32 numTriangles;
		deserial >> numPoints >> numTriangles;

		m_Points.m_numElements = 0;
		Maths::HVec3 point(Maths::UNDEFINED);
		for(U32 a = 0; a < numPoints; ++a)
		{	
			deserial >> point;
			m_Points.AddToEnd(&point);
		}
		Collision::C_ConvexHullShape* pHullShape = new Collision::C_ConvexHullShape(pFilter);//s_BoxAllocationManager.GetNextFreeItem();

		pHullShape->m_Hull.SetNumberVertices(m_Points.m_numElements);
		
		Maths::HVec3 low(Maths::Constants<Maths::HVec3>::Max());
		Maths::HVec3 high(-Maths::Constants<Maths::HVec3>::Max());
		for(S32 a = 0; a < m_Points.m_numElements; ++a)
		{
			low = Maths::Min(m_Points[a], low);
			high = Maths::Max(m_Points[a], high);
		}
		pHullShape->m_Hull.Low(low);
		pHullShape->m_Hull.High(high);
		pHullShape->m_Hull.Range(high - low);

		const Maths::HVec3 rangeRecip((high - low).Reciprocal());
		for(S32 a = 0; a < m_Points.m_numElements; ++a)
		{
			const Maths::HVec3 vert((m_Points[a] - low) * rangeRecip);
			//pHullShape->m_Hull.m_pVertices[a] = (MathsStorage::Fixed3_S16<14>)vert;
			pHullShape->m_Hull.SetVertex(vert, a);
		}
		/*for(S32 a = 0; a < m_Points.m_numElements; ++a)
		{
			Maths::Store(m_Points[a], pHullShape->m_Hull.m_pVertices + a);
		}*/
		
		pHullShape->m_Hull.SetNumberTriangles(numTriangles);
		for(U32 a=0; a< numTriangles; ++a)
		{
			S32 indexA, indexB, indexC;
			deserial >> indexA >> indexB >> indexC;
			BIZ_ASSERT(indexA < m_Points.m_numElements);
			BIZ_ASSERT(indexB < m_Points.m_numElements);
			BIZ_ASSERT(indexC < m_Points.m_numElements);
			pHullShape->m_Hull.SetTriangles(indexA, indexB, indexC);
		}
		pHullShape->m_Hull.FinaliseHull();
		pHullShape->m_Hull.Margin(this->m_Margin);
		pHullShape->m_LocalBoundingBox = GetBoundingBox();
		pHullShape->m_Hull.Epsilon(this->m_Epsilon);
		pHullShape->m_ElementOffset = m_RenderOffset;
		pHullShape->m_Filter.MergeFlags(m_CollisionFlag);
		pHullShape->m_Hull.CalculateExtents();
		pHullShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
		pHullShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
		pHullShape->m_UserData = m_UserDataFlag;
		return pHullShape;
	}

	Maths::HMat33 C_CharacterControllerDesc::InertiaTensor(const Maths::HF32 PCREF mass) const
	{
		return C_InertiaTensorGenerator::CharacterInertiaTensor(mass, m_CapsuleRadius, m_CapsuleHeight);
	}

	
	Maths::HMat33 C_ConvexHullDesc::InertiaTensor(const Maths::HF32 PCREF mass) const
	{
		//Caclualte the inertia tensor for a box!
		return C_InertiaTensorGenerator::ConvexHullInertiaTensor(mass, m_Points);
	}

	
	Collision::C_CRAxisBox C_ConvexHullDesc::GetBoundingBox() const
	{
		//Maths::HVec3 extents(Maths::Constants<Maths::HVec3>::Zero());
		Maths::HVec3 _max(-Maths::Constants<Maths::HVec3>::Max());
		Maths::HVec3 _min(Maths::Constants<Maths::HVec3>::Max());
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());

		for(S32 a = 0; a < m_Points.m_numElements; ++a)
		{
			Maths::HVec3& p(m_Points[a]);
			//extents = Maths::Max(Maths::Abs(p), extents);
			_max = Maths::Max(p, _max);
			_min = Maths::Min(p, _min);
		}
		Collision::C_CRAxisBox box;
		box.centre = (_max + _min) * half;
		box.extents = (_max - _min) * half + Maths::HVec3(m_Margin);
		return box;
	}



	/*
		C_CompositeDesc
	*/
	
	void C_CompositeDesc::AddShape(C_ShapeDesc* pDesc, const Maths::HMat43 PCREF relativeOrientation, const Maths::HF32 PCREF mass,
		Collision::C_CollisionFilter* pFilter)
	{

		if(pDesc->ValidShapeDescriptor())
		{
			C_ShapeDescWrapper wrapper;
			wrapper.m_pShapeDesc = pDesc;
			wrapper.m_Mass=mass;
			wrapper.m_RelativeTransformation = relativeOrientation;
			wrapper.m_pFilter = pFilter;
			m_CollisionFlag |= pDesc->m_CollisionFlag;
			shapes.AddToEnd(&wrapper);
		}
	}

	void C_CompositeDesc::ApplyStaticRotation(const Maths::HMat33 PCREF rot)
	{
		const Maths::HMat43 trans(rot, Maths::Constants<Maths::HVec3>::Zero());
		for(S32 a = 0; a < shapes.m_numElements; ++a)
		{
			C_ShapeDescWrapper& wrapper = shapes[a];
			wrapper.m_RelativeTransformation = wrapper.m_RelativeTransformation * trans;
		}
	}

	Maths::HF32 C_CompositeDesc::CalculateVolume()
	{
		Maths::HF32 totalVolume(Maths::Constants<Maths::HF32>::Zero());
		for(S32 a = 0; a < this->shapes.m_numElements; ++a)
		{
			totalVolume += shapes[a].m_pShapeDesc->CalculateVolume();
		}
		return totalVolume;
	}

	void C_CompositeDesc::CalculateNormalisedMasses(const Maths::HF32 PCREF totalMass)
	{
		const Maths::HF32 totalVolume(CalculateVolume());
		const Maths::HF32 density(totalMass/totalVolume);
		for(S32 a = 0; a < shapes.m_numElements; ++a)
		{
			const Maths::HF32 volume(shapes[a].m_pShapeDesc->CalculateVolume());
			shapes[a].m_Mass = shapes[a].m_pShapeDesc->m_Mass = volume * density;
		}
		this->m_Mass = totalMass;
	}
	
	void C_CompositeDesc::AddShape(C_ShapeDesc* pDesc, const Maths::HMat43 PCREF relativeOrientation, const Maths::HF32 PCREF mass)
	{

		if(pDesc->ValidShapeDescriptor())
		{
			C_ShapeDescWrapper wrapper;
			wrapper.m_pShapeDesc = pDesc;
			wrapper.m_Mass=mass;
			wrapper.m_RelativeTransformation = relativeOrientation;
			m_CollisionFlag |= pDesc->m_CollisionFlag;
			shapes.AddToEnd(&wrapper);
		}
	}

	
	void C_CompositeDesc::AddOwnShape(C_ShapeDesc* pDesc, const Maths::HMat43 PCREF relativeOrientation, const Maths::HMat33 PCREF ineriaTensor, const Maths::HF32 PCREF mass)
	{
		//if(pDesc->ValidShapeDescriptor())
		{
			C_ShapeDescWrapper wrapper;
			wrapper.m_pShapeDesc = pDesc;
			wrapper.m_Mass=mass;
			wrapper.m_RelativeTransformation = relativeOrientation;
			wrapper.m_IneriaTensor = ineriaTensor; 
			shapes.AddToEnd(&wrapper);

			m_OwnedShapes.AddToEnd(&pDesc);
			m_CollisionFlag |= pDesc->m_CollisionFlag;
		}
	}

	void C_CompositeDesc::SetRemapped()
	{
		m_Remapped = true;
		for(S32 a = 0; a < shapes.m_numElements; ++a)
		{
			shapes[a].m_pShapeDesc->SetRemapped();
		}
	}

	void C_CompositeDesc::RemapShape()
	{
		//(1) - Remap the sub-shapes so that they are centred around their respective centres of mass!
		if(!m_Remapped)
		{
			//(2) - Calculate the centre of mass of this shape
			const Maths::HVec3 centre = CalculateCentreOfMass();
			//m_RenderOffset = centre;

			//(3) - Remap the relative transformations such that they are declared around this new centre of mass

			for(S32 a = 0; a < shapes.m_numElements; ++a)
			{
				C_ShapeDescWrapper& wrapper = shapes[a];
				//if(wrapper.m_pShapeDesc->HasCentreOfMass())
				{
					//Remap its relative transformation around this centre of mass...NB - the 
					//shapes without centres of mass can't have this done, because they are static meshes!
					//Maths::HVec3 oldTranslation = wrapper.m_RelativeTransformation.Translation();
					//if(!shapes[a].m_pShapeDesc->m_Remapped)
					{
						const Maths::HVec3 relCentre(((wrapper.m_pShapeDesc->CalculateCentreOfMass())*wrapper.m_RelativeTransformation)-centre);
						wrapper.m_RelativeTransformation.Translation(relCentre);
					}
					/*else
					{
						const Maths::HVec3 relCentre(((wrapper.m_pShapeDesc->CalculateCentreOfMass() - wrapper.m_pShapeDesc->m_RenderOffset)*wrapper.m_RelativeTransformation)-centre);
						wrapper.m_RelativeTransformation.Translation(relCentre);
					}*/
				}
			}
			this->m_RenderOffset = centre;
			for(S32 a = 0; a < shapes.m_numElements; ++a)
			{
				shapes[a].m_pShapeDesc->RemapShape();
			}
			//m_Remapped = true;
		}
	}

	Maths::HVec3 C_CompositeDesc::CalculateCentreOfMass()
	{
		Maths::HVec3 centre(Maths::Constants<Maths::HVec3>::Zero());
		Maths::HF32 totalMass(Maths::Constants<Maths::HF32>::Zero());
		U32 numContributingShapes = 0;
		for(S32 a = 0; a < shapes.m_numElements; ++a)
		{
			C_ShapeDescWrapper& wrapper = shapes[a];
			if(wrapper.m_pShapeDesc->HasCentreOfMass())
			{
				++numContributingShapes;
				centre += ((shapes[a].m_pShapeDesc->CalculateCentreOfMass())* shapes[a].m_RelativeTransformation) * shapes[a].m_Mass;
				totalMass += shapes[a].m_Mass;
			}
		}

		if(numContributingShapes > 0)
		{
//			Maths::HF32 denom = Maths::HF32(F32(numContributingShapes)) * totalMass;
			centre = centre / totalMass;
		}

		return centre;

	}


	/*
		This method used in the exporter
	*/
	
	
	Collision::C_Shape* C_CompositeDesc::CreateShape(const Collision::C_CollisionFilter* pFilter) const
	{
		PERF_EVENT("C_CompositeDesc::CreateShape");
		if(m_DecomposeSingleComposite && shapes.m_numElements == 1)
		{
			C_ShapeDescWrapper& pShapeDesc=shapes[0];
			return pShapeDesc.m_pShapeDesc->CreateShape(pFilter);
		}
		Collision::C_CompositeShape* pCompositeShape=new Collision::C_CompositeShape(pFilter, GetCompositeSize(), shapes.m_numElements);
		U8* pData = pCompositeShape->m_pShapes;
		//pCompositeShape->SetNumberShapes(shapes.m_numElements);
		U32 shapeCount = 0;
		for(S32 i=0; i<shapes.m_numElements; ++i)
		{
			C_ShapeDescWrapper& pShapeDesc=shapes[i];
			//Each sub shape should has its own collision rule, but just a quick
			//hack, we set each collision filter is the same
			if(pShapeDesc.m_pShapeDesc->ValidShapeDescriptor())
			{
				Collision::C_Shape* pSubShape = pShapeDesc.Shape(pFilter, pData);
				pData += pShapeDesc.m_pShapeDesc->GetSize();
				pSubShape->SetIndex(shapeCount++);

				/*Collision::C_ShapeWrapper shapeWrapper=pShapeDesc.Shape(pFilter);
				shapeWrapper.m_pShape->SetHasBuoyancy(pShapeDesc.m_pShapeDesc->m_HasVolume);
				pCompositeShape->AddShape(shapeWrapper);*/
			}
		}
		pCompositeShape->m_NumShapes = shapeCount;
		pCompositeShape->m_CompositeSize = pData - pCompositeShape->m_pShapes;
		pCompositeShape->m_LocalBoundingBox = GetBoundingBox();
		//pCompositeShape->m_Radius = pCompositeShape->m_LocalBoundingBox.extents.Length();
		pCompositeShape->m_ElementOffset = m_RenderOffset;
		pCompositeShape->m_Filter.MergeFlags(m_CollisionFlag);
		pCompositeShape->m_UserData = m_UserDataFlag;

		if(pCompositeShape->m_NumShapes == 0)
		{
			delete pCompositeShape;
			return NULL;
		}
		return pCompositeShape;
	}

	Collision::C_Shape* C_CompositeDesc::CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const
	{
		PERF_EVENT("C_CompositeDesc::CreateShape");
		if(m_DecomposeSingleComposite && shapes.m_numElements == 1)
		{
			C_ShapeDescWrapper& pShapeDesc=shapes[0];
			return pShapeDesc.m_pShapeDesc->CreateShape(pFilter, pAddress);
		}
		Collision::C_CompositeShape* pCompositeShape=new (pAddress)Collision::C_CompositeShape(pFilter, GetCompositeSize(), shapes.m_numElements);
		//pCompositeShape->SetNumberShapes(shapes.m_numElements);
		//C_CompositeShapeIterator iter(pCompositeShape->m_pShapes, pCompositeShape->m_CompositeSize);
		U8* pData = pCompositeShape->m_pShapes;
		U32 shapeCount = 0;
		for(S32 i=0; i<shapes.m_numElements; ++i)
		{
			C_ShapeDescWrapper& pShapeDesc=shapes[i];
			//Each sub shape should has its own collision rule, but just a quick
			//hack, we set each collision filter is the same
			if(pShapeDesc.m_pShapeDesc->ValidShapeDescriptor())
			{
				Collision::C_Shape* pSubShape = pShapeDesc.Shape(pFilter, pData);
				pData += pShapeDesc.m_pShapeDesc->GetSize();
				pSubShape->SetIndex(shapeCount++);
				/*Collision::C_ShapeWrapper shapeWrapper=pShapeDesc.Shape(pFilter);
				shapeWrapper.m_pShape->SetHasBuoyancy(pShapeDesc.m_pShapeDesc->m_HasVolume);
				pCompositeShape->AddShape(shapeWrapper);*/
			}
		}
		pCompositeShape->m_NumShapes = shapeCount;
		pCompositeShape->m_CompositeSize = (pData - pCompositeShape->m_pShapes);
		pCompositeShape->m_LocalBoundingBox = GetBoundingBox();
		//pCompositeShape->m_Radius = pCompositeShape->m_LocalBoundingBox.extents.Length();
		pCompositeShape->m_ElementOffset = m_RenderOffset;
		pCompositeShape->m_Filter.MergeFlags(m_CollisionFlag);
		pCompositeShape->m_UserData = m_UserDataFlag;

		/*if(pCompositeShape->m_NumShapes == 0)
		{
			delete pCompositeShape;
			return NULL;
		}*/
		return pCompositeShape;
	}

	
	Collision::C_Shape* C_CompositeDesc::CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter)
	{	
		//C_ShapeDesc::ImportBasic(deserial);
		//S32 numShapes;
		//Collision::C_CompositeShape* pCompositeShape = new Collision::C_CompositeShape(pFilter, GetCompositeSize());//s_BoxAllocationMaanger.GetNextFreeItem();
		//deserial >> numShapes;
		//pCompositeShape->SetNumberShapes(numShapes);
		//for(S32 a = 0; a < numShapes; ++a)
		//{
		//	Gravity::C_ShapeDescWrapper wrapper;//=shapes[a];

		//	U32 m_ImportType = 0;
		//	deserial >> m_ImportType;
		//	Collision::C_Shape* shape = 0;
		//
		//	switch(m_ImportType)
		//	{
		//	case Collision::E_ImportTriangleMesh_Shape:
		//		{
		//			S32 m_id;
		//			deserial >> m_id;
		//			Gravity::C_TriangleMeshBaseDesc* triDes=new Gravity::C_TriangleMeshBaseDesc;
		//			Collision::C_TriangleMeshShape* tri=(Collision::C_TriangleMeshShape*)triDes->CreateShape(deserial, pFilter);
		//			/*Collision::C_TriangleStreamIterator iter(tri->m_AABBTree.m_Triangles, tri->m_AABBTree.m_NumTriangles);
		//			// Collision::C_TriangleHeader* header(NULL);
		//			while( (header = iter.NextHeader()))
		//			{
		//				header->m_CollisionFlags = 0xffffffff;
		//			}*/
		//			m_instanceMeshManager->AddBase(tri, m_id);

		//			Gravity::C_InstancedTriangleMeshDesc* pInst=new Gravity::C_InstancedTriangleMeshDesc;
		//			pInst->m_BaseNodeId = m_id;
		//			pInst->m_pShape = tri;
		//			wrapper.m_pShapeDesc = pInst;
		//			shape=pInst->CreateShape(deserial, pFilter);
		//			m_OwnedShapes.AddToEnd((Gravity::C_ShapeDesc**)&pInst);
		//			break;
		//		}
		//	case Collision::E_ImportBox_Shape:
		//		{
		//			
		//			Gravity::C_BoxDesc* box = new Gravity::C_BoxDesc;
		//			wrapper.m_pShapeDesc = box;
		//			shape=box->CreateShape(deserial, pFilter);
		//			m_OwnedShapes.AddToEnd((Gravity::C_ShapeDesc**)&box);
		//			break;
		//		}
		//	case Collision::E_ImportConvexHull_Shape:
		//		{
		//			Gravity::C_ConvexHullDesc* convex = new Gravity::C_ConvexHullDesc;
		//			wrapper.m_pShapeDesc = convex;
		//			shape=convex->CreateShape(deserial, pFilter);
		//			m_OwnedShapes.AddToEnd((Gravity::C_ShapeDesc**)&convex);
		//			break;
		//		}
		//	case Collision::E_ImportSphere_Shape:
		//		{
		//			Gravity::C_SphereDesc* sphere = new Gravity::C_SphereDesc;
		//			wrapper.m_pShapeDesc = sphere;
		//			shape=sphere->CreateShape(deserial, pFilter);
		//			m_OwnedShapes.AddToEnd((Gravity::C_ShapeDesc**)&sphere);
		//			break;
		//		}
		//	case Collision::E_ImportCapsule_Shape:
		//		{
		//			Gravity::C_CapsuleDesc* capsule=new Gravity::C_CapsuleDesc;
		//			wrapper.m_pShapeDesc = capsule;
		//			shape=capsule->CreateShape(deserial, pFilter);
		//			m_OwnedShapes.AddToEnd((Gravity::C_ShapeDesc**)&capsule);
		//			break;
		//		}
		//	case Collision::E_ImportCylinder_Shape:
		//		{
		//			Gravity::C_CylinderDesc* cylinder=new Gravity::C_CylinderDesc;
		//			wrapper.m_pShapeDesc=cylinder;
		//			shape=cylinder->CreateShape(deserial, pFilter);
		//			m_OwnedShapes.AddToEnd((Gravity::C_ShapeDesc**)&cylinder);
		//			break;
		//		}
		//	case Collision::E_ImportComposite_Shape:
		//		{
		//			Gravity::C_CompositeDesc* composite=new Gravity::C_CompositeDesc;
		//			wrapper.m_pShapeDesc=composite;
		//			shape=composite->CreateShape(deserial, pFilter);
		//			m_OwnedShapes.AddToEnd((Gravity::C_ShapeDesc**)&composite);
		//			break;
		//		}
		//	}
		//	
		//	deserial >> wrapper.m_Mass >> wrapper.m_RelativeTransformation >> wrapper.m_IneriaTensor;
		//	shapes.AddToEnd(&wrapper);
		//	Collision::C_ShapeWrapper shapeWrapper(shape, wrapper.m_RelativeTransformation);
		//	pCompositeShape->AddShape(shapeWrapper);
		//	
		//	/*Gravity::C_ShapeDescWrapper<Deserialiser> wrapper;
		//	Collision::C_ShapeWrapper shapeWrapper((wrapper.Serialise(deserial, pFilter)));
		//	pCompositeShape->AddShape(shapeWrapper);
		//	*/
		//}
		//pCompositeShape->m_LocalBoundingBox = GetBoundingBox();
		//pCompositeShape->m_ElementOffset = m_RenderOffset;
		//pCompositeShape->m_Filter.MergeFlags(m_CollisionFlag);
		//pCompositeShape->m_UserData = m_UserDataFlag;
		//return pCompositeShape;
		BIZ_ASSERT(0);
		return NULL;
	}

	
	Maths::HMat33 C_CompositeDesc::InertiaTensor(const Maths::HF32 PCREF mass) const
	{
		const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
		
		Maths::HF32 totalMass(Maths::Constants<Maths::HF32>::Zero());
		Maths::HMat33 inertiaTensor(Maths::ZERO_MATRIX);
		////Caclualte the inertia tensor for a box!
		for(S32 i=0; i<shapes.m_numElements; ++i)
		{
			C_ShapeDescWrapper& wrapper2=shapes[i];
			if(wrapper2.m_pShapeDesc->HasCentreOfMass())
			{
				Maths::HMat33 inertia(wrapper2.m_pShapeDesc->InertiaTensor(wrapper2.m_Mass));
	#ifdef PARALLEL_AXIS_INERTIA
				const Maths::HMat33 orientation(wrapper2.m_RelativeTransformation.Rotation());
				inertia = orientation.Transposed() * inertia * orientation;
				inertiaTensor = inertiaTensor + Gravity::C_InertiaTensorGenerator::ParallelAxis(inertia, wrapper2.m_RelativeTransformation.Translation(), wrapper2.m_Mass);
	#else			
				inertiaTensor = inertiaTensor + inertia;
	#endif
				totalMass += wrapper2.m_Mass;
			}
		}
		return inertiaTensor * mass/totalMass;

		/*Collision::C_CRAxisBox box(GetBoundingBox());
		return C_InertiaTensorGenerator::BoxInertiaTensor(mass, box.extents);*/

	}

	Collision::C_CRAxisBox C_CompositeDesc::GetBoundingBox() const
	{
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		Collision::C_AxisBoxMinMax box;
		box.m_max = -Maths::Constants<Maths::HVec3>::Max();
		box.m_min = Maths::Constants<Maths::HVec3>::Max();
		for(S32 a = 0; a < shapes.m_numElements; ++a)
		{
			if(shapes[a].m_pShapeDesc->ValidShapeDescriptor())
			{
				Collision::C_CRAxisBox oBox;
				shapes[a].m_pShapeDesc->GetBoundingBox().Transform(shapes[a].m_RelativeTransformation, oBox);

				//box.MergeBox(oBox);
				const Maths::HVec3 _max(oBox.centre + oBox.extents);
				const Maths::HVec3 _min(oBox.centre - oBox.extents);

				box.m_max = Maths::Max(box.m_max, _max);
				box.m_min = Maths::Min(box.m_min, _min);

			}
		}

		Collision::C_CRAxisBox retBox;
		retBox.centre = (box.m_max + box.m_min) * half;
		retBox.extents = (box.m_max - box.m_min) * half;
		return retBox;
	}

	
	Collision::C_Shape* C_CharacterControllerDesc::CreateShape(const Collision::C_CollisionFilter* pFilter) const
	{
		PERF_EVENT("C_CharacterControllerDesc::CreateShape");
		Collision::C_CharacterControllerShape* pShape = new Collision::C_CharacterControllerShape(pFilter);
		pShape->m_Controller = Collision::C_CharacterControllerPrimitive(m_CapsuleHeight, m_CapsuleRadius, m_RayHeight, m_MaxCosAngle, m_RayRadius, m_MinPen, m_FeetMaterial, m_EffectMaterial, pFilter);
		pShape->m_LocalBoundingBox = GetBoundingBox();
		pShape->m_Filter.MergeFlags(m_CollisionFlag);
		pShape->m_UserData = m_UserDataFlag;
		return pShape;
	}

	Collision::C_Shape* C_CharacterControllerDesc::CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const
	{
		BIZ_ASSERT(0);
		return NULL;
	}

	
	Collision::C_Shape* C_CharacterControllerDesc::CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter)
	{
		C_ShapeDesc::ImportBasic(deserial);
		deserial >> m_CapsuleHeight >> m_CapsuleRadius >> m_RayHeight >>  m_RayRadius >> m_MaxCosAngle >> m_MinPen >>m_FeetMaterial>>m_EffectMaterial;
		Collision::C_CharacterControllerShape* pShape = new Collision::C_CharacterControllerShape(pFilter);
		pShape->m_Controller = Collision::C_CharacterControllerPrimitive(m_CapsuleHeight, m_CapsuleRadius, m_RayHeight, m_MaxCosAngle, m_RayRadius, m_MinPen, m_FeetMaterial, m_EffectMaterial, pFilter);
		pShape->m_LocalBoundingBox = GetBoundingBox();
		pShape->m_Filter.MergeFlags(m_CollisionFlag);
		pShape->m_UserData = m_UserDataFlag;
		return pShape;
	}

#ifndef CFG_SPEED
#pragma optimize("", off)
#endif
	bool C_ShapeDescWrapper::Import(Deserialiser& deserial, SharedPhysics::SortedArray<C_ShapeDesc* >& ownedShapes, Gravity::C_InstanceMeshResourceManager* pInstMeshManager, bool bShareData)
	{
		Collision::E_ImportShapeType m_ImportType;
		deserial >> (U32&)m_ImportType;

		/*Collision::E_ShapeType m_Type;
		deserial >> (U32&)m_Type;*/

		bool succeeded = false;

		switch(m_ImportType)
		{
		case Collision::E_ImportBox_Shape:
			{
				m_pShapeDesc=new Gravity::C_BoxDesc;
				succeeded = m_pShapeDesc->Import(deserial);
				ownedShapes.AddToEnd(&m_pShapeDesc);
				deserial >> m_Mass;
				deserial >> m_RelativeTransformation;
				deserial >> m_IneriaTensor;
				break;
			}
		case Collision::E_ImportConvexHull_Shape:
			{
				m_pShapeDesc=new Gravity::C_ConvexHullDesc(bShareData);
				succeeded =m_pShapeDesc->Import(deserial);
				ownedShapes.AddToEnd(&m_pShapeDesc);
				deserial >> m_Mass;
				deserial >> m_RelativeTransformation;
				deserial >> m_IneriaTensor;
				break;
			}
		case Collision::E_ImportSphere_Shape:
			{
				m_pShapeDesc=new Gravity::C_SphereDesc;
				succeeded = m_pShapeDesc->Import(deserial);
				ownedShapes.AddToEnd(&m_pShapeDesc);
				deserial >> m_Mass;
				deserial >> m_RelativeTransformation;
				deserial >> m_IneriaTensor;
				break;
			}
		case Collision::E_ImportCapsule_Shape:
			{
				m_pShapeDesc=new Gravity::C_CapsuleDesc;
				succeeded = m_pShapeDesc->Import(deserial);
				ownedShapes.AddToEnd(&m_pShapeDesc);
				deserial >> m_Mass;
				deserial >> m_RelativeTransformation;
				deserial >> m_IneriaTensor;
				break;
			}
		case Collision::E_ImportCylinder_Shape:
			{
				m_pShapeDesc=new Gravity::C_CylinderDesc;
				succeeded =m_pShapeDesc->Import(deserial);
				ownedShapes.AddToEnd(&m_pShapeDesc);
				deserial >> m_Mass;
				deserial >> m_RelativeTransformation;
				deserial >> m_IneriaTensor;
				break;
			}
		case Collision::E_ImportComposite_Shape:
			{
				Gravity::C_CompositeDesc* pDesc = new Gravity::C_CompositeDesc;
				pDesc->m_ShareData = bShareData;
				m_pShapeDesc=pDesc;
				succeeded =m_pShapeDesc->Import(deserial);
				ownedShapes.AddToEnd(&m_pShapeDesc);
				deserial >> m_Mass;
				deserial >> m_RelativeTransformation;
				deserial >> m_IneriaTensor;
				break;
			}
		case Collision::E_ImportTriangleMesh_Shape:
			{
				//m_pShapeDesc=new Gravity::C_TriangleMeshDesc<Deserialiser>;
				/*
					In the compound shape, we depress all the mesh into just one big mesh, then set this mesh
					as the basic mesh
				*/
				Gravity::C_TriangleMeshBaseDesc* pInstBase= new Gravity::C_TriangleMeshBaseDesc;
				Collision::C_CollisionFilter filter;
				
				pInstBase->ImportBasic(deserial);
				
				Collision::C_TriangleMeshShape* tri=(Collision::C_TriangleMeshShape*)pInstBase->CreateShape(deserial, &filter);
				
				pInstMeshManager->AddBase(tri,pInstBase->m_OriginalExportID);


				deserial >> m_Mass;
				deserial >> m_RelativeTransformation;
				deserial >> m_IneriaTensor;

				Gravity::C_InstancedTriangleMeshDesc* pInst=new Gravity::C_InstancedTriangleMeshDesc;
				pInst->m_pShape=tri;
				pInst->m_BaseNodeId=pInstBase->m_OriginalExportID;
				pInst->m_OriginalExportID = pInstBase->m_OriginalExportID;
				pInst->m_Transform = m_RelativeTransformation;
				pInst->m_RenderOffset = pInstBase->m_RenderOffset;
				pInst->m_Remapped = pInstBase->m_Remapped;
				pInst->m_CollisionFlag = pInstBase->m_CollisionFlag;
				m_pShapeDesc = pInst;
				ownedShapes.AddToEnd(&m_pShapeDesc);
				delete pInstBase;

				succeeded = true;

				break;
			}
		case Collision::E_ImportCone_Shape:
			{
				m_pShapeDesc=new Gravity::C_ConeDesc;
				succeeded = m_pShapeDesc->Import(deserial);
				ownedShapes.AddToEnd(&m_pShapeDesc);
				deserial >> m_Mass;
				deserial >> m_RelativeTransformation;
				deserial >> m_IneriaTensor;
				break;
			}
		case Collision::E_ImportFluidMesh_Shape:
			{
				m_pShapeDesc= new Gravity::C_FluidMeshDesc;
				succeeded = m_pShapeDesc->Import(deserial);
				ownedShapes.AddToEnd(&m_pShapeDesc);

				deserial >> m_Mass;
				deserial >> m_RelativeTransformation;
				deserial >> m_IneriaTensor;

				Gravity::C_FluidMeshDesc* pFluidDesc = (Gravity::C_FluidMeshDesc*) m_pShapeDesc;
				pFluidDesc->m_Transform = m_RelativeTransformation;

				break;

			}
		default:
			{
				//BIZ_ASSERT(0);
				succeeded = false;
			}
		}
		return succeeded;// && m_pShapeDesc->ValidShapeDescriptor();
	}
#ifndef CFG_SPEED
#pragma optimize("", on)
#endif

	Collision::C_Shape* C_RopeDesc::CreateShape(const Collision::C_CollisionFilter* pFilter) const
	{
		PERF_EVENT("C_RopeDesc::CreateShape");
		if(m_HasCollision)
		{
			Collision::C_RopeShape* pRopeShape = new Collision::C_RopeShape(pFilter);

			Collision::C_RopeTree& rope = pRopeShape->m_Rope;

			rope.m_Damping = 0.995f;
			rope.m_RestLength = m_RestLength;
			rope.m_CompressCoefficient = 0.98f;
			rope.m_StretchCoefficient = 1.02f;
			rope.m_Thickness = m_Thickness;
			rope.m_numParticles = m_Vertices.m_numElements;
			rope.m_pParticles = new Collision::C_GravityParticle[rope.m_numParticles];
			for(U32 a = 0; a < rope.m_numParticles; ++a)
			{
				Collision::C_GravityParticle& par = rope.m_pParticles[a];
				par.m_CurPosition = m_Vertices[a].m_Position;
				par.m_PrePosition = m_Vertices[a].m_Position;
				par.m_Mass = m_Vertices[a].m_Mass;
				par.m_MassInverse = m_Vertices[a].m_Mass.Reciprocal();
				par.m_ExternalForce = Maths::Constants<Maths::HVec3>::Zero();
			}
			pRopeShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
			pRopeShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
			pRopeShape->m_UserData = m_UserDataFlag;
			pRopeShape->m_Rope.InitialiseEdgesFromParticleList();
			pRopeShape->m_Rope.CalculateParticleEdgeData();
			pRopeShape->m_Rope.CalculateRopeTree();

			return pRopeShape;
		}
		else
		{
			Collision::C_NonCollidableRopeShape* pRopeShape = new Collision::C_NonCollidableRopeShape(pFilter);

			Collision::C_Rope& rope = pRopeShape->m_Rope;

			rope.m_Damping = 0.995f;
			rope.m_RestLength = m_RestLength;
			rope.m_CompressCoefficient = 0.98f;
			rope.m_StretchCoefficient = 1.02f;
			rope.m_Thickness = m_Thickness;
			rope.m_numParticles = m_Vertices.m_numElements;
			rope.m_pParticles = new Collision::C_GravityParticle[rope.m_numParticles];
			for(U32 a = 0; a < rope.m_numParticles; ++a)
			{
				Collision::C_GravityParticle& par = rope.m_pParticles[a];
				par.m_CurPosition = m_Vertices[a].m_Position;
				par.m_PrePosition = m_Vertices[a].m_Position;
				par.m_Mass = m_Vertices[a].m_Mass;
				par.m_MassInverse = m_Vertices[a].m_Mass.Reciprocal();
				par.m_ExternalForce = Maths::Constants<Maths::HVec3>::Zero();
			}
			pRopeShape->m_PhysicsMaterialIndex = m_PhysicsMaterialIndex;
			pRopeShape->m_EffectMaterialIndex = m_EffectMaterialIndex;
			pRopeShape->m_UserData = m_UserDataFlag;
			pRopeShape->m_Rope.InitialiseEdgesFromParticleList();
			pRopeShape->m_Rope.CalculateParticleEdgeData();
			return pRopeShape;
		}
	}

	Collision::C_Shape* C_RopeDesc::CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const
	{
		BIZ_ASSERT(0);
		return NULL;
	}


	Collision::C_CRAxisBox C_RopeDesc::GetBoundingBox() const
	{
		Collision::C_AxisBoxMinMax box;
		box.SetInvalidExtents();
		for(S32 a = 0; a < m_Vertices.m_numElements; ++a)
		{
			box.m_min = Maths::Min(box.m_min, m_Vertices[a].m_Position);
			box.m_max = Maths::Max(box.m_max, m_Vertices[a].m_Position);
		}

		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		Collision::C_CRAxisBox axisBox;
		axisBox.centre = (box.m_min + box.m_max) * half;
		axisBox.extents = (box.m_max - box.m_min) * half;

		return axisBox;
	}


	Collision::C_Shape* C_ContinuousShapeDesc::CreateShape(const Collision::C_CollisionFilter* pFilter) const
	{
		PERF_EVENT("C_ContinuousDesc::CreateShape");
		if(!ValidShapeDescriptor())
			return NULL;
		Collision::C_ContinuousShape* pContinuous = new Collision::C_ContinuousShape(pFilter);
		Collision::C_Shape* pDiscrete = this->m_pDiscreteShape->CreateShape(pFilter);
		Collision::C_Shape* pCont = this->m_pContinuousShape->CreateShape(pFilter);
		pCont->m_ElementOffset = pDiscrete->m_ElementOffset;
		pContinuous->m_ElementOffset = pDiscrete->m_ElementOffset;
		
		pContinuous->m_Filter = pDiscrete->m_Filter;
		pContinuous->m_LocalBoundingBox = pDiscrete->m_LocalBoundingBox;

		pContinuous->m_pContinuousPrimitive = pCont;
		pContinuous->m_pDiscreteShape = pDiscrete;
		pContinuous->m_PreviousTransform = pDiscrete->m_PosRot;
		pContinuous->m_UserData = m_UserDataFlag;

		//...To be continued!!!
		return pContinuous;
	}

	Collision::C_Shape* C_ContinuousShapeDesc::CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const
	{
		BIZ_ASSERT(0);
		return NULL;
	}

	Collision::C_Shape* C_ContinuousShapeDesc::CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter)
	{
		return NULL;
	}


	U32 C_InstancedTriangleMeshDesc::GetSize() const
	{
		return sizeof(Collision::C_InstancedTriangleMeshShape);
	}

	U32 C_CompositeDesc::GetSize() const
	{
		return sizeof(Collision::C_CompositeShape);
	}

	U32 C_ContinuousShapeDesc::GetSize() const
	{
		return sizeof(Collision::C_ContinuousShape);
	}

	U32 C_PlaneDesc::GetSize() const
	{
		return sizeof(Collision::C_PlaneShape);
	}

	
	U32 C_BoxDesc::GetSize() const
	{
		return sizeof(Collision::C_BoxShape);
	}

	U32 C_ConeDesc::GetSize() const
	{
		return sizeof(Collision::C_ConeShape);
	}

	U32 C_SphereDesc::GetSize() const
	{
		return sizeof(Collision::C_SphereShape);
	}

	U32 C_CapsuleDesc::GetSize() const
	{
		return sizeof(Collision::C_CapsuleShape);
	}

	U32 C_CylinderDesc::GetSize() const
	{
		return sizeof(Collision::C_CylinderShape);
	}

	U32 C_ClothDesc::GetSize() const
	{
		return m_HasCollision ? sizeof(Collision::C_ClothShape) : sizeof(Collision::C_NonCollidableClothShape);
	}

	U32 C_DeformableDesc::GetSize() const
	{
		return sizeof(Collision::C_DeformableShape);
	}

	U32 C_FluidMeshDesc::GetSize() const
	{
		return sizeof(Collision::C_FluidMeshShape);
	}

	U32 C_RopeDesc::GetSize() const
	{
		return m_HasCollision ? sizeof(Collision::C_RopeShape) : sizeof(Collision::C_NonCollidableRopeShape);
	}

	U32 C_FluidDesc::GetSize() const
	{
		return sizeof(Collision::C_FluidShape);
	}

	U32 C_TriangleMeshDesc::GetSize() const
	{
		return sizeof(Collision::C_TriangleMeshShape);
	}

	U32 C_TriangleMeshBaseDesc::GetSize() const
	{
		return sizeof(Collision::C_TriangleMeshShape);
	}

	U32 C_ConvexHullDesc::GetSize() const
	{
		return sizeof(Collision::C_ConvexHullShape);
	}

	U32 C_CharacterControllerDesc::GetSize() const
	{
		return sizeof(Collision::C_CharacterControllerShape);
	}

#ifdef DEBUG_PHYSICS
#pragma optimize("", on)
#endif

};
