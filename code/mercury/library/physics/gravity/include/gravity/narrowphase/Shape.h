#ifndef NEWSHAPE_H
#define NEWSHAPE_H

/*
 * The following file defines the shape types supported in the engine. 
 * 
 */

#include "gravity/narrowphase/axisbox.h"
#include "gravity/narrowphase/collisionfilter.h"

namespace DbUtil
{
	class I_DebugRender;
}

namespace Collision
{

	enum E_ImportShapeType
	{
		E_ImportTriangleMesh_Shape,
		E_ImportKDTriangleMesh_Shape,
		E_ImportBox_Shape,
		E_ImportConvexHull_Shape,
		E_ImportSphere_Shape,
		E_ImportCapsule_Shape,
		E_ImportCylinder_Shape,
		E_ImportComposite_Shape,
		E_ImportCharacterController_Shape,
		E_ImportParticle_Shape,
		E_ImportInstancedTriangleMesh_Shape,
		E_ImportTriangleMeshBase_Shape,
		E_ImportCloth_Shape,
		E_ImportFluid_Shape,
		E_ImportPlane_Shape,
		E_ImportCone_Shape,
		E_ImportRope_Shape,
		E_ImportFluidMesh_Shape,
		E_ImportContinuous_Shape,
		E_ImportNoCollisionRope_Shape,
		E_ImportNoCollisionCloth_Shape,
		E_ImportNull_Shape,
		E_ImportDeformable_Shape,
		E_ImportForce_DWord = 0x7fffffff
	};

	enum E_ShapeType
	{
		E_TriangleMesh_Shape			= 0x2,//1 << 0,			
		E_KDTriangleMesh_Shape			= 0x4,//1 << 1,
		E_Box_Shape						= 0x8,//1 << 2,
		E_ConvexHull_Shape				= 0x10,//1 << 3,
		E_Sphere_Shape					= 0x20,//1 << 4,
		E_Capsule_Shape					= 0x40,//1 << 5,
		E_Cylinder_Shape				= 0x80,//1 << 6,
		E_Composite_Shape				= 0x100,//1 << 7,
		E_CharacterController_Shape		= 0x200,//1 << 8,
		E_Particle_Shape				= 0x400,//1 << 9,
		E_InstancedTriangleMesh_Shape	= 0x800,//1 << 10,
		E_TriangleMeshBase_Shape		= 0x1000,//1 << 11,
		E_Cloth_Shape					= 0x2000,//1 << 12,
		E_Fluid_Shape					= 0x4000,//1 << 13,
		E_Plane_Shape					= 0x8000,//1 << 14,
		E_Cone_Shape					= 0x10000,//1 << 15,
		E_Rope_Shape					= 0x20000,// << 16,
		E_FluidMesh_Shape				= 0x40000,//1 << 17,
		E_Continuous_Shape				= 0x80000,//1 << 18,
		E_NoCollisionRope_Shape			= 0x100000,//1 << 19,
		E_NoCollisionCloth_Shape		= 0x200000,//1 << 20,
		E_Null_Shape					= 0x400000,//1 << 21, 
		E_Deformable_Shape				= 0x800000,//1 << 22
		E_Force_DWord					= 0x7fffffff
	};

	class ALIGN16 C_Shape
	{
	public:

		C_Shape(E_ShapeType type, const C_CollisionFilter* pFilter, U32 size) : 
		  m_RelativeTransformation(Maths::Constants<Maths::HMat43>::Identity()), 
			  m_ShapeType(type), m_Filter(*pFilter), m_PhysicsMaterialIndex(0), 
			  m_EffectMaterialIndex(0), m_HasVolume(true), m_Size(size),
			  m_Index(-1),m_PosRot(Maths::Constants<Maths::HMat43>::Identity()),
			  m_ElementOffset(Maths::Constants<Maths::HVec3>::Zero()),
			  m_Volume(Maths::Constants<Maths::HF32>::Zero())
		{
			m_pPpuPtr = this;
		}
		
		#if 0 //defined( PLATFORM_SPU )
        C_Shape( const Spu::C_ConstructVirtualTable& cvt ) // TODO! Verify that nothing is getting initialised in here!
		    : m_PosRot( Maths::UNDEFINED )
			, m_RelativeTransformation(Maths::UNDEFINED) 
		    , m_ElementOffset( Maths::UNDEFINED )
		    , m_Volume( Maths::UNDEFINED )
		{
        }
        #endif

		
        
		

		NOALIAS inline const C_CollisionFilter* GetFilter() const
		{
			return &m_Filter;
		}

		U32 HasVolume() const
		{
			return m_HasVolume;
		}

		void SetHasBuoyancy(bool bHasBuoyancy)
		{
			m_HasVolume = bHasBuoyancy;
		}

		NOALIAS FORCE_INLINE bool CanCollide(C_Shape* pOther) const
		{
			return m_Filter.CanCollide(&pOther->m_Filter);
		}

		bool CanCollide(const U32 collisionFlags) const
		{
			return m_Filter.CanCollide(collisionFlags);
		}

		NOALIAS inline E_ShapeType GetShape() const
		{
			return m_ShapeType;
		}

		NOALIAS inline void Transform(Maths::HMat33 PCREF rotation, Maths::HVec3 PCREF position)
		{
			Transform(Maths::HMat43(rotation, position));
		}

		NOALIAS inline void Transform(Maths::C_Quaternion PCREF rotation, Maths::HVec3 PCREF position)
		{
			Transform(Maths::HMat43(Maths::HMat33(rotation), position));
		}

		// - This used to be virtual. Now it isn't!!!

		//The bit of code which actually transforms this shape into world position. We could do this per-test, 
		//but it would be much better to store this pre-transformed as much as possible...
		void Transform(const Maths::HMat43 PCREF newPosRot)
		{
			m_PosRot = newPosRot;
		}

		

#ifndef PLATFORM_SPU

		virtual ~C_Shape()
		{
			m_Index = -1;
		}

		virtual void SetFilter(C_CollisionFilter* pFilter)
		{
			m_Filter = *pFilter;
		}

		virtual void SetGroupId(const U32 index)
		{
			m_Filter.m_GroupId = index;
		}

		virtual void SetGroupMask(const U32 index)
		{
			m_Filter.m_GroupMask = index;
		}

		virtual void SetGroupFlags(const U32 index)
		{
			m_Filter.m_GroupFlags = index;
		}

		virtual void SetCollisionMask(const U32 index)
		{
			m_Filter.m_CollisionMask = index;
		}

		virtual void SetCollisionFlags(const U32 index)
		{
			m_Filter.m_CollisionFlags = index;
		}

		virtual void SetGroupFilter(C_CollisionFilter* pFilter)
		{
			m_Filter.m_GroupFlags =pFilter->m_GroupFlags;
			m_Filter.m_GroupMask = pFilter->m_GroupMask;
			m_Filter.m_GroupId = pFilter->m_GroupId;
		}

		virtual void InvalidateShape()
		{
			m_Index = -1;
		}

		virtual bool HasFluid() const
		{
			return false;
		}

		virtual void CalculateVolume() = 0;
		virtual void Release() = 0;
		virtual void DebugRender(DbUtil::I_DebugRender* pRender) const = 0;
		//Get the sub volume of the shape under a plane
		virtual Maths::HF32 GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio) = 0;
		virtual Maths::HVec3 Support(const Maths::HVec3 PCREF dir) =0;
#endif

		

		inline void TransformBox(Collision::C_CRAxisBox& transformedBox)
		{
			m_LocalBoundingBox.Transform(m_PosRot, transformedBox);
		}

		NOALIAS inline Maths::HVec3 Position() const
		{
			return m_PosRot.Translation();
		}

		NOALIAS inline Maths::HMat33 Orientation() const
		{
			return m_PosRot.Rotation();
		}

		NOALIAS inline U32 PhysicsMaterialIndex() const
		{
			return m_PhysicsMaterialIndex;
		}

		virtual void SetPhysicsMaterialIndex( const U16 index )
		{
			m_PhysicsMaterialIndex = index;
		}

		NOALIAS inline U32 EffectMaterialIndex() const
		{
			return m_EffectMaterialIndex;
		}

		virtual void SetEffectMaterialIndex(const U16 index)
		{
			m_EffectMaterialIndex= index;
		}

		virtual void SetIndex(const S32 index)
		{
			m_Index = index;
		}

		S32 GetIndex() const
		{
			return m_Index;
		}
	


		Maths::HMat43 GetElementMatrix()
		{
			Maths::HVec3 newPos((-m_ElementOffset) * m_PosRot);
			return Maths::HMat43(m_PosRot.Rotation(), newPos);
		}
	
		Collision::C_CRAxisBox m_LocalBoundingBox;
		Maths::HMat43 m_PosRot;
		Maths::HMat43 m_RelativeTransformation;
		Maths::HVec3 m_ElementOffset;
		Maths::HF32 m_Volume;
		Collision::C_Shape* m_pPpuPtr;
		E_ShapeType m_ShapeType;
		C_CollisionFilter m_Filter;
		S32 m_Index;
		U32 m_UserData;
		U32 m_HasVolume;
		U32 m_Size;
		U16 m_PhysicsMaterialIndex;
		U16 m_EffectMaterialIndex;
		
		
	};

	class ALIGN16 C_ShapeWrapper
	{
	public:
		Maths::HMat43 m_RelativeTransformation;
		//Collision::C_CRAxisBox m_Box;
		C_Shape* m_pShape;

		C_ShapeWrapper(C_Shape* pShape, Maths::HMat43 PCREF relTrans) : m_RelativeTransformation(relTrans), m_pShape(pShape)
		{
			//pShape->m_LocalBoundingBox.Transform(relTrans, m_Box);
		}

		C_ShapeWrapper() : m_pShape(NULL)
		{
		}

		~C_ShapeWrapper()
		{
			//delete m_pShape;
		}

		//void Transform(const Maths::HMat43 PCREF newPosRot)
		//{
		//	//m_RelativeTransformation=newPosPot;
		//	const Maths::HMat43 absPosRot(m_RelativeTransformation * newPosRot);
		//	m_pShape->Transform(absPosRot);
		//}
	};
};

#endif
