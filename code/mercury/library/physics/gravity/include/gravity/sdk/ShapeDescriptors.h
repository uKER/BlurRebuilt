#ifndef SHAPEDESCRIPTORS_H
#define SHAPEDESCRIPTORS_H

#include "gravity/narrowphase/AxisBox.h"
#include "gravity/sdk/BodyHelper.h"
#include "gravity/sdk/CompositeDescriptor.h"
#include "gravity/sharedphysics/Constants.h"

//#include "gravity/narrowphase/Shapes.h"
//#include "gravity/sharedphysics/Array.h"
//#include "gravity/treebuilder/AABBTree.h"
//#include "gravity/importer/InstanceMeshResourceManager.h"
//#include "gravity/importer/Importer.h"
//#include "gravity/Cloths/ClothCollision.h"
//#include "gravity/sdk/CompositeDescriptor.h"

//#define PARALLEL_AXIS_INERTIA
namespace Baking
{
	class C_BuildSoftBodyTree;
	class AABBTree;
}

namespace Collision
{
	class C_SoftBodyTree;
	class C_Shape;
	class C_CollisionFilter;
	class OptimisedAABBTree;
	class C_TriangleMeshShape;
	class C_ConvexHull;
}

namespace Gravity
{

class C_PlaneDesc : public C_ShapeDesc
{
	public:
	Maths::C_Normal m_Normal;
	Maths::HF32 m_Distance;

	C_PlaneDesc() : C_ShapeDesc(Collision::E_Plane_Shape, Collision::E_ImportPlane_Shape), m_Normal(Maths::UNDEFINED),
		m_Distance(Maths::UNDEFINED)
	{
	}

	virtual ~C_PlaneDesc()
	{
	}

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter) const;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const;

	virtual Collision::C_Shape* CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter);

	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass) const
	{
		return Maths::Constants<Maths::HMat33>::Zero();
	}

	virtual Collision::C_CRAxisBox GetBoundingBox() const
	{
		Collision::C_CRAxisBox box;
		box.centre = Maths::Constants<Maths::HVec3>::Zero();
		box.extents = box.centre;
		return box;
	}

	virtual Maths::HF32 GetSlop() const
	{
		return Maths::Constants<Maths::HF32>::Zero();
	}

	virtual U32 GetSize() const;
	
	//typedef C_ShapeDesc<Deserialiser> ShapeDescDeserialiser;

	virtual bool Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);
		//((C_ShapeDesc<Deserialiser>*)this)->Import(deserial);
		deserial >> m_Normal >> m_Distance;

		return true;
	}

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		//C_ShapeDesc<Deserialiser>::Export<Serialiser>(serial);
		((C_ShapeDesc*)this)->Export<Serialiser>(serial);
		serial<<m_Normal << m_Distance;
	}
};

class C_BoxDesc : public C_ShapeDesc 
{
public:
	Maths::HVec3 m_HalfExtents;

	C_BoxDesc() : C_ShapeDesc(Collision::E_Box_Shape, Collision::E_ImportBox_Shape), m_HalfExtents(Maths::UNDEFINED)
	{
	}

	virtual ~C_BoxDesc()
	{
	}

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter) const;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const;

	virtual Collision::C_Shape* CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter);

	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass) const;

	virtual Collision::C_CRAxisBox GetBoundingBox() const;

	virtual U32 GetSize() const;


	virtual Maths::HF32 GetSlop() const
	{
		return this->m_Margin * 0.005f;
	}

	virtual Maths::HF32 CalculateVolume()
	{
		return m_HalfExtents.X() * m_HalfExtents.Y() * m_HalfExtents.Z();
	}
	
	//typedef C_ShapeDesc<Deserialiser> ShapeDescDeserialiser;

	virtual bool Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);
		//((C_ShapeDesc<Deserialiser>*)this)->Import(deserial);
		deserial >> m_HalfExtents;

		return true;
	}

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		//C_ShapeDesc<Deserialiser>::Export<Serialiser>(serial);
		((C_ShapeDesc*)this)->Export<Serialiser>(serial);
		serial<<m_HalfExtents;
	}
};


class C_ConeDesc : public C_ShapeDesc 
{
public:
	
	Maths::HF32  m_BaseRadius;
	Maths::HF32  m_Height;

	C_ConeDesc() : C_ShapeDesc(Collision::E_Cone_Shape, Collision::E_ImportCone_Shape), m_BaseRadius(Maths::UNDEFINED),
		m_Height(Maths::UNDEFINED)
	{
	}

	virtual ~C_ConeDesc()
	{
	}

	virtual Maths::HF32 CalculateVolume()
	{
		return SharedPhysics::Constants::Third() * Maths::Constants<Maths::HF32>::Pi() * m_BaseRadius * m_BaseRadius * m_Height;
	}

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter) const;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const;

	virtual Collision::C_Shape* CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter);

	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass) const;

	virtual Collision::C_CRAxisBox GetBoundingBox() const;

	virtual U32 GetSize() const;


	virtual Maths::HF32 GetSlop() const
	{
		return this->m_Margin * 0.005f;
	}
	
	//typedef C_ShapeDesc<Deserialiser> ShapeDescDeserialiser;

	virtual bool Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);
		//((C_ShapeDesc<Deserialiser>*)this)->Import(deserial);
		deserial >> m_BaseRadius >> m_Height;

		return true;
	}

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		//C_ShapeDesc<Deserialiser>::Export<Serialiser>(serial);
		((C_ShapeDesc*)this)->Export<Serialiser>(serial);
		serial  << m_BaseRadius << m_Height;
	}
};


class C_SphereDesc : public C_ShapeDesc 
{
public:

	C_SphereDesc() : C_ShapeDesc(Collision::E_Sphere_Shape, Collision::E_ImportSphere_Shape), m_Radius(Maths::UNDEFINED)
	{
	}

	virtual ~C_SphereDesc()
	{
	}

	Maths::HF32 m_Radius;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter) const;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const;

	virtual Collision::C_Shape* CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter);

	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass) const;

	virtual Collision::C_CRAxisBox GetBoundingBox() const;

	virtual U32 GetSize() const;

	virtual Maths::HF32 GetSlop() const
	{
		return m_Radius * 0.001f;
	}

	virtual Maths::HF32 CalculateVolume()
	{
		return Maths::Constants<Maths::HF32>::Pi() * m_Radius * m_Radius;
	}


	virtual bool Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);
		//((C_ShapeDesc<Deserialiser>*)this)->Import(deserial);
		deserial >> m_Radius;

		return true;
	}

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		((C_ShapeDesc*)this)->Export<Serialiser>(serial);
		//C_ShapeDesc<Deserialiser>::Export<Serialiser>(serial);
		serial << m_Radius;
	}
};



class C_CapsuleDesc : public C_ShapeDesc
{
public:
	//Maths::HVec3 m_LocalStart;	//in local space
	//Maths::HVec3 m_LocalEnd;		//in local space
	Maths::HF32 m_Height; //the total length between the start and end point
	Maths::HF32 m_Radius;

	C_CapsuleDesc() : C_ShapeDesc(Collision::E_Capsule_Shape, Collision::E_ImportCapsule_Shape), m_Height(Maths::UNDEFINED),
		m_Radius(Maths::UNDEFINED)
	{
	}

	virtual ~C_CapsuleDesc()
	{
	}

	virtual Maths::HF32 CalculateVolume()
	{
		const Maths::HF32 pi(Maths::Constants<Maths::HF32>::Pi());
		const Maths::HF32 r2(m_Radius * m_Radius);
		return pi*r2*m_Height + pi*r2;
	}
	
	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter) const;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const;

	virtual Collision::C_Shape* CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter);

	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass) const;

	virtual Collision::C_CRAxisBox GetBoundingBox() const;

	virtual U32 GetSize() const;

	virtual bool Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);
		//((C_ShapeDesc<Deserialiser>*)this)->Import(deserial);
		deserial >> m_Height >> m_Radius;
		return true;
	}

	template <typename Serialiser>
	void Export(Serialiser& serial) const
	{
		((C_ShapeDesc*)this)->Export<Serialiser>(serial);
		//C_ShapeDesc<Deserialiser>::Export<Serialiser>(serial);
		serial<< m_Height << m_Radius;
	}

};



class C_CylinderDesc : public C_ShapeDesc
{
public:
	//Not needed - cylinder always declared along axis (0, 1, 0)
	//Maths::HVec3 m_LocalCenterAxis;	//in local space 
	//Maths::HVec3 m_Center;			//world space
	Maths::HF32  m_Height;
	Maths::HF32  m_Radius;

	C_CylinderDesc() : C_ShapeDesc(Collision::E_Cylinder_Shape, Collision::E_ImportCylinder_Shape), m_Height(Maths::UNDEFINED),
		m_Radius(Maths::UNDEFINED)
	{
	}

	virtual ~C_CylinderDesc()
	{
	}

	virtual Maths::HF32 CalculateVolume()
	{
		const Maths::HF32 pi(Maths::Constants<Maths::HF32>::Pi());
		const Maths::HF32 r2(m_Radius * m_Radius);
		return pi*r2*m_Height;
	}

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter) const;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const;

	virtual Collision::C_Shape* CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter);

	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass) const;

	virtual Collision::C_CRAxisBox GetBoundingBox() const;

	virtual U32 GetSize() const;

	virtual bool Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);
		//((C_ShapeDesc<Deserialiser>*)this)->Import(deserial);
		deserial >> m_Height >> m_Radius;
		return true;
	}

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		((C_ShapeDesc*)this)->Export<Serialiser>(serial);
		//C_ShapeDesc<Deserialiser>::Export<Serialiser>(serial);
		serial<< m_Height << m_Radius;
	}
};


class C_ClothDesc : public C_ShapeDesc
{
public:
	C_ClothDesc();

	virtual ~C_ClothDesc();

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter) const;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const;

	virtual Collision::C_Shape* CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter);

	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass) const;
	
	virtual Collision::C_CRAxisBox GetBoundingBox() const;

	virtual bool ValidShapeDescriptor() const;

	virtual bool Import(Deserialiser& deserial);

	virtual U32 GetSize() const;

	template <typename Serialiser>
	void Export(Serialiser& serial);

	Baking::C_BuildSoftBodyTree* m_ExportClothTree;
	Collision::C_SoftBodyTree* m_ImportClothTree;
	bool m_HasCollision;
};


class C_DeformableDesc: public C_ShapeDesc
{
public:
	C_DeformableDesc();

	virtual ~C_DeformableDesc();

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter) const;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const;

	virtual Collision::C_Shape* CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter);

	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass) const;
	
	virtual Collision::C_CRAxisBox GetBoundingBox() const;

	virtual bool ValidShapeDescriptor() const;

	virtual bool Import(Deserialiser& deserial);

	virtual U32 GetSize() const;

	template <typename Serialiser>
	void Export(Serialiser& serial);

	Baking::C_BuildSoftBodyTree* m_ExportDeformableTree;
	Collision::C_SoftBodyTree* m_ImportDeformableTree;
	bool m_HasCollision;

};


class C_FluidMeshDesc : public C_ShapeDesc
{
public:

	C_FluidMeshDesc();

	virtual ~C_FluidMeshDesc();

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter) const;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const;

	virtual Collision::C_Shape* CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter);
	
	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass) const;

	virtual Collision::C_CRAxisBox GetBoundingBox() const;

	virtual bool HasCentreOfMass()
	{
		return false;
	}

	virtual U32 GetSize() const;

	U32 GetCombinedCollisionFlags() const;

	virtual bool Import(Deserialiser& deserial);

	template <typename Serialiser>
	void Export(Serialiser& serial);

	virtual bool ValidShapeDescriptor() const;


#if !CFG_CONVERTER
	Collision::OptimisedAABBTree* m_OptimisedTree;
#else
	Baking::AABBTree* m_AABBTree;
#endif

	F32 m_Depth;
	Maths::HMat43 m_Transform;
	//Fluid plane!!!
	Maths::C_Normal m_Normal;
	Maths::HF32 m_Distance;
	Maths::HVec3 m_Vertex;
	Maths::HF32 m_Density;
	Maths::HF32 m_LinearDrag;
	Maths::HF32 m_AngularDrag;
	Maths::HVec3 m_Current;
	Maths::HF32 m_MaxBuoyancy;
	Maths::HF32 m_Gravity;

};


class C_DescriptorParticle
{
public:
	Maths::HVec3 m_Position;
	Maths::HF32 m_Mass;
};

class C_RopeDesc : public C_ShapeDesc
{
public:
	C_RopeDesc() : m_HasCollision(true), m_RestLength(Maths::UNDEFINED), m_Thickness(Maths::UNDEFINED)
	{
	}

	virtual ~C_RopeDesc()
	{
	}

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter) const;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const;
	
	virtual Collision::C_Shape* CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter)
	{
		return NULL;
	}

	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass) const
	{
		return Maths::HMat33(Maths::ZERO_MATRIX);
	}
	
	virtual Collision::C_CRAxisBox GetBoundingBox() const;

	virtual bool ValidShapeDescriptor() const
	{
		return true;
	}

	virtual bool Import(Deserialiser& deserial)
	{
		return true;
	}

	virtual U32 GetSize() const;

	SharedPhysics::SortedArray<C_DescriptorParticle> m_Vertices;
	Maths::HF32 m_RestLength;
	Maths::HF32 m_Thickness;
	bool m_HasCollision;
};


class C_FluidDesc : public C_ShapeDesc
{
public:

	C_FluidDesc() : C_ShapeDesc(Collision::E_Fluid_Shape, Collision::E_ImportFluid_Shape)
	{
	}

	virtual ~C_FluidDesc()
	{
	}

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter) const;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const;

	virtual Collision::C_Shape* CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter);

	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass) const;
	
	virtual Collision::C_CRAxisBox GetBoundingBox() const;

	virtual U32 GetSize() const;

	virtual bool Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);
		return true;
	}


	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		using namespace Maths;

		((C_ShapeDesc*)this)->Export<Serialiser>(serial);
	}

};


class C_TriangleMeshDesc : public C_ShapeDesc
{
public:

	C_TriangleMeshDesc();

	virtual ~C_TriangleMeshDesc();

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter) const;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const;

	virtual Collision::C_Shape* CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter);

	virtual bool HasCentreOfMass()
	{
		return false;
	}

	virtual bool ValidShapeDescriptor() const;
	
	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass) const;

	virtual Collision::C_CRAxisBox GetBoundingBox() const;

	U32 GetCombinedCollisionFlags() const;

	virtual bool Import(Deserialiser& deserial);

	virtual U32 GetSize() const;

//#ifdef CFG_CONVERTER
	template <typename Serialiser>
	void Export(Serialiser& serial);
#if !CFG_CONVERTER
	Collision::OptimisedAABBTree* m_OptimisedTree;
#else
	Baking::AABBTree* m_AABBTree;
#endif
	mutable bool m_Instanced;
//#endif



};

class C_TriangleMeshBaseDesc : public C_ShapeDesc
{
public:

	C_TriangleMeshBaseDesc();

	virtual ~C_TriangleMeshBaseDesc();

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter) const;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const;

	virtual Collision::C_Shape* CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter);
	
	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass) const;

	virtual Collision::C_CRAxisBox GetBoundingBox() const;

	virtual bool HasCentreOfMass()
	{
		return false;
	}

	U32 GetCombinedCollisionFlags() const;

	virtual bool Import(Deserialiser& deserial);

	template <typename Serialiser>
	void Export(Serialiser& serial);

	virtual bool ValidShapeDescriptor() const;

	virtual U32 GetSize() const;

#if !CFG_CONVERTER
	Collision::OptimisedAABBTree* m_OptimisedTree;
#else
	Baking::AABBTree* m_AABBTree;
#endif

};

class C_TriangleIndices
{
	
public:

	C_TriangleIndices(U32 indA, U32 indB, U32 indC) : m_IndexA(indA), m_IndexB(indB), m_IndexC(indC)
	{
	}

	C_TriangleIndices()
	{
	}

	
	void Deserialise(Deserialiser& deserial)
	{
		deserial >> m_IndexA >> m_IndexB >> m_IndexC;
	}

	template <class Serial>
	void Serialise(Serial& serial)
	{
		serial << m_IndexA << m_IndexB << m_IndexC;
	}

	U32 m_IndexA, m_IndexB, m_IndexC;

};
	
class C_ConvexHullDesc : public C_ShapeDesc
{
public:
	SharedPhysics::SortedArray<Maths::HVec3> m_Points;
	SharedPhysics::SortedArray<C_TriangleIndices> m_Triangles;

	Collision::C_ConvexHull* m_pBaseHull;
	bool m_ShareData;


	C_ConvexHullDesc(bool bShareData = false);

	virtual ~C_ConvexHullDesc();

	void AddPoint(Maths::HVec3 PCREF point);
	void AddTriangle(C_TriangleIndices tri);

	virtual Maths::HF32 CalculateVolume()
	{
		Collision::C_CRAxisBox box = GetBoundingBox();
		return box.extents.X() * box.extents.Y() * box.extents.Z();
	}

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter) const;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const;

	virtual Collision::C_Shape* CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter);

	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass) const;

	virtual Collision::C_CRAxisBox GetBoundingBox() const;

	virtual void RemapShape();

	virtual Maths::HVec3 CalculateCentreOfMass();

	virtual U32 GetSize() const;

	virtual void ApplyStaticRotation(const Maths::HMat33 PCREF rot);

	virtual bool ValidShapeDescriptor() const
	{
		return m_Triangles.m_numElements > 0;
	}

	void CreateSharedData();

	virtual bool Import(Deserialiser& deserial);

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		((C_ShapeDesc*)this)->Export<Serialiser>(serial);
		//C_ShapeDesc<Deserialiser>::Export<Serialiser>(serial);
		serial << m_Points.m_numElements << m_Triangles.m_numElements;

		for(S32 i=0; i<m_Points.m_numElements; ++i)
		{
			serial << m_Points[i];
		}

		for(S32 i=0; i<m_Triangles.m_numElements; ++i)
		{
			 m_Triangles[i].Serialise<Serialiser>(serial);
		}
	}

};


class C_CharacterControllerDesc : public C_ShapeDesc
{
public:
	C_CharacterControllerDesc() : C_ShapeDesc(Collision::E_CharacterController_Shape, Collision::E_ImportCharacterController_Shape),
		m_CapsuleHeight(Maths::Constants<Maths::HF32>::Zero()), m_CapsuleRadius(Maths::Constants<Maths::HF32>::Zero()),
		m_RayHeight(Maths::Constants<Maths::HF32>::Zero()), m_RayRadius(Maths::Constants<Maths::HF32>::Zero()), 
		m_MaxCosAngle(Maths::Constants<Maths::HF32>::Zero()), m_MinPen(Maths::Constants<Maths::HF32>::Zero())
	{
	}

	virtual ~C_CharacterControllerDesc()
	{
	}

	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass) const;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter) const;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const;

	virtual Collision::C_Shape* CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter);

	virtual U32 GetSize() const;

	virtual Collision::C_CRAxisBox GetBoundingBox() const
	{
		Collision::C_CRAxisBox box;
		box.centre = Maths::Constants<Maths::HVec3>::Zero();
		box.extents.X(Maths::Max(m_CapsuleRadius, m_RayRadius));
		box.extents.Y(Maths::Max(m_CapsuleHeight + m_CapsuleRadius, m_RayHeight + m_MinPen + m_RayRadius));
		box.extents.Z(Maths::Max(m_CapsuleRadius, m_RayRadius));
		return box;
	}


	virtual bool Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);
		//((C_ShapeDesc<Deserialiser>*)this)->Import(deserial);
		//C_ShapeDesc<Deserialiser>::Export<Serialiser>(serial);
		deserial >> m_CapsuleHeight >> m_CapsuleRadius >> m_RayHeight >> m_RayRadius >> m_MaxCosAngle;

		return true;
	}


	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		((C_ShapeDesc*)this)->Export<Serialiser>(serial);
		//C_ShapeDesc<Deserialiser>::Export<Serialiser>(serial);
		serial << m_CapsuleHeight << m_CapsuleRadius << m_RayHeight << m_RayRadius << m_MaxCosAngle;
	}

	Maths::HF32 m_CapsuleHeight;
	Maths::HF32 m_CapsuleRadius;
	Maths::HF32 m_RayHeight;
	Maths::HF32 m_RayRadius;
	Maths::HF32 m_MaxCosAngle;
	Maths::HF32 m_MinPen;
	U16 m_FeetMaterial;
	U16 m_EffectMaterial;
};


class C_InstancedTriangleMeshDesc : public C_ShapeDesc
{
public:
	Collision::C_TriangleMeshShape* m_pShape;
	C_InstancedTriangleMeshDesc() : C_ShapeDesc(Collision::E_InstancedTriangleMesh_Shape, Collision::E_ImportInstancedTriangleMesh_Shape), m_pBaseDesc(NULL), m_pShape(NULL)
	{
	}

	virtual ~C_InstancedTriangleMeshDesc()
	{
	}

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter) const;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const;

	virtual Collision::C_Shape* CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter);

	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass) const;

	virtual Collision::C_CRAxisBox GetBoundingBox() const;

	virtual bool HasCentreOfMass()
	{
		return false;
	}

	virtual U32 GetSize() const;

	virtual bool ValidShapeDescriptor() const;

	virtual bool Import(Deserialiser& deserial)
	{
		//deserial >> C_ShapeDesc::m_ID >> m_BaseNodeId;
		ImportBasic(deserial);
		deserial >> m_BaseNodeId;

		return true;
	}

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		((C_ShapeDesc*)this)->Export<Serialiser>(serial);
		//serial << (U32)C_ShapeDesc::m_Type << C_ShapeDesc::m_ID << m_BaseNodeId;
		serial << m_BaseNodeId;
	}

	S32 m_BaseNodeId;
	Maths::HMat43 m_Transform;
	Gravity::C_TriangleMeshBaseDesc* m_pBaseDesc;

};

class C_ContinuousShapeDesc : public C_ShapeDesc
{
public:
	C_ContinuousShapeDesc() : C_ShapeDesc(Collision::E_Continuous_Shape, Collision::E_ImportContinuous_Shape)
	{
	}

	virtual ~C_ContinuousShapeDesc()
	{
	}

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter) const;

	virtual Collision::C_Shape* CreateShape(const Collision::C_CollisionFilter* pFilter, void* pAddress) const;

	virtual Collision::C_Shape* CreateShape(Deserialiser& deserial, const Collision::C_CollisionFilter* pFilter);

	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass)
	{
		return m_pDiscreteShape->InertiaTensor(mass);
	}

	virtual Collision::C_CRAxisBox GetBoundingBox() const
	{
		return m_pDiscreteShape->GetBoundingBox();
	}

	virtual bool HasCentreOfMass()
	{
		return m_pDiscreteShape->HasCentreOfMass();
	}

	virtual bool ValidShapeDescriptor() const
	{
		return m_pDiscreteShape && m_pDiscreteShape->ValidShapeDescriptor() && m_pContinuousShape && m_pContinuousShape->ValidShapeDescriptor();
	}

	virtual bool Import(Deserialiser& deserial)
	{
		return true;
	}

	virtual U32 GetSize() const;

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
	}

	virtual Maths::HMat33 InertiaTensor(const Maths::HF32 PCREF mass) const
	{
		return m_pDiscreteShape->InertiaTensor(mass);
	}

	C_ShapeDesc* m_pDiscreteShape;
	C_ShapeDesc* m_pContinuousShape;
};


}

#endif
