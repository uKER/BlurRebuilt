#include "stdafx.h"
#include "gravity/Triggers/TriggerEvent.h"
#include "gravity/narrowphase/GJK.inl"
#include "gravity/narrowphase/PrimitiveTest.h"
#include "gravity/narrowphase/CollisionFilter.inl"



namespace Gravity
{

	void C_TriggerBox::CalculateLocalBoundingBox()
	{
		m_LocalExtents = m_Box.Extent();
	}

	void C_TriggerSphere::CalculateLocalBoundingBox()
	{
		m_LocalExtents = Maths::HVec3(m_Sphere.Radius());
	}

	void C_TriggerShape::GetTransformedBoundingBox(Collision::C_CRAxisBox& box)
	{
		box.extents = Maths::AbsTransform( m_LocalExtents, m_PosRot.Rotation() );
		box.centre = m_PosRot.Translation();
	}

	void C_TriggerBox::Transform(const Maths::HMat43 PCREF matrix)
	{
		m_PosRot = matrix;
		m_Box.Orientation( matrix.Rotation());
		m_Box.Centre( matrix.Translation());
	}

	void C_TriggerSphere::Transform(const Maths::HMat43 PCREF matrix)
	{
		m_PosRot = matrix;
		m_Sphere.Centre(matrix.Translation());
	}


	bool C_TriggerBox::isInsideTriggerShape(Collision::C_Shape* pShape)
	{
		using namespace Collision;
		
		if(CanCollide(pShape))
		{
			switch(pShape->m_ShapeType)
			{
			case Collision::E_Box_Shape:
				{	
					Collision::C_BoxShape* pBoxShape = (Collision::C_BoxShape*)pShape;
					C_OBB& box = pBoxShape->m_Box;
					return GJKCPair<C_OBB, C_OBB>(m_Box, box);
				}
			case Collision::E_Sphere_Shape:
				{
					Collision::C_SphereShape* pSphere = (Collision::C_SphereShape*)pShape;
					C_Sphere& sphere = pSphere->m_Sphere;
					/*return GJKCPair<C_OBB, C_Sphere>(m_Box, sphere);*/
					return CollideOBBSphere(m_Box, sphere);
				}
			case Collision::E_Capsule_Shape:
				{	
					Collision::C_CapsuleShape* pCapsuleShape = (Collision::C_CapsuleShape*)pShape;
					return GJKCPair<C_OBB, C_Capsule>(m_Box, pCapsuleShape->m_Capsule);
				}
			case Collision::E_Cylinder_Shape:
				{
					Collision::C_CylinderShape* pCylinder = (Collision::C_CylinderShape*)pShape;
					return GJKCPair<C_OBB, C_Cylinder>(m_Box, pCylinder->m_Cylinder);
				}
			case Collision::E_Cone_Shape:
				{
					Collision::C_ConeShape* pCone = (Collision::C_ConeShape*)pShape;
					return GJKCPair<C_OBB, C_Cone>(m_Box, pCone->m_Cone);
				}
			case Collision::E_ConvexHull_Shape:
				{
					Collision::C_ConvexHullShape* pConv = (Collision::C_ConvexHullShape*)pShape;
					return GJKCPair<C_OBB, C_ConvexHull>(m_Box, pConv->m_Hull);
				}
			case Collision::E_Composite_Shape:
				{
					Collision::C_CompositeShape* pComp = (Collision::C_CompositeShape*)pShape;
					Collision::C_CompositeShapeIterator iter(pComp);
					//for(U32 a = 0; a < pComp->m_NumShapes; ++a)
					while(iter.HasNextShape())
					{
						iter.NextShape();
						Collision::C_Shape* pSubShape = iter.GetCurrentShape<Collision::C_Shape>();
						if(isInsideTriggerShape(pSubShape))
							return true;
					}
					return false;
				}
			default:
				//assert(0 && "Not Supported Trigger Shape");
				return false;
			}
		}
		return false;
	}

	bool C_TriggerSphere::isInsideTriggerShape(Collision::C_Shape* pShape)
	{
		using namespace Collision;
		if(CanCollide(pShape))
		{
			switch(pShape->m_ShapeType)
			{
			case Collision::E_Box_Shape:
				{
					Collision::C_BoxShape* pBox = (Collision::C_BoxShape*)pShape;
					return CollideOBBSphere(pBox->m_Box, m_Sphere);
				}
			case Collision::E_Sphere_Shape:
				{
					Collision::C_SphereShape* pSphere = (Collision::C_SphereShape*)pShape;
					return Collision::CollideSphereSphere(pSphere->m_Sphere, m_Sphere);
				}
			case Collision::E_Capsule_Shape:
				{
					Collision::C_CapsuleShape* pCapsule = (Collision::C_CapsuleShape*)pShape;
					return Collision::CollideCapsuleSphere(pCapsule->m_Capsule, m_Sphere);
				}
			case Collision::E_Cylinder_Shape:
				{
					Collision::C_CylinderShape* pCylinder = (Collision::C_CylinderShape*)pShape;
					return Collision::CollideCylinderSphere(pCylinder->m_Cylinder, m_Sphere);
				}
			case Collision::E_Cone_Shape:
				{
					Collision::C_ConeShape* pCone = (Collision::C_ConeShape*)pShape;
					return Collision::CollideConeSphere(pCone->m_Cone, m_Sphere);
				}
			case Collision::E_ConvexHull_Shape:
				{
					Collision::C_ConvexHullShape* pHull = (Collision::C_ConvexHullShape*)pShape;
					return GJKCPair<C_Sphere, C_ConvexHull>(m_Sphere, pHull->m_Hull); 
				}
			case Collision::E_Composite_Shape:
				{
					Collision::C_CompositeShape* pComp = (Collision::C_CompositeShape*)pShape;
					Collision::C_CompositeShapeIterator iter(pComp);
					//for(U32 a = 0; a < pComp->m_NumShapes; ++a)
					while(iter.HasNextShape())
					{
						iter.NextShape();
						Collision::C_Shape* pSubShape = iter.GetCurrentShape<Collision::C_Shape>();
						if(isInsideTriggerShape(pSubShape))
							return true;
					}
					return false;
				}
			default:
				//assert(0 && "Not Supported Trigger Shape");
				return false;
			}
		}
		return false;
	}

}