#ifndef SHAPE_DESC_INL
#define SHAPE_DESC_INL

#include "gravity/treebuilder/BuildClothTree.h"
#include "gravity/treebuilder/BuildClothTree.inl"
#include "gravity/treebuilder/AABBTree.h"
#include "gravity/treebuilder/AABBTree.inl"

namespace Gravity
{
	template <typename Serialiser>
	void C_ClothDesc::Export(Serialiser& serial)
	{
#if CFG_CONVERTER
		using namespace Maths;

		((C_ShapeDesc*)this)->Export<Serialiser>(serial);

		m_ExportClothTree->Export(serial);
#endif
	}

	template <typename Serialiser>
	void C_DeformableDesc::Export(Serialiser& serial)
	{
#if CFG_CONVERTER
		((C_ShapeDesc*)this)->Export<Serialiser>(serial);
		m_ExportDeformableTree->Export(serial);
#endif
	}

	template <typename Serialiser>
	void C_FluidMeshDesc::Export(Serialiser& serial)
	{
#if CFG_CONVERTER
		((C_ShapeDesc*)this)->Export<Serialiser>(serial);
		serial << m_Depth;
		m_AABBTree->Serialise<Serialiser>(serial);
		serial << m_Density << m_LinearDrag << m_AngularDrag << m_Current << m_MaxBuoyancy << m_Gravity;
#endif	
	}

	template <typename Serialiser>
	void C_TriangleMeshDesc::Export(Serialiser& serial)
	{
#if CFG_CONVERTER
		//if(m_AABBTree.GetNumTriangles() > 0)
		//{
			((C_ShapeDesc*)this)->Export<Serialiser>(serial);
			//serial << (U32)C_ShapeDesc::m_Type << C_ShapeDesc::m_ID;
			m_AABBTree->Serialise<Serialiser>(serial);
			
		//}
#endif
		
	}

	template <typename Serialiser>
	void C_TriangleMeshBaseDesc::Export(Serialiser& serial)
	{
#if CFG_CONVERTER
		//if(m_AABBTree.GetNumTriangles() > 0)
		//{
			((C_ShapeDesc*)this)->Export<Serialiser>(serial);
			//serial << (U32)C_ShapeDesc::m_Type << C_ShapeDesc::m_ID;
			m_AABBTree->Serialise<Serialiser>(serial);
		//}

#endif
	}

	template <typename Serialiser>
void C_ShapeDescWrapper::Export(Serialiser& serial)
{
#if CFG_CONVERTER
	switch(m_pShapeDesc->m_ImportType)
	{
	case Collision::E_ImportBox_Shape:
		{
			Gravity::C_BoxDesc* box=(Gravity::C_BoxDesc*)m_pShapeDesc;
			box->Export<Serialiser>(serial);
			break;
		}
	case Collision::E_ImportConvexHull_Shape:
		{
			C_ConvexHullDesc* convex=(Gravity::C_ConvexHullDesc*)m_pShapeDesc;
			convex->Export<Serialiser>(serial);
			break;
		}
	case Collision::E_ImportSphere_Shape:
		{
			Gravity::C_SphereDesc* sphere=(Gravity::C_SphereDesc*)m_pShapeDesc;
			sphere->Export<Serialiser>(serial);
			break;
		}
	case Collision::E_ImportCapsule_Shape:
		{
			Gravity::C_CapsuleDesc* capsule=(Gravity::C_CapsuleDesc*)m_pShapeDesc;
			capsule->Export<Serialiser>(serial);
			break;
		}
	case Collision::E_ImportCylinder_Shape:
		{
			Gravity::C_CylinderDesc* cylinder=(Gravity::C_CylinderDesc*)m_pShapeDesc;
			cylinder->Export<Serialiser>(serial);
			break;
		}
	case Collision::E_ImportComposite_Shape:
		{
			C_CompositeDesc* composite=(Gravity::C_CompositeDesc*)m_pShapeDesc;
			composite->Export<Serialiser>(serial);
			break;
		}
	case Collision::E_ImportTriangleMesh_Shape:
		{
			Gravity::C_TriangleMeshDesc* pDesc=(Gravity::C_TriangleMeshDesc*)m_pShapeDesc;
			pDesc->Export<Serialiser>(serial);
			break;
		}
	case Collision::E_ImportCone_Shape:
		{
			Gravity::C_ConeDesc* pDesc = (Gravity::C_ConeDesc*)m_pShapeDesc;
			pDesc->Export<Serialiser>(serial);
			break;
		}
	case Collision::E_ImportFluidMesh_Shape:
		{
			Gravity::C_FluidMeshDesc* pDesc= (Gravity::C_FluidMeshDesc*)m_pShapeDesc;
			pDesc->Export<Serialiser>(serial);
			break;
		}
	default:
		{
			BIZ_ASSERT(0);
		}
	}
	serial << m_Mass;
	serial << m_RelativeTransformation;
	serial << m_IneriaTensor;
#endif
	
}
}

#endif

