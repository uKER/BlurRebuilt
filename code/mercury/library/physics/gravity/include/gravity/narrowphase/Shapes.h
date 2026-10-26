#ifndef SHAPES_H
#define SHAPES_H

#include "gravity/narrowphase/Shape.h"
#include "gravity/narrowphase/Primitive.h"
#include "gravity/narrowphase/Primitive.inl"
#include "gravity/narrowphase/OptimisedAABBTree.h"
#include "gravity/narrowphase/OptimisedKDTree.h"
#include "gravity/sharedphysics/AllocationManager.h"
#include "gravity/narrowphase/charactercontrollerprimitive.h"
#include "gravity/cloths/Cloth.h"
#include "gravity/cloths/HighFieldFluid.h"
#include "gravity/cloths/SoftBody.h"
#include "gravity/cloths/Rope.h"
#include "gravity/cloths/RopeCollision.h"
#include "gravity/cloths/DeformableBody.h"
#include "gravity/cloths/Particle.h"
#include "system/Debug/I_DebugRender.h"

/*
 * The following file defines the interfaces for the various shape types supported in the engine
 */
namespace Collision
{
	class C_SphereShape;
	class C_BoxShape;
	class C_TriangleMeshShape;
	class C_KDTriangleMeshShape;
	class C_CapsuleShape;
	class C_CylinderShape; 
	class C_CompositeShape;
	//class C_ParticleShape;
	class C_PlaneShape;
	class C_RopeShape;
	class C_ContinuousShape;

	class C_NullShape : public C_Shape, public SharedPhysics::C_BlockItem<C_NullShape>
	{
	public:
		C_NullShape(const C_CollisionFilter* pFilter) : C_Shape(Collision::E_Null_Shape, pFilter, sizeof(C_NullShape))
		{
		}

#ifndef PLATFORM_SPU
		void Release()
		{
		}

		void CalculateVolume()
		{
		}

		void DebugRender(DbUtil::I_DebugRender* pRender)const 
		{
		}

		NOALIAS inline Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			return Maths::Constants<Maths::HVec3>::Zero();
		}

		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
		{
			return Maths::Constants<Maths::HF32>::Zero();
		}

#endif
	};

	class C_PlaneShape : public C_Shape, public SharedPhysics::C_BlockItem<C_PlaneShape>
	{
	public:
		C_PlaneShape(const C_CollisionFilter* pFilter) : C_Shape(Collision::E_Plane_Shape, pFilter, sizeof(C_PlaneShape))
		{
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_PlaneShape( const Spu::C_ConstructVirtualTable& cvt ) : C_Shape( cvt ) {}
        #endif
        
#ifndef PLATFORM_SPU
		void Release()
		{
		}

		void CalculateVolume()
		{
		}

		void DebugRender(DbUtil::I_DebugRender* pRender)const
		{
		}

		NOALIAS inline Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			return Maths::Constants<Maths::HVec3>::Zero();
		}

		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
		{
			return Maths::Constants<Maths::HF32>::Zero();
		}
#endif
		Collision::C_Plane m_Plane;
	};

	class C_SphereShape : public C_Shape, public SharedPhysics::C_BlockItem<C_SphereShape>
	{
	public:
		C_SphereShape(const C_CollisionFilter* pFilter) : C_Shape(Collision::E_Sphere_Shape, pFilter, sizeof(C_SphereShape))
		{
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_SphereShape( const Spu::C_ConstructVirtualTable& cvt ) : C_Shape( cvt ) {}
        #endif

		void Transform(const Maths::HMat43 PCREF newPosRot)
		{
			m_PosRot = newPosRot;
			const Maths::HVec3 pos(newPosRot.Translation());
			m_Sphere.Centre(pos);
			m_Sphere.WarmStart(pos);
		}

#ifndef PLATFORM_SPU
		void Release()
		{
		}

		void DebugRender(DbUtil::I_DebugRender* pRender) const;
		void CalculateVolume();
#endif

		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio);
		NOALIAS inline Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			return m_Sphere.Support(dir);
		}

		C_Sphere m_Sphere;
	};

	class C_BoxShape : public C_Shape
	{
	public:
		C_BoxShape(const C_CollisionFilter* pFilter) : C_Shape(Collision::E_Box_Shape, pFilter, sizeof(C_BoxShape))
		{
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_BoxShape( const Spu::C_ConstructVirtualTable& cvt ) : C_Shape( cvt ) {}
        #endif

		void Transform(const Maths::HMat43 PCREF newPosRot)
		{
			m_PosRot = newPosRot;
			const Maths::HVec3 pos(newPosRot.Translation());
			m_Box.Centre(pos);
			m_Box.Orientation(newPosRot.Rotation());
			m_Box.WarmStart(pos);
		}

#ifndef PLATFORM_SPU
		void Release()
		{
		}
		void DebugRender(DbUtil::I_DebugRender* pRender) const;
		void CalculateVolume();
#endif

		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio);
		NOALIAS inline Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			return m_Box.Support(dir);
		}
		C_OBB m_Box;
	};

	class C_ConeShape : public C_Shape
	{
	public:
		C_ConeShape(const C_CollisionFilter* pFilter) : C_Shape(Collision::E_Cone_Shape, pFilter, sizeof(C_ConeShape))
		{
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_ConeShape( const Spu::C_ConstructVirtualTable& cvt ) : C_Shape( cvt ) {}
        #endif

		void Transform(const Maths::HMat43 PCREF newPosRot)
		{
			using namespace Maths;
			m_PosRot = newPosRot;
			const HVec3 pos(newPosRot.Translation());
			const HMat33 rotate(newPosRot.Rotation());
			const HVec3 centre(pos + m_Cone.OffSet() * rotate);
			const C_Normal axis(m_Cone.LocalCenterAxis() * rotate);

			m_Cone.CentreOfMass(pos);
			m_Cone.Centre(centre);
			m_Cone.MidPointAxis(axis);
			m_Cone.WarmStart(centre + axis*m_Cone.Height()*Maths::Constants<Maths::HF32>::Half());
			m_Cone.Rotation(rotate);

			/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(m_Cone.m_CenterOfMass, 0.5f, Maths::RGBA(0, 255, 0,0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(m_Cone.m_MidPoint, 0.5f, Maths::RGBA(255, 255, 0,0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(m_Cone.m_MidPoint, m_Cone.m_MidPointAxis, 1.f, 0.2f, Maths::RGBA(0, 255, 0,0));*/
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(pos, m_Cone.m_MidPointAxis, 1.f, 0.2f, Maths::RGBA(0, 255, 0,0));
		}

#ifndef PLATFORM_SPU
		void Release()
		{
		}
		void DebugRender(DbUtil::I_DebugRender* pRender) const;
		void CalculateVolume();
#endif

		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio);
		NOALIAS inline Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			return m_Cone.Support(dir);
		}
		C_Cone m_Cone;
	}; 

	class C_DeformableShape : public C_Shape
	{
	public:
		C_DeformableShape(const C_CollisionFilter* pFilter) : C_Shape(Collision::E_Deformable_Shape, pFilter, sizeof(C_DeformableShape))
		{
		}

		~C_DeformableShape()
		{
		}
#ifndef PLATFORM_SPU
		void Release()
		{
		}

		void CalculateVolume()
		{
			m_Volume = Maths::Constants<Maths::HF32>::Zero();
		}

		void DebugRender(DbUtil::I_DebugRender* pRender)const
		{
		}
#endif
		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
		{
			return m_Volume;
		}

		Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			return Maths::Constants<Maths::HVec3>::Zero();
		}
		Collision::C_DeformableBody m_Deformable;
	};

	class C_ClothShape: public C_Shape
	{
	public:

		C_ClothShape(const C_CollisionFilter* pFilter) : C_Shape(Collision::E_Cloth_Shape, pFilter, sizeof(C_ClothShape))
		{
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_ClothShape( const Spu::C_ConstructVirtualTable& cvt ) : C_Shape( cvt ) {}
        #endif

#ifndef PLATFORM_SPU
		void Release()
		{
		}

		void DebugRender(DbUtil::I_DebugRender* pRender)const
		{
		}

		void CalculateVolume()
		{
			m_Volume = Maths::Constants<Maths::HF32>::Zero();
		}
#endif

		
		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
		{
			return m_Volume;
		}

		Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			return Maths::Constants<Maths::HVec3>::Zero();
		}

		Collision::C_Cloth m_Cloth;
	};

	class C_NonCollidableClothShape: public C_Shape
	{
	public:

		C_NonCollidableClothShape(const C_CollisionFilter* pFilter) : C_Shape(Collision::E_NoCollisionCloth_Shape, pFilter, sizeof(C_NonCollidableClothShape))
		{
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_NonCollidableClothShape( const Spu::C_ConstructVirtualTable& cvt ) : C_Shape( cvt ) {}
        #endif

#ifndef	PLATFORM_SPU
		void Release()
		{
		}

		void DebugRender(DbUtil::I_DebugRender* pRender)const
		{
		}

		void CalculateVolume()
		{
			m_Volume = Maths::Constants<Maths::HF32>::Zero();
		}
#endif

		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
		{
			return m_Volume;
		}

		Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			return Maths::Constants<Maths::HVec3>::Zero();
		}

		Collision::C_Cloth m_Cloth;
	};


	class C_RopeShape: public C_Shape
	{
	public:

		C_RopeShape(const C_CollisionFilter* pFilter) : C_Shape(Collision::E_Rope_Shape, pFilter, sizeof(C_RopeShape))
		{
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_RopeShape( const Spu::C_ConstructVirtualTable& cvt ) : C_Shape( cvt ) {}
        #endif

#ifndef	PLATFORM_SPU
		void Release()
		{
		}

		void DebugRender(DbUtil::I_DebugRender* pRender)const
		{
		}

		void CalculateVolume()
		{
			m_Volume = Maths::Constants<Maths::HF32>::Zero();
		}
#endif

		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
		{
			return m_Volume;
		}

		Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			return Maths::Constants<Maths::HVec3>::Zero();
		}

		Collision::C_RopeTree m_Rope;
	};

	class C_NonCollidableRopeShape: public C_Shape
	{
	public:

		C_NonCollidableRopeShape(const C_CollisionFilter* pFilter) : C_Shape(Collision::E_NoCollisionRope_Shape, pFilter, sizeof(C_NonCollidableRopeShape))
		{
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_NonCollidableRopeShape( const Spu::C_ConstructVirtualTable& cvt ) : C_Shape( cvt ) {}
        #endif

#ifndef	PLATFORM_SPU
		void Release()
		{
		}

		void DebugRender(DbUtil::I_DebugRender* pRender)const
		{
		}

		void CalculateVolume()
		{
			m_Volume = Maths::Constants<Maths::HF32>::Zero();
		}
#endif
		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
		{
			return m_Volume;
		}

		Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			return Maths::Constants<Maths::HVec3>::Zero();
		}

		Collision::C_Rope m_Rope;
	};


	class C_FluidShape: public C_Shape
	{
	public:

		C_FluidShape(const C_CollisionFilter* pFilter) : C_Shape(Collision::E_Fluid_Shape, pFilter, sizeof(C_FluidShape))
		{
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_FluidShape( const Spu::C_ConstructVirtualTable& cvt ) : C_Shape( cvt ) {}
        #endif

#ifndef	PLATFORM_SPU
		void Release()
		{
		}

		void DebugRender(DbUtil::I_DebugRender* pRender)const
		{
		}

		void CalculateVolume()
		{
			m_Volume = Maths::Constants<Maths::HF32>::Zero();
		}

#endif

		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
		{
			return m_Volume;
		}

		Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			return Maths::Constants<Maths::HVec3>::Zero();
		}

		C_Fluid m_Fluid;
	};


	class C_FluidMeshShape : public C_Shape
	{
	public:
		C_FluidMeshShape(const C_CollisionFilter* pFilter) : C_Shape(Collision::E_FluidMesh_Shape, pFilter, sizeof(C_FluidMeshShape)),
			m_AABBTree(NULL), m_FluidDepth(Maths::Constants<Maths::HF32>::Zero()), m_Normal(Maths::UNDEFINED), 
			m_Distance(Maths::UNDEFINED), m_Vertex(Maths::UNDEFINED), m_Density(Maths::Constants<Maths::HF32>::Zero()),
			m_LinearDrag(Maths::Constants<Maths::HF32>::Zero()), m_AngularDrag(Maths::Constants<Maths::HF32>::Zero()),
			m_Current(Maths::Constants<Maths::HVec3>::Zero()), m_MaxBuoyancy(Maths::Constants<Maths::HF32>::Zero()),
			m_Gravity(Maths::Constants<Maths::HF32>::Zero())
		{
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_FluidMeshShape( const Spu::C_ConstructVirtualTable& cvt )
            : C_Shape( cvt )
			, m_AABBTree( cvt ),m_FluidDepth(Maths::UNDEFINED), m_Normal(Maths::UNDEFINED), 
			m_Distance(Maths::UNDEFINED), m_Vertex(Maths::UNDEFINED), m_Density(Maths::UNDEFINED),
			m_LinearDrag(Maths::UNDEFINED), m_AngularDrag(Maths::UNDEFINED),
			m_Current(Maths::UNDEFINED), m_MaxBuoyancy(Maths::UNDEFINED),
			m_Gravity(Maths::UNDEFINED)
        {
        }
        #endif

		void Transform(const Maths::HMat43 PCREF newPosRot)
		{
			m_PosRot = newPosRot;
			m_AABBTree.m_PosRot = newPosRot;
		}

#ifndef PLATFORM_SPU
		void Release()
		{
		}

		void DebugRender(DbUtil::I_DebugRender* pRender)const
		{
		}

		void CalculateVolume()
		{
			m_Volume = Maths::Constants<Maths::HF32>::Zero();
		}

		bool HasFluid() const
		{
			return true;
		}
#endif

		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
		{
			return m_Volume;
		}

		Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			return Maths::Constants<Maths::HVec3>::Zero();
		}

		C_InstancedAABBTree m_AABBTree;
		Maths::HF32 m_FluidDepth;
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


	class C_TriangleMeshShape : public C_Shape
	{
	public:
		C_TriangleMeshShape(const C_CollisionFilter* pFilter) : C_Shape(Collision::E_TriangleMesh_Shape, pFilter, sizeof(C_TriangleMeshShape))
		{
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_TriangleMeshShape( const Spu::C_ConstructVirtualTable& cvt ) : C_Shape( cvt ) {}
        #endif
        
#ifndef	PLATFORM_SPU
		void Release()
		{
		}

		void DebugRender(DbUtil::I_DebugRender* pRender)const
		{
		}

		void CalculateVolume()
		{
			m_Volume = Maths::Constants<Maths::HF32>::Zero();
		}
#endif

		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
		{
			return m_Volume;
		}
		
		Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			return Maths::Constants<Maths::HVec3>::Zero();
		}

		OptimisedAABBTree m_AABBTree;
	};

	class C_InstancedTriangleMeshShape : public C_Shape
	{
	public:
		C_InstancedTriangleMeshShape(const C_CollisionFilter* pFilter) : C_Shape(Collision::E_InstancedTriangleMesh_Shape, pFilter, sizeof(C_InstancedTriangleMeshShape)),
			m_AABBTree(NULL)
		{
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_InstancedTriangleMeshShape( const Spu::C_ConstructVirtualTable& cvt )
            : C_Shape( cvt )
            , m_AABBTree( cvt )
        {
        }
        #endif

		void Transform(const Maths::HMat43 PCREF newPosRot)
		{
			m_PosRot = newPosRot;
			m_AABBTree.m_PosRot = newPosRot;
		}

#ifndef	PLATFORM_SPU
		void Release()
		{
		}

		void DebugRender(DbUtil::I_DebugRender* pRender)const
		{
		}

		void CalculateVolume()
		{
			m_Volume = Maths::Constants<Maths::HF32>::Zero();
		}
#endif

		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
		{
			return m_Volume;
		}

		Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			return Maths::Constants<Maths::HVec3>::Zero();
		}

		C_InstancedAABBTree m_AABBTree;
	};

	class C_KDTriangleMeshShape : public C_Shape
	{
	public:
		C_KDTriangleMeshShape(const C_CollisionFilter* pFilter) : C_Shape(Collision::E_KDTriangleMesh_Shape, pFilter, sizeof(C_KDTriangleMeshShape))
		{
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_KDTriangleMeshShape( const Spu::C_ConstructVirtualTable& cvt ) : C_Shape( cvt ) {}
        #endif

		~C_KDTriangleMeshShape()
		{
		}

#ifndef	PLATFORM_SPU
		void Release()
		{
		}

		void DebugRender(DbUtil::I_DebugRender* pRender)const
		{
		}

		void CalculateVolume()
		{
			m_Volume = Maths::Constants<Maths::HF32>::Zero();
		}
	
#endif

		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
		{
			return m_Volume;
		}

		Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			return Maths::Constants<Maths::HVec3>::Zero();
		}

		OptimisedKDTree m_KDTree;
	};

	class C_CapsuleShape: public C_Shape
	{
	public:
		C_CapsuleShape(const C_CollisionFilter* pFilter): C_Shape(Collision::E_Capsule_Shape, pFilter, sizeof(C_CapsuleShape) )
		{
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_CapsuleShape( const Spu::C_ConstructVirtualTable& cvt ) : C_Shape( cvt ) {}
        #endif


		void Transform(const Maths::HMat43 PCREF newPosRot)
		{
			using namespace Maths;

			m_PosRot = newPosRot;
			const HVec3 pos(newPosRot.Translation());
			const HMat33 rot(newPosRot.Rotation());
			const HVec3 start((m_Capsule.LocalStart() * rot) + pos);
			const HVec3 end((m_Capsule.LocalEnd() * rot)   + pos);
			const HVec3 centre((start + end)*Constants<HF32>::Half());

			m_Capsule.Start(start);
			m_Capsule.End(end);
			m_Capsule.Centre(centre);
			m_Capsule.WarmStart(centre);
			//m_Capsule.WarmStart(pos);
		}

#ifndef	PLATFORM_SPU
		void Release()
		{
		}
		void DebugRender(DbUtil::I_DebugRender* pRender) const;
		void CalculateVolume();
		
#endif

		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio);
		NOALIAS inline Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			return m_Capsule.Support(dir);
		}

		C_Capsule m_Capsule;
	};

	class C_CylinderShape: public C_Shape
	{
	public:
		C_CylinderShape(const C_CollisionFilter* pFilter): C_Shape(Collision::E_Cylinder_Shape, pFilter, sizeof(C_CylinderShape))
		{
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_CylinderShape( const Spu::C_ConstructVirtualTable& cvt ) : C_Shape( cvt ) {}
        #endif


		void Transform(const Maths::HMat43 PCREF newPosRot)
		{
			m_PosRot = newPosRot;
			const Maths::HVec3 pos(newPosRot.Translation());
			const Maths::HMat33 rot(newPosRot.Rotation());
			m_Cylinder.Centre(pos);
			m_Cylinder.CentreAxis(m_Cylinder.LocalCentreAxis()* rot);
			m_Cylinder.WarmStart(pos);
		}

#ifndef	PLATFORM_SPU

		void Release()
		{
		}

		void DebugRender(DbUtil::I_DebugRender* pRender)const;
		void CalculateVolume();
		
#endif

		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio);
		NOALIAS inline Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			return m_Cylinder.Support(dir);
		}
		C_Cylinder m_Cylinder;
	};

	class C_ConvexHullShape : public C_Shape
	{
	public:
		C_ConvexHullShape(const C_CollisionFilter* pFilter): C_Shape(Collision::E_ConvexHull_Shape, pFilter, sizeof(C_ConvexHullShape))
		{
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_ConvexHullShape( const Spu::C_ConstructVirtualTable& cvt ) : C_Shape( cvt ) {}
        #endif

		void Transform(const Maths::HMat43 PCREF newPosRot)
		{
			m_PosRot = newPosRot;
			const Maths::HVec3 pos(newPosRot.Translation());
			m_Hull.Centre(pos);
			m_Hull.Orientation(newPosRot.Rotation());
			m_Hull.WarmStart(pos);
		}

#ifndef	PLATFORM_SPU
		void Release()
		{
		}

		void DebugRender(DbUtil::I_DebugRender* pRender) const;
		void CalculateVolume();
		
#endif

		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio);
		NOALIAS inline Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			return m_Hull.Support(dir);
		}
		C_ConvexHull m_Hull;
	};


	class C_CharacterControllerShape : public C_Shape
	{
	public:
		C_CharacterControllerShape(const C_CollisionFilter* pFilter) : C_Shape(Collision::E_CharacterController_Shape, pFilter, sizeof(C_CharacterControllerShape))
		{
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_CharacterControllerShape( const Spu::C_ConstructVirtualTable& cvt ) : C_Shape( cvt ) {}
        #endif


		void Transform(const Maths::HMat43 PCREF newPosRot)
		{
			m_PosRot = newPosRot;
			m_Controller.Translate(newPosRot.Translation());
		}

#ifndef	PLATFORM_SPU
		void Release()
		{
		}

		void CalculateVolume()
		{
			m_Volume = Maths::Constants<Maths::HF32>::Zero();
		}
	
		void DebugRender(DbUtil::I_DebugRender* pRender)const;
#endif

		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
		{
			return m_Volume;
		}

		Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			return Maths::Constants<Maths::HVec3>::Zero();
		}

		mutable C_CharacterControllerPrimitive m_Controller;
	};

	class C_ContinuousShape : public C_Shape
	{
	public:
		C_ContinuousShape(const C_CollisionFilter* pFilter) : C_Shape(Collision::E_Continuous_Shape, pFilter, sizeof(C_ContinuousShape)),
			m_pDiscreteShape(NULL), m_pContinuousPrimitive(NULL)
		{
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_ContinuousShape( const Spu::C_ConstructVirtualTable& cvt ) : C_Shape( cvt ) {}
        #endif

		~C_ContinuousShape()
		{
			if(m_pDiscreteShape)
				delete m_pDiscreteShape;
			if(m_pContinuousPrimitive)
				delete m_pContinuousPrimitive;
		}

		
#ifndef PLATFORM_SPU
		void Release()
		{
		}

		void CalculateVolume()
		{
			m_pDiscreteShape->CalculateVolume();
			m_Volume = m_pDiscreteShape->m_Volume;
		}

		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
		{
			return m_pDiscreteShape->GetSubVolume(p, normal, centroid, ratio);
		}

		void DebugRender(DbUtil::I_DebugRender* pRender)const
		{
		}

		void SetGroupId(const U32 groupId)
		{
			m_Filter.SetGroupId(groupId);
			m_pDiscreteShape->SetGroupId(groupId);
			m_pContinuousPrimitive->SetGroupId(groupId);
		}

		void SetGroupMask(const U32 index)
		{
			m_Filter.SetGroupMask(index);
			m_pDiscreteShape->SetGroupMask(index);
			m_pContinuousPrimitive->SetGroupMask(index);
		}

		void SetGroupFlags(const U32 index)
		{
			m_Filter.SetGroupFlags(index);
			m_pDiscreteShape->SetGroupFlags(index);
			m_pContinuousPrimitive->SetGroupFlags(index);
		}

		void SetCollisionMask(const U32 index)
		{
			m_Filter.SetCollisionMask(index);
			m_pDiscreteShape->SetCollisionMask(index);
			m_pContinuousPrimitive->SetCollisionMask(index);
		}

		void SetCollisionFlags(const U32 index)
		{
			m_Filter.SetCollisionFlags(index);
			m_pDiscreteShape->SetCollisionFlags(index);
			m_pContinuousPrimitive->SetCollisionFlags(index);
		}


		Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			//return Maths::Constants<Maths::HVec3>::Zero();
			return m_pContinuousPrimitive->Support(dir);
		}
#endif


		//Can't do this in here - has to be done in manager!
		/*virtual void Transform(const Maths::HMat43 PCREF newPosRot)
		{
			m_PreviousTransform = m_PosRot;
			m_PosRot = newPosRot;
			m_pDiscreteShape->Transform(newPosRot);
			m_pContinuousPrimitive->Transform(newPosRot);
		}*/

		Maths::HMat43 m_PreviousTransform;
		C_Shape* m_pDiscreteShape;
		C_Shape* m_pContinuousPrimitive;
		C_Shape* m_pSpuContinuousPrimitive;
	};

	/*
	 * An iterator for a shape which is just a splat of memory.
	 * Note - this iterator will not traverse a recursive data structure.
	 * You will need to make a new iterator to traverse any sub-shape!!!
	 */

	class C_CompositeShapeIterator
	{
	private:
		U8* m_pData;
		Collision::C_Shape* m_pCurrentShape;
		U32 m_Offset;
		U32 m_DataSize;

	public:
		//N.B. The data in pShapeData MUST be on SPU in the case of using this class on the 
		//SPU. This requires some thought as to how to manage the memory usage of very large objects
		C_CompositeShapeIterator(void* pShapeData, U32 dataSize) : m_pData((U8*)pShapeData),
			m_pCurrentShape(NULL),
			m_Offset(0), m_DataSize(dataSize)
		{
		}

		C_CompositeShapeIterator(const Collision::C_CompositeShape* pShape);

		E_ShapeType GetCurrentShapeType()
		{
			return m_pCurrentShape->m_ShapeType;
		}

		template <typename Shape>
		Shape* GetCurrentShape()
		{
			return (Shape*)m_pCurrentShape;
		}

		bool HasNextShape()
		{
			return m_Offset < m_DataSize;
		}

		void Reset()
		{
			m_Offset = 0;
			m_pCurrentShape = NULL;
		}

		void NextShape()
		{
			Collision::C_Shape* pShape = (Collision::C_Shape*)(m_pData + m_Offset);
			//Warning - probably a LHS!!!
			m_Offset += pShape->m_Size;
			m_pCurrentShape = pShape;
		}

	};


	class C_CompositeShape : public C_Shape
	{
	public:
		C_CompositeShape(const C_CollisionFilter* pFilter, U32 size, U32 numShapes): C_Shape(Collision::E_Composite_Shape, pFilter, sizeof(C_CompositeShape))
			, m_pShapes(NULL), m_NumShapes(numShapes), m_CompositeSize(size)
		{
#ifndef PLATFORM_SPU
			m_pShapes = (U8*)BizAlignedMalloc(size, 16);
#endif
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_CompositeShape( const Spu::C_ConstructVirtualTable& cvt ) : C_Shape( cvt ) {}
        #endif

		~C_CompositeShape();

#ifndef PLATFORM_SPU
		void SetFilter(C_CollisionFilter* pFilter)
		{
			m_Filter = *pFilter;
			C_CompositeShapeIterator iter(m_pShapes, m_CompositeSize);

			while(iter.HasNextShape())
			{
				iter.NextShape();
				C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
				pShape->SetFilter(pFilter);
			}
		}


		void SetGroupId(const U32 groupId)
		{
			m_Filter.SetGroupId(groupId);

			C_CompositeShapeIterator iter(m_pShapes, m_CompositeSize);

			while(iter.HasNextShape())
			{
				iter.NextShape();
				C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
				pShape->SetGroupId(groupId);
			}

		}

		void SetGroupMask(const U32 groupId)
		{
			m_Filter.SetGroupMask(groupId);

			C_CompositeShapeIterator iter(m_pShapes, m_CompositeSize);

			while(iter.HasNextShape())
			{
				iter.NextShape();
				C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
				pShape->SetGroupMask(groupId);
			}

		}

		void SetGroupFlags(const U32 groupFlags)
		{
			m_Filter.SetGroupFlags(groupFlags);

			C_CompositeShapeIterator iter(m_pShapes, m_CompositeSize);

			while(iter.HasNextShape())
			{
				iter.NextShape();
				C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
				pShape->SetGroupFlags(groupFlags);
			}

		}

		void SetCollisionFlags(const U32 collisionFlags)
		{
			m_Filter.SetCollisionFlags(collisionFlags);
		}

		void SetCollisionMask(const U32 collisionMask)
		{
			m_Filter.SetCollisionMask(collisionMask);

			C_CompositeShapeIterator iter(m_pShapes, m_CompositeSize);

			while(iter.HasNextShape())
			{
				iter.NextShape();
				C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
				pShape->SetCollisionMask(collisionMask);
			}
		}

		void SetGroupFilter(C_CollisionFilter* pFilter);

		void Release()
		{
		}

		void DebugRender(DbUtil::I_DebugRender* pRender) const;
		void CalculateVolume();

		Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio);

		bool HasFluid() const
		{
			bool hasFluid = m_NumShapes > 0;
			C_CompositeShapeIterator iter(m_pShapes, m_CompositeSize);

			while(iter.HasNextShape())
			{
				iter.NextShape();
				C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
				hasFluid = hasFluid && pShape->HasFluid();
			}
			return hasFluid;
		}


		Maths::HVec3 Support(const Maths::HVec3 PCREF dir)
		{
			using namespace Maths;
			HVec3 support(Constants<HVec3>::Zero());
			HF32 dist(Constants<HF32>::Zero());
			C_CompositeShapeIterator iter(m_pShapes, m_CompositeSize);

			while(iter.HasNextShape())
			{
				iter.NextShape();
				C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
				const HVec3 s=pShape->Support(dir);
				const HF32 d = dir|s;
				if(d.GT(dist))
				{
					support = s;
					dist =d;
				}
			}
			return support;
		}

		void InvalidateShape();
#endif

		

		//Set the sub shapes' index as the parent shape
		void SetIndex(const S32 index)
		{
			m_Index = index;
			C_CompositeShapeIterator iter(m_pShapes, m_CompositeSize);

			while(iter.HasNextShape())
			{
				iter.NextShape();
				C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
				pShape->SetIndex(index);
			}
		}

		void SetPhysicsMaterialIndex( const U16 index )
		{
			m_PhysicsMaterialIndex = index;
			C_CompositeShapeIterator iter(m_pShapes, m_CompositeSize);

			while(iter.HasNextShape())
			{
				iter.NextShape();
				C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
				pShape->SetPhysicsMaterialIndex(index);
			}
		}

		void SetEffectMaterialIndex( const U16 index )
		{
			m_EffectMaterialIndex = index;
			C_CompositeShapeIterator iter(m_pShapes, m_CompositeSize);

			while(iter.HasNextShape())
			{
				iter.NextShape();
				C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
				pShape->SetEffectMaterialIndex(index);
			} 
		}

		U8* m_pShapes;
		U32 m_NumShapes;
		U32 m_CompositeSize;
	};

	inline C_CompositeShapeIterator::C_CompositeShapeIterator(const Collision::C_CompositeShape* pShape)
	{
		m_pData = (U8*)Spu::FetchAllocUnaligned(pShape->m_pShapes, pShape->m_CompositeSize);
		m_pCurrentShape = NULL;
		m_Offset = 0;
		m_DataSize = pShape->m_CompositeSize;
	}





};

#endif

